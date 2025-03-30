//
// Created by RestRegular on 2025/1/17.
//

#ifndef RVM_RA_UTILS_H
#define RVM_RA_UTILS_H

#include <iostream>
#include <map>

namespace utils {

    struct Number;
    enum class NumType {
        double_type,
        int_type
    };

    enum class ArgType{
        number,
        string,
        identifier,
        keyword,
        unknown
    };

    enum class SerializationProfile;

    void serializeArgType(std::ostream &os, const ArgType &argType, const SerializationProfile &profile);

    ArgType deserializeArgType(std::istream &is, const SerializationProfile &profile);

    struct Number{
    public:
        NumType type;
        int int_value;
        double double_value;
        explicit Number(int int_value);
        explicit Number(double double_value);

        [[nodiscard]] std::string toString() const;

        friend std::ostream& operator<<(std::ostream& os, const Number& num);
    };

    std::string printProgramSpentTimeInfo();

    std::string getFileNameFromPath(const std::string &path);

    std::string getFileExtFromPath(const std::string &path);

    std::pair<std::string, std::string> getFileInfoFromPath(const std::string &path);

    std::string getFileFromPath(const std::string &path);

    std::string getFileDirFromPath(const std::string &path);

    std::string getObjectFormatString(const std::string &type, const std::string &name);

    std::string getWindowsRVMDir();

    std::string getWindowsDefaultDir();

    std::string getAbsolutePath(const std::string& path);

    void setProgramDir(const std::string &dir_path);

    int stringToInt(const std::string& str);

    double stringToDouble(const std::string& str);

    char stringToChar(const std::string& str);

    bool stringToBool(const std::string& str);

    Number stringToNumber(const std::string& str);

    std::string doubleToString(double value);

    bool isStringFormat(const std::string& str);

    std::string parseStringFormat(const std::string& str);

    void parseStringFormat_noReturn(std::string& result);

    bool isNumber(const std::string& str);

    std::string getPosStrFromFilePath(const std::string &file_path);

    struct StringManager {
    public:
        static StringManager &getInstance();

        static std::vector<std::string> split(const std::string &content, char delimiter = ',');

        std::string processQuotedString(std::string_view input);

        void registerEscapeHandler(char escapeChar, std::function<std::string(void)> handler);

        std::string unescape(const std::string &input);

        void unescape_nret(std::string &input);

        static std::string parseStringFormat(const std::string &input);

        static void parseStringFormat_nret(std::string &result);

        static bool isStringFormat(const std::string &str);

        static void trim(std::string &str);

        static std::string trim(const std::string &str);

        static std::string toStringFormat(const std::string &str);

        static std::map<std::string, std::string> splitStringByChars(const std::string& input, const std::string& delimiters);

        static std::string escape(const std::string &input);

        static std::string wrapText(const std::string& text, size_t lineWidth, size_t indent = 0,
                                    std::string last_line_suffix = "", std::string next_line_prefix = "");

        static std::string combineNearbyString(const std::string &input, const int &line_row);
    private:
        StringManager();

        std::string handleEscapeSequence(std::string_view input, size_t &pos);

        std::map<char, std::function<std::string(void)>> escapeHandlers;

        static constexpr size_t INITIAL_BUFFER_SIZE = 1024;
    };

    struct Pos {
    public:
        Pos() = default;

        Pos(int line, int column, std::string filepath);

        [[nodiscard]] int getLine() const;

        [[nodiscard]] int getColumn() const;

        [[nodiscard]] std::string getFilepath() const;

        [[nodiscard]] std::string getFilePosStr() const;

        friend std::ostream& operator<<(std::ostream& out, const Pos& pos);

        [[nodiscard]] std::string toString() const;

        void serialize(std::ostream& out, const SerializationProfile &profile) const;

        void deserialize(std::istream& in, const SerializationProfile &profile);

    private:
        int line{0};
        int column{0};
        std::string filepath{};
    };

    ArgType getArgType(const std::string& str);

    std::string getArgTypeName(const ArgType& argType);

    std::string getTypeFormatString(const ArgType &argType);

    struct Arg {
    public:
        Arg() = default;

        Arg(Pos pos, const std::string &value);

        explicit Arg(std::string value);

        [[nodiscard]] const Pos &getPos() const;

        [[nodiscard]] utils::ArgType getType() const;

        [[nodiscard]] const std::string &getValue() const;

        [[nodiscard]] std::string toString() const;

        const std::string getPosStr() const;

        void serialize(std::ostream& out, const SerializationProfile &profile) const;

        void deserialize(std::istream& in, const SerializationProfile &profile);

    private:
        Pos pos{};
        utils::ArgType type{};
        std::string value;
    };

    std::string readFile(const std::string& filepath);

    std::vector<std::string> readFileToLines(const std::string &filepath);

    bool writeFile(const std::string &filepath, const std::string &content);

    bool appendFile(const std::string &filepath, const std::string &content);

    bool isValidIdentifier(const std::string &content);

    inline bool isDigit(char c);

    inline bool isSign(char c);

    inline bool isDoubleQuoted(const std::string &content);

    bool isValidNumber(const std::string &content);

    bool isValidKeyWord(const std::string &content);

    bool isValidParameter(const std::string &content);

    std::string getSpaceFormatString(std::string name, std::string value);

    size_t getSpaceFrontOfLineCode(const std::string& lineCode);

    void addCharBeforeNewline(std::string &code, const char &c= ';');

    enum class FieldType{
        InstField,
        TPField
    };

    enum class SerializationProfile {
        Debug,         // 完整信息：源码、符号表、行号、版本（开发/调试）
        Testing,       // 移除源码但保留符号表 & 行号（CI/CD自动化测试）
        Release,       // 仅指令流 + 必要元数据 + 版本（生产环境）
        Minified       // 极致精简（无版本/元数据，仅用于封闭沙盒）
    };

    std::string getSerializationProfileName(const SerializationProfile &profile);

    class ProgArgParser {
    public:
        enum class CheckDir {
            UniDir,
            BiDir,
            Unidirectional = UniDir,
            Bidirectional = BiDir
        };

        void addMutuallyExclusive(const std::vector<std::string>& options,
                                  const std::string& target, CheckDir direction = CheckDir::Bidirectional);

        void addMutuallyExclusive(const std::vector<std::string>& options,
                                  const std::vector<std::string>& targets,
                                  CheckDir direction = CheckDir::Bidirectional);

        void addDependent(const std::vector<std::string>& options,
                          const std::string& target, CheckDir direction = CheckDir::Bidirectional);

        void addDependent(const std::vector<std::string>& options,
                          const std::vector<std::string>& targets,
                          CheckDir direction = CheckDir::Bidirectional);

        void addMutuallyExclusiveGroup(const std::vector<std::string>& options,
                                       CheckDir direction = CheckDir::Bidirectional);

        void addDependentGroup(const std::vector<std::string>& options,
                               CheckDir direction = CheckDir::Bidirectional);

        void addMutuallyExclusive(const std::string& opt1, const std::string& opt2,
                                  CheckDir direction = CheckDir::UniDir);

        void addMutuallyExclusive(const std::string& opt1, const std::vector<std::string>& opt2,
                                  CheckDir direction = CheckDir::UniDir);

        void addDependent(const std::string& opt1, const std::string& opt2,
                          CheckDir direction = CheckDir::UniDir);

        void addFlag(const std::string& name, bool* var, bool defaultValue, bool whenPresent,
                     const std::string& description = "", const std::vector<std::string>& aliases = {});

        template<typename T>
        void addOption(const std::string& name, T* var, const T& defaultValue, const std::string& description = "", const std::vector<std::string>& aliases = {}) {
            *var = defaultValue;
            setupOptionSetter(name, var, aliases, description);
        }

        void parse(int argc, char* argv[]);

        std::string getOptionDescription(const std::string& name) const;

        std::string getHelpString(size_t lineWidth = 80, size_t optionIndent = 2, size_t descriptionIndent = 6) const;
    private:
        struct FlagInfo {
            std::string name;
            bool* var;
            bool whenPresent;
            std::vector<std::string> aliases; // 存储别名
            std::string description;         // 存储描述

            std::string toString() const;
        };

        struct OptionInfo {
            std::string name;
            std::function<void(const std::string&)> setter;
            std::vector<std::string> aliases; // 存储别名
            std::string description;         // 存储描述

            std::string toString() const {
                std::ostringstream oss;
                oss << "[Option: '" << name;
                for (const auto& alias : aliases) {
                    oss << "' | '" << alias;
                }
                oss << "']: ";
                oss << StringManager::wrapText(description, 80, oss.str().size());
                return oss.str();
            }
        };

        std::vector<FlagInfo> flags_;
        std::vector<OptionInfo> options_;

        OptionInfo* findOption(const std::string& name);

        FlagInfo* findFlag(const std::string& name);

        std::string getOptionMainName(const std::string& name) const;

        struct MutuallyExclusiveRule {
            std::string opt1;
            std::string opt2;
            CheckDir direction;
        };
        std::vector<MutuallyExclusiveRule> mutuallyExclusive_;

        struct DependentRule {
            std::string opt1;
            std::string opt2;
            CheckDir direction;
        };
        std::vector<DependentRule> dependent_;

        template<typename T>
        void setupOptionSetter(const std::string& name, T* var, const std::vector<std::string>& aliases,
                               const std::string& description) {
            if constexpr (std::is_same_v<T, std::string>) {
                options_.push_back({name, [var](const std::string& value) { *var = value; }, aliases, description});
            } else if constexpr (std::is_same_v<T, int>) {
                options_.push_back({name, [var](const std::string& value) {
                    try { *var = std::stoi(value); }
                    catch (...) { throw std::invalid_argument("Invalid integer value"); }
                }, aliases, description});
            } else if constexpr (std::is_same_v<T, double>) {
                options_.push_back({name, [var](const std::string& value) {
                    try { *var = std::stod(value); }
                    catch (...) { throw std::invalid_argument("Invalid double value"); }
                }, aliases, description});
            } else if constexpr (std::is_same_v<T, bool>) {
                options_.push_back({name, [var](const std::string& value) {
                    if (value == "true" || value == "1") *var = true;
                    else if (value == "false" || value == "0") *var = false;
                    else throw std::invalid_argument("Invalid boolean value");
                }, aliases, description});
            } else if constexpr (std::is_same_v<T, SerializationProfile>) {
                options_.push_back({name, [var](const std::string& value) {
                    if (value == "Debug" || value == "-db" || value == "0" || value == "L0")
                        *var = SerializationProfile::Debug;
                    else if (value == "Testing" || value == "-tst" || value == "1" || value == "L1")
                        *var = SerializationProfile::Testing;
                    else if (value == "Release" || value == "-rel" || value == "2" || value == "L2")
                        *var = SerializationProfile::Release;
                    else if (value == "Minified" || value == "-min" || value == "3" || value == "L3")
                        *var = SerializationProfile::Minified;
                    else throw std::invalid_argument("Invalid SerializationProfile value");
                }, aliases, description});
            } else {
                static_assert(!std::is_same_v<T, T>, "Unsupported type");
            }
        }

        void handleOption(const std::string& arg, int argc, char* argv[], int& i);

    };

    enum class TimeFormat {
        ISO,        // YYYY-MM-DD
        US,         // MM/DD/YYYY
        European,   // DD/MM/YYYY
        Timestamp   // Unix timestamp
    };

    std::tuple<char, char, char, char, char> getSeparators(TimeFormat format);

    bool parseDateFromString(const std::string &dateString, TimeFormat format,
                             int &year, int &month, int &day, int &hour, int &minute, int &second);

    int getRandomInt(int min, int max);

    double getRandomFloat(double min, double max, int decimalPlaces = 6);

} // utils

#endif //RVM_RA_UTILS_H
