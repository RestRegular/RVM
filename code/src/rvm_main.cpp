//
// Created by RestRegular on 2025/1/15.
//

#include <iostream>
#include <chrono>
#include "../include/ra_base.h"
#include "../include/ra_core.h"
#include "../include/ra_parser.h"
#include "../include/lib/ra_utils.h"
#include "../include/lib/newrcc.h"

using namespace base;
using namespace core;
using namespace parser;
using namespace utils;

ProgArgParser argParser{};

std::string program_target_file_path_;
std::string program_archive_file_path_;
std::string program_help_option_name_;
std::string program_environment_directory_;
std::string program_output_redirect_;
std::string program_working_directory_;
bool program_run_executable_;
bool program_serialize_;
bool program_debug_option_;
bool program_run_time_info_option_;
bool program_help_option_;
bool program_version_option_;
bool program_vs_check_option_;
bool program_rvm_work_directory_;

void initializeArgumentParser() {
    argParser.addFlag("help", &program_help_option_, false, true,
                      "Displays this help message and exits. "
                      "Use this flag to show a list of available options, flags, and their descriptions.",
                      {"h"});
    argParser.addFlag("version", &program_version_option_, false, true,
                      "Displays the version information and exits. "
                      "Use this flag to check the current version of the program.",
                      {"v"});
    argParser.addFlag("vs-check", &program_vs_check_option_, false, true,
                      "Displays the program version and exits. "
                      "Use this flag to check the current program version "
                      "and the version of the RSI file, whose path must be "
                      "specified using the target option.",
                      {"vc"});
    argParser.addFlag("run", &program_run_executable_, false, true,
                      "Runs the specified executable file. "
                      "This flag is used to execute the target file provided via the '--target' option.",
                      {"r"});
    argParser.addFlag("comp", &program_serialize_, false, true,
                      "Compiles the specified source file. "
                      "This flag is used to compile the target file provided via the '--target' option "
                      "and generate the output file specified by the '--archive' option.",
                      {"c"});
    argParser.addFlag("time-info", &program_run_time_info_option_, false, true,
                      "Enables timing information during execution. "
                      "This flag outputs detailed timing metrics for the program's execution, "
                      "useful for performance analysis and optimization.",
                      {"ti"});
    argParser.addFlag("debug", &program_debug_option_, false, true,
                      "Enables debug mode. "
                      "This flag activates additional debugging information and tools "
                      "to assist in diagnosing issues during execution or compilation.",
                      {"d", "db"});
    argParser.addFlag("rvm-work-directory", &program_rvm_work_directory_, false, true,
                      "This flag is used to specify the rvm directory as the program's environment working directory.",
                      {"rwd"});
    argParser.addFlag("precomp-link", &env::precompiled_link_option, false, true,
                      "This flag is used to mark whether to pre-compile the statically linked files in the source "
                      "code into the RSI file. When using this flag, the '--precomp-link-dir' option is required to specify "
                      "the folder path where the pre-compiled files are stored.",
                      {"pcl", "pl"});

    argParser.addOption<std::string>("help-option", &program_help_option_name_,
                                                     "",
                                                     "Specifies the name of the help option. "
                                                     "This option is used to customize the name of the help option displayed in the help message.",
                                                     {"ho"});
    argParser.addOption<SerializationProfile>("comp-level", &env::__program_serialization_profile__,
                                                     SerializationProfile::Debug,
                                                     "Specifies the compilation level for the operation. "
                                                     "Determines the optimization and debugging information included in the output. "
                                                     "For example, 'Debug' includes additional debugging information, while other profiles may optimize for performance or size.",
                                                     {"cl"});
    argParser.addOption<std::string>("target", &program_target_file_path_,
                                     R"(D:\ClionProjects\RVM\test\test.ra)",
                                     "Specifies the target file path for the operation. "
                                     "When the operation is '--run', it refers to the executable file path. "
                                     "When the operation is '--comp', it refers to the source file to be compiled.",
                                     {"tar", "t"});
    argParser.addOption<std::string>("archive", &program_archive_file_path_,
                                     ".\\a.rsi",
                                     "Specifies the output file path for the generated file. "
                                     "When the operation is '--comp', it refers to the path where the compiled file will be saved.",
                                     {"arc", "a"});
    argParser.addOption<std::string>("env-dir", &program_environment_directory_,
                                     PROGRAM_ENVIRONMENT_DIRECTORY,
                                     "Specifies the environment working directory for the operation. ",
                                     {"ed"});
    argParser.addOption<std::string>("output-redirect", &program_output_redirect_,
                                     "console",
                                     "Specifies the output redirect place for the operation.",
                                     {"or"});
    argParser.addOption<std::string>("precomp-link-dir", &env::precompiled_link_dir_path,
                                     R"(D:\ClionProjects\RVM\lib)",
                                     "Specifies the path to the precompiled RSI file. "
                                     "When the '--precomp-link' flag is used, this option specifies the path to the precompiled RSI file.",
                                     {"pcld", "pld"});
    argParser.addOption<std::string>("working-dir", &program_working_directory_,
                                     undefined_,
                                     "Specifies the working directory for the operation. ",
                                     {"wd"});
    argParser.addOption<bool>("enable-colorful-output",&cc::enableColorfulOutput, true,
                              "Enables or disables colorful terminal output. "
                              "This option respects the NO_COLOR environment variable "
                              "which, if set, will override this setting.",
                              {"eco", "e-color", "colorful", "clr"});

    argParser.addMutuallyExclusive("run", "comp", ProgArgParser::CheckDir::BiDir);
    argParser.addMutuallyExclusive("rvm-work-directory", "work-directory",
                                   ProgArgParser::CheckDir::BiDir);
    argParser.addMutuallyExclusive(std::vector<std::string>{"help", "version"},
                                   std::vector<std::string>{"run", "comp", "debug",
                                                            "target", "comp-level", "archive",
                                                            "vs-check", "working-dir", "time-info"},
                                   ProgArgParser::CheckDir::BiDir);
    argParser.addMutuallyExclusive("help", std::vector<std::string>{"version", "vs-check"},
                                   ProgArgParser::CheckDir::BiDir);
    argParser.addMutuallyExclusive("version", std::vector<std::string>{"help", "vs-check"},
                                   ProgArgParser::CheckDir::BiDir);
    argParser.addMutuallyExclusive("vs-check", std::vector<std::string>{"run", "comp", "debug",
                                                                        "comp-level", "archive", "time-info"},
                                   ProgArgParser::CheckDir::BiDir);
    argParser.addDependent("help-option", "help", ProgArgParser::CheckDir::UniDir);
    argParser.addDependent(std::vector<std::string>{"run", "comp"}, "target",
                           ProgArgParser::CheckDir::UniDir);
    argParser.addDependent("debug", "run", ProgArgParser::CheckDir::UniDir);
    argParser.addDependent("vs-check", "target",
                           ProgArgParser::CheckDir::UniDir);
}

int main(const int argc, char *argv[]){
    try {
        initializeArgumentParser();
        argParser.parse(argc, argv);
        setProgramEnvDir(program_rvm_work_directory_ ?
                                PROGRAM_RVM_DIRECTORY : program_environment_directory_);
        if (env::precompiled_link_option) {
            env::precompiled_link_dir_path = getAbsolutePath(env::precompiled_link_dir_path);
        }
        if (program_output_redirect_ != "console") {
            io->setOutputToConsole(false);
            io->setOutputToFile(program_output_redirect_);
        }
        if (program_debug_option_) {
            if (program_output_redirect_ != "console") {
                throw std::runtime_error("Cannot redirect output to file when debug mode is enabled.");
            }
            InsSet::debug_mode = DebugMode::Standard;
        }
        if (program_serialize_) {
            auto file_ext = getFileExtFromPath(program_target_file_path_);
            env::initialize(program_target_file_path_, program_working_directory_);
            const auto ins_set = parse::parseCodeFromPath(program_target_file_path_, false);
            parse::serializeExecutableInsToBinaryFile(program_archive_file_path_, ins_set,
                                                      env::__program_serialization_profile__);
            *io << "[RVM Msg] >> Compilation finished with ["
                << getSerializationProfileName(env::__program_serialization_profile__) << "] profile.";
        }
        if (program_run_executable_) {
            const auto file_ext = getFileExtFromPath(program_target_file_path_);
            appendProgramWorkingDir(program_target_file_path_);
            std::shared_ptr<InsSet> ins_set;
            if (file_ext == RSI_EXT) {
                ins_set = parse::deserializeExecutableInsFromBinaryFile(
                        program_target_file_path_, env::__program_serialization_profile__);
            } else {
                ins_set = parse::parseCodeFromPath(program_target_file_path_, false);
            }
            env::initialize(ins_set->scope_leader_file, program_working_directory_);
            ins_set->execute();
        }
        if (program_help_option_) {
            if (program_help_option_name_.empty()) {
                *io << argParser.getHelpString();
            } else {
                *io << argParser.getOptionDescription(program_help_option_name_);
            }
        }
        if (program_version_option_) {
            *io << RVMSerialHeader::getRVMVersionInfo();
        }
        if (program_vs_check_option_) {
            auto file_ext = getFileExtFromPath(program_target_file_path_);
            try {
                const auto &binary_file_path = getAbsolutePath(program_target_file_path_);
                std::ifstream binary_file(binary_file_path, std::ios::binary);
                rvm_serial_header.deserialize(binary_file, env::__program_serialization_profile__,
                                                    binary_file_path, false);
                env::deserializeLinkedExtensions(binary_file);
                binary_file.close();
                if (auto version_info = rvm_serial_header.getRSIVersionInfo(program_target_file_path_);
                    version_info.empty()) {
                    *io += "[RVM Error] >> No version information found in the target RSI file.";
                } else {
                    *io << "[RVM Msg] >> " + version_info;
                }
            } catch (const std::exception &e) {
                *io += "[RVM Error] >> " +
                       StringManager::wrapText(
                               "Failed to retrieve the version information from the target RSI file. "
                               "This may be caused by file corruption or the use of a compilation mode that excludes "
                               "version details during the build process.", 80, 15);
                *io += "\n[RVM Error] >> " + std::string(e.what());
            }
        }
        if (program_run_time_info_option_) {
            *io << printProgramSpentTimeInfo();
        }
    } catch (RVM_Error &rvm_error) {
        if (rvm_serial_header.getProfile() < SerializationProfile::Release) {
            *io += rvm_error.toString();
        } else {
            *io += rvm_error.getErrorTitle() + rvm_error.getErrorInfo();
        }
    } catch (std::exception &e) {
        *io += "[RVM Error] >> ";
        *io += e.what();
    }
    io->flushOutputCache();
    data_space_pool.clearAllScopes();
    return 0;
}
