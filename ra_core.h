//
// Created by RestRegular on 2025/1/15.
//

#ifndef RVM_RA_CORE_H
#define RVM_RA_CORE_H

#include <iostream>
#include <ranges>
#include <string>
#include <utility>
#include <vector>
#include <utility>
#include <functional>
#include <list>
#include <atomic>
#include <algorithm>
#include <stack>
#include "ra_base.h"
#include "ra_utils.h"

namespace core {
    namespace id {
        struct TypeID : base::RVM_ID {
            std::string type_name;

            explicit TypeID(std::string type_name, const base::IDType &idType);

            explicit TypeID(std::string type_name, const std::shared_ptr<TypeID> &parent_type,
                            const base::IDType &idType);

            void printInfo() const override;

            [[nodiscard]] std::string toString() const override;

            [[nodiscard]] std::string toString(const std::string &detail) const override;

            bool operator==(const RVM_ID &other) const override;

            std::shared_ptr<TypeID> getTopParentTypeID() const;

            bool equalWith(const RVM_ID &other) const override;

        private:
            std::shared_ptr<TypeID> parent_type_id;
        };

        struct RIID : base::RVM_ID {
            explicit RIID();

            void printInfo() const override;

            [[nodiscard]] std::string toString() const override;
        };

        struct InsID : base::RVM_ID {
            explicit InsID();

            void printInfo() const override;

            [[nodiscard]] std::string toString() const override;

        };

        struct DataID : public base::RVM_ID {
            std::string idstring;

            explicit DataID();

            explicit DataID(std::string name, std::string spaceName = "");

            [[nodiscard]] std::string toFullString() const;

            [[nodiscard]] const std::string &getName() const;

            [[nodiscard]] const std::string &getScopeName() const;

            [[nodiscard]] size_t getIndex() const;

            void printInfo() const override;

            bool operator==(const DataID &other) const;

            [[nodiscard]] std::string toString() const override;

        private:
            std::string name_;        // ���Ʋ���
            std::string scopeName_;   // �ռ䲿��
            size_t index_ = -1;       // ��������

            static size_t generateIndex();
        };
    }

    namespace components {
        // һЩ��Ҫ���������
        enum class ExecutionStatus;
        struct RI;
        struct Ins;
        struct InsSet;
        struct DetectBlock;
        struct AtmpBlock;
        using RIID [[maybe_unused]] = id::RIID;
        using InsID = id::InsID;
        using StdArgs = std::vector<utils::Arg>;
        typedef ExecutionStatus (*ExecutorFunc)(const Ins&, int&, const StdArgs&);
    }

    namespace data {
        struct Null;
        struct Numeric;
        struct Int;
        struct Float;
        struct Char;
        struct Bool;
        struct Iterable;
        struct String;
        struct List;
        struct Dict;
        struct Series;
        struct Structure;
        struct KeyValuePair;
        struct CompareGroup;
        struct CustomType;
        struct CustomInst;
        struct Callable;
        struct Function;
        struct RetFunction;
        struct Quote;
        struct File;
        struct Time;

        struct Null : base::RVM_Data {
            static id::TypeID typeId;

            explicit Null();

            ~Null() override = default;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] std::string toEscapedString() const override;

            bool convertToBool() const override;
        };

        struct Numeric : base::RVM_Data {
            static id::TypeID typeId;

            static std::vector<id::TypeID> numericTypeIDs;

            explicit Numeric();

            ~Numeric() override = default;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            static bool isNumeric(const std::shared_ptr<RVM_Data> &data);

            [[nodiscard]] virtual std::shared_ptr<Numeric> add(const std::shared_ptr<Numeric> &other) const = 0;

            [[nodiscard]] virtual std::shared_ptr<Numeric> subtract(const std::shared_ptr<Numeric> &other) const = 0;

            [[nodiscard]] virtual std::shared_ptr<Numeric> multiply(const std::shared_ptr<Numeric> &other) const = 0;

            [[nodiscard]] virtual std::shared_ptr<Numeric> divide(const std::shared_ptr<Numeric> &other) const = 0;

            [[nodiscard]] virtual std::shared_ptr<Numeric> opp() const = 0;

            [[nodiscard]] virtual std::shared_ptr<Numeric> pow(const std::shared_ptr<Numeric> &other) const = 0;

            [[nodiscard]] virtual std::shared_ptr<Numeric> root(const std::shared_ptr<Numeric> &other) const = 0;

            std::string toEscapedString() const override;
        };

        struct Int : Numeric {
            static id::TypeID typeId;

            explicit Int(int value);

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] int getValue() const;

            [[nodiscard]] id::TypeID &getTypeID() const override;

            template<typename Op>
            std::shared_ptr<Numeric> executeOperator(const std::shared_ptr<Numeric> &other, Op op, const std::string &opstr) const;

            [[nodiscard]] std::shared_ptr<Numeric> add(const std::shared_ptr<Numeric> &other) const override;

            [[nodiscard]] std::shared_ptr<Numeric> subtract(const std::shared_ptr<Numeric> &other) const override;

            [[nodiscard]] std::shared_ptr<Numeric> multiply(const std::shared_ptr<Numeric> &other) const override;

            [[nodiscard]] std::shared_ptr<Numeric> divide(const std::shared_ptr<Numeric> &other) const override;

            [[nodiscard]] std::shared_ptr<Numeric> opp() const override;

            [[nodiscard]] std::shared_ptr<Numeric> pow(const std::shared_ptr<Numeric> &other) const override;

            [[nodiscard]] std::shared_ptr<Numeric> root(const std::shared_ptr<Numeric> &other) const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            bool convertToBool() const override;

        protected:
            int value;
        };

        struct Float : Numeric {
            static id::TypeID typeId;

            explicit Float(double value);

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] double getValue() const;

            [[nodiscard]] id::TypeID &getTypeID() const override;

            template<typename Op>
            std::shared_ptr<Numeric> executeOperator(const std::shared_ptr<Numeric> &other, Op op) const;

            [[nodiscard]] std::shared_ptr<Numeric> add(const std::shared_ptr<Numeric> &other) const override;

            [[nodiscard]] std::shared_ptr<Numeric> subtract(const std::shared_ptr<Numeric> &other) const override;

            [[nodiscard]] std::shared_ptr<Numeric> multiply(const std::shared_ptr<Numeric> &other) const override;

            [[nodiscard]] std::shared_ptr<Numeric> divide(const std::shared_ptr<Numeric> &other) const override;

            [[nodiscard]] std::shared_ptr<Numeric> opp() const override;

            std::shared_ptr<Numeric> pow(const std::shared_ptr<Numeric> &other) const override;

            std::shared_ptr<Numeric> root(const std::shared_ptr<Numeric> &other) const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<base::RVM_Data> &other, const base::Relational &relational) const override;

            bool convertToBool() const override;

        private:
            double value;
        };

        struct Bool : Int {
            static id::TypeID typeId;

            explicit Bool(bool value);

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] id::TypeID &getTypeID() const override;

            std::string getValStr() const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            std::shared_ptr<Numeric> opp() const override;
        };

        struct Char : Int {
            static id::TypeID typeId;

            explicit Char(char value);

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] id::TypeID &getTypeID() const override;

            std::string toEscapedString() const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            std::string toString() const override;
        };

        struct Iterable : base::RVM_Data {
            static id::TypeID typeId;

            explicit Iterable();

            ~Iterable() override = default;

            virtual void begin() const = 0;

            virtual void end() const = 0;

            [[nodiscard]] virtual std::shared_ptr<RVM_Data> next() const = 0;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            virtual size_t size() const = 0;

            virtual void append(const std::shared_ptr<RVM_Data> &data) = 0;

            virtual std::shared_ptr<Iterable> subpart(int begin, int end) = 0;

            virtual std::shared_ptr<RVM_Data> getDataAt(int index) = 0;

            virtual void setDataAt(int index, const std::shared_ptr<RVM_Data> &data) = 0;

            virtual void eraseDataAt(int index) = 0;

            virtual void insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) = 0;
        };

        struct String : Iterable {
            static id::TypeID typeId;

            explicit String(std::string value);

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] id::TypeID &getTypeID() const override;

            void begin() const override;

            void end() const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> next() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            size_t size() const override;

            void append(const std::shared_ptr<RVM_Data> &data) override;

            std::shared_ptr<Iterable> subpart(int begin, int end) override;

            std::shared_ptr<RVM_Data> getDataAt(int index) override;

            void setDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            void eraseDataAt(int index) override;

            std::string toEscapedString() const override;

            std::shared_ptr<List> trans_to_list() const;

            std::shared_ptr<Series> trans_to_series() const;

            void insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            std::string toString() const override;

            bool convertToBool() const override;
        private:
            std::string value;
        };

        struct List : Iterable {
            static id::TypeID typeId;

            explicit List();

            List(const std::vector<std::shared_ptr<RVM_Data>> &dataList);

            ~List() override = default;

            void begin() const override;

            void end() const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> next() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            size_t size() const override;

            void append(const std::shared_ptr<RVM_Data> &data) override;

            std::shared_ptr<Iterable> subpart(int begin, int end) override;

            virtual std::string getValStdStr(const base::RVM_ID &main_container_id, const std::string &prefix = "[",
                                             const std::string &suffix = "]") const;

            std::string getValStr() const override;

            std::string getTypeName() const override;

            std::shared_ptr<RVM_Data> getDataAt(int index) override;

            void setDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            void eraseDataAt(int index) override;

            std::string toEscapedString() const override;

            const std::vector<std::shared_ptr<base::RVM_Data>> &getDataList() const;

            void insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            base::RVM_ID &getTypeID() const override;

            bool convertToBool() const override;

        protected:
            std::vector<std::shared_ptr<base::RVM_Data>> dataList{};
        };

        struct Dict: Iterable {
            static id::TypeID typeId;

            explicit Dict();

            Dict (std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> dataDict,
                  std::vector<std::string> keyList);

            Dict (std::shared_ptr<List> list);

            ~Dict() override = default;

            void begin() const override;

            void end() const override;

            std::shared_ptr<RVM_Data> next() const override;

            size_t size() const override;

            void append(const std::shared_ptr<RVM_Data> &data) override;

            std::shared_ptr<Iterable> subpart(int begin, int end) override;

            std::shared_ptr<RVM_Data> getDataAt(int index) override;

            std::shared_ptr<RVM_Data> getDataAt(const std::string &key);

            void setDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            void setDataAt(std::string key, const std::shared_ptr<RVM_Data> &data);

            void eraseDataAt(int index) override;

            void eraseDataAt(std::string key);

            std::string getValStr() const override;

            std::string getTypeName() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            std::string toEscapedString() const override;

            void insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            base::RVM_ID &getTypeID() const override;

            bool convertToBool() const override;

        private:

            std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> dataDict{};
            std::vector<std::string> keyList{};

        };

        struct Series : List {
            static id::TypeID typeId;

            explicit Series();

            Series(std::shared_ptr<base::RVM_Data> list);

            ~Series() override = default;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<Iterable> subpart(int begin, int end) override;

            std::string getValStr() const override;

            std::string getTypeName() const override;

            std::string toEscapedString() const override;
        };

        struct Structure: base::RVM_Data {
            static id::TypeID typeId;

            explicit Structure();

            ~Structure() override = default;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;
        };

        struct KeyValuePair: Structure {
            static id::TypeID typeId;
            std::shared_ptr<base::RVM_Data> key = nullptr;
            std::shared_ptr<base::RVM_Data> value = nullptr;

            explicit KeyValuePair();

            KeyValuePair(std::shared_ptr<base::RVM_Data> key, std::shared_ptr<base::RVM_Data> value);

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            base::RVM_ID &getTypeID() const override;

            std::string getValStr() const override;

            std::string getTypeName() const override;

            bool convertToBool() const override;
        };

        struct CompareGroup : Structure{
            static id::TypeID typeId;
            std::shared_ptr<id::DataID> compLeft;
            std::shared_ptr<id::DataID> compRight;

            CompareGroup(std::shared_ptr<id::DataID> dataLeft, std::shared_ptr<id::DataID> dataRight);

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] bool compare(const base::Relational & relational);

            bool convertToBool() const override;
        };

        /*
         * CustomType ��ʾһ���Զ�����������ͣ�������������ֶκ�ʵ���ֶ��Լ������ֶΣ�
         * �����ֶ�Ҳ��Ϊ���֣�ʵ�����������ͷ�����
         * �ڲ����游����
         * ����ֶ�ֻ��ͨ�� CustomType ������ӣ�����ͨ�� CustomInst �������
         * ���������ֶε�����ֻ��ͨ�� CustomType �������ã�����ͨ�� CustomInst ��������
         * ����ʵ���ֶε�����ֻ��ͨ�� CustomInst �������ã�����ͨ�� CustomType ��������
         */
        struct CustomType: Structure {
            static id::TypeID typeId;
            std::string typeName;
            std::shared_ptr<CustomType> parentType;
            std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> tpFields {};
            std::vector<std::string> instFields {};
            std::unordered_set<std::string> methodFields {};

            CustomType(std::string className, std::shared_ptr<CustomType> parentType = nullptr);

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] std::string getValStr() const override;

            base::RVM_ID &getTypeID() const override;

            std::string getTypeName() const override;

            [[nodiscard]] bool hasField(const std::string &fieldName);

            [[nodiscard]] bool hasInstField(const std::string &fieldName);

            [[nodiscard]] bool hasTpField(const std::string &fieldName);

            [[nodiscard]] bool hasFieldItself(const std::string &fieldName);

            [[nodiscard]] bool hasTpFieldItself(const std::string &fieldName);

            [[nodiscard]] bool hasInstFieldItself(const std::string &fieldName);

            void travalTypes(std::function<bool(std::shared_ptr<CustomType>)> callback);

            [[nodiscard]] std::shared_ptr<base::RVM_Data> getTpField(const std::string &fieldName);

            void setTpField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData);

            void addTpField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData = nullptr);

            void addInstField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData = nullptr);

            [[nodiscard]] bool checkBelongTo(const std::shared_ptr<CustomType> &type) const;

            bool convertToBool() const override;

            [[nodiscard]] std::string getTypeIDString() const;
        };

        /*
         * CustomInst ��ʾһ���Զ������������ʵ��
         * ��������ֶ�
         * �����������ֶκͷ���������
         * �����Ի�ȡʵ���ֶΣ�Ҳ���Ի�ȡ�����ֶ�
         */
        struct CustomInst : Structure {
            static id::TypeID typeId;
            std::unordered_map
            <std::string, std::unordered_map
            <std::string, std::shared_ptr<base::RVM_Data>>> instFields; //instFields: �洢ʵ���� CustomType �ͼ̳еĸ�����Ԥ�������ʵ���ֶ�
            std::shared_ptr<CustomType> customType;

            CustomInst(std::shared_ptr<CustomType> instType);

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            base::RVM_ID &getTypeID() const override;

            std::string getValStr() const override;

            std::string getTypeName() const override;

            bool hasField(const std::string &fieldName);

            [[nodiscard]] std::shared_ptr<base::RVM_Data>
            getField(const std::string &fieldName, const std::shared_ptr<CustomType> &specCustomType = nullptr);

            void setField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData,
                          const std::shared_ptr<CustomType> &specCustomType = nullptr);

            bool convertToBool() const override;

            [[nodiscard]] std::string getTypeIDString() const;

            void derivedToChildType(const std::shared_ptr<CustomType> &childType);
        };

        struct Callable : base::RVM_Data {
            static id::TypeID typeId;
            id::DataID dataId;
            using StdArgs = std::vector<utils::Arg>;
            StdArgs args;

            explicit Callable(StdArgs);

            ~Callable() override = default;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] base::RVM_ID & getTypeID() const override;

            virtual void callSelf() const = 0;

            bool convertToBool() const override;
        };

        struct Function : data::Callable {
            static id::TypeID typeId;
            std::string func_name;
            std::shared_ptr<components::InsSet> func_body;

            explicit Function(std::string, StdArgs, std::shared_ptr<components::InsSet>);

            void callSelf() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool
            compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;
        };

        struct RetFunction : data::Function {
            static id::TypeID typeId;

            RetFunction(std::string, StdArgs, std::shared_ptr<components::InsSet>);

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool
            compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;
        };

        struct Quote : base::RVM_Data {
            static id::TypeID typeId;
            id::DataID dataId;
            std::string quotedDataIDString;
            id::DataID quoteDataID;

            explicit Quote(const id::DataID &quoteDataID);

            ~Quote() override = default;

            std::string getValStr() const override;

            std::string getTypeName() const override;

            base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            void updateQuoteData(const std::shared_ptr<RVM_Data> &newData);

            std::shared_ptr<base::RVM_Data> getQuotedData() const;

            bool convertToBool() const override;
        };

        enum class FileMode {
            Read,
            Write,
            Append,
            ReadWrite,
            ReadAppend,
            WriteAppend
        };

        struct File : base::RVM_Data {
            static id::TypeID typeId;
            id::DataID dataId;
            std::string filepath;
            FileMode fileMode;

            static std::string fileModeToString(const FileMode &fileMode);

            static std::string fileModeToFormatString(const FileMode &fileMode);

            static FileMode stringToFileMode(const std::string &fileModeString);

            explicit File(const FileMode &mode, const std::string &filepath);

            ~File() override = default;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            void setModeByString(const std::string& modeStr);

            [[nodiscard]] virtual std::string readFile() const;

            [[nodiscard]] virtual std::vector<std::string> readFileToLines() const;

            [[nodiscard]] virtual bool writeFile(const std::string &content) const;

            bool convertToBool() const override;
        };

        struct Time : base::RVM_Data {
            static id::TypeID typeId;
            id::DataID dataId = id::DataID();
            int year;
            int month;
            int day;
            int hour;
            int minute;
            int second;
            utils::TimeFormat format = utils::TimeFormat::ISO;

            explicit Time () = default;

            explicit Time(int year, int month, int day, int hour = 0, int minute = 0, int second = 0,
                          const utils::TimeFormat &format = utils::TimeFormat::ISO);

            ~Time() override = default;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            void setFormat(const utils::TimeFormat &format);

            [[nodiscard]] std::string getTimeString() const;

            [[nodiscard]] std::string toString() const override;

            static Time fromString(const std::string &dateString, const utils::TimeFormat &format = utils::TimeFormat::ISO);

            static Time now();

            void addDays(int days);
            void addMonths(int months);
            void addYears(int years);
            void addSeconds(int seconds);
            void addMinutes(int minutes);
            void addHours(int hours);

            bool convertToBool() const override;
        };

    }

    namespace memory {

        /*
         * RA_Space�����ڹ���ռ��ڵ����ݣ�
         * ֧��ͨ���ַ���ID��DataID�������ݵ���ӡ����ҡ����º�ɾ��������
         * ���໹�ṩ�˿ռ����Ƶ����úͻ�ȡ���ܣ�
         * �Լ����������Ĵ�С��ѯ����պ͵�����֧�֡�
         */
        struct RVM_Scope {
        public:
            explicit RVM_Scope(std::string name = "");

            RVM_Scope(std::string name, const std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> &dataMap);

            RVM_Scope(RVM_Scope &&) noexcept = default;

            RVM_Scope &operator=(RVM_Scope &&) noexcept = default;

            RVM_Scope(const RVM_Scope &) = delete;

            RVM_Scope &operator=(const RVM_Scope &) = delete;

            [[nodiscard]] const std::string &getName() const noexcept;

            void setName(std::string name) noexcept;

            bool contains(const std::string &name) const noexcept;

            void addDataByName(const std::string &name, const std::shared_ptr<base::RVM_Data> &data);

            void addDataByID(const id::DataID &dataId, const std::shared_ptr<base::RVM_Data> &data);

            std::shared_ptr<base::RVM_Data> findDataByIdString(const std::string &idString) const;

            std::shared_ptr<base::RVM_Data> findDataByName(const std::string &name) const;

            std::pair<std::optional<id::DataID>, std::shared_ptr<base::RVM_Data>>
            findDataInfoByName(const std::string &name) const;

            std::shared_ptr<base::RVM_Data> findDataByID(const id::DataID &varId) const;

            bool updateDataByID(const id::DataID &id, std::shared_ptr<base::RVM_Data> newData);

            bool updateDataByName(const std::string &name, std::shared_ptr<base::RVM_Data> newData) noexcept;

            std::optional<id::DataID> getDataIDByName(const std::string &name) const;

            bool removeDataByID(const id::DataID &dataId);

            bool removeDataByName(const std::string &name);

            void collectSelf();

            bool thisNeedCollect() const;

            void printInfo() const;

            size_t size() const noexcept;

            bool empty() const noexcept;

            void clear() noexcept;

            base::InstID getInstID() const;

        private:
            base::InstID instID {};
            std::string name_;
            bool needCollect = false;
            std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> dataMap;
            std::unordered_map<std::string, id::DataID> nameMap;
        };

        /*
         * RVM_Memory
         *
         * ����RA_Space���������ںͷ��ʣ��ṩSpace�Ĵ������ͷš����ҵȹ��ܡ�
         *
         * RA_SpacePool��һ�������࣬���ڹ���RA_Space����ĳء���֧�����¹��ܣ�
         * - ��ȡ���е�Space��acquire��
         * - �ͷ�Space��release��
         * - ����ָ��ID�����ݣ�findInSpaces��
         * - ��������ָ�����Ƶ�Space��getAppointedInsSet��
         * - ͨ�����Ʋ���Space��findSpaceByName��
         * - �г���������Space��listNamedSpaces��
         * - �������Pool��reset��
         * - ��ȫ�ͷ�����Space��Դ��releaseAll��
         * - ��ȡ��ǰ��Ծ�Ϳ��е�Space������getActiveScopeCount, getFreeScopeCount��
         * - ��ӡSpace pool��������Ϣ��printPoolInfo��
         * - �����±�����addDataByID��
         * - �������µı�����findDataByName��
         * - ����VarID���ұ�����findDataByID��
         * - ��ȡָ����ʶ�������б�����getAllVars��
         * - ��ȡ���´�����Space�������ƣ�getCurrentScope, getCurrentSpaceName��
         * - ����Ƿ���ڻ�Ծ��Space��hasActiveScope��
         */
        struct

        RVM_Memory {
            std::string main_scope_name = MAIN_SCOPE;

            RVM_Memory(const RVM_Memory &) = delete;

            RVM_Memory &operator=(const RVM_Memory &) = delete;

            RVM_Memory(RVM_Memory &&) = delete;

            RVM_Memory &operator=(RVM_Memory &&) = delete;

            using ScopePtr = std::shared_ptr<RVM_Scope>;
            using DataPtr = std::shared_ptr<base::RVM_Data>;
            using DataPair = std::pair<id::DataID, DataPtr>;

            static constexpr size_t INITIAL_POOL_SIZE = 10;
            static constexpr const char *DEFAULT_SCOPE_PREFIX = "SCOPE-";

            static RVM_Memory &getInstance();

            ScopePtr acquireScope(const std::string &prefix, const std::string &scopeName = "");

            void releaseScope(const ScopePtr &space);

            void releaseScope();

            DataPair addData(const std::string &name, const DataPtr &data, const std::string &scopeName = "");

            DataPair addGlobalData(const std::string &name, const DataPtr &data);

            void addGlobalDataBatch(const std::unordered_map<std::string, DataPtr> &datas);

            [[nodiscard]] DataPtr findDataByID(const id::DataID &dataId) const noexcept;

            [[nodiscard]] DataPair findDataByName(const std::string &name) const noexcept;

            void updateDataByID(const id::DataID &dataId, DataPtr newData);

            void updateDataByIDNoLock(const id::DataID &dataId, DataPtr newData);

            void updateDataByName(const std::string &name, const DataPtr &newData);

            void removeDataByID(const id::DataID &dataId);

            void removeDataByName(const std::string &name);

            [[nodiscard]] bool hasActiveScope() const noexcept;

            [[nodiscard]] size_t getActiveScopeCount() const noexcept;

            [[nodiscard]] ScopePtr getCurrentScope() const noexcept;

            void reset();

            [[nodiscard]] size_t getActiveScopeCount();

            [[nodiscard]] size_t getFreeScopeCount();

            void printPoolInfo();

            [[nodiscard]] ScopePtr acquireScopeNoLock(const std::string &name, bool unnamed);

            void releaseScopeNoLock(const ScopePtr &scope);

            [[nodiscard]] ScopePtr getCurrentScopeNoLock() const noexcept;

            [[nodiscard]] ScopePtr findScopeByNameNoLock(const std::string &name) const;

            void updateDataNoLock(const id::DataID &dataId, const DataPtr &newData) const;

            void updateDataByNameNoLock(const std::string& name, DataPtr newData);

            void removeDataNoLock(const id::DataID &varId);

            void removeDataByNameNoLock(const std::string &name);

            void clearAllScopes();

            void initializePool(size_t size);

            void expandPool(size_t size = INITIAL_POOL_SIZE);

            [[nodiscard]] DataPtr findDataByIDNoLock(const id::DataID &dataId) const;

            [[nodiscard]] DataPair findDataByNameNoLock(const std::string &name) const;

            void start(int intervalMs = 100);

            void stop();

            void setCurrentScopeByName(const std::string &scope_name);

            const ScopePtr &getGlobalScope() const;

        private:
            explicit RVM_Memory(size_t initialSize = INITIAL_POOL_SIZE);

            void collectGarbage();

            std::queue<ScopePtr> freeScopes_;
            std::list<ScopePtr> activeScopes_;
            mutable std::unordered_map<std::string, std::pair<id::DataID, std::weak_ptr<base::RVM_Data>>> recentAccessCache_;
            mutable std::mutex mutex_;
            std::unordered_map<std::string, std::weak_ptr<RVM_Scope>> namedScopes_;
            std::size_t nextDefaultNameId_;
            bool running_;
            bool hasInitialized_ {false};
            mutable ScopePtr globalScope_ = std::make_shared<RVM_Scope>(GLOBAL_SCOPE);
            ScopePtr currentScope_ = nullptr;
        };

        extern RVM_Memory &data_space_pool;
    }
} // namespace core

#endif //RVM_RA_CORE_H
