//
// Created by RestRegular on 2025/1/15.
//

#include <utility>
#include "../include/ra_base.h"

#include <cstring>
#include <ranges>

#include "../include/lib/ra_utils.h"

namespace base {

    void RVMSerialHeader::serialize(std::ostream &out) const {
        if (profile < static_cast<uint16_t>(utils::SerializationProfile::Minified)) {
            out.write(reinterpret_cast<const char *>(this), sizeof(RVMSerialHeader));
            if (out.bad() || out.fail()) {
                throw std::runtime_error("Serialization failed");
            }
        }
    }

    void RVMSerialHeader::deserialize(std::istream &in, const utils::SerializationProfile &profile_,
                                      const std::string &file_path, const bool &validate_) {
        if (profile_ < utils::SerializationProfile::Minified) {
            in.read(reinterpret_cast<char *>(this), sizeof(RVMSerialHeader));
            if (in.gcount() != sizeof(RVMSerialHeader)) {
                throw std::runtime_error("Deserialization failed");
            }
            if (validate_) {
                validate(file_path);
            }
        } else {
            profile = static_cast<uint16_t>(utils::SerializationProfile::Minified);
        }
    }

    void RVMSerialHeader::validate(const std::string &file_path) const {
        constexpr uint8_t expected_magic[] = {'R', 'S', 'I', 0x1A};
        if (memcmp(magic, expected_magic, sizeof(expected_magic)) != 0) {  // 固定使用 expected_magic 的长度
            throw std::runtime_error("Invalid file format!");
        }
        if (major_ver > VERSION_MAJOR || minor_ver > VERSION_MINOR || patch_ver > VERSION_PATCH) {
            throw std::runtime_error("Unsupported version\nRVM version: " + getRVMVersionInfo() + "\nRSI File Version: " +
                                             getRSIVersionInfo(file_path));
        }
    }

    utils::SerializationProfile RVMSerialHeader::getProfile() const {
        return static_cast<utils::SerializationProfile>(profile);
    }

    void RVMSerialHeader::setProfile(const utils::SerializationProfile &profile_) {
        this->profile = static_cast<uint16_t>(profile_);
    }

    std::string RVMSerialHeader::getRVMVersionInfo() {
        return "RVM version " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "."
               + std::to_string(VERSION_PATCH) + " (on " + std::string(__DATE__) + " at " +
               std::string(__TIME__) + ")";
    }

    std::string RVMSerialHeader::getRSIVersionInfo(const std::string &path) const {
        auto file_name = utils::getFileFromPath(path);
        if (utils::getFileExtFromPath(path) != RSI_EXT) {
            file_name += RSI_EXT_;
        }
        return "[RSI File: '" + file_name + "'] version " + std::to_string(major_ver) + +"." +
               std::to_string(minor_ver) + "."
               + std::to_string(patch_ver) + " (" + getSerializationProfileName(getProfile()) + ")";
    }

    RVMSerialHeader rvm_serial_header{};

    const std::string PROGRAM_RVM_DIRECTORY = utils::getWindowsRVMDir();
    const std::string PROGRAM_ENVIRONMENT_DIRECTORY = utils::getWindowsDefaultDir();
    std::stack<std::string> PROGRAM_WORKING_DIRECTORY_STACK {};

    std::unordered_map<std::string, Relational> relationalMap = {
            {"RG",  Relational::RG},
            {"RGE", Relational::RGE},
            {"RNE", Relational::RNE},
            {"RE",  Relational::RE},
            {"RLE", Relational::RLE},
            {"RL",  Relational::RL},
            {"AND", Relational::AND},
            {"OR",  Relational::OR}
    };

    Relational stringToRelational(const std::string &str) {
        if (auto rel = relationalMap.find(str); rel != relationalMap.end()) {
            return rel->second;
        } else {
            throw RVM_Error(ErrorType::ValueError, unknown_, unknown_,
                            {"This error is caused by using an invalid relation string.",
                             "Invalid relation string: '" + str + "'"},
                            {"The relation string contains only 'RG', 'RGE', 'RNE', 'RE', 'RLE', 'RL', 'RT', 'RF'."});
        }
    }

    std::string relationalToString(const Relational &relational) {
        switch (relational) {
            case Relational::RG:
                return "RG";
            case Relational::RGE:
                return "RGE";
            case Relational::RNE:
                return "RNE";
            case Relational::RE:
                return "RE";
            case Relational::RAE:
                return "RAE";
            case Relational::RLE:
                return "RLE";
            case Relational::RL:
                return "RL";
            case Relational::RT:
                return "RT";
            case Relational::RF:
                return "RF";
            case Relational::AND:
                return "AND";
            case Relational::OR:
                return "OR";
            default:
                throw RVM_Error(ErrorType::ValueError, unknown_, unknown_,
                                {"This error is caused by using an invalid relation.",
                                 "Invalid relation: 'unknown_'"},
                                {"The relation string contains only 'RG', 'RGE', 'RNE', 'RE', 'RLE', 'RL', 'RT', 'RF', 'AND', 'OR'."});
        }
    }

    int RVM_ID::counter = 0;

    RVM_ID::RVM_ID(char sign, IDType idType)
            : sign(sign), uid(++counter), idType(idType) {
        dis_id = uid;
    }

    std::string RVM_ID::getIDString () const {
        std::string str = std::to_string(uid); // 将整数转换为字符串
        if (str.length() < 6) {
            str.insert(0, 6 - str.length(), '0'); // 在前面补零
        }
        const std::string &s{sign};
        return s + "x" + str;
    }

    std::string RVM_ID::toString(const std::string &detail) const {
        const auto &id_str = getIDString();
        return !detail.empty() ?
               ("<" + detail + ": " + id_str + ">") :
               ("<" + id_str + ">");
    }

    void RVM_ID::printInfo() const {
        std::cout << "ID Type: Base, Sign: " << sign << ", Unique ID: " << uid << std::endl;
    }

    char RVM_ID::getIDSign() const { return sign; }

    int RVM_ID::getUID() const { return uid; }

    std::string RVM_ID::toString() const { return toString(""); }

    bool RVM_ID::operator==(const RVM_ID &other) const { return uid == other.uid; }

    bool RVM_ID::fullEqualWith(const RVM_ID &other) const { return uid == other.uid; }

    bool RVM_ID::equalWith(const RVM_ID &other) const {
        return operator==(other);
    }

    std::string RVM_ID::getIdentStr() const {
        return std::string(1, sign);
    }

    InstID::InstID() : RVM_ID('I', base::IDType::Inst) {}

    void InstID::printInfo() const {
        std::cout << "ID Type: Inst, Sign: " << getIDSign() << ", Unique ID: " << getUID() << std::endl;
    }

    std::string InstID::toString() const {
        return RVM_ID::toString("Inst");
    }

    RVM_Data::RVM_Data() : instID(InstID()) {}

    std::string RVM_Data::toString() const {
        return "[Data(" + getTypeName() + "): " + getValStr() + "]";
    }

    std::string RVM_Data::toEscapedString() const {
        return this->toString();
    }

    std::string RVM_Data::getDataInfo() const {
        return "{typeID: " + this->getTypeID().toString() + ", instID: " +
               instID.toString() + ", data: '" + this->getValStr() + "'}";
    }

    bool RVM_Data::operator==(const RVM_Data &other) const {
        return instID == other.instID;
    }

    const RVM_ID &RVM_Data::getInstID() const { return instID; }


    RVM_ThreadPool &RVM_ThreadPool::getInstance(size_t numThreads) {
        static RVM_ThreadPool instance(numThreads); // 静态局部变量，确保唯一实例
        return instance;
    }

    RVM_ThreadPool::RVM_ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    RVM_ThreadPool::~RVM_ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker: workers) {
            worker.join();
        }
    }

    std::string getIODataTypeName(IODataType type) {
        switch (type) {
            case IODataType::Bool:
                return "[IOType: Bool]";
            case IODataType::Char:
                return "[IOType: Char]";
            case IODataType::Float:
                return "[IOType: Float]";
            case IODataType::Int:
                return "[IOType: Int]";
            case IODataType::Null:
                return "[IOType: Null]";
            case IODataType::String:
                return "[IOType: String]";
            default:
                throw RVM_Error(ErrorType::ValueError, unknown_, unknown_,
                                {"This error is caused by using an invalid IO data customType."},
                                {"The IO data customType contains only 'Bool', 'Char', 'Float', 'Int', 'Null', 'String'."});
        }
    }

    RVM_IO *RVM_IO::instance = nullptr;

    RVM_IO::RVM_IO(int threshold_size) : toConsole(true), toFile(false),
                                         bufferThreshold(1024 * threshold_size),
                                         currentSize(0) {}  // 默认阈值 20kb

    RVM_IO *RVM_IO::getInstance() {
        if (instance == nullptr) {
            instance = new RVM_IO();
        }
        return instance;
    }

    void RVM_IO::setOutputToConsole(bool enable) {
        toConsole = enable;
    }

    void RVM_IO::setOutputToFile(const std::string &filename) {
        if (fileStream.is_open()) {
            fileStream.close();
        }

        if (!filename.empty()) {
            fileStream.open(filename, std::ios::out);
            toFile = true;
        } else {
            toFile = false;
        }
    }

    void RVM_IO::setBufferThreshold(size_t threshold) {
        bufferThreshold = threshold;
        // 如果当前缓存已经超过新的阈值，立即刷新
        if (currentSize >= bufferThreshold) {
            flushOutputCache();
        }
    }

    size_t RVM_IO::getCurrentBufferSize() const {
        return currentSize;
    }

    // 刷新输出缓冲区
    void RVM_IO::flushOutputCache() {
        std::string normalContent = buffer.str();
        std::string errorContent = errorBuffer.str();

        // 清空缓冲区
        buffer.str("");
        buffer.clear();
        errorBuffer.str("");
        errorBuffer.clear();
        currentSize = 0;

        // 使用线程池异步执行控制台输出
        if (toConsole) {
            threadPool.enqueue([normalContent, errorContent]() {
                std::cout << normalContent;
                std::cout.flush();
                std::cerr << errorContent;
                std::cerr.flush();
            });
        }

        // 使用线程池异步执行文件写入
        if (toFile && fileStream.is_open()) {
            threadPool.enqueue([this, normalContent, errorContent]() {
                std::lock_guard<std::mutex> lock(fileWriteMutex);
                fileStream << normalContent << errorContent;
                fileStream.flush();
            });
        }
    }

    RVM_IO::~RVM_IO() {
        flushOutputCache();  // 确保所有数据都被输出
        if (fileStream.is_open()) {
            fileStream.close();
        }
        delete instance;
        instance = nullptr;
        inputCache.clear();
    }

    void RVM_IO::readLineRaw() {
        std::string line;
        std::getline(std::cin, line);
        line = utils::StringManager::escape(line);
        inputCache.push_back(std::move(line));
    }

    std::vector<std::string> RVM_IO::nextStrings(size_t count) {
        std::vector<std::string> result;
        for (size_t i = 0; i < count && !inputCache.empty(); ++i) {
            result.push_back(inputCache.front());
            inputCache.erase(inputCache.begin());
        }
        return result;
    }

    std::string RVM_IO::nextString() {
        if (inputCache.empty()) {
            throw RVM_Error(ErrorType::IOError, unknown_, unknown_,
                            {"This error is due to getting input from an empty cache."},
                            {"Please enter data in the forward cache before getting input data."});
        }
        std::string result = inputCache.front();
        inputCache.erase(inputCache.begin());
        return result;
    }

    void RVM_IO::readLineAndSplit() {
        std::string line;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            inputCache.push_back(utils::StringManager::escape(word));
        }
    }

    void RVM_IO::flashInputCache() {
        inputCache.clear();
    }

    bool RVM_IO::hasNextString() const {
        return !inputCache.empty();
    }

    static const std::unordered_map<ErrorType, std::string> errorTypeMap = {
            {ErrorType::SyntaxError,    "SyntaxError"},
            {ErrorType::RuntimeError,   "RuntimeError"},
            {ErrorType::IOError,        "IOError"},
            {ErrorType::TypeError,      "TypeError"},
            {ErrorType::ValueError,     "ValueError"},
            {ErrorType::MemoryError,    "MemoryError"},
            {ErrorType::RecursionError, "RecursionError"},
            {ErrorType::UnknownError,   "UnknownError"},
            {ErrorType::CustomError,    "CustomError"},
            {ErrorType::ArgumentError,  "ArgumentError"},
            {ErrorType::RangeError,     "RangeError"},
            {ErrorType::FileError,      "FileError"},
            {ErrorType::IDError,        "IDError"},
            {ErrorType::LinkError,      "LinkError"},
            {ErrorType::KeyError,       "KeyError"},
            {ErrorType::FieldError,     "FieldError"}
    };

    std::string getErrorTypeName(const ErrorType &errorType) {
        auto it = errorTypeMap.find(errorType);
        if (it != errorTypeMap.end()) {
            return it->second;
        } else {
            throw RVM_Error(ErrorType::ValueError, unknown_, unknown_,
                            {"This error is caused by using an invalid error customType.",
                             "Invalid error customType: 'unknown_'"},
                            {"The error customType contains only 'SyntaxError', 'RuntimeError', "
                             "'IOError', 'TypeError', 'ValueError', 'MemoryError', 'RecursionError', "
                             "'UnknownError', 'CustomError', 'ArgumentError'."});
        }
    }


    RVM_Error::RVM_Error(ErrorType error_type, std::string error_position, std::string error_line,
                         std::vector<std::string> error_info,
                         std::vector<std::string> repair_tips) : error_type(std::move(error_type)),
                                                                 error_position(std::move(error_position)),
                                                                 error_line(error_line),
                                                                 error_info(std::move(error_info)),
                                                                 repair_tips(std::move(repair_tips)) {
        error_type_name = getErrorTypeName(this->error_type);
    }

    RVM_Error::RVM_Error(std::string error_type, std::string error_position, std::string error_line,
                         std::vector<std::string> error_info,
                         std::vector<std::string> repair_tips) : error_type(ErrorType::CustomError),
                                                                 error_type_name(std::move(error_type)),
                                                                 error_position(std::move(error_position)),
                                                                 error_line(error_line),
                                                                 error_info(std::move(error_info)),
                                                                 repair_tips(std::move(repair_tips)) {}


    std::string RVM_Error::toString() const {
        const auto &space_size = space.size();
        std::ostringstream oss;

        oss << getErrorTitle();

        if (!trace_info.empty()) {
            oss << " [ Trace Back ]\n" << utils::listJoin(trace_info) << "\n";
        }

        if (!error_position.empty() && error_position != unknown_) {
            oss << space << "[ Line ] : "
                << utils::StringManager::wrapText(error_position, 80, space_size + 10, "", "~ ") << "\n"
                << std::string(space_size + 9, ' ') << "| "
                << utils::StringManager::wrapText(error_line, 80, space_size + 9, "", "| ~ ") << "\n\n";
        }

        if (!error_info.empty()) {
            oss << getErrorInfo();
        }

        if (!repair_tips.empty()) {
            oss << space << "[ Tips ] : ";
            for (size_t i = 0; i < repair_tips.size(); ++i) {
                oss << utils::StringManager::wrapText(repair_tips[i], 80, space_size + 10, "", "~ ");
                if (i < repair_tips.size() - 1) {
                    oss << "\n" << std::string(space_size + 9, ' ') << "- ";
                }
            }
            oss << "\n";
        }

        return oss.str();
    }

    void RVM_Error::addTraceInfo(const std::string &traceInfo) {
        this->trace_info.push_front(traceInfo);
    }

    std::string RVM_Error::getErrorInfo() const {
        std::ostringstream oss;
        oss << space << "[ Info ] : ";
        for (size_t i = 0; i < error_info.size(); ++i) {
            oss << utils::StringManager::wrapText(error_info[i], 80, space.size() + 10, "", "~ ");
            if (i < error_info.size() - 1) {
                oss << "\n" << std::string(space.size() + 9, ' ') << "- ";
            }
        }
        oss << "\n\n";
        return oss.str();
    }

    std::string RVM_Error::getErrorTitle() const {
        std::ostringstream oss;
        oss << "\n" << std::string(20, '=');
        oss << "[ " << error_type_name << " ]" << std::string(60, '=') << "\n";
        return oss.str();
    }

    std::string getKeywordTypeName(const KeywordType &keywordType) {
        switch (keywordType) {
            case KeywordType::DataTypes:
                return "DataTypes";
            case KeywordType::FileModes:
                return "FileModes";
            case KeywordType::IOModes:
                return "IOModes";
            case KeywordType::Instructions:
                return "Instructions";
            case KeywordType::Logical:
                return "Logical";
            case KeywordType::MemoryVars:
                return "MemoryVars";
            default:
                return "Unknown";
        }
    }

    bool containsKeyword(const std::string &keyword) {
        for (const auto &val: categorizedKeywords | std::views::values) {
            if (const auto &keywords = val;
                keywords.contains(keyword)) {
                return true;
            }
        }
        return false;
    }

    bool containsKeywordInCategory(const KeywordType &category, const std::string &keyword) {
        // 获取分类名称
        const std::string &categoryName = getKeywordTypeName(category);

        // 获取分类的关键字集合
        const auto &keywords = categorizedKeywords[categoryName];

        // 查找关键字
        return keywords.contains(keyword);
    }

    std::string getKeywordTypeFormatString(const KeywordType &keywordType){
        return "[KeywordType: " + getKeywordTypeName(keywordType) + "]";
    }

    void addKeywordInCategory(const KeywordType &category, const std::string &keyword) {
        const std::string &categoryName = getKeywordTypeName(category);
        categorizedKeywords[categoryName].insert(keyword);
    }

    std::string OpModeTools::getOpModeString(const OpMode &opMode) {
        switch (opMode) {
            case OpMode::Add:
                return "[OpMode: Add]";
            case OpMode::Delete:
                return "[OpMode: Delete]";
            case OpMode::Find:
                return "[OpMode: Find]";
            case OpMode::Update:
                return "[OpMode: Update]";
            case OpMode::Remove:
                return "[OpMode: Remove]";
            default:
                return "[OpMode: UNKNOWN]";
        }
    }
} // base