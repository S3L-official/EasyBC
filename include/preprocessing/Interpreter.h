//
// Created by Septi on 4/18/2023.
//

#ifndef EASYBC_INTERPRETER_H
#define EASYBC_INTERPRETER_H


#include "ASTNode.h"
#include "Value.h"
#include "ValueCommon.h"

extern int yylineno;

class CodeGenBlock{
public:
    ValuePtr returnValue;
    std::map<std::string, ValuePtr> env;
    std::map<std::string, std::vector<int>> arraySizes;
    std::vector<ValuePtr> parameters;
    std::map<std::string, std::vector<std::string>> arrayNames;
    std::vector<ValuePtr> sequence;
};

typedef std::shared_ptr<CodeGenBlock> CodeGenBlockPtr;


class Interpreter {
private:
    std::vector<CodeGenBlockPtr> blockStack;
    std::vector<ProcValuePtr> procs;

public:
    static std::vector<std::string> functionName;
    Interpreter() {}

    void generateCode(ASTNode::NBlock& root) {
        // 这里好像不需要在compute前后分别进行pushbBlock和popBlock，
        // 因为会多添加一个空的 CodeGenBlockPtr对象和
        // 弹出最后一个CodeGenBlockPtr对象，
        // 如果main函数在最后写，表现就是没有main函数对应的CodeGenBlockPtr实例
        pushBlock();
        CodeGenBlockPtr topDefs = blockStack.back();
        ValuePtr valuePtr = root.compute(*this);
        popBlock();
    }

    ValuePtr getValue(std::string type, std::string name, bool isParameter) {
        ValuePtr re = nullptr;
        if(isParameter && type == "uint") {
            re = std::make_shared<ConstantValue>(name);
        }
        else if(isParameter) {
            ValuePtr va = std::make_shared<ParameterValue>(name);
            va->setVarType(VarTypeTrans(type));
            re = va;
        } else if(type == "uint") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint);
            re = va;
        } else if(type == "uint1") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint1);
            re = va;
        } else if(type == "uint4") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint4);
            re = va;
        } else if(type == "uint6") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint6);
            re = va;
        } else if(type == "uint8") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint8);
            re = va;
        } else if(type == "uint12") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint12);
            re = va;
        } else if(type == "uint16") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint16);
            re = va;
        } else if(type == "uint32") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint32);
            re = va;
        } else if(type == "uint64") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint64);
            re = va;
        } else if(type == "uint128") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint128);
            re = va;
        } else if(type == "uint256") {
            ValuePtr va = std::make_shared<ConstantValue>(name);
            va->setVarType(ASTNode::uint256);
            re = va;
        } else {
            assert(false);
        }
        return re;
    }

    void addToEnv(std::string name, ValuePtr value) {
        blockStack.back()->env[name] = value;
    }

    ValuePtr getFromEnv(std::string name) {
        assert(blockStack.back()->env.count(name));
        for(auto it = blockStack.rbegin(); it != blockStack.rend(); it++){
            if( (*it)->env.find(name) != (*it)->env.end() ){
                return (*it)->env[name];
            }
        }
        return nullptr;
    }

    void pushBlock() {
        CodeGenBlockPtr codeGenBlock = std::make_shared<CodeGenBlock>();
        codeGenBlock->returnValue = nullptr;
        blockStack.push_back(codeGenBlock);
    }

    void popBlock() {
        CodeGenBlockPtr codeGenBlock = blockStack.back();
        blockStack.pop_back();
    }

    void addParameter(std::vector<ValuePtr>& para) {
        blockStack.back()->parameters.insert(blockStack.back()->parameters.end(), para.begin(), para.end());
    }

    void addParameter(const ValuePtr& para) {
        blockStack.back()->parameters.push_back(para);
    }

    const std::vector<ValuePtr>& getParameter() {
        return blockStack.back()->parameters;
    }

    std::vector<int> getArraySize(std::string name){
        for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++){
            if( (*it)->arraySizes.find(name) != (*it)->arraySizes.end() ){
                return (*it)->arraySizes[name];
            }
        }
        return blockStack.back()->arraySizes[name];
    }

    void setArraySize(std::string name, std::vector<int> value){
        blockStack.back()->arraySizes[name] = value;
    }

    void setArrayNames(std::string name, std::vector<std::string>& names) {
        blockStack.back()->arrayNames[name] = names;
    }

    std::vector<std::string> getArrayNames(std::string name) {
        if(blockStack.back()->arrayNames.count(name) == 0)
            assert(false);
        return blockStack.back()->arrayNames[name];
    }

    void setCurrentReturnValue(ValuePtr value){
        blockStack.back()->returnValue = value;
    }

    ValuePtr getCurrentReturnValue(){
        return blockStack.back()->returnValue;
    }

    // 新增该函数为了可以获取所有的blockStack，用于roundFunction和cipherFunction中添加全局的evn
    std::vector<CodeGenBlockPtr> getblockStack() {
        return blockStack;
    }

    std::vector<ValuePtr>& getSequence() {
        return blockStack.back()->sequence;
    }

    void addToSequence(ValuePtr loc) {
        blockStack.back()->sequence.push_back(loc);
    }

    std::map<std::string, ValuePtr> getEnv() {
        return blockStack.back()->env;
    }

    void addProc(ValuePtr proc) {
        ProcValuePtr procPtr = std::dynamic_pointer_cast<ProcValue>(proc);
        procs.push_back(procPtr);
    }

    const std::vector<ProcValuePtr> &getProcs() const {
        return procs;
    }

    ValuePtr getProc(std::string name) {
        for(auto ele : procs) {
            if(ProcValue* proc = dynamic_cast<ProcValue*>(ele.get())) {
                if(proc->getProcName() == name) {
                    return ele;
                } else {
                    continue;
                }
            } else {
                assert(false);
                return nullptr;
            }
        }
        assert(false);
        return nullptr;
    }

    // 只被用于获取 ConcreteNumValue 的具体 number
    int value_of(ValuePtr value) {
        if(ConcreteNumValue* number = dynamic_cast<ConcreteNumValue*>(value.get())) {
            return number->getNumer();
        } else {
            assert(false);
            return 0;
        }
    }

    /*
     * computeIndex 用于 NArrayAssignment 的 compute，因此只用于对某个 一维数组 的某个 index 的 evaluate，
     * 即，输入参数 expList 应该长度为1，并且可以 evaluate 出具体值，
     * 所以 expList 中不应该出现 round function 的参数等无法 evaluate 的参数。
     * 那么 computeIndex 应该返回的是一个 vector<int>, 并且 index 的计算只有两种情况：
     * 其是某一个元素的 index，或者是空（对应这个整个数组的所有index）
     *    1） 对数组的某个元素进行赋值，index 是一个确定的值，
     *    也就是说此时 index（即expList） 只能是一个，且确定的可以evaluate的值;
     *    2） 对整个数组进行赋值，此时 index 应该是整个数组的 index, 但是因为传入参数是在 NArrayAssignment 中
     *    传入的 this->arrayIndex->dimons，所以此时 expList 应该为空，对应着需要后续会被赋值的数组应该是整个数组，
     *    而不是其中某一个元素。并且最终的 index 返回应该也是一个空的 vector<int>
     *
     * 注：对于我们的语言，二维数组只能出现在pboxm，ffm以及常数取位，不存在对某个二维数组的某个元素进行赋值时.
     * 我们这里的实现兼容上述的两种情况，但是也同时考虑了二维数组取index，具体是把一维转换为二维进行取值。
     * 二维索引 a[i][j]的返回 index 应该是 i * a[i].size + j
     * */
    std::vector<int> computeIndex(std::string name, ASTNode::NExpressionListPtr expList) {
        // 这里返回的是一个 vector<int>，如果 vector<int> 的 size 为1，则是一维数组，为2则为二维数组
        auto sizeVec = getArraySize(name);
        int index = 0;
        int dimons = sizeVec.size(); // dimons 的数目即对应数组是 一维 还是 二维
        for(int i = 0; i < expList->size(); i++) {
            auto arrayIndex = expList->at(i);
            if(arrayIndex->getTypeName() == "NInteger") {
                ASTNode::NIntegerPtr ident = std::dynamic_pointer_cast<ASTNode::NInteger>(arrayIndex);
                if(i != dimons - 1) {
                    int temp = 1;
                    for(int j = i + 1; j < sizeVec.size(); j++)
                        temp *= sizeVec[j];
                    index += temp * ident->getValue();
                } else
                    index += ident->getValue();
            } else if(arrayIndex->getTypeName() == "NIdentifier") {
                ASTNode::NIdentifierPtr ident = std::dynamic_pointer_cast<ASTNode::NIdentifier>(arrayIndex);
                if(i != dimons - 1) {
                    int temp = 1;
                    for (int j = i + 1; j < sizeVec.size(); j++)
                        temp *= sizeVec[j];
                    index += temp * value_of(ident->compute(*this));
                }
                else
                    index += value_of(ident->compute(*this));
            } else if(arrayIndex->getTypeName() == "NBinaryOperator") {
                ValuePtr arraySizeValue = arrayIndex->compute(*this);
                if(!ValueCommon::isNoParameter(arraySizeValue)) {
                    // 当arraySizeValue中含有parameter的时候
                }
                int value = value_of(arraySizeValue);
                if(i != dimons - 1) {
                    int temp = 1;
                    for (int j = i + 1; j < sizeVec.size(); j++)
                        temp *= sizeVec[j];
                    index += temp * value;
                }
                else
                    index += value;
            }
            else {
                std::cout << "debug info : " << arrayIndex->getTypeName() << std::endl;
                assert(false);
            }
        }

        int number = 1;
        if(expList->size() < sizeVec.size()) {
            // 意味着要返回一个数组的值了，不是单个的值
            // from index to
            // 计算个数就是后面所有的乘起来
            for(int i = expList->size(); i < sizeVec.size(); i++) {
                number *= sizeVec.at(i);
            }
        }

        std::vector<int> res;
        if(expList->size() == sizeVec.size()) {
            res.push_back(index);
        } else {
            for(int i = index; i < index + number; i++) {
                res.push_back(i);
            }
        }
        return res;
    }




    // computeIndexWithSymbol 没有重构
    // 按照目前debug的结果，调用该函数时，走的分支都是 if(arrayIndex->getTypeName() == "NInteger") { }
    // 并且直接返回了 index = identValue;
    // 考虑是 index 直接是
    /*
     * 在这里我们要计算index，在这里我们分为两种情况：
     * 1. index是包含输入参数的，此时index具体的值不能被计算出来，只能用symbol方式表示出来；
     * 2. index没有包含输入参数，index具体值可以被计算出，此时计算index的具体值又分为三种情况：
     *      1）当数组是一维的时候，index的值直接计算即可；
     *      2）当数组是二维的时候，index的值需要由两个值来决定，具体的，
     *         数组a中，a[i1][i2]的具体index值需要求解的是 i1 * rowSize + i2
     *      3）当数组是一维，取到数组某一个元素时，又对该元素的某位bit取值，如 rc[r-1][i]
     * */
    std::vector<ValuePtr> computeIndexWithSymbol(std::string name, ASTNode::NExpressionListPtr expList) {
        auto sizeVec = getArraySize(name);
        ValuePtr index = std::make_shared<ConcreteNumValue>("index", 0);
        int dimons = sizeVec.size();
        for(int i = 0; i < expList->size(); i++) {
            auto arrayIndex = expList->at(i);
            if(arrayIndex->getTypeName() == "NInteger") {
                ASTNode::NIntegerPtr ident = std::dynamic_pointer_cast<ASTNode::NInteger>(arrayIndex);
                ValuePtr identValue = std::make_shared<ConcreteNumValue>("ident", ident->getValue());
                if(i != dimons - 1) {
                    ValuePtr temp = std::make_shared<ConcreteNumValue>("1", 1);
                    for(int j = i + 1; j < sizeVec.size(); j++) {
                        std::shared_ptr<ConcreteNumValue> sizeVecJ = std::make_shared<ConcreteNumValue>("j", sizeVec[j]);
                        temp = std::make_shared<InternalBinValue>("temp", temp, sizeVecJ, ASTNode::Operator::FFTIMES);
                    }
                    temp = std::make_shared<InternalBinValue>("temp", temp, identValue, ASTNode::Operator::FFTIMES);
                    index = std::make_shared<InternalBinValue>("index", index, temp, ASTNode::Operator::ADD);
                }
                else {
                    // 这里我们先注释，因为实际上是可以直接得到具体的index的值的
                    //index = std::make_shared<InternalBinValue>("index", index, identValue, ASTNode::Operator::ADD);
                    index = identValue;
                }
            }
            else if(arrayIndex->getTypeName() == "NIdentifier") {
                ASTNode::NIdentifierPtr ident = std::dynamic_pointer_cast<ASTNode::NIdentifier>(arrayIndex);
                ValuePtr identValue = ident->compute(*this);
                //这里注释是因为有一些输入相关的表达式时，其值可能无法计算，
                if(!ValueCommon::isNoParameter(identValue)) {
                    // 这里直接将表达式保存了
                    index = identValue;
                    //assert(false);
                }
                else if(i != dimons - 1) {
//                    int temp = 1;
                    ValuePtr temp = std::make_shared<ConcreteNumValue>("1", 1);
                    for(int j = i + 1; j < sizeVec.size(); j++) {
                        std::shared_ptr<ConcreteNumValue> sizeVecJ = std::make_shared<ConcreteNumValue>("j", sizeVec[j]);
                        temp = std::make_shared<InternalBinValue>("temp", temp, sizeVecJ, ASTNode::Operator::FFTIMES);
                    }
                    temp = std::make_shared<InternalBinValue>("temp", temp, identValue, ASTNode::Operator::FFTIMES);
                    index = std::make_shared<InternalBinValue>("index", index, temp, ASTNode::Operator::ADD);
                }
                else {
                    //index = std::make_shared<InternalBinValue>("index", index, identValue, ASTNode::Operator::ADD);
                    index = identValue;
                }
            }
            else if(arrayIndex->getTypeName() == "NBinaryOperator" || arrayIndex->getTypeName() == "NArrayIndex") {
                ValuePtr arraySizeValue = arrayIndex->compute(*this);
                // 如果arraysize中是含有参数的表达式
                if(!ValueCommon::isNoParameter(arraySizeValue)) {
                    // 当arraySizeValue中含有parameter的时候
                    index = arraySizeValue;
                    // 也将该index标记为symbol, 这里我先不改index的symbol的标记了。后续有用到再进行更改，现在debug的有点混乱
                    // index->set_symbol_value_f();
                    //assert(false);
                }
                    // 如果dimons为1，那么数组为一维数组，所以在这里我们添加一部分约束，即也要求dimons为一维数组。
                else if(i != dimons - 1 and dimons != 1) {
                    ValuePtr temp = std::make_shared<ConcreteNumValue>("1", 1);
                    for(int j = i + 1; j < sizeVec.size(); j++) {
                        std::shared_ptr<ConcreteNumValue> sizeVecJ = std::make_shared<ConcreteNumValue>("j", sizeVec[j]);
                        temp = std::make_shared<InternalBinValue>("temp", temp, sizeVecJ, ASTNode::Operator::FFTIMES);
                    }
                    temp = std::make_shared<InternalBinValue>("temp", temp, arraySizeValue, ASTNode::Operator::FFTIMES);
                    index = std::make_shared<InternalBinValue>("index", index, temp, ASTNode::Operator::ADD);
                }
                    // 当数组为一维数组，且index要访问数组某元素的某个bit时
                    // 此时需要将两次不同的数组访问用操作符联系起来，因此新定义操作符 "NEWINDEX"
                    // 第二个index访问某个元素的某个bit的valueptr即为 arraySizeValue
                else if (i != dimons - 1 and dimons == 1 ) {
                    index = std::make_shared<InternalBinValue>("index", index, arraySizeValue, ASTNode::Operator::NEWINDEX);
                }
                else {
                    //index = std::make_shared<InternalBinValue>("index", index, arraySizeValue, ASTNode::Operator::ADD);
                    index = arraySizeValue;
                }
//                }
            } else {
                assert(false);
            }
        }

        int number = 1;
        if(expList->size() < sizeVec.size()) {
            // 意味着要返回一个数组的值了，不是单个的值
            // from index to
            // 计算个数就是后面所有的乘起来
            for(int i = expList->size(); i < sizeVec.size(); i++) {
                number *= sizeVec.at(i);
            }
        }

        std::vector<ValuePtr> res;
        if(expList->size() == sizeVec.size()) {
            res.push_back(index);
        } else {
            res.push_back(index);
            for(int i = 1; i < number; i++) {
                ValuePtr temp = std::make_shared<ConcreteNumValue>("i", i);
                res.push_back(std::make_shared<InternalBinValue>(std::to_string(i), index, temp, ASTNode::Operator::ADD));
            }
        }
        return res;
    }


    static ASTNode::Type VarTypeTrans(std::basic_string<char> type) {
        if (type == "uint")
            return ASTNode::Type::uint;
        else if (type == "uint1")
            return ASTNode::Type::uint1;
        else if (type == "uint4")
            return ASTNode::Type::uint4;
        else if (type == "uint6")
            return ASTNode::Type::uint6;
        else if (type == "uint8")
            return ASTNode::Type::uint8;
        else if (type == "uint16")
            return ASTNode::Type::uint16;
        else if (type == "uint32")
            return ASTNode::Type::uint32;
        else if (type == "uint64")
            return ASTNode::Type::uint64;
        else if (type == "uint128")
            return ASTNode::Type::uint128;
        else if (type == "uint256")
            return ASTNode::Type::uint256;
        return ASTNode::Type::uint;
    }

    static ASTNode::Type int2Type(int size) {
        switch (size) {
            case 0:
                return ASTNode::Type::uint;
            case 1:
                return ASTNode::Type::uint1;
            case 2:
                return ASTNode::Type::uint4;
            case 3:
                return ASTNode::Type::uint4;
            case 4:
                return ASTNode::Type::uint4;
            case 5:
                return ASTNode::Type::uint6;
            case 6:
                return ASTNode::Type::uint6;
            case 7:
                return ASTNode::Type::uint8;
            case 8:
                return ASTNode::Type::uint8;
            case 11:
                return ASTNode::Type::uint12;
            case 12:
                return ASTNode::Type::uint12;
            case 13:
                return ASTNode::Type::uint16;
            case 14:
                return ASTNode::Type::uint16;
            case 15:
                return ASTNode::Type::uint16;
            case 16:
                return ASTNode::Type::uint16;
            case 32:
                return ASTNode::Type::uint32;
            case 64:
                return ASTNode::Type::uint64;
            case 128:
                return ASTNode::Type::uint128;
            case 256:
                return ASTNode::Type::uint256;
            default:
                return ASTNode::Type::uint;
        }
    }


    static int getVarTypeSize(ASTNode::Type type) {
        if (type == ASTNode::Type::uint) {
            return 0;
        }
        else if (type == ASTNode::Type::uint1) {
            return 1;
        }
        else if (type == ASTNode::Type::uint4) {
            return 4;
        }
        else if (type == ASTNode::Type::uint6) {
            return 6;
        }
        else if (type == ASTNode::Type::uint8) {
            return 8;
        }
        else if (type == ASTNode::Type::uint12) {
            return 12;
        }
        else if (type == ASTNode::Type::uint16) {
            return 16;
        }
        else if (type == ASTNode::Type::uint32) {
            return 32;
        }
        else if (type == ASTNode::Type::uint64) {
            return 64;
        }
        else if (type == ASTNode::Type::uint128) {
            return 128;
        }
        else if (type == ASTNode::Type::uint256) {
            return 256;
        }
        return 0;
    }
};


#endif //EASYBC_INTERPRETER_H
