//
// Created by RestRegular on 2025/1/15.
//

#ifndef RVM_RA_CORE_H
#define RVM_RA_CORE_H

#include <ranges>
#include <string>
#include <utility>
#include <vector>
#include <functional>
#include <list>
#include <algorithm>
#include "ra_base.h"
#include "lib/ra_utils.h"

namespace core {
    namespace id {
        struct TypeID final : base::RVM_ID {
            std::string type_name;
            std::string dtype_ident;

            explicit TypeID(std::string type_name, const base::IDType &idType, const std::string &dtype_ident);

            explicit TypeID(std::string type_name, const std::shared_ptr<TypeID> &parent_type,
                            const base::IDType &idType, const std::string &dtype_ident);

            void printInfo() const override;

            [[nodiscard]] std::string toString() const override;

            [[nodiscard]] std::string toString(const std::string &detail) const override;

            bool operator==(const RVM_ID &other) const override;

            std::shared_ptr<TypeID> getTopParentTypeID() const;

            bool equalWith(const RVM_ID &other) const override;

            std::string getIdentStr() const override;

        private:
            std::shared_ptr<TypeID> parent_type_id;
        };

        struct RIID final : base::RVM_ID {
            explicit RIID();

            void printInfo() const override;

            [[nodiscard]] std::string toString() const override;
        };

        struct InsID : base::RVM_ID {
            explicit InsID();

            void printInfo() const override;

            [[nodiscard]] std::string toString() const override;

        };

        struct DataID final : base::RVM_ID {
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
            std::string name_;        // 名称部分
            std::string scopeName_;   // 空间部分
            size_t index_ = -1;       // 索引部分

            static size_t generateIndex();
        };
    }

    namespace components {
        // 一些必要组件的声明
        enum class ExecutionStatus;
        struct RI;
        struct Ins;
        struct InsSet;
        struct DetectBlock;
        struct AtmpBlock;
        using RIID [[maybe_unused]] = id::RIID;
        using InsID = id::InsID;
        using StdArgs = std::vector<utils::Arg>;
        typedef ExecutionStatus (*ExecutorFunc)(const Ins&, size_t&, const StdArgs&);
    }

    namespace data {
        struct DType;
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
        struct Error;
        struct Extension;

        struct DType final : base::RVM_Data {
            static id::TypeID typeId;

            explicit DType() = delete;

            explicit DType(const id::TypeID &_typeId): dTypeId(_typeId){};

            ~DType() override = default;

            std::string getValStr() const override;

            std::string getTypeName() const override;

            base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            bool convertToBool() const override;

            std::shared_ptr<base::RVM_Data> copy_ptr() const override;

            id::TypeID getDTypeID() const;

        protected:
            id::TypeID dTypeId;
        };

        struct Null final : base::RVM_Data {
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

        extern Null nullInstance; // 空对象：供其他地方使用

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

            [[nodiscard]] virtual std::shared_ptr<Numeric> mod(const std::shared_ptr<Numeric> &other) const = 0;

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

            [[nodiscard]] std::shared_ptr<Numeric> mod(const std::shared_ptr<Numeric> &other) const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            bool convertToBool() const override;

        protected:
            int value;
        };

        struct Float final : Numeric {
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

            [[nodiscard]] std::shared_ptr<Numeric> mod(const std::shared_ptr<Numeric> &other) const override;

            std::shared_ptr<Numeric> pow(const std::shared_ptr<Numeric> &other) const override;

            std::shared_ptr<Numeric> root(const std::shared_ptr<Numeric> &other) const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<base::RVM_Data> &other, const base::Relational &relational) const override;

            bool convertToBool() const override;

        private:
            double value;
        };

        struct Bool final : Int {
            static id::TypeID typeId;

            explicit Bool(bool value);

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] id::TypeID &getTypeID() const override;

            std::string getValStr() const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            std::shared_ptr<Numeric> opp() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;
        };

        struct Char final : Int {
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

            virtual void splice(const std::shared_ptr<Iterable> &other) = 0;

            virtual bool contains(const std::shared_ptr<RVM_Data> &data) = 0;
        };

        struct String final : Iterable {
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

            void splice(const std::shared_ptr<Iterable>& other) override;

            bool contains(const std::shared_ptr<RVM_Data>& data) override;

        private:
            std::string value;
        };

        struct List : Iterable {
            static id::TypeID typeId;

            explicit List();

            explicit List(const std::vector<std::shared_ptr<RVM_Data>> &dataList);

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

            const std::vector<std::shared_ptr<RVM_Data>> &getDataList() const;

            void insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            base::RVM_ID &getTypeID() const override;

            bool convertToBool() const override;

            void splice(const std::shared_ptr<Iterable>& other) override;

            bool contains(const std::shared_ptr<RVM_Data>& data) override;

        protected:
            std::vector<std::shared_ptr<RVM_Data>> dataList{};
        };

        struct Dict final : Iterable {
            static id::TypeID typeId;

            explicit Dict();

            Dict (std::unordered_map<std::string, std::shared_ptr<KeyValuePair>> dataDict,
                  std::vector<std::string> keyList);

            explicit Dict (const std::shared_ptr<List>& list);

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

            void setDataAt(const std::string& key, const std::shared_ptr<KeyValuePair> &data);

            void eraseDataAt(int index) override;

            void eraseDataAt(const std::string& key);

            std::string getValStr() const override;

            std::string getTypeName() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            std::string toEscapedString() const override;

            void insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            base::RVM_ID &getTypeID() const override;

            bool convertToBool() const override;

            void splice(const std::shared_ptr<Iterable>& other) override;

            bool contains(const std::shared_ptr<RVM_Data>& data) override;

        private:
            std::unordered_map<std::string, std::shared_ptr<KeyValuePair>> dataDict{};
            std::vector<std::string> keyList{};
        };

        struct Series final : List {
            static id::TypeID typeId;

            explicit Series();

            Series(const std::shared_ptr<base::RVM_Data>& list);

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

        struct KeyValuePair final : Structure {
            static id::TypeID typeId;

            explicit KeyValuePair();

            KeyValuePair(std::shared_ptr<RVM_Data> key, std::shared_ptr<RVM_Data> value);

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            base::RVM_ID &getTypeID() const override;

            std::string getValStr() const override;

            std::string getTypeName() const override;

            bool convertToBool() const override;

            [[nodiscard]] std::string toEscapedString() const override;

            void setKey(const std::shared_ptr<RVM_Data> &key);

            void setValue(const std::shared_ptr<RVM_Data> &value);

            [[nodiscard]] std::shared_ptr<RVM_Data> getKey() const;

            [[nodiscard]] std::shared_ptr<RVM_Data> getValue() const;

            [[nodiscard]] std::string toKVPairString() const;
        private:
            std::shared_ptr<RVM_Data> key = nullptr;
            std::shared_ptr<RVM_Data> value = nullptr;
        };

        struct CompareGroup final : Structure{
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

            [[nodiscard]] bool compare(const base::Relational & relational) const;

            bool convertToBool() const override;

        };

        /*
         * CustomType 表示一种自定义的数据类型，可以添加类型字段和实例字段以及方法字段，
         * 方法字段也分为两种：实例方法和类型方法。
         * 内部保存父类型
         * 添加字段只能通过 CustomType 进行添加，不能通过 CustomInst 进行添加
         * 设置类型字段的数据只能通过 CustomType 进行设置，不能通过 CustomInst 进行设置
         * 设置实例字段的数据只能通过 CustomInst 进行设置，不能通过 CustomType 进行设置
         */
        struct CustomType: Structure {
            static id::TypeID typeId;
            std::string typeName;
            std::shared_ptr<CustomType> parentType;
            std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> tpFields {};
            std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> instFields {}; // 示例字段可以设置默认值
            std::unordered_set<std::string> methodFields {};

            CustomType(std::string className, std::shared_ptr<CustomType> parentType = nullptr);

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] std::string getValStr() const override;

            base::RVM_ID &getTypeID() const override;

            std::string getTypeName() const override;

            [[nodiscard]] bool hasField(const std::string &fieldName) const;

            [[nodiscard]] bool hasInstField(const std::string &fieldName);

            [[nodiscard]] bool hasTpField(const std::string &fieldName) const;

            [[nodiscard]] bool hasFieldItself(const std::string &fieldName) const;

            [[nodiscard]] bool hasTpFieldItself(const std::string &fieldName) const;

            [[nodiscard]] bool hasInstFieldItself(const std::string &fieldName) const;

            void travalTypes(const std::function<bool(std::shared_ptr<CustomType>)>& callback) const;

            [[nodiscard]] std::shared_ptr<base::RVM_Data> getTpField(const std::string &fieldName);

            void setTpField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData);

            void addTpField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData = nullptr);

            void addInstField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData = nullptr);

            [[nodiscard]] bool checkBelongTo(const std::shared_ptr<CustomType> &type) const;

            bool convertToBool() const override;

            [[nodiscard]] std::string getTypeIDString() const;
        };

        /*
         * CustomInst 表示一种自定义的数据类型实例
         * 不能添加字段
         * 它可以设置字段和方法的数据
         * 它可以获取实例字段，也可以获取类型字段
         */
        struct CustomInst : Structure {
            static id::TypeID typeId;
            std::unordered_map
            <std::string, std::unordered_map
            <std::string, std::shared_ptr<base::RVM_Data>>> instFields; //instFields: 存储实例的 CustomType 和继承的父类中预设的所有实例字段
            std::shared_ptr<CustomType> customType;

            CustomInst(std::shared_ptr<CustomType> instType);

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            base::RVM_ID &getTypeID() const override;

            std::string getValStr() const override;

            std::string getTypeName() const override;

            bool hasField(const std::string &fieldName) const;

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

            virtual core::components::ExecutionStatus callSelf() const = 0;

            bool convertToBool() const override;
        };

        struct Function : data::Callable {
            static id::TypeID typeId;
            std::string func_name;
            std::shared_ptr<components::InsSet> func_body;

            explicit Function(std::string, StdArgs, std::shared_ptr<components::InsSet>);

            core::components::ExecutionStatus callSelf() const override;

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

            void updateQuoteData(const std::shared_ptr<RVM_Data> &newData) const;

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

        struct Error: base::RVM_Data {
            static id::TypeID typeId;
            id::DataID dataId = id::DataID();
            std::string error_position;
            std::string error_line;
            std::vector<std::string> error_info;
            std::shared_ptr<base::errors::ExposedError> error;

            Error(std::string error_position, std::string error_line, std::vector<std::string> error_info);

            std::string getValStr() const override;

            std::string toString() const override;

            std::string getTypeName() const override;

            base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            bool convertToBool() const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;
        };

    }

    namespace memory {
        extern std::unordered_map<std::string, std::shared_ptr<core::data::Extension>> loadedExtensions;

        /*
         * RA_Space类用于管理空间内的数据，
         * 支持通过字符串ID和DataID进行数据的添加、查找、更新和删除操作。
         * 该类还提供了空间名称的设置和获取功能，
         * 以及数据容器的大小查询、清空和迭代器支持。
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

            void checkOpPermission(const base::OpMode &opMode) const;

            bool contains(const std::string &name) const noexcept;

            void addDataByName(const std::string &name, const std::shared_ptr<base::RVM_Data> &data);

            void addDataByID(const id::DataID &dataId, const std::shared_ptr<base::RVM_Data> &data);

            std::shared_ptr<base::RVM_Data> findDataByIdString(const std::string &idString) const;

            std::shared_ptr<base::RVM_Data> findDataByName(const std::string &name) const;

            std::pair<std::optional<id::DataID>, std::shared_ptr<base::RVM_Data>>
            findDataInfoByName(const std::string &name) const;

            std::shared_ptr<base::RVM_Data> findDataByID(const id::DataID &varId) const;

            bool updateDataByID(const id::DataID &id, const std::shared_ptr<base::RVM_Data>& newData);

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

            bool isModifiable() const;

            void setModification(const std::tuple<bool, bool, bool, bool> &modification);

            bool getOpPermission(const base::OpMode &opMode) const;

            bool isAddable() const;

            void setAddable(bool isAddable);

            bool isUpdatable() const;

            void setUpdatable(bool isUpdatable);

            bool isDeletable() const;

            void setDeletable(bool isDeletable);

            bool isFindable() const;

            void setFindable(bool isFindable);

            bool isRemovable() const;

            void setRemovable(bool isRemovable);

        private:
            base::InstID instID {};
            std::string name_;
            bool needCollect = false;
            std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> dataMap;
            std::unordered_map<std::string, id::DataID> nameMap;
            bool addable = true;
            bool updatable = true;
            bool deletable = true;
            bool removable = true;
            bool findable = true;
        };

        /*
         * RVM_Memory
         *
         * 管理RA_Space的生命周期和访问，提供Space的创建、释放、查找等功能。
         *
         * RA_SpacePool是一个单例类，用于管理RA_Space对象的池。它支持以下功能：
         * - 获取空闲的Space（acquire）
         * - 释放Space（release）
         * - 查找指定ID的数据（findInSpaces）
         * - 创建具有指定名称的Space（getAppointedInsSet）
         * - 通过名称查找Space（findSpaceByName）
         * - 列出所有命名Space（listNamedSpaces）
         * - 清空整个Pool（reset）
         * - 完全释放所有Space资源（releaseAll）
         * - 获取当前活跃和空闲的Space数量（getActiveScopeCount, getFreeScopeCount）
         * - 打印Space pool的所有信息（printPoolInfo）
         * - 创建新变量（addDataByID）
         * - 查找最新的变量（findDataByName）
         * - 根据VarID查找变量（findDataByID）
         * - 获取指定标识符的所有变量（getAllVars）
         * - 获取最新创建的Space及其名称（getCurrentScope, getCurrentSpaceName）
         * - 检查是否存在活跃的Space（hasActiveScope）
         */
        struct RVM_Memory {
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

            bool checkScopeExist(const std::string &scopeName) const;

            bool checkScopeExist(const ScopePtr &space) const;

            void removeScope(const ScopePtr &space);

            void removeScope();

            void appendScope(const ScopePtr &space);

            DataPair addData(const std::string &name, const DataPtr &data, const std::string &scopeName = "") const;

            DataPair addGlobalData(const std::string &name, const DataPtr &data) const;

            void addGlobalDataBatch(const std::unordered_map<std::string, DataPtr> &datas) const;

            [[nodiscard]] DataPtr findDataByID(const id::DataID &dataId) const noexcept;

            [[nodiscard]] DataPair findDataByName(const std::string &name) const noexcept;

            void updateDataByID(const id::DataID &dataId, DataPtr newData) const;

            void updateDataByIDNoLock(const id::DataID &dataId, DataPtr newData) const;

            void updateDataByName(const std::string &name, const DataPtr &newData) const;

            void removeDataByID(const id::DataID &dataId) const;

            void removeDataByName(const std::string &name);

            [[nodiscard]] bool hasActiveScope() const noexcept;

            [[nodiscard]] size_t getActiveScopeCount() const noexcept;

            [[nodiscard]] ScopePtr getCurrentScope() const noexcept;

            void reset();

            [[nodiscard]] size_t getFreeScopeCount() const;

            void printPoolInfo();

            [[nodiscard]] ScopePtr acquireScopeNoLock(const std::string &name, bool unnamed);

            void releaseScopeNoLock(const ScopePtr &scope);

            bool checkScopeExistNoLock(const std::string &scopeName) const;

            bool checkScopeExistNoLock(const ScopePtr &space) const;

            void removeScopeNoLock(const ScopePtr &scope);

            void appendScopeNoLock(const ScopePtr &space);

            [[nodiscard]] ScopePtr getCurrentScopeNoLock() const noexcept;

            [[nodiscard]] ScopePtr findScopeByNameNoLock(const std::string &name) const;

            void updateDataNoLock(const id::DataID &dataId, const DataPtr &newData) const;

            void updateDataByNameNoLock(const std::string& name, DataPtr newData) const;

            void removeDataNoLock(const id::DataID &varId) const;

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

            RVM_Memory::ScopePtr
            findOperableScopeByDataName(const std::string &name, bool needContains, const base::OpMode &opMode) const;
        };

        extern RVM_Memory &data_space_pool;
    }

    namespace data {

        enum class ExtensionType {
            RA,
            RSI,
            UNKNOWN
        };

        namespace ExtensionTypeTools {
            [[nodiscard]] ExtensionType getExtensionType(const std::string &extension_path);
            [[nodiscard]] std::string getExtensionTypeName(ExtensionType type);
        }

        struct Extension: base::RVM_Data {
            static id::TypeID typeId;
            id::DataID dataId = id::DataID();
            ExtensionType extension_type;
            std::string extension_name;
            std::string extension_ident;
            std::string extension_path;

            using ScopePtr = std::shared_ptr<memory::RVM_Scope>;

            ScopePtr extension_scope;

            Extension(const std::string &ident, const std::string &extension_name, const std::string &extension_path);

            Extension(const std::string &ident, const std::string &extension_path);

            Extension(const std::string &ident, const std::string &extension_path, const ScopePtr &scope);

            std::string getValStr() const override;

            std::string getTypeName() const override;

            base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            bool convertToBool() const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;
        };

    }
} // namespace core

#endif //RVM_RA_CORE_H
