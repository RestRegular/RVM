//
// Created by RestRegular on 2025/1/15.
//

#include <iostream>
#include <chrono>
#include "ra_base.h"
#include "ra_core.h"
#include "ra_parser.h"
#include "ra_utils.h"

using namespace base;
using namespace core;
using namespace parser;
using namespace utils;

const auto &PROGRAM_RVM_WORKING_DIRECTORY = utils::getWindowsRVMDir();
const auto &PROGRAM_DEFAULT_WORKING_DIRECTORY = utils::getWindowsDefaultDir();

utils::ProgArgParser argParser{};

std::string __program_target_file_path__;
std::string __program_archive_file_path__;
std::string __program_help_option_name__;
std::string __program_working_directory__;
std::string __program_output_redirect__;
bool __program_run_executable__;
bool __program_serialize__;
bool __program_debug_option__;
bool __program_run_time_info_option__;
bool __program_help_option__;
bool __program_version_option__;
bool __program_vs_check_option__;
bool __program_rvm_work_directory__;

void initializeArgumentParser() {
    argParser.addFlag("help", &__program_help_option__, false, true,
                      "Displays this help message and exits. "
                      "Use this flag to show a list of available options, flags, and their descriptions.",
                      {"h"});
    argParser.addFlag("version", &__program_version_option__, false, true,
                      "Displays the version information and exits. "
                      "Use this flag to check the current version of the program.",
                      {"v"});
    argParser.addFlag("vs-check", &__program_vs_check_option__, false, true,
                      "Displays the program version and exits. "
                      "Use this flag to check the current program version "
                      "and the version of the RSI file, whose path must be "
                      "specified using the target option.",
                      {"vc"});
    argParser.addFlag("run", &__program_run_executable__, false, true,
                      "Runs the specified executable file. "
                      "This flag is used to execute the target file provided via the '--target' option.",
                      {"r"});
    argParser.addFlag("comp", &__program_serialize__, false, true,
                      "Compiles the specified source file. "
                      "This flag is used to compile the target file provided via the '--target' option "
                      "and generate the output file specified by the '--archive' option.",
                      {"c"});
    argParser.addFlag("time-info", &__program_run_time_info_option__, false, true,
                      "Enables timing information during execution. "
                      "This flag outputs detailed timing metrics for the program's execution, "
                      "useful for performance analysis and optimization.",
                      {"ti"});
    argParser.addFlag("debug", &__program_debug_option__, false, true,
                      "Enables debug mode. "
                      "This flag activates additional debugging information and tools "
                      "to assist in diagnosing issues during execution or compilation.",
                      {"d", "db"});
    argParser.addFlag("rvm-work-directory", &__program_rvm_work_directory__, false, true,
                      "This flag is used to specify the rvm directory as the program's working directory.",
                      {"rwd"});
    argParser.addFlag("precomp-link", &parser::env::precompiled_link_option, false, true,
                      "This flag is used to specify the path to the precompiled RSI file. "
                      "When this flag is used, the program will attempt to link the specified RSI file "
                      "with the target file, which must be specified using the '--precomp-link-dir' option.",
                      {"pcl", "pl"});

    argParser.addOption<std::string>("help-option", &__program_help_option_name__,
                                                     "",
                                                     "Specifies the name of the help option. "
                                                     "This option is used to customize the name of the help option displayed in the help message.",
                                                     {"ho"});
    argParser.addOption<utils::SerializationProfile>("comp-level", &parser::env::__program_serialization_profile__,
                                                     SerializationProfile::Debug,
                                                     "Specifies the compilation level for the operation. "
                                                     "Determines the optimization and debugging information included in the output. "
                                                     "For example, 'Debug' includes additional debugging information, while other profiles may optimize for performance or size.",
                                                     {"cl"});
    argParser.addOption<std::string>("target", &__program_target_file_path__,
                                     R"(D:\ClionProjects\RVM\test\test.ra)",
                                     "Specifies the target file path for the operation. "
                                     "When the operation is '--run', it refers to the executable file path. "
                                     "When the operation is '--comp', it refers to the source file to be compiled.",
                                     {"tar", "t"});
    argParser.addOption<std::string>("archive", &__program_archive_file_path__,
                                     R"(D:\ClionProjects\RVM\test\test.rsi)",
                                     "Specifies the output file path for the generated file. "
                                     "When the operation is '--comp', it refers to the path where the compiled file will be saved.",
                                     {"arc", "a"});
    argParser.addOption<std::string>("working-dir", &__program_working_directory__,
                                     PROGRAM_DEFAULT_WORKING_DIRECTORY,
                                     "Specifies the working directory for the operation. ",
                                     {"wd"});
    argParser.addOption<std::string>("output-redirect", &__program_output_redirect__,
                                     "console",
                                     "Specifies the output redirect place for the operation.",
                                     {"or"});
    argParser.addOption<std::string>("precomp-link-dir", &parser::env::precompiled_link_dir_path,
                                     R"(D:\ClionProjects\RVM\lib)",
                                     "Specifies the path to the precompiled RSI file. "
                                     "When the '--precomp-link' flag is used, this option specifies the path to the precompiled RSI file.",
                                     {"pcld", "pld"});

    // 互斥选项：运行与编译互斥
    argParser.addMutuallyExclusive("run", "comp", utils::ProgArgParser::CheckDir::BiDir);
    // 互斥选项：工作目录 与 rvm工作目录互斥
    argParser.addMutuallyExclusive("rvm-work-directory", "work-directory",
                                   utils::ProgArgParser::CheckDir::BiDir);
    // 互斥选项：帮助、版本、版本检查 与 其他操作所有选项互斥
    argParser.addMutuallyExclusive(std::vector<std::string>{"help", "version"},
                                   std::vector<std::string>{"run", "comp", "debug",
                                                            "target", "comp-level", "archive",
                                                            "vs-check", "working-dir", "time-info"},
                                   utils::ProgArgParser::CheckDir::BiDir);
    // 互斥选项：帮助、版本、版本检查 两两互斥
    argParser.addMutuallyExclusive("help", std::vector<std::string>{"version", "vs-check"},
                                   utils::ProgArgParser::CheckDir::BiDir);
    argParser.addMutuallyExclusive("version", std::vector<std::string>{"help", "vs-check"},
                                   utils::ProgArgParser::CheckDir::BiDir);
    argParser.addMutuallyExclusive("vs-check", std::vector<std::string>{"run", "comp", "debug",
                                                                        "comp-level", "archive", "time-info"},
                                   utils::ProgArgParser::CheckDir::BiDir);
    // 关联选项：帮助选项 与 帮助 单向关联
    argParser.addDependent("help-option", "help", utils::ProgArgParser::CheckDir::UniDir);
    // 关联选项：运行、编译 与 目标选项 单向关联
    argParser.addDependent(std::vector<std::string>{"run", "comp"}, "target",
                           utils::ProgArgParser::CheckDir::UniDir);
    // 关联选项：运行 与 调试选项 单向关联
    argParser.addDependent("debug", "run", utils::ProgArgParser::CheckDir::UniDir);
    // 关联选项：版本检查 与 目标选项 单向关联
    argParser.addDependent("vs-check", "target",
                           utils::ProgArgParser::CheckDir::UniDir);
}

int main(int argc, char *argv[]){
    try {
        initializeArgumentParser();
        argParser.parse(argc, argv);
        utils::setProgramDir(__program_rvm_work_directory__ ?
                             PROGRAM_RVM_WORKING_DIRECTORY : __program_working_directory__);
        if (parser::env::precompiled_link_option) {
            parser::env::precompiled_link_dir_path = utils::getAbsolutePath(parser::env::precompiled_link_dir_path);
        }
        if (__program_output_redirect__ != "console") {
            io->setOutputToConsole(false);
            io->setOutputToFile(__program_output_redirect__);
        }
        if (__program_debug_option__) {
            if (__program_output_redirect__ != "console") {
                throw std::runtime_error("Cannot redirect output to file when debug mode is enabled.");
            }
            InsSet::debug_mode = DebugMode::Standard;
        }
        if (__program_serialize__) {
            auto file_ext = utils::getFileExtFromPath(__program_target_file_path__);
            env::initialize(__program_target_file_path__);
            auto ins_set = parse::parseCodeFromPath(__program_target_file_path__);
            parse::serializeExecutableInsToBinaryFile(__program_archive_file_path__, ins_set,
                                                      parser::env::__program_serialization_profile__);
            *io << "[RVM Msg] >> Compilation finished with ["
                << getSerializationProfileName(parser::env::__program_serialization_profile__) << "] profile.";
        }
        if (__program_run_executable__) {
            auto file_ext = utils::getFileExtFromPath(__program_target_file_path__);
            if (file_ext == RSI_EXT) {
                auto ins_set = parse::deserializeExecutableInsFromBinaryFile(
                        __program_target_file_path__, parser::env::__program_serialization_profile__);
                env::initialize(ins_set->scope_leader_file);
                ins_set->execute();
            } else {
                auto ins_set = parse::parseCodeFromPath(__program_target_file_path__);
                env::initialize(ins_set->scope_leader_file);
                ins_set->execute();
            }
        }
        if (__program_help_option__) {
            if (__program_help_option_name__.empty()) {
                *io << argParser.getHelpString();
            } else {
                *io << argParser.getOptionDescription(__program_help_option_name__);
            }
        }
        if (__program_version_option__) {
            *io << "[RVM Msg] >> " << rvm_serial_header.getRVMVersionInfo();
        }
        if (__program_vs_check_option__) {
            auto file_ext = utils::getFileExtFromPath(__program_target_file_path__);
            try {
                const auto &binary_file_path = utils::getAbsolutePath(__program_target_file_path__);
                std::ifstream binary_file(binary_file_path, std::ios::binary);
                base::rvm_serial_header.deserialize(binary_file, parser::env::__program_serialization_profile__,
                                                    binary_file_path, false);
                env::deserializeLinkedExtensions(binary_file);
                binary_file.close();
                auto version_info = rvm_serial_header.getRSIVersionInfo(__program_target_file_path__);
                if (version_info.empty()) {
                    *io += "[RVM Error] >> No version information found in the target RSI file.";
                } else {
                    *io << "[RVM Msg] >> " + version_info;
                }
            } catch (const std::exception &e) {
                *io += "[RVM Error] >> " +
                       utils::StringManager::wrapText(
                               "Failed to retrieve the version information from the target RSI file. "
                               "This may be caused by file corruption or the use of a compilation mode that excludes "
                               "version details during the build process.", 80, 15);
                *io += "\n[RVM Error] >> " + std::string(e.what());
            }
        }
        if (__program_run_time_info_option__) {
            *io << utils::printProgramSpentTimeInfo();
        }
    } catch (base::RVM_Error &rvm_error) {
        if (base::rvm_serial_header.getProfile() < SerializationProfile::Release) {
            *io += rvm_error.toString();
        } else {
            *io += rvm_error.getErrorTitle() + rvm_error.getErrorInfo();
        }
    } catch (std::exception &e) {
        *io += "[RVM Error] >> ";
        *io += e.what();
    }
    io->flushOutputCache();
    return 0;
}
