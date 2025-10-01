//
// Created by RestRegular on 2025/3/19.
//

#include <cstdlib>
#include <filesystem>
#include "../include/rvm_ris.h"
#include "../include/ra_parser.h"
#include "../include/lib/newrcc.h"

core::memory::RVM_Memory &data_space_pool = core::memory::RVM_Memory::getInstance();
base::RVM_IO *io{base::RVM_IO::getInstance()};

namespace core::components {
    std::unordered_map<std::string, RI> insMap{};
    id::DataID SR_SpaceID{};
    id::DataID _SE_SpaceID{};

    RI::RI(std::string name, const int arity,
        const ExecutorFunc executor,
        const bool isCreatedScope,
        const bool isDelayedReleaseScope)
            : name(std::move(name)),
              arity(arity), executor(executor), hasScope(isCreatedScope),
              isDelayedReleaseScope(isDelayedReleaseScope) {
        memory::RVM_Memory::getInstance().addGlobalData(this->name,
                                                        std::make_shared<data::String>(toString()));
        base::categorizedKeywords[base::getKeywordTypeName(base::KeywordType::Instructions)].insert(this->name);
        auto upper_name = this->name;
        std::ranges::transform(upper_name, upper_name.begin(),
                               [](const unsigned char c) { return std::toupper(c); });
        insMap[upper_name] = *this;
    }

    bool RI::operator==(const RI &other) const {
        return this->id == other.id;
    }

    std::string RI::toString() const {
        return "[RI: " + this->name + "]";
    }

    // 序列化函数：只序列化 ri_index
    void RI::serialize(std::ostream &os, const utils::SerializationProfile &profile) const {
        const size_t nameLength = name.size();
        os.write(reinterpret_cast<const char *>(&nameLength), sizeof(nameLength));
        if (nameLength > 0) {
            os.write(name.c_str(), nameLength);
        }
    }

    // 反序列化函数：通过 ri_index 从 ri_list 获取对象
    RI RI::deserialize(std::istream &is, const utils::SerializationProfile &profile)
    {
        std::string name {};
        size_t nameLength;
        is.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
        if (nameLength > 0) {
            name.resize(nameLength);
            is.read(&name[0], nameLength);
        } else {
            name.clear();
        }
        if (const auto &it = insMap.find(name);
            it != insMap.end()) {
            return it->second;
        }
        throw std::runtime_error("Invalid RI: " + name);
    }

    std::optional<RI> RI::getRIByStr(const std::string &content) {
        // 将输入内容转换为大写以实现大小写不敏感匹配
        std::string upperContent = content;
        std::ranges::transform(upperContent, upperContent.begin(),
                               [](const unsigned char c) { return std::toupper(c); });
        // 查找指令映射表
        auto it = insMap.find(upperContent);
        if (it != insMap.end()) {
            return {it->second};
        }
        return std::nullopt;
    }

    // Ins具体实现
    Ins::Ins(utils::Pos pos, std::string raw_code, RI ri, StdArgs args,
             std::string ext)
            : pos(pos), ri(std::move(ri)), args(std::move(args)),
              scopeInsSet(std::make_shared<InsSet>(ri.name)), raw_code(std::move(raw_code)),
              ext(std::move(ext)){
        if (this->ri.arity != -1 && static_cast<int>(this->args.size()) != this->ri.arity) {
            throw base::errors::ArgumentNumberError(this->pos.toString(), this->raw_code,
                                                    std::to_string(this->ri.arity), this->args.size(),
                                                    this->ri.name, {});
        }
    }

    ExecutionStatus Ins::execute(size_t &pointer) const {
        return this->ri.executor(*this, pointer, args);
    }

    void Ins::addIns(std::shared_ptr<Ins> ins) const {
        if (this->ri.hasScope) {
            this->scopeInsSet->addIns(std::move(ins));
        } else {
            throw base::RVM_Error(base::ErrorType::RuntimeError, ins->pos.toString(), ins->raw_code,
                                  {"This error occurs when an Ins is added to an Ins without a domain.",
                                   "Error Ins: " + ins->raw_code},
                                  {"Check whether the target instruction contains internal instFields when adding instructions."});
        }
    }

    // 序列化函数
    void Ins::serialize(std::ostream &os, const utils::SerializationProfile &profile) const {
        // 序列化 Pos
        pos.serialize(os, profile);

        // 序列化 RI
        ri.serialize(os, profile);

        // 序列化 StdArgs (std::vector<utils::Arg>)
        const size_t argsSize = args.size();
        os.write(reinterpret_cast<const char *>(&argsSize), sizeof(argsSize));
        for (const auto &arg: args) {
            arg.serialize(os, profile);
        }

        // 序列化 scopeInsSet
        const bool hasScopeInsSet = ri.hasScope;
        os.write(reinterpret_cast<const char *>(&hasScopeInsSet), sizeof(hasScopeInsSet));
        if (hasScopeInsSet) {
            scopeInsSet->serialize(os, profile);
        }

        // 序列化 raw_code
        if (profile < utils::SerializationProfile::Testing) {
            // 高于测试模式下，不序列化 raw_code
            const size_t codeLength = raw_code.size();
            os.write(reinterpret_cast<const char *>(&codeLength), sizeof(codeLength));
            if (codeLength > 0) {
                os.write(raw_code.c_str(), codeLength);
            }
        }

        // 序列化布尔值
        os.write(reinterpret_cast<const char *>(&is_delayed_release_scope), sizeof(is_delayed_release_scope));

        // 序列化 ext_info
        const size_t extLength = ext.size();
        os.write(reinterpret_cast<const char *>(&extLength), sizeof(extLength));
        if (extLength > 0) {
            os.write(ext.c_str(), extLength);
        }
    }

    // 反序列化函数
    void Ins::deserialize(std::istream &is, const utils::SerializationProfile &profile) {
        // 反序列化 Pos
        pos.deserialize(is, profile);

        // 反序列化 RI
        ri = RI::deserialize(is, profile);

        // 反序列化 StdArgs (std::vector<utils::Arg>)
        size_t argsSize;
        is.read(reinterpret_cast<char *>(&argsSize), sizeof(argsSize));
        args.clear();
        for (size_t i = 0; i < argsSize; ++i) {
            utils::Arg arg;
            arg.deserialize(is, profile);
            args.push_back(arg);
        }

        // 反序列化 scopeInsSet
        bool hasScopeInsSet;
        is.read(reinterpret_cast<char *>(&hasScopeInsSet), sizeof(hasScopeInsSet));
        if (hasScopeInsSet) {
            scopeInsSet = std::make_shared<InsSet>(unknown_);
            scopeInsSet->deserialize(is, profile);
        } else {
            scopeInsSet = nullptr;
        }

        // 反序列化 raw_code
        if (profile < utils::SerializationProfile::Testing) {
            size_t codeLength;
            is.read(reinterpret_cast<char *>(&codeLength), sizeof(codeLength));
            if (codeLength > 0) {
                raw_code.resize(codeLength);
                is.read(&raw_code[0], codeLength);
            } else {
                raw_code.clear();
            }
        }

        // 反序列化布尔值
        is.read(reinterpret_cast<char *>(&is_delayed_release_scope), sizeof(is_delayed_release_scope));

        // 反序列化 ext_info
        size_t extLength;
        is.read(reinterpret_cast<char *>(&extLength), sizeof(extLength));
        if (extLength > 0) {
            ext.resize(extLength);
            is.read(&ext[0], extLength);
        } else {
            ext.clear();
        }
    }

    AtmpBlock::AtmpBlock(std::vector<std::shared_ptr<DetectBlock>> detectBlocks, std::shared_ptr<InsSet> block_ins_set,
                         std::shared_ptr<InsSet> finally_block_ins_set)
            : detect_blocks(std::move(detectBlocks)), block_ins_set(std::move(block_ins_set)),
              finally_block_ins_set(std::move(finally_block_ins_set)) {}

    DetectBlock::DetectBlock(std::shared_ptr<data::CustomType> detected_type,
                             utils::Arg error_stored_data, std::shared_ptr<InsSet> block_ins_set)
            : detected_type(std::move(detected_type)), error_stored_arg(std::move(error_stored_data)),
              block_ins_set(std::move(block_ins_set)) {}

    std::stack<InsSet*> InsSet::exe_stack{};

    std::vector<std::shared_ptr<AtmpBlock>> InsSet::atmp_stack{};

    DebugMode InsSet::debug_mode = DebugMode::None;

    std::string InsSet::file_record{};

    void InsSet::insertInsSet(const std::shared_ptr<InsSet>& insSet_) {
        this->insSet.reserve(this->insSet.size() + insSet_->insSet.size());
        this->insSet.insert(this->insSet.end(), insSet_->insSet.begin(), insSet_->insSet.end());
    }

    void InsSet::addIns(std::shared_ptr<Ins> ins) {
        insSet.push_back(std::move(ins));
    }

    int InsSet::getLabel(const std::string &label) const {
        if (const auto &it = set_labels.find(label);
                it != set_labels.end()) {
            return it->second;
        }
        return -1;
    }

    void InsSet::setScopeLeader(const Ins &leaderIns) {
        scope_leader = leaderIns.raw_code;
        scope_leader_pos = leaderIns.pos.toString();
        scope_leader_file = leaderIns.pos.getFilepath();
    }

    void InsSet::setScopeLeaderPos(const utils::Pos &leaderPos) {
        scope_leader_pos = leaderPos.toString();
        scope_leader_file = leaderPos.getFilepath();
    }

    void InsSet::setScopeLeaderPos(const std::string &leaderPosStr, const std::string &file_path) {
        scope_leader_pos = leaderPosStr;
        scope_leader_file = file_path;
    }

    std::string InsSet::makeFileIdentiFromPath(const std::string &pos) {
        const auto &code_name = id::DataID(utils::getFileFromPath(pos)).toFullString();
        return utils::getObjectFormatString("File", code_name);
    }

    const auto &debug_command_help = cc::colorfulText(R"(Debug Command Info:
    /?   -help              :打印此帮助信息
    /p   -print             :打印数据池信息
    /e   -exit              :退出debug环境
    /fs  -fastSkip          :快速跳过指令到断点处
    /ns  -normalSkip        :普通跳过指令到断点处
    /cls -cleanScreen       :清空控制台
    /vn  -viewName          :检查指定名称的数据
    /bp  -breakPoint        :添加断点，参数为断点行号，空格分隔
    /rp  -removePoint       :移除断点，参数为断点行号，空格分隔
    /pcc -printCallChain    :打印函数调用链信息
)", cc::TextColor::GREEN);

    std::unordered_set<int> break_points{};

    void InsSet::printDebugExeStackInfo(const bool& hasError) {
        io->immediateOutput(">> ", cc::colorfulText("Current call chain info:\n", cc::TextColor::LIGHT_GRAY, {cc::Decoration::BOLD}));

        const auto &identSpace = "    ";

        if (exe_stack.empty()) {
            io->immediateOutput(
                identSpace,
                cc::colorfulText(
                    "->Process has finished without any errors.\n",
                    cc::TextColor::LIGHT_GREEN,
                    {cc::Decoration::BOLD}));
        } else {
            // 记录每个 InsSet 实例在栈中的出现次数（即递归深度）
            std::unordered_map<InsSet*, int> instance_counts;
            // 存储执行栈的元素及其在栈中的位置（从栈底到栈顶的索引）
            std::vector<std::pair<InsSet*, int>> stack_elements;
            // 复制执行栈到临时栈
            std::stack<InsSet*> temp_stack = exe_stack;
            int stack_index = 0;
            // 收集执行栈中的元素及其位置信息
            while (!temp_stack.empty()) {
                InsSet* ins_set = temp_stack.top();
                stack_elements.push_back({ins_set, instance_counts[ins_set]});
                // 更新该 InsSet 实例的出现次数
                instance_counts[ins_set]++;
                temp_stack.pop();
                stack_index++;
            }
            // 反向遍历（从栈底到栈顶）打印信息
            size_t last = stack_elements.size() - 1;
            for (auto it = stack_elements.rbegin(); it != stack_elements.rend(); ++it) {
                const InsSet *ins_set = it->first;
                const int target_depth = it->second; // 该元素的递归深度
                // 复制 pointers 栈并弹出到目标深度
                std::stack<std::shared_ptr<size_t> > temp_pointers = ins_set->pointers;
                for (int i = 0; i < target_depth && !temp_pointers.empty(); i++) {
                    temp_pointers.pop();
                }
                // 确保指针栈非空
                if (!temp_pointers.empty()) {
                    const auto &ins = ins_set->insSet[*temp_pointers.top()];
                    const bool is_top_element = (last == 0); // 是否为栈顶元素
                    // 输出格式：栈底元素用"->"标记，其他用"v"标记
                    io->immediateOutput(
                        identSpace,
                        cc::colorfulText("| ", cc::TextColor::CYAN), // 浅蓝色竖线
                        target_depth == 0
                            ? (is_top_element
                                   ? cc::colorfulText(ins->pos.toString(), cc::TextColor(50, 150, 255),
                                                      {cc::Decoration::BOLD})
                                   : cc::colorfulText(ins->pos.toString(), cc::TextColor(100, 200, 255)))
                            : // 明亮的蓝色
                            cc::colorfulText(ins->pos.toString(), cc::TextColor(150, 150, 150)), // 灰色
                        "\n",
                        identSpace,
                        is_top_element
                            ? cc::colorfulText("->", cc::TextColor(0, 255, 100), {cc::Decoration::BOLD})
                            : // 亮绿色箭头
                            cc::colorfulText("v ", cc::TextColor(100, 200, 255)), // 浅蓝色箭头
                        target_depth == 0
                            ? (is_top_element
                                   ? cc::colorfulText(ins->raw_code, cc::TextColor(0, 255, 100), {cc::Decoration::BOLD})
                                   : // 亮蓝色代码
                                   cc::colorfulText(ins->raw_code, cc::TextColor(180, 180, 180)))
                            : // 浅灰色代码
                            cc::colorfulText(ins->raw_code, cc::TextColor(120, 120, 120)), // 深灰色代码
                        "\n");
                }
                last--;
            }
        }
        if (hasError)
        {
            io->immediateOutput(
                ">> ",
                cc::colorfulText(
                    "RVM encountered error.",
                    cc::TextColor(201, 79, 79),
                    {cc::Decoration::BOLD}
                ),
                "\n");
        }
        if (debug_mode == DebugMode::NormalSkip) {
            io->immediateOutput("<< ", cc::colorfulText("[skip]", cc::TextColor(120, 120, 120)), "\n");
        }
    }

    void InsSet::debug_process(const std::shared_ptr<Ins> &_, const bool &hasError) {
        io->flushOutputCache();
        printDebugExeStackInfo(hasError);
        while (debug_mode == DebugMode::Standard) {
            io->immediateOutput("<< ");
            io->readLineAndSplit();
            if (const auto &input = io->hasNextString() ? io->nextString() : "";
                input.empty()) {
                if (debug_mode != DebugMode::Standard) {
                    debug_mode = DebugMode::Standard;
                }
                break;
            } else if (input == "/?" || input == "-help") { // 帮助
                io->immediateOutput(debug_command_help);
            } else if (input == "/p" || input == "-print") { // 打印数据池
                data_space_pool.printPoolInfo();
                io->flushOutputCache();
            } else if (input == "/e" || input == "-exit") { // 退出程序
                data_space_pool.clearAllScopes();
                *io << ">> " << cc::colorfulText("Exit RVM Debug Environment.", cc::TextColor::GREEN, {cc::Decoration::BOLD});
                io->flushOutputCache();
                io->flashInputCache();
                exit(-1);
            } else if (input == "/fs" || input == "-fastSkip") { // 快速跳过
                debug_mode = DebugMode::FastSkip;
                break;
            } else if (input == "/ns" || input == "-normalSkip") {
                debug_mode = DebugMode::NormalSkip;
                break;
            } else if (input == "/cls" || input == "-clearScreen") {
                system("cls");
            } else if (input == "/vn" || input == "-viewName") {
                while (io->hasNextString()) {
                    const auto &view_item = io->nextString();
                    if (const auto &[fst, snd] = data_space_pool.findDataByNameNoLock(view_item);
                        snd != nullptr) {
                        io->immediateOutput(snd->toString(), "\n");
                    } else {
                        io->immediateOutputError("Invalid data name: '", view_item, "'\n");
                    }
                }
            } else if (input == "/bp" || input == "-breakPoint") { // 添加断点
                while (io->hasNextString()) {
                    if (const auto &break_point = utils::stringToInt(io->nextString());
                        break_point > 0 && !break_points.contains(break_point)) {
                        break_points.insert(break_point);
                    } else {
                        io->immediateOutputError("Invalid break point: '", break_point, "'\n");
                    }
                }
            } else if (input == "/rp" || input == "-r") {
                while (io->hasNextString()) {
                    if (const auto &break_point = utils::stringToInt(io->nextString());
                        break_points.contains(break_point)) {
                        break_points.erase(break_point);
                    } else {
                        io->immediateOutputError("Invalid break point: '", break_point, "'\n");
                    }
                }
            } else if (input == "/pcc" || input == "-printCallChain") {
                printDebugExeStackInfo(hasError);
            } else {
                io->immediateOutputError("Invalid debug command: '" + input + "'\n");
            }
        }
    }

    std::string InsSet::getTraceInfo(const std::string &file_record_, const std::string &error_pos_filepath,
                                     const std::string &utils_getPosStrFromFilePath,
                                     const std::string &makeFileIdentiFromPath, const std::string &trace_info,
                                     const std::string &error_pos_str, const std::string &ins_set_raw_code,
                                     const std::string &scope_leader_pos_, const std::string &scope_leader_) {
        std::stringstream ss;

        ss << std::string(7, ' ') << "| " << utils::StringManager::wrapText(scope_leader_pos_, 80, 17, "", "~ ")
           << "\n";
        ss << std::string(7, ' ') << "v " << utils::StringManager::wrapText(scope_leader_, 80, 15, "", "| ~ ")
           << "\n";
        ss << std::string(15, ' ') << "| " << utils::StringManager::wrapText(error_pos_str, 80, 9, "", "~ ")
           << "\n";
        ss << std::string(15, ' ')
           << (trace_info.empty() ? "->" : "v ") << utils::StringManager::wrapText(ins_set_raw_code, 80, 7, "", "| ~ ")
           << "\n";

        if (file_record_ != error_pos_filepath) {
            ss << std::string(7, ' ') << "| "
               << utils::StringManager::wrapText(utils_getPosStrFromFilePath, 80, 17, "", "~ ") << "\n";
            ss << std::string(7, ' ') << "v "
               << utils::StringManager::wrapText(makeFileIdentiFromPath, 80, 15, "", "| ~ ")
               << "\n";
        }

        return ss.str();
    }

    std::shared_ptr<memory::RVM_Scope> InsSet::loadinExtension(const std::string &ext_name) {
        if (!ext_name.empty() && !data_space_pool.checkScopeExistNoLock(ext_name)) {
            std::shared_ptr<data::Extension> ext;
            if (tools::checkExtensionExistStrict(ext_name)) {
                ext = memory::loadedExtensions[ext_name];
            } else {
                const auto &[_, res_ext] = data_space_pool.findDataByNameNoLock(ext_name);
                if (res_ext && res_ext->getTypeID().fullEqualWith(data::Extension::typeId)) {
                    ext = std::static_pointer_cast<data::Extension>(res_ext);
                }
                else {
                    const auto &lastExeStack = exe_stack.top();
                    const auto &lastIns = lastExeStack->insSet[*lastExeStack->pointers.top()];
                    throw base::errors::MemoryError(lastIns->pos.toString(), lastIns->raw_code,
                                                    {"This error is caused by a mismatch between the data type stored "
                                                     "by the memory entity in memory and the required data type.",
                                                     "Error space: " +
                                                     utils::getSpaceFormatString(
                                                             ext_name,
                                                             ext ? ext->toString() : data::nullInstance.toString()),
                                                     "Error type: " + (ext ? ext->getTypeID().toString()
                                                                           : data::nullInstance.getTypeID().toString()),
                                                     "Expected type: " + data::Extension::typeId.toString()},
                                                    {"This error may be caused by an internal system error. Please contact "
                                                     "the RVM developers (https://github.com/RestRegular) for assistance."});
                }
            }
            const auto &ext_scope = ext->extension_scope;
            if (ext_scope->isAddable()) {
                ext_scope->setAddable(false);
            }
            data_space_pool.appendScopeNoLock(ext_scope);
            return ext_scope;
        }
        return nullptr;
    }

    void InsSet::unloadExtension(const std::shared_ptr<memory::RVM_Scope> &ext_data) {
        if (ext_data != nullptr) {
            data_space_pool.removeScopeNoLock(ext_data);
        }
    }

    void InsSet::handleError(const std::shared_ptr<size_t> &pointer, base::RVM_Error &error) const {
        const auto &error_pos = insSet[*pointer]->pos;
        if (file_record.empty()) {
            file_record = error_pos.getFilepath();
        }
        error.addTraceInfo(getTraceInfo(
                file_record,
                error_pos.getFilepath(),
                utils::getPosStrFromFilePath(file_record),
                this->makeFileIdentiFromPath(file_record),
                utils::listJoin(error.trace_info),
                error_pos.toString(),
                insSet[*pointer]->raw_code,
                scope_leader_pos,
                scope_leader
        ));
        if (file_record != error_pos.getFilepath()) {
            file_record = error_pos.getFilepath();
        }
        if (debug_mode != DebugMode::None) {
            debug_mode = DebugMode::Standard;
            debug_process(insSet[*pointer], true);
        }
    }

    ExecutionStatus InsSet::execute() {
        // 更新当前程序的工作目录
        base::PROGRAM_WORKING_DIRECTORY_STACK.push(utils::getFileDirFromPath(this->scope_leader_file));
        scope_name_stack.push(data_space_pool.getCurrentScopeNoLock()->getName());
        const std::string &ext_name = insSet.empty() ? "" : (insSet[0]->ext.empty() ? "" : insSet[0]->ext);
        const auto &ext_scope = loadinExtension(ext_name);
        exe_stack.push(this);
        ExecutionStatus resultStatus{};
        const auto pointer = std::make_shared<size_t>(0);
        pointers.push(pointer);
        try {
            while (*pointer < insSet.size()) {
                const int &pointer_num = *pointer;
                if (debug_mode != DebugMode::None && break_points.contains(insSet[pointer_num]->pos.getLine())){
                    debug_mode = DebugMode::Standard;
                }
                if (debug_mode == DebugMode::Standard || debug_mode == DebugMode::NormalSkip) {
                    debug_process(insSet[pointer_num]);
                }
                resultStatus = insSet[pointer_num]->execute(*pointer);
                if (resultStatus == ExecutionStatus::Aborted || resultStatus == ExecutionStatus::AbortedLoop
                    || resultStatus == ExecutionStatus::AbortedFunction) {
                    break;
                } else if (resultStatus == ExecutionStatus::ExposedError)
                {
                    if (const auto &se = data_space_pool.findDataByIDNoLock(_SE_SpaceID);
                        se && se->getTypeID().fullEqualWith(data::Error::typeId)) {
                        const auto &seData = static_pointer_cast<data::Error>(se);
                        handleError(pointer, *seData->error);
                        }
                    break;
                }
                *pointer += 1;
            }
        } catch (base::RVM_Error &e) {
            const auto &error_pos = insSet[*pointer]->pos;
            if (file_record.empty()) {
                file_record = error_pos.getFilepath();
            }
            handleError(pointer, e);
            throw;
        }
        exe_stack.pop();
        pointers.pop();
        scope_name_stack.pop();
        if (exe_stack.empty()) {
            io->flushOutputCache();
            if (resultStatus == ExecutionStatus::ExposedError) {
                if (const auto se = data_space_pool.findDataByIDNoLock(_SE_SpaceID);
                    se && se->getTypeID().fullEqualWith(data::Error::typeId)) {
                    throw *std::static_pointer_cast<data::Error>(se)->error;
                }
                throw base::errors::ExposedError(unknown_, unknown_, {});
            }
            if (debug_mode != DebugMode::None) {
                debug_mode = DebugMode::Standard;
                debug_process(nullptr);
                io->immediateOutput(
                    ">> ",
                    cc::colorfulText("Process has finished without any errors.",
                        cc::TextColor::GREEN, {cc::Decoration::BOLD}));
            }
        }
        // 恢复程序的上层工作目录
        base::PROGRAM_WORKING_DIRECTORY_STACK.pop();
        unloadExtension(ext_scope);
        return resultStatus;
    }

    void InsSet::serialize(std::ostream &os, const utils::SerializationProfile &profile) const {
        // 序列化布尔值
        os.write(reinterpret_cast<const char *>(&is_delayed_release_scope), sizeof(is_delayed_release_scope));

        // 序列化整数
        os.write(reinterpret_cast<const char *>(&end_pointer), sizeof(end_pointer));

        if (profile < utils::SerializationProfile::Testing) {
            // 序列化字符串
            const size_t prefixLength = scope_prefix.size();
            os.write(reinterpret_cast<const char *>(&prefixLength), sizeof(prefixLength));
            if (prefixLength > 0) {
                os.write(scope_prefix.c_str(), prefixLength);
            }

            const size_t leaderLength = scope_leader.size();
            os.write(reinterpret_cast<const char *>(&leaderLength), sizeof(leaderLength));
            if (leaderLength > 0) {
                os.write(scope_leader.c_str(), leaderLength);
            }
        }

        if (profile < utils::SerializationProfile::Release) {
            const size_t leaderPosLength = scope_leader_pos.size();
            os.write(reinterpret_cast<const char *>(&leaderPosLength), sizeof(leaderPosLength));
            if (leaderPosLength > 0) {
                os.write(scope_leader_pos.c_str(), leaderPosLength);
            }

            const size_t leaderFileLength = scope_leader_file.size();
            os.write(reinterpret_cast<const char *>(&leaderFileLength), sizeof(leaderFileLength));
            if (leaderFileLength > 0) {
                os.write(scope_leader_file.c_str(), leaderFileLength);
            }

//            size_t nameLength = scope_name_stack.size();
//            os.write(reinterpret_cast<const char *>(&nameLength), sizeof(nameLength));
//            if (nameLength > 0) {
//                os.write(scope_name_stack.c_str(), nameLength);
//            }
        }

        // 序列化 vector<std::shared_ptr<Ins>>
        size_t insSetSize = insSet.size();
        os.write(reinterpret_cast<const char *>(&insSetSize), sizeof(insSetSize));
        for (const auto &ins: insSet) {
            ins->serialize(os, profile);
        }

        // 序列化 unordered_map<string, int>
        size_t labelsSize = set_labels.size();
        os.write(reinterpret_cast<const char *>(&labelsSize), sizeof(labelsSize));
        for (const auto &[key, value]: set_labels) {
            size_t keyLength = key.size();
            os.write(reinterpret_cast<const char *>(&keyLength), sizeof(keyLength));
            if (keyLength > 0) {
                os.write(key.c_str(), keyLength);
            }
            os.write(reinterpret_cast<const char *>(&value), sizeof(value));
        }
    }

    void InsSet::deserialize(std::istream &is, const utils::SerializationProfile &profile) {
        is.read(reinterpret_cast<char *>(&is_delayed_release_scope), sizeof(is_delayed_release_scope));

        is.read(reinterpret_cast<char *>(&end_pointer), sizeof(end_pointer));

        if (profile < utils::SerializationProfile::Testing) {
            size_t prefixLength = 0;
            is.read(reinterpret_cast<char *>(&prefixLength), sizeof(prefixLength));
            if (prefixLength > 0) {
                std::vector<char> buffer(prefixLength);
                is.read(buffer.data(), prefixLength);
                scope_prefix.assign(buffer.data(), buffer.size());
            } else {
                scope_prefix.clear();
            }

            size_t leaderLength = 0;
            is.read(reinterpret_cast<char *>(&leaderLength), sizeof(leaderLength));
            if (leaderLength > 0) {
                std::vector<char> buffer(leaderLength);
                is.read(buffer.data(), leaderLength);
                scope_leader.assign(buffer.data(), buffer.size());
            } else {
                scope_leader.clear();
            }
        }

        if (profile < utils::SerializationProfile::Release) {
            size_t leaderPosLength = 0;
            is.read(reinterpret_cast<char *>(&leaderPosLength), sizeof(leaderPosLength));
            if (leaderPosLength > 0) {
                std::vector<char> buffer(leaderPosLength);
                is.read(buffer.data(), leaderPosLength);
                scope_leader_pos.assign(buffer.data(), buffer.size());
            } else {
                scope_leader_pos.clear();
            }

            size_t leaderFileLength = 0;
            is.read(reinterpret_cast<char *>(&leaderFileLength), sizeof(leaderFileLength));
            if (leaderFileLength > 0) {
                std::vector<char> buffer(leaderFileLength);
                is.read(buffer.data(), leaderFileLength);
                scope_leader_file.assign(buffer.data(), buffer.size());
            } else {
                scope_leader_file.clear();
            }

//            size_t nameLength = 0;
//            is.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
//            if (nameLength > 0) {
//                std::vector<char> buffer(nameLength);
//                is.read(buffer.data(), nameLength);
//                scope_name_stack.assign(buffer.data(), buffer.size());
//            } else {
//                scope_name_stack.clear();
//            }
        }

        size_t insSetSize = 0;
        is.read(reinterpret_cast<char *>(&insSetSize), sizeof(insSetSize));
        insSet.clear();
        insSet.reserve(insSetSize);
        for (size_t i = 0; i < insSetSize; ++i) {
            auto ins = std::make_shared<Ins>();
            ins->deserialize(is, profile);
            insSet.push_back(ins);
        }

        size_t labelsSize = 0;
        is.read(reinterpret_cast<char *>(&labelsSize), sizeof(labelsSize));
        set_labels.clear();
        for (size_t i = 0; i < labelsSize; ++i) {
            size_t keyLength = 0;
            is.read(reinterpret_cast<char *>(&keyLength), sizeof(keyLength));
            std::string key;
            if (keyLength > 0) {
                std::vector<char> buffer(keyLength);
                is.read(buffer.data(), keyLength);
                key.assign(buffer.data(), buffer.size());
            }
            int value = 0;
            is.read(reinterpret_cast<char *>(&value), sizeof(value));
            set_labels[key] = value;
        }
    }
}

namespace ris {
    const RI S_M{"s-m", 0, exes::ri_flag};
    const RI S_L{"s-l", 0, exes::ri_flag};
    const RI S_F{"s-f", 0, exes::ri_flag};
    const RI S_N{"s-n", 0, exes::ri_flag};
    const RI S_UNPACK{"s-unpack", 0, exes::ri_flag};
    const RI RG{"RG", 0, exes::ri_flag};
    const RI RGE{"RGE", 0, exes::ri_flag};
    const RI RNE{"RNE", 0, exes::ri_flag};
    const RI RE{"RE", 0, exes::ri_flag};
    const RI RLE{"RLE", 0, exes::ri_flag};
    const RI RL{"RL", 0, exes::ri_flag};
    const RI RT{"RT", 0, exes::ri_flag};
    const RI RF{"RF", 0, exes::ri_flag};
    const RI AND{"AND", 0, exes::ri_flag};
    const RI OR{"OR", 0, exes::ri_flag};

    // 无实际影响的指令集
    const RI PASS{"PASS", -1, exes::ri_pass};
    const RI UNKNOWN{"UNKNOWN", -1, exes::ri_unknown};
    const RI BREAKPOINT{"BREAKPOINT", -1, exes::ri_breakpoint};

    // 可造成实际影响的指令集
    const RI ALLOT{"ALLOT", -1, exes::ri_allot};
    const RI DELETE{"Delete", -1, exes::ri_delete};
    const RI PUT{"PUT", 2, exes::ri_put};
    const RI COPY{"COPY", 2, exes::ri_copy};
    const RI ADD{"ADD", 3, exes::ri_add};
    const RI OPP{"OPP", 2, exes::ri_opp};
    const RI REPEAT{"REPEAT", -1, exes::ri_repeat, true, true}; // repeat 指令：循环执行指令集，重复指定次数，只可接受 1/2 个参数
    const RI END{"END", -1, exes::ri_end};
    const RI FUNC{"FUNC", -1, exes::ri_func, true}; // func 指令：定义函数，可接受多个参数
    const RI FUNI{"FUNI", -1, exes::ri_funi, true}; // funi 指令：定义函数，可接受多个参数
    const RI CALL{"CALL", -1, exes::ri_call}; // call 指令：调用函数，可接受多个参数，用于无返回值的调用
    const RI IVOK{"IVOK", -1, exes::ri_ivok}; // ivok 指令：调用函数，可接受多个参数，用于有返回值的调用
    const RI UNTIL{"UNTIL", 2, exes::ri_until, true, true}; // until 指令：循环执行指令集，直到条件为真
    const RI EXIT{"EXIT", -1, exes::ri_exit}; // exit 指令：退出局部域
    const RI SOUT{"SOUT", -1, exes::ri_sout}; // sout 指令：输出字符串，可接受多个参数
    const RI SIN{"SIN", -1, exes::ri_sin}; // sin 指令：输入字符串，可接受多个参数
    const RI FILE_GET{"FILE_GET", 3, exes::ri_file_get};
    const RI FILE_READ{"FILE_READ", 2, exes::ri_file_read};
    const RI FILE_WRITE{"FILE_WRITE", 2, exes::ri_file_write};
    const RI FILE_GET_PATH{"FILE_GET_PATH", 2, exes::ri_file_get_path};
    const RI FILE_GET_MODE{"FILE_GET_MODE", 2, exes::ri_file_get_mode};
    const RI FILE_GET_SIZE{"FILE_GET_SIZE", 2, exes::ri_file_get_size};
    const RI FILE_SET_MODE{"FILE_SET_MODE", 2, exes::ri_file_set_mode};
    const RI FILE_SET_PATH{"FILE_SET_PATH", 2, exes::ri_file_set_path};
    const RI CMP{"CMP", 3, exes::ri_cmp};
    const RI CREL{"CREL", 3, exes::ri_crel};
    const RI SET{"SET", 1, exes::ri_pass};
    const RI JMP{"JMP", 1, exes::ri_jmp};
    const RI JR{"JR", 3, exes::ri_jr};
    const RI JT{"JT", 2, exes::ri_jt};
    const RI JF{"JF", 2, exes::ri_jf};
    const RI MUL{"MUL", 3, exes::ri_mul};
    const RI ITER_APND{"ITER_APND", -1, exes::ri_iter_apnd};
    const RI ITER_SUB{"ITER_SUB", 4, exes::ri_iter_sub};
    const RI ITER_SIZE{"ITER_SIZE", 2, exes::ri_iter_size};
    const RI ITER_GET{"ITER_GET", 3, exes::ri_iter_get};
    const RI ITER_TRAV{"ITER_TRAV", 3, exes::ri_iter_trav, true,
                       true}; // iter_trav 指令：遍历迭代器，接受三个参数，arg1：迭代容器，arg2：存储迭代元素，arg3：存储迭代索引
    const RI ITER_REV_TRAV{"ITER_REV_TRAV", 3, exes::ri_iter_rev_trav, true,
                       true}; // iter_rev_trav 指令：遍历迭代器，接受三个参数，arg1：迭代容器，arg2：存储迭代元素，arg3：存储迭代索引
    const RI ITER_SET{"ITER_SET", 3,
                      exes::ri_iter_set}; // iter_trav 指令：遍历迭代器，接受三个参数，arg1：迭代容器，arg2：存储迭代元素，arg3：存储迭代索引
    const RI ITER_DEL{"ITER_DEL", -1, exes::ri_iter_del};
    const RI ITER_INSERT{"ITER_INSERT", 3, exes::ri_iter_insert};
    const RI ITER_UNPACK{"ITER_UNPACK", -1, exes::ri_iter_unpack};
    const RI RAND_INT{"RAND_INT", 3, exes::ri_rand_int}; // rand_int 指令：生成随机数，可接受三个参数，arg1：随机数最小值，arg2：随机数最大值，arg3：存储随机数
    const RI RAND_FLOAT{"RAND_FLOAT", 4, exes::ri_rand_float}; // rand_float 指令：生成随机数，可接受三个参数，arg1：随机数最小值，arg2：随机数最大值，arg3：存储随机数
    const RI LOADIN{"LOADIN", -1, exes::ri_loadin};
    const RI LINK{"LINK", -1, exes::ri_link};
    const RI EXT_GET{"EXT_GET", 3, exes::ri_ext_get};
    const RI EXE_RASM{"EXE_RASM", -1, exes::ri_exe_rasm};
    const RI TP_GET{"TP_GET", 2, exes::ri_tp_get};
    const RI DIV{"DIV", 3, exes::ri_div};
    const RI POW{"POW", 3, exes::ri_pow};
    const RI ROOT{"ROOT", 3, exes::ri_root};
    const RI MOD{"MOD", 3, exes::ri_mod};
    const RI TP_SET{"TP_SET", 2, exes::ri_tp_set};
    const RI TP_DEF{"TP_DEF", -1, exes::ri_tp_def};
    const RI TP_NEW{"TP_NEW", 2, exes::ri_tp_new};
    const RI TP_ADD_INST_FIELD{"TP_ADD_INST_FIELD", -1, exes::ri_tp_add_inst_field};
    const RI TP_ADD_TP_FIELD{"TP_ADD_TP_FIELD", -1, exes::ri_tp_add_tp_field};
    const RI TP_SET_FIELD{"TP_SET_FIELD", 3, exes::ri_tp_set_field};
    const RI TP_GET_FIELD{"TP_GET_FIELD", 3, exes::ri_tp_get_field};
    const RI TP_GET_SUPER_FIELD{"TP_GET_SUPER_FIELD", 4, exes::ri_tp_get_super_field};
    const RI TP_DERIVE{"TP_DERIVE", 2, exes::ri_tp_derive};
    const RI SP_SET{"SP_SET", 1, exes::ri_sp_set};
    const RI SP_GET{"SP_GET", 1, exes::ri_sp_get};
    const RI SP_NEW{"SP_NEW", 1, exes::ri_sp_new}; // sp_new 指令：创建一个新域并自动设置为当前作用域，接受一个参数，指令将新域的ID存储到参数中
    const RI SP_DEL{"SP_DEL", 1, exes::ri_sp_del}; // sp_del 指令：释放指定的域，接受一个参数
    const RI QOT{"QOT", 2, exes::ri_qot}; // qot 指令：将指定数据的ID存储到参数中，可接受两个参数
    const RI QOT_VAL{"QOT_VAL", 2, exes::ri_qot_val};
    const RI EXPOSE{"EXPOSE", 1, exes::ri_expose};
    const RI ATMP{"ATMP", 0, exes::ri_atmp, true};
    const RI DETECT{"DETECT", 2, exes::ri_flag, true};
    const RI RET{"RET", -1, exes::ri_ret};
    const RI PAIR_SET{"PAIR_SET", 3, exes::ri_pair_set};
    const RI PAIR_SET_KEY{"PAIR_SET_KEY", 2, exes::ri_pair_set_key};
    const RI PAIR_SET_VALUE{"PAIR_SET_VALUE", 2, exes::ri_pair_set_value};
    const RI PAIR_GET_KEY{"PAIR_GET_KEY", 2, exes::ri_pair_get_key};
    const RI PAIR_GET_VALUE{"PAIR_GET_VALUE", 2, exes::ri_pair_get_value};
    const RI DICT_SET{"DICT_SET", 3, exes::ri_dict_set};
    const RI DICT_GET{"DICT_GET", 3, exes::ri_dict_get};
    const RI DICT_DEL{"DICT_DEL", -1, exes::ri_dict_del};
    // ToDo: 添加迭代器相关指令
}

namespace core::data {

    Function::Function(std::string func_name, StdArgs args, std::shared_ptr<components::InsSet> func_body)
            : Callable(std::move(args)), func_name(std::move(func_name)), func_body(std::move(func_body)) {}

    components::ExecutionStatus Function::callSelf() const {
        return func_body->execute();
    }

    bool Function::updateData(const std::shared_ptr<RVM_Data> &newData) {
        return false;
    }

    base::RVM_ID &Function::getTypeID() const {
        return typeId;
    }

    std::string Function::getValStr() const {
        std::string args_str;
        for (int i = 0; i < args.size(); ++i) {
            args_str += args[i].getValue();
            if (i != args.size() - 1) {
                args_str += ", ";
            }
        }
        return "[Function: " + this->func_name + "(" + args_str + ")]";
    }

    std::string Function::getTypeName() const {
        return "Function";
    }

    std::shared_ptr<base::RVM_Data> Function::copy_ptr() const {
        return std::make_shared<Function>(func_name, args, func_body);
    }

    bool Function::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        switch (relational) {
            case base::Relational::RE: {
                return other->getTypeID() == Function::typeId;
            }
            case base::Relational::RNE: {
                return other->getTypeID() != Function::typeId;
            }
            case base::Relational::AND: {
                return this->convertToBool() && other->convertToBool();
            }
            case base::Relational::OR: {
                return this->convertToBool() || other->convertToBool();
            }
            default: {
                throw base::errors::ArgTypeMismatchError(unknown_, unknown_,
                                                      {"A type mismatch error occurred during data "
                                                       "comparison: a [Function] value could not be compared for an '"
                                                       + base::relationalToString(relational) + "' relationship."},
                                                      {"Check whether the data is Null during data comparison."});
            }
        }
    }

    RetFunction::RetFunction(std::string func_name, StdArgs args, std::shared_ptr<components::InsSet> func_body)
            : Function(std::move(func_name), std::move(args), std::move(func_body)) {}

    base::RVM_ID &RetFunction::getTypeID() const {
        return RetFunction::typeId;
    }

    std::string RetFunction::getValStr() const {
        std::string args_str;
        for (int i = 0; i < args.size(); ++i) {
            args_str += args[i].getValue();
            if (i != args.size() - 1) {
                args_str += ", ";
            }
        }
        return "[RetFunction: " + this->func_name + "(" + args_str + ")]";
    }

    std::string RetFunction::getTypeName() const {
        return "RetFunction";
    }

    std::shared_ptr<base::RVM_Data> RetFunction::copy_ptr() const {
        return std::make_shared<RetFunction>(func_name, args, func_body);
    }

    bool RetFunction::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        switch (relational) {
            case base::Relational::RE: {
                return other->getTypeID() == RetFunction::typeId;
            }
            case base::Relational::RNE: {
                return other->getTypeID() != RetFunction::typeId;
            }
            case base::Relational::AND: {
                return this->convertToBool() && other->convertToBool();
            }
            case base::Relational::OR: {
                return this->convertToBool() || other->convertToBool();
            }
            default: {
                throw base::errors::ArgTypeMismatchError(unknown_, unknown_,
                                                      {"A type mismatch error occurred during data "
                                                       "comparison: a [RetFunction] value could not be compared for an '"
                                                       + base::relationalToString(relational) + "' relationship."},
                                                      {"Check whether the data is Null during data comparison."});
            }
        }
    }

}

namespace tools {

    std::shared_ptr<base::RVM_Data> getArgOriginDataNoError(const utils::Arg &arg) noexcept {
        switch (arg.getType()) {
            case utils::ArgType::keyword:
            case utils::ArgType::identifier: {
                auto [dataID, data] = data_space_pool.findDataByNameNoLock(arg.getValue());
                if (!data) {
                    return nullptr; // 返回 nullptr 表示找不到数据
                }
                return data;
            }
            case utils::ArgType::number: {
                switch (auto num = utils::stringToNumber(arg.getValue());
                    num.type) {
                    case utils::NumType::int_type:
                        return std::make_shared<data::Int>(num.int_value);
                    case utils::NumType::double_type:
                        return std::make_shared<data::Float>(num.double_value);
                    default:
                        return nullptr; // 返回 nullptr 表示无效的数字类型
                }
            }
            case utils::ArgType::string: {
                return std::make_shared<data::String>(arg.getValue());
            }
            default:
                return nullptr; // 返回 nullptr 表示无效的参数类型
        }
    }

    std::shared_ptr<base::RVM_Data> getArgOriginData(const utils::Arg &arg) {
        auto result = getArgOriginDataNoError(arg);
        if (!result) {
            // 根据不同的情况抛出对应的异常
            switch (arg.getType()) {
                case utils::ArgType::keyword:
                case utils::ArgType::identifier:
                    throw base::errors::MemoryError(unknown_, unknown_, {}, {});
                case utils::ArgType::number:
                    throw std::runtime_error("Invalid number type: " + arg.getValue());
                default:
                    throw std::runtime_error("Invalid argument: " + arg.toString());
            }
        }
        return result;
    }

    std::shared_ptr<base::RVM_Data> getArgNewData(const utils::Arg &arg) {
        if (arg.getType() == utils::ArgType::identifier || arg.getType() == utils::ArgType::keyword) {
            return getArgOriginData(arg)->copy_ptr();
        } else {
            return getArgOriginData(arg);
        }
    }

    std::shared_ptr<base::RVM_Data> processQuoteData(const std::shared_ptr<base::RVM_Data> &data) {
        if (data && data->getTypeID() == data::Quote::typeId){
            return std::static_pointer_cast<data::Quote>(data)->getQuotedData();
        }
        return data;
    }

    bool isNumericData(const std::shared_ptr<base::RVM_Data> &data) {
        return data->getTypeID() == data::Int::typeId || data->getTypeID() == data::Float::typeId ||
               data->getTypeID() == data::Char::typeId || data->getTypeID() == data::Bool::typeId ||
               data->getTypeID() == data::Numeric::typeId;
    }

    bool isIterableData(const base::RVM_ID &data_id) {
        return data_id.fullEqualWith(data::String::typeId) || data_id.fullEqualWith(data::Iterable::typeId) ||
               data_id.fullEqualWith(data::List::typeId) || data_id.fullEqualWith(data::Dict::typeId) ||
               data_id.fullEqualWith(data::Series::typeId);
    }

    bool isMutableData(const base::RVM_ID &data_id) {
        return (isIterableData(data_id) and !data_id.fullEqualWith(data::String::typeId))
        || data_id.fullEqualWith(data::CustomInst::typeId);
    }

    std::pair<bool, std::string> checkExtensionExist(const std::string &file_path) {
        const auto &path = utils::getAbsolutePath(file_path);
        return {memory::loadedExtensions.contains(path),
                path};
    }

    bool checkExtensionExistStrict(const std::string &file_path) {
        return  memory::loadedExtensions.contains(file_path);
    }

    void processQuotedData(std::shared_ptr<base::RVM_Data> &data) {
        if (data->getTypeID().fullEqualWith(data::Quote::typeId)) {
            data = std::static_pointer_cast<data::Quote>(data)->getQuotedData();
        }
    }

    template<typename ExpectedType>
    std::shared_ptr<ExpectedType> checkArgumentDataTypeAndRet(const std::shared_ptr<base::RVM_Data>& arg_data,
                           const components::Ins& ins, const utils::Arg& arg,
                           const int &argIndex, const bool retTargetTypeData = false) {
        if (!arg_data->getTypeID().fullEqualWith(ExpectedType::typeId)) {
            const std::string expectedTypeName = ExpectedType::typeId.toString();
            const std::string instructionName = ins.ri.toString();
            throw base::errors::DataTypeMismatchError(
                    arg.getPosStr(),
                    ins.raw_code,
                    {
                            "The target data type of argument " + std::to_string(argIndex) +
                            " of " + instructionName + " must be " + expectedTypeName + ".",
                            "Error data: " + utils::getSpaceFormatString(
                                arg.getValue(),
                                getArgOriginData(arg)->toString())
                    },
                    {
                            "Check if the argument " + std::to_string(argIndex) + " of " +
                                    instructionName + " is a valid " + expectedTypeName + " data type."
                    }
            );
        }
        if (retTargetTypeData) {
            return std::static_pointer_cast<ExpectedType>(arg_data);
        }
        return nullptr;
    }

    void checkArgumentType(const components::Ins& ins,
        const utils::Arg& arg, const int &argIndex,
        const std::unordered_set<utils::ArgType> &expectedArgTypes) {
        if (!expectedArgTypes.contains(arg.getType())) {
            std::string expectedTypeName = "";
            for (int i = 0;
                const auto &type : expectedArgTypes) {
                expectedTypeName += utils::getTypeFormatString(type);
                if (i != expectedArgTypes.size() - 1) {
                    expectedTypeName += " or ";
                }
                i++;
            }
            const std::string instructionName = ins.ri.toString();
            throw base::errors::ArgTypeMismatchError(
                arg.getPosStr(), ins.raw_code, {
                    "The target arg type of argument " + std::to_string(argIndex) +
                    " of " + instructionName + " must be " + expectedTypeName + ".",
                    "Error arg: " + arg.toString(),
                    "Error arg type: " + utils::getTypeFormatString(arg.getType())
                },
                {
                    "Check if the argument " + std::to_string(argIndex) + " of "
                    + instructionName + " is a valid " + expectedTypeName + " arg type."
                });
        }
    }

}

// 指令执行函数
namespace exes {

    ExecutionStatus ri_flag(const Ins &ins, size_t &pointer, const StdArgs &args) {
        throw std::runtime_error("Executing flag instruction.");
        return ExecutionStatus::FailedWithError;
    }

    ExecutionStatus ri_pass(const Ins &ins, size_t &pointer, const StdArgs &args) {
        // ToDo: temporary do nothing
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_unknown(const Ins &ins, size_t &pointer, const StdArgs &args) {
        throw std::runtime_error("Executing unknown_ instruction.");
        return ExecutionStatus::FailedWithError;
    }

    ExecutionStatus ri_breakpoint(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (InsSet::debug_mode != DebugMode::None) {
            io->immediateOutputError("[Breakpoint]\n");
            InsSet::debug_mode = DebugMode::Standard;
            InsSet::debug_process(std::make_shared<Ins>(ins));
        }
        return ExecutionStatus::Success;
    }

    // 创建变量
    ExecutionStatus ri_allot(const Ins &ins, size_t &pointer, const StdArgs &args) {
        for (const auto &arg: args) {
            if (arg.getType() != utils::ArgType::identifier) {
                throw base::errors::ArgTypeMismatchError(arg.getPosStr(), ins.raw_code,
                                                      {"Error Arg: " + arg.toString(),
                                                       "Target Type: " +
                                                       utils::getTypeFormatString(utils::ArgType::identifier)},
                                                      {"The target argument type of the " +
                                                       ris::ALLOT.toString() + " can only be the " +
                                                       utils::getTypeFormatString(utils::ArgType::identifier) +
                                                       "."});
                return ExecutionStatus::FailedWithError;
            }
            try {
                data_space_pool.addData(arg.getValue(), std::make_shared<data::Null>(data::Null()));
            } catch (const base::errors::DuplicateKeyError &_) {
                throw base::errors::DuplicateKeyError(
                        arg.getPos().toString(), ins.raw_code,
                        {"Error Arg: " + arg.toString(),
                         "Exist Space: " +
                         utils::getSpaceFormatString(arg.getValue(),
                                                     data_space_pool.findDataByNameNoLock(
                                                             arg.getValue()).second->toString())},
                        {"Please use a different name for the variable."});
            }
        }
        return ExecutionStatus::Success;
    }

    // 删除变量
    ExecutionStatus ri_delete(const Ins &ins, size_t &pointer, const StdArgs &args) {
        utils::Arg error_arg;
        try {
            for (const auto &arg: args) {
                error_arg = arg;
                if (arg.getType() != utils::ArgType::identifier) {
                    throw base::errors::ArgTypeMismatchError(arg.getPos().toString(), ins.raw_code,
                                                          {"The type of the target argument of the " +
                                                           ins.ri.toString() +
                                                           " cannot be " + getTypeFormatString(arg.getType()) + ".",
                                                           "Error Arg: " + arg.toString()},
                                                          {"The " + ins.ri.toString() + " can only operate "
                                                                                        "on arguments of " +
                                                           getTypeFormatString(utils::ArgType::identifier) + "."});
                    return ExecutionStatus::FailedWithError;
                }
                data_space_pool.removeDataByNameNoLock(arg.getValue());
            }
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPos().toString(), ins.raw_code,
                                            {"This error is caused by deleting memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"If this memory space is necessary, allocate it manually "
                                             "using " + ris::ALLOT.toString() + " before manipulating it.",
                                             "You seem to be trying to delete a non-existent named memory space, "
                                             "you can fix this error by removing this code."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus mov(const std::shared_ptr<base::RVM_Data> &source_data,
                        const std::shared_ptr<base::RVM_Data> &target_data,
                        const std::string &target_data_name, const bool is_copy){
        if (is_copy) {
            if (const auto copied_data = source_data->copy_ptr();
                !target_data->updateData(copied_data)){
                data_space_pool.updateDataByNameNoLock(target_data_name, copied_data);
            }
        } else {
            if (!target_data->updateData(source_data)){
                data_space_pool.updateDataByNameNoLock(target_data_name, source_data);
            }
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_put(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[1];
        try {
            const auto &arg1 = args[0];
            const auto &arg2 = args[1];
            if (arg2.getType() == utils::ArgType::number || arg2.getType() == utils::ArgType::string) {
                throw base::errors::ArgTypeMismatchError(arg2.getPos().toString(), ins.raw_code,
                                                      {"The type of the target argument of the " +
                                                       ins.ri.toString() +
                                                       " cannot be an immutable argument type.",
                                                       "Error Arg: " + arg2.toString()},
                                                      {"Check whether the target data of the " + ins.ri.toString() +
                                                       " instruction is of mutable type.",
                                                       "Mutable argument types include, but are not limited to 'identifier', 'keyword'."});
            }
            error_arg = arg1;
            const auto &source_data = tools::getArgOriginData(arg1);
            error_arg = arg2;
            const auto &target_data = tools::getArgOriginData(arg2);
            return mov(source_data, target_data, arg2.getValue(), !tools::isIterableData(source_data->getTypeID()));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_copy(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[1];
        try {
            if (args[1].getType() == utils::ArgType::number || args[1].getType() == utils::ArgType::string) {
                throw base::errors::ArgTypeMismatchError(args[1].getPos().toString(), ins.raw_code,
                                                      {"The type of the target argument of the " +
                                                       ins.ri.toString() +
                                                       " cannot be an immutable argument type.",
                                                       "Error Arg: " + args[1].toString()},
                                                      {"Check whether the target data of the " + ins.ri.toString() +
                                                       " instruction is of mutable type.",
                                                       "Mutable argument types include, but are not limited to 'identifier', 'keyword'."});
            }
            error_arg = args[0];
            const auto &source_data = tools::getArgOriginData(args[0]);
            error_arg = args[1];
            const auto &target_data = tools::getArgOriginData(args[1]);
            return mov(source_data, target_data, args[1].getValue(), true);
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_add(const Ins &ins, size_t &pointer, const StdArgs &args)
    {
        tools::checkArgumentType(ins, args[2], 3, {utils::ArgType::identifier, utils::ArgType::keyword});

        auto [data1, data2] = [&args, &ins]() -> std::pair<std::shared_ptr<base::RVM_Data>, std::shared_ptr<base::RVM_Data>> {
            int index = 0;
            try {
                const auto d1 = tools::processQuoteData(tools::getArgNewData(args[0]));
                index = 1;
                const auto d2 = tools::processQuoteData(tools::getArgNewData(args[1]));
                return {d1, d2};
            } catch (const base::errors::MemoryError &_) {
                throw base::errors::MemoryError(args[index].getPosStr(), ins.raw_code,
                                                {"This error is caused by accessing memory space that does not exist.",
                                                 "Nonexistent Space Name: " + args[index].toString()},
                                                {"Use the " + ris::ALLOT.toString() +
                                                 " to manually allocate a named memory space before accessing it."});
            }
        }();

        try {
            // 检查是否有任何一个参数是Iterable类型
            const auto &isData1Iterable = tools::isIterableData(data1->getTypeID());
            const auto &isData2Iterable = tools::isIterableData(data2->getTypeID());
            std::shared_ptr<base::RVM_Data> result;
            if (isData1Iterable && isData2Iterable && data1->getTypeID().fullEqualWith(data2->getTypeID()))
            {
                std::static_pointer_cast<data::Iterable>(data1)->splice(
                    std::static_pointer_cast<data::Iterable>(data2));
                result = data1;
            } else if (isData1Iterable || isData2Iterable)
            {
                if (isData1Iterable)
                {
                    std::static_pointer_cast<data::Iterable>(data1)->append(data2);
                    result = data1;
                } else
                {
                    std::static_pointer_cast<data::Iterable>(data2)->append(data1);
                    result = data2;
                }
            } else
            {
                // 检查是否都是Numeric类型
                const auto numeric1 = dynamic_pointer_cast<data::Numeric>(data1);
                const auto numeric2 = dynamic_pointer_cast<data::Numeric>(data2);

                if (!numeric1 || !numeric2) {
                    // 不是数值类型，抛出类型不匹配错误
                    const auto &error_arg = (!numeric1 ? args[0] : args[1]);
                    const auto &error_data = (!numeric1 ? data1 : data2);
                    throw base::errors::ArgTypeMismatchError(
                            (error_arg.getPosStr()),
                            ins.raw_code,
                            {"Error Arg: " + utils::getSpaceFormatString(error_arg.getValue(), error_data->toString()),
                             "Target Data Type: " + data::Numeric::typeId.toString() + " or " + data::Iterable::typeId.toString()},
                            {"The " + ins.ri.toString() + " requires that "
                                                          "arguments be either both " +
                             data::Numeric::typeId.toString() + " or at least one " +
                             data::Iterable::typeId.toString() + "."});
                }

                // 都是Numeric类型，使用add操作
                result = numeric1->add(numeric2);
            }
            data_space_pool.updateDataByNameNoLock(args[2].getValue(), result);
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[2].getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[2].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        } catch (base::errors::DataTypeMismatchError &e)
        {
            e.error_line = ins.raw_code;
            throw;
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_opp(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &arg1 = args[0];
        const auto &arg2 = args[1];
        if (arg2.getType() == utils::ArgType::number || arg2.getType() == utils::ArgType::string) {
            throw std::runtime_error(
                    "Invalid argument: " + arg2.toString() + ". Third argument must be an identifier.");
            return ExecutionStatus::FailedWithError;
        }
        std::shared_ptr<base::RVM_Data> data1 = tools::getArgNewData(arg1);
        std::shared_ptr<base::RVM_Data> result;
        if (data1->getTypeID() == data::Int::typeId || data1->getTypeID() == data::Float::typeId ||
            data1->getTypeID() == data::Bool::typeId || data1->getTypeID() == data::Char::typeId ||
            data1->getTypeID() == data::Numeric::typeId) {
            auto numeric1 = static_pointer_cast<data::Numeric>(data1);
            result = numeric1->opp();
        } else {
            throw std::runtime_error("Invalid argument types for addition: " + data1->getTypeID().toString());
            return ExecutionStatus::FailedWithError;
        }
        data_space_pool.updateDataByNameNoLock(arg2.getValue(), result);
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_repeat(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args.size() > 2 || args.empty()) {
            throw std::runtime_error("Invalid argument number: " + std::to_string(args.size()));
            return ExecutionStatus::FailedWithError;
        }
        const auto &arg1 = args[0];
        int repeat_times = 0;
        std::shared_ptr<base::RVM_Data> repeated_index = nullptr;
        if (args.size() == 2 && args[1].getValue() != "_") {
            data_space_pool.updateDataByNameNoLock(args[1].getValue(), std::make_shared<data::Int>(0));
            repeated_index = tools::getArgOriginData(args[1]);
        }
        if (arg1.getType() == utils::ArgType::string) {
            throw std::runtime_error("Invalid argument type: " + arg1.toString());
            return ExecutionStatus::FailedWithError;
        }
        if (const auto value1 = tools::getArgNewData(arg1);
            value1->getTypeID() == data::Int::typeId) {
            repeat_times = dynamic_pointer_cast<data::Int>(value1)->getValue();
        } else {
            throw std::runtime_error("Invalid argument type: " + value1->getTypeID().toString());
            return ExecutionStatus::FailedWithError;
        }
        const auto &repeatScope = data_space_pool.acquireScope(pre_REPEAT);
        for (int i = 0; i < repeat_times; i++) {
            if (repeated_index && !repeated_index->updateData(std::make_shared<data::Int>(i))){
                throw std::runtime_error("Invalid argument type: " + repeated_index->getTypeID().toString());
            }
            if (ins.scopeInsSet->execute() == ExecutionStatus::Aborted) {
                break;
            }
        }
        data_space_pool.releaseScopeNoLock(repeatScope);
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_end(const Ins &ins, size_t &pointer, const StdArgs &args) {
        data_space_pool.setCurrentScopeByName(InsSet::exe_stack.top()->scope_name_stack.top());
        if (ins.is_delayed_release_scope) {
            data_space_pool.getCurrentScopeNoLock()->clear();
        } else {
            data_space_pool.releaseScopeNoLock(data_space_pool.getCurrentScopeNoLock());
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_func(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[0].getType() != utils::ArgType::identifier) {
            throw std::runtime_error("Invalid argument type: " + args[0].toString());
        }
        const auto &func_name = args[0].getValue();
        // 使用 emplace 直接在容器中构造对象，避免额外的拷贝或移动
        data_space_pool.addData(func_name,
                                std::make_shared<data::Function>(func_name,
                                                                       std::move(StdArgs{args.begin() + 1,
                                                                                         args.end()}),
                                                                       ins.scopeInsSet));
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_funi(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[0].getType() != utils::ArgType::identifier) {
            throw std::runtime_error("Invalid argument type: " + args[0].toString());
        }
        const auto &func_name = args[0].getValue();
        // 使用 emplace 直接在容器中构造对象，避免额外的拷贝或移动
        data_space_pool.addData(func_name,
                                std::make_shared<data::RetFunction>(func_name,
                                                                          std::move(StdArgs{args.begin() + 1,
                                                                                            args.end()}),
                                                                          ins.scopeInsSet));
        return ExecutionStatus::Success;
    }

    template<typename FuncType>
    std::pair<ExecutionStatus, std::shared_ptr<base::RVM_Data>>
    executeFunctionCall(const Ins &ins, size_t &pointer, const StdArgs &args,
                        const std::string &funcTypeName)
    {
        // 检查第一个参数是否为合法类型
        static auto checkArgType = [&ins](const utils::Arg &arg) {
            if (arg.getType() != utils::ArgType::identifier && arg.getType() != utils::ArgType::keyword) {
                throw base::errors::ArgTypeMismatchError(arg.getPosStr(), ins.raw_code,
                                                      {"The target type of the first argument to the " +
                                                       ins.ri.toString() + " is " +
                                                       getTypeFormatString(utils::ArgType::identifier) +
                                                       " or " + getTypeFormatString(utils::ArgType::keyword) +
                                                       ", not " + getTypeFormatString(arg.getType()) + ".",
                                                       "Error Arg: " + utils::getTypeFormatString(arg.getType())},
                                                      {"Make sure that the target function argument invoked by " +
                                                       ins.ri.toString() + " is of " + utils::getTypeFormatString(
                                                              utils::ArgType::identifier) + " or " +
                                                       utils::getTypeFormatString(utils::ArgType::keyword) +
                                                       "."});
            }
        };

        checkArgType(args[0]);

        const auto &func_name = args[0].getValue();
        auto error_arg = args[0];

        std::shared_ptr<FuncType> function = nullptr;
        std::shared_ptr<memory::RVM_Scope> space = nullptr;

        try {
            // 查找函数
            auto [func_id, func] = data_space_pool.findDataByNameNoLock(func_name);
            if (func == nullptr) {
                throw base::RVM_Error(base::ErrorType::MemoryError, args[0].getPosStr(), ins.raw_code,
                                      {"This error is caused by the access memory space does not exist.",
                                       "Nonexistent Function Name: " + args[0].toString()},
                                      {"The function is defined using the " + ris::FUNC.toString() +
                                       " before it is called."});
            }
            tools::processQuotedData(func);
            if (!func->getTypeID().fullEqualWith(FuncType::typeId)) {
                throw base::errors::DataTypeMismatchError(error_arg.getPosStr(), ins.raw_code,
                                                          {"The target type of the first argument to the " +
                                                              ins.ri.toString() +
                                                              " is " + FuncType::typeId.toString() + ", not " +
                                                              func->getTypeID().toString() + ".",
                                                              "Error Data: " +
                                                              utils::getSpaceFormatString(func_name,
                                                                  func->toString())},
                                                          {"Make sure that the data type of the first argument to the " +
                                                              ins.ri.toString() + " is " +
                                                              FuncType::typeId.toString() + "."});
            }

            function = std::static_pointer_cast<FuncType>(func);

            // 检查参数数量
            const size_t requiredArgCount = function->args.size();

            std::vector<std::shared_ptr<base::RVM_Data>> arg_table{};
            for (int i = 1; i < args.size(); ++i) {
                error_arg = args[i];
                if (const auto &arg = tools::getArgOriginData(args[i]);
                    arg->getTypeID().fullEqualWith(data::Series::typeId)) {
                    const auto &arg_data = std::static_pointer_cast<data::Series>(arg);
                    arg_table.insert(arg_table.end(),
                                     arg_data->getDataList().begin(),
                                     arg_data->getDataList().end());
                    } else {
                        if (tools::isNumericData(arg)) {
                            arg_table.push_back(arg->copy_ptr());
                        } else {
                            arg_table.push_back(arg);
                        }
                    }
            }
            const bool is_ret_func = func->getTypeID().fullEqualWith(data::RetFunction::typeId);
            const size_t providedArgCount = is_ret_func ? arg_table.size() - 1 : arg_table.size();
            if (requiredArgCount != providedArgCount) {
                const std::string &call_str = is_ret_func ? "invoking" : "calling";
                const std::string &call_upper_str = is_ret_func ? "Invoking" : "Calling";
                throw base::RVM_Error(base::ErrorType::ArgumentError, args[args.size() - 1].getPosStr(),
                                      ins.raw_code,
                                      {"This error is caused by a mismatch in the number of arguments passed in when "
                                       + call_str + " the function.",
                                       call_upper_str + " Function: " + function->toString(),
                                       "Required Arg Number: " + std::to_string(requiredArgCount),
                                       "Received Arg Number: " + std::to_string(providedArgCount)},
                                      {"Check that the argument passed into the " + call_str + " function match the "
                                                                                               "number of arguments defined by the function."});
            }

            // 绑定参数
            space = data_space_pool.acquireScope(pre_FUNC + function->func_name);
            const auto &arg_size = args.size();
            for (size_t i = 0; i < requiredArgCount; ++i) {
                error_arg = args[i + 1 < arg_size ? i + 1 : arg_size - 1];
                space->addDataByName(function->args[i].getValue(), arg_table[i]);
            }
        } catch (const base::errors::MemoryError &) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::FUNC.toString() +
                                             " to defined a named function before calling it."});
        }
        // 调用函数
        const auto &res = function->callSelf();
        if (res == ExecutionStatus::AbortedFunction)
        {
            data_space_pool.releaseScopeNoLock(space);
        }
        return {res, function};
    }

    ExecutionStatus ri_call(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto [res, _] = executeFunctionCall<data::Function>(
                ins, pointer, args, data::Function::typeId.toString());
        return res;
    }

    ExecutionStatus ri_ivok(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args.size() < 2) {
            throw base::RVM_Error(base::ErrorType::ArgumentError, args[0].getPosStr(), ins.raw_code,
                                  {"This error is caused by a mismatch in the number of arguments passed in when invoking the func.",
                                   "Details: The " + ins.ri.toString() + " requires 2+ arguments, but " +
                                   std::to_string(args.size())
                                   + " is received."},
                                  {"Check that the argument passed into the invoking func match the " +
                                   ins.ri.toString() + " defined by the func."});
        }
        // 检查最后一个参数是否为合法类型
        auto &ret_name_arg = args[args.size() - 1];
        if (ret_name_arg.getType() != utils::ArgType::identifier &&
            ret_name_arg.getType() != utils::ArgType::keyword) {
            throw base::errors::ArgTypeMismatchError(ret_name_arg.getPosStr(), ins.raw_code,
                                                  {"The target type of the last argument to the " +
                                                   ins.ri.toString() +
                                                   " is " + getTypeFormatString(utils::ArgType::identifier) +
                                                   " or " +
                                                   getTypeFormatString(utils::ArgType::keyword) + ", not " +
                                                   getTypeFormatString(ret_name_arg.getType()) + ".",
                                                   "Error Arg: " +
                                                   utils::getTypeFormatString(ret_name_arg.getType())},
                                                  {"Make sure that the target func argument invoked by " +
                                                   ins.ri.toString() + " is of " +
                                                   utils::getTypeFormatString(utils::ArgType::identifier)
                                                   + " or " + utils::getTypeFormatString(utils::ArgType::keyword) +
                                                   "."});
        }
        auto [res, func] = executeFunctionCall<data::RetFunction>(ins, pointer, args,
                                                                 data::RetFunction::typeId.toString());
        if (res > ExecutionStatus::Aborted && res != ExecutionStatus::AbortedFunction) {
            return res;
        }
        if (func && func->getTypeID() == data::RetFunction::typeId) {
            try {
                data_space_pool.updateDataByNameNoLock(ret_name_arg.getValue(),
                                                       data_space_pool.findDataByIDNoLock(SR_SpaceID)->copy_ptr());
            } catch (const base::errors::MemoryError &) {
                throw base::errors::MemoryError(ret_name_arg.getPosStr(), ins.raw_code,
                                                {"This error is caused by accessing memory space that does not exist.",
                                                 "Nonexistent Space Name: " + ret_name_arg.toString()},
                                                {"Use the " + ris::ALLOT.toString() +
                                                 " to allocate a named memory space before accessing it."});
            }
            return ExecutionStatus::Success;
        }
        return ExecutionStatus::FailedWithError;
    }

    ExecutionStatus ri_until(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[0].getType() != utils::ArgType::identifier && args[0].getType() != utils::ArgType::keyword) {
            throw std::runtime_error("Invalid argument type: " + args[0].toString());
        }
        const auto compData = tools::getArgOriginData(args[0]);
        if (compData->getTypeID() != data::CompareGroup::typeId) {
            throw std::runtime_error("Invalid argument type: " + args[0].toString());
        }
        const auto compGroup = std::static_pointer_cast<data::CompareGroup>(compData);
        if (args[1].getType() != utils::ArgType::keyword) {
            throw std::runtime_error("Invalid argument type: " + args[1].toString());
        }
        const auto relation = base::stringToRelational(args[1].getValue());
        const auto &until_scope = data_space_pool.acquireScope(pre_UNTIL);
        while (!compGroup->compare(relation)) {
            if (const ExecutionStatus result = ins.scopeInsSet->execute();
                result == ExecutionStatus::Aborted)
            {
                break;
            } else if (result > ExecutionStatus::Aborted)
            {
                data_space_pool.releaseScopeNoLock(until_scope);
                return result;
            }
        }
        data_space_pool.releaseScopeNoLock(until_scope);
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_exit(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args.size() > 1) {
            throw std::runtime_error("Invalid argument number: " + std::to_string(args.size()));
        }
        const auto exit_label = args.size() == 1 ? args[0].getValue() : "";
        const auto &top_ins_set = InsSet::exe_stack.top();
        int end_pointer = top_ins_set->getLabel(top_ins_set->scope_prefix + exit_label + suf_END);
        if (end_pointer == -1) {
            end_pointer = top_ins_set->end_pointer;
        }
        if (end_pointer != -1) {
            top_ins_set->getInsSet()[end_pointer]->execute(pointer);
        }
        return ExecutionStatus::Aborted;
    }

    ExecutionStatus ri_sout(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[0].getType() != utils::ArgType::keyword) {
            throw base::RVM_Error(base::ErrorType::SyntaxError, args[0].getPos().toString(), ins.raw_code,
                                  {"This error was caused by using a parameter of the wrong type.",
                                   "Error Arg: " + args[0].toString(),
                                   "Expected Type: " + getTypeFormatString(utils::ArgType::keyword)},
                                  {"The first argument of " + ins.ri.toString() +
                                   " requires the output mode keyword argument.",
                                   "Output mode keywords include 's-l' and 's-m'."});
        }
        const auto ri_opt = RI::getRIByStr(args[0].getValue());
        const bool is_success = ri_opt.has_value();
        const auto &flag = ri_opt.value();
        std::string end_sign;
        if (!is_success) {
            throw std::runtime_error("Invalid argument type: " + args[0].toString());
            return ExecutionStatus::FailedWithError;
        }
        if (flag == ris::S_L) {
            end_sign = "";
        } else if (flag == ris::S_M) {
            end_sign = "\n";
        }
        int error_pointer = 0;
        try {
            for (error_pointer = 1; error_pointer < args.size(); ++error_pointer)
            {
                auto arg = args[error_pointer];

                if (arg.getType() != utils::ArgType::keyword) {
                    *io << tools::getArgOriginData(arg)->getValStr() << end_sign;
                    continue;
                }

                const auto inner_ri_opt = RI::getRIByStr(arg.getValue());
                if (!inner_ri_opt.has_value()) {
                    *io << tools::getArgOriginData(arg)->getValStr() << end_sign;
                    continue;
                }

                if (const auto& inner_flag = inner_ri_opt.value();
                    inner_flag == ris::S_F)
                {
                    io->flushOutputCache();
                } else if (inner_flag == ris::S_N)
                {
                    *io << "\n";
                } else if (inner_flag == ris::S_UNPACK && error_pointer + 1 < args.size())
                {
                    arg = args[++error_pointer];
                    if (const auto &argData = tools::getArgOriginData(arg);
                        tools::isIterableData(argData->getTypeID()))
                    {
                        const auto &iterData = std::static_pointer_cast<data::Iterable>(argData);
                        for (int i = 0; i < iterData->size(); i++)
                        {
                            *io << iterData->getDataAt(i)->getValStr() << end_sign;
                        }
                    } else
                    {
                        *io << argData->getValStr() << end_sign;
                    }
                } else
                {
                    *io << tools::getArgOriginData(arg)->getValStr() << end_sign;
                }
            }
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[error_pointer].getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[error_pointer].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_sin(const Ins &ins, size_t &pointer, const StdArgs &args) {
        io->flushOutputCache();
        // 首先检查参数类型是否为关键字类型
        if (args[0].getType() != utils::ArgType::keyword) {
            throw base::RVM_Error(
                    base::ErrorType::SyntaxError,
                    args[0].getPos().toString(),
                    ins.raw_code,
                    {"This error was caused by using an argument of the wrong type.",
                     "Error Arg: " + args[0].toString(),
                     "Expected Type: " + utils::getTypeFormatString(utils::ArgType::keyword)},
                    {"The first argument of " + ins.ri.toString() +
                     " requires the input mode keyword argument.",
                     "Input mode keyword argument contains 's-l' and 's-m'."}
            );
            return ExecutionStatus::FailedWithError;
        }
        // 类型正确，尝试解析参数值
        const auto ri_opt = RI::getRIByStr(args[0].getValue());
        const bool is_success = ri_opt.has_value();
        const auto &flag = ri_opt.value();
        if (!is_success) {
            throw base::RVM_Error(
                    base::ErrorType::SyntaxError,
                    args[0].getPos().toString(),
                    ins.raw_code,
                    {"This error was caused by an invalid keyword argument value.",
                     "Error Arg: " + args[0].toString(),
                     "Failed to parse as a valid input mode keyword."},
                    {"The keyword argument for " + ins.ri.toString() +
                     " must be 's-l' or 's-m'.",
                     "Ensure the keyword format matches supported input modes."}
            );
            return ExecutionStatus::FailedWithError;
        }
        // 检查解析后的值是否在允许范围内
        if (flag != ris::S_L && flag != ris::S_M) {
            throw base::RVM_Error(
                    base::ErrorType::SyntaxError,
                    args[0].getPos().toString(),
                    ins.raw_code,
                    {"This error was caused by using an unsupported keyword argument.",
                     "Error Arg: " + args[0].toString(),
                     "Expected RI: " + ris::S_L.toString() + " or " + ris::S_M.toString()},
                    {"The first argument of " + ins.ri.toString() +
                     " only accepts 's-l' or 's-m' as input modes.",
                     "Check the documentation for valid input mode keywords."}
            );
            return ExecutionStatus::FailedWithError;
        }
        // 定义数据类型映射
        static const std::unordered_map<std::string, base::IODataType> type_map = {
                {data::Int::typeId.toString(),    base::IODataType::Int},
                {data::Float::typeId.toString(),  base::IODataType::Float},
                {data::Char::typeId.toString(),   base::IODataType::Char},
                {data::Bool::typeId.toString(),   base::IODataType::Bool},
                {data::Null::typeId.toString(),   base::IODataType::Null},
                {data::String::typeId.toString(), base::IODataType::String}
        };
        auto input_type = base::IODataType::String; // 默认为字符串类型
        auto updateData = [&input_type](const utils::Arg &arg) {
            const auto &input_str = io->nextString();
            switch (input_type) {
                case base::IODataType::String: {
                    data_space_pool.updateDataByNameNoLock(arg.getValue(),
                                                           std::make_shared<data::String>(input_str));
                    break;
                }
                case base::IODataType::Int: {
                    if (!utils::isNumber(input_str)) {
                        throw base::errors::ArgTypeMismatchError(unknown_, input_str, {}, {});
                    }
                    data_space_pool.updateDataByNameNoLock(arg.getValue(),
                                                           std::make_shared<data::Int>(
                                                                   utils::stringToInt(input_str)));
                    break;
                }
                case base::IODataType::Null: {
                    data_space_pool.updateDataByNameNoLock(arg.getValue(),
                                                           std::make_shared<data::Null>());
                    break;
                }
                case base::IODataType::Float: {
                    if (!utils::isNumber(input_str)) {
                        throw base::errors::ArgTypeMismatchError(unknown_, input_str, {}, {});
                    }
                    const auto &input_float = utils::stringToDouble(input_str);
                    data_space_pool.updateDataByNameNoLock(arg.getValue(),
                                                           std::make_shared<data::Float>(input_float));
                    break;
                }
                case base::IODataType::Char: {
                    if (!utils::isNumber(input_str)) {
                        throw base::errors::ArgTypeMismatchError(unknown_, input_str, {}, {});
                    }
                    data_space_pool.updateDataByNameNoLock(arg.getValue(),
                                                           std::make_shared<data::Char>(
                                                                   utils::stringToChar(input_str)));
                    break;
                }
                case base::IODataType::Bool: {
                    if (!utils::isNumber(input_str)) {
                        throw base::errors::ArgTypeMismatchError(unknown_, input_str, {}, {});
                    }
                    data_space_pool.updateDataByNameNoLock(arg.getValue(),
                                                           std::make_shared<data::Bool>(
                                                                   utils::stringToBool(input_str)));
                    break;
                }
                default:
                    throw std::runtime_error("Invalid argument type: " + arg.toString());
            }
        };
        auto inputData = [&input_type, &updateData](const utils::Arg &arg, const bool isInput) {
            switch (arg.getType()) {
                case utils::ArgType::identifier: {
                    if (isInput) {
                        io->readLineRaw();
                    }
                    updateData(arg);
                    break;
                }
                case utils::ArgType::keyword: {
                    if (const auto opt = RI::getRIByStr(arg.getValue()); opt.has_value()) {
                        if (const auto inner_flag = opt.value(); inner_flag == ris::S_F) {
                            io->flashInputCache();
                        } else {
                            throw std::runtime_error("Invalid argument type: " + arg.toString());
                        }
                    } else {
                        const auto flag_data = tools::getArgOriginData(arg);
                        if (const auto it = type_map.find(flag_data->getValStr());
                            it != type_map.end()) {
                            input_type = it->second;
                        } else {
                            throw std::runtime_error("Invalid argument type: " + arg.toString());
                        }
                    }
                    break;
                }
                default: {
                    throw std::runtime_error("Invalid argument type: " + arg.toString());
                }
            }
        };
        int error_pointer = 0;
        try {
            if (flag == ris::S_L) {
                for (error_pointer = 1; error_pointer < args.size(); error_pointer++) {
                    if (!io->hasNextString())
                    {
                        io->readLineAndSplit();
                    }
                    inputData(args[error_pointer], false);
                }
            } else if (flag == ris::S_M) {
                for (error_pointer = 1; error_pointer < args.size(); error_pointer++) {
                    inputData(args[error_pointer], true);
                }
            }
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[error_pointer].getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[error_pointer].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        } catch (const base::errors::ArgTypeMismatchError &error) {
            throw base::errors::ArgTypeMismatchError(args[error_pointer].getPos().toString(), ins.raw_code,
                                                  {"The input data type cannot be converted to the specified type.",
                                                   "Input Data: '" + error.error_line + "'",
                                                   "Data Receiver: " + args[error_pointer].toString(),
                                                   "Expected Type: " + base::getIODataTypeName(input_type)},
                                                  {"Ensure that the input data type meets the requirements."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_file_get(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[0];
        try {
            if (args[0].getType() != utils::ArgType::keyword) {
                throw std::runtime_error("Invalid argument type: " + args[0].toString());
            }
            const auto filemode = data::File::stringToFileMode(args[0].getValue());
            error_arg = args[1];
            const auto filepath = tools::getArgOriginData(args[1]);
            if (filepath->getTypeID() != data::String::typeId) {
                throw std::runtime_error("Invalid argument type: " + args[1].toString());
            }
            error_arg = args[2];
            if (args[2].getType() != utils::ArgType::identifier && args[2].getType() != utils::ArgType::keyword) {
                throw std::runtime_error("Invalid argument type: " + args[2].toString());
            }
            data_space_pool.updateDataByNameNoLock(args[2].getValue(),
                                                   std::make_shared<data::File>(filemode, filepath->getValStr()));
            return ExecutionStatus::Success;
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
    }

    ExecutionStatus ri_file_read(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto filedata = tools::getArgOriginData(args[0]);
        if (filedata->getTypeID() != data::File::typeId) {
            throw std::runtime_error("Invalid argument type: " + args[0].toString());
        }
        if (args[1].getType() != utils::ArgType::identifier && args[1].getType() != utils::ArgType::keyword) {
            throw std::runtime_error("Invalid argument type: " + args[1].toString());
        }
        try {
            data_space_pool.updateDataByNameNoLock(args[1].getValue(),
                                                   std::make_shared<data::String>(
                                                           static_pointer_cast<data::File>(filedata)->readFile()));
        } catch (const base::errors::FileReadError &_) {
            throw base::errors::FileReadError(args[0].getPosStr(), ins.raw_code,
                                              {"This error is caused by reading a file that does not exist.",
                                               "File Name: " + utils::getSpaceFormatString(args[0].getValue(),
                                                                                           filedata->toString())},
                                              {"Ensure that the file exists."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_file_write(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[1];
        try {
            if (args[1].getType() != utils::ArgType::identifier && args[1].getType() != utils::ArgType::keyword) {
                throw std::runtime_error("Invalid argument type: " + args[1].toString());
            }
            const auto filedata = tools::getArgOriginData(args[1]);
            if (filedata->getTypeID() != data::File::typeId) {
                throw std::runtime_error("Invalid argument type: " + args[1].toString());
            }
            const auto file = static_pointer_cast<data::File>(filedata);
            error_arg = args[0];
            if (file->writeFile(tools::getArgOriginData(args[0])->getValStr())) {
                return ExecutionStatus::Success;
            } else {
                throw base::errors::FileWriteError(args[0].getPos().toString(), ins.raw_code,
                                                   {"The file cannot be written.",
                                                    "Error File: " + file->toString()},
                                                   {"Ensure that the file exists and has write permission."});
            }
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
    }

    ExecutionStatus ri_file_get_path(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[0];
        try {
            if (args[0].getType() != utils::ArgType::identifier && args[0].getType() != utils::ArgType::keyword) {
                throw std::runtime_error("Invalid argument type: " + args[0].toString());
            }
            auto file = tools::getArgOriginData(args[0]);
            if (file->getTypeID() != data::File::typeId) {
                throw std::runtime_error("Invalid argument type: " + args[0].toString());
            }
            error_arg = args[1];
            if (args[1].getType() != utils::ArgType::identifier && args[1].getType() != utils::ArgType::keyword) {
                throw std::runtime_error("Invalid argument type: " + args[1].toString());
            }
            data_space_pool.updateDataByNameNoLock(args[1].getValue(),
                                                   std::make_shared<data::String>(
                                                           static_pointer_cast<data::File>(file)->filepath));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_file_get_mode(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[1];
        try {
            if (args[1].getType() != utils::ArgType::identifier && args[1].getType() != utils::ArgType::keyword) {
                throw std::runtime_error("Invalid argument type: " + args[1].toString());
            }
            error_arg = args[0];
            const auto &file = tools::getArgOriginData(args[0]);
            if (file->getTypeID() != data::File::typeId) {
                throw std::runtime_error("Invalid argument type: " + args[0].toString());
            }
            error_arg = args[1];
            data_space_pool.updateDataByNameNoLock(args[1].getValue(),
                                                   std::make_shared<data::String>(
                                                           data::File::fileModeToString(
                                                                   static_pointer_cast<data::File>(
                                                                           file)->fileMode)));
            return ExecutionStatus::Success;
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
    }

    ExecutionStatus ri_file_get_size(const Ins &ins, size_t &pointer, const StdArgs &args) {
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_file_set_mode(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[1].getType() != utils::ArgType::identifier && args[1].getType() != utils::ArgType::keyword) {
            throw std::runtime_error("Invalid argument type: " + args[1].toString());
        }
        auto file = tools::getArgOriginData(args[1]);
        if (file->getTypeID() != data::File::typeId) {
            throw std::runtime_error("Invalid argument type: " + args[1].toString());
        }
        if (args[0].getType() != utils::ArgType::keyword) {
            throw std::runtime_error("Invalid argument type: " + args[0].toString());
        }
        static_pointer_cast<data::File>(file)->setModeByString(args[0].getValue());
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_file_set_path(const Ins &ins, size_t &pointer, const StdArgs &args) {
        return ExecutionStatus::Success;
    }


    ExecutionStatus ri_jmp(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &top_ins_set = InsSet::exe_stack.top();
        const int &label = top_ins_set->getLabel(args[0].getValue());
        if (label == -1) {
            throw std::runtime_error("Invalid label: " + args[0].getValue());
            return ExecutionStatus::FailedWithError;
        }
        pointer = label;
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_jr(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[0].getType() != utils::ArgType::identifier && args[0].getType() != utils::ArgType::keyword) {
            throw std::runtime_error("Invalid argument type: " + args[0].toString());
        }
        auto compData = tools::getArgOriginData(args[0]);
        if (compData->getTypeID() != data::CompareGroup::typeId) {
            throw std::runtime_error("Invalid argument type: " + args[0].toString());
        }
        auto compGroup = static_pointer_cast<data::CompareGroup>(compData);
        if (args[1].getType() != utils::ArgType::keyword) {
            throw std::runtime_error("Invalid argument type: " + args[1].toString());
        }
        auto relational = base::stringToRelational(args[1].getValue());
        if (args[2].getType() != utils::ArgType::identifier) {
            throw std::runtime_error("Invalid argument type: " + args[2].toString());
        }
        if (compGroup->compare(relational)) {
            const int &label = InsSet::exe_stack.top()->getLabel(args[2].getValue());
            if (label == -1) {
                throw std::runtime_error("Invalid label: " + args[2].getValue());
                return ExecutionStatus::FailedWithError;
            }
            pointer = label;
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_jf(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &est = tools::getArgOriginData(args[0]);
        const auto &est_type_id = est->getTypeID();
        const auto &label = InsSet::exe_stack.top()->getLabel(args[1].getValue());
        if (label == -1) {
            throw std::runtime_error("Invalid label: " + args[1].getValue());
            return ExecutionStatus::FailedWithError;
        }
        if (est_type_id.fullEqualWith(data::Int::typeId) || est_type_id.fullEqualWith(data::Bool::typeId) ||
            est_type_id.fullEqualWith(data::Char::typeId)) {
            const auto &est_value = static_pointer_cast<data::Int>(est)->getValue();
            if (est_value == 0) {
                pointer = label;
            }
        } else if (est_type_id.fullEqualWith(data::Float::typeId)) {
            auto est_value = static_pointer_cast<data::Float>(est)->getValue();
            if (est_value < 0) {
                est_value = 0 - est_value;
            }
            if (est_value < 0.0000001) {
                pointer = label;
            }
        } else if (est_type_id.fullEqualWith(data::String::typeId)) {
            if (static_pointer_cast<data::String>(est)->getValStr().empty()) {
                pointer = label;
            }
        } else if (est_type_id.fullEqualWith(data::Null::typeId)) {
            pointer = label;
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_jt(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &est = tools::getArgOriginData(args[0]);
        const auto &est_type_id = est->getTypeID();
        const auto &label = InsSet::exe_stack.top()->getLabel(args[1].getValue());

        if (label == -1) {
            throw std::runtime_error("Invalid label: " + args[1].getValue());
            return ExecutionStatus::FailedWithError;
        }

        if (est_type_id.fullEqualWith(data::Int::typeId) ||
            est_type_id.fullEqualWith(data::Bool::typeId) ||
            est_type_id.fullEqualWith(data::Char::typeId)){
            const auto &est_value = static_pointer_cast<data::Int>(est)->getValue();
            if (est_value != 0) {
                pointer = label;
            }
        } else if (est_type_id.fullEqualWith(data::Float::typeId)) {
            auto est_value = static_pointer_cast<data::Float>(est)->getValue();
            est_value = std::abs(est_value); // 使用标准库函数更简洁
            if (est_value >= 0.0000001) {
                pointer = label;
            }
        } else if (est_type_id.fullEqualWith(data::String::typeId)) {
            if (!static_pointer_cast<data::String>(est)->getValStr().empty()) {
                pointer = label;
            }
        } else if (!est_type_id.fullEqualWith(data::Null::typeId)) {
            // 非Null类型视为真，执行跳转
            pointer = label;
        }

        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_cmp(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[0];
        try
        {
            static auto data_check = [&args](
                    const utils::Arg &arg) -> std::pair<id::DataID, std::shared_ptr<base::RVM_Data>> {
                switch (arg.getType()) {
                case utils::ArgType::identifier:
                case utils::ArgType::keyword: {
                        auto [id, data] = data_space_pool.findDataByNameNoLock(arg.getValue());
                        return {id, data};
                }
                default: {
                        auto [id, data] = data_space_pool.addData(
                                args[2].getValue() + "_id_" + std::to_string(
                                        tools::getArgOriginData(arg)->getInstID().dis_id),
                                tools::getArgOriginData(arg));
                        return {id, data};
                }
                }
            };
            auto [leftID, dataLeft] = data_check(args[0]);
            if (!dataLeft) {
                throw base::errors::MemoryError(unknown_, unknown_, {}, {});
            }
            error_arg = args[1];
            auto [rightID, dataRight] = data_check(args[1]);
            if (!dataRight) {
                throw base::errors::MemoryError(unknown_, unknown_, {}, {});
            }
            error_arg = args[2];
            tools::checkArgumentType(ins, args[2], 3,
                                         {utils::ArgType::keyword, utils::ArgType::identifier});
            const auto& dataName = args[2].getValue();
            data_space_pool.updateDataByNameNoLock(dataName,
                                                   std::make_shared<data::CompareGroup>(
                                                       std::make_shared<id::DataID>(leftID),
                                                       std::make_shared<id::DataID>(rightID)));
        }
        catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_crel(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[0];
        try {
            const auto &cmp = tools::getArgOriginData(args[0]);
            if (!cmp->getTypeID().fullEqualWith(data::CompareGroup::typeId)) {
                throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                      {"Error Type: " + cmp->getTypeID().toString(),
                                                       "Expected Type: " + data::CompareGroup::typeId.toString()},
                                                      {"The first argument of " + ins.ri.toString() + " must be " +
                                                       data::CompareGroup::typeId.toString() + "."});
            }
            const auto &cmp_data = static_pointer_cast<data::CompareGroup>(cmp);
            error_arg = args[1];
            const auto &relational = base::stringToRelational(args[1].getValue());
            error_arg = args[2];
            data_space_pool.updateDataByNameNoLock(args[2].getValue(),
                                                   std::make_shared<data::Bool>(cmp_data->compare(relational)));
        } catch (const base::errors::MemoryError &_){
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_mul(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[2].getType() != utils::ArgType::identifier) {
            throw std::runtime_error("Third argument must be an identifier");
            return ExecutionStatus::FailedWithError;
        }
        auto [data1, data2] = [&args]() -> std::pair<std::shared_ptr<data::Numeric>, std::shared_ptr<data::Numeric>> {
            auto d1 = tools::getArgNewData(args[0]);
            auto d2 = tools::getArgNewData(args[1]);
            // 提前进行类型检查，避免多次dynamic_cast
            if (d1->getTypeID() != data::Numeric::typeId ||
                d2->getTypeID() != data::Numeric::typeId) {
                throw std::runtime_error("Both arguments must be numeric");
            }
            return {
                    static_pointer_cast<data::Numeric>(d1),
                    static_pointer_cast<data::Numeric>(d2)
            };
        }();
        data_space_pool.updateDataByNameNoLock(args[2].getValue(), data1->multiply(data2));
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_iter_apnd(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[0];
        try {
            const auto &arg_size = args.size();
            if (arg_size < 2) {
                throw base::errors::ArgumentNumberError(ins.pos.toString(), ins.raw_code,
                                                        "2+", arg_size,
                                                        ins.ri.name, {});
            }
            const auto &target_arg = args[arg_size - 1];
            error_arg = target_arg;
            if (target_arg.getType() != utils::ArgType::identifier &&
                target_arg.getType() != utils::ArgType::keyword) {
                throw base::errors::ArgTypeMismatchError(target_arg.getPos().toString(), ins.raw_code,
                                                      {"The last arg of the " + ins.ri.toString() + " must be " +
                                                       getTypeFormatString(utils::ArgType::identifier)
                                                       + " or " + getTypeFormatString(utils::ArgType::keyword) +
                                                       ".",
                                                       "Error Arg: " + target_arg.toString()},
                                                      {"Check the last arg type for which the " +
                                                       ins.ri.toString() +
                                                       " acts."});
            }
            const auto &target_data = tools::getArgOriginData(target_arg);
            if (target_data->getTypeID() != data::Iterable::typeId &&
                target_data->getTypeID() != data::String::typeId) {
                throw base::errors::DataTypeMismatchError(target_arg.getPos().toString(), ins.raw_code,
                                                      {"The target data of the "
                                                          + ris::ITER_APND.toString() + " must be "
                                                          + data::Iterable::typeId.toString() + ".",
                                                       "Error Data: " +
                                                       utils::getSpaceFormatString(target_arg.getValue(),
                                                                                   target_data->toString())},
                                                      {"Check the target data type for which the "
                                                          + ris::ITER_APND.toString() + " acts."});
            }
            const auto target = std::static_pointer_cast<data::Iterable>(target_data);
            for (int i = 0; i < arg_size - 1; i++) {
                error_arg = args[i];
                auto data = tools::getArgOriginData(args[i]);
                if (tools::isNumericData(data) || data->getTypeID().fullEqualWith(data::String::typeId)) {
                    data = data->copy_ptr();
                }
                target->append(data);
            }
        } catch (const base::errors::MemoryError &_){
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        } catch (base::errors::ArgTypeMismatchError &e) {
            e.error_position = error_arg.getPosStr();
            e.error_line = ins.raw_code;
            e.error_info.insert(e.error_info.begin() + 1,
                "Error arg: " + utils::getSpaceFormatString(error_arg.getValue(),
                    tools::getArgOriginDataNoError(error_arg)->toString()));
            throw;
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_iter_sub(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[3];
        try {
            if (args[3].getType() != utils::ArgType::identifier && args[3].getType() != utils::ArgType::keyword) {
                throw base::errors::ArgTypeMismatchError(args[3].getPos().toString(), ins.raw_code,
                                                      {"The type of the target argument of the " +
                                                       ins.ri.toString() +
                                                       " cannot be an immutable argument type.",
                                                       "Error Arg: " + args[3].toString()},
                                                      {"Check whether the target data of the " + ins.ri.toString() +
                                                       " instruction is of mutable type."});
            }
            error_arg = args[0];
            const auto &target = tools::getArgOriginData(args[0]);
            if (!tools::isIterableData(target->getTypeID())) {
                throw base::errors::ArgTypeMismatchError(args[0].getPos().toString(), ins.raw_code,
                                                      {"The target data of the " + ins.ri.toString() +
                                                       " must be " + data::Iterable::typeId.toString() + ".",
                                                       "Error Data: " +
                                                       utils::getSpaceFormatString(args[0].getValue(),
                                                                                   target->toString())},
                                                      {"Check the target data type for which the " +
                                                       ins.ri.toString() +
                                                       " acts."});
            }
            const auto &target_data = std::static_pointer_cast<data::Iterable>(target);
            const auto [begin, end] = [&]() -> std::pair<int, int> {
                auto validateArgType = [&](const auto &arg, size_t argIndex) {
                    const auto &index = tools::getArgOriginData(arg);
                    if (index->getTypeID() != data::Int::typeId) {
                        throw base::errors::DataTypeMismatchError(arg.getPosStr(), ins.raw_code,
                                                              {"The type of the " + std::to_string(argIndex) +
                                                               "-th argument of the " +
                                                               ins.ri.toString() + " must be " +
                                                               data::Int::typeId.toString() + ".",
                                                               "Error Data: " +
                                                               utils::getSpaceFormatString(arg.getValue(),
                                                                                           index->toString())
                                                              },
                                                              {"Check the type of the " +
                                                               std::to_string(argIndex) +
                                                               "-th argument for which the " + ins.ri.toString() +
                                                               " acts."});
                    }
                    return std::static_pointer_cast<data::Int>(index)->getValue();
                };
                error_arg = args[1];
                int beginValue = validateArgType(args[1], 1);
                error_arg = args[2];
                int endValue = validateArgType(args[2], 2);
                return {beginValue, endValue};
            }();
            error_arg = args[3];
            if (begin < 0 || begin > end || begin >= target_data->size() || end < 0) {
                throw base::RVM_Error(base::ErrorType::RangeError, args[2].getPosStr(), ins.raw_code,
                                      {"This error is caused by index values that are out of the iterable range.",
                                       "Error Range: [" + std::to_string(begin) + ", " + std::to_string(end) + ")",
                                       "Available Range: [0, " + std::to_string(target_data->size()) + ")"},
                                      {"Modify the range of indexes to be obtained."});
            }
            data_space_pool.updateDataByNameNoLock(args[3].getValue(), target_data->subpart(begin, end));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_iter_size(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[1].getType() != utils::ArgType::identifier && args[1].getType() != utils::ArgType::keyword) {
            throw base::errors::ArgTypeMismatchError(args[1].getPos().toString(), ins.raw_code,
                                                  {"The type of the target argument of the " +
                                                   ins.ri.toString() +
                                                   " cannot be an immutable argument type.",
                                                   "Error Arg: " + args[1].toString()},
                                                  {"Check whether the target data of the " + ins.ri.toString() +
                                                   " instruction is of mutable type."});
        }
        auto error_arg = args[0];
        try {
            const auto &data = tools::getArgOriginData(args[0]);
            if (!tools::isIterableData(data->getTypeID())) {
                throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                      {"The type of the target argument of the " +
                                                       ins.ri.toString() +
                                                       " must be " + data::Iterable::typeId.toString() + ".",
                                                       "Error Data: " +
                                                       utils::getSpaceFormatString(args[0].getValue(),
                                                                                   data->toString())},
                                                      {"Check whether the target data of the " + ins.ri.toString() +
                                                       " instruction is of iterable type."});
            }
            error_arg = args[1];
            data_space_pool.updateDataByNameNoLock(args[1].getValue(),
                                                   std::make_shared<data::Int>(
                                                           std::static_pointer_cast<data::Iterable>(data)->
                                                                   size()));
        } catch ([[maybe_unused]] const base::errors::MemoryError &e) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_iter_get(const Ins &ins, size_t &pointer, const StdArgs &args)
    {
        // 提前检查 args[2] 的类型
        const auto &targetArg = args[2];
        if (targetArg.getType() != utils::ArgType::identifier && targetArg.getType() != utils::ArgType::keyword) {
            throw base::errors::ArgTypeMismatchError(
                    targetArg.getPos().toString(), ins.raw_code,
                    {"The type of the target argument of the " + ins.ri.toString() +
                     " cannot be an immutable argument type.",
                     "Error Arg: " + targetArg.toString()},
                    {"Check whether the target data of the " + ins.ri.toString() +
                     " instruction is of mutable type."});
        }

        // 获取迭代数据
        const auto &iterArg = args[0];
        const auto iterData = tools::getArgOriginData(iterArg);
        if (!tools::isIterableData(iterData->getTypeID())) {
            throw base::errors::ArgTypeMismatchError(
                    iterArg.getPosStr(), ins.raw_code,
                    {"The type of the target argument of the " + ins.ri.toString() +
                     " must be " + data::Iterable::typeId.toString() + ".",
                     "Error Data: " + utils::getSpaceFormatString(iterArg.getValue(), iterData->toString())},
                    {"Check whether the target data of the " + ins.ri.toString() +
                     " instruction is of " + data::Iterable::typeId.toString() + "."});
        }
        const auto &iter = std::static_pointer_cast<data::Iterable>(iterData);

        // 获取索引数据
        const auto &indexArg = args[1];
        std::shared_ptr<base::RVM_Data> data;
        try
        {
            if (const auto indexData = tools::getArgOriginData(indexArg);
                iterData->getTypeID().fullEqualWith(data::Dict::typeId) &&
                indexData->getTypeID().fullEqualWith(data::String::typeId))
            {
                const auto &dictData = std::static_pointer_cast<data::Dict>(iterData);
                const auto &index = std::static_pointer_cast<data::String>(indexData);
                data = dictData->getDataAt(index->toEscapedString());
            } else
            {
                if (!indexData->getTypeID().fullEqualWith(data::Int::typeId)) {
                    throw base::errors::ArgTypeMismatchError(
                            indexArg.getPosStr(), ins.raw_code,
                            {"The type of the index argument of the " + ins.ri.toString() +
                             " must be " + data::Int::typeId.toString() + ".",
                             "Error Data: " + utils::getSpaceFormatString(indexArg.getValue(), indexData->toString())},
                            {"Check whether the target data of the " + ins.ri.toString() +
                             " instruction is of " + data::Int::typeId.toString() + "."});
                }
                // 检查索引范围
                const auto &index = std::static_pointer_cast<data::Int>(indexData);
                if (index->getValue() >= iter->size() || index->getValue() < 0) {
                    throw base::RVM_Error(
                            base::ErrorType::RangeError, indexArg.getPosStr(), ins.raw_code,
                            {"The index of the " + ins.ri.toString() + " instruction is out of range.",
                             "Index: " + std::to_string(index->getValue()),
                             "Size: " + std::to_string(iter->size())},
                            {"Check whether the specified index value is out of the range."});
                }
                data = iter->getDataAt(index->getValue());
            }

            // 获取数据并更新内存池
            data_space_pool.updateDataByNameNoLock(targetArg.getValue(), data);
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(targetArg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + targetArg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_iter_common(const Ins &ins, size_t &pointer, const StdArgs &args, bool reverse) {
        auto error_arg = args[0];
        try{
            const auto &iter_container = tools::getArgOriginData(args[0]);
            const auto &iter_elem = args[1];
            error_arg = args[1];
            tools::getArgOriginData(error_arg);
            const auto &iter_index = args[2];
            error_arg = args[2];
            tools::getArgOriginData(error_arg);

            // 检查迭代容器的类型
            if (!tools::isIterableData(iter_container->getTypeID())) {
                throw base::errors::ArgTypeMismatchError(
                        args[0].getPosStr(), ins.raw_code,
                        {"The type of the target argument of the " + ins.ri.toString() +
                         " must be " + data::Iterable::typeId.toString() + ".",
                         "Error Data: " + utils::getSpaceFormatString(args[0].getValue(),
                                                                      iter_container->toString())},
                        {"Check whether the target data of the " + ins.ri.toString() +
                         " is of " + data::Iterable::typeId.toString() + "."});
            }

            const auto &iter_container_data = std::static_pointer_cast<data::Iterable>(iter_container);

            // 根据 reverse 参数决定使用正向还是反向遍历
            data_space_pool.acquireScope(reverse ? pre_ITER_REV_TRAV : pre_ITER_TRAV);

            const int &container_size = static_cast<int>(iter_container_data->size());
            const int start = reverse ? container_size - 1 : 0;
            const int end = reverse ? -1 : container_size;
            const int step = reverse ? -1 : 1;
            for (int i = start; i != end; i += step) {
                data_space_pool.updateDataByNameNoLock(iter_elem.getValue(),
                                                       iter_container_data->getDataAt(i)->copy_ptr());
                data_space_pool.updateDataByNameNoLock(iter_index.getValue(),
                                                       std::make_shared<data::Int>(reverse ? start - i : i));
                if (ins.scopeInsSet->execute() == ExecutionStatus::Aborted) {
                    break;
                }
            }
            data_space_pool.releaseScopeNoLock(data_space_pool.getCurrentScopeNoLock());
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_iter_trav(const Ins &ins, size_t &pointer, const StdArgs &args) {
        return ri_iter_common(ins, pointer, args, false);
    }

    ExecutionStatus ri_iter_rev_trav(const Ins &ins, size_t &pointer, const StdArgs &args) {
        return ri_iter_common(ins, pointer, args, true);
    }

    ExecutionStatus ri_iter_set(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &iter_container = tools::getArgOriginData(args[0]);
        if (!tools::isIterableData(iter_container->getTypeID())) {
            throw base::errors::ArgTypeMismatchError(
                    args[0].getPosStr(), ins.raw_code,
                    {"The type of the target argument of the " + ins.ri.toString() +
                     " must be " + data::Iterable::typeId.toString() + ".",
                     "Error Data: " + utils::getSpaceFormatString(args[0].getValue(),
                                                                  iter_container->toString())},
                    {"Check whether the target data of the " + ins.ri.toString() +
                     " is of " + data::Iterable::typeId.toString() + "."});
        }
        const auto &iter_container_data = std::static_pointer_cast<data::Iterable>(iter_container);
        const auto &index = tools::getArgOriginData(args[1]);
        const auto &data = tools::getArgOriginData(args[2]);
        if (index->getTypeID() != data::Int::typeId) {
            throw base::errors::ArgTypeMismatchError(
                    args[1].getPosStr(), ins.raw_code,
                    {"The type of the index argument of the " + ins.ri.toString() +
                     " must be " + data::Int::typeId.toString() + ".",
                     "Error Data: " + utils::getSpaceFormatString(args[1].getValue(),
                                                                  index->toString())},
                    {"Check whether the target data of the " + ins.ri.toString() +
                     " is of " + data::Int::typeId.toString() + "."});
        }
        const auto &index_data = std::static_pointer_cast<data::Int>(index)->getValue();
        iter_container_data->setDataAt(index_data, data);
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_iter_del(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args.size() < 2) {
            throw base::errors::ArgumentNumberError(ins.pos.toString(), ins.raw_code,
                                                    "2+", args.size(), ins.ri.toString(),
                                                    {"Add more index arg to fix this error."});
        }
        const auto &iter_container = tools::getArgOriginData(args[0]);
        if (!tools::isIterableData(iter_container->getTypeID())) {
            throw base::errors::ArgTypeMismatchError(
                    args[0].getPosStr(), ins.raw_code,
                    {"The type of the target argument of the " + ins.ri.toString() +
                     " must be " + data::Iterable::typeId.toString() + ".",
                     "Error Data: " + utils::getSpaceFormatString(args[0].getValue(),
                                                                  iter_container->toString())},
                    {"Check whether the target data of the " + ins.ri.toString() +
                     " is of " + data::Iterable::typeId.toString() + "."});
        }
        const auto &iter_container_data = std::static_pointer_cast<data::Iterable>(iter_container);
        for (int i = 1; i < args.size(); i++) {
            const auto &index = tools::getArgOriginData(args[i]);
            if (index->getTypeID() != data::Int::typeId) {
                throw base::errors::ArgTypeMismatchError(
                        args[i].getPosStr(), ins.raw_code,
                        {"The type of the index argument of the " + ins.ri.toString() +
                         " must be " + data::Int::typeId.toString() + "."},
                        {"Check whether the target data of the " + ins.ri.toString() +
                         " is of " + data::Int::typeId.toString() + "."});
            }
            iter_container_data->eraseDataAt(std::static_pointer_cast<data::Int>(index)->getValue() - i + 1);
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_iter_insert(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &iter_container = tools::getArgOriginData(args[0]);
        if (!tools::isIterableData(iter_container->getTypeID())) {
            throw base::errors::ArgTypeMismatchError(
                    args[0].getPosStr(), ins.raw_code,
                    {"The type of the target argument of the " + ins.ri.toString() +
                     " must be " + data::Iterable::typeId.toString() + ".",
                     "Error Data: " + utils::getSpaceFormatString(args[0].getValue(),
                                                                  iter_container->toString())},
                    {"Check whether the target data of the " + ins.ri.toString() +
                     " is of " + data::Iterable::typeId.toString() + "."});
        }
        const auto &iter_container_data = std::static_pointer_cast<data::Iterable>(iter_container);
        const auto &index = tools::getArgOriginData(args[1]);
        if (index->getTypeID() != data::Int::typeId) {
            throw base::errors::ArgTypeMismatchError(
                    args[1].getPosStr(), ins.raw_code,
                    {"The type of the index argument of the " + ins.ri.toString() +
                     " must be " + data::Int::typeId.toString() + "."},
                    {"Check whether the target data of the " + ins.ri.toString() +
                     " is of " + data::Int::typeId.toString() + "."});
        }
        const auto &data = tools::getArgOriginData(args[2]);
        iter_container_data->insertDataAt(std::static_pointer_cast<data::Int>(index)->getValue(), data);
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_iter_unpack(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[0];
        try
        {
            const auto &iter_container = tools::getArgOriginData(args[0]);
            if (!tools::isIterableData(iter_container->getTypeID())) {
                throw base::errors::ArgTypeMismatchError(
                        args[0].getPosStr(), ins.raw_code,
                        {"The type of the target argument of the " + ins.ri.toString() +
                         " must be " + data::Iterable::typeId.toString() + ".",
                         args[0].getType() == utils::ArgType::identifier ?
                         "Error Space: " + utils::getSpaceFormatString(
                                 args[0].getValue(),iter_container->toString())
                                                                         :"Error Data: " + iter_container->toString()},
                        {"Check whether the target data of the " + ins.ri.toString() +
                         " is of " + data::Iterable::typeId.toString() + "."});
            }
            const auto &iter_container_data = std::static_pointer_cast<data::Iterable>(iter_container);
            if (args.size() - 1 != iter_container_data->size()) {
                throw base::RVM_Error(base::ErrorType::ArgumentError, args[args.size() - 1].getPosStr(),
                                      ins.raw_code,
                                      {"This error is caused by a mismatch in the number of arguments passed in when unpacking the iterable data.",
                                       "Iterable Data: " + iter_container_data->toString(),
                                       "Required Arg Number: " + std::to_string(iter_container_data->size()),
                                       "Supported Arg Number: " + std::to_string(args.size() - 1)},
                                      {"Check that the argument passed into the " + ins.ri.toString() + " match the "
                                                                                                        "number of arguments required by the iterable data."});
            }
            for (int i = 0; i < args.size() - 1; i++)
            {
                const auto& data = iter_container_data->getDataAt(i);
                error_arg = args[i + 1];
                data_space_pool.updateDataByNameNoLock(args[i + 1].getValue(), data);
            }
        }
        catch (base::errors::MemoryError &_){
            throw base::errors::MemoryError(error_arg.getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        catch (base::errors::KeyNotFoundError &e)
        {
            e.error_line = ins.raw_code;
            e.error_position = error_arg.getPosStr();
            e.error_info.push_back("Dict data space: " + utils::getSpaceFormatString(
                args[0].getValue(), tools::getArgOriginData(args[0])->toString()));
            e.error_info.push_back("Nonexistent key: '" + error_arg.getValue() + "'");
            throw;
        }
        return ExecutionStatus::Success;
    }
    template <typename T>
    std::shared_ptr<T> check_arg_data_type(const Ins &ins, const utils::Arg &check_arg,
                                           const std::shared_ptr<base::RVM_Data> check_data) {
        if (!check_data->getTypeID().fullEqualWith(T::typeId)){
            throw base::errors::DataTypeMismatchError(
                    check_arg.getPosStr(), ins.raw_code,
                    {"The type of the target argument of the " + ins.ri.toString() +
                     " must be " + T::typeId.toString() + "."},
                    {"Check whether the target data of the " + ins.ri.toString() +
                     " is of " + T::typeId.toString() + "."});
        }
        return std::static_pointer_cast<T>(check_data);
    }

    ExecutionStatus ri_rand_int(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &min = tools::getArgOriginData(args[0]);
        const auto &min_data = check_arg_data_type<data::Int>(ins, args[0], min);
        const auto &max = tools::getArgOriginData(args[1]);
        const auto &max_data = check_arg_data_type<data::Int>(ins, args[1], max);
        data_space_pool.updateDataByNameNoLock(args[2].getValue(),
                                               std::make_shared<data::Int>(utils::getRandomInt(min_data->getValue(), max_data->getValue())));
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_rand_float(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &min = tools::getArgOriginData(args[0]);
        const auto &min_data = check_arg_data_type<data::Float>(ins, args[0], min);
        const auto &max = tools::getArgOriginData(args[1]);
        const auto &max_data = check_arg_data_type<data::Float>(ins, args[1], max);
        const auto &decimal_places = tools::getArgOriginData(args[2]);
        const auto &decimal_places_data = check_arg_data_type<data::Int>(ins, args[2], decimal_places);
        data_space_pool.updateDataByNameNoLock(
                args[3].getValue(),
                std::make_shared<data::Float>(
                        utils::getRandomFloat(min_data->getValue(),
                                              max_data->getValue(),
                                              decimal_places_data->getValue()))
        );
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_tp_get(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &op_arg = args[0];
        const auto &target_arg = args[1];
        if (target_arg.getType() != utils::ArgType::identifier && target_arg.getType() != utils::ArgType::keyword) {
            throw base::errors::ArgTypeMismatchError(target_arg.getPos().toString(), ins.raw_code,
                                                  {"The type of the target argument of the " +
                                                   ins.ri.toString() +
                                                   " cannot be an immutable argument type.",
                                                   "Error Arg: " + target_arg.toString()},
                                                  {"Check whether the target data of the " + ins.ri.toString() +
                                                   " instruction is of mutable type.",
                                                   "Mutable argument types include, but are not limited to 'identifier', 'keyword'."});
        }
        auto error_arg = op_arg;
        try {
            std::shared_ptr<base::RVM_Data> type_info;
            switch (op_arg.getType()) {
                case utils::ArgType::number:
                case utils::ArgType::string:
                case utils::ArgType::identifier: {
                    const auto &op_data = tools::getArgOriginData(op_arg);
                    if (op_data->getTypeID().fullEqualWith(data::CustomType::typeId)) {
                        type_info = std::make_shared<data::String>(std::static_pointer_cast<data::CustomType>(op_data)->getTypeIDString());
                    } else if (op_data->getTypeID().fullEqualWith(data::CustomInst::typeId)) {
                        type_info = std::make_shared<data::String>(static_pointer_cast<data::CustomInst>(op_data)->getTypeIDString());
                    } else {
                        type_info = std::make_shared<data::String>(op_data->getTypeID().toString());
                    }
                }
                    break;
                case utils::ArgType::keyword: {
                    if (const auto ri_opt = RI::getRIByStr(op_arg.getValue()); ri_opt.has_value()) {
                        const auto &ri = ri_opt.value();
                        type_info = std::make_shared<data::String>(ri.id.toString());
                    } else {
                        const auto &op_data = tools::getArgOriginData(op_arg);
                        type_info = std::make_shared<data::String>(op_data->getTypeID().toString());
                    }
                }
                    break;
                case utils::ArgType::unknown:
                    type_info = std::make_shared<data::String>(unknown_);
                    break;
            }
            error_arg = target_arg;
            data_space_pool.updateDataByNameNoLock(target_arg.getValue(), type_info);
        } catch ([[maybe_unused]] const base::errors::MemoryError &e) {
            throw base::errors::MemoryError(error_arg.getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_div(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[2].getType() != utils::ArgType::identifier) {
            throw std::runtime_error("Third argument must be an identifier");
            return ExecutionStatus::FailedWithError;
        }
        auto [data1, data2] = [&args]() -> std::pair<std::shared_ptr<data::Numeric>, std::shared_ptr<data::Numeric>> {
            const auto d1 = tools::getArgNewData(args[0]);
            const auto d2 = tools::getArgNewData(args[1]);
            // 提前进行类型检查，避免多次cast
            if (d1->getTypeID() != data::Numeric::typeId ||
                d2->getTypeID() != data::Numeric::typeId) {
                throw std::runtime_error("Both arguments must be numeric");
            }
            return {
                    static_pointer_cast<data::Numeric>(d1),
                    static_pointer_cast<data::Numeric>(d2)
            };
        }();
        try {
            data_space_pool.updateDataByNameNoLock(args[2].getValue(), data1->divide(data2));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[2].getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[2].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_tp_set(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto validateFirstArg = [&]() {
            if (args[0].getType() != utils::ArgType::keyword && args[0].getType() != utils::ArgType::identifier) {
                throw base::errors::ArgTypeMismatchError(
                        args[0].getPos().toString(), ins.raw_code,
                        {"The type of the first argument must be " +
                         getTypeFormatString(utils::ArgType::keyword) + " or " +
                         getTypeFormatString(utils::ArgType::identifier) + ".",
                         "Error Arg: " + args[0].toString()},
                        {"Check whether the first data is of " +
                         getTypeFormatString(utils::ArgType::keyword) + " or " +
                         getTypeFormatString(utils::ArgType::identifier) + "."});
            }
        };

        auto updateDataSpace = [&](const std::string &name, const std::shared_ptr<base::RVM_Data>& newData) {
            data_space_pool.updateDataByNameNoLock(name, newData);
        };

        validateFirstArg();
        auto error_arg = args[0];
        try {
            const auto type = tools::getArgOriginData(args[0]);
            int type_uid;
            if (type->getTypeID().fullEqualWith(data::DType::typeId)) {
                type_uid = static_pointer_cast<data::DType>(type)->getDTypeID().dis_id;
            } else {
                throw base::errors::ArgumentError(error_arg.getPosStr(), ins.raw_code,
                                                  "Error Arg: " + utils::getSpaceFormatString(error_arg.getValue(), type->toString()),
                                                  {"Please check that the argument of this instruction are used correctly.",
                                                   "The " + ins.ri.toString() +
                                                   " requires the first argument to be " + data::DType::typeId.toString() + " type."});
            }
            error_arg = args[1];
            const auto &data = tools::getArgOriginData(args[1]);
            const auto &data_val_str = data->getValStr();
            const auto data_val = utils::stringToNumber(data_val_str == null_ ? "0" : data_val_str);

            static const std::unordered_map<int, std::function<void(const std::shared_ptr<base::RVM_Data> &,
                                                                            const StdArgs &,
                                                                            const utils::Number &)>> typeHandlers = {
                    {data::Int::typeId.dis_id,          [&updateDataSpace](const auto &data, const auto &args,
                                                                               const auto &data_val) {
                        updateDataSpace(args[1].getValue(), std::make_shared<data::Int>(
                                data_val.type == utils::NumType::int_type ?
                                data_val.int_value : static_cast<int>(data_val.double_value)));
                    }},
                    {data::Float::typeId.dis_id,        [&updateDataSpace](const auto &data, const auto &args,
                                                                               const auto &data_val) {
                        updateDataSpace(args[1].getValue(), std::make_shared<data::Float>(
                                data_val.type == utils::NumType::int_type ?
                                static_cast<double>(data_val.int_value) : data_val.double_value));
                    }},
                    {data::String::typeId.dis_id,       [&updateDataSpace](const auto &data, const auto &args,
                                                                               const auto &data_val) {
                        updateDataSpace(args[1].getValue(), std::make_shared<data::String>(
                                data->getValStr() == null_ ? "" : data->getValStr()));
                    }},
                    {data::Bool::typeId.dis_id,         [&updateDataSpace](const auto &data, const auto &args,
                                                                               const auto &data_val) {
                        if (data::String::typeId.fullEqualWith(data->getTypeID())){
                            const auto &data_val_str = data->getValStr();
                            updateDataSpace(args[1].getValue(), std::make_shared<data::Bool>(
                                    utils::stringToBool(data_val_str == null_ ? "0" : data_val_str)));
                        } else {
                            updateDataSpace(args[1].getValue(), std::make_shared<data::Bool>(data->convertToBool()));
                        }
                    }},
                    {data::Char::typeId.dis_id,         [&updateDataSpace](const auto &data, const auto &args,
                                                                               const utils::Number &data_val) {
                        if (data->getTypeID().fullEqualWith(data::Int::typeId) || data->getTypeID().fullEqualWith(data::Bool::typeId)) {
                            updateDataSpace(args[1].getValue(), std::make_shared<data::Char>(
                                    static_cast<char>(static_pointer_cast<data::Int>(data)->getValue())));
                        } else {
                            const auto &data_val_str = data->getValStr();
                            if (data_val.type == utils::NumType::int_type) {
                                updateDataSpace(args[1].getValue(), std::make_shared<data::Char>(
                                        static_cast<char>(data_val.int_value)));
                            } else {
                                updateDataSpace(args[1].getValue(), std::make_shared<data::Char>(
                                        utils::stringToChar(data_val_str == null_ ? "\000" : data_val_str)));
                            }
                        }
                    }},
                    {data::Null::typeId.dis_id,         [&updateDataSpace](const auto &data, const auto &args,
                                                                               const auto &data_val) {
                        updateDataSpace(args[1].getValue(), std::make_shared<data::Null>());
                    }},
                    {data::List::typeId.dis_id,         [&updateDataSpace](const auto &data, const auto &args,
                                                                               const auto &data_val) {
                        const auto data_type = data->getTypeID();
                        if (!data::Null::typeId.fullEqualWith(data_type) && !tools::isIterableData(data_type)) {
                            throw base::errors::ArgTypeMismatchError(unknown_, unknown_, {}, {});
                        }
                        if (data::Null::typeId.fullEqualWith(data_type)) {
                            updateDataSpace(args[1].getValue(), std::make_shared<data::List>());
                        } else if (data::String::typeId.fullEqualWith(data_type)) {
                            updateDataSpace(args[1].getValue(),
                                            std::static_pointer_cast<data::String>(data)->trans_to_list());
                        } else if (data::Series::typeId.fullEqualWith(data_type)) {
                            updateDataSpace(args[1].getValue(),
                                            std::make_shared<data::List>(
                                                    std::static_pointer_cast<data::Series>(data)->getDataList()));
                        }
                    }},
                    {data::Series::typeId.dis_id,       [&updateDataSpace](const auto &data, const auto &args,
                                                                               const auto &data_val) {
                        if (data->getTypeID() != data::Null::typeId && !tools::isIterableData(data->getTypeID())) {
                            throw base::errors::ArgTypeMismatchError(unknown_, unknown_, {}, {});
                        }
                        if (data->getTypeID() == data::Null::typeId) {
                            updateDataSpace(args[1].getValue(), std::make_shared<data::Series>());
                        } else if (data::List::typeId.fullEqualWith(data->getTypeID())) {
                            updateDataSpace(args[1].getValue(),
                                            std::make_shared<data::Series>(std::static_pointer_cast<data::List>(data)));
                        } else if (data::String::typeId.fullEqualWith(data->getTypeID())) {
                            updateDataSpace(args[1].getValue(),
                                            std::make_shared<data::Series>(
                                                    std::static_pointer_cast<data::String>(data)->trans_to_list()));
                        }
                    }},
                    {data::Dict::typeId.dis_id,         [&updateDataSpace](const auto &data, const auto &args,
                                                                               const auto &data_val) {
                        const auto &data_type = data->getTypeID();
                        if (data_type != data::Null::typeId && data_type != data::Dict::typeId &&
                            data_type != data::List::typeId && data_type != data::Series::typeId &&
                            data_type != data::String::typeId) {
                            throw base::errors::DataTypeMismatchError(unknown_, unknown_,
                                {"Error Type: " + data->getTypeID().toString()},
                                {"When the target data type is 'tp-dict', the data to be converted "
                                 "must be of 'tp-null' type or 'tp-list', 'tp-series', 'tp-str'."});
                        }
                        if (data_type == data::Null::typeId) {
                            updateDataSpace(args[1].getValue(), std::make_shared<data::Dict>());
                        } else if (data_type == data::List::typeId) {
                            updateDataSpace(args[1].getValue(), std::make_shared<data::Dict>(
                                    static_pointer_cast<data::List>(data)));
                        } else if (data_type == data::Series::typeId) {
                            updateDataSpace(args[1].getValue(), std::make_shared<data::Dict>(
                                    static_pointer_cast<data::Series>(data)));
                        } else if (data_type == data::String::typeId) {
                            updateDataSpace(args[1].getValue(), std::make_shared<data::Dict>(
                                    static_pointer_cast<data::String>(data)->trans_to_list()));
                        }
                    }},
                    {data::KeyValuePair::typeId.dis_id, [&updateDataSpace](const auto &data, const auto &args,
                                                                               const auto &data_val) {
                        const auto &data_type = data->getTypeID();
                        if (data_type != data::Null::typeId && data_type != data::KeyValuePair::typeId) {
                            throw base::errors::DataTypeMismatchError(unknown_, unknown_,
                                {"Error Type: " + data->getTypeID().toString()},
                                {"When the target data type is 'tp-pair', the data to be converted "
                                 "must be of 'tp-null' type or 'tp-pair'."});
                        }
                        if (data_type == data::Null::typeId) {
                            updateDataSpace(args[1].getValue(), std::make_shared<data::KeyValuePair>());
                        }
                    }},
                    {data::Time::typeId.dis_id,          [&updateDataSpace](const auto &data, const auto &args,
                                                                                const auto &data_val){
                        if (data->getTypeID() != data::Null::typeId && data->getTypeID() != data::Time::typeId
                        && data->getTypeID() != data::String::typeId) {
                            throw base::errors::ArgTypeMismatchError(unknown_, unknown_, {}, {});
                        }
                        if (data->getTypeID() == data::Null::typeId) {
                            updateDataSpace(args[1].getValue(), std::make_shared<data::Time>());
                        } else if (data->getTypeID() == data::String::typeId) {
                            updateDataSpace(args[1].getValue(),
                                            std::make_shared<data::Time>(data::Time::fromString(
                                                    data->getValStr())));
                        }
                    }}
            };

            if (const auto it = typeHandlers.find(type_uid);
                it != typeHandlers.end()) {
                it->second(data, args, data_val);
            } else {
                throw base::errors::ArgTypeMismatchError(args[0].getPos().toString(), ins.raw_code, {}, {});
            }
        } catch (const base::errors::MemoryError &) {
            throw base::errors::MemoryError(
                    error_arg.getPos().toString(), ins.raw_code,
                    {"This error is caused by accessing memory space that does not exist.",
                     "Nonexistent Space Name: " + error_arg.toString()},
                    {"Use the " + ris::ALLOT.toString() +
                     " to manually allocate a named memory space before accessing it."});
        } catch (base::errors::ArgTypeMismatchError &e) {
            e.error_position = error_arg.getPosStr();
            e.error_line = ins.raw_code;
            e.error_info.insert(e.error_info.begin() + 1, "Error arg: " + utils::getSpaceFormatString(error_arg.getValue(),
                    tools::getArgOriginDataNoError(error_arg)->toString()));
            throw;
        }

        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_tp_def(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args.size() > 2 || args.size() < 1) {
            throw base::errors::ArgumentNumberError(args[0].getPosStr(), ins.raw_code, "1 / 2", args.size(),
                                                    ins.ri.toString(), {});
        }
        auto tp_name = args[0].getValue();
        if (const auto arg_data = tools::getArgOriginDataNoError(args[0])) {
            if (arg_data->getTypeID() != data::String::typeId) {
                throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                      {"Error Arg: " + utils::getSpaceFormatString(
                                                              args[0].getValue(), arg_data->toString()),
                                                       "Target Type: " + data::String::typeId.toString()},
                                                      {"The target type of " + ins.ri.toString() + " is " +
                                                       data::String::typeId.toString() + "."});
            }
            tp_name = arg_data->getValStr();
        }
        if (base::containsKeywordInCategory(base::KeywordType::DataTypes, tp_name)) {
            throw base::errors::DuplicateKeyError(args[0].getPosStr(), ins.raw_code,
                                                  {"This error is caused by defining a data type with the duplicate name.",
                                                   "Duplicate Name: tp-" + tp_name},
                                                  {"Use a different data type name."});
        }
        std::shared_ptr<base::RVM_Data> parent_type = nullptr;
        if (args.size() == 2) {
            parent_type = tools::getArgOriginData(args[1]);
            if (parent_type->getTypeID() != data::CustomType::typeId) {
                throw base::errors::ArgTypeMismatchError(args[1].getPosStr(), ins.raw_code,
                                                      {"Error Arg: " + args[1].toString(),
                                                       "Target Type: " + data::CustomType::typeId.toString()},
                                                      {"The target argument type of the " +
                                                       ins.ri.toString() + " can only be the " +
                                                       data::CustomType::typeId.toString() + "."});
            }
        }
        auto new_data_type = std::make_shared<data::CustomType>(tp_name,
                                                                parent_type
                                                                ? static_pointer_cast<data::CustomType>(parent_type)
                                                                : nullptr);
        const auto &_ = data_space_pool.addData(tp_name, new_data_type);
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_tp_new(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &type_data = tools::getArgOriginData(args[0]);
        if (type_data->getTypeID() != data::CustomType::typeId) {
            throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                  {"Error Arg: " + args[0].toString(),
                                                   "Target Type: " + data::CustomType::typeId.toString()},
                                                  {"The target argument type of the " +
                                                   ins.ri.toString() + " can only be the " +
                                                   data::CustomType::typeId.toString()
                                                  });
        }
        auto type = static_pointer_cast<data::CustomType>(type_data);
        auto new_inst = std::make_shared<data::CustomInst>(type);
        try{
            data_space_pool.updateDataByNameNoLock(args[1].getValue(), new_inst);
        } catch (const base::errors::MemoryError &) {
            throw base::errors::MemoryError(
                    args[1].getPosStr(), ins.raw_code,
                    {"This error is caused by accessing memory space that does not exist.",
                     "Nonexistent Space Name: " + args[1].toString()},
                    {"Use the " + ris::ALLOT.toString() +
                     " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_tp_add_tp_field(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args.size() > 3 || args.size() < 2) {
            throw base::errors::ArgumentNumberError(args[0].getPosStr(), ins.raw_code, "2 / 3", args.size(),
                                                    ins.ri.toString(), {});
        }
        auto arg_data = tools::getArgOriginData(args[0]);
        if (arg_data->getTypeID() != data::CustomType::typeId) {
            throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                  {"Error Arg: " + args[0].toString(),
                                                   "Target Type: " + data::CustomType::typeId.toString()},
                                                  {"The target argument type of the " +
                                                   ins.ri.toString() + " can only be the " +
                                                   data::CustomType::typeId.toString() + "."});
        }
        const auto ct = static_pointer_cast<data::CustomType>(arg_data);
        auto field_name = args[1].getValue();
        arg_data = tools::getArgOriginDataNoError(args[1]);
        if (arg_data) {
            if (arg_data->getTypeID() != data::String::typeId) {
                throw base::errors::ArgTypeMismatchError(args[1].getPosStr(), ins.raw_code,
                                                      {"Error Space: " + utils::getSpaceFormatString(field_name,
                                                                                                     arg_data->toString()),
                                                       "Target Type: " + data::String::typeId.toString()},
                                                      {"The target argument type of the " +
                                                       ins.ri.toString() + " can only be the " +
                                                       data::String::typeId.toString() + "."});
            }
            field_name = arg_data->getValStr();
        }
        std::shared_ptr<base::RVM_Data> field_data;
        if (args.size() == 3) {
            field_data = tools::getArgOriginData(args[2]);
        }
        try {
            ct->addTpField(field_name, field_data);
        } catch ([[maybe_unused]] base::errors::DuplicateKeyError &e) {
            throw base::errors::DuplicateKeyError(args[1].getPosStr(), ins.raw_code,
                                                  {"This error is caused by defining a data type field with the duplicate name.",
                                                   "Duplicate Name: " + field_name},
                                                  {"Use a different data type field name."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_tp_add_inst_field(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args.size() > 3 || args.size() < 2) {
            throw base::errors::ArgumentNumberError(args[0].getPosStr(), ins.raw_code, "2 / 3", args.size(),
                                                    ins.ri.toString(), {});
        }
        auto field_name = args[1].getValue();
        auto arg_data = tools::getArgOriginDataNoError(args[1]);
        if (arg_data) {
            if (arg_data->getTypeID() != data::String::typeId) {
                throw base::errors::ArgTypeMismatchError(args[1].getPosStr(), ins.raw_code,
                                                      {"Error Space: " + utils::getSpaceFormatString(field_name,
                                                                                                     arg_data->toString()),
                                                       "Target Type: " + data::String::typeId.toString()},
                                                      {"The target argument type of the " +
                                                       ins.ri.toString() + " can only be the " +
                                                       data::String::typeId.toString() + "."});
            }
            field_name = arg_data->getValStr();
        }
        arg_data = tools::getArgOriginData(args[0]);
        std::shared_ptr<base::RVM_Data> field_data = nullptr;
        int error_index = 0;
        try {
            if (args.size() == 3) {
                error_index = 2;
                field_data = tools::getArgOriginData(args[2]);
            }

            if (arg_data->getTypeID() == data::CustomType::typeId) {
                static_pointer_cast<data::CustomType>(arg_data)->addInstField(field_name, field_data);
            } else {
                throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                      {"Error Arg: " + args[0].toString(),
                                                       "Target Type: " + data::CustomType::typeId.toString()},
                                                      {"The target argument type of the " +
                                                       ins.ri.toString() + " can only be the " +
                                                       data::CustomType::typeId.toString() + "."});
            }
        } catch ([[maybe_unused]] base::errors::DuplicateKeyError &e) {
            throw base::errors::DuplicateKeyError(args[1].getPosStr(), ins.raw_code,
                                                  {"This error is caused by defining a data type field with the duplicate name.",
                                                   "Duplicate Name: " + field_name},
                                                  {"Use a different data type field name."});
        } catch ([[maybe_unused]] base::errors::MemoryError &e) {
            throw base::errors::MemoryError(args[error_index].getPosStr(), ins.raw_code,
                                            {"This error is caused by using an non-existent memory space.",
                                             "Error Space: " + utils::getSpaceFormatString(args[error_index].getValue(),
                                                                                           arg_data->toString())},
                                            { "Check the memory space provided for " +
                                                         ins.ri.toString() + " has been allocate by " + ris::ALLOT.toString() + "."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_tp_set_field(const Ins &ins, size_t &pointer, const StdArgs &args) {
        int current_arg_index = 0;  // 新增：跟踪当前访问的参数索引

        // 处理字段名称参数(args[1])
        current_arg_index = 1;
        auto field_name = args[current_arg_index].getValue();
        auto arg_data = tools::getArgOriginDataNoError(args[current_arg_index]);
        if (arg_data) {
            if (arg_data->getTypeID() != data::String::typeId) {
                throw base::errors::ArgTypeMismatchError(args[current_arg_index].getPosStr(), ins.raw_code,
                                                      {"Error Space: " + utils::getSpaceFormatString(field_name,
                                                                                                     arg_data->toString()),
                                                       "Target Type: " + data::String::typeId.toString()},
                                                      {"The target argument type of the " +
                                                       ins.ri.toString() + " can only be the " +
                                                       data::String::typeId.toString() + "."});
            }
            field_name = arg_data->getValStr();
        }

        // 处理目标对象参数(args[0])
        current_arg_index = 0;
        arg_data = tools::getArgOriginData(args[current_arg_index]);
        try {
            // 处理字段值参数(args[2])
            current_arg_index = 2;
            auto field_data = tools::getArgOriginData(args[current_arg_index]);
            if (!tools::isMutableData(field_data->getTypeID())) {
                field_data = field_data->copy_ptr();
            }

            // 设置字段值
            if (arg_data->getTypeID() == data::CustomType::typeId) {
                static_pointer_cast<data::CustomType>(arg_data)->setTpField(field_name, field_data);
            } else if (arg_data->getTypeID() == data::CustomInst::typeId) {
                static_pointer_cast<data::CustomInst>(arg_data)->setField(field_name, field_data);
            } else {
                throw base::errors::ArgTypeMismatchError(args[current_arg_index].getPosStr(), ins.raw_code,
                                                      {"Error Arg: " + args[0].toString(),
                                                       "Target Type: " + data::CustomType::typeId.toString() +
                                                       " / " + data::CustomInst::typeId.toString()},
                                                      {"The target argument type of the " +
                                                       ins.ri.toString() + " can only be the " +
                                                       data::CustomType::typeId.toString() + " / " +
                                                       data::CustomInst::typeId.toString() + "."});
            }
        } catch (const base::errors::FieldNotFoundError &_) {
            // 使用当前参数索引精确定位错误
            throw base::errors::FieldNotFoundError(args[current_arg_index].getPosStr(), ins.raw_code,
                                                   arg_data->toString(), field_name,
                                                   {"Please ensure that the accessed fields have been defined by the " +
                                                    ris::TP_ADD_INST_FIELD.toString() + " or " +
                                                    ris::TP_ADD_TP_FIELD.toString() + "."});
        } catch ([[maybe_unused]] const base::errors::MemoryError &e) {
            // 使用当前参数索引精确定位错误
            throw base::errors::MemoryError(args[current_arg_index].getPosStr(), ins.raw_code,
                                            {"This error is caused by using an non-existent memory space.",
                                             "Error Arg: " + args[current_arg_index].toString()},
                                            {"Check the memory space provided for " +
                                             ins.ri.toString() + " has been allocated by " + ris::ALLOT.toString() +
                                             "."});
        } catch (const std::exception &e) {
            // 捕获其他异常时也使用当前参数索引
            throw base::RVM_Error(base::ErrorType::RuntimeError, args[current_arg_index].getPosStr(), ins.raw_code,
                                             {"Unexpected error occurred while processing argument",
                                              "Error Arg: " + args[current_arg_index].toString(),
                                              "Error Details: " + std::string(e.what())},
                                    {"Have no repair tips."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_tp_get_field(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[1];
        try {
            auto field_name = args[1].getValue();
            auto arg_data = tools::getArgOriginDataNoError(args[1]);
            if (arg_data) {
                if (arg_data->getTypeID() != data::String::typeId) {
                    throw base::errors::ArgTypeMismatchError(args[1].getPosStr(), ins.raw_code,
                                                          {"Error Space: " + utils::getSpaceFormatString(field_name,
                                                                                                         arg_data->toString()),
                                                           "Target Type: " + data::String::typeId.toString()},
                                                          {"The target argument type of the " +
                                                           ins.ri.toString() + " can only be the " +
                                                           data::String::typeId.toString() + "."});
                }
                field_name = arg_data->getValStr();
            }
            error_arg = args[0];
            arg_data = tools::getArgOriginData(args[0]);
            error_arg = args[1];
            std::shared_ptr<base::RVM_Data> field_data;
            if (arg_data->getTypeID().fullEqualWith(data::CustomType::typeId)) {
                field_data = static_pointer_cast<data::CustomType>(arg_data)->getTpField(field_name);
            } else if (arg_data->getTypeID().fullEqualWith(data::CustomInst::typeId)) {
                field_data = static_pointer_cast<data::CustomInst>(arg_data)->getField(field_name);
            } else {
                throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                      {"Error Data Space: " +
                                                      utils::getSpaceFormatString(args[0].getValue(),
                                                                                  arg_data->toString()),
                                                       "Target Type: " + data::CustomType::typeId.toString() +
                                                       " or " + data::CustomInst::typeId.toString()},
                                                      {"The target argument type of the " +
                                                       ins.ri.toString() + " can only be the " +
                                                       data::CustomType::typeId.toString() + " or " +
                                                       data::CustomInst::typeId.toString() + "."});
            }
            error_arg = args[2];
            data_space_pool.updateDataByNameNoLock(args[2].getValue(), field_data);
        } catch (const base::errors::MemoryError &_){
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by the getting field was not exist.",
                                             "Nonexistent Field Name: " + utils::getSpaceFormatString(
                                                     error_arg.getValue(),
                                                     tools::getArgOriginData(error_arg)->toString())},
                                            {"Please use " + ris::TP_ADD_TP_FIELD.toString() + " or " +
                                            ris::TP_ADD_INST_FIELD.toString() + " directive adds field properties"
                                                                                " to a custom type or instance."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_tp_get_super_field(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[0];
        try {
            // First, get the parent type parameter (new parameter)
            error_arg = args[1];
            auto parent_type_name = args[1].getValue();
            auto arg_data = tools::getArgOriginData(args[1]);
            if (!arg_data->getTypeID().fullEqualWith(data::CustomType::typeId)) {
                throw base::errors::ArgTypeMismatchError(args[1].getPosStr(), ins.raw_code,
                                                      {"Error Space: " + utils::getSpaceFormatString(parent_type_name,
                                                                                                     arg_data->toString()),
                                                       "Target Type: " + data::CustomType::typeId.toString()},
                                                      {"The parent type argument of the " + ins.ri.toString() +
                                                       " can only be the " +
                                                       data::CustomType::typeId.toString() + "."});
            }
            const auto &parent_data = static_pointer_cast<data::CustomType>(arg_data);
            // Then get the field name parameter (now args[2] instead of args[1])
            error_arg = args[2];
            auto field_name = args[2].getValue();
            arg_data = tools::getArgOriginDataNoError(args[2]);
            if (arg_data) {
                if (arg_data->getTypeID() != data::String::typeId) {
                    throw base::errors::ArgTypeMismatchError(args[2].getPosStr(), ins.raw_code,
                                                          {"Error Space: " + utils::getSpaceFormatString(field_name,
                                                                                                         arg_data->toString()),
                                                           "Target Type: " + data::String::typeId.toString()},
                                                          {"The field name argument of the " +
                                                           ins.ri.toString() + " can only be the " +
                                                           data::String::typeId.toString() + "."});
                }
                field_name = arg_data->getValStr();
            }

            // Get the target object (args[0] remains the same)
            error_arg = args[0];
            arg_data = tools::getArgOriginData(args[0]);

            error_arg = args[2]; // Field name is now at args[2]
            std::shared_ptr<base::RVM_Data> field_data;

            if (arg_data->getTypeID() == data::CustomType::typeId) {
                throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                      {"Error Data Space: " +
                                                       utils::getSpaceFormatString(args[0].getValue(),
                                                                                   arg_data->toString()),
                                                       "Target Type: " + data::CustomInst::typeId.toString()},
                                                     {"The target argument type of the " + ins.ri.toString() + " can only be the " +
                                                      data::CustomInst::typeId.toString() + "."});
            } else if (arg_data->getTypeID() == data::CustomInst::typeId) {
                const auto &inst_data = static_pointer_cast<data::CustomInst>(arg_data);
                field_data = inst_data->getField(field_name, parent_data);
            } else {
                throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                      {"Error Data Space: " +
                                                       utils::getSpaceFormatString(args[0].getValue(),
                                                                                   arg_data->toString()),
                                                       "Target Type: " + data::CustomType::typeId.toString() +
                                                       " or " + data::CustomInst::typeId.toString()},
                                                      {"The target argument type of the " +
                                                       ins.ri.toString() + " can only be the " +
                                                       data::CustomType::typeId.toString() + " or " +
                                                       data::CustomInst::typeId.toString() + "."});
            }

            // Store target is now args[3] instead of args[2]
            error_arg = args[3];
            data_space_pool.updateDataByNameNoLock(args[3].getValue(), field_data);
        } catch ([[maybe_unused]] const base::errors::MemoryError &e) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by the getting field was not exist.",
                                             "Nonexistent Field Name: '" + error_arg.getValue() + "'"},
                                            {"Please use " + ris::TP_ADD_TP_FIELD.toString() + " or " +
                                             ris::TP_ADD_INST_FIELD.toString() + " directive adds field properties"
                                                                                 " to a custom type or instance."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_tp_derive(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_index = 0;
        try {
            const auto &inst = tools::getArgOriginData(args[0]);
            if (!inst->getTypeID().fullEqualWith(data::CustomInst::typeId)) {
                throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                      {"Error Arg: " + args[0].toString(),
                                                       "Expected Type: " + data::CustomInst::typeId.toString()},
                                                      {"Check whether the first data of the " + ins.ri.toString() +
                                                       " is of " + data::CustomInst::typeId.toString() + "."});
            }
            const auto &inst_data = static_pointer_cast<data::CustomInst>(inst);
            error_index = 1;
            const auto &derive_type = tools::getArgOriginData(args[1]);
            if (!derive_type->getTypeID().fullEqualWith(data::CustomType::typeId)) {
                throw base::errors::ArgTypeMismatchError(args[1].getPosStr(), ins.raw_code,
                                                      {"Error Arg: " + args[1].toString(),
                                                       "Expected Type: " + data::CustomType::typeId.toString()},
                                                      {"Check whether the second data of the " + ins.ri.toString() +
                                                       " is of " + data::CustomType::typeId.toString() + "."});
            }
            inst_data->derivedToChildType(static_pointer_cast<data::CustomType>(derive_type));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[error_index].getPosStr(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: '" + args[error_index].getValue() + "'"},
                                            { "Please use " + ris::ALLOT.toString() +
                                            " directive to allocate memory spaces before manipulating it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_sp_get(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[0].getType() != utils::ArgType::keyword && args[0].getType() != utils::ArgType::identifier) {
            throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                  {"Error Arg: " + args[0].toString(),
                                                   "Expected Type: " +
                                                   getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " + getTypeFormatString(utils::ArgType::identifier)},
                                                  {"Check whether the first data of the " + ins.ri.toString() +
                                                   " is of " + getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " +
                                                   getTypeFormatString(utils::ArgType::identifier) + "."});
        }
        auto scopeName = data_space_pool.getCurrentScopeNoLock()->getName();
        try {
            data_space_pool.updateDataByNameNoLock(args[0].getValue(),
                                                   std::make_shared<data::String>(scopeName));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[0].getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[0].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_sp_set(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[0].getType() != utils::ArgType::keyword && args[0].getType() != utils::ArgType::identifier) {
            throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                  {"Error Arg: " + args[0].toString(),
                                                   "Expected Type: " +
                                                   getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " + getTypeFormatString(utils::ArgType::identifier)},
                                                  {"Check whether the first data of the " + ins.ri.toString() +
                                                   " is of " + getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " +
                                                   getTypeFormatString(utils::ArgType::identifier) + "."});
        }
        auto data = tools::getArgOriginData(args[0]);
        try {
            data_space_pool.setCurrentScopeByName(data->getValStr());
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[0].getPos().toString(), ins.raw_code,
                                            {"This error is caused by the fact that the target scope of the " +
                                             ins.ri.toString() + " does not exist.",
                                             "Nonexistent Scope ID: " + args[0].toString()},
                                            {"Before setting the scope, ensure that the ID of the target scope "
                                             "exists and is correct."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_sp_new(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[0].getType() != utils::ArgType::keyword && args[0].getType() != utils::ArgType::identifier) {
            throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                  {"Error Arg: " + args[0].toString(),
                                                   "Expected Type: " +
                                                   getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " + getTypeFormatString(utils::ArgType::identifier)},
                                                  {"Check whether the first data of the " + ins.ri.toString() +
                                                   " is of " + getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " +
                                                   getTypeFormatString(utils::ArgType::identifier) + "."});
        }
        try {
            auto new_scope = data_space_pool.acquireScope(pre_SP_NEW);
            data_space_pool.updateDataByNameNoLock(args[0].getValue(),
                                                   std::make_shared<data::String>(
                                                           new_scope->getInstID().toString()));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[0].getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[0].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_sp_del(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[0].getType() != utils::ArgType::keyword && args[0].getType() != utils::ArgType::identifier) {
            throw base::errors::ArgTypeMismatchError(args[0].getPosStr(), ins.raw_code,
                                                  {"Error Arg: " + args[0].toString(),
                                                   "Expected Type: " +
                                                   getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " + getTypeFormatString(utils::ArgType::identifier)},
                                                  {"Check whether the first data of the " + ins.ri.toString() +
                                                   " is of " + getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " +
                                                   getTypeFormatString(utils::ArgType::identifier) + "."});
        }
        try {
            auto data = tools::getArgOriginData(args[0]);
            data_space_pool.releaseScope(data_space_pool.findScopeByNameNoLock(data->getValStr()));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[0].getPos().toString(), ins.raw_code,
                                            {"This error is caused by deleting memory space that does not exist.",
                                             "Nonexistent Scope ID: " + args[0].toString()},
                                            {"Use the " + ris::SP_GET.toString() + " or " + ris::SP_NEW.toString() +
                                             " to get a scope ID before manipulate it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_qot(const Ins &ins, size_t &pointer, const StdArgs &args) {
        static auto throw_error = [&ins](const utils::Arg &error_arg) {
            throw base::errors::ArgTypeMismatchError(error_arg.getPosStr(), ins.raw_code,
                                                  {"Error Arg: " + error_arg.toString(),
                                                   "Expected Type: " +
                                                   getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " + getTypeFormatString(utils::ArgType::identifier)},
                                                  {"Check whether the first data of the " + ins.ri.toString() +
                                                   " is of " + getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " +
                                                   getTypeFormatString(utils::ArgType::identifier) + "."});
        };
        if (args[0].getType() != utils::ArgType::keyword && args[0].getType() != utils::ArgType::identifier) {
            throw_error(args[0]);
        }
        if (args[1].getType() != utils::ArgType::keyword && args[1].getType() != utils::ArgType::identifier) {
            throw_error(args[1]);
        }
        auto error_arg = args[0];
        try {
            auto [id, data] = data_space_pool.findDataByNameNoLock(args[0].getValue());
            if (!data) {
                throw base::errors::MemoryError(unknown_, unknown_, {}, {});
            }
            error_arg = args[1];
            data_space_pool.updateDataByNameNoLock(args[1].getValue(),
                                                   std::make_shared<data::Quote>(id));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(error_arg.getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_qot_val(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[1].getType() != utils::ArgType::keyword && args[1].getType() != utils::ArgType::identifier) {
            throw base::errors::ArgTypeMismatchError(args[1].getPosStr(), ins.raw_code,
                                                  {"Error Arg: " + args[1].toString(),
                                                   "Expected Type: " +
                                                   getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " + getTypeFormatString(utils::ArgType::identifier)},
                                                  {"Check whether the second data of the " + ins.ri.toString() +
                                                   " is of " + getTypeFormatString(utils::ArgType::keyword) +
                                                   " or " +
                                                   getTypeFormatString(utils::ArgType::identifier) + "."});
        }

        try {
            auto data = tools::getArgNewData(args[0]);
            auto [id, quote_data] = data_space_pool.findDataByNameNoLock(args[1].getValue());
            if (!quote_data) {
                throw base::errors::MemoryError(unknown_, unknown_, {}, {});
            }
            if (quote_data->getTypeID() != data::Quote::typeId) {
                throw base::errors::ArgTypeMismatchError(args[1].getPosStr(), ins.raw_code,
                                                      {"Error Arg: " + args[1].toString(),
                                                       "Expected Type: " + data::Quote::typeId.toString()},
                                                      {"Check whether the first data of the " +
                                                       ins.ri.toString() + " is of " +
                                                       data::Quote::typeId.toString()
                                                       + "."});
            }
            static_pointer_cast<data::Quote>(quote_data)->updateQuoteData(data);
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[1].getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[1].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_ret(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args.size() > 1) {
            throw base::errors::ArgumentNumberError(
                args[args.size() - 1].getPosStr(), ins.raw_code, "0 / 1",
                args.size(), ins.ri.toString(), {});
        }
        try {
            if (args.size() == 1) {
                data_space_pool.updateDataNoLock(SR_SpaceID, tools::getArgNewData(args[0]));
            }
            return ExecutionStatus::AbortedFunction;
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[0].getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[0].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_pow(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[2].getType() != utils::ArgType::identifier && args[2].getType() != utils::ArgType::keyword) {
            throw base::errors::ArgTypeMismatchError(args[2].getPos().toString(), ins.raw_code,
                                                  {"Error Arg: " + args[2].toString(),
                                                   "Expected Type: " +
                                                   getTypeFormatString(utils::ArgType::identifier) +
                                                   " or " + getTypeFormatString(utils::ArgType::keyword)},
                                                  {"Check the type of the target argument of the " +
                                                   ins.ri.toString() + "."});
            return ExecutionStatus::FailedWithError;
        }
        const auto &data1 = tools::getArgNewData(args[0]);
        const auto &data2 = tools::getArgNewData(args[1]);
        if (!tools::isNumericData(data1)) {
            throw base::errors::ArgTypeMismatchError(args[0].getPos().toString(), ins.raw_code,
                                                  {"Error Data: " + utils::getSpaceFormatString(args[0].getValue(),
                                                                                                data1->toString()),
                                                   "Expected Type: " + data::Numeric::typeId.toString()},
                                                  {"Check the type of the arguments of the " +
                                                   ins.ri.toString() +
                                                   "."});
        }
        if (!tools::isNumericData(data2)) {
            throw base::errors::ArgTypeMismatchError(args[1].getPos().toString(), ins.raw_code,
                                                  {"Error Data: " + utils::getSpaceFormatString(args[1].getValue(),
                                                                                                data2->toString()),
                                                   "Expected Type: " + data::Numeric::typeId.toString()},
                                                  {"Check the type of the arguments of the" +
                                                   ins.ri.toString() +
                                                   "."});
        }
        try {
            data_space_pool.updateDataByNameNoLock(args[2].getValue(),
                                                   static_pointer_cast<data::Numeric>(data1)->pow(
                                                           static_pointer_cast<data::Numeric>(data2)));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[2].getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[2].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_root(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[2].getType() != utils::ArgType::identifier && args[2].getType() != utils::ArgType::keyword) {
            throw base::errors::ArgTypeMismatchError(args[2].getPos().toString(), ins.raw_code,
                                                  {"Error Arg: " + args[2].toString(),
                                                   "Expected Type: " +
                                                   getTypeFormatString(utils::ArgType::identifier) +
                                                   " or " + getTypeFormatString(utils::ArgType::keyword)},
                                                  {"Check the type of the target argument of the " +
                                                   ins.ri.toString() + "."});
            return ExecutionStatus::FailedWithError;
        }
        const auto &data1 = tools::getArgNewData(args[0]);
        const auto &data2 = tools::getArgNewData(args[1]);
        if (!tools::isNumericData(data1)) {
            throw base::errors::ArgTypeMismatchError(args[0].getPos().toString(), ins.raw_code,
                                                  {"Error Data: " + utils::getSpaceFormatString(args[0].getValue(),
                                                                                                data1->toString()),
                                                   "Expected Type: " + data::Numeric::typeId.toString()},
                                                  {"Check the type of the arguments of the " +
                                                   ins.ri.toString() +
                                                   "."});
        }
        if (!tools::isNumericData(data2)) {
            throw base::errors::ArgTypeMismatchError(args[1].getPos().toString(), ins.raw_code,
                                                  {"Error Data: " + utils::getSpaceFormatString(args[1].getValue(),
                                                                                                data2->toString()),
                                                   "Expected Type: " + data::Numeric::typeId.toString()},
                                                  {"Check the type of the arguments of the" +
                                                   ins.ri.toString() +
                                                   "."});
        }
        try {
            data_space_pool.updateDataByNameNoLock(args[2].getValue(),
                                                   static_pointer_cast<data::Numeric>(data1)->root(
                                                           static_pointer_cast<data::Numeric>(data2)));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[2].getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[2].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_mod(const Ins &ins, size_t &pointer, const StdArgs &args) {
        if (args[2].getType() != utils::ArgType::identifier && args[2].getType() != utils::ArgType::keyword) {
            throw base::errors::ArgTypeMismatchError(args[2].getPos().toString(), ins.raw_code,
                                                  {"Error Arg: " + args[2].toString(),
                                                   "Expected Type: " +
                                                   getTypeFormatString(utils::ArgType::identifier) +
                                                   " or " + getTypeFormatString(utils::ArgType::keyword)},
                                                  {"Check the type of the target argument of the " +
                                                   ins.ri.toString() + "."});
            return ExecutionStatus::FailedWithError;
        }
        const auto &data1 = tools::getArgNewData(args[0]);
        const auto &data2 = tools::getArgNewData(args[1]);
        if (!tools::isNumericData(data1)) {
            throw base::errors::ArgTypeMismatchError(args[0].getPos().toString(), ins.raw_code,
                                                  {"Error Data: " + utils::getSpaceFormatString(args[0].getValue(),
                                                                                                data1->toString()),
                                                   "Expected Type: " + data::Numeric::typeId.toString()},
                                                  {"Check the type of the arguments of the " +
                                                   ins.ri.toString() +
                                                   "."});
        }
        if (!tools::isNumericData(data2)) {
            throw base::errors::ArgTypeMismatchError(args[1].getPos().toString(), ins.raw_code,
                                                  {"Error Data: " + utils::getSpaceFormatString(args[1].getValue(),
                                                                                                data2->toString()),
                                                   "Expected Type: " + data::Numeric::typeId.toString()},
                                                  {"Check the type of the arguments of the" +
                                                   ins.ri.toString() +
                                                   "."});
        }
        try {
            data_space_pool.updateDataByNameNoLock(args[2].getValue(),
                                                   static_pointer_cast<data::Numeric>(data1)->mod(
                                                           static_pointer_cast<data::Numeric>(data2)));
        } catch (const base::errors::MemoryError &_) {
            throw base::errors::MemoryError(args[2].getPos().toString(), ins.raw_code,
                                            {"This error is caused by accessing memory space that does not exist.",
                                             "Nonexistent Space Name: " + args[2].toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before accessing it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_atmp(const Ins &ins, size_t &pointer, const StdArgs &args) {
        // 初始化必要的变量
        std::shared_ptr<InsSet> atmp_block_ins_set = std::make_shared<InsSet>("ATMP");
        std::shared_ptr<InsSet> finally_ins_set = std::make_shared<InsSet>("FINALLY");
        std::vector<std::shared_ptr<DetectBlock>> detect_blocks;
        bool has_detect = false;

        // 遍历指令集
        for (const auto &inner_ins: ins.scopeInsSet->getInsSet()) {
            if (inner_ins->ri == ris::DETECT) {
                // 如果是 DETECT 指令
                if (!has_detect) {
                    has_detect = true; // 标记已进入 DETECT 块
                }

                // 检查参数类型是否合法
                const auto &arg0 = inner_ins->args[0];
                if (arg0.getType() != utils::ArgType::identifier && arg0.getType() != utils::ArgType::keyword) {
                    throw base::errors::ArgTypeMismatchError(
                            arg0.getPos().toString(), ins.raw_code,
                            {"Error Arg: " + arg0.toString(),
                             "Expected Type: " + getTypeFormatString(utils::ArgType::identifier) +
                             " or " + getTypeFormatString(utils::ArgType::keyword)},
                            {"Check the type of the target argument of the " + ris::DETECT.toString() + "."});
                }

                // 获取检测类型并创建 DetectBlock
                auto detect_type = tools::getArgOriginData(inner_ins->args[0]);
                if (detect_type->getTypeID() != data::CustomType::typeId) {
                    throw base::errors::ArgTypeMismatchError(
                            arg0.getPos().toString(), ins.raw_code,
                            {"Error Data: " + utils::getSpaceFormatString(arg0.getValue(),
                                                                          detect_type->toString()),
                             "Expected Type: " + data::CustomType::typeId.toString()},
                            {"Check the type of the target argument of the " + ris::DETECT.toString() + "."});
                }
                detect_blocks.emplace_back(std::make_shared<DetectBlock>(
                        static_pointer_cast<data::CustomType>(detect_type),
                        inner_ins->args[1], inner_ins->scopeInsSet));
            } else if (!has_detect) {
                // 如果尚未进入 DETECT 块，将指令添加到 atmp_block_ins_set
                atmp_block_ins_set->addIns(inner_ins);
            } else {
                // 如果已进入 DETECT 块，将指令添加到 finally_ins_set
                finally_ins_set->addIns(inner_ins);
            }
        }

        // 创建 AtmpBlock 并执行
        const auto &atmp_block = std::make_shared<AtmpBlock>(detect_blocks, atmp_block_ins_set, finally_ins_set);
        InsSet::atmp_stack.push_back(atmp_block);
        atmp_block_ins_set->setScopeLeader(ins);
        data_space_pool.acquireScope(atmp_block->block_ins_set->scope_prefix);
        const auto &exe_res = atmp_block_ins_set->execute();
        finally_ins_set->execute();
        InsSet::atmp_stack.pop_back();
        if (atmp_block->is_hit) {
            const auto detect_scope = data_space_pool.acquireScope(
                    atmp_block->hit_detect_block->block_ins_set->scope_prefix);
            data_space_pool.addData(atmp_block->hit_detect_block->error_stored_arg.getValue(),
                                    atmp_block->hit_detect_block->error_data);
            return atmp_block->hit_detect_block->block_ins_set->execute();
        }
        return exe_res;
    }

    ExecutionStatus ri_expose(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &argData = tools::getArgOriginData(args[0]);
        if (argData->getTypeID() != data::CustomInst::typeId) {
            throw base::errors::ArgTypeMismatchError(args[0].getPos().toString(), ins.raw_code,
                                                  {"Error Data: " + utils::getSpaceFormatString(args[0].getValue(),
                                                                                                argData->toString()),
                                                   "Expected Type: " + data::CustomInst::typeId.toString()},
                                                  {"Check the type of the arguments of the " +
                                                   ins.ri.toString() + "."});
        }
        auto expose_inst = static_pointer_cast<data::CustomInst>(argData);
        auto error_data = std::make_shared<data::Error>(
                ins.pos.toString(), ins.raw_code,
                std::vector<std::string>{expose_inst->getValStr()});
        data_space_pool.updateDataByNameNoLock("SE", error_data);
        data_space_pool.updateDataByNameNoLock("_SE", error_data);
        for (auto atmp = InsSet::atmp_stack.rbegin(); atmp != InsSet::atmp_stack.rend(); ++atmp) {
            for (auto &detect_block: (*atmp)->detect_blocks) {
                if (expose_inst->customType->checkBelongTo(detect_block->detected_type)) {
                    detect_block->error_data = expose_inst;
                    (*atmp)->is_hit = true;
                    (*atmp)->hit_detect_block = detect_block;
                    ri_exit(ins, pointer, args);
                    return ExecutionStatus::ExposedError;
                }
            }
        }
        return ExecutionStatus::ExposedError;
    }

    ExecutionStatus ri_loadin(const Ins &ins, size_t &pointer, const StdArgs &args) {
        // 检查参数数量是否合法
        if (args.empty() || args.size() > 2) {
            throw base::errors::ArgumentNumberError(
                    ins.pos.toString(), ins.raw_code, "1 / 2",
                    static_cast<int>(args.size()), ins.ri.toString(), {});
        }

        // 获取并验证第一个参数（文件路径）的类型
        auto filePathData = tools::getArgOriginData(args[0]);
        if (filePathData->getTypeID() != data::String::typeId) {
            throw base::errors::ArgTypeMismatchError(args[0].getPos().toString(), ins.raw_code,
                                                  {"Error Data: " + utils::getSpaceFormatString(args[0].getValue(),
                                                                                                filePathData->toString()),
                                                   "Expected Type: " + data::String::typeId.toString()},
                                                  {"Check the type of the arguments of the " + ins.ri.toString() + "."});
        }

        // 构建完整文件路径
        const std::string& filePath = utils::getAbsolutePath(
                static_pointer_cast<data::String>(filePathData)->getValStr(),
                base::PROGRAM_WORKING_DIRECTORY_STACK.top());

        // 检查是否提供了扩展存储参数
        const bool hasExtensionStore = (args.size() == 2);
        std::shared_ptr<base::RVM_Data> extensionData = nullptr;
        if (hasExtensionStore) {
            extensionData = tools::getArgOriginDataNoError(args[1]);
        }

        // 检查文件扩展名是否已存在
        auto [extensionExists, extensionUnicPath] = tools::checkExtensionExist(filePath);

        if (extensionExists) {
            // 如果扩展名已存在且提供了存储参数，则更新数据
            if (hasExtensionStore) {
                const auto& existingExtension = memory::loadedExtensions[extensionUnicPath];
                if (!extensionData) {
                    const auto &_ = data_space_pool.addGlobalData(args[1].getValue(), existingExtension);
                } else {
                    data_space_pool.updateDataByNameNoLock(args[1].getValue(), existingExtension);
                }
            }
            return ExecutionStatus::Success;
        }

        // 创建新的扩展并添加到数据池
        std::shared_ptr<data::Extension> newExtension = nullptr;
        if (hasExtensionStore) {
            newExtension = std::make_shared<data::Extension>(args[1].getValue(), filePath);
            if (!extensionData) {
                const auto &_ = data_space_pool.addGlobalData(args[1].getValue(), newExtension);
            } else {
                data_space_pool.updateDataByNameNoLock(args[1].getValue(), newExtension);
            }
        }

        // 将新扩展添加到已加载扩展列表
        memory::loadedExtensions.emplace(extensionUnicPath, newExtension);

        try {
            // 解析并执行文件中的代码
            const auto& instructionSet = parser::parse::parseCodeFromPath(
                    filePath, false,
                    hasExtensionStore ? args[1].getValue() : "");
            instructionSet->execute();

            // 如果提供了存储参数，则移除当前作用域
            if (hasExtensionStore) {
                data_space_pool.removeScopeNoLock(newExtension->extension_scope);
            }
        } catch ([[maybe_unused]] const base::errors::FileReadError &e) {
            // 重新抛出文件读取错误并添加更多上下文信息
            throw base::errors::FileReadError(ins.pos.toString(), ins.raw_code,
                                              {"Error file path: " + utils::getEscapedPathFormatString(filePath),
                                               "Program entry directory: " + utils::getEscapedPathFormatString(base::PROGRAM_WORKING_DIRECTORY_STACK.top())},
                                              {"Please check if the provided file exists or is correct."});
        }

        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_link(const Ins &ins, size_t &pointer, const StdArgs &args) {
        throw std::runtime_error(ins.ri.toString() + "unable to be executed directly.");
        return ExecutionStatus::FailedWithError;
    }

    ExecutionStatus ri_exe_rasm(const Ins &ins, size_t &pointer, const StdArgs &args) {
        std::string rasm_str;
        for (const auto &arg: args){
            rasm_str += tools::getArgOriginData(arg)->getValStr();
        }
        try{
            parser::parse::parseCode(parser::parse::ParseType::Rasm, ins.pos.getFilepath(), rasm_str,
                                     false, ins.pos, "")->execute();
        }catch (base::RVM_Error &e){
            e.error_position = args[args.size() - 1].getPosStr();
            throw;
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_ext_get(const Ins &ins, size_t &pointer, const StdArgs &args) {
        auto error_arg = args[0];
        try {
            const auto &ext = tools::getArgOriginData(args[0]);
            error_arg = args[1];
            const auto &field = tools::getArgOriginData(args[1]);

            tools::checkArgumentDataTypeAndRet<data::Extension>(ext, ins, args[0], 1);
            tools::checkArgumentDataTypeAndRet<data::String>(field, ins, args[1], 2);

            const auto &res_data = static_pointer_cast<data::Extension>(ext)->extension_scope
                    ->findDataByName(field->getValStr());

            error_arg = args[2];
            data_space_pool.updateDataByNameNoLock(
                    args[2].getValue(), res_data);
        } catch (const base::errors::MemoryError&) {
            throw base::errors::MemoryError(error_arg.getPosStr(), ins.raw_code,
                                            {"This error is caused by manipulating memory space that does not exist.",
                                             "Nonexistent Space Name: " + error_arg.toString()},
                                            {"Use the " + ris::ALLOT.toString() +
                                             " to manually allocate a named memory space before manipulating it."});
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_pair_set(const Ins &ins, size_t &pointer, const StdArgs &args) {
        // ToDo: 注意错误处理
        auto error_arg = args[0];
        tools::checkArgumentType(ins, args[2], 3,
            {utils::ArgType::identifier, utils::ArgType::keyword});
        const auto &key_data = tools::getArgNewData(args[0]);
        const auto &value_data = tools::getArgOriginData(args[1]);
        const auto &pair_data = std::make_shared<data::KeyValuePair>(key_data, value_data);
        if (const auto &target_data = tools::getArgOriginData(args[2]);
            !target_data->updateData(pair_data)) {
            data_space_pool.updateDataByNameNoLock(args[2].getValue(), pair_data);
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_pair_set_key(const Ins &ins, size_t &pointer, const StdArgs &args) {
        // ToDo: 注意错误处理
        auto error_arg = args[0];
        tools::checkArgumentType(ins, args[1], 2,
            {utils::ArgType::identifier, utils::ArgType::keyword});
        const auto &key_data = tools::getArgNewData(args[0]);
        const auto &pair_data = tools::checkArgumentDataTypeAndRet<data::KeyValuePair>(
                tools::getArgOriginData(args[1]), ins, args[1], 2, true);
        pair_data->setKey(key_data);
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_pair_set_value(const Ins &ins, size_t &pointer, const StdArgs &args) {
        // ToDo: 注意错误处理
        auto error_arg = args[0];
        tools::checkArgumentType(ins, args[1], 2,
            {utils::ArgType::identifier, utils::ArgType::keyword});
        const auto &value_data = tools::getArgOriginData(args[0]);
        const auto &pair_data = tools::checkArgumentDataTypeAndRet<data::KeyValuePair>(
                tools::getArgOriginData(args[1]), ins, args[1], 2, true);
        pair_data->setValue(value_data);
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_pair_get_key(const Ins &ins, size_t &pointer, const StdArgs &args) {
        tools::checkArgumentType(ins, args[1], 2, {
            utils::ArgType::identifier, utils::ArgType::keyword
        });
        const auto &pair_data =
            tools::checkArgumentDataTypeAndRet<data::KeyValuePair>(
                tools::getArgOriginData(args[0]),
                ins, args[0], 1, true);
        if (! tools::getArgOriginData(args[1])->updateData(pair_data->getKey())) {
            data_space_pool.updateDataByNameNoLock(args[1].getValue(), pair_data->getKey());
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_pair_get_value(const Ins &ins, size_t &pointer, const StdArgs &args) {
        tools::checkArgumentType(ins, args[1], 2, {
            utils::ArgType::identifier, utils::ArgType::keyword
        });
        const auto &pair_data =
            tools::checkArgumentDataTypeAndRet<data::KeyValuePair>(
                tools::getArgOriginData(args[0]),
                ins, args[0], 1, true);
        if (! tools::getArgOriginData(args[1])->updateData(pair_data->getValue())) {
            data_space_pool.updateDataByNameNoLock(args[1].getValue(), pair_data->getValue());
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_dict_set(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &dict_data = tools::checkArgumentDataTypeAndRet<data::Dict>(
                tools::getArgOriginData(args[0]),
                ins, args[0], 1, true);
        const auto &key_data = tools::getArgNewData(args[1]);
        dict_data->setDataAt(key_data->toEscapedString(),
            std::make_shared<data::KeyValuePair>(key_data,
                tools::getArgOriginData(args[2])));
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_dict_get(const Ins &ins, size_t &pointer, const StdArgs &args) {
        tools::checkArgumentType(ins, args[1], 2,
            {utils::ArgType::identifier, utils::ArgType::keyword});
        const auto &dict_data = tools::checkArgumentDataTypeAndRet<data::Dict>(
                tools::getArgOriginData(args[0]),
                ins, args[0], 1, true);
        const auto &key_data = tools::getArgNewData(args[1]);
        if (const auto &value_data = dict_data->getDataAt(key_data->toEscapedString());
            !tools::getArgOriginData(args[1])->updateData(value_data)) {
            data_space_pool.updateDataByNameNoLock(args[1].getValue(), value_data);
        }
        return ExecutionStatus::Success;
    }

    ExecutionStatus ri_dict_del(const Ins &ins, size_t &pointer, const StdArgs &args) {
        const auto &dict_data =
            tools::checkArgumentDataTypeAndRet<data::Dict>(
                tools::getArgOriginData(args[0]),
            ins, args[0], 1, true);
        for (size_t i = 1; i < args.size(); ++i) {
            dict_data->eraseDataAt(tools::getArgOriginData(args[i])->toEscapedString());
        }
        return ExecutionStatus::Success;
    }

}