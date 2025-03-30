//
// Created by RestRegular on 2025/1/15.
//

#include <functional>
#include <cmath>
#include <utility>
#include <cstdio>
#include "ra_core.h"

core::memory::RVM_Memory &core::memory::data_space_pool = RVM_Memory::getInstance();

namespace core::id {
    TypeID::TypeID(std::string type_name, const base::IDType &idType)
            : RVM_ID('T', idType), type_name(std::move(type_name)) {}

    TypeID::TypeID(std::string type_name, const std::shared_ptr<TypeID> &parent_type, const base::IDType &idType)
            : RVM_ID('T', idType), type_name(std::move(type_name)),
            parent_type_id(parent_type) {}

    void TypeID::printInfo() const {
        std::cout << "ID Type: Type, Sign: " << getIDSign() << ", Unique ID: " << getUID() << std::endl;
    }

    std::string TypeID::toString() const {
        return RVM_ID::toString(type_name);
    }

    bool TypeID::operator==(const base::RVM_ID &other) const {
        return equalWith(other);
    }

    std::string TypeID::toString(const std::string &detail) const {
        return "<" + type_name + ": " + detail + ">";
    }

    bool TypeID::equalWith(const base::RVM_ID &other) const {
        if (dis_id == other.dis_id) {
            return true;
        }

        if (other.sign != sign) {
            return false;
        }

        auto this_top = getTopParentTypeID();
        auto other_top = static_cast<const TypeID &>(other).getTopParentTypeID();

        return (this_top && other_top && this_top->dis_id == other_top->dis_id) ||
               (other_top && dis_id == other_top->dis_id) ||
               (this_top && other.dis_id == this_top->dis_id);
    }

    std::shared_ptr<TypeID> TypeID::getTopParentTypeID() const {
        if (parent_type_id) {
            auto cur_type = std::make_shared<TypeID>(*this);
            while (cur_type->parent_type_id) {
                cur_type = cur_type->parent_type_id;
            }
            return cur_type;
        } else {
            return nullptr;
        }
    }

    RIID::RIID(): RVM_ID('R', base::IDType::RI) {}

    void RIID::printInfo() const {
        std::cout << "ID Type: RI, Sign: " << getIDSign() << ", Unique ID: " << getUID() << std::endl;
    }

    std::string RIID::toString() const {
        return RVM_ID::toString("RI");
    }

    InsID::InsID() : RVM_ID('S', base::IDType::Ins) {}

    void InsID::printInfo() const {
        std::cout << "ID Type: Ins, Sign: " << getIDSign() << ", Unique ID: " << getUID() << std::endl;
    }

    std::string InsID::toString() const {
        return RVM_ID::toString("Ins");
    }

    DataID::DataID() : RVM_ID('D', base::IDType::Data) {}

    DataID::DataID(std::string name, std::string spaceName) : RVM_ID('D', base::IDType::Data),
                                                              name_(std::move(name)),
                                                              scopeName_(std::move(spaceName)),
                                                              index_(generateIndex()){
        size_t reserveSize = name_.size() + 1 +
                             (scopeName_.empty() ? 0 : scopeName_.size() + 1) +
                             std::to_string(index_).size();
        idstring.reserve(reserveSize);
        idstring += name_;
        if (scopeName_.empty()) {
            idstring += '#';
            idstring += std::to_string(index_);
        } else {
            idstring += '@';
            idstring += scopeName_;
            idstring += '#';
            idstring += std::to_string(index_);
        }
    }

    std::string DataID::toFullString() const {
        return RVM_ID::toString("Data(" + idstring + ")");
    }

    const std::string &DataID::getName() const { return name_; }

    const std::string &DataID::getScopeName() const { return scopeName_; }

    size_t DataID::getIndex() const { return index_; }

    void DataID::printInfo() const {
        std::cout << "DataID: " << idstring << std::endl;
    }

    bool DataID::operator==(const DataID &other) const {
        return index_ == other.index_;
    }

    std::string DataID::toString() const {
        return RVM_ID::toString("Data");
    }

    size_t DataID::generateIndex() {
        static std::atomic<size_t> counter{0};
        return counter++;
    }
}

namespace core::data {

    id::TypeID Null::typeId{"Null", base::IDType::Null};

    Null::Null() : base::RVM_Data() {}

    base::RVM_ID &Null::getTypeID() const {
        return Null::typeId;
    }

    std::string Null::getValStr() const {
        return "null";
    }

    std::string Null::getTypeName() const {
        return "Null";
    }

    bool Null::updateData(const std::shared_ptr<RVM_Data> &newData) {
        return false;
    }

    std::shared_ptr<base::RVM_Data> Null::copy_ptr() const {
        return std::make_shared<Null>(Null());
    }

    bool Null::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        switch (relational) {
            case base::Relational::RE: {
                return other->getTypeID() == Null::typeId;
            }
            case base::Relational::RNE: {
                return other->getTypeID() != Null::typeId;
            }
            case base::Relational::AND: {
                return false;
            }
            case base::Relational::OR: {
                return other->convertToBool();
            }
            default:{
                throw base::errors::TypeMismatchError(unknown_, unknown_,
                                                      {"A customType mismatch error occurred during data "
                                                       "comparison: a Null value could not be compared for an '"
                                                       + base::relationalToString(relational) + "' relationship."},
                                                      {"Check whether the data is Null during data comparison."});
            }
        }
        return false;
    }

    std::string Null::toEscapedString() const {
        return "null";
    }

    bool Null::convertToBool() const {
        return false;
    }

    id::TypeID Numeric::typeId{"Numeric", base::IDType::Numeric};

    Numeric::Numeric() : base::RVM_Data() {}

    base::RVM_ID &Numeric::getTypeID() const {
        return typeId;
    }

    bool Numeric::isNumeric(const std::shared_ptr<RVM_Data> &data) {
        return false;
    }

    std::string Numeric::toEscapedString() const {
        return this->getValStr();
    }

    id::TypeID Int::typeId{"Int", std::make_shared<id::TypeID>(Numeric::typeId), base::IDType::Int};

    Int::Int(int value) : Numeric(), value(value) {}

    std::string Int::getValStr() const {
        return std::to_string(value);
    }

    std::string Int::getTypeName() const {
        return "Int";
    }

    template<typename Op>
    std::shared_ptr<Numeric> Int::executeOperator(const std::shared_ptr<Numeric> &other, Op op, const std::string &opstr) const {
        if (!other) {
            throw base::RVM_Error(base::ErrorType::ValueError, unknown_, unknown_,
                                  {"This error is caused by manipulating a null value.",
                                   "Error expression: " + this->getValStr() + " " + opstr + " " + other->getValStr()},
                                  {"Check whether the operation value is Null during the operation."});
        }
        const auto other_id_type = other->getTypeID().idType;
        switch (other_id_type) {
            case base::IDType::Int: {
                if (auto otherInt = static_pointer_cast<const Int>(other)) {
                    return std::make_shared<Int>(op(value, otherInt->value));
                }
                break;
            }
            case base::IDType::Float: {
                if (auto otherFloat = static_pointer_cast<const Float>(other)) {
                    return std::make_shared<Float>(op(static_cast<double>(value), otherFloat->getValue()));
                }
                break;
            }
            case base::IDType::Char: {
                if (auto otherChar = static_pointer_cast<const Char>(other)) {
                    return std::make_shared<Int>(op(value, otherChar->getValue()));
                }
                break;
            }
            case base::IDType::Bool: {
                if (auto otherBool = static_pointer_cast<const Bool>(other)) {
                    return std::make_shared<Int>(op(value, otherBool->getValue()));
                }
                break;
            }
            default: {
                throw base::errors::TypeMismatchError(unknown_, unknown_,
                                                      {"A customType mismatch error occurred during data "
                                                       "operation: data of the [Int] customType can be operated only on [Numeric] data.",
                                                       "Error expression: " + this->getValStr() + " " + opstr + " " +
                                                       other->getValStr()},
                                                      {"Check whether the data customType matches that of a data operation."});
            }
        }
        return nullptr;
    }

    std::shared_ptr<Numeric> Int::add(const std::shared_ptr<Numeric> &other) const {
        return executeOperator(other, std::plus<>(), "+");
    }

    std::shared_ptr<Numeric> Int::subtract(const std::shared_ptr<Numeric> &other) const {
        return executeOperator(other, std::minus<>(), "-");
    }

    std::shared_ptr<Numeric> Int::multiply(const std::shared_ptr<Numeric> &other) const {
        return executeOperator(other, std::multiplies<>(), "*");
    }

    std::shared_ptr<Numeric> Int::divide(const std::shared_ptr<Numeric> &other) const {
        if (!other) {
            throw std::runtime_error("Null operand");
        }
        if (other->getTypeID() == Float::typeId) {
            return executeOperator(other, [](auto a, auto b) {
                if (b == 0) {
                    throw std::runtime_error("Division by zero");
                }
                return a / b;
            }, "/");
        } else if (other->getTypeID() == Int::typeId || other->getTypeID() == Char::typeId ||
                   other->getTypeID() == Bool::typeId) {
            auto otherInt = dynamic_pointer_cast<const Int>(other);
            if (otherInt->value == 0) {
                throw std::runtime_error("Division by zero");
            }
            if (value % otherInt->value == 0) {
                return std::make_shared<Int>(Int(value / otherInt->value));
            } else {
                return std::make_shared<Float>(Float(static_cast<double>(value) / otherInt->value));
            }
        }
        return nullptr;
    }

    std::shared_ptr<Numeric> Int::opp() const {
        return std::make_shared<Int>(Int(0 - this->value));
    }

    int Int::getValue() const {
        return value;
    }

    id::TypeID &Int::getTypeID() const {
        return Int::typeId;
    }

    bool Int::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (typeId == newData->getTypeID()) {
            if (auto newInt = dynamic_pointer_cast<const Int>(newData)) {
                this->value = newInt->getValue();
                return true;
            } else {
                throw std::runtime_error("Type mismatch");
            }
        } else {
            return false;
        }
    }

    std::shared_ptr<base::RVM_Data> Int::copy_ptr() const {
        return std::make_shared<Int>(value);
    }

    bool Int::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        double otherValue;
        bool isNumeric = false;
        if (other->getTypeID() == Int::typeId || other->getTypeID() == Char::typeId ||
            other->getTypeID() == Bool::typeId) {
            otherValue = static_pointer_cast<const Int>(other)->getValue();
            isNumeric = true;
        } else if (other->getTypeID() == Float::typeId) {
            otherValue = static_pointer_cast<const Float>(other)->getValue();
            isNumeric = true;
        }
        if (!isNumeric) {
            bool thisBool = convertToBool();
            bool otherBool = other->convertToBool();
            switch (relational) {
                case base::Relational::RE: return false;
                case base::Relational::RNE: return true;
                case base::Relational::AND: return thisBool && otherBool;
                case base::Relational::OR: return thisBool || otherBool;
                default: throw std::runtime_error("Invalid relational operator");
            }
        }
        switch (relational) {
            case base::Relational::RL: return value > otherValue;
            case base::Relational::RLE: return value >= otherValue;
            case base::Relational::RNE: return value != otherValue;
            case base::Relational::RE: return value == otherValue;
            case base::Relational::RS: return value < otherValue;
            case base::Relational::RSE: return value <= otherValue;
            case base::Relational::AND: return value && otherValue;
            case base::Relational::OR: return value || otherValue;
            default: throw std::runtime_error("Invalid relational operator");
        }
    }

    std::shared_ptr<Numeric> Int::pow(const std::shared_ptr<Numeric> &other) const {
        if (other->getTypeID() == Int::typeId || other->getTypeID() == Char::typeId || other->getTypeID() == Bool::typeId){
            return std::make_shared<Int>(std::pow(value, static_pointer_cast<const Int>(other)->getValue()));
        } else if (other->getTypeID() == Float::typeId){
            return std::make_shared<Float>(std::pow(value, static_pointer_cast<const Float>(other)->getValue()));
        } else {
            throw std::runtime_error("Type mismatch");
        }
        return nullptr;
    }

    std::shared_ptr<Numeric> Int::root(const std::shared_ptr<Numeric> &other) const {
        if (other->getTypeID() == Int::typeId || other->getTypeID() == Char::typeId || other->getTypeID() == Bool::typeId){
            return std::make_shared<Int>(std::pow(value, 1.0 / static_pointer_cast<const Int>(other)->getValue()));
        } else if (other->getTypeID() == Float::typeId){
            return std::make_shared<Float>(std::pow(value, 1.0 /static_pointer_cast<const Float>(other)->getValue()));
        } else {
            throw std::runtime_error("Type mismatch");
        }
        return nullptr;
    }

    bool Int::convertToBool() const {
        return value != 0;
    }

    id::TypeID Float::typeId{"Float", std::make_shared<id::TypeID>(Numeric::typeId), base::IDType::Float};

    Float::Float(double value)
            : Numeric(), value(value) {}

    std::string Float::getValStr() const {
        return std::to_string(value);
    }

    std::string Float::getTypeName() const {
        return "Float";
    }

    template<typename Op>
    std::shared_ptr<Numeric> Float::executeOperator(const std::shared_ptr<Numeric> &other, Op op) const {
        if (!other) {
            throw std::runtime_error("Null operand");
        }
        if (other->getTypeID() == Int::typeId) {
            if (auto otherInt = dynamic_pointer_cast<const Int>(other)) {
                return std::make_shared<Float>(Float(op(value, otherInt->getValue())));
            }
        } else if (other->getTypeID() == Float::typeId) {
            if (auto otherFloat = dynamic_pointer_cast<const Float>(other)) {
                return std::make_shared<Float>(Float(op(value, otherFloat->getValue())));
            }
        } else if (other->getTypeID() == Char::typeId) {
            if (auto otherChar = dynamic_pointer_cast<const Char>(other)) {
                return std::make_shared<Float>(Float(op(value, otherChar->getValue())));
            }
        } else if (other->getTypeID() == Bool::typeId) {
            if (auto otherBool = dynamic_pointer_cast<const Bool>(other)) {
                return std::make_shared<Float>(Float(op(value, otherBool->getValue())));
            }
        }
        return nullptr;
    }

    std::shared_ptr<Numeric> Float::add(const std::shared_ptr<Numeric> &other) const {
        return executeOperator(other, std::plus<>());
    }

    std::shared_ptr<Numeric> Float::subtract(const std::shared_ptr<Numeric> &other) const {
        return executeOperator(other, std::minus<>());
    }

    std::shared_ptr<Numeric> Float::multiply(const std::shared_ptr<Numeric> &other) const {
        return executeOperator(other, std::multiplies<>());
    }

    std::shared_ptr<Numeric> Float::divide(const std::shared_ptr<Numeric> &other) const {
        if (!other) {
            throw std::runtime_error("Null operand");
        }
        const auto &otherType = other->getTypeID();
        if (otherType == Int::typeId || otherType == Char::typeId || otherType == Bool::typeId) {
            auto otherInt = dynamic_pointer_cast<const Int>(other);
            if (otherInt) {
                if (otherInt->getValue() == 0) {
                    throw std::runtime_error("Division to zero.");
                }
                return std::make_shared<Float>(Float(value / otherInt->getValue()));
            }
        } else if (otherType == Float::typeId) {
            auto otherFloat = dynamic_pointer_cast<const Float>(other);
            if (otherFloat) {
                if (otherFloat->getValue() == 0.0) {
                    throw std::runtime_error("Division to zero.");
                }
                double result = static_cast<double>(value) / otherFloat->getValue();
                if (result == std::floor(result)) {
                    return std::make_shared<Int>(Int(int(result)));
                } else {
                    return std::make_shared<Float>(Float(result));
                }
            }
        }
        return nullptr;
    }

    std::shared_ptr<Numeric> Float::opp() const {
        return std::make_shared<Float>(Float(0 - this->value));
    }

    double Float::getValue() const {
        return value;
    }

    id::TypeID &Float::getTypeID() const {
        return Float::typeId;
    }

    bool Float::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (typeId == newData->getTypeID()) {
            if (auto newFloat = dynamic_pointer_cast<const Float>(newData)) {
                this->value = newFloat->getValue();
                return true;
            } else {
                throw std::runtime_error("Type mismatch");
            }
        } else {
            return false;
        }
    }

    std::shared_ptr<base::RVM_Data> Float::copy_ptr() const {
        return std::make_shared<Float>(Float(value));
    }

    bool Float::compare(const std::shared_ptr<base::RVM_Data> &other, const base::Relational &relational) const {
        double otherValue;
        bool isNumeric = false;
        if (other->getTypeID() == Int::typeId) {
            otherValue = static_pointer_cast<const Int>(other)->getValue();
            isNumeric = true;
        } else if (other->getTypeID() == Float::typeId) {
            otherValue = static_pointer_cast<const Float>(other)->getValue();
            isNumeric = true;
        }
        if (!isNumeric) {
            bool thisBool = convertToBool();
            bool otherBool = other->convertToBool();
            switch (relational) {
                case base::Relational::RE: return false;
                case base::Relational::RNE: return true;
                case base::Relational::AND: return thisBool && otherBool;
                case base::Relational::OR: return thisBool || otherBool;
                default: throw std::runtime_error("Invalid relational operator");
            }
        }
        switch (relational) {
            case base::Relational::RL: return value > otherValue;
            case base::Relational::RLE: return value >= otherValue;
            case base::Relational::RS: return value < otherValue;
            case base::Relational::RSE: return value <= otherValue;
            case base::Relational::RNE: return value != otherValue;
            case base::Relational::RE: return value == otherValue;
            case base::Relational::AND: return value && otherValue;
            case base::Relational::OR: return value || otherValue;
            default: throw std::runtime_error("Invalid relational operator");
        }
    }

    std::shared_ptr<Numeric> Float::pow(const std::shared_ptr<Numeric> &other) const {
        if (other->getTypeID() == Int::typeId || other->getTypeID() == Char::typeId || other->getTypeID() == Bool::typeId){
            return std::make_shared<Float>(std::pow(value, static_pointer_cast<const Int>(other)->getValue()));
        } else if (other->getTypeID() == Float::typeId){
            return std::make_shared<Float>(std::pow(value, static_pointer_cast<const Float>(other)->getValue()));
        } else {
            throw std::runtime_error("Type mismatch");
        }
        return nullptr;
    }

    std::shared_ptr<Numeric> Float::root(const std::shared_ptr<Numeric> &other) const {
        if (other->getTypeID() == Int::typeId || other->getTypeID() == Char::typeId || other->getTypeID() == Bool::typeId){
            return std::make_shared<Float>(std::pow(value, 1.0 / static_pointer_cast<const Int>(other)->getValue()));
        } else if (other->getTypeID() == Float::typeId){
            return std::make_shared<Float>(std::pow(value, 1.0 / static_pointer_cast<const Float>(other)->getValue()));
        } else {
            throw std::runtime_error("Type mismatch");
        }
        return nullptr;
    }

    bool Float::convertToBool() const {
        return (value < 0 ? 0 - value : value) < 0.000001;
    }

    id::TypeID Bool::typeId{"Bool", std::make_shared<id::TypeID>(Int::typeId), base::IDType::Bool};

    Bool::Bool(bool value) : Int(value ? 1 : 0) {}

    std::string Bool::getTypeName() const {
        return "Bool";
    }

    id::TypeID &Bool::getTypeID() const {
        return Bool::typeId;
    }

    std::string Bool::getValStr() const {
        return value ? "true" : "false";
    }

    std::shared_ptr<base::RVM_Data> Bool::copy_ptr() const {
        return std::make_shared<Bool>(value);
    }

    std::shared_ptr<Numeric> Bool::opp() const {
        return std::make_shared<Bool>(!this->convertToBool());
    }

    id::TypeID Char::typeId{"Char", std::make_shared<id::TypeID>(Int::typeId), base::IDType::Char};

    Char::Char(char value) : Int(value) {}

    std::string Char::getTypeName() const {
        return "Char";
    }

    id::TypeID &Char::getTypeID() const {
        return Char::typeId;
    }

    std::string Char::getValStr() const {
        return std::string (1, static_cast<char>(this->getValue()));
    }

    std::string Char::toEscapedString() const {
        return "'" + utils::StringManager::escape(getValStr()) + "'";
    }

    std::shared_ptr<base::RVM_Data> Char::copy_ptr() const {
        return std::make_shared<Char>(value);
    }

    std::string Char::toString() const {
        return "[Data(Char): '" + getValStr() + "']";
    }

    id::TypeID Iterable::typeId{"Iterable", base::IDType::Iterable};

    Iterable::Iterable() :
            base::RVM_Data() {}

    base::RVM_ID &Iterable::getTypeID() const {
        return Iterable::typeId;
    }

    id::TypeID String::typeId{"String", std::make_shared<id::TypeID>(Iterable::typeId), base::IDType::String};

    String::String(std::string value) :
            Iterable(), value(std::move(value)) {}

    std::string String::getTypeName() const {
        return "String";
    }

    std::string String::getValStr() const {
        return value;
    }

    void String::begin() const {

    }

    void String::end() const {

    }

    std::shared_ptr<base::RVM_Data> String::next() const {
        return nullptr;
    }

    id::TypeID &String::getTypeID() const {
        return String::typeId;
    }

    bool String::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (typeId.fullEqualWith(newData->getTypeID())) {
            auto newString = std::static_pointer_cast<const String>(newData);
            this->value = newString->value;
            return true;
        } else {
            return false;
        }
    }

    std::shared_ptr<base::RVM_Data> String::copy_ptr() const {
        return std::make_shared<String>(String(value));
    }

    bool String::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        if (other->getTypeID() != String::typeId){
            return false;
        }
        switch (relational) {
            case base::Relational::RE:{
                return value == static_pointer_cast<const String>(other)->value;
            }
            case base::Relational::RNE:{
                return value != static_pointer_cast<const String>(other)->value;
            }
            case base::Relational::AND:{
                return convertToBool() && other->convertToBool();
            }
            case base::Relational::OR:{
                return convertToBool() || other->convertToBool();
            }
            default:{
                throw std::runtime_error("Type mismatch");
            }
        }
        return false;
    }

    void String::append(const std::shared_ptr<RVM_Data> &data) {
        value += data->getValStr();
    }

    std::shared_ptr<Iterable> String::subpart(int begin, int end) {
        if (begin < 0 || end >= value.size()){
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        return std::make_shared<String>(value.substr(begin, end - begin));
    }

    size_t String::size() const {
        return value.size();
    }

    std::shared_ptr<base::RVM_Data> String::getDataAt(int index) {
        if (index < 0){
            index += value.size();
        }
        if (index >= value.size()){
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        return std::make_shared<Char>(value[index]);
    }

    void String::setDataAt(int index, const std::shared_ptr<RVM_Data> &data) {
        if (index < 0){
            index += value.size();
        }
        if (index >= value.size()){
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        if (data->getTypeID() == Char::typeId){
            value[index] = static_pointer_cast<const Char>(data)->getValue();
        } else if (data->getTypeID() == String::typeId){
            const auto & str_data= static_pointer_cast<const String>(data);
            if (str_data->size() != 1){
                throw std::runtime_error("Type mismatch");
            }
            value[index] = str_data->value[0];
        } else {
            throw std::runtime_error("Type mismatch");
        }
    }

    void String::eraseDataAt(int index) {
        if (index < 0){
            index += value.size();
        }
        if (index >= value.size()){
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        value.erase(index);
    }

    std::string String::toEscapedString() const {
        return "\"" + utils::StringManager::escape(value) + "\"";
    }

    std::shared_ptr<List> String::trans_to_list() const {
        return std::make_shared<List>(
                [this]() {
                    std::vector<std::shared_ptr<base::RVM_Data>> dataList;
                    dataList.reserve(value.size());
                    for (const auto &ch : value) {
                        dataList.emplace_back(std::make_shared<Char>(ch));
                    }
                    return dataList;
                }()
        );
    }

    void String::insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) {
        if (index < 0){
            index += value.size();
        }
        if (index > value.size()){
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        if (data->getTypeID() == Char::typeId){
            value.insert(value.begin() + index, static_pointer_cast<const Char>(data)->getValue());
        } else {
            throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
        }
    }

    std::shared_ptr<Series> String::trans_to_series() const {
        return std::make_shared<Series>(trans_to_list());
    }

    std::string String::toString() const {
        return "[Data(String): " + toEscapedString() + "]";
    }

    bool String::convertToBool() const {
        return !value.empty();
    }

    id::TypeID List::typeId("List", std::make_shared<id::TypeID>(Iterable::typeId), base::IDType::List);

    List::List() : Iterable() {}

    void List::begin() const {

    }

    void List::end() const {

    }

    std::shared_ptr<base::RVM_Data> List::next() const {
        return std::shared_ptr<RVM_Data>();
    }

    bool List::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (typeId.fullEqualWith(newData->getTypeID())) {
            this->dataList = static_pointer_cast<const List>(newData)->dataList;
            return true;
        } else {
            return false;
        }
    }

    std::shared_ptr<base::RVM_Data> List::copy_ptr() const {
        auto newList = std::make_shared<List>();
        newList->dataList = this->dataList;
        return newList;
    }

    bool List::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        switch (relational) {
            case base::Relational::RE:{
                if (other->getTypeID() != List::typeId){
                    return false;
                }
                return dataList == static_pointer_cast<const List>(other)->dataList;
            }
            case base::Relational::RNE:{
                if (other->getTypeID() != List::typeId){
                    return true;
                }
                return dataList != static_pointer_cast<const List>(other)->dataList;
            }
            case base::Relational::AND: return convertToBool() && other->convertToBool();
            case base::Relational::OR: return convertToBool() || other->convertToBool();
            default: throw std::runtime_error("Type mismatch");
        }
    }

    size_t List::size() const {
        return dataList.size();
    }

    void List::append(const std::shared_ptr<RVM_Data> &data) {
        dataList.push_back(data);
    }

    std::shared_ptr<Iterable> List::subpart(int begin, int end) {
        auto subList = std::make_shared<List>();
        subList->dataList.assign(dataList.begin() + begin, dataList.begin() + end);
        return subList;
    }

    std::vector<int> main_container_ids{};

    bool is_getting_std_str = false;

    std::string List::getValStdStr(const base::RVM_ID &main_container_id,
                                   const std::string &prefix, const std::string &suffix) const {
        main_container_ids.push_back(main_container_id.dis_id);
        std::ostringstream oss{};
        oss << prefix;
        for (size_t i = 0; i < dataList.size(); ++i) {
            auto item = dataList[i];
            if (item->getTypeID().fullEqualWith(Quote::typeId)){
                item = static_pointer_cast<const Quote>(item)->getQuotedData();
            }
            if (std::find(main_container_ids.begin(), main_container_ids.end(),
                          item->getInstID().dis_id) != main_container_ids.end()) {
                oss << prefix + "..." + suffix;
            } else {
                oss << item->toEscapedString();
            }
            if (i < dataList.size() - 1) {
                oss << ", ";
            }
        }
        oss << suffix;
        main_container_ids.pop_back();
        return oss.str();
    }

    std::string List::getValStr() const {
        return getValStdStr(this->instID);
    }

    std::string List::getTypeName() const {
        return "List";
    }

    std::shared_ptr<base::RVM_Data> List::getDataAt(int index) {
        if (index < 0) {
            index += dataList.size();
        }
        if (index >= dataList.size()) {
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        return dataList[index];
    }

    void List::setDataAt(int index, const std::shared_ptr<RVM_Data> &data) {
        if (index < 0) {
            index += dataList.size();
        }
        if (index >= dataList.size()) {
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        dataList[index] = data;
    }

    void List::eraseDataAt(int index) {
        if (index < 0) {
            index += dataList.size();
        }
        if (index >= dataList.size()) {
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        dataList.erase(dataList.begin() + index);
    }

    std::string List::toEscapedString() const {
        return this->getValStdStr(this->getInstID());
    }

    const std::vector<std::shared_ptr<base::RVM_Data>> &List::getDataList() const {
        return dataList;
    }

    List::List(const std::vector<std::shared_ptr<RVM_Data>> &dataList): dataList(std::move(dataList)) {}

    void List::insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) {
        if (index < 0){
            index += dataList.size();
        }
        if (index > dataList.size()){
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        dataList.insert(dataList.begin() + index, data);
    }

    base::RVM_ID &List::getTypeID() const {
        return List::typeId;
    }

    bool List::convertToBool() const {
        return size() > 0;
    }

    id::TypeID Dict::typeId{"Dict", std::make_shared<id::TypeID>(Iterable::typeId), base::IDType::Dict};

    Dict::Dict() {}

    void Dict::begin() const {

    }

    void Dict::end() const {

    }

    std::shared_ptr<base::RVM_Data> Dict::next() const {
        return std::shared_ptr<RVM_Data>();
    }

    size_t Dict::size() const {
        return this->dataDict.size();
    }

    void Dict::append(const std::shared_ptr<RVM_Data> &data) {
        if (data->getTypeID() != data::KeyValuePair::typeId) {
            throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
        }
        const auto &kvPair = std::dynamic_pointer_cast<data::KeyValuePair>(data);
        const std::string &key = kvPair->key->toEscapedString();
        if (this->dataDict.find(key) == this->dataDict.end()) {
            this->dataDict[key] = kvPair->value;
            this->keyList.push_back(key);
        } else {
            throw base::errors::DuplicateKeyError(unknown_, unknown_, {}, {});
        }
    }

    std::shared_ptr<Iterable> Dict::subpart(int begin, int end) {
        if (begin < 0 || end > static_cast<int>(this->keyList.size()) || begin > end) {
            throw std::out_of_range("Invalid range: begin and end must satisfy 0 <= begin <= end <= keyList.size()");
        }
        std::unordered_map<std::string, std::shared_ptr<RVM_Data>> subDict;
        subDict.reserve(end - begin);
        for (int i = begin; i < end; ++i) {
            const std::string &key = this->keyList[i];
            subDict[key] = this->dataDict.at(key);
        }
        std::vector<std::string> subList(this->keyList.begin() + begin, this->keyList.begin() + end);
        return std::make_shared<Dict>(std::move(subDict), std::move(subList));
    }


    std::shared_ptr<base::RVM_Data> Dict::getDataAt(int index) {
        if (index < 0) {
            index += keyList.size();
        }
        if (index >= static_cast<int>(keyList.size())) {
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        return dataDict[keyList[index]];
    }

    void Dict::setDataAt(int index, const std::shared_ptr<RVM_Data> &data) {
        if (index < 0) {
            index += keyList.size();
        }
        if (index >= static_cast<int>(keyList.size())) {
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        dataDict[keyList[index]] = data;
    }

    void Dict::eraseDataAt(int index) {
        if (index < 0) {
            index += keyList.size();
        }
        if (index >= static_cast<int>(keyList.size())) {
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        dataDict.erase(keyList[index]);
        keyList.erase(keyList.begin() + index);
    }

    std::string Dict::getValStr() const {
        std::string str = "{";
        for (int i = 0; i < keyList.size(); ++i) {
            str += keyList[i] + ": " + (this->dataDict.at(keyList[i])->getInstID() != getInstID() ?
                                        this->dataDict.at(keyList[i])->toEscapedString() : "{...}");
            if (i != keyList.size() - 1) {
                str += ", ";
            }
        }
        return str + "}";
    }

    std::string Dict::getTypeName() const {
        return "Dict";
    }

    bool Dict::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (newData->getTypeID().fullEqualWith(data::Dict::typeId)) {
            const auto &otherDict = static_pointer_cast<data::Dict>(newData);
            this->keyList = otherDict->keyList;
            this->dataDict = otherDict->dataDict;
            return true;
        }
        return false;
    }

    bool Dict::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        if (other->getTypeID() != data::Dict::typeId) {
            if (relational == base::Relational::RE || relational == base::Relational::RNE){
                return relational == base::Relational::RNE;
            } else if (relational == base::Relational::AND) {
                return convertToBool() && other->convertToBool();
            } else if (relational == base::Relational::OR) {
                return convertToBool() || other->convertToBool();
            } else {
                throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
            }
        }
        const auto &otherDict = std::dynamic_pointer_cast<data::Dict>(other);
        switch (relational) {
            case base::Relational::RE:
                return this->keyList == otherDict->keyList && this->dataDict == otherDict->dataDict;
            case base::Relational::RNE:
                return this->keyList != otherDict->keyList || this->dataDict != otherDict->dataDict;
            case base::Relational::AND:
                return convertToBool() && other->convertToBool();
            case base::Relational::OR:
                return convertToBool() || other->convertToBool();
            default:
                throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
        }
    }

    std::shared_ptr<base::RVM_Data> Dict::copy_ptr() const {
        return std::make_shared<Dict>(dataDict, keyList);
    }

    Dict::Dict(std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> dataDict,
               std::vector<std::string> keyList)
            : dataDict(std::move(dataDict)), keyList(std::move(keyList)){}

    std::shared_ptr<base::RVM_Data> Dict::getDataAt(const std::string &key) {
        if (dataDict.find(key) == dataDict.end()) {
            throw base::errors::KeyNotFoundError(unknown_, unknown_, {}, {});
        }
        return dataDict[key];
    }

    void Dict::setDataAt(std::string key, const std::shared_ptr<RVM_Data> &data) {
        if (dataDict.find(key) == dataDict.end()) {
            dataDict[key] = data;
            keyList.push_back(key);
        } else {
            dataDict[key] = data;
        }
    }

    void Dict::eraseDataAt(std::string key) {
        if (dataDict.find(key) == dataDict.end()) {
            throw base::errors::KeyNotFoundError(unknown_, unknown_, {}, {});
        }
        dataDict.erase(key);
        keyList.erase(std::find(keyList.begin(), keyList.end(), key));
    }

    Dict::Dict(std::shared_ptr<List> list) {
        if (!list) {
            throw std::invalid_argument("List pointer is null");
        }
        size_t size = list->size();
        dataDict.reserve(size);
        keyList.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            const std::string key = "\"" + std::to_string(i) + "\"";
            dataDict[key] = list->getDataAt(i);
            keyList.push_back(key);
        }
    }

    std::string Dict::toEscapedString() const {
        return getValStr();
    }

    void Dict::insertDataAt(int index, const std::shared_ptr<RVM_Data> &data) {
        if (index < 0) {
            index += keyList.size();
        }
        if (index > keyList.size()) {
            throw base::errors::IndexOutOfRangeError(unknown_, unknown_, {}, {});
        }
        if (data->getTypeID().fullEqualWith(data::KeyValuePair::typeId)){
            throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
        }
        const auto &new_kvp = std::static_pointer_cast<data::KeyValuePair>(data);
        keyList.insert(keyList.begin() + index, new_kvp->key->getValStr());
        dataDict.insert(std::make_pair(new_kvp->key->getValStr(), new_kvp->value));
    }

    base::RVM_ID &Dict::getTypeID() const {
        return Dict::typeId;
    }

    bool Dict::convertToBool() const {
        return size() > 0;
    }

    id::TypeID Series::typeId{"Series", std::make_shared<id::TypeID>(data::Iterable::typeId), base::IDType::Series};

    Series::Series() {}

    std::shared_ptr<base::RVM_Data> Series::copy_ptr() const {
        const auto &newSeries = std::make_shared<Series>();
        newSeries->dataList = dataList;
        return newSeries;
    }

    base::RVM_ID &Series::getTypeID() const {
        return Series::typeId;
    }

    bool Series::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (newData->getTypeID().fullEqualWith(Series::typeId)){
            dataList = std::static_pointer_cast<Series>(newData)->dataList;
            return true;
        }
        return false;
    }

    bool Series::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        if (other->getTypeID() != Series::typeId) {
            if (relational == base::Relational::RE || relational == base::Relational::RNE) {
                return relational == base::Relational::RNE;
            } else if (relational == base::Relational::AND) {
                return convertToBool() && other->convertToBool();
            } else if (relational == base::Relational::OR) {
                return convertToBool() || other->convertToBool();
            } else {
                throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
            }
        }
        switch (relational) {
            case base::Relational::RE:
                return dataList == std::static_pointer_cast<Series>(other)->dataList;
            case base::Relational::RNE:
                return dataList != std::static_pointer_cast<Series>(other)->dataList;
            default:
                throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
        }
    }

    std::shared_ptr<Iterable> Series::subpart(int begin, int end) {
        return std::make_shared<Series>(List::subpart(begin, end));
    }

    std::string Series::getValStr() const {
        return List::getValStdStr(this->instID, "(", ")");
    }

    std::string Series::getTypeName() const {
        return "Series";
    }

    std::string Series::toEscapedString() const {
        return getValStdStr(this->instID, "(", ")");
    }

    Series::Series(std::shared_ptr<base::RVM_Data> list) {
        if (list->getTypeID().fullEqualWith(List::typeId)){
            dataList = std::move(static_pointer_cast<List>(list)->getDataList());
        } else {
            throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
        }
    }


    id::TypeID Structure::typeId{"Structure", base::IDType::Structure};

    Structure::Structure() : RVM_Data() {}

    base::RVM_ID &Structure::getTypeID() const {
        return typeId;
    }

    std::string Structure::getValStr() const {
        return "[Structure: " + getInstID().toString() + "]";
    }

    std::string Structure::getTypeName() const {
        return "Structure";
    }

    id::TypeID KeyValuePair::typeId{"KeyValuePair", base::IDType::KeyValuePair};

    KeyValuePair::KeyValuePair() {
        key = std::make_shared<Null>();
        value = std::make_shared<Null>();
    }

    KeyValuePair::KeyValuePair(std::shared_ptr<base::RVM_Data> key, std::shared_ptr<base::RVM_Data> value)
            : key(std::move(key)), value(std::move(value)) {}

    bool KeyValuePair::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (typeId == newData->getTypeID()){
            auto newKVP = static_pointer_cast<const KeyValuePair>(newData);
            this->key = newKVP->key;
            this->value = newKVP->value;
            return true;
        }
        return false;
    }

    bool KeyValuePair::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        if (typeId != other->getTypeID()){
            if (relational == base::Relational::RE || relational == base::Relational::RNE) {
                return relational == base::Relational::RNE;
            } else if (relational == base::Relational::AND) {
                return convertToBool() && other->convertToBool();
            } else if (relational == base::Relational::OR) {
                return convertToBool() || other->convertToBool();
            } else {
                throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
            }
        }
        const auto &newKVP = static_pointer_cast<const KeyValuePair>(other);
        switch (relational){
            case base::Relational::RE:{
                return key->compare(newKVP->key, base::Relational::RE) &&
                       value->compare(newKVP->value, base::Relational::RE);
            }
            case base::Relational::RNE:{
                return key->compare(newKVP->key, base::Relational::RNE) ||
                       value->compare(newKVP->value, base::Relational::RNE);
            }
            default: throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
        }
    }

    std::shared_ptr<base::RVM_Data> KeyValuePair::copy_ptr() const {
        return std::make_shared<KeyValuePair>(this->key, this->value);
    }

    base::RVM_ID &KeyValuePair::getTypeID() const {
        return typeId;
    }

    std::string KeyValuePair::getValStr() const {
        return "<" + key->getValStr() + ": " + value->getValStr() + ">";
    }

    std::string KeyValuePair::getTypeName() const {
        return "KeyValuePair";
    }

    bool KeyValuePair::convertToBool() const {
        return this->key->convertToBool() && this->value->convertToBool();
    }

    id::TypeID CompareGroup::typeId{"CompareGroup", base::IDType::CompareGroup};

    CompareGroup::CompareGroup(std::shared_ptr<id::DataID> dataLeft, std::shared_ptr<id::DataID> dataRight)
            : compLeft(std::move(dataLeft)), compRight(std::move(dataRight)){}

    bool CompareGroup::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (typeId == newData->getTypeID()){
            if (auto newComp = static_pointer_cast<const CompareGroup>(newData)){
                this->compLeft = newComp->compLeft;
                this->compRight = newComp->compRight;
                return true;
            } else {
                throw std::runtime_error("Type mismatch");
            }
        } else {
            return false;
        }
    }

    std::shared_ptr<base::RVM_Data> CompareGroup::copy_ptr() const {
        return std::make_shared<CompareGroup>(compLeft, compRight);
    }

    base::RVM_ID &CompareGroup::getTypeID() const {
        return typeId;
    }

    std::string CompareGroup::getValStr() const {
        return "[CompareGroup: (" + compLeft->toString() + " & " + compRight->toString() + ")]";
    }

    std::string CompareGroup::getTypeName() const {
        return "CompareGroup";
    }

    bool CompareGroup::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        if (other->getTypeID() != typeId){
            if (relational == base::Relational::RE || relational == base::Relational::RNE){
                return relational != base::Relational::RE;
            } else if (relational == base::Relational::AND) {
                return convertToBool() && other->convertToBool();
            } else if (relational == base::Relational::OR) {
                return convertToBool() || other->convertToBool();
            }
            throw std::runtime_error("Type mismatch");
        }
        auto otherComp = static_pointer_cast<const CompareGroup>(other);
        switch (relational) {
            case base::Relational::RE: {
                return compLeft.get() == otherComp.get()->compLeft.get() &&
                       compRight.get() == otherComp.get()->compRight.get();
            }
            case base::Relational::RNE: {
                return compLeft.get() != otherComp.get()->compLeft.get() ||
                       compRight.get() != otherComp.get()->compRight.get();
            }
            case base::Relational::AND: {
                return convertToBool() && other->convertToBool();
            }
            case base::Relational::OR: {
                return convertToBool() || other->convertToBool();
            }
            default: {
                throw std::runtime_error("Type mismatch");
            }
        }
        return false;
    }

    bool CompareGroup::compare(const base::Relational &relational) {
        auto leftData = memory::data_space_pool.findDataByIDNoLock(*compLeft);
        auto rightData = memory::data_space_pool.findDataByIDNoLock(*compRight);
        return leftData->compare(rightData, relational);
    }

    bool CompareGroup::convertToBool() const {
        auto leftData = memory::data_space_pool.findDataByIDNoLock(*compLeft);
        auto rightData = memory::data_space_pool.findDataByIDNoLock(*compRight);
        return leftData->convertToBool() && rightData->convertToBool();
    }

    id::TypeID CustomType::typeId {"CustomType", base::IDType::CustomType};

    CustomType::CustomType(std::string className, std::shared_ptr<CustomType> parentType)
            : typeName(std::move(className)), parentType(std::move(parentType)) {}

    bool CustomType::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (typeId == newData->getTypeID()){
            if (auto newCustomStruct = static_pointer_cast<const CustomType>(newData)){
                this->typeName = newCustomStruct->typeName;
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    bool CustomType::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        if (other->getTypeID() != typeId){
            if (relational == base::Relational::RE || relational == base::Relational::RNE){
                return relational != base::Relational::RE;
            } else if (relational == base::Relational::AND) {
                return convertToBool() && other->convertToBool();
            } else if (relational == base::Relational::OR) {
                return convertToBool() || other->convertToBool();
            }
            throw std::runtime_error("Type mismatch");
        }
        auto newCustomStruct = static_pointer_cast<const CustomType>(other);
        switch (relational) {
            case base::Relational::RE: {
                return tpFields == newCustomStruct->tpFields;
            }
            case base::Relational::RNE: {
                return tpFields != newCustomStruct->tpFields;
            }
            default: {
                throw std::runtime_error("Type mismatch");
            }
        }
    }

    std::shared_ptr<base::RVM_Data> CustomType::copy_ptr() const {
        auto newCustomStruct = std::make_shared<CustomType>(typeName, parentType);
        newCustomStruct->tpFields = tpFields;
        return newCustomStruct;
    }

    std::string CustomType::getValStr() const {
        return "[CustomType: " + typeName + "]";
    }

    base::RVM_ID &CustomType::getTypeID() const {
        return CustomType::typeId;
    }

    std::string CustomType::getTypeName() const {
        return "CustomType";
    }

    bool CustomType::hasFieldItself(const std::string &fieldName) {
        return tpFields.find(fieldName) != tpFields.end() ||
        std::find(instFields.begin(), instFields.end(), fieldName) != instFields.end();
    }

    bool CustomType::hasField(const std::string &fieldName) {
        if (hasFieldItself(fieldName)) {
            return true;
        } else {
            auto pre_type = parentType;
            while (pre_type) {
                if (pre_type->hasFieldItself(fieldName)) {
                    return true;
                } else {
                    pre_type = pre_type->parentType;
                }
            }
            return false;
        }
    }

    std::shared_ptr<base::RVM_Data> CustomType::getTpField(const std::string &fieldName) {
        // CustomType 仅能获取类型字段，不可获取实例字段
        if (tpFields.find(fieldName) != tpFields.end()) {
            return tpFields[fieldName];
        } else if (parentType != nullptr){
            return parentType->getTpField(fieldName);
        } else {
            return nullptr;
        }
    }

    void CustomType::setTpField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData) {
        if (tpFields.find(fieldName) != tpFields.end()) {
            if (!tpFields[fieldName]->updateData(fieldData)){
                tpFields[fieldName] = fieldData;
            }
            bool has_this_method_field = std::find(methodFields.begin(), methodFields.end(), fieldName) != methodFields.end();
            if (fieldData->getTypeID() == Callable::typeId && has_this_method_field) {
                methodFields.insert(fieldName);
            } else if (has_this_method_field) {
                methodFields.erase(std::find(methodFields.begin(), methodFields.end(), fieldName));
            }
        } else if (parentType != nullptr){
            parentType->setTpField(fieldName, fieldData);
        } else {
            throw base::errors::MemoryError(unknown_, unknown_, {}, {});
        }
    }

    void CustomType::addTpField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData) {
        if (hasFieldItself(fieldName)){
            throw base::errors::DuplicateKeyError(unknown_, unknown_, {}, {});
        }
        tpFields[fieldName] = fieldData ? fieldData : std::make_shared<Null>();
        if (fieldData && fieldData->getTypeID() == Callable::typeId) {
            methodFields.insert(fieldName);
        }
    }

    void CustomType::addInstField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData) {
        if (hasFieldItself(fieldName)){
            throw base::errors::DuplicateKeyError(unknown_, unknown_, {}, {});
        }
        instFields.push_back(fieldName);
        if (fieldData && fieldData->getTypeID() == Callable::typeId) {
            methodFields.insert(fieldName);
        }
    }

    // 检查此类型是否属于 type，即检查此类型是否是 type 的子类或等于 type
    bool CustomType::checkBelongTo(const std::shared_ptr<CustomType> &type) const {
        if (!type) {
            return false;
        }
        const CustomType* current = this;
        while (current) {
            if (current == type.get()) {
                return true;
            }
            current = current->parentType.get();
        }
        return false;
    }

    bool CustomType::convertToBool() const {
        return true;
    }

    std::string CustomType::getTypeIDString() const {
        return typeId.toString(typeName + "@" + instID.getIDString());
    }

    bool CustomType::hasInstField(const std::string &fieldName) {
        if (std::find(instFields.begin(), instFields.end(), fieldName) != instFields.end()){
            return true;
        } else {
            for (auto &cur_type = parentType; cur_type != nullptr; cur_type = cur_type->parentType) {
                if (std::find(cur_type->instFields.begin(), cur_type->instFields.end(), fieldName) != cur_type->instFields.end()){
                    return true;
                }
            }
            return false;
        }
    }

    bool CustomType::hasTpFieldItself(const std::string &fieldName) {
        return tpFields.find(fieldName) != tpFields.end();
    }

    bool CustomType::hasInstFieldItself(const std::string &fieldName) {
        return std::find(instFields.begin(), instFields.end(), fieldName) != instFields.end();
    }

    bool CustomType::hasTpField(const std::string &fieldName) {
        if (hasTpFieldItself(fieldName)){
            return true;
        } else {
            auto pre_type = parentType;
            while (pre_type) {
                if (pre_type->hasTpFieldItself(fieldName)) {
                    return true;
                } else {
                    pre_type = pre_type->parentType;
                }
            }
            return false;
        }
    }

    void CustomType::travalTypes(std::function<bool(std::shared_ptr<CustomType>)> callback) {
        for (auto cur_type = parentType; cur_type != nullptr; cur_type = cur_type->parentType) {
            if (callback(cur_type)) return;
        }
    }

    Callable::Callable(Callable::StdArgs args) : RVM_Data(), dataId(id::DataID()), args(std::move(args)) {
        for (auto &arg : args){
            if (arg.getType() != utils::ArgType::identifier){
                throw std::runtime_error("Invalid argument customType");
            }
        }
    }

    id::TypeID CustomInst::typeId{"CustomInst", base::IDType::CustomTypeInst};

    static const auto &null_inst = std::make_shared<Null>();

    CustomInst::CustomInst(std::shared_ptr<CustomType> instType)
            : customType(std::move(instType)){
        for (auto cur_type = customType; cur_type != nullptr; cur_type = cur_type->parentType){
            auto& type_fields = instFields[cur_type->typeName]; // 一次map访问
            type_fields.reserve(cur_type->instFields.size());
            for (const auto& field_name : cur_type->instFields) {
                type_fields.emplace(field_name, null_inst);
            }
        }
    }

    bool CustomInst::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (typeId == newData->getTypeID()){
            if (auto newCustomStruct = static_pointer_cast<const CustomInst>(newData)){
                customType = newCustomStruct->customType;
                instFields = newCustomStruct->instFields;
                return true;
            } else {
                return false;
            }
        }
        return false;
    }

    bool CustomInst::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        if (other->getTypeID() != typeId){
            if (relational == base::Relational::RE || relational == base::Relational::RNE){
                return relational != base::Relational::RE;
            } else if (relational == base::Relational::AND) {
                return convertToBool() && other->convertToBool();
            } else if (relational == base::Relational::OR) {
                return convertToBool() || other->convertToBool();
            }
            throw std::runtime_error("Type mismatch");
        }
        auto newCustomStruct = static_pointer_cast<const CustomInst>(other);
        switch (relational) {
            case base::Relational::RE: return customType == newCustomStruct->customType && instFields == newCustomStruct->instFields;
            case base::Relational::RNE: return customType != newCustomStruct->customType || instFields != newCustomStruct->instFields;
            case base::Relational::AND: return convertToBool() && other->convertToBool();
            case base::Relational::OR: return convertToBool() || other->convertToBool();
            default: throw std::runtime_error("Type mismatch");
        }
    }

    std::shared_ptr<base::RVM_Data> CustomInst::copy_ptr() const {
        auto new_inst = std::make_shared<CustomInst>(this->customType);
        new_inst->instFields = instFields;
        return new_inst;
    }

    base::RVM_ID &CustomInst::getTypeID() const {
        return CustomInst::typeId;
    }

    std::string CustomInst::getValStr() const {
        return "[" + customType->typeName + ": inst]";
    }

    std::string CustomInst::getTypeName() const {
        return "CustomInst";
    }

    std::shared_ptr<base::RVM_Data>
    CustomInst::getField(const std::string &fieldName, const std::shared_ptr<CustomType> &specCustomType) {
        // CustomInst 可以获取任意存在的字段，且可以指定查询的 CustomType
        if (specCustomType) {
            // 如果指定了 CustomType，则只查询该 CustomType 的字段
            if (instFields.find(specCustomType->typeName) != instFields.end()) {
                const auto &fieldData = specCustomType->getTpField(fieldName); // 查询该 CustomType 的类型字段
                if (fieldData) {
                    return fieldData;
                }
                if (instFields[specCustomType->typeName].find(fieldName) != instFields[specCustomType->typeName].end()) {
                    return instFields[specCustomType->typeName][fieldName];
                }
            }
        } else {
            // 如果没有指定 CustomType，则查询所有 CustomType 的字段
            auto cur_type = customType;
            while (cur_type) {
                if (cur_type->hasTpFieldItself(fieldName)) {
                    return cur_type->getTpField(fieldName);
                }
                if (instFields[cur_type->typeName].find(fieldName) != instFields[cur_type->typeName].end()) {
                    return instFields[cur_type->typeName][fieldName];
                }
                cur_type = cur_type->parentType;
            }
        }
        throw base::errors::MemoryError(unknown_, unknown_, {}, {});
    }

    void CustomInst::setField(const std::string &fieldName, const std::shared_ptr<base::RVM_Data> &fieldData,
                              const std::shared_ptr<CustomType> &specCustomType) {
        if (specCustomType) {
            if (specCustomType->hasTpFieldItself(fieldName)) {
                specCustomType->setTpField(fieldName, fieldData);
            } else if (instFields.find(specCustomType->typeName) != instFields.end()) {
                instFields[specCustomType->typeName][fieldName] = fieldData;
            } else {
                throw base::errors::TypeMismatchError(unknown_, unknown_, {}, {});
            }
        } else {
            auto &target_fields = instFields[customType->typeName];
            if (target_fields.find(fieldName) != target_fields.end()) {
                target_fields[fieldName] = fieldData;
            } else {
                const auto &callback =
                        [fieldName, &fieldData, this](const std::shared_ptr<CustomType> &curType) -> bool {
                    if (curType->hasInstFieldItself(fieldName)) {
                        instFields[customType->typeName].emplace(fieldName, fieldData);
                        return true;
                    }
                    if (curType->hasTpFieldItself(fieldName)) {
                        curType->setTpField(fieldName, fieldData);
                        return true;
                    }
                    return false;
                };

                // 先检查当前类型
                if (callback(customType)) {
                    return;
                }
                // 再遍历父类型
                customType->travalTypes(callback);
            }
        }
    }

    bool CustomInst::hasField(const std::string &fieldName) {
        // 直接通过 CustomType 进行检查
        return customType->hasFieldItself(fieldName);
    }

    bool CustomInst::convertToBool() const {
        return true;
    }

    std::string CustomInst::getTypeIDString() const {
        return customType->getTypeIDString();
    }

    void CustomInst::derivedToChildType(const std::shared_ptr<CustomType> &childType) {
        if (childType->checkBelongTo(customType)){
            auto cur_type = childType;
            while (cur_type && cur_type != customType) {  // 添加nullptr检查
                auto& type_fields = instFields[cur_type->typeName];  // 一次map访问
                type_fields.reserve(cur_type->instFields.size());
                for (const auto& field_name : cur_type->instFields) {
                    type_fields.emplace(field_name, null_inst);  // 共享Null实例
                }
                cur_type = cur_type->parentType;
            }
            customType = childType;  // 最后更新customType
        } else {
            throw base::errors::TypeMismatchError(unknown_, unknown_,
                                                  {}, {});
        }
    }

    id::TypeID Callable::typeId{"Callable", base::IDType::Callable};

    std::string Callable::getValStr() const {
        return "[Callable Object]";
    }

    std::string Callable::getTypeName() const {
        return "Callable";
    }

    base::RVM_ID & Callable::getTypeID() const {
        return typeId;
    }

    bool Callable::convertToBool() const {
        return true;
    }

    id::TypeID Function::typeId{"Function", std::make_shared<id::TypeID>(data::Callable::typeId), base::IDType::Function};

    id::TypeID RetFunction::typeId{"RetFunction", std::make_shared<id::TypeID>(Function::typeId), base::IDType::RetFunction};

    id::TypeID Quote::typeId ("Quote", base::IDType::Quote);

    Quote::Quote(const id::DataID &quoteDataID)
            : base::RVM_Data(), dataId(id::DataID()), quoteDataID(quoteDataID),
              quotedDataIDString(quoteDataID.toString()){}

    std::string Quote::getValStr() const {
        const auto &value_data = memory::data_space_pool.findDataByIDNoLock(quoteDataID);
        if (value_data == nullptr){
            return this->quotedDataIDString;
        }
        return value_data->getValStr();
    }

    std::string Quote::getTypeName() const {
        return "Quote";
    }

    base::RVM_ID &Quote::getTypeID() const {
        return typeId;
    }

    bool Quote::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (typeId == newData->getTypeID()){
            if (auto newQuote = static_pointer_cast<const Quote>(newData)){
                this->quoteDataID = newQuote->quoteDataID;
                this->quotedDataIDString = newQuote->quotedDataIDString;
                return true;
            } else {
                throw std::runtime_error("Type mismatch");
            }
        }
        return false;
    }

    bool Quote::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        auto quotedData = memory::data_space_pool.findDataByIDNoLock(quoteDataID);
        return quotedData->compare(other, relational);
    }

    std::shared_ptr<base::RVM_Data> Quote::copy_ptr() const {
        return std::make_shared<Quote>(this->quoteDataID);
    }

    void Quote::updateQuoteData(const std::shared_ptr<RVM_Data> &newData) {
        memory::data_space_pool.updateDataNoLock(quoteDataID, newData);
    }

    std::shared_ptr<base::RVM_Data> Quote::getQuotedData() const {
        return core::memory::RVM_Memory::getInstance().findDataByIDNoLock(quoteDataID);
    }

    bool Quote::convertToBool() const {
        return this->getQuotedData()->convertToBool();
    }

    std::string File::fileModeToString(const FileMode &fileMode){
        switch (fileMode) {
            case FileMode::Append: return "fl-a";
            case FileMode::Read: return "fl-r";
            case FileMode::Write: return "fl-w";
            case FileMode::ReadWrite: return "fl-rw";
            case FileMode::ReadAppend: return "fl-ra";
            case FileMode::WriteAppend: return "fl-wa";
            default: return unknown_;
        }
    }

    std::string File::fileModeToFormatString(const FileMode &fileMode){
        switch (fileMode) {
            case FileMode::Append: return "[FileMode: Append]";
            case FileMode::Read: return "[FileMode: Read]";
            case FileMode::Write: return "[FileMode: Write]";
            case FileMode::ReadWrite: return "[FileMode: ReadWrite]";
            case FileMode::ReadAppend: return "[FileMode: ReadAppend]";
            case FileMode::WriteAppend: return "[FileMode: WriteAppend]";
            default: return "[FileMode: Unknown]";
        }
    }

    static const std::unordered_map<std::string, FileMode> fileModeMap = {
            {"fl-a", FileMode::Append},
            {"fl-r", FileMode::Read},
            {"fl-w", FileMode::Write},
            {"fl-rw", FileMode::ReadWrite},
            {"fl-ra", FileMode::ReadAppend},
            {"fl-wa", FileMode::WriteAppend}
    };

    FileMode File::stringToFileMode(const std::string &fileModeString) {
        auto it = fileModeMap.find(fileModeString);
        if (it != fileModeMap.end()) {
            return it->second;
        } else {
            throw std::runtime_error("Invalid file mode: " + fileModeString);
        }
    }

    id::TypeID File::typeId {"File", base::IDType::File};

    File::File(const FileMode &mode, const std::string &filepath) : base::RVM_Data(), dataId(id::DataID()),
                                                                    fileMode(mode), filepath(filepath){}

    std::string File::getValStr() const {
        return "[File(" + fileModeToString(fileMode) + "): '" + utils::getFileFromPath(filepath) + "']";
    }

    std::string File::getTypeName() const {
        return "File";
    }

    base::RVM_ID &File::getTypeID() const {
        return File::typeId;
    }

    bool File::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (File::typeId == newData->getTypeID()){
            if (auto newFile = static_pointer_cast<const File>(newData)){
                this->fileMode = newFile->fileMode;
                this->filepath = newFile->filepath;
                return true;
            }
        }
        return false;
    }

    bool File::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        if (other->getTypeID() != File::typeId){
            if (relational == base::Relational::RE || relational == base::Relational::RNE){
                return relational == base::Relational::RNE;
            } else if (relational == base::Relational::AND) {
                return convertToBool() && other->convertToBool();
            } else if (relational == base::Relational::OR) {
                return convertToBool() || other->convertToBool();
            }
            throw std::runtime_error("Type mismatch");
        }
        auto otherFile = static_pointer_cast<const File>(other);
        switch (relational) {
            case base::Relational::RE: return fileMode == otherFile->fileMode && filepath == otherFile->filepath;
            case base::Relational::RNE: return fileMode != otherFile->fileMode || filepath != otherFile->filepath;
            case base::Relational::AND: return convertToBool() && other->convertToBool();
            case base::Relational::OR: return convertToBool() || other->convertToBool();
            default: throw std::runtime_error("Type mismatch");
        }
    }

    std::shared_ptr<base::RVM_Data> File::copy_ptr() const {
        return std::make_shared<File>(this->fileMode, this->filepath);
    }

    std::string File::readFile() const {
        if (fileMode == FileMode::Read || fileMode == FileMode::ReadAppend || fileMode == FileMode::ReadWrite){
            return utils::readFile(filepath);
        }
        throw base::errors::FileReadError(unknown_, unknown_, {}, {});
    }

    std::vector<std::string> File::readFileToLines() const {
        if (fileMode == FileMode::Read || fileMode == FileMode::ReadAppend || fileMode == FileMode::ReadWrite){
            return utils::readFileToLines(filepath);
        }
        throw std::runtime_error("File mode not support read.");
    }

    bool File::writeFile(const std::string &content) const {
        if (fileMode == FileMode::Write){
            return utils::writeFile(filepath, content);
        } else if (fileMode == FileMode::WriteAppend || fileMode == FileMode::ReadWrite) {
            return utils::appendFile(filepath, content);
        }
        return false;
    }

    void File::setModeByString(const std::string &modeStr) {
        if (fileModeMap.find(modeStr) == fileModeMap.end()){
            throw std::runtime_error("Invalid file mode");
        }
        fileMode = fileModeMap.at(modeStr);
    }

    bool File::convertToBool() const {
        return true;
    }

    id::TypeID Time::typeId{"Time", base::IDType::Time};

    Time::Time(int year, int month, int day, int hour, int minute, int second, const utils::TimeFormat &format)
    : base::RVM_Data(), dataId(id::DataID()), year(year), month(month), day(day),
    hour(hour), minute(minute), second(second), format(format){}

    Time Time::now() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_time = *std::localtime(&now_time);
        return Time(local_time.tm_year + 1900, local_time.tm_mon + 1,
                    local_time.tm_mday, local_time.tm_hour,
                    local_time.tm_min, local_time.tm_sec);
    }

    std::string Time::getValStr() const {
        return getTimeString();
    }

    std::string Time::toString() const {
        return "[Time: '" + getTimeString() + "']";
    }

    Time Time::fromString(const std::string &dateString, const utils::TimeFormat &format) {
        int year = 0, month = 0, day = 0;
        int hour = 0, minute = 0, second = 0;

        switch (format) {
            case utils::TimeFormat::ISO:
            case utils::TimeFormat::US:
            case utils::TimeFormat::European:
                if (!parseDateFromString(dateString, format, year, month, day, hour, minute, second)) {
                    throw std::runtime_error("Invalid date format: " + dateString);
                }
                break;

            case utils::TimeFormat::Timestamp:
                try {
                    std::time_t timestamp = std::stol(dateString);
                    std::tm local_time = *std::localtime(&timestamp);
                    year = local_time.tm_year + 1900;
                    month = local_time.tm_mon + 1;
                    day = local_time.tm_mday;
                    hour = local_time.tm_hour;
                    minute = local_time.tm_min;
                    second = local_time.tm_sec;
                } catch (const std::exception &e) {
                    throw std::runtime_error("Invalid timestamp format: " + dateString);
                }
                break;

            default:
                throw std::runtime_error("Unsupported date format");
        }

        return Time(year, month, day, hour, minute, second);
    }

    std::string Time::getTypeName() const {
        return "Time";
    }

    base::RVM_ID &Time::getTypeID() const {
        return typeId;
    }

    bool Time::updateData(const std::shared_ptr<RVM_Data> &newData) {
        if (newData->getTypeID().fullEqualWith(typeId)){
            const auto & newTime = std::static_pointer_cast<Time>(newData);
            year = newTime->year;
            month = newTime->month;
            day = newTime->day;
            hour = newTime->hour;
            minute = newTime->minute;
            second = newTime->second;
            return true;
        }
        return false;
    }

    std::shared_ptr<base::RVM_Data> Time::copy_ptr() const {
        return std::make_shared<Time>(year, month, day, hour, minute, second);
    }

    bool Time::compare(const std::shared_ptr<RVM_Data> &other, const base::Relational &relational) const {
        bool thisBool = convertToBool();
        bool otherBool = other->convertToBool();
        if (other->getTypeID().fullEqualWith(typeId)){
            const auto & otherTime = std::static_pointer_cast<Time>(other);
            auto thisTimeTuple = std::tie(year, month, day, hour, minute, second);
            auto otherTimeTuple = std::tie(otherTime->year, otherTime->month, otherTime->day, otherTime->hour, otherTime->minute, otherTime->second);
            switch (relational) {
                case base::Relational::RE:
                    return thisTimeTuple == otherTimeTuple;
                case base::Relational::RNE:
                    return thisTimeTuple != otherTimeTuple;
                case base::Relational::RS:
                    return thisTimeTuple < otherTimeTuple;
                case base::Relational::RL:
                    return thisTimeTuple > otherTimeTuple;
                case base::Relational::RSE:
                    return thisTimeTuple <= otherTimeTuple;
                case base::Relational::RLE:
                    return thisTimeTuple >= otherTimeTuple;
                case base::Relational::AND:
                    return thisBool && otherBool;
                case base::Relational::OR:
                    return thisBool || otherBool;
                default:
                    throw std::runtime_error("Invalid relational operator");
            }
        } else {
            switch (relational) {
                case base::Relational::AND:
                    return thisBool && otherBool;
                case base::Relational::OR:
                    return thisBool || otherBool;
                case base::Relational::RE:
                    return false;
                case base::Relational::RNE:
                    return true;
                default:
                    throw std::runtime_error("Invalid relational operator");
            }
        }
    }

    void Time::setFormat(const utils::TimeFormat &new_format) {
        format = new_format;
    }

    std::string Time::getTimeString() const {
        std::array<char, 64> buffer{};
        switch (format) {
            case utils::TimeFormat::ISO:
                std::snprintf(buffer.data(), buffer.size(), "%04d-%02d-%02d %02d:%02d:%02d",
                              year, month, day, hour, minute, second);
                break;
            case utils::TimeFormat::US:
                std::snprintf(buffer.data(), buffer.size(), "%02d/%02d/%04d %02d:%02d:%02d",
                              month, day, year, hour, minute, second);
                break;
            case utils::TimeFormat::European:
                std::snprintf(buffer.data(), buffer.size(), "%02d/%02d/%04d %02d:%02d:%02d",
                              day, month, year, hour, minute, second);
                break;
            case utils::TimeFormat::Timestamp:
                std::snprintf(buffer.data(), buffer.size(), "%lld",
                              std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
                break;
            default:
                throw std::runtime_error("Invalid date format");
        }
        return std::string(buffer.data());
    }

    bool Time::convertToBool() const {
        return year > 0 && month >= 1 && month <= 12 && day >= 1 && day <= 31 &&
               hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59;
    }

    void Time::addDays(int days) {
        if (days == 0) {
            return;
        }
        day += days;
    }

    void Time::addYears(int years) {
        if (years == 0) {
            return;
        }
        year += years;
    }

    void Time::addSeconds(int seconds) {
        if (seconds == 0) {
            return;
        }
        second += seconds;
    }

    void Time::addMonths(int months) {
        if (months == 0) {
            return;
        }
        month += months;
    }

    void Time::addHours(int hours) {
        if (hours == 0) {
            return;
        }
        hour += hours;
    }

    void Time::addMinutes(int minutes) {
        if (minutes == 0) {
            return;
        }
        minute += minutes;
    }
}

namespace core::memory{
    RVM_Scope::RVM_Scope(std::string name): name_(std::move(name)) {}

    RVM_Scope::RVM_Scope(std::string name, const std::unordered_map<std::string, std::shared_ptr<base::RVM_Data>> &dataMap)
        : name_(std::move(name)) {
        nameMap.reserve(dataMap.size());
        this->dataMap.reserve(dataMap.size());
        for (const auto& [key, value] : dataMap) {
            const auto dataId = id::DataID(key, name_);
            nameMap.emplace(key, dataId);
            this->dataMap.emplace(dataId.idstring, std::move(value));
        }
    }

    const std::string &RVM_Scope::getName() const noexcept {
        return name_;
    }

    void RVM_Scope::setName(std::string name) noexcept {
        name_ = std::move(name);
    }

    bool RVM_Scope::contains(const std::string &name) const noexcept {
        return nameMap.find(name) != nameMap.end();
    }

    void RVM_Scope::addDataByName(const std::string &name, const std::shared_ptr<base::RVM_Data> &data) {
        auto id = id::DataID{name, name_};
        if (auto [it, inserted] = dataMap.try_emplace(id.idstring, data); inserted) {
            nameMap[name] = id;
        } else {
            throw base::errors::DuplicateKeyError(unknown_, unknown_,
                                                  {"Duplicate key: [" + name + "]"},
                                                  {"Change duplicate key names."});
        }
    }

    void RVM_Scope::addDataByID(const id::DataID &dataId, const std::shared_ptr<base::RVM_Data> &data) {
        if (!dataId.getScopeName().empty() && dataId.getScopeName() != name_) {
            throw base::RVM_Error(base::ErrorType::IDError, unknown_, unknown_,
                                  {"This error is caused by a mismatch in the scope name of the data ID used.",
                                   "Error ID: " + dataId.idstring,
                                   "Scope name: " + name_},
                                  {"Check whether the scope of the data is currently valid."});
        }
        if (auto [it, inserted] = dataMap.try_emplace(dataId.idstring, data); inserted) {
            nameMap[dataId.getName()] = dataId;
        } else {
            throw base::errors::DuplicateKeyError(unknown_, unknown_,
                                                  {"Duplicate key: [" + dataId.idstring + "]"},
                                                  {"Change duplicate key names."});
        }
    }

    std::shared_ptr<base::RVM_Data> RVM_Scope::findDataByIdString(const std::string &idString) const {
        if (const auto &it = dataMap.find(idString); it != dataMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::shared_ptr<base::RVM_Data> RVM_Scope::findDataByName(const std::string &name) const {
        if (const auto &nameIt = nameMap.find(name); nameIt != nameMap.end()){
            return findDataByIdString(nameIt->second.idstring);
        }
        return nullptr;
    }

    std::pair<std::optional<id::DataID>, std::shared_ptr<base::RVM_Data>>
    RVM_Scope::findDataInfoByName(const std::string &name) const {
        if (const auto &nameIt = nameMap.find(name); nameIt != nameMap.end()){
            return {nameIt->second, findDataByIdString(nameIt->second.idstring)};
        }
        return {std::nullopt, nullptr};
    }

    std::shared_ptr<base::RVM_Data> RVM_Scope::findDataByID(const id::DataID &varId) const {
        return findDataByIdString(varId.idstring);
    }

    bool RVM_Scope::updateDataByID(const id::DataID &id, std::shared_ptr<base::RVM_Data> newData) {
        if (auto it = dataMap.find(id.idstring); it != dataMap.end()) {
            if (!it->second->updateData(newData)){
                it->second = newData;
            }
            return true;
        }
        return false;
    }

    bool RVM_Scope::updateDataByName(const std::string &name, std::shared_ptr<base::RVM_Data> newData) noexcept {
        if (auto it = nameMap.find(name); it != nameMap.end()){
            return updateDataByID(it->second, std::move(newData));
        }
        return false;
    }

    std::optional<id::DataID> RVM_Scope::getDataIDByName(const std::string &name) const {
        if (const auto &it = nameMap.find(name); it != nameMap.end()){
            return {it->second};
        }
        return {std::nullopt}; // Fixme: 可能有bug
    }

    bool RVM_Scope::removeDataByID(const id::DataID &dataId) {
        const auto &fullId = dataId.idstring;
        if (dataMap.erase(fullId) > 0) {
            nameMap.erase(dataId.getName());
            return true;
        }
        return false;
    }

    bool RVM_Scope::removeDataByName(const std::string &name) {
        if (const auto &it = nameMap.find(name); it != nameMap.end()){
            return removeDataByID(it->second);
        }
        return false;
    }

    void RVM_Scope::collectSelf() {
        for (auto it = dataMap.begin(); it != dataMap.end(); ) {
            if (it->second.use_count() == 1) {
                nameMap.erase(it->first);
                it = dataMap.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool RVM_Scope::thisNeedCollect() const {
        return needCollect;
    }

    void RVM_Scope::printInfo() const {
        auto &out = *base::RVM_IO::getInstance();
        out << "Space Name: " << (name_.empty() ? "<unnamed>" : name_) << "\n";
        out << "Data Count: " << dataMap.size() << "\n";

        if (!dataMap.empty()) {
            out << "Contents:\n";
            for (const auto &[id, data]: dataMap) {
                out << "    " << id << ": ";
                if (data) {
                    out << data->getDataInfo();
                } else {
                    out << " (null data)";
                }
                out << "\n";
            }
        }
        out << "---\n";
    }

    size_t RVM_Scope::size() const noexcept { return dataMap.size(); }

    bool RVM_Scope::empty() const noexcept { return dataMap.empty(); }

    void RVM_Scope::clear() noexcept {
        dataMap.clear();
        nameMap.clear();
    }

    base::InstID RVM_Scope::getInstID() const { return instID; }

    RVM_Memory &core::memory::RVM_Memory::getInstance() {
        static RVM_Memory instance;
        return instance;
    }

    RVM_Memory::ScopePtr
    RVM_Memory::acquireScope(const std::string &prefix, const std::string &scopeName) {
        std::lock_guard<std::mutex> lock(mutex_);
        return acquireScopeNoLock(prefix + scopeName, scopeName.empty());
    }

    void RVM_Memory::releaseScope(const core::memory::RVM_Memory::ScopePtr &space) {
        if (!space) return;
        std::lock_guard<std::mutex> lock(mutex_);
        releaseScopeNoLock(space);
    }

    void RVM_Memory::releaseScope() {
        std::lock_guard<std::mutex> lock(mutex_);
        releaseScopeNoLock(getCurrentScopeNoLock());
    }

    RVM_Memory::DataPair
    RVM_Memory::addData(const std::string &name, const RVM_Memory::DataPtr &data, const std::string &scopeName) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto scope = scopeName.empty() ? getCurrentScopeNoLock() : findScopeByNameNoLock(scopeName);
        if (!scope) {
            throw std::runtime_error("No valid scope available");
        }
        if (scope->contains(name)){
            throw base::errors::DuplicateKeyError(unknown_, unknown_, {}, {});
        }
        id::DataID dataId(name, scope->getName());
        scope->addDataByID(dataId, data);
        return {dataId, data};
    }

    RVM_Memory::DataPair RVM_Memory::addGlobalData(const std::string &name, const RVM_Memory::DataPtr &data) {
        if (!globalScope_->contains(name)){
            id::DataID dataId(name, globalScope_->getName());
            globalScope_->addDataByID(dataId, data);
            return {dataId, data};
        }
        throw base::errors::DuplicateKeyError(unknown_, unknown_, {}, {});
    }

    void RVM_Memory::addGlobalDataBatch(const std::unordered_map<std::string, DataPtr> &datas) {
        for (const auto &[name, data]: datas) {
            addGlobalData(name, data);
        }
    }

    RVM_Memory::DataPtr RVM_Memory::findDataByID(const id::DataID &dataId) const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return findDataByIDNoLock(dataId);
    }

    RVM_Memory::DataPair RVM_Memory::findDataByName(const std::string &name) const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return findDataByNameNoLock(name);
    }

    void RVM_Memory::updateDataByID(const id::DataID &dataId, RVM_Memory::DataPtr newData) {
        std::lock_guard<std::mutex> lock(mutex_);
        updateDataNoLock(dataId, std::move(newData));
    }

    void RVM_Memory::updateDataByIDNoLock(const id::DataID &dataId, RVM_Memory::DataPtr newData)  {
        updateDataNoLock(dataId, std::move(newData));
    }

    void RVM_Memory::updateDataByName(const std::string &name, const RVM_Memory::DataPtr &newData) {
        std::lock_guard<std::mutex> lock(mutex_);
        updateDataByNameNoLock(name, newData);
    }

    void RVM_Memory::removeDataByID(const id::DataID &dataId) {
        std::lock_guard<std::mutex> lock(mutex_);
        removeDataNoLock(dataId);
    }

    void RVM_Memory::removeDataByName(const std::string &name) {
        std::lock_guard<std::mutex> lock(mutex_);
        removeDataByNameNoLock(name);
    }

    bool RVM_Memory::hasActiveScope() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return !activeScopes_.empty();
    }

    size_t RVM_Memory::getActiveScopeCount() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return activeScopes_.size();
    }

    RVM_Memory::ScopePtr RVM_Memory::getCurrentScope() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return getCurrentScopeNoLock();
    }

    void RVM_Memory::reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        clearAllScopes();
        initializePool(INITIAL_POOL_SIZE);
    }

    size_t RVM_Memory::getActiveScopeCount() {
        std::lock_guard<std::mutex> lock(mutex_);
        return activeScopes_.size();
    }

    size_t RVM_Memory::getFreeScopeCount() {
        std::lock_guard<std::mutex> lock(mutex_);
        return freeScopes_.size();
    }

    void RVM_Memory::printPoolInfo() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto &out = *base::RVM_IO::getInstance();
        out << "\n=== Space Pool Information ===\n";
        out << "Active Spaces: " << activeScopes_.size() << "\n";
        out << "Free Spaces: " << freeScopes_.size() << "\n";
        out << "Named Spaces: " << namedScopes_.size() << "\n\n";

        out << "--- Active Spaces Details ---\n";
        for (const auto &space: activeScopes_) {
            space->printInfo();
        }

        out << "\n--- Named Spaces Index ---\n";
        for (const auto &[name, weakSpace]: namedScopes_) {
            out << "Name: " << name;
            if (auto space = weakSpace.lock()) {
                out << " (active, size: " << space->size() << ")";
            } else {
                out << " (expired)";
            }
            out << "\n";
        }

        out << "\n--- Recent Access Cache ---\n";
        size_t validCache = 0;
        for (const auto &[id, weakData]: recentAccessCache_) {
            if (weakData.second.lock()) {
                validCache++;
            }
        }
        out << "Cache Entries: " << recentAccessCache_.size()
            << " (Valid: " << validCache << ")\n";

        out << "\n=== End of Pool Information ===\n\n";
    }

    RVM_Memory::ScopePtr RVM_Memory::acquireScopeNoLock(const std::string &name, bool unnamed) {
        if (freeScopes_.empty()) {
            expandPool();
        }

        auto scope = std::move(freeScopes_.front());
        freeScopes_.pop();

        const std::string &spaceName = unnamed ?
                                       DEFAULT_SCOPE_PREFIX + name + std::to_string(nextDefaultNameId_++) :
                                       name;

        scope->setName(spaceName);
        namedScopes_[spaceName] = scope;
        activeScopes_.push_back(scope);
        currentScope_ = scope;
        return scope;
    }

    void RVM_Memory::releaseScopeNoLock(const RVM_Memory::ScopePtr &scope) {
        const auto &name = scope->getName();
        if (!name.empty()) {
            namedScopes_.erase(name);
        }
        activeScopes_.remove(scope);
        recentAccessCache_.clear();

        scope->clear();
        scope->setName("");
        freeScopes_.push(scope);
        currentScope_ = activeScopes_.empty() ? nullptr : activeScopes_.back();
    }

    RVM_Memory::ScopePtr RVM_Memory::getCurrentScopeNoLock() const noexcept {
        return currentScope_ ? currentScope_ : (activeScopes_.empty() ? nullptr : activeScopes_.back());
    }

    RVM_Memory::ScopePtr RVM_Memory::findScopeByNameNoLock(const std::string &name) const {
        auto it = namedScopes_.find(name);
        return (it != namedScopes_.end()) ? it->second.lock() : nullptr;
    }

    void RVM_Memory::updateDataNoLock(const id::DataID &dataId, const RVM_Memory::DataPtr &newData) const {
        if (dataId.getScopeName() == globalScope_->getName()){
            globalScope_->updateDataByID(dataId, std::move(newData));
            return;
        }
        if (currentScope_){
            if(currentScope_->updateDataByID(dataId, newData)){
                return;
            }
        }
        auto scope = findScopeByNameNoLock(dataId.getScopeName());
        if (scope){
            scope->updateDataByID(dataId, std::move(newData));
            return;
        } else {
            throw base::errors::MemoryError(unknown_, unknown_, {}, {});
        }
    }

    void RVM_Memory::updateDataByNameNoLock(const std::string &name, RVM_Memory::DataPtr newData) {
        if (name == "_") {
            return;
        }
        const auto &cache_name = name + "@" + getCurrentScopeNoLock()->getName();
        if (auto cachedData = recentAccessCache_.find(cache_name);
                cachedData != recentAccessCache_.end()) {
            auto &[id, weakData] = *cachedData;
            if (auto data = weakData.second.lock()) {
                if (data->updateData(newData)) {
                    return;
                }
            }
        }
        if (currentScope_){
            if (currentScope_->contains(name)){
                currentScope_->updateDataByName(name, std::move(newData));
                return;
            }
        }
        if (globalScope_->contains(name)){
            globalScope_->updateDataByName(name, std::move(newData));
            return;
        }
        for (auto &space: std::ranges::reverse_view(activeScopes_)) {
            if (auto get_res = space->getDataIDByName(name); get_res.has_value()){
                if (space->updateDataByID(get_res.value(), std::move(newData))) {
                    recentAccessCache_[cache_name] = {get_res.value(), newData};
                    return;
                }
            }
        }
        throw base::errors::MemoryError(unknown_, unknown_,{},{});
    }

    void RVM_Memory::removeDataNoLock(const id::DataID &varId) {
        if (currentScope_){
            if (currentScope_->removeDataByID(varId)) {
                return;
            }
        }
        for (auto &space: activeScopes_) {
            if (space->removeDataByID(varId)) {
                return;
            }
        }
        throw base::errors::MemoryError(unknown_, unknown_,{},{});
    }

    void RVM_Memory::removeDataByNameNoLock(const std::string &name) {
        if (currentScope_){
            if (currentScope_->removeDataByName(name)) {
                return;
            }
        }
        for (auto &space: std::ranges::reverse_view(activeScopes_)) {
            if (auto get_res = space->getDataIDByName(name); get_res.has_value()) {
                space->removeDataByID(get_res.value());
                return;
            }
        }
        throw base::errors::MemoryError(unknown_, unknown_,{},{});
    }

    void RVM_Memory::clearAllScopes() {
        if (currentScope_) {
            currentScope_ = nullptr;
        }

        for (auto &space: activeScopes_) {
            space->clear();
        }
        activeScopes_.clear();

        while (!freeScopes_.empty()) {
            auto space = std::move(freeScopes_.front());
            space->clear();
            freeScopes_.pop();
        }

        namedScopes_.clear();
        recentAccessCache_.clear();
        nextDefaultNameId_ = 0;
        globalScope_->clear();
    }

    void RVM_Memory::initializePool(size_t size) {
        for (size_t i = 0; i < size; ++i) {
            freeScopes_.push(std::make_shared<RVM_Scope>());
        }
    }

    void RVM_Memory::expandPool(size_t size) {
        initializePool(size);
    }

    RVM_Memory::DataPtr RVM_Memory::findDataByIDNoLock(const id::DataID &dataId) const {
        if (!dataId.getScopeName().empty()) {
            if (dataId.getScopeName() == globalScope_->getName()){
                return globalScope_->findDataByID(dataId);
            }
            if (auto space = findScopeByNameNoLock(dataId.getScopeName())) {
                return space->findDataByID(dataId);
            }
        }
        if (currentScope_){
            if (auto data = currentScope_->findDataByID(dataId)){
                return data;
            }
        }
        for (const auto &space: std::ranges::reverse_view(activeScopes_)) {
            if (auto data = space->findDataByID(dataId)) {
                return data;
            }
        }
        return nullptr;
    }

    RVM_Memory::DataPair RVM_Memory::findDataByNameNoLock(const std::string &name) const {
        const auto &cache_name = name + "@" + getCurrentScopeNoLock()->getName();
        if (auto it = recentAccessCache_.find(cache_name); it != recentAccessCache_.end()) {
            if (auto data = it->second.second.lock()) {
                return {it->second.first, data};
            }
            recentAccessCache_.erase(it);
        }
        if (auto dataID = globalScope_->getDataIDByName(name); dataID.has_value()){
            return {dataID.value(), globalScope_->findDataByIdString(dataID->idstring)};
        }
        if (currentScope_){
            if (auto dataID = currentScope_->getDataIDByName(name); dataID.has_value()) {
                return {dataID.value(), currentScope_->findDataByIdString(dataID->idstring)};
            }
        }
        for (const auto &space: std::ranges::reverse_view(activeScopes_)) {
            if (auto data = space->findDataInfoByName(name);
                    data.first.has_value() && data.second) {
                // 更新缓存
                recentAccessCache_[cache_name] = {data.first.value(), data.second};
                return {data.first.value(), data.second};
            }
        }
        return {id::DataID(name), nullptr};
    }

    void RVM_Memory::start(int intervalMs) {
        if (running_) return;
        running_ = true;
        base::RVM_ThreadPool::getInstance().enqueue([this, intervalMs]() {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
                collectGarbage();
            }
        });
    }

    void RVM_Memory::stop() {
        running_ = false;
    }

    void RVM_Memory::setCurrentScopeByName(const std::string &scope_name) {
        currentScope_ = findScopeByNameNoLock(scope_name);
        if (!currentScope_){
            throw base::errors::MemoryError(unknown_, unknown_,{},{});
        }
    }

    const RVM_Memory::ScopePtr &RVM_Memory::getGlobalScope() const {
        return globalScope_;
    }

    RVM_Memory::RVM_Memory(size_t initialSize) : nextDefaultNameId_(0), running_(false) {
        initializePool(initialSize);
    }

    void RVM_Memory::collectGarbage() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& space : activeScopes_) {
            if (space->thisNeedCollect()){
                releaseScopeNoLock(space);
                continue;
            }
            space->collectSelf();
        }
    }
}