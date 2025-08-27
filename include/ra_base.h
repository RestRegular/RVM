//
// Created by RestRegular on 2025/1/15.
//

#ifndef RVM_RA_BASE_H
#define RVM_RA_BASE_H

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <queue>
#include <fstream>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <list>
#include <unordered_set>
#include <stack>
#include "lib/ra_utils.h"
#include "rvm_version.h"


namespace base {

#define MAIN_SCOPE      "MainScope"
#define GLOBAL_SCOPE    "GlobalScope"

#define RA_EXT          "ra"
#define RSI_EXT         "rsi"
#define RA_EXT_         ".ra"
#define RSI_EXT_        ".rsi"

#define fix "-"

#define pre_SRC             "SRC-"
#define pre_EXT             "EXT-"
#define pre_DATA            "DATA-"
#define pre_FUNC            "FUNC-"
#define pre_REPEAT          "REPEAT-"
#define pre_UNTIL           "UNTIL-"
#define pre_SP_NEW          "SP_NEW-"
#define pre_ATMP            "ATMP-"
#define pre_DETECT          "DETECT-"
#define pre_FINALLY         "FINALLY-"
#define pre_ITER_TRAV       "ITER_TRAV-"
#define pre_ITER_REV_TRAV   "ITER_REV_TRAV-"
#define pre_RASM            "RASM-"

#define suf_END         "-END"

#define unknown_        "unknown"
#define undefined_      "undefined"
#define null_           "null"

#define tp              "tp"
#define tp_null         "tp-null"
#define tp_int          "tp-int"
#define tp_float        "tp-float"
#define tp_str          "tp-str"
#define tp_bool         "tp-bool"
#define tp_char         "tp-char"
#define tp_list         "tp-list"
#define tp_dict         "tp-dict"
#define tp_func         "tp-func"
#define tp_pair         "tp-pair"
#define tp_series       "tp-series"
#define tp_time         "tp-time"
#define tp_qot          "tp-qot"
#define tp_file         "tp-file"
#define tp_error        "tp-error"

#pragma pack(push, 1)

    struct RVMSerialHeader {
        uint8_t magic[4];
        uint16_t major_ver;
        uint16_t minor_ver;
        uint16_t patch_ver;
        uint16_t profile;

        explicit RVMSerialHeader(const utils::SerializationProfile &profile = utils::SerializationProfile::Debug)
                : magic{'R', 'S', 'I', 0x1A},
                  major_ver(VERSION_MAJOR),
                  minor_ver(VERSION_MINOR),
                  patch_ver(VERSION_PATCH),
                  profile(static_cast<uint16_t>(profile)) {}

        void serialize(std::ostream &out) const;

        void deserialize(std::istream &in, const utils::SerializationProfile &profile_,
                         const std::string &file_path, const bool &validate_ = true);

        void validate(const std::string &file_path) const;

        utils::SerializationProfile getProfile() const;

        void setProfile(const utils::SerializationProfile &profile_);

        static std::string getRVMVersionInfo();

        std::string getRSIVersionInfo(const std::string &path) const;
    };

#pragma pack(pop)

    static_assert(sizeof(RVMSerialHeader) == 12, "Header size must be 12 bytes");

    extern RVMSerialHeader rvm_serial_header;

    extern const std::string PROGRAM_RVM_DIRECTORY; // RVM working directory
    extern const std::string PROGRAM_ENVIRONMENT_DIRECTORY; // Command line working directory
    extern std::stack<std::string> PROGRAM_WORKING_DIRECTORY_STACK; // Program working directory

    enum class IDType;

    enum class Relational;

    Relational stringToRelational(const std::string &str);

    std::string relationalToString(const Relational &relational);

    enum class OpMode {
        Add,
        Delete,
        Remove,
        Update,
        Find
    };

    namespace OpModeTools {
        std::string getOpModeString(const base::OpMode &opMode);
    }

    struct RVM_ID {
        int dis_id;
        char sign;
        IDType idType;

        explicit RVM_ID(char sign, IDType idType);

        [[nodiscard]] char getIDSign() const;

        [[nodiscard]] int getUID() const;

        [[nodiscard]] std::string getIDString() const;

        [[nodiscard]] virtual std::string toString(const std::string &detail) const;

        [[nodiscard]] virtual std::string toString() const;

        virtual void printInfo() const;

        virtual bool operator==(const RVM_ID &other) const;

        virtual bool equalWith(const RVM_ID &other) const;

        virtual bool fullEqualWith(const RVM_ID &other) const;

        virtual std::string getIdentStr() const;

        virtual ~RVM_ID() = default;

    private:
        static int counter;
        int uid;
    };

    struct InstID : base::RVM_ID {
    public:
        explicit InstID();

        void printInfo() const override;

        [[nodiscard]] std::string toString() const override;
    };

    struct RVM_Data {
        explicit RVM_Data();

        virtual ~RVM_Data() = default;

        [[nodiscard]] virtual std::string getValStr() const = 0;

        [[nodiscard]] virtual std::string getTypeName() const = 0;

        [[nodiscard]] virtual RVM_ID &getTypeID() const = 0;

        [[nodiscard]] virtual bool updateData(const std::shared_ptr<RVM_Data> &newData) = 0;

        [[nodiscard]] virtual bool compare(const std::shared_ptr<RVM_Data> &other,
                                           const base::Relational &relational) const = 0;

        [[nodiscard]] virtual bool convertToBool() const = 0;

        [[nodiscard]] virtual std::shared_ptr<RVM_Data> copy_ptr() const = 0;

        [[nodiscard]] virtual std::string toString() const;

        [[nodiscard]] virtual std::string toEscapedString() const;

        [[nodiscard]] const RVM_ID &getInstID() const;

        [[nodiscard]] std::string getDataInfo() const;

        bool operator==(const RVM_Data &other) const;

    protected:
        InstID instID;
    };

    struct RVM_ThreadPool {
        RVM_ThreadPool(const RVM_ThreadPool &) = delete;

        RVM_ThreadPool &operator=(const RVM_ThreadPool &) = delete;

        static RVM_ThreadPool &getInstance(size_t numThreads = std::thread::hardware_concurrency());

        template<class F>
        void enqueue(F &&f) {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                tasks.emplace(std::forward<F>(f));
            }
            condition.notify_one();
        }

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queueMutex;
        std::condition_variable condition;
        bool stop = false;

        explicit RVM_ThreadPool(size_t numThreads);

        ~RVM_ThreadPool();
    };


    enum class IODataType {
        Null,
        Int,
        Float,
        String,
        Char,
        Bool
    };

    std::string getIODataTypeName(IODataType type);

    struct RVM_IO {

        RVM_IO(const RVM_IO &) = delete;

        RVM_IO &operator=(const RVM_IO &) = delete;

        static RVM_IO *getInstance();

        void setOutputToConsole(bool enable);

        void setOutputToFile(const std::string &filename);

        void setBufferThreshold(size_t threshold);

        size_t getCurrentBufferSize() const;

        template<typename T>
        RVM_IO &operator<<(const T &data) {
            std::stringstream temp;
            temp << data;
            std::string strData = temp.str();
            currentSize += strData.size();
            buffer << strData;
            if (currentSize >= bufferThreshold) {
                flushOutputCache();
            }
            return *this;
        }

        template<typename T>
        RVM_IO &operator+=(const T &data) {
            std::stringstream temp;
            temp << data;
            std::string strData = temp.str();
            currentSize += strData.size();
            errorBuffer << strData;
            if (currentSize >= bufferThreshold) {
                flushOutputCache();
            }
            return *this;
        }

        template<typename... Args>
        void immediateOutput(const Args &... args) {
            if constexpr (sizeof...(args) == 0) return;
            if (toConsole) {
                immediateOutputImpl(std::cout, args...);
                std::cout << std::flush;
            }
            if (toFile) {
                immediateOutputImpl(fileStream, args...);
                fileStream << std::flush;
            }
        }

        template<typename... Args>
        void immediateOutputError(const Args &... args) {
            if constexpr (sizeof...(args) == 0) return;
            if (toConsole) {
                immediateOutputImpl(std::cerr, args...);
                std::cerr << std::flush;
            }
            if (toFile) {
                immediateOutputImpl(fileStream, args...);
                fileStream << std::flush;
            }
        }

        void flushOutputCache();

        void readLineRaw();

        void readLineAndSplit();

        std::string nextString();

        std::vector<std::string> nextStrings(size_t count);

        bool hasNextString() const;

        void flashInputCache();

        ~RVM_IO();

    private:
        static RVM_IO *instance;
        std::stringstream buffer;
        std::stringstream errorBuffer;
        std::ofstream fileStream;
        std::mutex fileWriteMutex;
        bool toConsole;
        bool toFile;
        size_t bufferThreshold;
        size_t currentSize;
        std::vector<std::string> inputCache;
        RVM_ThreadPool &threadPool = RVM_ThreadPool::getInstance();

        explicit RVM_IO(int threshold_size = 20);

        template<typename T>
        void immediateOutputImpl(std::ostream &os, const T &arg) {
            os << arg;
        }

        template<typename T, typename... Args>
        void immediateOutputImpl(std::ostream &out, const T &first, const Args &... args) {
            out << first;
            immediateOutputImpl(out, args...);
        }
    };

    enum class ErrorType {
        SyntaxError,
        RuntimeError,
        IOError,
        TypeError,
        ValueError,
        RangeError,
        IDError,
        KeyError,
        FileError,
        ArgumentError,
        MemoryError,
        FieldError,
        RecursionError,
        LinkError,
        UnknownError,
        CustomError
    };

    std::string getErrorTypeName(const ErrorType &errorType);

    struct RVM_Error : std::exception {
        ErrorType error_type;
        std::string error_type_name;
        std::string error_position;
        std::string error_line;
        std::vector<std::string> error_info;
        std::vector<std::string> repair_tips;
        std::list<std::string> trace_info;
        std::string space = std::string(4, ' ');

        RVM_Error(ErrorType error_type, std::string error_position, std::string error_line,
                  std::vector<std::string> error_info, std::vector<std::string> repair_tips);

        RVM_Error(std::string error_type, std::string error_position, std::string error_line,
                  std::vector<std::string> error_info, std::vector<std::string> repair_tips);

        [[nodiscard]] std::string toString() const;

        [[nodiscard]] std::string getErrorTitle() const;

        [[nodiscard]] std::string getErrorInfo() const;

        void addTraceInfo(const std::string &traceInfo);
    };

    namespace errors {

        struct ExposedError final : RVM_Error {
            ExposedError(std::string error_position, std::string error_line,
                         const std::vector<std::string> &error_info)
                    : RVM_Error(ErrorType::CustomError, error_position, error_line,
                                error_info, {}) {}
        };

        struct ModificationError final : RVM_Error {
            ModificationError(std::string error_position, std::string error_line,
                              const std::vector<std::string> &error_info,
                              std::vector<std::string> repair_tips)
                    : RVM_Error(ErrorType::CustomError, error_position, error_line,
                                error_info, {}) {}
        };

        struct DataTypeMismatchError final : RVM_Error {
            DataTypeMismatchError(const std::string& error_position,
                const std::string& error_line,
                std::vector<std::string> error_info,
                const std::vector<std::string>& repair_tips)
                    : RVM_Error(ErrorType::TypeError, error_position, error_line,
                                {"This error is caused by a data type mismatch."}, repair_tips) {
                this->error_info.insert(this->error_info.end(), error_info.begin(), error_info.end());
            }
        };

        struct ArgTypeMismatchError final : RVM_Error {
            ArgTypeMismatchError(const std::string& error_position,
                const std::string& error_line,
                std::vector<std::string> error_info,
                const std::vector<std::string>& repair_tips)
                    : RVM_Error(ErrorType::TypeError, error_position, error_line,
                                {"This error is caused by an arg type mismatch."}, repair_tips) {
                this->error_info.insert(this->error_info.end(), error_info.begin(), error_info.end());
            }
        };

        struct DuplicateKeyError final : RVM_Error {
            DuplicateKeyError(std::string error_position, std::string error_line, std::vector<std::string> error_info,
                              std::vector<std::string> repair_tips) :
                    RVM_Error(base::ErrorType::MemoryError, error_position, error_line,
                              {"This error is caused by allocating memory space with the duplicate name."},
                              repair_tips) {
                this->error_info.insert(this->error_info.end(), error_info.begin(), error_info.end());
            }
        };

        struct ArgumentNumberError final : RVM_Error {
            ArgumentNumberError(std::string error_position, std::string error_line,
                                std::string arg_num, int supported_num, std::string error_ri,
                                std::vector<std::string> repair_tips) :
                    RVM_Error(base::ErrorType::ArgumentError, error_position, error_line,
                                    {"This error is caused by a mismatch between required and received arguments.",
                                     "Target RI: " + error_ri, "Required Arg Number: " + arg_num,
                                     "Received Arg Number: " + std::to_string(supported_num)},
                                    {"Check the number of arguments provided for " + error_ri + "."}) {
                this->repair_tips.insert(this->repair_tips.end(), repair_tips.begin(), repair_tips.end());
            }
        };

        struct ArgumentError final : RVM_Error {
            ArgumentError(std::string error_position, std::string error_line, std::string error_argument_info,
                          std::vector<std::string> repair_tips) :
                    base::RVM_Error(base::ErrorType::ArgumentError, error_position, error_line,
                                    {"This error is caused by a mismatch between required and received arguments.",
                                     std::move(error_argument_info)},
                                    repair_tips) {}
        };

        struct MemoryError final : RVM_Error {
            MemoryError(std::string error_position, std::string error_line, std::vector<std::string> error_info,
                        std::vector<std::string> repair_tips) :
                    RVM_Error(ErrorType::MemoryError, error_position, error_line,
                              error_info, repair_tips) {}
        };

        struct FieldNotFoundError final : RVM_Error {
            FieldNotFoundError(std::string error_position, std::string error_line, std::string target_tp_or_inst,
                               std::string undefined_field,
                               std::vector<std::string> repair_tips) :
                    RVM_Error(base::ErrorType::FieldError, error_position, error_line,
                              {"This error is caused by accessing an undefined field.",
                               "Target: " + target_tp_or_inst,
                               "Undefined Field: \"" + undefined_field + "\""}, repair_tips) {}
        };

        struct FileWriteError : public RVM_Error {
            FileWriteError(std::string error_position, std::string error_line, std::vector<std::string> error_info,
                           std::vector<std::string> repair_tips) :
                    RVM_Error(base::ErrorType::FileError, error_position, error_line,
                              error_info, repair_tips) {}
        };

        struct FileReadError : public RVM_Error {
            FileReadError(std::string error_position, std::string error_line, std::vector<std::string> error_info,
                          std::vector<std::string> repair_tips) :
                    RVM_Error(base::ErrorType::FileError, error_position, error_line,
                              {"This error is caused by the non - existence or incorrectness"
                               " of the path provided for reading the file."},
                              repair_tips) {
                this->error_info.insert(this->error_info.end(), error_info.begin(), error_info.end());
            }
        };

        struct KeyNotFoundError : public RVM_Error {
            KeyNotFoundError(std::string error_position, std::string error_line, std::vector<std::string> error_info,
                             std::vector<std::string> repair_tips)
                    : RVM_Error(base::ErrorType::KeyError, error_position, error_line,
                                {"This error is caused by the specified key not being found."},
                                {"When accessing a value by key, ensure the key exists and has a corresponding value."}) {
                this->error_info.insert(this->error_info.end(), error_info.begin(), error_info.end());
                this->repair_tips.insert(this->repair_tips.end(), repair_tips.begin(), repair_tips.end());
            }
        };

        struct IndexOutOfRangeError : public RVM_Error {
            IndexOutOfRangeError(std::string error_position,
                                 std::string error_line,
                                 std::vector<std::string> error_info,
                                 std::vector<std::string> repair_tips)
                    : RVM_Error(base::ErrorType::RangeError,
                                error_position,
                                error_line,
                                {"This error is caused by accessing an invalid index position."},
                                {"Ensure the index is within the valid range of the container."}) {
                this->error_info.insert(this->error_info.end(),
                                        error_info.begin(), error_info.end());
                this->repair_tips.insert(this->repair_tips.end(),
                                         repair_tips.begin(), repair_tips.end());
            }
        };

    }

    enum class KeywordType {
        IOModes,
        Logical,
        MemoryVars,
        DataTypes,
        FileModes,
        Instructions
    };

    std::string getKeywordTypeName(const KeywordType &keywordType);

    static std::unordered_map<std::string, std::unordered_set<std::string>> categorizedKeywords{
            {"IOMode",       {"s-m",  "s-l",   "s-f",   "s-n",   "s-unpack"}},
            {"Logical",      {"RG",   "RGE",   "RNE",   "RE",    "RAE",   "RGE", "RG", "RT", "RF", "AND", "OR"}},
            {"MemoryVar",    {"true", "false", "null",  "SN",    "SE",    "SS",  "SR"}},
            {"DataTypes",    {tp, tp_int, tp_float, tp_str, tp_bool, tp_null,
                                     tp_char, tp_list, tp_dict, tp_series, tp_pair,
                                     tp_time, tp_qot, tp_error}},
            {"FileModes",    {"fl-r", "fl-w",  "fl-rw", "fl-ap", "fl-ra", "fl-wa"}},
            {"Instructions", {}}
    };

    void addKeywordInCategory(const KeywordType &category, const std::string &keyword);

    bool containsKeyword(const std::string &keyword);

    bool containsKeywordInCategory(const KeywordType &category, const std::string &keyword);

    std::string getKeywordTypeFormatString(const KeywordType &keywordType);

    enum class IDType {
        Inst,
        Type,
        Data,
        Func,
        RI,
        Ins,

        DType,
        Null,
        Numeric,
        Int,
        Float,
        Char,
        Bool,
        Iterable,
        String,
        List,
        Dict,
        Series,
        Structure,
        KeyValuePair,
        CompareGroup,
        CustomType,
        CustomTypeInst,
        Callable,
        Function,
        RetFunction,
        TempFunc,
        Quote,
        File,
        Time,
        Error,
        Extension
    };

    enum class Relational {
        RG,
        RGE,
        RNE,
        RE,
        RAE,
        RLE,
        RL,
        RT,
        RF,
        AND,
        OR
    };
} // base

#endif //RVM_RA_BASE_H
