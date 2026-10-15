//
// Created by RestRegular on 2025/1/15.
//

#include <iostream>
#include <chrono>
#include "../include/ra_base.h"
#include "../include/ra_core.h"
#include "../include/ra_parser.h"
#include "../include/lib/newrcc.h"
#include "../include/lib/ra_utils.h"

using namespace base;
using namespace core;
using namespace utils;
using namespace parser;

static ProgArgParser argParser{};

static std::string program_target_file_path_;
static std::string program_archive_file_path_;
static std::string program_help_option_name_;
static std::string program_environment_directory_;
static std::string program_output_redirect_;
static std::string program_input_redirect_;
static std::string program_working_directory_;
static bool program_run_executable_;
static bool program_serialize_;
static bool program_debug_option_;
static bool program_run_time_info_option_;
static bool program_help_option_;
static bool program_version_option_;
static bool program_vs_check_option_;
static bool program_rvm_work_directory_;
static bool program_get_ris_;
static bool program_parse_info_;
static bool program_slot_mode_;
static bool program_legacy_mode_;
static bool program_dump_rsi_;
static std::string program_dump_rsi_to_;

static void initializeArgumentParser() {
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
    argParser.addFlag("get-ris", &program_get_ris_, false, true,
                      "This flag is used to get the hole RI names.",
                      {"ris", "gr"});
    argParser.addFlag("parse-info", &program_parse_info_, false, true,
                      "Prints the parse-time variable slot registration for the target file. "
                      "Every ALLOT / FUNC / FUNI / TP_DEF declaration is registered into a "
                      "'name -> frame slot' table while parsing; this flag dumps that table "
                      "(each scope's slot range, plus the symbol table visible from it). "
                      "It is a read-only diagnostic: it does not change execution semantics. "
                      "Use it together with '--run' or '--comp'.",
                      {"pi", "psi", "parse-scope-info"});
    argParser.addFlag("slot-mode", &program_slot_mode_, false, true,
                      "Enables the static (parse-time) variable slot architecture. "
                      "Variable names are resolved to (frame, slot) addresses while parsing, and the "
                      "runtime performs no name lookup at all. Scoping becomes lexical: a function body "
                      "sees its definition site rather than its call site. "
                      "Works for both '.ra' sources and '.rsi' artifacts (1.8.0+ carries the slot table). "
                      "Use '--legacy-mode' to force the legacy system instead.",
                      {"sm", "slot"});
    argParser.addFlag("legacy-mode", &program_legacy_mode_, false, true,
                      "Forces the legacy dynamic-scope memory system, disabling the static slot "
                      "architecture for '.ra' sources. Useful for A/B comparison against '--slot-mode'.",
                      {"lm", "legacy"});
    argParser.addFlag("dump-rsi", &program_dump_rsi_, false, true,
                      "Reads the '.rsi' target, re-serializes it, and writes the result to "
                      "'--dump-rsi-to'. Used to verify that the serializer and deserializer are "
                      "symmetric: a correct round trip reproduces the input byte for byte. "
                      "Note that '.rsi' has no header under the 'Minified' profile, so that profile "
                      "also requires '--comp-level Minified' on input.",
                      {"dr"});
    argParser.addOption<std::string>("dump-rsi-to", &program_dump_rsi_to_,
                                     "",
                                     "Output path for '--dump-rsi'.",
                                     {"drt"});

    argParser.addOption<std::string>("help-option", &program_help_option_name_,
                                                     "",
                                                     "Specifies the name of the help option. "
                                                     "This option is used to customize the name of the help option displayed in the help message.",
                                                     {"ho"});
    argParser.addOption<SerializationProfile>("comp-level", &env::program_serialization_profile_,
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
    argParser.addOption<std::string>("input-redirect", &program_input_redirect_,
                                     "console",
                                     "Specifies the input redirect place for the operation.",
                                     {"ir"});
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
    argParser.addMutuallyExclusive(std::vector<std::string>{"help", "version", "get-ris"},
                                   std::vector<std::string>{
                                       "run", "comp", "debug",
                                       "target", "comp-level", "archive",
                                       "vs-check", "working-dir", "time-info"},
                                   ProgArgParser::CheckDir::BiDir);
    argParser.addMutuallyExclusive("help", std::vector<std::string>{"version", "get-ris"}, ProgArgParser::CheckDir::BiDir);
    argParser.addMutuallyExclusive("version", "get-ris", ProgArgParser::CheckDir::BiDir);
    argParser.addMutuallyExclusive("vs-check", std::vector<std::string>{"run", "comp", "debug", "get-ris",
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
        program_target_file_path_ = getAbsolutePath(program_target_file_path_, getDefaultDir());
        if (env::precompiled_link_option) {
            env::precompiled_link_dir_path = getAbsolutePath(env::precompiled_link_dir_path);
        }
        if (program_output_redirect_ != "console") {
            io->setOutputToConsole(false);
            io->setOutputToFile(program_output_redirect_);
        }
        if (program_input_redirect_ != "console")
        {
            io->setInputFromFile(program_input_redirect_);
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
            const auto ins_set = parse::parseCodeFromPath(program_target_file_path_, program_parse_info_);
            if (program_parse_info_) {
                printParseScopeRecord(parse::last_parse_scope_record, std::cout,
                                      program_target_file_path_);
            }
            parse::serializeExecutableInsToBinaryFile(program_archive_file_path_, ins_set,
                                                      env::program_serialization_profile_);
            *io << "[RVM Msg] >> Compilation finished with ["
                << getSerializationProfileName(env::program_serialization_profile_) << "] profile.";
        }
        if (program_run_executable_) {
            const auto file_ext = getFileExtFromPath(program_target_file_path_);
            appendProgramWorkingDir(program_target_file_path_);
            // 环境必须在解析之前初始化：解析期要用全局名字集合补全根作用域，
            // 否则 true / false / null / tp-* / SN / SE / SS 这些全局标识符解析不到槽位。
            env::initialize(program_target_file_path_, program_working_directory_);
            std::shared_ptr<InsSet> ins_set;
            if (file_ext == RSI_EXT)
            {
                ins_set = parse::deserializeExecutableInsFromBinaryFile(
                        program_target_file_path_, env::program_serialization_profile_);
            } else if (file_ext == RA_EXT)
            {
                ins_set = parse::parseCodeFromPath(program_target_file_path_, program_parse_info_);
            } else
            {
                try
                {
                    ins_set = parse::deserializeExecutableInsFromBinaryFile(
                        program_target_file_path_, env::program_serialization_profile_);
                } catch (...)
                {
                    ins_set = parse::parseCodeFromPath(program_target_file_path_, program_parse_info_);
                }
            }
            if (program_parse_info_) {
                printParseScopeRecord(parse::last_parse_scope_record, std::cout,
                                      program_target_file_path_);
                core::memory::RVM_RUNTIME_MEMORY.printInfo(std::cout);
            }
            /*
             * 往返验证：把刚反序列化出来的指令集重新序列化。
             * 解析器与序列化器对称时，产物应与输入逐字节相同——能一次性抓出
             * 写读两端不对称、路径表错位这类问题。
             */
            if (program_dump_rsi_) {
                if (program_dump_rsi_to_.empty()) {
                    throw base::RVM_Error(
                            base::ErrorType::RuntimeError, program_target_file_path_, {},
                            {"--dump-rsi requires --dump-rsi-to <path>."},
                            {"Specify the output path for the re-serialized RSI."});
                }
                parse::serializeExecutableInsToBinaryFile(
                        program_dump_rsi_to_, ins_set, env::program_serialization_profile_);
                *io += "[RVM Msg] >> Round-trip RSI written to " + program_dump_rsi_to_ + "\n";
            } else {
            /*
             * 决定运行期使用哪一套变量系统。
             *
             *   --legacy-mode 优先：显式要求旧系统时，即使槽位表就绪也不启用。
             *   其余情况启用静态槽位架构（这是 RVM 的目标架构，故为默认）。
             *
             * 可用性由「帧表是否就绪」唯一决定，而不是按文件类型判断：
             *   - '.ra'  ：解析期建立帧表；
             *   - '.rsi' ：1.8.0 起槽位表随文件落盘，反序列化时重建帧表。
             * 两者都就绪才启用；否则明确拒绝，不静默降级到旧系统。
             */
            if (program_legacy_mode_) {
                core::memory::setSlotBindingModeEnabled(false);
            } else if (!parse::isSlotModeAvailable()) {
                if (program_slot_mode_) {
                    throw base::RVM_Error(
                            base::ErrorType::RuntimeError, program_target_file_path_, {},
                            {"Static slot mode is not available for this target.",
                             "Target: " + program_target_file_path_,
                             "Reason: no static slot table could be built or loaded."},
                            {"Compile the '.ra' source with this RVM version to produce an RSI "
                             "carrying the slot table, or drop '--slot-mode' to use the legacy "
                             "dynamic-scope system."});
                }
                core::memory::setSlotBindingModeEnabled(false);
            } else {
                core::memory::setSlotBindingModeEnabled(true);
            }
            ins_set->execute();
            }
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
                rvm_serial_header.deserialize(binary_file, env::program_serialization_profile_,
                                                    binary_file_path, false);
                env::deserializeLinkedExtensions(binary_file);
                binary_file.close();
                if (const auto version_info = rvm_serial_header.getRSIVersionInfo(program_target_file_path_);
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
        if (program_get_ris_)
        {
            for (const auto& riName: insNameMap | std::views::keys)
            {
                *io << riName << " ";
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
