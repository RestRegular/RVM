//
// Created by RestRegular on 2025/1/16.
//

#include <iostream>
#include <utility>
#include <stack>
#include <filesystem>
#include "ra_parser.h"
#include "ra_base.h"
#include "ra_core.h"
#include "ra_utils.h"

using namespace core;

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
        utils::SerializationProfile __program_serialization_profile__ = utils::SerializationProfile::Debug;

        bool precompiled_link_option {false};

        std::string precompiled_link_dir_path = R"(D:\ClionProjects\RVM\lib)";

        std::unordered_set<std::string> linked_extensions {};

        bool has_initial = false;

        void initialize(const std::string &executed_file_path) {
            if (has_initial) {
                data_space_pool.reset();
            }
            has_initial = true;
            auto [SR_id, _] = data_space_pool.addGlobalData("SR", std::make_shared<data::Null>());
            store_SR_id = SR_id;
            data_space_pool.addGlobalDataBatch({
                    {"SN", std::make_shared<data::Null>()},
                    {"SE", std::make_shared<data::Null>()},
                    {"SS", std::make_shared<data::Null>()},
                    {"true", std::make_shared<data::Bool>(true)},
                    {"false", std::make_shared<data::Bool>(false)},
                    {"null", std::make_shared<data::Null>()},
                    {"tp-int", std::make_shared<data::String>(data::Int::typeId.toString())},
                    {"tp-float", std::make_shared<data::String>(data::Float::typeId.toString())},
                    {"tp-char", std::make_shared<data::String>(data::Char::typeId.toString())},
                    {"tp-bool", std::make_shared<data::String>(data::Bool::typeId.toString())},
                    {"tp-str", std::make_shared<data::String>(data::String::typeId.toString())},
                    {"tp-null", std::make_shared<data::String>(data::Null::typeId.toString())},
                    {"tp-list", std::make_shared<data::String>(data::List::typeId.toString())},
                    {"tp-dict", std::make_shared<data::String>(data::Dict::typeId.toString())},
                    {"tp-series", std::make_shared<data::String>(data::Series::typeId.toString())},
                    {"tp-pair",std::make_shared<data::String>(data::KeyValuePair::typeId.toString())},
                    {"tp-time", std::make_shared<data::String>(data::Time::typeId.toString())}
            });
            const auto &main_space_name = utils::getFileNameFromPath(executed_file_path);
            data_space_pool.acquireScope(pre_SRC, main_space_name);
            data_space_pool.main_scope_name = pre_SRC + main_space_name;
        }

        void addIns(const utils::Pos &pos, const std::string &raw_code, const RI &ri, const StdArgs &args) {
            components::INS_SET->addIns(std::make_shared<Ins>(pos, raw_code, ri, args));
        }

        void serializeLinkedExtensions(std::ostream &out) {
            size_t extension_count = linked_extensions.size();
            out.write(reinterpret_cast<const char *>(&extension_count), sizeof(extension_count));
            for (const auto &extension: linked_extensions) {
                size_t extension_size = extension.size();
                out.write(reinterpret_cast<const char *>(&extension_size), sizeof(extension_size));
                if (extension_size > 0){
                    out.write(extension.c_str(), extension_size);
                }
            }
        }

        void deserializeLinkedExtensions(std::istream &in) {
            size_t extension_count;
            in.read(reinterpret_cast<char *>(&extension_count), sizeof(extension_count));
            for (size_t i = 0; i < extension_count; ++i) {
                size_t extension_size;
                in.read(reinterpret_cast<char *>(&extension_size), sizeof(extension_size));
                if (extension_size > 0){
                    std::string extension;
                    extension.resize(extension_size);
                    in.read(&extension[0], extension_size);
                    linked_extensions.insert(extension);
                }
            }
        }

    }

    namespace parse {
        std::tuple<std::vector<std::string>, std::vector<size_t>, std::vector<size_t>>
        preprocessCode(std::string &code) {
            utils::addCharBeforeNewline(code);
            std::vector<std::string> lines = utils::StringManager::split(code, '\n');
            std::vector<std::string> processedLines;
            std::string currentLine;
            std::vector<size_t> lineRawIndex;
            std::vector<size_t> lineIndents;
            size_t rawIndex = 0;
            for (const std::string &line: lines) {
                std::string trimmedLine = line;
                utils::StringManager::trim(trimmedLine);
                if (trimmedLine.empty()) {
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
                        currentLine = utils::StringManager::combineNearbyString(currentLine, rawIndex);
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
                        if (i > 0 && currentLine[i - 1] == '\\') {
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
                currentLine = utils::StringManager::combineNearbyString(currentLine, rawIndex);
                processedLines.push_back(currentLine);
                lineIndents.push_back(utils::getSpaceFrontOfLineCode(lines[rawIndex - 1]));
                lineRawIndex.push_back(rawIndex);
            }

            return {processedLines, lineRawIndex, lineIndents};
        }

        std::shared_ptr<InsSet> parseCode(const std::string &type, std::string code_path,
                                          std::string &code, bool output_info,
                                          const std::optional<utils::Pos> &fixed_pos) {
            code_path = utils::getAbsolutePath(code_path);
            const auto &code_name = id::DataID(utils::getFileFromPath(code_path)).toFullString();
            std::stack<std::shared_ptr<InsSet>> insSetStack{};
            insSetStack.push(ins_set_pool.acquire(pre_SRC));
            insSetStack.top()->setScopeLeader(utils::getObjectFormatString(type, code_name));
            insSetStack.top()->setScopeLeaderPos(fixed_pos ? fixed_pos->toString() :
                "\"" + code_path + ":1:1\", line 1", code_path);
            auto [lines, rawIndex, indents] = preprocessCode(code);
            int lineNumber = 1;
            for (const std::string &line: lines) {
                size_t colonPos = line.find(':');
                if (colonPos == std::string::npos) {
                    throw base::RVM_Error(base::ErrorType::SyntaxError,
                                          code_path + ":" + std::to_string(lineNumber) + ":1", line,
                                          {"This error is caused by missing colon in the line."},
                                          {"Please add the missing colon at the appropriate place in this line of code."});
                    lineNumber++;
                    continue;
                }

                std::string instruction = line.substr(0, colonPos);
                std::string parametersStr = line.substr(colonPos + 1);

                if (!utils::isValidIdentifier(instruction)) {
                    std::cerr << "Error: Invalid instruction in line: " << lineNumber << " - " << line << std::endl;
                    lineNumber++;
                    continue;
                }

                std::vector<std::string> rawParameters = utils::StringManager::split(parametersStr, ',');
                std::vector<utils::Arg> args;
                int paramStartColumn = indents[lineNumber - 1] + int(colonPos) + 2;
                for (std::string &param: rawParameters) {
                    const int &orig_param_length = int(param.length());
                    utils::StringManager::trim(param);

                    utils::Pos argPos;

                    if (!fixed_pos) {
                        argPos = utils::Pos(rawIndex[lineNumber - 1], paramStartColumn + orig_param_length, code_path);
                    } else {
                        argPos = fixed_pos.value();
                    }
                    utils::Arg arg(argPos, param);

                    if (!utils::isValidParameter(param)) {
                        throw base::RVM_Error(base::ErrorType::SyntaxError, argPos.toString(), line,
                                              {"This error is caused by providing an argument of the wrong type.",
                                               "Error Arg: " + arg.toString()},
                                              {"Argument types include only 'identifier', 'string', 'number', 'keyword'."});
                        lineNumber++;
                        continue;
                    }

                    args.push_back(arg);

                    paramStartColumn += orig_param_length + 1;
                }

                utils::Pos insPos;
                if (!fixed_pos){
                    insPos = utils::Pos(rawIndex[lineNumber - 1], 1, code_path);
                } else {
                    insPos = fixed_pos.value();
                }

                const auto ri_opt = RI::getRIByStr(instruction);
                bool is_success = ri_opt.has_value();
                if (!is_success) {
                    throw base::RVM_Error(base::ErrorType::SyntaxError, insPos.toString(), line,
                                          {"This error is caused by using an undefined instruction.",
                                           "Unknown RI: [RI: " + instruction + "]"},
                                          {"Please refer to the official instruction documentation for help."});
                }
                const auto &ri = ri_opt.value();

                auto ins_ptr = std::make_shared<Ins>(insPos, line, ri, args);
                auto &topInsSet = insSetStack.top();

                if (ri.hasScope) {
                    topInsSet->addIns(ins_ptr);
                    insSetStack.push(ins_ptr->scopeInsSet);
                    insSetStack.top()->scope_prefix = ri.name + "-";
                    insSetStack.top()->is_delayed_release_scope = ri.isDelayedReleaseScope;
                    insSetStack.top()->setScopeLeader(ins_ptr->raw_code);
                    insSetStack.top()->setScopeLeaderPos(ins_ptr->pos);
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
                        topInsSet->end_pointer = int(topInsSet->getInsSet().size());
                        topInsSet->setLabel(topInsSet->scope_prefix + end_label + suf_END, topInsSet->end_pointer);
                        topInsSet->addIns(ins_ptr);
                        insSetStack.pop();
                    } else {
                        std::cerr << "Error: END instruction without matching scope in line: " << lineNumber
                                  << std::endl;
                    }
                    ins_ptr->is_delayed_release_scope = topInsSet->is_delayed_release_scope;
                } else if (instruction == "SET") {
                    topInsSet->setLabel(args[0].getValue(), int(topInsSet->getInsSet().size()));
                    topInsSet->addIns(ins_ptr);
                } else if (instruction == "LINK") {
                    try {
                        auto linked_file_path = tools::getArgOriginData(args[0])->getValStr();
                        const auto &[link_name, link_ext] = utils::getFileInfoFromPath(linked_file_path);
                        const auto &linked_ins_set = getLinkedInsSet(*ins_ptr.get());
                        if (env::precompiled_link_option && link_ext == RA_EXT) {
                            std::filesystem::path dirPath(env::precompiled_link_dir_path);
                            std::filesystem::path filePath(link_name + RSI_EXT_);
                            std::filesystem::path fullPath = dirPath / filePath;
                            serializeExecutableInsToBinaryFile(fullPath.string(),
                                                               getLinkedInsSet(*ins_ptr.get(),
                                                                               false),
                                                               env::__program_serialization_profile__);
                        }
                        if (linked_ins_set) {
                            topInsSet->insertInsSet(linked_ins_set);
                        } else {
                            throw base::RVM_Error(base::ErrorType::LinkError, insPos.toString(), line,
                                                  {"This error is caused by an error encountered while statically linking other files.",
                                                   "Error Linking File: " +
                                                   utils::getObjectFormatString("File", ins_ptr->args[0].getValue())},
                                                  {"The " + ins_ptr->ri.toString() +
                                                   " requires a argument to specify the path of the file to be linked. ",
                                                   "Only 'RA' and 'RSI' files are supported."});
                        }
                    } catch (const base::errors::ArgumentError &_) {
                        throw base::errors::ArgumentError(ins_ptr->args[0].getPos().toString(), ins_ptr->raw_code,
                                                          "Error Arg: " +
                                                          utils::getSpaceFormatString(ins_ptr->args[0].getValue(),
                                                                                      tools::getArgOriginData(
                                                                                              ins_ptr->args[0])->toString()),
                                                          {"Please check that the argument of this instruction are used correctly.",
                                                           "The " + ins_ptr->ri.toString() +
                                                           " requires an argument to specify the path of the file to be linked. ",
                                                           "Only 'RA' and 'RSI' files are supported."});
                    }
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
                lineNumber++;
            }
            if (insSetStack.size() != 1) {
                throw std::runtime_error("Unclosed scope.");
            }
            return insSetStack.top();
        }

        std::shared_ptr<InsSet> parseCodeFromPath(const std::string &code_file_path, bool output_info) {
            auto code = utils::readFile(code_file_path);
            return parseCode("File", code_file_path, code, output_info);
        }

        void serializeExecutableInsToBinaryFile(std::string binary_file_path,
                                                const std::shared_ptr<InsSet> &ins_set,
                                                const utils::SerializationProfile &profile){
            binary_file_path = utils::getAbsolutePath(binary_file_path);
            std::ofstream binary_file(binary_file_path, std::ios::binary);
            base::rvm_serial_header.setProfile(profile);
            base::rvm_serial_header.serialize(binary_file);
            env::serializeLinkedExtensions(binary_file);
            ins_set->serialize(binary_file, profile);
            binary_file.close();
        }

        std::shared_ptr<InsSet> deserializeExecutableInsFromBinaryFile(std::string binary_file_path, const utils::SerializationProfile &profile){
            binary_file_path = utils::getAbsolutePath(binary_file_path);
            std::ifstream binary_file(binary_file_path, std::ios::binary);
            base::rvm_serial_header.deserialize(binary_file, profile, binary_file_path);
            env::deserializeLinkedExtensions(binary_file);
            auto ins_set = std::make_shared<InsSet>("File");
            ins_set->deserialize(binary_file, base::rvm_serial_header.getProfile());
            binary_file.close();
            return ins_set;
        }

        std::shared_ptr<InsSet> getLinkedInsSet(const Ins &link_ins, bool check_exist) {
            if (link_ins.ri != ris::LINK){
                throw std::runtime_error("The instruction is not a link instruction.");
            }
            const auto &link_path = utils::getAbsolutePath(tools::getArgOriginData(link_ins.args[0])->getValStr());
            const auto &link_file_type = utils::getFileExtFromPath(link_path);
            if (check_exist){
                const auto &check_result = tools::check_extension_exist(link_path);
                if (check_result.first) {
                    return nullptr;
                }
                env::linked_extensions.insert(check_result.second);
            }
            if (link_file_type == "ra") {
                return parse::parseCodeFromPath(link_path, false);
            } else if (link_file_type == "rsi") {
                return parse::deserializeExecutableInsFromBinaryFile(link_path);
            } else {
                throw base::errors::ArgumentError(unknown_, unknown_, unknown_, {});
            }
        }
    }
}
