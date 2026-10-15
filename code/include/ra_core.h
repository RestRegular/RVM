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

            [[nodiscard]] std::shared_ptr<TypeID> getTopParentTypeID() const;

            [[nodiscard]] bool equalWith(const RVM_ID &other) const override;

            [[nodiscard]] std::string getIdentStr() const override;

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
            std::string name_;        // ���Ʋ���
            std::string scopeName_;   // �ռ䲿��
            size_t index_ = -1;       // ��������

            static size_t generateIndex();
        };
    }

    namespace components {
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

            explicit DType(id::TypeID _typeId): dTypeId(std::move(_typeId)){};

            ~DType() override = default;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] bool convertToBool() const override;

            [[nodiscard]] std::shared_ptr<base::RVM_Data> copy_ptr() const override;

            [[nodiscard]] id::TypeID getDTypeID() const;

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

            [[nodiscard]] bool convertToBool() const override;
        };

        extern Null nullInstance; // �ն��󣺹������ط�ʹ��

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

            [[nodiscard]] std::string toEscapedString() const override;
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

            [[nodiscard]] bool convertToBool() const override;

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

            [[nodiscard]] std::shared_ptr<Numeric> pow(const std::shared_ptr<Numeric> &other) const override;

            [[nodiscard]] std::shared_ptr<Numeric> root(const std::shared_ptr<Numeric> &other) const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<base::RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] bool convertToBool() const override;

        private:
            double value;
        };

        struct Bool final : Int {
            static id::TypeID typeId;

            explicit Bool(bool value);

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] id::TypeID &getTypeID() const override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] std::shared_ptr<Numeric> opp() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;
        };

        struct Char final : Int {
            static id::TypeID typeId;

            explicit Char(char value);

            bool updateData(const std::shared_ptr<RVM_Data>& newData) override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] id::TypeID &getTypeID() const override;

            [[nodiscard]] std::string toEscapedString() const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] std::string toString() const override;
        };

        struct Iterable : base::RVM_Data {
            static id::TypeID typeId;

            explicit Iterable();

            ~Iterable() override = default;

            virtual void begin() const = 0;

            virtual void end() const = 0;

            [[nodiscard]] virtual std::shared_ptr<RVM_Data> next() const = 0;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] virtual size_t size() const = 0;

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

            explicit String(std::string  value);

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] id::TypeID &getTypeID() const override;

            void begin() const override;

            void end() const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> next() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] size_t size() const override;

            void append(const std::shared_ptr<RVM_Data> &data) override;

            std::shared_ptr<Iterable> subpart(int begin, int end) override;

            std::shared_ptr<RVM_Data> getDataAt(int index) override;

            void setDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            void eraseDataAt(int index) override;

            [[nodiscard]] std::string toEscapedString() const override;

            [[nodiscard]] std::shared_ptr<List> trans_to_list() const;

            [[nodiscard]] std::shared_ptr<Series> trans_to_series() const;

            void insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            [[nodiscard]] std::string toString() const override;

            [[nodiscard]] bool convertToBool() const override;

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

            [[nodiscard]] size_t size() const override;

            void append(const std::shared_ptr<RVM_Data> &data) override;

            std::shared_ptr<Iterable> subpart(int begin, int end) override;

            [[nodiscard]] virtual std::string getValStdStr(const base::RVM_ID &main_container_id, const std::string &prefix,
                                             const std::string &suffix) const;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            std::shared_ptr<RVM_Data> getDataAt(int index) override;

            void setDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            void eraseDataAt(int index) override;

            [[nodiscard]] std::string toEscapedString() const override;

            [[nodiscard]] const std::vector<std::shared_ptr<RVM_Data>> &getDataList() const;

            void insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] bool convertToBool() const override;

            void splice(const std::shared_ptr<Iterable>& other) override;

            bool contains(const std::shared_ptr<RVM_Data>& data) override;

        protected:
            std::vector<std::shared_ptr<RVM_Data>> dataList{};
        };

        struct Dict final : Iterable {
            static id::TypeID typeId;

            explicit Dict();

            Dict(std::unordered_map<std::string, std::shared_ptr<KeyValuePair>> dataDict,
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

            std::shared_ptr<List> getKeyDataList() const;

            std::shared_ptr<List> getValueDataList() const;

        private:
            std::unordered_map<std::string, std::shared_ptr<KeyValuePair>> dataDict{};
            std::vector<std::string> keyList{};
        };

        struct Series final : List {
            static id::TypeID typeId;

            explicit Series();

            explicit Series(const std::shared_ptr<base::RVM_Data>& list);

            ~Series() override = default;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<Iterable> subpart(int begin, int end) override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] std::string toEscapedString() const override;
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

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] bool convertToBool() const override;

            [[nodiscard]] std::string toEscapedString() const override;

            void setKey(const std::shared_ptr<RVM_Data> &key_);

            void setValue(const std::shared_ptr<RVM_Data> &value_);

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

            /*
             * 槽位模式下的直接数据引用。
             *
             * 旧系统用 DataID 到 data_space_pool 里回查数据；槽位模式下数据在新帧的槽位里，
             * 没有可用的 DataID，所以这里直接持有数据指针。
             * 置位时优先于 DataID——只影响运行期取值，不参与 RSI 序列化。
             */
            std::shared_ptr<RVM_Data> directLeft;
            std::shared_ptr<RVM_Data> directRight;

            CompareGroup(std::shared_ptr<id::DataID> dataLeft, std::shared_ptr<id::DataID> dataRight);

            /* 槽位模式：直接用数据构造，绕过 data_space_pool 回查 */
            CompareGroup(std::shared_ptr<RVM_Data> dataLeft, std::shared_ptr<RVM_Data> dataRight);

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] bool compare(const base::Relational & relational) const;

            [[nodiscard]] bool convertToBool() const override;

        };

        /*
         * CustomType ��ʾһ���Զ�����������ͣ��������������ֶκ�ʵ���ֶ��Լ������ֶΣ�
         * �����ֶ�Ҳ��Ϊ���֣�ʵ�����������ͷ�����
         * �ڲ����游����
         * �����ֶ�ֻ��ͨ�� CustomType �������ӣ�����ͨ�� CustomInst ��������
         * ���������ֶε�����ֻ��ͨ�� CustomType �������ã�����ͨ�� CustomInst ��������
         * ����ʵ���ֶε�����ֻ��ͨ�� CustomInst �������ã�����ͨ�� CustomType ��������
         */
        struct CustomType final : Structure {
            static id::TypeID typeId;
            std::string typeName;
            std::shared_ptr<CustomType> parentType;
            std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> tpFields {};
            std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> instFields {}; // ʾ���ֶο�������Ĭ��ֵ
            std::unordered_set<std::string> methodFields {};

            explicit CustomType(std::string className, std::shared_ptr<CustomType> parentType = nullptr);

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
         * CustomInst ��ʾһ���Զ������������ʵ��
         * ���������ֶ�
         * �����������ֶκͷ���������
         * �����Ի�ȡʵ���ֶΣ�Ҳ���Ի�ȡ�����ֶ�
         */
        struct CustomInst final : Structure {
            static id::TypeID typeId;
            std::unordered_map
            <std::string, std::unordered_map
            <std::string, std::shared_ptr<base::RVM_Data>>> instFields; //instFields: �洢ʵ���� CustomType �ͼ̳еĸ�����Ԥ�������ʵ���ֶ�
            std::shared_ptr<CustomType> customType;

            explicit CustomInst(std::shared_ptr<CustomType> instType);

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            std::shared_ptr<RVM_Data> copy_ptr() const override;

            base::RVM_ID &getTypeID() const override;

            std::string getValStr() const override;

            std::string toString() const override;

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

            [[nodiscard]] virtual core::components::ExecutionStatus callSelf() const = 0;

            [[nodiscard]] bool convertToBool() const override;
        };

        struct Function : data::Callable {
            static id::TypeID typeId;
            std::string func_name;
            std::shared_ptr<components::InsSet> func_body;

            explicit Function(std::string, StdArgs, std::shared_ptr<components::InsSet>);

            [[nodiscard]] core::components::ExecutionStatus callSelf() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool
            compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;
        };

        struct RetFunction final : data::Function {
            static id::TypeID typeId;

            RetFunction(std::string, StdArgs, std::shared_ptr<components::InsSet>);

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            [[nodiscard]] bool
            compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;
        };

        struct Quote final : base::RVM_Data {
            static id::TypeID typeId;
            id::DataID dataId;
            std::string quotedDataIDString;
            id::DataID quoteDataID;

            explicit Quote(const id::DataID &quoteDataID);

            ~Quote() override = default;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            void updateQuoteData(const std::shared_ptr<RVM_Data> &newData) const;

            [[nodiscard]] std::shared_ptr<RVM_Data> getQuotedData() const;

            [[nodiscard]] bool convertToBool() const override;
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

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

            void setModeByString(const std::string& modeStr);

            [[nodiscard]] virtual std::string readFile() const;

            [[nodiscard]] virtual std::vector<std::string> readFileToLines() const;

            [[nodiscard]] virtual bool writeFile(const std::string &content) const;

            [[nodiscard]] bool convertToBool() const override;
        };

        struct Time final : base::RVM_Data {
            static id::TypeID typeId;
            id::DataID dataId = id::DataID();
            int year{};
            int month{};
            int day{};
            int hour{};
            int minute{};
            int second{};
            utils::TimeFormat format = utils::TimeFormat::ISO;

            explicit Time () = default;

            explicit Time(int year, int month, int day, int hour = 0, int minute = 0, int second = 0,
                          const utils::TimeFormat &format = utils::TimeFormat::ISO);

            ~Time() override = default;

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;

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

            [[nodiscard]] bool convertToBool() const override;
        };

        struct Error final : base::RVM_Data {
            static id::TypeID typeId;
            id::DataID dataId = id::DataID();
            std::string error_position;
            std::string error_line;
            std::vector<std::string> error_info;
            std::shared_ptr<base::errors::ExposedError> error;

            Error(std::string error_position, std::string error_line, std::vector<std::string> error_info);

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string toString() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] bool convertToBool() const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;
        };

    }

    namespace memory {
        extern std::unordered_map<std::string, std::shared_ptr<data::Extension>> loadedExtensions;

        struct RVM_Scope {
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

            void activate();

            void deactivate();

            bool isActive() const;

        private:
            base::InstID instID {};
            std::string name_;
            bool needCollect = false;
            bool isActive_ = false;
            std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> dataMap;
            std::unordered_map<std::string, id::DataID> nameMap;
            bool addable = true;
            bool updatable = true;
            bool deletable = true;
            bool removable = true;
            bool findable = true;
        };

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

            /*
             * 槽位模式的写入路径：目标就是参数本身，地址 (帧下标, 槽位号) 挂在它上面。
             * 不做任何名字查找——这是静态槽位架构下唯一的写入方式。
             */
            void updateDataByArg(const utils::Arg &arg, DataPtr newData) const;

            /*
             * 两套模式共用的写入入口：
             * 槽位模式按参数上挂的地址写；旧系统按名字写。
             * 执行器只拿到参数，名字其实就是 arg.getValue()。
             */
            void updateDataByArgOrName(const utils::Arg &arg, const std::string &name,
                                       DataPtr newData) const;

            /*
             * 按名字写一个【全局】数据（SE / _SE 这类由 RVM 自己维护的槽位）。
             *
             * 这类目标在 RA 源码里没有对应的参数，所以拿不到 Arg；但它们必然是根帧的槽位，
             * 可以直接按名字查出帧内槽位号。只用于 RVM 内部，不是给程序变量用的后门。
             */
            void updateGlobalByName(const std::string &name, DataPtr newData) const;

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

            ScopePtr
            findOperableScopeByDataName(const std::string &name, bool needContains, const base::OpMode &opMode) const;
        };

        /*
         * 无效下标哨兵。帧下标与槽位号共用——两者都是「必须由解析期填好，
         * 否则运行期直接报错」的地址信息。
         */
        constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();

        struct RVM_ParseScope
        {
            RVM_ParseScope(size_t startIndex_, RVM_ParseScope* parentScope);

            /*
             * 解析期寄存器：把一个变量名登记到本作用域的槽位。
             *
             * - 同一个作用域内重复登记同名变量时，直接返回已分配的槽位号，
             *   不会重复占用槽位（幂等）。
             * - 返回的是本作用域内的局部槽位号（0 起），不是帧内的全局槽位号。
             *   要拿帧内全局槽位号请用 getFrameSlotIndex()。
             */
            size_t registerSlot(const std::string &name);

            /* 本作用域是否登记过该名字（不向上查找父作用域） */
            [[nodiscard]] bool hasSlotItself(const std::string &name) const;

            /* 仅在本作用域内查找槽位号，找不到返回 nullopt */
            [[nodiscard]] std::optional<size_t> findSlotItself(const std::string &name) const;

            /* 沿 parentScope 链向上查找槽位号，返回帧内全局槽位号；找不到返回 nullopt */
            [[nodiscard]] std::optional<size_t> resolveSlot(const std::string &name) const;

            /*
             * 收集本作用域及其所有父作用域可见的全部符号。
             * 链表顺序为「内层优先」：先写入的是内层作用域的名字。
             * 同名符号只保留最内层的那个（符合遮蔽语义）。
             */
            void collectVisibleSlots(std::unordered_map<std::string, size_t> &out) const;

            /*
             * 只收集「本作用域所属帧内」可见的符号，在帧根处停止向上查找。
             * 与 collectVisibleSlots 的区别：不会把帧外的名字混进来。
             */
            void collectFrameVisibleSlots(std::unordered_map<std::string, size_t> &out) const;

            /* 本作用域所属帧的帧根作用域（若自身就是帧根则返回自身） */
            [[nodiscard]] const RVM_ParseScope *getFrameRootScope() const;

            /* 本作用域是否是一个帧的根（FUNC / FUNI 等）；帧根意味着槽位空间从头开始 */
            [[nodiscard]] bool isFrameRoot() const;

            /* 标记本作用域为帧根 */
            void markAsFrameRoot();

            /* 本作用域的局部槽位号 → 帧内全局槽位号 */
            [[nodiscard]] size_t getFrameSlotIndex(size_t localSlotIndex) const;

            /* 本作用域已分配的槽位数量 */
            [[nodiscard]] size_t getSlotCount() const;

            /* 本作用域在所属帧内的起始偏移 */
            [[nodiscard]] size_t getStartIndex() const;

            [[nodiscard]] const RVM_ParseScope* getParentScope() const;

            /* 登记顺序的槽位名表，下标即局部槽位号 */
            [[nodiscard]] const std::vector<std::string> &getSlotNames() const;

            /* 本作用域的名字 → 局部槽位号 映射表 */
            [[nodiscard]] const std::unordered_map<std::string, size_t> &getNameToSlot() const;

            /*
             * 词法（静态）名字解析：沿 parentScope 链向上查找，返回名字所在的词法作用域。
             *
             * 与 resolveSlot 的区别：resolveSlot 只返回帧槽位号；本函数还返回是哪个作用域提供的，
             * 从而能区分「同一个帧内的不同作用域」——这在判断遮蔽与跨帧可见性时是必需的，
             * 因为不同帧的槽位号会重复（每个帧都从 0 开始）。
             *
             * parentScope 在解析期指向的正是「定义处的外层作用域」，所以这条链天然就是词法链：
             * FUNC / FUNI 体内使用的自由名字会解析到函数定义处的外层作用域，
             * 而不是调用处的动态作用域。
             */
            [[nodiscard]] const RVM_ParseScope *resolveLexically(const std::string &name) const;

            /*
             * 打印「从本作用域出发，各名字按词法解析到哪个帧的哪个槽位」。
             *
             * 覆盖本词法链上的每一个名字（含外层提供的、本作用域并未声明的名字），
             * 用「声明处的作用域链路径」标识归属，从而能区分不同帧——因为不同帧的
             * 槽位号会重复（每个帧都从 0 开始），只给槽位号无法判断。
             */
            void printLexicalResolution(std::ostream &os = std::cout) const;

            /* 词法链上全部可见名字 → 声明该名字的作用域（内层优先，已处理遮蔽） */
            [[nodiscard]] std::vector<std::pair<std::string, const RVM_ParseScope *>>
            collectLexicalBindings() const;

            /* 开启本作用域的指令名（FUNC / REPEAT / ITER_TRAV ...），仅用于可读输出 */
            void setOpeningInsName(const std::string &name);

            [[nodiscard]] const std::string &getOpeningInsName() const;

            /* 从文件根作用域到本作用域的路径，例如 "root/REPEAT/UNTIL" */
            [[nodiscard]] std::string getScopePath() const;

            /*
             * 本作用域对应的 InsSet 标识（来自 rvm_ris 的 InsSet::getInstID()）。
             * 运行期用它把「正在执行的指令集」映射回解析期作用域，从而知道当前帧是什么。
             */
            void setInsSetId(std::string insSetId);
            [[nodiscard]] const std::string &getInsSetId() const;

            /*
             * 沿 parentScope 链找到路径为 scopePath 的那个作用域。
             *
             * 运行期用它把「解析期记录的帧路径」翻译回一个具体的作用域对象，
             * 从而知道该帧的槽位规模、以及是本作用域自己声明还是外层声明。
             */
            [[nodiscard]] const RVM_ParseScope *findScopeByPath(const std::string &scopePath) const;

            /* 本帧（含本作用域之后关闭的子作用域）的最大槽位跨度，即运行期 slots 数组的最小安全长度 */
            [[nodiscard]] size_t getFrameSlotSpan() const;

            /*
             * 本作用域所属帧的运行期下标（仅帧根有效，非帧根返回 INVALID_INDEX）。
             * 解析期在建立运行期帧表后统一分配，随后写进该帧内所有参数的槽位引用。
             */
            void setFrameIndex(uint32_t index);
            [[nodiscard]] uint32_t getFrameIndex() const;

            /* 以人类可读形式打印本作用域的槽位分配情况（调试用，不参与运行） */
            void printInfo(std::ostream &os = std::cout) const;

            /* 打印本作用域及其所有父作用域的槽位分配情况（调试用，不参与运行） */
            void printScopeChainInfo(std::ostream &os = std::cout) const;

        private:
            RVM_ParseScope* parentScope;
            std::vector<std::string> slotNames; // slotNames.size() 作为本作用域的槽位数量
            std::unordered_map<std::string, size_t> nameToSlot;
            size_t startIndex; // 相对于所属帧的起始偏移
            bool frameRoot = false; // 是否为帧根（帧根之间的槽位空间互相独立）
            std::string openingInsName{"root"}; // 开启本作用域的指令名，仅用于可读输出
            std::string insSetId{}; // 对应 InsSet 的 InstID，用于解析期诊断
            /*
             * 本作用域所属帧的运行期下标（仅帧根有效）。
             * 解析期统一分配后，会写进该帧根作用域里所有参数的 (frameIndex, slotIndex)，
             * 运行期直接 frames[idx]->slots[n] 取值，不做任何查找。
             */
            uint32_t frameIndex{INVALID_INDEX};
        };

        struct RVM_RuntimeScope
        {
            RVM_RuntimeScope(size_t slotCount, RVM_RuntimeScope* parentScope_);

            explicit RVM_RuntimeScope(size_t slotCount, RVM_RuntimeScope* parentScope_,
                                      std::string framePath_);

            /* 槽位读写。越界视为错误（返回 nullptr / 失败），不静默扩展数组。 */
            [[nodiscard]] const std::shared_ptr<base::RVM_Data> &getSlot(size_t frameSlot) const;
            [[nodiscard]] bool setSlot(size_t frameSlot, std::shared_ptr<base::RVM_Data> data);
            [[nodiscard]] bool hasSlot(size_t frameSlot) const;

            /* 槽位数组长度（= 帧内槽位规模），按需增长 */
            [[nodiscard]] size_t getSlotCount() const;
            void ensureSlotCount(size_t slotCount);

            /*
             * 清空全部槽位（保留数组容量）。
             * 函数每次进入都要复位帧：参数与局部变量必须重新绑定，不能残留上次调用的值。
             * 注意这也会清掉该帧里 ALLOT 出来的实体，所以 ALLOT 之后的代码负责重新放置。
             */
            void resetSlots();

            [[nodiscard]] RVM_RuntimeScope* getParentScope() const;
            void setParentScope(RVM_RuntimeScope* parentScope_);

            [[nodiscard]] const std::string &getFramePath() const;
            void setFramePath(std::string framePath_);

            /* 本帧是否为解析期的帧根（函数帧）；文件根帧也算帧根 */
            [[nodiscard]] bool isFrameRoot() const;
            void markAsFrameRoot();

            /* 调试输出（不参与运行逻辑） */
            void printInfo(std::ostream &os = std::cout) const;

        private:
            std::vector<std::shared_ptr<base::RVM_Data>> slots;
            RVM_RuntimeScope* parentScope = nullptr;
            std::string framePath{};   // 解析期的帧路径，如 "root" / "root/FUNC"
            bool frameRoot = false;    // 是否为帧根
        };

        struct RVM_RuntimeMemory
        {
            RVM_RuntimeMemory() = default;
            static RVM_RuntimeMemory &getInstance();

            /*
             * 帧注册表：用【下标】寻址。
             *
             * 静态槽位架构里，参数在解析期就被写上了 (帧下标, 槽位号)，
             * 运行期取值只需 frames[frameIndex]->slots[slotIndex]——两次数组索引，
             * 没有任何字符串或哈希查找。所以帧的身份必须是下标而不是路径。
             *
             * framePathIndex 只在解析期/诊断里用（按路径找帧、打印），运行期不碰。
             */
            [[nodiscard]] uint32_t addFrame(const std::shared_ptr<RVM_RuntimeScope> &frame,
                                            const std::string &framePath);
            [[nodiscard]] const std::shared_ptr<RVM_RuntimeScope> &frameAt(uint32_t frameIndex) const;
            [[nodiscard]] size_t frameCount() const;

            /* 按路径查帧下标（解析期/诊断用）；找不到返回 INVALID_INDEX */
            [[nodiscard]] uint32_t findFrameIndexByPath(const std::string &framePath) const;

            /*
             * 文件根帧的下标。SE / _SE 这类 RVM 内部全局槽位都住在根帧里，
             * 而它们在 RA 源码里没有对应参数，所以只能按这个下标 + 槽位名去定位。
             */
            void setRootFrameIndex(uint32_t frameIndex);
            [[nodiscard]] uint32_t getRootFrameIndex() const;

            /* 登记/写入根帧的槽位名（SR / _SE / SE 这类 RVM 内部全局槽位） */
            void setRootSlotByName(const std::string &name, uint32_t slotIndex);
            [[nodiscard]] bool writeRootSlotByName(const std::string &name,
                                                   std::shared_ptr<base::RVM_Data> data) const;
            /* 按名字读根帧槽位。名字未登记或槽位为空时返回 nullptr。 */
            [[nodiscard]] std::shared_ptr<base::RVM_Data>
            readRootSlotByName(const std::string &name) const;

            /* 当前正在执行的帧下标 */
            void setCurrentFrameIndex(uint32_t frameIndex);
            [[nodiscard]] uint32_t getCurrentFrameIndex() const;

            /* 帧切换栈：进入/退出一个指令集时保存并恢复当前帧下标 */
            void pushCurrentFrameIndex(uint32_t frameIndex);
            void popCurrentFrameIndex();

            /* 运行期帧是否已经建立（由解析期帧表一次性建好） */
            [[nodiscard]] bool isFrameTableReady() const;
            void markFrameTableReady();

            /* 槽位读写：帧下标无效或槽位越界都返回失败，由调用方决定如何报错 */
            [[nodiscard]] bool writeSlot(uint32_t frameIndex, uint32_t frameSlot,
                                         std::shared_ptr<base::RVM_Data> data) const;
            [[nodiscard]] std::shared_ptr<base::RVM_Data>
            readSlot(uint32_t frameIndex, uint32_t frameSlot) const;

            /* 清理全部帧注册（程序结束/重新解析时调用） */
            void clearAllFrames();

            /* 调试输出 */
            void printInfo(std::ostream &os = std::cout) const;

        private:
            std::vector<std::shared_ptr<RVM_RuntimeScope>> frames;
            std::unordered_map<std::string, uint32_t> framePathIndex;
            /* 解析期登记的「帧内槽位名 → 槽位号」，只用于 RVM 内部全局槽位 */
            std::unordered_map<std::string, uint32_t> rootSlotByName;
            std::vector<uint32_t> currentFrameStack;
            uint32_t rootFrameIndex = INVALID_INDEX;
            uint32_t currentFrameIndex = INVALID_INDEX;
            bool frameTableReady = false;
        };

        extern RVM_Memory &data_space_pool;
        extern RVM_RuntimeMemory &RVM_RUNTIME_MEMORY;

        /*
         * 槽位寻址总开关。
         *
         * 由调用方决定（CLI 的 --slot-mode / --legacy-mode）；解析期只负责把
         * (帧下标, 槽位号) 挂到参数上，不自己置位。
         */
        void setSlotBindingModeEnabled(bool enabled);
        [[nodiscard]] bool isSlotBindingModeEnabled();

        /*
         * 按 (帧下标, 槽位号) 读写槽位。
         *
         * 这是运行期唯一的取变量方式：解析期已把地址写进 utils::Arg，
         * 运行期直接顺着参数读，不做任何名字或字符串查找。
         *
         * 帧下标无效或槽位越界都抛错，不回退——静默回退会把解析期模型错误藏起来。
         */
        void writeArgSlot(const utils::Arg &arg, std::shared_ptr<base::RVM_Data> data);
        [[nodiscard]] std::shared_ptr<base::RVM_Data> readArgSlot(const utils::Arg &arg);

        /*
         * 构造「槽位模型错误」异常。
         * 这类错误必须与程序自身的数据错误区分开，否则会被上层包装成
         * "内存空间不存在" 而掩盖真正原因。
         */
        [[nodiscard]] base::RVM_Error makeSlotModelError(const std::string &phase,
                                                        const std::string &argName);
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

            [[nodiscard]] std::string getValStr() const override;

            [[nodiscard]] std::string getTypeName() const override;

            [[nodiscard]] base::RVM_ID &getTypeID() const override;

            bool updateData(const std::shared_ptr<RVM_Data> &newData) override;

            [[nodiscard]] bool compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const override;

            [[nodiscard]] bool convertToBool() const override;

            [[nodiscard]] std::shared_ptr<RVM_Data> copy_ptr() const override;
        };

    }
} // namespace core

#endif //RVM_RA_CORE_H
