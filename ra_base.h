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
#include <unordered_set>
#include <cstring>
#include "ra_utils.h"

/*
 * RA语言的基础组件命名空间
 */
namespace base {
    // 预设的空间名
#define MAIN_SCOPE    "MainScope"
#define GLOBAL_SCOPE  "GlobalScope"

// 预设的扩展名
#define RA_EXT       "ra"
#define RSI_EXT      "rsi"
#define RA_EXT_      ".ra"
#define RSI_EXT_     ".rsi"

// 预设的前后缀连接符
#define fix "-"

// 预设的 Space 前缀
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

// 预设的 InsSet 的后缀
#define suf_END     "-END"

// 预设的通用默认值
#define unknown_     "unknown"
#define null_        "null"

// 预设的 ANSI 颜色
#define Reset        "\033[0m"
#define Bold         "\033[1m"
#define Italic       "\033[3m"
#define L_UnderLine  "\033[4m"
#define ColorReverse "\033[7m"
#define MiddleLine   "\033[9m"
#define B_UnderLine  "\033[21m"

#define VERSION_MAJOR   1
#define VERSION_MID     0
#define VERSION_MINOR   0

#pragma pack(push, 1)

    struct RVMSerialHeader {
        uint8_t magic[4];
        uint16_t major_ver;  // 主机字节序
        uint16_t mid_ver;    // 主机字节序
        uint16_t minor_ver;  // 主机字节序
        uint16_t profile;    // 序列化配置

        RVMSerialHeader(const utils::SerializationProfile &profile = utils::SerializationProfile::Debug)
                : magic{'R', 'S', 'I', 0x1A},
                  major_ver(VERSION_MAJOR),
                  mid_ver(VERSION_MID),
                  minor_ver(VERSION_MINOR),
                  profile(static_cast<uint16_t>(profile)) {}

        void serialize(std::ostream &out) const;

        void deserialize(std::istream &in, const utils::SerializationProfile &profile_,
                         const std::string &file_path, const bool &validate_=true);

        void validate(const std::string &file_path) const;

        utils::SerializationProfile getProfile() const;

        void setProfile(const utils::SerializationProfile &profile_);

        std::string getRVMVersionInfo() const;

        std::string getRSIVersionInfo(const std::string &path) const;
    };

#pragma pack(pop)

    static_assert(sizeof(RVMSerialHeader) == 12, "Header size must be 12 bytes");

    extern RVMSerialHeader rvm_serial_header;

    // ID 的类型枚举
    enum class IDType {
        // ID 的类型枚举
        Inst,
        Type,
        Data,
        Func,
        RI,
        Ins,

        // TypeID 的类型枚举
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
        Time
    };

    // 比较关系枚举
    enum class Relational {
        RL,
        RLE,
        RNE,
        RE,
        RAE,
        RSE,
        RS,
        RT,
        RF,
        AND,
        OR
    };

    Relational stringToRelational(const std::string &str);

    std::string relationalToString(const Relational &relational);

    /*
     * RVM_ID
     * RA语言中所有ID的父类，每一种ID都有自己专属的ID符号，ID的统一输出格式为<?x......>，例：<Tx000001>
     * 目前已注册的ID：
     *      1. InstID：表示一个实例的ID，全局唯一
     *      2. TypeID：表示一个类型的ID
     *      3. DataID：表示一个数据的ID
     *      ~~4. FuncID：表示一个函数的ID~~ => 移除函数ID，改为使用DataID
     *      5. RIID：表示一个指令的ID
     *      6. InsID：表示一条可执行指令的ID
     */
    struct RVM_ID {
        // 用于展示的id
        int dis_id;
        char sign; // ID 的符号
        IDType idType;

        explicit RVM_ID(char sign, IDType idType);

        [[nodiscard]] char getIDSign() const; // deprecated method: 废弃，请直接访问sign属性，保留以供兼容性使用

        [[nodiscard]] int getUID() const;

        [[nodiscard]] std::string getIDString() const;

        [[nodiscard]] virtual std::string toString(const std::string &detail) const;

        [[nodiscard]] virtual std::string toString() const;

        virtual void printInfo() const;

        virtual bool operator==(const RVM_ID &other) const;

        virtual bool equalWith(const RVM_ID &other) const;

        virtual bool fullEqualWith(const RVM_ID &other) const;

        virtual ~RVM_ID() = default;

    private:
        static int counter; // 静态计数器，用于生成唯一ID
        int uid;            // 唯一序列号
    };

    /*
     * InstID
     * 表示实例ID
     */
    struct InstID : base::RVM_ID {
    public:
        explicit InstID();

        void printInfo() const override;

        [[nodiscard]] std::string toString() const override;
    };

    /*
     * RVM_Data
     * RA语言中所有数据类型的基类，所有数据类型都继承自此基类
     * RA语言中的数据类型包括：
     *  1. Numeric（数值类型/不变数据类型）：Numeric类型是所有数值类型数据的父类，数值类型数据可以参与数值计算等操作。
     *      Numeric类型数据中包括：
     *          a. Int（整数类型）：整数类型的数据
     *          b. Float（浮点类型）：浮点类型数据
     *          c. Bool（布尔类型）：布尔类型数据
     *              Bool类型数据也算做Number类型中的特殊Int类型。
     *              Bool类型数据只有两个，0（false）和 1（true）。
     *              默认情况下false取值为 0，true取值为 1；
     *              特殊情况下true取值为一切非 0 Number类型数据。
     *          d. Char（字符类型）：字符类型数据
     *              字符类型数据是特殊的Int类型数值类型数据，其是通过ASCII码映射于Int类型数据。
     *  2. Iterable（迭代类型/可变数据类型）：Iterable类型数据是所有组合型数据的父类，可迭代类型数据可以参与迭代相关操作。
     *      Iterable类型数据中包括：
     *          a. String（字符串类型）：字符串类型组合数据
     *              String类型数据就是由多个Char类型数据组合而成的组合可迭代类型数据。
     *              虽然String类型是Iterable类型数据的子类，但是它是不可变数据类型。它本质上不能被修改。
     *          b. List（列表类型）：列表类型组合数据
     *              List类型数据就是由多种数据类型组合而成的组合可迭代类型数据。
     *          c. Stack（栈类型）：栈类型组合数据 [NotImplemented]
     *              Stack类型数据就是由多种数据类型组合而成的组合可迭代类型数据。
     *              Stack类型与List类型数据的不同在于Stack类型数据仅能在容器的末尾添加内容，无法插入数据到指定位置。
     *              并且Stack类型仅能从容器末尾依次访问内部数据。访问过的数据不再保留。
     *          d. Pairs（键值对类型）：键值对类型数据 [NotImplemented]
     *              Pairs类型数据就是由Numeric类型数据与其他不可变数据类型作为键，所有数据类型作为值的键值对组成的组合
     *              数据类型。
     *  ...
     */
    struct RVM_Data {
        explicit RVM_Data();

        // 虚析构函数，确保正确释放资源
        virtual ~RVM_Data() = default;

        // 纯虚函数
        [[nodiscard]] virtual std::string getValStr() const = 0;

        [[nodiscard]] virtual std::string getTypeName() const = 0;

        [[nodiscard]] virtual RVM_ID &getTypeID() const = 0;

        [[nodiscard]] virtual bool updateData(const std::shared_ptr<RVM_Data> &newData) = 0;

        [[nodiscard]] virtual bool compare(const std::shared_ptr<RVM_Data> &other,
                                           const base::Relational &relational) const = 0;

        [[nodiscard]] virtual bool convertToBool() const = 0;

        [[nodiscard]] virtual std::shared_ptr<RVM_Data> copy_ptr() const = 0;

        // 虚函数
        [[nodiscard]] virtual std::string toString() const;

        [[nodiscard]] virtual std::string toEscapedString() const;

        [[nodiscard]] const RVM_ID &getInstID() const;

        [[nodiscard]] std::string getDataInfo();

        bool operator==(const RVM_Data &other) const;

    protected:
        InstID instID; // 实例ID
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
        std::vector<std::thread> workers;          // 工作线程
        std::queue<std::function<void()>> tasks;   // 任务队列
        std::mutex queueMutex;                     // 任务队列互斥锁
        std::condition_variable condition;         // 条件变量
        bool stop = false;                         // 线程池停止标志

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
        // 暂时只支持以上类型
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
            // 如果超过阈值，自动刷新
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
        void immediateOutput(const Args&... args) {
            if (toConsole){
                immediateOutputImpl(std::cout, args...);
                std::cout << std::flush;
            }
            if (toFile) {
                immediateOutputImpl(fileStream, args...);
                fileStream << std::flush;
            }
        }

        template<typename... Args>
        void immediateOutputError(const Args&... args) {
            if (toConsole){
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
        void immediateOutputImpl(std::ostream& os, const T& arg) {
            os << arg;
        }

        template<typename T, typename... Args>
        void immediateOutputImpl(std::ostream &out, const T& first, const Args&... args) {
            out << first;
            immediateOutputImpl(out, args...);
        }
    };

    // 错误类型枚举
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

    struct RVM_Error : public std::exception {
        ErrorType error_type;
        std::string error_type_name;
        std::string error_position;
        std::string error_line;
        std::vector<std::string> error_info;
        std::vector<std::string> repair_tips;
        std::string trace_info;
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

        struct ExposedError : public RVM_Error {
            ExposedError(std::string error_position, std::string error_line,
                         const std::shared_ptr<RVM_Data> &error_info)
                    : RVM_Error(base::ErrorType::CustomError, error_position, error_line,
                                {error_info->getValStr()}, {}) {}
        };

        struct TypeMismatchError : public RVM_Error {
            TypeMismatchError(std::string error_position, std::string error_line,
                              std::vector<std::string> error_info,
                              std::vector<std::string> repair_tips)
                    : RVM_Error(base::ErrorType::TypeError, error_position, error_line,
                                {"This error is caused by a data type mismatch."}, repair_tips) {
                this->error_info.insert(this->error_info.end(), error_info.begin(), error_info.end());
            }
        };

        struct DuplicateKeyError : public RVM_Error {
            DuplicateKeyError(std::string error_position, std::string error_line, std::vector<std::string> error_info,
                              std::vector<std::string> repair_tips) :
                    RVM_Error(base::ErrorType::MemoryError, error_position, error_line,
                              {"This error is caused by allocating memory space with the duplicate name."},
                              repair_tips) {
                this->error_info.insert(this->error_info.end(), error_info.begin(), error_info.end());
            }
        };

        struct ArgumentNumberError : public RVM_Error {
            ArgumentNumberError(std::string error_position, std::string error_line,
                                std::string arg_num, int supported_num, std::string error_ri,
                                std::vector<std::string> repair_tips) :
                    base::RVM_Error(base::ErrorType::ArgumentError, error_position, error_line,
                                    {"This error is caused by a mismatch between required and received arguments.",
                                     "Target RI: " + error_ri, "Required Arg Number: " + arg_num,
                                     "Received Arg Number: " + std::to_string(supported_num)},
                                    {"Check the number of arguments provided for " + error_ri + "."}) {
                this->repair_tips.insert(this->repair_tips.end(), repair_tips.begin(), repair_tips.end());
            }
        };

        struct ArgumentError : public RVM_Error {
            ArgumentError(std::string error_position, std::string error_line, std::string error_argument_info,
                          std::vector<std::string> repair_tips) :
                    base::RVM_Error(base::ErrorType::ArgumentError, error_position, error_line,
                                    {"This error is caused by a mismatch between required and received arguments.",
                                     std::move(error_argument_info)},
                                    repair_tips) {}
        };

        struct MemoryError : public RVM_Error {
            MemoryError(std::string error_position, std::string error_line, std::vector<std::string> error_info,
                        std::vector<std::string> repair_tips) :
                    RVM_Error(base::ErrorType::MemoryError, error_position, error_line,
                              error_info, repair_tips) {}
        };

        struct FieldNotFoundError: public RVM_Error {
            FieldNotFoundError(std::string error_position, std::string error_line, std::string target_tp_or_inst, std::string undefined_field,
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
                              error_info, repair_tips) {}
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

    static std::unordered_map<std::string, std::unordered_set<std::string>> categorizedKeywords {
            {"IOMode",       {"s-m",    "s-l",      "s-f",    "s-n"}},
            {"Logical",      {"RL",     "RLE",      "RNE",    "RE",      "RAE",   "RSE",     "RS",      "RT",      "RF",        "AND", "OR"}},
            {"MemoryVar",    {"true",   "false",    "null",   "SN",      "SE",    "SS",      "SR"}},
            {"DataTypes",    {"tp-int", "tp-float", "tp-str", "tp-bool", "tp-null",
                              "tp-char", "tp-list", "tp-dict", "tp-series", "tp-pair",
                              "tp-time"}},
            {"FileModes",    {"fl-r",   "fl-w",     "fl-rw",  "fl-ap",   "fl-ra", "fl-wa"}},
            {"Instructions", {}}
    };

    void addKeywordInCategory(const KeywordType &category, const std::string &keyword);

    bool containsKeyword(const std::string &keyword);

    bool containsKeywordInCategory(const KeywordType &category, const std::string &keyword);

    std::string getKeywordTypeFormatString(const KeywordType &keywordType);
} // base

#endif //RVM_RA_BASE_H
