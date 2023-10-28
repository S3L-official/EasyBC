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

    int value_of(ValuePtr value) {
        if(ConcreteNumValue* number = dynamic_cast<ConcreteNumValue*>(value.get())) {
            return number->getNumer();
        } else {
            assert(false);
            return 0;
        }
    }

    std::vector<int> computeIndex(std::string name, ASTNode::NExpressionListPtr expList) {
        auto sizeVec = getArraySize(name);
        int index = 0;
        int dimons = sizeVec.size();
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
                assert(false);
            }
        }

        int number = 1;
        if(expList->size() < sizeVec.size()) {
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
                    index = identValue;
                }
            }
            else if(arrayIndex->getTypeName() == "NIdentifier") {
                ASTNode::NIdentifierPtr ident = std::dynamic_pointer_cast<ASTNode::NIdentifier>(arrayIndex);
                ValuePtr identValue = ident->compute(*this);
                if(!ValueCommon::isNoParameter(identValue)) {
                    index = identValue;
                    //assert(false);
                }
                else if(i != dimons - 1) {
                    ValuePtr temp = std::make_shared<ConcreteNumValue>("1", 1);
                    for(int j = i + 1; j < sizeVec.size(); j++) {
                        std::shared_ptr<ConcreteNumValue> sizeVecJ = std::make_shared<ConcreteNumValue>("j", sizeVec[j]);
                        temp = std::make_shared<InternalBinValue>("temp", temp, sizeVecJ, ASTNode::Operator::FFTIMES);
                    }
                    temp = std::make_shared<InternalBinValue>("temp", temp, identValue, ASTNode::Operator::FFTIMES);
                    index = std::make_shared<InternalBinValue>("index", index, temp, ASTNode::Operator::ADD);
                }
                else {
                    index = identValue;
                }
            }
            else if(arrayIndex->getTypeName() == "NBinaryOperator" || arrayIndex->getTypeName() == "NArrayIndex") {
                ValuePtr arraySizeValue = arrayIndex->compute(*this);
                if(!ValueCommon::isNoParameter(arraySizeValue)) {
                    index = arraySizeValue;
                }
                else if(i != dimons - 1 and dimons != 1) {
                    ValuePtr temp = std::make_shared<ConcreteNumValue>("1", 1);
                    for(int j = i + 1; j < sizeVec.size(); j++) {
                        std::shared_ptr<ConcreteNumValue> sizeVecJ = std::make_shared<ConcreteNumValue>("j", sizeVec[j]);
                        temp = std::make_shared<InternalBinValue>("temp", temp, sizeVecJ, ASTNode::Operator::FFTIMES);
                    }
                    temp = std::make_shared<InternalBinValue>("temp", temp, arraySizeValue, ASTNode::Operator::FFTIMES);
                    index = std::make_shared<InternalBinValue>("index", index, temp, ASTNode::Operator::ADD);
                }
                else if (i != dimons - 1 and dimons == 1 ) {
                    index = std::make_shared<InternalBinValue>("index", index, arraySizeValue, ASTNode::Operator::NEWINDEX);
                }
                else {
                    index = arraySizeValue;
                }
//                }
            } else {
                assert(false);
            }
        }

        int number = 1;
        if(expList->size() < sizeVec.size()) {
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
