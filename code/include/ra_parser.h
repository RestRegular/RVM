//
// Created by RestRegular on 2025/1/16.
//

#ifndef RA_PARSER_H
#define RA_PARSER_H

#include <algorithm>
#include <iostream>
#include <map>
#include "ra_base.h"
#include "ra_core.h"
#include "rvm_ris.h"
#include "lib/ra_utils.h"

namespace parser {
    using namespace core::components;

    struct InsSetPool;

    extern core::memory::RVM_Memory &data_space_pool;
    extern base::RVM_IO* io;
    extern InsSetPool &ins_set_pool;
    extern utils::StringManager &string_manager;
    using StdInsSet = std::shared_ptr<InsSet>;

    struct InsSetPool {
    public:
        static InsSetPool& getInstance();

        InsSetPool (const InsSetPool&) = delete;
        InsSetPool& operator= (const InsSetPool&) = delete;

        StdInsSet acquire(const std::string &prefix);

        StdInsSet getAppointedInsSet(const std::string& insSetId);

        void release(const std::string& insSetId);

        void release(const StdInsSet& insSet);

        void releaseAll();

        std::shared_ptr<Ins> findIns(const std::string& insSetId, const std::string& insId) const;

        void printInfo(std::ostream& out = std::cout) const;

    private:
        InsSetPool() = default;

        std::unordered_map<std::string, StdInsSet> insSetSpace_;
        std::list<StdInsSet> insSetOrder_;
        std::vector<StdInsSet> insSetPool_;
    };

    namespace components {
        extern StdInsSet INS_SET;
    }

    namespace env {
        using namespace core::components;

        extern utils::SerializationProfile program_serialization_profile_;

        extern bool precompiled_link_option;

        extern std::string precompiled_link_dir_path;

        void initialize(const std::string &executed_file_path, const std::string &working_directory);

        /* 运行环境是否已经初始化过（决定解析期是否需要把全局名字并入根作用域） */
        [[nodiscard]] bool isInitialized();

        /*
         * 运行期全局数据名的单一事实来源。
         * 解析期用它把同名变量登记进文件根作用域，使每个标识符都能被解析成槽位。
         */
        [[nodiscard]] const std::vector<std::string> &getRuntimeGlobalDataNames();

        void addIns(const utils::Pos &pos, const std::string& raw_code, const RI *ri, const StdArgs &args);

        void serializeLinkedExtensions(std::ostream &out);

        void deserializeLinkedExtensions(std::istream &in);
    }

    namespace parse {
        using namespace core::components;

        /*
         * 解析期槽位注册的结果快照。
         *
         * parseCode 内部的 RVM_ParseScope 链是函数局部对象，解析中途抛异常就会全部丢失。
         * 这个结构把「已成功关闭的作用域」的分配结果留存下来，供诊断与后续按名字查槽位使用，
         * 不参与任何运行期逻辑。
         */
        struct ParseScopeRecord {
            /* 槽位分配：帧内起始偏移 + 该作用域登记的名字（下标即局部槽位号） */
            std::vector<std::pair<size_t, std::vector<std::string>>> closedScopes;

            /* 从每个已关闭作用域出发、本帧内可见的符号表（名字 → 帧内槽位号） */
            std::vector<std::pair<size_t, std::unordered_map<std::string, size_t>>> visibleSymbols;

            /* 最外层作用域（文件根帧）的分配结果 */
            size_t rootStartIndex{0};
            size_t rootFrameNextSlot{0};
            std::vector<std::string> rootSlotNames;
            std::unordered_map<std::string, size_t> rootVisibleSymbols;

            /*
             * 每个已关闭作用域的词法解析结果文本。
             * 词法（静态）作用域下，名字沿解析期的 parentScope 链（= 定义处的外层作用域）解析，
             * 与调用处的动态作用域无关。
             */
            std::vector<std::string> lexicalResolutions;
            std::string rootLexicalResolution;

            /*
             * 解析期无法解析到槽位的标识符（名字 → 首次出现的指令原始文本）。
             *
             * 槽位替换要求每个标识符都能翻成 (帧, 槽位)；任何解析不到的名字都必须在这里暴露出来，
             * 否则运行期会静默按错槽位取值。切换运行期之前这个表必须为空。
             */
            std::vector<std::pair<std::string, std::string>> unresolvedIdentifiers;

            /* 根作用域里那些来自运行期全局数据的名字（便于核对全局是否已并入） */
            std::vector<std::string> seededGlobalNames;

            /*
             * 帧清单：帧身份在解析期就已完全确定，运行期一次性把帧建好即可。
             * parentFramePath 是「词法父帧」——函数体看定义处的落点。
             */
            struct FrameInfo {
                std::string framePath;
                size_t slotSpan{0};             // 帧内槽位规模（最大槽位号 + 1）
                std::string parentFramePath;    // 词法父帧；文件根帧为空
            };
            std::map<std::string, FrameInfo> frames;

            /* 每个已关闭作用域的链路径，下标与 closedScopes 一致；根作用域另见 rootScopePath */
            std::vector<std::string> closedScopePaths;
            std::string rootScopePath;

            void clear();
        };

        /* 最近一次 parseCode 的解析期槽位注册结果 */
        extern ParseScopeRecord last_parse_scope_record;

        /*
         * 把解析期槽位注册结果打印成人类可读形式。
         * 只读，不修改 record；供 --parse-info 开关与诊断使用。
         */
        void printParseScopeRecord(const ParseScopeRecord &record, std::ostream &os = std::cout,
                                   const std::string &source_name = "");

        /*
         * 用解析期确定的帧表建立运行期帧实例（RVM_RuntimeMemory 的帧注册表）。
         *
         * 帧身份在解析期就已完全确定，所以运行期一次性建好，函数调用时不需要新建帧——
         * 只按参数绑定写入既有帧的槽位。
         *
         * 帧的词法父链在此建立：函数帧的父 = 定义处外层作用域所属的帧。
         * 这正是「函数体看定义处、不看调用处」的落点。
         */
        void prepareRuntimeFrames(const ParseScopeRecord &record);

        /*
         * RSI 里的静态槽位表。
         *
         * 解析期把 (帧下标, 槽位号) 挂到了每个参数上，但那是内存里的字段，不随指令流落盘。
         * 没有这张表，反序列化出来的参数就没有地址，槽位寻址无从谈起。
         *
         * 落盘内容分两部分，与内存里的存放位置一一对应：
         *   - 帧表：规模 + 词法父下标（对应 RVM_RuntimeMemory 的 frames）
         *   - 根帧的槽位名（对应 rootSlotByName，用于把全局数据 tp-* / true / null … 灌回根帧）
         * 每个 InsSet 自己的 frameIndex 与每个参数的 (frameIndex, slotIndex) 写在 InsSet / Arg 里。
         */
        struct RVMSlotTable {
            struct FrameEntry {
                uint32_t slotSpan{0};
                /* 词法父帧的下标；没有父帧时为 INVALID_INDEX */
                uint32_t parentFrameIndex{core::memory::INVALID_INDEX};
                bool frameRoot{false};
            };
            std::vector<FrameEntry> frames;
            uint32_t rootFrameIndex{core::memory::INVALID_INDEX};
            std::vector<std::string> rootSlotNames;

            [[nodiscard]] bool empty() const { return frames.empty(); }

            void serialize(std::ostream &os) const;
            /* 读回并直接建立运行期帧表；表为空则抛错（不静默降级） */
            void deserialize(std::istream &is);
        };

        /* 把最近一次解析得到的槽位表交给序列化方 */
        [[nodiscard]] const RVMSlotTable &getLastSlotTable();

        /*
         * 运行期槽位绑定的全局开关。
         *
         * 切换之后，变量存取一律走「解析期绑定表 → 帧 + 槽位」，不再有任何按名字查找；
         * 绑定缺失时直接报错，不回退到旧路径——静默回退会把模型错误藏起来。
         */
        void setSlotModeEnabled(bool enabled);
        [[nodiscard]] bool isSlotModeEnabled();
        /*
         * 槽位表是否已建立且可用（'.rsi' 反序列化路径不会建立它们）。
         * 供调用方在决定是否启用静态槽位架构前判断。
         */
        [[nodiscard]] bool isSlotModeAvailable();

        /* 取得最近一次解析的完整记录（运行期读帧表用） */
        [[nodiscard]] const ParseScopeRecord &getLastParseScopeRecord();

        enum class ParseType {
            File,
            Rasm,
            Extension,
        };

        std::tuple<std::vector<std::string>, std::vector<size_t>, std::vector<size_t>> preprocessCode(std::string &code);

        std::shared_ptr<InsSet>
        parseCode(const ParseType &type, std::string code_path, std::string &code, bool output_info,
                  const std::optional<utils::Pos> &fixed_pos, const std::string &ext = "");

        std::shared_ptr<InsSet> parseCodeFromPath(const std::string &code_file_path, bool output_info,
                                                  const std::string &extension_name = "");

        void serializeExecutableInsToBinaryFile(std::string binary_file_path, const std::shared_ptr<InsSet> &ins_set, const utils::SerializationProfile &profile);

        std::shared_ptr<InsSet> deserializeExecutableInsFromBinaryFile(std::string binary_file_path,
                                                                       const utils::SerializationProfile &profile = utils::SerializationProfile::Debug);

        std::shared_ptr<InsSet> getLinkedInsSet(const Ins &link_ins, bool check_exist = true, const std::string &path = "");
    }

}

#endif //RA_PARSER_H
