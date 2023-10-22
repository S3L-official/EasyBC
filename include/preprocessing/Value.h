//
// Created by Septi on 4/18/2023.
//

#ifndef EASYBC_VALUE_H
#define EASYBC_VALUE_H

#include <string>
#include <utility>
#include <vector>
#include <set>
#include <map>
#include <ASTNode.h>
#include <cmath>


enum ValueType {
    VTInternalBinValue,
    VTInternalUnValue,
    VTArrayValue,
    VTBoxValue,
    VTProcValue,
    VTParameterValue,
    VTProcCallValue,
    VTProcCallValueIndex,
    VTConcreteNumValue,
    VTConstantValue,
    VTArrayValueIndex
};


// 这里我声明了一个Value父类，在其中没有声明一个pure virtual函数，意味着我允许他们在没有实现的，可以继承default实现
class Value {
private:
    std::vector<ValuePtr> parents;
    std::string name;
    ASTNode::Type type = ASTNode::null;
    bool symbol_value = false;
    bool isViewOrTouint = false;
public:
    Value(std::string valueName) {
        name = std::move(valueName);
    }
    virtual ~Value()= default;

    void set_symbol_value_f() {
        if (!symbol_value) symbol_value = true;
        else symbol_value = false;
    }

    bool get_symbol_value_f() const {return symbol_value;}

    void setVarType(ASTNode::Type ty){
        type = ty;
    }

    ASTNode::Type getVarType() {
        return type;
    }

    virtual std::string toString() {
        assert(false);
        return "1";
    }

    virtual int value_of(const std::map<std::string, ValuePtr>& env) {
        assert(false);
        return 0;
    }

    virtual ValuePtr copy() {
        return nullptr;
    }

    void addParents(const ValuePtr& valuePtr) {
        parents.push_back(valuePtr);
    }

    std::vector<ValuePtr> &getParents() {
        return parents;
    }

    void setParents(const std::vector<ValuePtr> &ps) {
        Value::parents = ps;
    }

    virtual void replaceChild(ValuePtr v1, ValuePtr v2){
        std::cout << "no implementation" << std::endl;
        assert(false);
    }

    virtual void removeChild(ValuePtr child) {
        assert(false);
    };

    virtual ValueType getValueType() = 0;

    virtual std::string getName() {
        return name;
    }

    void setName(std::string newName) {
        name = std::move(newName);
    }

    void setIsViewOrTouint () {
        if (isViewOrTouint)
            isViewOrTouint = false;
        else
            isViewOrTouint = true;
    }

    bool getIsViewOrTouint () {
        return isViewOrTouint;
    }
};


typedef std::shared_ptr<Value> ValuePtr;


// 下面两个类是运算类型的value，可以通过value_of函数来evaluate表达式的具体值，
// 但是easybc还支持其他语法糖的操作，比如View和touint，这个在实际分析时我们并不对其进行直接计算，而是以symbol_value的方式保存实例对象。
class InternalBinValue : public Value {
private:
    ValuePtr left;
    ValuePtr right;
    ASTNode::Operator op;
public:
    InternalBinValue(std::string name, ValuePtr left, ValuePtr right, ASTNode::Operator op) :
            Value(std::move(name)),left(std::move(left)), right(std::move(right)), op(op) {}

    virtual std::string toString() override {
        if(op == ASTNode::Operator::ADD)
            return left->toString() + " + " + right->toString();
        else if(op == ASTNode::Operator::AND)
            return left->toString() + " & " + right->toString();
        else if(op == ASTNode::Operator::FFTIMES)
            return left->toString() + " * " + right->toString();
        else if(op == ASTNode::Operator::XOR)
            return left->toString() + " ^ " + right->toString();
        else if(op == ASTNode::Operator::OR)
            return left->toString() + " | " + right->toString();
        else if(op == ASTNode::Operator::LSH)
            return left->toString() + " << " + right->toString();
        else if(op == ASTNode::Operator::RSH)
            return left->toString() + " >> " + right->toString();
        else if(op == ASTNode::Operator::RLSH)
            return left->toString() + " <<< " + right->toString();
        else if(op == ASTNode::Operator::RRSH)
            return left->toString() + " >>> " + right->toString();
        else if(op == ASTNode::Operator::MINUS)
            return left->toString() + " - " + right->toString();
            // 这里我们添加两个操作 mod 和 divide，之前是没有的
        else if(op == ASTNode::Operator::MOD)
            return left->toString() + " % " + right->toString();
        else if(op == ASTNode::Operator::DIVIDE)
            return left->toString() + " / " + right->toString();
            // 新增boxop
        else if (op == ASTNode::Operator::BOXOP)
            return left->toString() + " boxop " + right->toString();
            // 新增symbolindex
        else if (op == ASTNode::Operator::SYMBOLINDEX)
            return left->toString() + " symbolindex " + right->toString();
        else {
            assert(false);
            return left->toString() + " * " + right->toString();
        }
    }

    virtual int value_of(const std::map<std::string, ValuePtr>& env) override {
        int lvalue = left->value_of(env);
        int rvalue = right->value_of(env);
        if(this->getOp() == ASTNode::Operator::ADD) {
            return lvalue + rvalue;
        } else if(this->getOp() == ASTNode::Operator::MINUS){
            return lvalue - rvalue;
        } else if(this->getOp() == ASTNode::Operator::FFTIMES){
            return lvalue * rvalue;
        } else if(this->getOp() == ASTNode::Operator::DIVIDE){
            return lvalue / rvalue;
        } else if(this->getOp() == ASTNode::Operator::OR){
            return lvalue | rvalue;
        } else if(this->getOp() == ASTNode::Operator::XOR){
            return lvalue ^ rvalue;
        } else if(this->getOp() == ASTNode::Operator::LSH){
            return lvalue << rvalue;
        } else if(this->getOp() == ASTNode::Operator::RSH){
            return lvalue >> rvalue;
            // 新增的循环移位,这里还需要专门实现函数来实现<<<和>>>操作
        } else if(this->getOp() == ASTNode::Operator::RLSH){
            return lvalue << rvalue;
        } else if(this->getOp() == ASTNode::Operator::RRSH){
            return lvalue >> rvalue;
        } else if(this->getOp() == ASTNode::Operator::AND){
            return lvalue & rvalue;
        } else if(this->getOp() == ASTNode::Operator::MOD){
            return lvalue % rvalue;
        } else {
            assert(false);
            return 0;
        }
    }

    const ValuePtr &getLeft() {return left;}

    const ValuePtr &getRight() {return right;}

    ASTNode::Operator getOp() {return op;}

    virtual void replaceChild(ValuePtr v1, ValuePtr v2) override {
        if(left == v1) {
            left = v2;
        } else if(right == v1) {
            right = v2;
        } else {
            assert(left == v2 || right == v2);
        }
    }

    // 对bin来说，参数是remove哪个child
    virtual void removeChild(ValuePtr child) override {
        std::vector<ValuePtr>& childParents = child->getParents();
        auto it = childParents.begin();
        while(it != childParents.end()) {
            if(it->get() == this) {
                childParents.erase(it);
                break;
            } else {
                it++;
            }
        }
    }

    // 这是这个类自己的函数
    ValuePtr removeAnotherChild(ValuePtr child) {
        if(left == child) {
            removeChild(right);
            return right;
        }
        else if(right == child) {
            removeChild(left);
            return left;
        }
        else
            assert(false);
    }

    virtual ValueType getValueType() override {
        return ValueType::VTInternalBinValue;
    }
};


typedef std::shared_ptr<InternalBinValue> InternalBinValuePtr;


class InternalUnValue : public Value {
private:
    ValuePtr rand;
    ASTNode::Operator op;
public:
    InternalUnValue(std::string name, ValuePtr rand, ASTNode::Operator op) :
            Value(std::move(name)), rand(std::move(rand)), op(op) {}

    virtual std::string toString() override {
        if(op == ASTNode::Operator::MINUS)
            return "MINUS " + rand->toString();
        else if(op == ASTNode::Operator::NOT)
            return "NOT " + rand->toString();
        else if (op == ASTNode::Operator::TOUINT)
            return "TOUINT " + rand->toString();
        else {
            assert(false);
            return "1";
        }
    }
    virtual int value_of(const std::map<std::string, ValuePtr>& env) override {
        int randValue = rand->value_of(env);
        if(op == ASTNode::Operator::MINUS)
            return 0 - randValue;
        else if(op == ASTNode::Operator::NOT) {
            if (randValue)
                return false;
            else
                return true;
        }
        else {
            assert(false);
            return 1;
        }
    }

    const ValuePtr &getRand() {return rand;}

    ASTNode::Operator getOp() {return op;}

    virtual void replaceChild(ValuePtr v1, ValuePtr v2) override {
        assert(rand = v1);
        rand = v2;
    }

    virtual void removeChild(ValuePtr child) override {
        // 左孩子是random，那么就remove右孩子
        std::vector<ValuePtr>& childParents = rand->getParents();
        auto it = childParents.begin();
        while(it != childParents.end()) {
            if(it->get() == this) {
                childParents.erase(it);
                break;
            } else {
                it++;
            }
        }
    }

    virtual ValueType getValueType() override {
        return ValueType::VTInternalUnValue;
    }
};


typedef std::shared_ptr<InternalUnValue> InternalUnValuePtr;


class ArrayValue : public Value {
private:
    std::vector<ValuePtr> arrayValue;
public:
    ArrayValue(std::string name, std::vector<ValuePtr> arrayValue) :
            Value(name), arrayValue(arrayValue) {}

    virtual std::string toString() override {
        std::string res = "(Array type)";
        for(int i = 0; i < arrayValue.size(); i++) {
            if(i == 0) {
                if(!arrayValue[i]) {
                    res += "NULL";
                } else {
                    res += arrayValue[i]->toString();
                }
            } else {
                if(!arrayValue[i]) {
                    res += ", NULL";
                } else {
                    res += ", " + arrayValue[i]->toString();
                }
            }
        }
        return res;
    }

    void setArrayValue(std::vector<ValuePtr> arrayVal) {
        this->arrayValue = arrayVal;
    }

    ValuePtr getValueAt(int i) {
        return arrayValue[i];
    }

    void setValueAt(int loc, ValuePtr v) {
        arrayValue[loc] = v;
    }

    const std::vector<ValuePtr> &getArrayValue() {
        return arrayValue;
    }

    virtual void replaceChild(ValuePtr v1, ValuePtr v2) override {
        bool flag = false;
        for(auto ele : arrayValue) {
            if(ele == v1) {
                flag = true;
                ele = v2;
            }
        }
        //assert(flag);
    }

    virtual ValueType getValueType() override {
        return ValueType::VTArrayValue;
    }
};


typedef std::shared_ptr<ArrayValue> ArrayValuePtr;


// 该类型用于表示四种box：sbox，pbox，pboxm，ffm
// 其中，成员变量rowSize表示box每行的size，
// 即，当sbox或pbox时，rowSize等于sbox或pbox的size；
// 当pboxm或ffm时，rowSize等于每行subvector的size
class BoxValue : public Value {
private:
    std::string boxType;
    int rowSize;
    std::vector<ValuePtr> boxValue;
public:
    BoxValue(std::string name, std::string boxType, int rowSize, const std::vector<ValuePtr> &boxValue) :
            Value(name), boxType(boxType), rowSize(rowSize), boxValue(boxValue) {}

    virtual std::string toString() override {
        std::string res = "(Box type)";
        for(int i = 0; i < boxValue.size(); i++) {
            if(i == 0) {
                if(!boxValue[i]) {
                    res += "NULL";
                } else {
                    res += boxValue[i]->toString();
                }
            } else {
                if(!boxValue[i]) {
                    res += ", NULL";
                } else {
                    res += ", " + boxValue[i]->toString();
                }
            }
        }
        return res;
    }

    int getRowSize() { return rowSize;}

    ValuePtr getValueAt(int i) {
        return boxValue[i];
    }

    std::string getBoxType() {
        return boxType;
    }

    void setValueAt(int loc, ValuePtr v) {
        boxValue[loc] = v;
    }

    const std::vector<ValuePtr> &getBoxValue() {
        return boxValue;
    }

    virtual void replaceChild(ValuePtr v1, ValuePtr v2) override {
        bool flag = false;
        for(auto ele : boxValue) {
            if(ele == v1) {
                flag = true;
                ele = v2;
            }
        }
        //assert(flag);
    }

    virtual ValueType getValueType() override {
        return ValueType::VTBoxValue;
    }
};


typedef std::shared_ptr<BoxValue> BoxValuePtr;


class Procedure {
private:
    std::string procName;
    std::vector<ValuePtr> parameters;
    std::map<std::string, ValuePtr> env;
    std::vector<ValuePtr> block;
    ValuePtr returns;
    std::vector<std::set<ValuePtr>> domOfReturn;

    // 下面三个成员变量用以标识函数的类型
    bool isRndf = false;
    bool isKschd = false;
    bool isFn = false;
    bool isSboxf = false;

public:
    Procedure(std::string procName, std::vector<ValuePtr> parameters,
              std::map<std::string, ValuePtr> env, std::vector<ValuePtr> block, ValuePtr returns) :
            procName(std::move(procName)),
            parameters(std::move(parameters)),
            env(std::move(env)),
            block(std::move(block)),
            returns(std::move(returns)) {}

    const std::string &getProcName() {
        return procName;
    }

    const std::vector<ValuePtr> &getParameters() {
        return parameters;
    }

    const std::map<std::string, ValuePtr> &getEnv() {
        return env;
    }

    std::vector<ValuePtr> &getBlock() {
        return block;
    }

    const ValuePtr &getReturns() {
        return returns;
    }

    virtual std::string toString() {
        if(procName == "KeyExpansion") {
            return "ProcName: KeyExpansion jump\n";
        }
        std::string result;
        result += "ProcName: " + procName + "\n";

        result += "Parameters: ";
        for(int i = 0; i < parameters.size(); i++) {
            if(i == 0)
                result += parameters[i]->toString();
            else
                result += ", " + parameters[i]->toString();
        }
        result += "\n";
        result += "Body: \n";
        for(const auto& ele : block) {
            if(ele != nullptr) {
                result += ele->toString() + "\n";
                // cout << ele->toString() << endl;
            } else {
                result += "NULL\n";
            }
        }
        result += "Returns: \n";
        if(returns)
            result += returns->toString();
        return result;
    }

    void setIsRndf() {
        if (isRndf)
            isRndf = false;
        else
            isRndf = true;
    }

    bool getIsRndf() const {
        return isRndf;
    }

    void setIsKschd() {
        if (isKschd)
            isKschd = false;
        else
            isKschd = true;
    }

    bool getIsKschd() {
        return isKschd;
    }

    void setIsFn() {
        if (isFn)
            isFn = false;
        else
            isFn = true;
    }

    bool getIsFn() const {
        return isFn;
    }

    void setIsSboxf() {
        if (isSboxf)
            isSboxf = false;
        else
            isSboxf = true;
    }

    bool getIsSboxf() const {
        return isSboxf;
    }

    void setBlock(const std::vector<ValuePtr> &block) {
        Procedure::block = block;
    }

    void setDomofReturn(const std::vector<std::set<ValuePtr>>& domOfReturn1) {
        domOfReturn = domOfReturn1;
    }

    std::vector<std::set<ValuePtr>>& getDomOfReturn() {
        return domOfReturn;
    }
};


typedef std::shared_ptr<Procedure> ProcedurePtr;


class ProcValue : public Value {
private:
    ProcedurePtr procedurePtr;
public:
    ProcValue(std::string name, ProcedurePtr procedurePtr) :
            Value(std::move(name)), procedurePtr(std::move(procedurePtr)) {}

    virtual std::string toString() override {
        return this->procedurePtr->toString();
    }

    std::string getProcName() {
        return procedurePtr->getProcName();
    }

    const std::vector<ValuePtr> &getParameters() {
        return procedurePtr->getParameters();
    }

    const ProcedurePtr &getProcedurePtr() {
        return procedurePtr;
    }

    virtual ValueType getValueType() override {
        return ValueType::VTProcValue;
    }
};


typedef std::shared_ptr<ProcValue> ProcValuePtr;


class ParameterValue : public Value {
private:
    std::string name;
public:
    ParameterValue(std::string name) : Value(name), name(std::move(name)) {}

    virtual std::string toString() override {
        return "Parameter(" + name + ")";
    }

    std::string getName() override{
        return name;
    }

    virtual ValueType getValueType() override {
        return ValueType::VTParameterValue;
    }
};


typedef std::shared_ptr<ParameterValue> ParameterValuePtr;


class ConcreteNumValue : public Value {
private:
    int numer;
public:
    ConcreteNumValue(std::string name, int numer) : Value(name), numer(numer) {}

    int getNumer() {
        return numer;
    }

    virtual std::string toString() override {
        return std::to_string(numer);
    }

    virtual int value_of(const std::map<std::string, ValuePtr>& env) override {
        return numer;
    }

    virtual ValueType getValueType() override {
        return ValueType::VTConcreteNumValue;
    }
};


typedef std::shared_ptr<ConcreteNumValue> ConcreteNumValuePtr;


class ConstantValue : public Value {
private:
public:
    ConstantValue(std::string name) : Value(name) {}

    virtual ValueType getValueType() override {
        return ValueType::VTConstantValue;
    }

    virtual std::string toString() override {
        return "Parameter(Constant(" + Value::getName() + "))";
    }
};

typedef std::shared_ptr<ConstantValue> ConstantValuePtr;

class ProcCallValue : public Value {
private:
    ProcedurePtr procedurePtr;
    std::vector<ValuePtr> arguments;
    std::vector<int> callsite;
    std::vector<std::set<ValuePtr>> domOfProcCall;

public:
    ProcCallValue(std::string name, ProcedurePtr procedurePtr, std::vector<ValuePtr> arguments, std::vector<int> callsite) :
            Value(std::move(name)),
            procedurePtr(std::move(procedurePtr)),
            arguments(std::move(arguments)),
            callsite(std::move(callsite)) {}

    virtual std::string toString() override {
        std::string res = "";
        res += procedurePtr->getProcName() + "(";
//        for(int i = 0; i < arguments.size(); i++) {
//            if(i == 0) {
//                res += arguments[i]->toString();
//            } else {
//                res += ", " + arguments[i]->toString();
//            }
//        }
        res += "arguments";
        res += ")";
        std::string callsites;
        for(auto ele : callsite) {
            callsites += "@" + std::to_string(ele);
        }
        res += callsites;
        return res;
    }

    const ProcedurePtr &getProcedurePtr() {
        return procedurePtr;
    }

    const std::vector<ValuePtr> &getArguments() {
        return arguments;
    }

    std::vector<int> getCallsite() {
        return callsite;
    }

    virtual void replaceChild(ValuePtr v1, ValuePtr v2) override{
        bool flag = false;
        for(int i = 0; i < arguments.size(); i++) {
            // 如果第i个实际参数是v1，就换成v2
            if(arguments[i] == v1) {
                arguments[i] = v2;
                flag = true;
            }
            // 如果第i个实际参数已经替换为了v2,就不用替换
            if(arguments[i] == v2) {
                flag = true;
            }
            if(ArrayValue* array = dynamic_cast<ArrayValue*>(arguments[i].get())) {
                for(int i = 0; i < array->getArrayValue().size(); i++) {
                    if(array->getValueAt(i) == v1) {
                        array->setValueAt(i, v2);
                        flag = true;
                    } else if(array->getValueAt(i) == v2){
                        flag = true;
                    }
                }
            }
        }
        assert(flag);
    }

    std::set<ValuePtr> getDomOfNumberX(int i) {
        std::set<ValuePtr> temp;
        if(domOfProcCall.empty()) {
            return temp;
        } else {
            assert(domOfProcCall.size() > i);
            return domOfProcCall.at(i);
        }
    }

    //对于函数调用， remove的是本身
    virtual void removeChild(ValuePtr child) override {
        ValuePtr returnNode = nullptr;
        std::vector<ValuePtr>& childParents = child->getParents();
        auto it = childParents.begin();
        while(it != childParents.end()) {
            if(it->get() == this) {
                childParents.erase(it);
                break;
            } else {
                it++;
            }
        }
    }

    virtual ValueType getValueType() override {
        return ValueType::VTProcCallValue;
    }
};


typedef std::shared_ptr<ProcCallValue> ProcCallValuePtr;


class ProcCallValueIndex : public Value{
private:
    ValuePtr procCallValuePtr;
    int number;
public:
    ProcCallValueIndex(std::string name, const ValuePtr &procCallValuePtr, int number) :
            Value(name), procCallValuePtr(procCallValuePtr), number(number) {}

    const ValuePtr &getProcCallValuePtr() {
        return procCallValuePtr;
    }

    int getNumber() {
        return number;
    }

    virtual std::string toString() override {
        return procCallValuePtr->toString() + "[" + std::to_string(number) + "]";
    }

    virtual ValueType getValueType() override {
        return ValueType::VTProcCallValueIndex;
    }
};


typedef std::shared_ptr<ProcCallValueIndex> ProcCallValueIndexPtr;


class ArrayValueIndex : public Value {
private:
    ValuePtr arrayValuePtr;
    ValuePtr symbolIndex;
public:
    ArrayValueIndex(const std::string &valueName, ValuePtr arrayValuePtr, ValuePtr symbolIndex) :
            Value(valueName),
            arrayValuePtr(std::move(arrayValuePtr)),
            symbolIndex(std::move(symbolIndex)) {}

    virtual std::string toString() override{
        return arrayValuePtr->getName() + "[SYMBOL]";
    }
    virtual ValueType getValueType() override {
        return ValueType::VTArrayValueIndex;
    }

    const ValuePtr &getArrayValuePtr() {
        return arrayValuePtr;
    }

    const ValuePtr &getSymbolIndex() {
        return symbolIndex;
    }
};

#endif //EASYBC_VALUE_H
