//
// Created by RestRegular on 2025/1/16.
//

#include <iostream>
#include <utility>
#include <stack>
#include <filesystem>
#include "../include/ra_parser.h"
#include "../include/ra_base.h"
#include "../include/ra_core.h"
#include "../include/lib/ra_utils.h"

using namespace core;

auto &loadedExtensions = memory::loadedExtensions;

namespace parser {

    core::memory::RVM_Memory &data_space_pool = memory::RVM_Memory::getInstance();
    base::RVM_IO *io{base::RVM_IO::getInstance()};
    InsSetPool &ins_set_pool = InsSetPool::getInstance();
    utils::StringManager &string_manager = utils::StringManager::getInstance();

    InsSetPool &InsSetPool::getInstance() {
        static InsSetPool instance;
        return instance;
    }

    StdInsSet InsSetPool::acquire(const std::string &prefix) {
        if (!insSetPool_.empty()) {
            auto insSet = std::move(insSetPool_.back());
            insSetPool_.pop_back();
            *insSet = InsSet{prefix};
            insSetSpace_[insSet->getInstID().toString()] = insSet;
            insSetOrder_.push_back(insSet);
            return insSet;
        } else {
            auto new_ins_set = std::make_shared<InsSet>(InsSet{prefix});
            insSetSpace_[new_ins_set->getInstID().toString()] = new_ins_set;
            insSetOrder_.push_back(new_ins_set);
            return new_ins_set;
        }
    }

    StdInsSet InsSetPool::getAppointedInsSet(const std::string &insSetId) {
        auto it = insSetSpace_.find(insSetId);
        if (it != insSetSpace_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void InsSetPool::release(const std::string &insSetId) {
        auto it = insSetSpace_.find(insSetId);
        if (it != insSetSpace_.end()) {
            insSetPool_.push_back(it->second);
            insSetSpace_.erase(it);
            insSetOrder_.remove_if([&insSetId](const StdInsSet& insSet) {
                return insSet->getInstID().toString() == insSetId;
            });
        }
    }

    void InsSetPool::release(const StdInsSet &insSet) {
        insSetPool_.push_back(insSet);
        insSetOrder_.remove(insSet);
        insSetSpace_.erase(insSet->getInstID().toString());
    }

    void InsSetPool::releaseAll() {
        for (auto& insSet : insSetOrder_) {
            insSetPool_.push_back(insSet);
        }
        insSetOrder_.clear();
        insSetSpace_.clear();
    }

    std::shared_ptr<Ins> InsSetPool::findIns(const std::string &insSetId, const std::string &insId) const {
        auto it = insSetSpace_.find(insSetId);
        if (it != insSetSpace_.end()) {
            for (const auto& ins : it->second->getInsSet()) {
                if (ins->insId.toString() == insId) {
                    return ins;
                }
            }
        }
        return nullptr;
    }

    void InsSetPool::printInfo(std::ostream &out) const {
        out << "\n=== InsSetPool Information ===\n";
        out << "InsSets Count: " << insSetSpace_.size() << "\n\n";
        for (const auto& [name, insSet] : insSetSpace_) {
            out << "InsSet ID: " << name << "\n";
            out << "Instructions Count: " << insSet->getInsSet().size() << "\n";
            for (const auto& ins : insSet->getInsSet()) {
                out << "    Instruction ID: " << ins->insId.toString() << "\n";
            }
            out << "\n";
        }
        out << "=== End of InsSetPool Information ===\n";
    }

    namespace components {
        StdInsSet INS_SET = ins_set_pool.acquire(pre_SRC); // 主文件指令集
    }

    // 运行环境必要函数实现
    namespace env {
        utils::SerializationProfile program_serialization_profile_ = utils::SerializationProfile::Debug;

        bool precompiled_link_option {false};

        std::string precompiled_link_dir_path = R"(D:\ClionProjects\RVM\lib)";

        bool has_initial = false;

        /*
         * 运行期全局数据名的单一事实来源。
         *
         * 这个列表同时被两处使用，两边绝不允许出现差异：
         *   1. initialize() 用它把全局数据注册进 RVM_Memory 的 globalScope；
         *   2. 解析期用它把同名变量登记进文件根作用域，使每个标识符都能被解析成槽位。
         *
         * SR 与 _SE 也在列表中：它们同样需要根帧槽位。SR 是 RET/IVOK 的返回值中转位，
         * _SE 是 EXPO 的错误传播位，两者都会被运行期改写；槽位模式必须在根帧里给它们
         * 留出位置，否则取到的是空槽位。它们的 DataID 另外由 SR_SPACE_ID / SE_SPACE_ID
         * 保存，供旧系统路径使用（见 initialize()）。
         */
        const std::vector<std::string> &getRuntimeGlobalDataNames() {
            static const std::vector<std::string> names{
                    "SN", "SE", "SS",
                    "true", "false", "null",
                    "tp",
                    "tp-int", "tp-float", "tp-char", "tp-bool",
                    "tp-str", "tp-null", "tp-list", "tp-dict",
                    "tp-series", "tp-pair", "tp-time", "tp-qot",
                    "SR", "_SE"
            };
            return names;
        }

        void initialize(const std::string &executed_file_path, const std::string &working_directory) {
            if (has_initial) {
                data_space_pool.reset();
            }
            has_initial = true;
            base::PROGRAM_WORKING_DIRECTORY_STACK.push(
                    utils::getAbsolutePath(working_directory.empty() || working_directory == undefined_ ?
                    utils::getFileDirFromPath(executed_file_path) : working_directory));
            /*
             * 全局数据的初值表。键必须与 getRuntimeGlobalDataNames() 完全一致。
             *
             * SR / _SE 也在表内，与其他全局量走同一条注册路径（不能再单独 addGlobalData，
             * 否则重复注册会抛 DuplicateKeyError）。它们会被运行期改写：
             *   SR  —— RET 写入返回值、IVOK 取出后清空
             *   _SE —— EXPO 写入错误对象、错误处理后清空
             */
            const std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> globalInitialValues{
                    {"SN", std::make_shared<data::Null>()},
                    {"SE", std::make_shared<data::Null>()},
                    {"SS", std::make_shared<data::Null>()},
                    {"true", std::make_shared<data::Bool>(true)},
                    {"false", std::make_shared<data::Bool>(false)},
                    {"null", std::make_shared<data::Null>()},
                    {"tp", std::make_shared<data::DType>(data::DType::typeId)},
                    {"tp-int", std::make_shared<data::DType>(data::Int::typeId)},
                    {"tp-float", std::make_shared<data::DType>(data::Float::typeId)},
                    {"tp-char", std::make_shared<data::DType>(data::Char::typeId)},
                    {"tp-bool", std::make_shared<data::DType>(data::Bool::typeId)},
                    {"tp-str", std::make_shared<data::DType>(data::String::typeId)},
                    {"tp-null", std::make_shared<data::DType>(data::Null::typeId)},
                    {"tp-list", std::make_shared<data::DType>(data::List::typeId)},
                    {"tp-dict", std::make_shared<data::DType>(data::Dict::typeId)},
                    {"tp-series", std::make_shared<data::DType>(data::Series::typeId)},
                    {"tp-pair", std::make_shared<data::DType>(data::KeyValuePair::typeId)},
                    {"tp-time", std::make_shared<data::DType>(data::Time::typeId)},
                    {"tp-qot", std::make_shared<data::DType>(data::Quote::typeId)},
                    {"SR", std::make_shared<data::Null>()},
                    {"_SE", std::make_shared<data::Null>()}
            };
            for (const auto &name: getRuntimeGlobalDataNames()) {
                const auto it = globalInitialValues.find(name);
                if (it == globalInitialValues.end()) {
                    throw base::RVM_Error(
                            base::ErrorType::RuntimeError, {}, {},
                            {"No initial value is registered for a runtime global.",
                             "Global name: \"" + name + "\""},
                            {"getRuntimeGlobalDataNames() and globalInitialValues must list the "
                             "same names; this is an RVM internal inconsistency."});
                }
                const auto [dataId, _] = data_space_pool.addGlobalData(name, it->second);
                if (name == "SR") {
                    SR_SPACE_ID = dataId;
                } else if (name == "_SE") {
                    SE_SPACE_ID = dataId;
                }
            }
            const auto &main_space_name = utils::getFileNameFromPath(executed_file_path);
            data_space_pool.acquireScope(pre_SRC, main_space_name);
            data_space_pool.main_scope_name = pre_SRC + main_space_name;
        }

        bool isInitialized() {
            return has_initial;
        }

        void addIns(const utils::Pos &pos, const std::string &raw_code, const RI *ri, const StdArgs &args, const std::string &ext) {
            components::INS_SET->addIns(std::make_shared<Ins>(pos, raw_code, ri, args, ext));
        }

        void serializeLinkedExtensions(std::ostream &out) {
            // RSI 1.9.0：计数与字符串都改用变长编码
            utils::writeVarint(out, loadedExtensions.size());
            for (const auto& ext_name : loadedExtensions | std::views::keys) {
                utils::writeString(out, ext_name);
                // FixMe
            }
        }

        void deserializeLinkedExtensions(std::istream &in) {
            const auto extension_count = utils::readVarint(in);
            for (uint64_t i = 0; i < extension_count; ++i) {
                loadedExtensions.emplace(utils::readString(in), nullptr); // FixMe
            }
        }

    }

    namespace parse {
        ParseScopeRecord last_parse_scope_record{};

        void ParseScopeRecord::clear() {
            closedScopes.clear();
            visibleSymbols.clear();
            rootStartIndex = 0;
            rootFrameNextSlot = 0;
            rootSlotNames.clear();
            rootVisibleSymbols.clear();
            lexicalResolutions.clear();
            rootLexicalResolution.clear();
            unresolvedIdentifiers.clear();
            seededGlobalNames.clear();
            closedScopePaths.clear();
            rootScopePath.clear();
            frames.clear();
        }

        namespace {
            /*
             * 解析期分配的「帧路径 → 运行期帧下标」。
             * prepareRuntimeFrames 填好，收尾时用来把下标写进 InsSet 与每个参数。
             */
            std::unordered_map<std::string, uint32_t> runtimeFrameIndexByPath{};

            /*
             * InsSet 的 InstID → InsSet 指针（本次解析期间有效）。
             * 用于收尾时把帧下标写回对应的指令集，运行期据此切换当前帧。
             */
            std::unordered_map<std::string, core::components::InsSet *> insSetById{};

            /* 深度优先遍历指令集树，登记每个 InsSet 的 InstID */
            void indexInsSets(const std::shared_ptr<core::components::InsSet> &insSet) {
                if (!insSet) {
                    return;
                }
                insSetById[insSet->getInstID().toString()] = insSet.get();
                for (const auto &ins: insSet->getInsSet()) {
                    if (ins) {
                        indexInsSets(ins->scopeInsSet);
                    }
                }
            }

            core::components::InsSet *findInsSetByInstId(const std::string &instId) {
                if (const auto it = insSetById.find(instId); it != insSetById.end()) {
                    return it->second;
                }
                return nullptr;
            }
        }

        bool isSlotModeAvailable() {
            /*
             * 槽位表是否已建立且可用。
             * 只走 '.rsi' 反序列化时解析期不运行，帧表不会就绪——
             * 那时参数上没有槽位地址，启用只会让运行期报错，所以必须明确拒绝。
             */
            return core::memory::RVM_RUNTIME_MEMORY.isFrameTableReady();
        }

        /* 最近一次解析得到的槽位表，供 --comp 落盘 */
        RVMSlotTable lastSlotTable{};

        const ParseScopeRecord &getLastParseScopeRecord() {
            return last_parse_scope_record;
        }

        namespace {
            /* 把 printLexicalResolution 的输出抓成字符串 */
            std::string captureLexicalResolution(const core::memory::RVM_ParseScope &scope) {
                std::ostringstream oss;
                scope.printLexicalResolution(oss);
                return oss.str();
            }
        }

        namespace {
            /* 按帧内槽位号排序输出，便于人工核对 */
            std::vector<std::pair<std::string, size_t>>
            sortByFrameSlot(const std::unordered_map<std::string, size_t> &symbols) {
                std::vector<std::pair<std::string, size_t>> ordered(symbols.begin(), symbols.end());
                std::sort(ordered.begin(), ordered.end(),
                          [](const auto &lhs, const auto &rhs) { return lhs.second < rhs.second; });
                return ordered;
            }
        }

        void RVMSlotTable::serialize(std::ostream &os) const {
            // RSI 1.9.0：全部改用变长编码，帧下标/父下标用 +1 编码
            utils::writeVarint(os, frames.size());
            for (const auto &frame: frames) {
                utils::writeVarint(os, frame.slotSpan);
                utils::writeIndexPlusOne(os, frame.parentFrameIndex);
                const uint8_t isRoot = frame.frameRoot ? 1 : 0;
                os.write(reinterpret_cast<const char *>(&isRoot), sizeof(isRoot));
            }

            utils::writeVarint(os, rootFrameIndex);

            utils::writeVarint(os, rootSlotNames.size());
            for (const auto &name: rootSlotNames) {
                utils::writeString(os, name);
            }
        }

        void RVMSlotTable::deserialize(std::istream &is) {
            /*
             * frameCount 来自文件，必须校验上限后再 resize。
             * 未校验时，一个几 KB 的畸形文件声明 10 亿帧就能让进程申请几十 GB，
             * 表现为整机卡死，而不是一条可读的错误。
             */
            const auto frameCount =
                    static_cast<uint32_t>(utils::readCount(is, utils::RSI_MAX_FRAME_COUNT,
                                                          "slot table frame count"));
            if (!is || frameCount == 0) {
                // 只有「读到 0」才是真的没有槽位表；超限已在 readCount 里单独报错，
                // 不能把两者混为一谈（否则会报出「没有槽位表，帧数=1000000000」这种自相矛盾的话）。
                throw base::RVM_Error(
                        base::ErrorType::RuntimeError, {}, {},
                        {"The RSI file carries no static slot table.",
                         "Frame count read: " + std::to_string(frameCount)},
                        {"Recompile the source with this RVM version to produce a compatible RSI."});
            }
            frames.resize(frameCount);
            for (auto &frame: frames) {
                // slotSpan 决定 RVM_RuntimeScope 的槽位数组规模，同样必须设限
                frame.slotSpan = static_cast<uint32_t>(
                        utils::readCount(is, utils::RSI_MAX_SLOT_SPAN, "frame slot span"));
                frame.parentFrameIndex = utils::readIndexPlusOne(is);
                uint8_t isRoot = 0;
                is.read(reinterpret_cast<char *>(&isRoot), sizeof(isRoot));
                frame.frameRoot = (isRoot != 0);
            }
            rootFrameIndex = static_cast<uint32_t>(utils::readVarint(is));
            if (rootFrameIndex >= frameCount) {
                /*
                 * 越界的 root_frame_index 必须在此拒绝。
                 *
                 * 原先这里不校验：setRootFrameIndex(999) 照样生效，frameAt(999) 返回 null，
                 * 于是「把全局数据灌回根帧」整段被静默跳过；直到运行期某个无关指令
                 * 因为取不到 DType 而报错——报错位置与真正原因毫无关系，极难排查。
                 * 合法产物里 rootFrameIndex 必然是有效帧下标，越界只可能是文件损坏。
                 */
                throw base::RVM_Error(
                        base::ErrorType::RuntimeError, {}, {},
                        {"The RSI slot table has an out-of-range root frame index.",
                         "Root frame index: " + std::to_string(rootFrameIndex),
                         "Frame count: " + std::to_string(frameCount)},
                        {"This indicates a corrupted RSI file. Recompile the source."});
            }

            const auto slotNameCount =
                    utils::readCount(is, utils::RSI_MAX_NAME_COUNT, "root slot name count");
            rootSlotNames.resize(static_cast<size_t>(slotNameCount));
            for (auto &name: rootSlotNames) {
                name = utils::readString(is);
            }
            if (!is) {
                throw base::RVM_Error(base::ErrorType::RuntimeError, {}, {},
                                      {"The static slot table in the RSI file is truncated."},
                                      {"Recompile the source with this RVM version."});
            }

            /*
             * 直接建好运行期帧表。
             *
             * 帧下标在编译期就写死在各处（InsSet::frameIndex、Arg::frameIndex），
             * 所以这里必须【按原下标顺序】重建，不能重新编号。
             */
            auto &runtimeMemory = core::memory::RVM_RUNTIME_MEMORY;
            runtimeMemory.clearAllFrames();
            for (uint32_t i = 0; i < frameCount; ++i) {
                auto scope = std::make_shared<core::memory::RVM_RuntimeScope>(
                        std::max<size_t>(frames[i].slotSpan, 1), nullptr, std::string{});
                if (frames[i].frameRoot) {
                    scope->markAsFrameRoot();
                }
                const uint32_t index = runtimeMemory.addFrame(scope, std::string{});
                if (index != i) {
                    /*
                     * 防御性检查：当前实现下不可达。
                     *
                     * addFrame 每次 push_back 并返回 frames.size()-1，而循环恰好在
                     * clearAllFrames() 之后顺序执行 frameCount 次，所以 index 必然等于 i。
                     * 保留是为了在未来改动 addFrame 语义（例如复用槽位、并发建帧）时立刻暴露问题。
                     */
                    throw base::RVM_Error(base::ErrorType::RuntimeError, {}, {},
                                          {"Slot table frame indices are inconsistent."},
                                          {"This indicates a corrupted RSI file."});
                }
            }
            // 接词法父链：所有帧实例都已就位，取地址安全
            for (uint32_t i = 0; i < frameCount; ++i) {
                const uint32_t parentIndex = frames[i].parentFrameIndex;
                if (parentIndex == core::memory::INVALID_INDEX) {
                    continue;   // 无父帧（词法链终点），是正常值
                }
                if (parentIndex >= frameCount) {
                    /*
                     * 越界父下标必须拒绝，不能静默跳过。
                     *
                     * 原先 `if (frame && parent)` 在越界时直接不做绑定，帧的词法链就断了，
                     * 反序列化却一声不吭——运行期才会以取不到变量的形式暴露，
                     * 报错位置与真正原因完全无关。合法产物里父下标要么是有效帧下标、
                     * 要么是 INVALID_INDEX，越界只可能是文件损坏。
                     */
                    throw base::RVM_Error(
                            base::ErrorType::RuntimeError, {}, {},
                            {"The RSI slot table has an out-of-range parent frame index.",
                             "Frame " + std::to_string(i) + " parent index: " +
                             std::to_string(parentIndex),
                             "Frame count: " + std::to_string(frameCount)},
                            {"This indicates a corrupted RSI file. Recompile the source."});
                }
                const auto frame = runtimeMemory.frameAt(i);
                const auto parent = runtimeMemory.frameAt(parentIndex);
                if (frame && parent) {
                    frame->setParentScope(parent.get());
                }
            }

            /*
             * 把全局数据灌回根帧槽位。
             *
             * 编译期这些名字（tp-* / true / false / null / SN / SE / SS …）已经分好了槽位，
             * 但它们的值只存在于 RVM_Memory 的全局作用域里，必须复制进根帧。
             */
            runtimeMemory.setRootFrameIndex(rootFrameIndex);
            const auto rootFrame = runtimeMemory.frameAt(rootFrameIndex);
            if (rootFrame) {
                for (size_t slotIndex = 0; slotIndex < rootSlotNames.size(); ++slotIndex) {
                    const auto [dataID, globalData] =
                            data_space_pool.findDataByNameNoLock(rootSlotNames[slotIndex]);
                    if (globalData) {
                        rootFrame->ensureSlotCount(slotIndex + 1);
                        const auto &_ = rootFrame->setSlot(slotIndex, globalData->copy_ptr());
                    }
                    runtimeMemory.setRootSlotByName(rootSlotNames[slotIndex],
                                                    static_cast<uint32_t>(slotIndex));
                }
            }
            runtimeMemory.markFrameTableReady();
        }

        const RVMSlotTable &getLastSlotTable() {
            return lastSlotTable;
        }

        void prepareRuntimeFrames(const ParseScopeRecord &record) {
            auto &runtimeMemory = core::memory::RVM_RUNTIME_MEMORY;
            runtimeMemory.clearAllFrames();

            /*
             * 第一趟：建帧。
             *
             * 帧的身份是【下标】——addFrame 返回的下标就是它的地址，
             * 随后写进该帧内所有参数的 (frameIndex, slotIndex)。
             * 必须先把所有帧建完再接词法父链，否则取不到父帧地址。
             */
            std::unordered_map<std::string, uint32_t> frameIndexByPath;
            for (const auto &[framePathKey, frameInfo]: record.frames) {
                if (framePathKey.empty()) {
                    continue;
                }
                const size_t slotCount = std::max<size_t>(frameInfo.slotSpan, 1);
                auto scope = std::make_shared<core::memory::RVM_RuntimeScope>(
                        slotCount, nullptr, framePathKey);
                // 只有真正没有词法父帧的帧才是帧根（链遍历的终点）。
                // 函数帧定义在文件根作用域里，它的词法父就是文件根帧，因此不能标成帧根。
                if (frameInfo.parentFramePath.empty()) {
                    scope->markAsFrameRoot();
                }
                frameIndexByPath[framePathKey] = runtimeMemory.addFrame(scope, framePathKey);
            }

            // 第二趟：接词法父链。所有帧实例都已存在，取地址安全。
            for (const auto &[framePathKey, frameInfo]: record.frames) {
                if (framePathKey.empty() || frameInfo.parentFramePath.empty()) {
                    continue;
                }
                const auto frame = runtimeMemory.frameAt(frameIndexByPath[framePathKey]);
                const auto parentIt = frameIndexByPath.find(frameInfo.parentFramePath);
                if (!frame || parentIt == frameIndexByPath.end()) {
                    continue;
                }
                if (const auto parentFrame = runtimeMemory.frameAt(parentIt->second)) {
                    frame->setParentScope(parentFrame.get());
                }
            }

            /*
             * 第三趟：把运行期全局数据灌进文件根帧的槽位。
             *
             * 解析期已把这些名字登记进根作用域并分好槽位（true / false / null / tp / tp-* /
             * SN / SE / SS …），但它们的「值」目前只存在于旧的 RVM_Memory::globalScope_ 里。
             * 新系统按槽位取值，所以必须把它们复制进根帧，否则诸如 TP_SET: tp-int, x
             * 这类指令会读到空槽位。
             */
            const auto rootIt = frameIndexByPath.find(record.rootScopePath);
            if (const auto rootFrame = rootIt != frameIndexByPath.end()
                                               ? runtimeMemory.frameAt(rootIt->second)
                                               : nullptr) {
                const auto &rootSlotNames = record.rootSlotNames;
                for (size_t slotIndex = 0; slotIndex < rootSlotNames.size(); ++slotIndex) {
                    const auto [dataID, globalData] =
                            data_space_pool.findDataByNameNoLock(rootSlotNames[slotIndex]);
                    if (globalData) {
                        rootFrame->ensureSlotCount(slotIndex + 1);
                        const auto &_ = rootFrame->setSlot(slotIndex, globalData->copy_ptr());
                    }
                    /*
                     * 登记根帧的槽位名。
                     * SE / _SE 这类 RVM 内部全局槽位在 RA 源码里没有对应参数，
                     * 拿不到 Arg，只能按「根帧下标 + 槽位名」定位。
                     */
                    runtimeMemory.setRootSlotByName(rootSlotNames[slotIndex],
                                                    static_cast<uint32_t>(slotIndex));
                }
                runtimeMemory.setRootFrameIndex(rootIt->second);
            }
            runtimeFrameIndexByPath = std::move(frameIndexByPath);

            /*
             * 落盘用的槽位表。
             *
             * 必须与 addFrame 的调用顺序完全一致——帧下标是各参数里写死的身份，
             * 重建时不能重新编号。上面遍历 record.frames（std::map，按路径有序）
             * 依次调用 addFrame，这里就按同一顺序填表。
             */
            lastSlotTable = RVMSlotTable{};
            lastSlotTable.frames.reserve(runtimeFrameIndexByPath.size());
            for (const auto &[framePathKey, frameInfo]: record.frames) {
                if (framePathKey.empty()) {
                    continue;
                }
                const auto indexIt = runtimeFrameIndexByPath.find(framePathKey);
                if (indexIt == runtimeFrameIndexByPath.end()) {
                    continue;
                }
                RVMSlotTable::FrameEntry entry{};
                entry.slotSpan = static_cast<uint32_t>(
                        std::max<size_t>(frameInfo.slotSpan, 1));
                entry.frameRoot = frameInfo.parentFramePath.empty();
                if (!frameInfo.parentFramePath.empty()) {
                    if (const auto parentIt = runtimeFrameIndexByPath.find(frameInfo.parentFramePath);
                        parentIt != runtimeFrameIndexByPath.end()) {
                        entry.parentFrameIndex = parentIt->second;
                    }
                }
                // 以帧下标为准定位，避免顺序假设出问题
                if (indexIt->second >= lastSlotTable.frames.size()) {
                    lastSlotTable.frames.resize(indexIt->second + 1);
                }
                lastSlotTable.frames[indexIt->second] = entry;
            }
            lastSlotTable.rootSlotNames = record.rootSlotNames;
            if (const auto rootIt2 = runtimeFrameIndexByPath.find(record.rootScopePath);
                rootIt2 != runtimeFrameIndexByPath.end()) {
                lastSlotTable.rootFrameIndex = rootIt2->second;
            }
        }

        void printParseScopeRecord(const ParseScopeRecord &record, std::ostream &os,
                                   const std::string &source_name) {
            os << "\n=== Parse-scope slot registration";
            if (!source_name.empty()) {
                os << " for " << source_name;
            }
            os << " ===\n";

            for (size_t scopeOrdinal = 0; scopeOrdinal < record.closedScopes.size(); ++scopeOrdinal) {
                const auto &[scopeStartIndex, scopeSlotNames] = record.closedScopes[scopeOrdinal];
                os << "  scope #" << scopeOrdinal
                   << " {startIndex: " << scopeStartIndex
                   << ", slotCount: " << scopeSlotNames.size()
                   << ", slotRange: [" << scopeStartIndex << ", "
                   << (scopeStartIndex + scopeSlotNames.size()) << ")}\n";
                for (size_t i = 0; i < scopeSlotNames.size(); ++i) {
                    os << "      local[" << i << "] / frame[" << (scopeStartIndex + i)
                       << "] <- \"" << scopeSlotNames[i] << "\"\n";
                }
            }

            os << "  root scope {startIndex: " << record.rootStartIndex
               << ", slotCount: " << record.rootSlotNames.size()
               << ", frameNextSlot: " << record.rootFrameNextSlot << "}\n";
            for (size_t i = 0; i < record.rootSlotNames.size(); ++i) {
                os << "      local[" << i << "] / frame[" << (record.rootStartIndex + i)
                   << "] <- \"" << record.rootSlotNames[i] << "\"\n";
            }

            os << "\n--- Visible symbol table (per frame, innermost scope wins) ---\n";
            for (size_t scopeOrdinal = 0; scopeOrdinal < record.visibleSymbols.size(); ++scopeOrdinal) {
                const auto &[scopeStartIndex, visibleSymbols] = record.visibleSymbols[scopeOrdinal];
                os << "  from scope #" << scopeOrdinal
                   << " (startIndex " << scopeStartIndex << "): "
                   << visibleSymbols.size() << " visible name(s)\n";
                for (const auto &[symbolName, frameSlot]: sortByFrameSlot(visibleSymbols)) {
                    os << "      \"" << symbolName << "\" -> frame[" << frameSlot << "]\n";
                }
            }
            os << "  from root scope: " << record.rootVisibleSymbols.size() << " visible name(s)\n";
            for (const auto &[symbolName, frameSlot]: sortByFrameSlot(record.rootVisibleSymbols)) {
                os << "      \"" << symbolName << "\" -> frame[" << frameSlot << "]\n";
            }

            os << "\n--- Lexical (static) resolution ---\n";
            os << "Names resolve along the parse-time parent chain = the DEFINING scope,\n";
            os << "so a function body sees its definition site, not its call site.\n";
            for (size_t scopeOrdinal = 0; scopeOrdinal < record.lexicalResolutions.size(); ++scopeOrdinal) {
                os << "  [scope #" << scopeOrdinal << "]\n";
                os << record.lexicalResolutions[scopeOrdinal];
            }
            os << "  [root scope]\n" << record.rootLexicalResolution;

            os << "\n--- Names that could not be resolved to a slot ---\n";
            if (record.seededGlobalNames.empty()) {
                os << "  (globals were NOT seeded into the root scope: environment was not initialized\n";
                os << "   before parsing, so global identifiers cannot be resolved yet)\n";
            } else {
                os << "  globals seeded into root scope: " << record.seededGlobalNames.size() << " name(s)\n";
            }
            if (record.unresolvedIdentifiers.empty()) {
                os << "  (none — every identifier resolved to a slot)\n";
            } else {
                os << "  " << record.unresolvedIdentifiers.size() << " unresolved occurrence(s):\n";
                std::unordered_set<std::string> reported{};
                for (const auto &[name, rawCode]: record.unresolvedIdentifiers) {
                    if (!reported.insert(name).second) {
                        continue; // 同一个名字只报第一次
                    }
                    os << "      \"" << name << "\"  (first at: " << rawCode << ")\n";
                }
            }

            /*
             * 槽位地址已直接挂在每个参数上（utils::Arg::frameIndex / slotIndex），
             * 运行期不再需要任何绑定表，所以这里只报告帧清单。
             */
            os << "\n--- Frames (slot addresses are mounted on each argument) ---\n";
            os << "  root scope path: " << record.rootScopePath << "\n";
            os << "  total frames: " << record.frames.size() << "\n";

            os << "\n--- Runtime frame table (built from parse-time info) ---\n";
            for (const auto &[framePath, frameInfo]: record.frames) {
                os << "  " << framePath
                   << " {slotSpan: " << frameInfo.slotSpan
                   << ", lexicalParentFrame: "
                   << (frameInfo.parentFramePath.empty() ? std::string("<none>")
                                                         : frameInfo.parentFramePath)
                   << "}\n";
            }
            os << "=== End of parse-scope slot registration ===\n";
        }

        std::tuple<std::vector<std::string>, std::vector<size_t>, std::vector<size_t>>
        preprocessCode(std::string &code) {
            utils::addCharBeforeNewline(code);
            const std::vector<std::string> lines = utils::StringManager::split(code, '\n');
            std::vector<std::string> processedLines;
            std::string currentLine;
            std::vector<size_t> lineRawIndex;
            std::vector<size_t> lineIndents;
            size_t rawIndex = 0;
            for (const std::string &line: lines) {
                std::string trimmedLine = line;
                utils::StringManager::trim(trimmedLine);
                if (trimmedLine.empty()) {
                    if (!currentLine.empty() && currentLine[0] != ';') {
                        currentLine = utils::StringManager::combineNearbyString(currentLine, static_cast<int>(rawIndex));
                        processedLines.push_back(currentLine);
                        lineIndents.push_back(utils::getSpaceFrontOfLineCode(lines[rawIndex - 1]));
                        lineRawIndex.push_back(rawIndex);
                        currentLine.clear();
                    }
                    rawIndex++;
                    continue;
                }
                // 处理拼接符 '~' 先于注释符 ';'
                if (trimmedLine[0] == '~') {
                    if (!currentLine.empty()) {
                        currentLine += trimmedLine.substr(1);
                    }
                } else {
                    if (!currentLine.empty() && currentLine[0] != ';') {
                        currentLine = utils::StringManager::combineNearbyString(currentLine, static_cast<int>(rawIndex));
                        processedLines.push_back(currentLine);
                        lineIndents.push_back(utils::getSpaceFrontOfLineCode(lines[rawIndex - 1]));
                        lineRawIndex.push_back(rawIndex);
                    }
                    currentLine = trimmedLine;
                }
                if (!currentLine.empty() && currentLine[0] != ';') {
                    size_t semicolonPos = std::string::npos;
                    bool inQuotes = false;
                    for (size_t i = 0; i < currentLine.length(); ++i) {
                        if (currentLine[i] == '\\') {
                            i ++;
                            continue;
                        }
                        if (currentLine[i] == '"') {
                            inQuotes = !inQuotes;
                        } else if (currentLine[i] == ';' && !inQuotes) {
                            semicolonPos = i;
                            break;
                        }
                    }
                    if (semicolonPos != std::string::npos) {
                        currentLine = currentLine.substr(0, semicolonPos);
                    }
                }
                rawIndex++;
            }
            if (!currentLine.empty() && currentLine[0] != ';') {
                currentLine = utils::StringManager::combineNearbyString(currentLine, static_cast<int>(rawIndex));
                processedLines.push_back(currentLine);
                lineIndents.push_back(utils::getSpaceFrontOfLineCode(lines[rawIndex - 1]));
                lineRawIndex.push_back(rawIndex);
            }
            return {processedLines, lineRawIndex, lineIndents};
        }

        namespace ParseTypeUtils {
            std::string toString(const ParseType type) {
                switch (type) {
                    case ParseType::File:
                        return "File";
                    case ParseType::Rasm:
                        return "Rasm";
                    case ParseType::Extension:
                        return "Extension";
                    default:
                        return undefined_;
                }
            }
        }

        std::shared_ptr<InsSet>
        parseCode(const ParseType &type, std::string code_path, std::string &code, bool output_info,
                  const std::optional<utils::Pos> &fixed_pos, const std::string &ext) {
            code_path = utils::getAbsolutePath(code_path);
            const auto &code_name = id::DataID(utils::getFileFromPath(code_path)).toFullString();
            std::stack<std::shared_ptr<InsSet>> insSetStack{};
            insSetStack.push(ins_set_pool.acquire(pre_SRC));
            // 解析期名称→槽位注册：与 insSetStack 同进同出的一条作用域链（仅解析期使用，不影响运行）
            // 作用域对象堆分配，保证父作用域指针在解析过程中始终有效
            std::vector<std::unique_ptr<memory::RVM_ParseScope>> parseScopes{};
            parseScopes.reserve(16);
            parseScopes.push_back(std::make_unique<memory::RVM_ParseScope>(0, nullptr));
            // 文件根作用域本身就是帧根：它是词法链的终点，也是全局数据（tp-* / true / null …）的宿主
            parseScopes.front()->markAsFrameRoot();
            parseScopes.front()->setInsSetId(insSetStack.top()->getInstID().toString());
            // 文件根指令集是帧根：它的指令集对象不是任何 Ins 的内嵌 scopeInsSet
            insSetStack.top()->frameRootScope = true;
            // 已关闭作用域的索引顺序（按关闭顺序），用于解析结束后统一生成词法解析结果
            std::vector<size_t> closedScopeOrder{};
            // 每个作用域在所属帧内的「下一个空闲槽位」水位线。
            // 子作用域关闭后要把父作用域的该值推进到子作用域区间之后，
            // 否则父子/兄弟作用域会拿到同一批槽号（槽位重叠，解析期模型就是错的）。
            std::vector<size_t> parseScopeNextSlot{0};
            std::vector<size_t> parseScopeStack{0};
            // 解析期槽位绑定的原始素材：每条指令的 identifier 参数及其所在作用域。
            // 解析结束后统一换算成 (帧路径, 帧内槽位号)——必须等所有作用域的声明都登记完，
            // 否则前向引用的名字会取到过期结果。
            /*
             * 解析期待回填的参数。
             *
             * 解析时要等到全部作用域的声明都登记完，前向引用的名字才能换算出正确的槽位，
             * 所以先记录「哪个作用域的哪条指令的第几个参数」，收尾时统一把地址挂上去。
             */
            struct ScopedArgRecord {
                size_t scopeIndex{0};
                size_t argOrdinal{0};
                std::string name;
                /* 直接指向参数本体——收尾时把解析出的 (帧下标, 槽位号) 写回它 */
                utils::Arg *arg{nullptr};
            };
            std::vector<ScopedArgRecord> scopedArgRecords{};
            // 解析期槽位注册结果的留存（挂到 last_parse_scope_record，解析中途抛异常也不会丢）
            last_parse_scope_record.clear();
            // 运行期全局数据必须先并入根作用域：槽位替换要求每个标识符都能翻成槽位，
            // 而 true / false / null / tp-* / SN / SE / SS 这些是全局可见、不需要 ALLOT 的。
            // 只在最外层解析调用时并入一次（此时环境已初始化，全局名字集合才是确定的）。
            if (env::isInitialized()) {
                parseScopes.front()->setOpeningInsName("root");
                for (const auto &globalName: env::getRuntimeGlobalDataNames()) {
                    parseScopes.front()->registerSlot(globalName);
                }
                parseScopeNextSlot.front() = parseScopes.front()->getStartIndex()
                                             + parseScopes.front()->getSlotCount();
                last_parse_scope_record.seededGlobalNames = env::getRuntimeGlobalDataNames();
            }
            insSetStack.top()->setScopeLeader(
                    utils::getObjectFormatString(ParseTypeUtils::toString(type), code_name));
            insSetStack.top()->setScopeLeaderPos(
                    fixed_pos ? fixed_pos->toString() :
                    utils::Pos(1, 0, code_path).toString(), code_path);
            auto [lines, rawIndex, indents] = preprocessCode(code);

            /*
             * 声明预扫描：只提前登记「文件顶层直接声明」的名字。
             *
             * 槽位替换在解析期完成，因此函数体里用到的自由名字必须在该函数被解析时已经可见。
             * 但 RCC 产物里函数体的位置可以早于它所引用的顶层名字（test_1.ra 里
             * List_empty_v500 / List_list_v502 在 992/1355 行就被调用，定义却在 1549/1570 行），
             * 所以顶层声明必须先扫一遍登记进根作用域。
             *
             * 只处理顶层，不处理函数体内部：
             *   - 函数体内的声明在主循环里本来就按正确顺序、落在正确的作用域中，提前登记反而会
             *     把嵌套 FUNI（自定义类型的方法）的名字错误地提到根作用域；
             *   - 函数体内引用「同一函数里后声明的变量」是真正的未定义引用，应当被报出来。
             *
             * 顶层声明的登记顺序与主循环一致（按出现先后），所以槽位号不变，主循环里的同名
             * 登记是幂等的。
             */
            {
                size_t nestingDepth = 0; // 只统计函数体/块；顶层为 0
                for (const std::string &line: lines) {
                    const size_t colonPos = line.find(':');
                    if (colonPos == std::string::npos) {
                        continue; // 缺冒号的语法错误留给主循环报
                    }
                    const std::string instruction = line.substr(0, colonPos);
                    if (!utils::isValidIdentifier(instruction)) {
                        continue;
                    }
                    if (instruction == "END") {
                        if (nestingDepth > 0) {
                            --nestingDepth;
                        }
                        continue;
                    }
                    const bool opensLevel = instruction == "FUNC" || instruction == "FUNI"
                                            || instruction == "REPEAT" || instruction == "UNTIL"
                                            || instruction == "ITER_TRAV" || instruction == "ITER_REV_TRAV"
                                            || instruction == "ATMP" || instruction == "DETECT";
                    const bool isTopLevel = (nestingDepth == 0);
                    if (opensLevel) {
                        ++nestingDepth;
                    }
                    if (!isTopLevel) {
                        continue; // 函数体/块内的声明交给主循环
                    }
                    const bool declaresNames = instruction == "ALLOT" || instruction == "FUNC"
                                               || instruction == "FUNI" || instruction == "TP_DEF";
                    if (!declaresNames) {
                        continue;
                    }
                    const std::vector<std::string> rawParameters =
                            utils::StringManager::split(line.substr(colonPos + 1), ',');
                    size_t paramOrdinal = 0;
                    for (std::string param: rawParameters) {
                        utils::StringManager::trim(param);
                        // FUNC / FUNI / TP_DEF 只登记首参（函数名 / 类型名）；ALLOT 登记全部
                        if (instruction != "ALLOT" && paramOrdinal > 0) {
                            break;
                        }
                        ++paramOrdinal;
                        if (param.empty() || utils::getArgType(param) != utils::ArgType::identifier) {
                            continue;
                        }
                        parseScopes.front()->registerSlot(param);
                        parseScopeNextSlot.front() = parseScopes.front()->getStartIndex()
                                                     + parseScopes.front()->getSlotCount();
                    }
                }
            }

            int lineNumber = 1;
            for (const std::string &line: lines) {
                size_t colonPos = line.find(':');
                if (colonPos == std::string::npos) {
                    throw base::RVM_Error(base::ErrorType::SyntaxError,
                                          code_path + ":" + std::to_string(lineNumber) + ":1", line,
                                          {"This error is caused by missing colon in the line."},
                                          {"Please add the missing colon at the appropriate place in this line of code."});
                }
                std::string instruction = line.substr(0, colonPos);
                std::string parametersStr = line.substr(colonPos + 1);
                if (!utils::isValidIdentifier(instruction)) {
                    std::cerr << "Error: Invalid instruction in line: " << lineNumber << " - " << line << std::endl;
                    lineNumber++;
                    continue;
                }
                std::vector<std::string> rawParameters = utils::StringManager::split(parametersStr, ',');
                std::vector<utils::Arg> args{};
                int paramStartColumn = static_cast<int>(indents[lineNumber - 1]) + static_cast<int>(colonPos) + 2;
                static auto getPrefixSpaceLength = [](const std::string &content) -> size_t {
                    size_t count = 0;
                    for (const char c : content) {
                        if (utils::StringManager::isSpace(c)) {
                            ++count;
                        } else {
                            break;
                        }
                    }
                    return count;
                };
                for (std::string &param: rawParameters) {
                    const int &orig_param_length = static_cast<int>(param.length());
                    utils::Pos argPos;
                    if (!fixed_pos) {
                        argPos = utils::Pos(static_cast<int>(rawIndex[lineNumber - 1]), paramStartColumn + static_cast<int>(getPrefixSpaceLength(param)), code_path);
                    } else {
                        argPos = fixed_pos.value();
                    }
                    utils::StringManager::trim(param);
                    if (!param.empty()) {
                        utils::Arg arg(argPos, param);
                        if (!utils::isValidParameter(param))
                        {
                            throw base::RVM_Error(base::ErrorType::SyntaxError, argPos.toString(), line,
                                                  {
                                                      "This error is caused by providing an argument of the wrong type.",
                                                      "Error Arg: " + arg.toString()
                                                  },
                                                  {
                                                      "Argument types include only 'identifier', 'string', 'number', 'keyword'."
                                                  });
                        }
                        args.push_back(arg);
                    }
                    paramStartColumn += orig_param_length + 1;
                }
                utils::Pos insPos;
                if (!fixed_pos){
                    insPos = utils::Pos(static_cast<int>(rawIndex[lineNumber - 1]), 1, code_path);
                } else {
                    insPos = fixed_pos.value();
                }
                const auto ri_opt = RI::getRIByStr(instruction);
                if (bool is_success = ri_opt; !is_success) {
                    throw base::RVM_Error(base::ErrorType::SyntaxError, insPos.toString(), line,
                                          {"This error is caused by using an undefined instruction.",
                                           "Unknown RI: [RI: " + instruction + "]"},
                                          {"Please refer to the official instruction documentation for help."});
                }
                const auto &ri = ri_opt;
                // 记录每个参数在指令里的序号：运行期槽位绑定按 (作用域, 指令序号, 参数序号) 索引
                for (size_t argOrdinal = 0; argOrdinal < args.size(); ++argOrdinal) {
                    args[argOrdinal].setOrdinal(argOrdinal);
                }
                auto ins_ptr = std::make_shared<Ins>(
                        insPos, line, ri, args, ext);
                auto &topInsSet = insSetStack.top();
                /*
                 * 本指令在「它将落入的那个作用域」里的序号，以及它所属的作用域。
                 *
                 * 必须在下面的 if/else 链之前取：hasScope 分支会把 topInsSet / parseScopeStack
                 * 切成子作用域，而这条指令【自身】是要落进父作用域的——
                 * 在链之后取会把它的参数错误地归属到它自己开的作用域。
                 */
                const size_t insOrdinalHere = topInsSet->getInsSet().size();
                const size_t ownerScopeIndexHere = parseScopeStack.back();
                /*
                 * 指令自身的参数绑定必须【就地】记录（压栈之前）。
                 *
                 * 适用两类：
                 *   1. FUNC / FUNI / TP_DEF —— 它们的首参是声明出来的新名字；
                 *   2. 所有 hasScope 指令（REPEAT/UNTIL/ITER_TRAV/ATMP/DETECT）——
                 *      它们自身的参数（如 UNTIL 的条件）是在【父】作用域里求值的。
                 * 通用记录块位于 if/else 链之后，那时作用域已经切换，会把它们归错作用域。
                 */
                if (env::isInitialized() && ri->hasScope) {
                    for (size_t argOrdinal = 0; argOrdinal < args.size(); ++argOrdinal) {
                        const utils::Arg &param = args[argOrdinal];
                        const auto &name = param.getValue();
                        if (RI::getRIByStr(name) != nullptr || utils::looksLikeLabelName(name)) {
                            continue;
                        }
                        if (parseScopes[ownerScopeIndexHere]->resolveLexically(name) == nullptr) {
                            continue; // 操作数关键字等，不参与数据解析
                        }
                        scopedArgRecords.push_back(
                                {ownerScopeIndexHere, argOrdinal, name,
                                 &ins_ptr->args[argOrdinal]});
                    }
                }
                // 帧边界：FUNC / FUNI 会开启一个新的帧，其内部槽位空间从头开始，与外部互不干扰
                const bool isFrameBoundary = (instruction == "FUNC" || instruction == "FUNI");
                // FUNC / FUNI / TP_DEF 的首参在运行期会被 addData 登记成数据实体，解析期同样登记下来
                const bool declaresNameHere =
                        instruction == "FUNC" || instruction == "FUNI" || instruction == "TP_DEF";
                if (declaresNameHere && !args.empty() &&
                    args[0].getType() == utils::ArgType::identifier) {
                    const size_t declaringScopeIndex = parseScopeStack.back();
                    parseScopes[declaringScopeIndex]->registerSlot(args[0].getValue());
                    parseScopeNextSlot[declaringScopeIndex] =
                            parseScopes[declaringScopeIndex]->getStartIndex()
                            + parseScopes[declaringScopeIndex]->getSlotCount();
                    /*
                     * 声明类指令的槽位绑定必须【就地】记录。
                     *
                     * 不能在下面的通用记录块里做：hasScope 分支会把 topInsSet 切成子作用域，
                     * 而 FUNC / FUNI / TP_DEF 本身要落进【父】作用域——
                     * 在链之后记录会把它们的 owner 错认成自己的函数体作用域。
                     */
                    if (env::isInitialized()) {
                        scopedArgRecords.push_back(
                                {declaringScopeIndex, 0, args[0].getValue(), &ins_ptr->args[0]});
                    }
                }
                if (ri->hasScope) {
                    topInsSet->addIns(ins_ptr);
                    insSetStack.push(ins_ptr->scopeInsSet);
                    insSetStack.top()->scope_prefix = ri->name + "-";
                    insSetStack.top()->is_delayed_release_scope = ri->isDelayedReleaseScope;
                    insSetStack.top()->setScopeLeader(ins_ptr->raw_code);
                    insSetStack.top()->setScopeLeaderPos(ins_ptr->pos);
                    // 同步进入新的解析期作用域：起始偏移取父作用域当前的槽位水位线
                    const size_t parentScopeIndex = parseScopeStack.back();
                    memory::RVM_ParseScope *parentParseScope = parseScopes[parentScopeIndex].get();
                    // 帧根（FUNC / FUNI）开启独立帧，槽位空间从 0 重新开始，
                    // 与外部帧的槽位完全解耦（调用时是各自独立的执行上下文）
                    const size_t newScopeStartIndex = isFrameBoundary
                                                              ? 0
                                                              : parseScopeNextSlot[parentScopeIndex];
                    parseScopes.push_back(std::make_unique<memory::RVM_ParseScope>(
                            newScopeStartIndex, parentParseScope));
                    parseScopeNextSlot.push_back(newScopeStartIndex);
                    /*
                     * 记录开启本作用域的指令名。
                     *
                     * 帧根（FUNC / FUNI）必须带上函数名：否则同一个文件里所有函数都会得到
                     * 相同的路径（root/FUNC），运行期帧表里只剩一个帧，多个函数互相覆盖槽位。
                     * 同一函数的多次调用仍然共用同一个帧——这正是我们要的。
                     */
                    if (isFrameBoundary && !args.empty()) {
                        parseScopes.back()->setOpeningInsName(instruction + "@" + args[0].getValue());
                    } else {
                        parseScopes.back()->setOpeningInsName(instruction);
                    }
                    // 桥接：本作用域 ↔ 它对应的 InsSet
                    parseScopes.back()->setInsSetId(insSetStack.top()->getInstID().toString());
                    // FUNC / FUNI 的体是帧根；REPEAT/UNTIL/ITER_TRAV 等块作用域与外层同属一个帧
                    if (isFrameBoundary) {
                        insSetStack.top()->frameRootScope = true;
                    }
                    if (isFrameBoundary) {
                        parseScopes.back()->markAsFrameRoot();
                        // 函数形参是该帧的第一批名字。运行期由 executeFunctionCall 在调用时
                        // 通过 addDataByName(function->args[i]) 绑定，因此解析期必须按声明顺序
                        // 把它们排在帧槽位的最前面（局部[0..k-1]），函数体里的 ALLOT 会命中同名而幂等。
                        for (size_t paramIndex = 1; paramIndex < args.size(); ++paramIndex) {
                            if (args[paramIndex].getType() != utils::ArgType::identifier) {
                                continue;
                            }
                            parseScopes.back()->registerSlot(args[paramIndex].getValue());
                        }
                        parseScopeNextSlot.back() = parseScopes.back()->getStartIndex()
                                                    + parseScopes.back()->getSlotCount();
                    }
                    parseScopeStack.push_back(parseScopes.size() - 1);
                } else if (instruction == "END") {
                    if (args.size() > 1) {
                        std::cerr << "Error: END instruction with more than one parameter in line: "
                                  << std::to_string(lineNumber) << "\n";
                    }
                    if (!args.empty() && args[0].getType() != utils::ArgType::identifier &&
                        args[0].getType() != utils::ArgType::keyword) {
                        std::cerr << "Error: END instruction with non-identifier parameter in line: "
                                  << std::to_string(lineNumber) << "\n";
                    }
                    const auto &end_label = args.empty() ? insSetStack.top()->scope_prefix + "END"
                                                         : args[0].getValue();
                    if (insSetStack.size() > 1) {
                        topInsSet->end_pointer = static_cast<int>(topInsSet->getInsSet().size());
                        topInsSet->setLabel(topInsSet->scope_prefix + end_label + suf_END, topInsSet->end_pointer);
                        topInsSet->addIns(ins_ptr);
                        insSetStack.pop();
                        if (parseScopeStack.size() > 1) {
                            const size_t closedScopeIndex = parseScopeStack.back();
                            const auto &closedScope = *parseScopes[closedScopeIndex];
                            // 总是记录分配结果（供后续按名字查槽位使用），与诊断开关无关
                            last_parse_scope_record.closedScopes.emplace_back(
                                    closedScope.getStartIndex(), closedScope.getSlotNames());
                            std::unordered_map<std::string, size_t> visibleSymbols{};
                            closedScope.collectFrameVisibleSlots(visibleSymbols);
                            last_parse_scope_record.visibleSymbols.emplace_back(
                                    closedScope.getStartIndex(), std::move(visibleSymbols));
                            // 词法解析结果刻意不在这里生成：此刻外层作用域还没解析完，
                            // 名字的槽位号会取到过期值。统一在解析结束后计算（见下方 closedScopeOrder）。
                            closedScopeOrder.push_back(closedScopeIndex);
                            parseScopeStack.pop_back();
                            // 只有非帧根的子作用域才把父作用域的水位线推进到自己的区间之后；
                            // 帧根（FUNC / FUNI）内部的槽位空间与外部完全独立，不参与父作用域的槽位分配
                            if (!closedScope.isFrameRoot()) {
                                const size_t parentScopeIndex = parseScopeStack.back();
                                const size_t closedScopeEnd = parseScopeNextSlot[closedScopeIndex];
                                if (closedScopeEnd > parseScopeNextSlot[parentScopeIndex]) {
                                    parseScopeNextSlot[parentScopeIndex] = closedScopeEnd;
                                }
                            }
                        }
                    } else {
                        std::cerr << "Error: END instruction without matching scope in line: " << lineNumber
                                  << std::endl;
                    }
                    ins_ptr->is_delayed_release_scope = topInsSet->is_delayed_release_scope;
                } else if (instruction == "SET") {
                    topInsSet->setLabel(args[0].getValue(), static_cast<int>(topInsSet->getInsSet().size()));
                    topInsSet->addIns(ins_ptr);
                } else if (instruction == "LINK") {
                    try {
                        auto linked_file_path = tools::getArgOriginData(args[0])->getValStr();
                        const auto &[link_name, link_ext] = utils::getFileInfoFromPath(linked_file_path);
                        const auto &linked_ins_set = getLinkedInsSet(*ins_ptr.get(), true, linked_file_path);
                        if (env::precompiled_link_option && link_ext == RA_EXT) {
                            std::filesystem::path dirPath(env::precompiled_link_dir_path);
                            std::filesystem::path filePath(link_name + RSI_EXT_);
                            std::filesystem::path fullPath = dirPath / filePath;
                            serializeExecutableInsToBinaryFile(fullPath.string(),
                                                               getLinkedInsSet(*ins_ptr.get(),
                                                                               false),
                                                               env::program_serialization_profile_);
                        }
                        if (linked_ins_set) {
                            topInsSet->insertInsSet(linked_ins_set);
                        } else {
                            throw base::RVM_Error(base::ErrorType::LinkError, insPos.toString(), line,
                                                  {"This error is caused by an error encountered while statically linking other files.",
                                                   "Error Linking File: " +
                                                   utils::getObjectFormatString("File", ins_ptr->args[0].getValue())},
                                                  {"The " + ins_ptr->ri->toString() +
                                                   " requires a argument to specify the path of the file to be linked. ",
                                                   "Only 'RA' and 'RSI' files are supported."});
                        }
                    } catch (const base::errors::ArgumentError &_) {
                        throw base::errors::ArgumentError(ins_ptr->args[0].getPos().toString(), ins_ptr->raw_code,
                                                          "Error Arg: " +
                                                          utils::getEntityFormatString(ins_ptr->args[0].getValue(),
                                                                                      tools::getArgOriginData(
                                                                                              ins_ptr->args[0])->toString()),
                                                          {"Please check that the argument of this instruction are used correctly.",
                                                           "The " + ins_ptr->ri->toString() +
                                                           " requires an argument to specify the path of the file to be linked. ",
                                                           "Only 'RA' and 'RSI' files are supported."});
                    }
                } else if (instruction == "ALLOT") {
                    // 解析期只登记名称 → 槽位，不触碰运行时内存；运行行为仍由 ri_allot 决定
                    const size_t scopeIndex = parseScopeStack.back();
                    for (const utils::Arg &allotArg: args) {
                        if (allotArg.getType() != utils::ArgType::identifier) {
                            continue;
                        }
                        parseScopes[scopeIndex]->registerSlot(allotArg.getValue());
                        // 水位线 = 起始偏移 + 已分配槽位数，单调不减
                        parseScopeNextSlot[scopeIndex] =
                                parseScopes[scopeIndex]->getStartIndex()
                                + parseScopes[scopeIndex]->getSlotCount();
                    }
                    topInsSet->addIns(ins_ptr);
                } else {
                    topInsSet->addIns(ins_ptr);
                }
                if (output_info) {
                    std::cout << "Instruction: " << instruction << " at " << insPos << std::endl;
                    std::cout << "Parameters: ";
                    for (const utils::Arg &param: args) {
                        std::cout << "[" << param.getValue() << " at " << param.getPos().toString() << "] ";
                    }
                    std::cout << std::endl;
                }
                // 槽位替换的前置条件：每个会按名字解析的标识符都必须能翻成 (帧, 槽位)。
                // 这里把解析不到的名字记录下来——运行期切换到槽位寻址之前，这张表必须为空，
                // 否则那些名字会在运行期静默落到错误的槽位上。
                if (env::isInitialized()) {
                    for (size_t argOrdinal = 0; argOrdinal < args.size(); ++argOrdinal) {
                        const utils::Arg &param = args[argOrdinal];
                        /*
                         * 哪些参数需要槽位？
                         *
                         * 运行期按名字解析数据的是 identifier 与 keyword 两类
                         * （见 getArgOriginDataNoError）。但 keyword 里绝大多数是操作数关键字
                         * （s-l / RE / RG / AND / fl-r …），它们不查数据表。
                         *
                         * 判据：只有能沿词法链解析到槽位的参数才记录绑定。
                         * 典型例子是 tp-int / tp-list 这类 DType 全局名——它们
                         * 被 getArgType 判成 identifier（不在关键字集合里），却是真正的数据引用，
                         * 运行期会去读它们。若不给它们绑定槽位，运行期就查不到。
                         */
                        const auto &name = param.getValue();
                        // 指令名本身是全局可见的 RI 名，不需要槽位
                        if (RI::getRIByStr(name) != nullptr) {
                            continue;
                        }
                        // 标签（JMP/JF/JT/EXIT 的操作数）不是数据实体，由 SET: 登记进 InsSet::set_labels
                        if (utils::looksLikeLabelName(name)) {
                            continue;
                        }
                        /*
                         * 只跳过【声明出来的那个名字】（arg[0]）。
                         *
                         * 不能整条指令跳过：TP_DEF 可以带第二个参数（父类型），
                         * 它是一个真实的变量引用，必须照常记录绑定。
                         */
                        if (declaresNameHere && argOrdinal == 0) {
                            continue;
                        }
                        // hasScope 指令自身的参数已就地记录
                        if (ri->hasScope) {
                            continue;
                        }
                        if (parseScopes[parseScopeStack.back()]->resolveLexically(name) == nullptr) {
                            if (param.getType() == utils::ArgType::keyword) {
                                continue; // 操作数关键字，不参与数据解析
                            }
                            last_parse_scope_record.unresolvedIdentifiers.emplace_back(
                                    name, ins_ptr->raw_code);
                            continue;
                        }
                        scopedArgRecords.push_back(
                                {parseScopeStack.back(), argOrdinal, name,
                                 &ins_ptr->args[argOrdinal]});
                    }
                }
                lineNumber++;
            }
            if (insSetStack.size() != 1) {
                throw std::runtime_error("Unclosed scope.");
            }
            // 根作用域的分配结果与可见符号（解析正常结束后一定可用）
            last_parse_scope_record.rootStartIndex = parseScopes.front()->getStartIndex();
            last_parse_scope_record.rootFrameNextSlot = parseScopeNextSlot.front();
            last_parse_scope_record.rootSlotNames = parseScopes.front()->getSlotNames();
            parseScopes.front()->collectFrameVisibleSlots(last_parse_scope_record.rootVisibleSymbols);
            // 词法解析结果必须在解析全部结束后统一生成：
            // 解析中途外层作用域的槽位表尚未定型，此时算出的槽位号是过期的。
            last_parse_scope_record.lexicalResolutions.clear();
            last_parse_scope_record.lexicalResolutions.reserve(closedScopeOrder.size());
            for (const size_t closedScopeIndex: closedScopeOrder) {
                last_parse_scope_record.lexicalResolutions.push_back(
                        captureLexicalResolution(*parseScopes[closedScopeIndex]));
            }
            last_parse_scope_record.rootLexicalResolution =
                    captureLexicalResolution(*parseScopes.front());

            /*
             * 作用域路径留存（仅诊断输出用）。
             *
             * 注意：这里【不再】换算任何绑定表。名字到地址的换算在下面直接
             * 写进每个参数的 (frameIndex, slotIndex)，运行期不做任何查找。
             */
            last_parse_scope_record.rootScopePath = parseScopes.front()->getScopePath();
            last_parse_scope_record.closedScopePaths.clear();
            last_parse_scope_record.closedScopePaths.reserve(closedScopeOrder.size());
            for (const size_t closedScopeIndex: closedScopeOrder) {
                last_parse_scope_record.closedScopePaths.push_back(
                        parseScopes[closedScopeIndex]->getScopePath());
            }

            /*
             * 帧清单：每个帧的槽位规模、以及它的词法父帧。
             *
             * 必须遍历【所有帧根作用域】，不能只从槽位引用反推——反推会漏掉
             * 「函数体里没有任何变量引用」的帧（例如一个只做 SOUT 的函数）。
             */
            last_parse_scope_record.frames.clear();
            for (const auto &scopePtr: parseScopes) {
                if (scopePtr == nullptr || !scopePtr->isFrameRoot()) {
                    continue;
                }
                const std::string framePathKey = scopePtr->getScopePath();
                auto &frameInfo = last_parse_scope_record.frames[framePathKey];
                frameInfo.framePath = framePathKey;
                frameInfo.slotSpan = std::max(frameInfo.slotSpan, scopePtr->getFrameSlotSpan());
            }
            for (auto &[framePathKey, frameInfo]: last_parse_scope_record.frames) {
                const memory::RVM_ParseScope *frameRootScope =
                        parseScopes.front()->findScopeByPath(framePathKey);
                if (frameRootScope == nullptr) {
                    continue;
                }
                // 参数与局部变量可能没有被任何引用用到，也要计入规模
                frameInfo.slotSpan = std::max(frameInfo.slotSpan, frameRootScope->getFrameSlotSpan());
                if (const memory::RVM_ParseScope *parentScope = frameRootScope->getParentScope();
                    parentScope != nullptr) {
                    frameInfo.parentFramePath = parentScope->getFrameRootScope()->getScopePath();
                }
            }

            /*
             * 建立运行期帧表，并把每个帧根作用域的 InsSet 认到下标记。
             *
             * 帧的身份是【下标】：运行期取值只需 frames[idx]->slots[n]，不做任何查找。
             * 下标在此一次性分配，'execute' 时通过 InsSet::frameIndex 直接使用。
             */
            prepareRuntimeFrames(last_parse_scope_record);
            // 建立 InsSet 索引：收尾时按 InstID 把帧下标写回对应的指令集
            insSetById.clear();
            indexInsSets(insSetStack.top());
            // 帧根作用域 → 它对应的 InsSet 也要记住帧下标，运行期据此切换当前帧
            for (const auto &scopePtr: parseScopes) {
                if (scopePtr == nullptr || !scopePtr->isFrameRoot()) {
                    continue;
                }
                const auto it = runtimeFrameIndexByPath.find(scopePtr->getScopePath());
                if (it != runtimeFrameIndexByPath.end()) {
                    scopePtr->setFrameIndex(it->second);
                }
            }
            for (const size_t scopeIndex: closedScopeOrder) {
                const auto &scope = *parseScopes[scopeIndex];
                if (!scope.isFrameRoot()) {
                    continue;
                }
                const auto insSet = findInsSetByInstId(scope.getInsSetId());
                if (insSet) {
                    insSet->frameIndex = scope.getFrameIndex();
                    insSet->frameRootScope = true;
                }
            }
            if (const auto rootInsSet = findInsSetByInstId(parseScopes.front()->getInsSetId())) {
                rootInsSet->frameIndex = parseScopes.front()->getFrameIndex();
                rootInsSet->frameRootScope = true;
            }
            core::memory::RVM_RUNTIME_MEMORY.markFrameTableReady();

            /*
             * 把解析出的 (帧下标, 槽位号) 直接挂到每个参数上。
             *
             * 这是静态槽位架构的核心：名字在解析期就消失了，运行期只顺着参数读地址。
             */
            for (const auto &record: scopedArgRecords) {
                if (record.arg == nullptr) {
                    continue;
                }
                const memory::RVM_ParseScope *ownerScope = parseScopes[record.scopeIndex].get();
                const memory::RVM_ParseScope *declaringScope = ownerScope->resolveLexically(record.name);
                if (declaringScope == nullptr) {
                    continue;
                }
                const memory::RVM_ParseScope *frameRoot = declaringScope->getFrameRootScope();
                const auto localSlot = declaringScope->findSlotItself(record.name);
                if (!localSlot.has_value() || frameRoot->getFrameIndex() == memory::INVALID_INDEX) {
                    continue;
                }
                record.arg->setSlotRef(frameRoot->getFrameIndex(),
                                       static_cast<uint32_t>(
                                               declaringScope->getFrameSlotIndex(*localSlot)));
            }
            /*
             * 这里【不】开启槽位模式。
             *
             * 解析期只负责把地址挂到参数上；是否真的走新架构由调用方决定
             * （CLI 的 --slot-mode / --legacy-mode）。解析器自己置位会让开关失去意义。
             */

            // 解析期指令级输出（逐条指令与其参数），由 output_info 控制
            // 槽位注册结果的打印交给调用方（printParseScopeRecord），避免与 main 的输出来重复

            return insSetStack.top();
        }

        std::shared_ptr<InsSet> parseCodeFromPath(const std::string &code_file_path, bool output_info,
                                                  const std::string &extension_name) {
            auto code = utils::readFile(code_file_path);
            return parseCode(ParseType::File, code_file_path, code, output_info,
                             {}, extension_name);
        }

        void serializeExecutableInsToBinaryFile(std::string binary_file_path,
                                                const std::shared_ptr<InsSet> &ins_set,
                                                const utils::SerializationProfile &profile){
            binary_file_path = utils::getAbsolutePath(binary_file_path);
            std::ofstream binary_file(binary_file_path, std::ios::binary);

            /*
             * 路径表只有在整个指令流写完后才完整，所以先把指令流写进内存缓冲区，
             * 再按「头 → 扩展表 → 槽位表 → 路径表 → 指令流」的顺序落盘。
             *
             * 路径表是【局部变量】而不是全局/线程局部状态：--precomp-link 会在解析主文件的
             * 循环里对本函数递归调用（见 LINK 分支），全局表会被内层调用清掉，
             * 写进主文件的路径表就会被链接文件的内容污染。
             */
            utils::PathTable paths;
            std::ostringstream body;
            ins_set->serialize(body, profile, paths);

            base::rvm_serial_header.setProfile(profile);
            base::rvm_serial_header.serialize(binary_file);
            env::serializeLinkedExtensions(binary_file);
            /*
             * 静态槽位表必须落在指令流之前：反序列化时要先按它把运行期帧表建好，
             * 各处参数里写死的帧下标才有对应的帧实例。
             */
            getLastSlotTable().serialize(binary_file);
            // 路径表同样必须在指令流之前：反序列化 Pos 时就要能按下标取回路径
            paths.serialize(binary_file);

            const std::string bodyStr = body.str();
            if (!bodyStr.empty()) {
                binary_file.write(bodyStr.data(), static_cast<std::streamsize>(bodyStr.size()));
            }
            if (!binary_file) {
                throw base::RVM_Error(base::ErrorType::RuntimeError, binary_file_path, {},
                                      {"Failed to write the RSI file."},
                                      {"Check that the target directory exists and is writable."});
            }
            binary_file.close();
        }

        std::shared_ptr<InsSet> deserializeExecutableInsFromBinaryFile(std::string binary_file_path, const utils::SerializationProfile &profile){
            binary_file_path = utils::getAbsolutePath(binary_file_path);
            std::ifstream binary_file(binary_file_path, std::ios::binary);
            base::rvm_serial_header.deserialize(binary_file, profile, binary_file_path);
            env::deserializeLinkedExtensions(binary_file);
            // 先重建运行期帧表，指令里写死的帧下标才有落点
            lastSlotTable = RVMSlotTable{};
            lastSlotTable.deserialize(binary_file);
            // 路径表：局部实例，与写入端顺序一致
            utils::PathTable paths;
            paths.deserialize(binary_file);
            auto ins_set = std::make_shared<InsSet>("File");
            ins_set->deserialize(binary_file, base::rvm_serial_header.getProfile(), paths);
            binary_file.close();
            return ins_set;
        }

        std::shared_ptr<InsSet> getLinkedInsSet(const Ins &link_ins, const bool check_exist, const std::string &path) {
            if (!ris::LINK.equalWith(link_ins.ri)){
                throw std::runtime_error("The instruction is not a link instruction.");
            }
            const auto &link_path = utils::getAbsolutePath(
                    path.empty() ? tools::getArgOriginData(link_ins.args[0])->getValStr() : path,
                    base::PROGRAM_WORKING_DIRECTORY_STACK.top());
            const auto &link_file_type = utils::getFileExtFromPath(link_path);
            if (check_exist){
                const auto & [isExist, extPath] = tools::checkExtensionExist(link_path);
                if (isExist) {
                    return nullptr;
                }
                loadedExtensions.emplace(extPath, nullptr);
            }
            if (link_file_type == "ra") {
                return parseCodeFromPath(link_path, false);
            }
            if (link_file_type == "rsi") {
                return deserializeExecutableInsFromBinaryFile(link_path);
            }
            throw base::errors::ArgumentError(unknown_, unknown_, unknown_, {});
        }
    }
}
