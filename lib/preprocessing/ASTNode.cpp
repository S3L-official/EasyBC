//
// Created by Septi on 4/15/2023.
//
#include "ASTNode.h"
#include "Value.h"
#include "Interpreter.h"

std::vector<std::string> Interpreter::functionName;

extern int ASTNodeCounter;

extern std::map<std::string, std::vector<int>> allBox;
extern std::map<std::string, std::vector<int>> pboxM;
extern std::map<std::string, int> pboxMSize;
extern std::map<std::string, std::vector<int>> Ffm;



ValuePtr ASTNode::NIdentifier::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::string nodeName = this->getName();
    ValuePtr valuePtr = interpreter.getFromEnv(nodeName);
    assert(valuePtr);
    return valuePtr;
}

ValuePtr ASTNode::NArrayIndex::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::string name = this->arrayName->getName();

    if (this->dimons->size() == 2) {
        NExpressionPtr d1 = dimons->at(0);
        NExpressionPtr d2 = dimons->at(1);
        NExpressionListPtr tempDimons = std::make_shared<NExpressionList>(0);
        tempDimons->push_back(d1);

        NArrayIndex temp(this->arrayName, 1);
        temp.dimons = tempDimons;
        ValuePtr tempCompute = temp.compute(interpreter);
        interpreter.addToEnv(tempCompute->getName(), tempCompute);

        NIdentifier tempIdent(tempCompute->getName());
        this->setArrayName(std::make_shared<NIdentifier>(tempIdent));
        NExpressionListPtr tempDimons2 = std::make_shared<NExpressionList>(0);
        tempDimons2->push_back(d2);
        this->dimons = tempDimons2;
        name = tempCompute->getName();
    }

    auto sizeVec = interpreter.getArraySize(name);
    ValuePtr array = interpreter.getFromEnv(name);
    ArrayValue *arrayV = dynamic_cast<ArrayValue *>(array.get());

    if (arrayV == nullptr) {
        int size;
        std::vector<ValuePtr> res;
        if (InternalBinValue* array2Bin = dynamic_cast<InternalBinValue*>(array.get())) {
            if (array2Bin->getOp() == ASTNode::Operator::BOXOP) {
                if (ArrayValue *tArray = dynamic_cast<ArrayValue*>(array2Bin->getRight().get())) {
                    size = tArray->getArrayValue().size();
                }
                else if (InternalUnValue *tInternalUnValue = dynamic_cast<InternalUnValue*>(array2Bin->getRight().get())) {
                    if (tInternalUnValue->getOp() == TOUINT) {
                        size = interpreter.getVarTypeSize(tInternalUnValue->getVarType());
                    } else
                        assert(false);
                } else
                    assert(false);
                for (int i = 0; i < size; ++i) {
                    ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                    InternalBinValuePtr tArray2bin = std::make_shared<InternalBinValue>(
                            array->getName() + "_" + std::to_string(i), array,
                            index, ASTNode::Operator::SYMBOLINDEX);
                    tArray2bin->setVarType(array->getVarType());
                    res.push_back(tArray2bin);
                }
            }
            else if (array2Bin->getOp() == ASTNode::Operator::SYMBOLINDEX and array2Bin->getVarType() != ASTNode::uint1) {
                size = this->transType2Int(array->getVarType());
                for (int i = 0; i < size; ++i) {
                    ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                    InternalBinValuePtr tArray2bin = std::make_shared<InternalBinValue>(
                            array->getName() + "_" + std::to_string(i), array,
                            index, ASTNode::Operator::SYMBOLINDEX);
                    tArray2bin->setVarType(uint1);
                    res.push_back(tArray2bin);
                }
            }
            else
                assert(false);
        }
        else if (InternalUnValue *array2Un = dynamic_cast<InternalUnValue*>(array.get())) {
            if (array2Un->getOp() == ASTNode::Operator::TOUINT) {
                ArrayValue *tArray = dynamic_cast<ArrayValue*>(array2Un->getRand().get());
                size = tArray->getArrayValue().size();
                for (int i = 0; i < size; ++i) {
                    ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                    InternalBinValuePtr tArray2bin = std::make_shared<InternalBinValue>(
                            array->getName() + "_" + std::to_string(i), array,
                            index, ASTNode::Operator::SYMBOLINDEX);
                    tArray2bin->setVarType(array->getVarType());
                    res.push_back(tArray2bin);
                }
            } else
                assert(false);
        }
        else if (ConcreteNumValue *array2Num = dynamic_cast<ConcreteNumValue*>(array.get())) {
            size = interpreter.getVarTypeSize(array2Num->getVarType());
            for (int i = 0; i < size; ++i) {
                ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                InternalBinValuePtr temp2bin = std::make_shared<InternalBinValue>(
                        name + "_" + std::to_string(i), array,
                        index, ASTNode::Operator::SYMBOLINDEX);
                temp2bin->setVarType(ASTNode::uint1);
                res.push_back(temp2bin);
            }
        }
        else if (ParameterValue *array2Param = dynamic_cast<ParameterValue*>(array.get())) {
            size = interpreter.getVarTypeSize(array2Param->getVarType());
            for (int i = 0; i < size; ++i) {
                ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                InternalBinValuePtr temp2bin = std::make_shared<InternalBinValue>(
                        name + "_" + std::to_string(i), array,
                        index, ASTNode::Operator::SYMBOLINDEX);
                temp2bin->setVarType(ASTNode::uint1);
                res.push_back(temp2bin);
            }
        }
        else if (ArrayValueIndex *array2arrIdx = dynamic_cast<ArrayValueIndex*>(array.get())) {
            size = interpreter.getVarTypeSize(array2arrIdx->getVarType());
            for (int i = 0; i < size; ++i) {
                ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                InternalBinValuePtr temp2bin = std::make_shared<InternalBinValue>(
                        name + "_" + std::to_string(i), array,
                        index, ASTNode::Operator::SYMBOLINDEX);
                temp2bin->setVarType(ASTNode::uint1);
                res.push_back(temp2bin);
            }
        }
        else
            assert(false);

        ValuePtr tArrayOut = std::make_shared<ArrayValue>(name, res);
        arrayV = dynamic_cast<ArrayValue *>(tArrayOut.get());
        interpreter.addToEnv(name, tArrayOut);
        std::vector<int> resArraySize;
        resArraySize.push_back(res.size());
        interpreter.setArraySize(name, resArraySize);
    }

    std::vector<ValuePtr> indexesSymbol = interpreter.computeIndexWithSymbol(name, this->dimons);

    bool indexHasSymbol = false;
    for(auto ele : indexesSymbol) {
        if(!ValueCommon::isNoParameter(ele)) {
            indexHasSymbol = true;
            interpreter.getEnv()[name]->set_symbol_value_f();
        }
    }

    if(!indexHasSymbol and !interpreter.getEnv()[name]->get_symbol_value_f()) {
        std::vector<int> indexes;
        for (auto ele : indexesSymbol) {
            int res = ele->value_of(interpreter.getEnv());
            indexes.push_back(res);
        }

        if(indexes.size() == 1) {
            if (arrayV->getValueAt(indexes[0])->getValueType() == ValueType::VTConcreteNumValue) {
                ValuePtr con = arrayV->getValueAt(indexes[0]);
                con->setName(arrayV->getName() + "_" + std::to_string(indexes[0]));
                con->setVarType(arrayV->getVarType());
                return con;
            } else
                return arrayV->getValueAt(indexes[0]);
        } else {
            std::vector<ValuePtr> res;
            std::string returnName = name + "_" + std::to_string(indexes.at(0)) + "_" + std::to_string(indexes.at(indexes.size() - 1));
            for(auto ele : indexes)
                res.push_back(arrayV->getValueAt(ele));
            return std::make_shared<ArrayValue>(returnName, res);
        }
    } else {
        if(indexesSymbol.size() == 1) {
            std::shared_ptr<ArrayValueIndex> res = std::make_shared<ArrayValueIndex>(name+"_symbol", array, indexesSymbol[0]);
            assert(res->getArrayValuePtr()->getValueType() == ValueType::VTArrayValue);
            res->setVarType(arrayV->getVarType());
            interpreter.addToEnv(name+"_symbol", res);
            return res;
        } else {
            std::vector<ValuePtr> res;
            std::string returnName = name + "symbol_symbol";
            for(auto ele : indexesSymbol) {
                std::shared_ptr<ArrayValueIndex> temp = std::make_shared<ArrayValueIndex>(name, array, ele);
                assert(temp->getArrayValuePtr()->getValueType() == ValueType::VTArrayValue);
                res.push_back(temp);
            }
            return std::make_shared<ArrayValue>(returnName, res);
        }
    }
}


ValuePtr ASTNode::NFunctionCall::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::string functionName = this->ident->getName();

    std::vector<ValuePtr> realParameters;
    for (auto arg : *(this->arguments)) {
        ValuePtr argPtr = arg->compute(interpreter);
        realParameters.push_back(argPtr);
    }

    ValuePtr proc = interpreter.getProc(functionName);
    ProcedurePtr procedurePtr = nullptr;
    if(auto* procValue = dynamic_cast<ProcValue*>(proc.get())) {
        procedurePtr = procValue->getProcedurePtr();
    } else {
        assert(false);
    }

    std::vector<int> callsites;
    callsites.push_back(this->callSite);
    std::string callsitesString = "";
    for (auto ele : callsites)
        callsitesString += "@" + std::to_string(ele);
    ProcCallValuePtr procCallValuePtr = std::make_shared<ProcCallValue>(procedurePtr->getProcName() + callsitesString,
                                                                        procedurePtr, realParameters, callsites);

    const ValuePtr& returnValue = procedurePtr->getReturns();
    auto* arrayValue = dynamic_cast<ArrayValue*>(returnValue.get());
    ValuePtr res;
    if(arrayValue) {
        std::vector<ValuePtr> returnArray;
        for (int i = 0; i < arrayValue->getArrayValue().size(); i++) {
            ValuePtr value = std::make_shared<ProcCallValueIndex>(procCallValuePtr->getName() + "_" + std::to_string(i),
                                                                  procCallValuePtr, i);
            value->setVarType(proc->getVarType());
            returnArray.push_back(value);
        }
        res = std::make_shared<ArrayValue>(procCallValuePtr->getName() + "_ret", returnArray);
        res->setVarType(proc->getVarType());
        for (auto ele : returnArray) {
            procCallValuePtr->addParents(ele);
        }
    } else {
        res = std::make_shared<ProcCallValueIndex>(procCallValuePtr->getName() + "_" + std::to_string(0), procCallValuePtr, 0);
        res->setVarType(proc->getVarType());
        procCallValuePtr->addParents(res);
    }

    for (auto ele : realParameters) {
        if (auto *array = dynamic_cast<ArrayValue *>(ele.get())) {
            for (auto item : array->getArrayValue()) {
                assert(item);
                if(item)
                    item->addParents(procCallValuePtr);
            }
        } else {
            ele->addParents(procCallValuePtr);
        }
    }
    interpreter.addToSequence(res);
    return res;
}


ValuePtr ASTNode::NBinaryOperator::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr L = this->lhs->compute(interpreter);
    assert(L);
    ValuePtr R = this->rhs->compute(interpreter);
    assert(R);

    ValuePtr res = std::make_shared<InternalBinValue>("", L, R, this->op);
    if (L->getVarType() != ASTNode::null)
        res->setVarType(L->getVarType());
    else if (R->getVarType() != ASTNode::null)
        res->setVarType(R->getVarType());

    bool isLeftConcreteNum = L->getValueType() == VTConcreteNumValue ? true : false;
    bool isRightConcreteNum = R->getValueType() == VTConcreteNumValue ? true : false;
    if(isLeftConcreteNum && isRightConcreteNum) {
        int lvalue = interpreter.value_of(L);
        int rvalue = interpreter.value_of(R);
        if(this->getOp() == ASTNode::Operator::ADD) {
            return std::make_shared<ConcreteNumValue>("", lvalue + rvalue);
        } else if(this->getOp() == ASTNode::Operator::MINUS){
            return std::make_shared<ConcreteNumValue>("", lvalue - rvalue);
        } else if(this->getOp() == ASTNode::Operator::FFTIMES){
            return std::make_shared<ConcreteNumValue>("", lvalue * rvalue);
        } else if(this->getOp() == ASTNode::Operator::DIVIDE){
            return std::make_shared<ConcreteNumValue>("", lvalue / rvalue);
        } else if(this->getOp() == ASTNode::Operator::MOD){
            return std::make_shared<ConcreteNumValue>("", lvalue % rvalue);
        } else if(this->getOp() == ASTNode::Operator::XOR){
            return std::make_shared<ConcreteNumValue>("", lvalue ^ rvalue);
        } else if(this->getOp() == ASTNode::Operator::AND){
            return std::make_shared<ConcreteNumValue>("", lvalue & rvalue);
        } else if(this->getOp() == ASTNode::Operator::OR){
            return std::make_shared<ConcreteNumValue>("", lvalue | rvalue);
        } else {
            assert(false);
        }
    }

    if (this->getOp() == ASTNode::Operator::RRSH) {
        ArrayValue* arrayValue = dynamic_cast<ArrayValue*>(L.get());
        assert(arrayValue);
        int displacement = interpreter.value_of(R);
        std::vector<ValuePtr> array = arrayValue->getArrayValue();
        std::vector<ValuePtr> rtn;
        for (int i = 0; i < displacement; ++i) {
            rtn.push_back(array[array.size() - displacement + i]);
        }
        for (int i = 0; i < array.size() - displacement; ++i) {
            rtn.push_back(array[i]);
        }
        ValuePtr rtnArrayValue = std::make_shared<ArrayValue>(L->getName(), rtn);
        rtnArrayValue->setVarType(arrayValue->getVarType());
        return rtnArrayValue;
    }
    else if (this->getOp() == ASTNode::Operator::RLSH) {
        ArrayValue* arrayValue = dynamic_cast<ArrayValue*>(L.get());
        assert(arrayValue);
        int displacement = interpreter.value_of(R);
        std::vector<ValuePtr> array = arrayValue->getArrayValue();
        std::vector<ValuePtr> rtn;
        for (int i = 0; i < array.size() - displacement; ++i) {
            rtn.push_back(array[displacement + i]);
        }
        for (int i = 0; i < displacement; ++i) {
            rtn.push_back(array[i]);
        }
        ValuePtr rtnArrayValue = std::make_shared<ArrayValue>(L->getName(), rtn);
        rtnArrayValue->setVarType(arrayValue->getVarType());
        return rtnArrayValue;
    }

    BoxValue* boxValue = dynamic_cast<BoxValue*>(L.get());
    if (boxValue != nullptr) {
        if (boxValue->getBoxType() == "pboxm") {
            std::string mName = boxValue->getName();
            ValuePtr mFfm = interpreter.getEnv()[mName + "_ffm"];
            BoxValue* boxFfm = dynamic_cast<BoxValue*>(mFfm.get());
            ArrayValue* rVector = dynamic_cast<ArrayValue*>(R.get());
            if (boxValue->getValueAt(0)->getVarType() == ASTNode::uint1) {
            }

            std::vector<int> pboxmValue;
            std::vector<ValuePtr> pboxmV = boxValue->getBoxValue();
            for (auto ele : pboxmV) {
                ConcreteNumValue* eleConcrete = dynamic_cast<ConcreteNumValue*>(ele.get());
                pboxmValue.push_back(eleConcrete->getNumer());
            }
            pboxM["pboxm" + mName] = pboxmValue;
            pboxMSize["pboxm" + mName] = interpreter.getVarTypeSize(boxValue->getVarType());

            std::vector<int> ffmValue;
            std::vector<ValuePtr> ffmV = boxFfm->getBoxValue();
            for (auto ele : ffmV) {
                ConcreteNumValue* eleConcrete = dynamic_cast<ConcreteNumValue*>(ele.get());
                ffmValue.push_back(eleConcrete->getNumer());
            }
            Ffm["pboxm" + mName] = ffmValue;
        }
        else {
            assert(false);
        }
    }

    L->addParents(res);
    R->addParents(res);

    if (L->getValueType() != ValueType::VTArrayValue and R->getValueType() != ValueType::VTArrayValue)
        interpreter.addToSequence(res);
    return res;
}


ValuePtr ASTNode::NUnaryOperator::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr rand = this->lhs->compute(interpreter);
    ValuePtr res = std::make_shared<InternalUnValue>("", rand, this->op);
    rand->addParents(res);
    interpreter.addToSequence(res);
    return res;
}


ValuePtr ASTNode::NInteger::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    if (this->value < 0) {
        std::cout << "TYPE ERROR : Integer value less than 0" << std::endl;
        abort();
    }
    ValuePtr valuePtr = std::make_shared<ConcreteNumValue>("", this->value);
    return std::make_shared<ConcreteNumValue>("", this->value);
}


ValuePtr ASTNode::NViewArray::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr array = interpreter.getEnv()[this->arrayName->getName()];
    auto *arrayV = dynamic_cast<ArrayValue *>(array.get());

    if (arrayV == nullptr) {
        std::cout << "TYPE ERROR : The operated object of View-Operation should be an array" << std::endl;
        abort();
    }

    int lowerBound = from->compute(interpreter)->value_of(interpreter.getEnv());
    int upperBound = to->compute(interpreter)->value_of(interpreter.getEnv());
    std::vector<ValuePtr> resArray;
    for (int i = lowerBound; i <= upperBound; ++i) {
        resArray.push_back(arrayV->getValueAt(i));
    }
    ValuePtr rtn = std::make_shared<ArrayValue>(this->arrayName->getName() + "_view", resArray);
    rtn->setVarType(resArray[0]->getVarType());
    return rtn;
}


ValuePtr ASTNode::NToUint::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::string name;
    std::vector<ValuePtr> expListTrans;
    int size = 0;
    if (this->expressionListPtr != nullptr) {
        for (auto exp : *expressionListPtr) {
            ValuePtr expValue = exp->compute(interpreter);

            ConcreteNumValue* expConcrete = dynamic_cast<ConcreteNumValue*>(expValue.get());
            if (expValue->getVarType() != ASTNode::Type::uint1 and expConcrete == nullptr) {
                std::cout << "TYPE ERROR : The type operated object of ToUint-Operation should be 'uint1'" << std::endl;
                abort();
            }

            name = expValue->getName();
            expListTrans.push_back(expValue);
            size++;
        }
    }
    else if (this->arrayName != nullptr) {
        ValuePtr array = interpreter.getEnv()[this->arrayName->getName()];
        auto *arrayV = dynamic_cast<ArrayValue *>(array.get());
        assert(arrayV);
        for (auto ele : arrayV->getArrayValue()) {
            if (ele->getVarType() != ASTNode::Type::uint1) {
                std::cout << "TYPE ERROR : The type operated object of ToUint-Operation should be 'uint1'" << std::endl;
                abort();
            }

            name = ele->getName();
            expListTrans.push_back(ele);
            size++;
        }
    }

    ValuePtr arrayValue = std::make_shared<ArrayValue>(name + "touint", expListTrans);
    ValuePtr rtn = std::make_shared<InternalUnValue>(name + "touint", arrayValue, ASTNode::Operator::TOUINT);
    rtn->setVarType(interpreter.int2Type(size));
    return rtn;
}


ValuePtr ASTNode::NBoxOperation::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr L = boxname->compute(interpreter);
    ValuePtr R = expressionPtr->compute(interpreter);

    if (L->getValueType() != ValueType::VTBoxValue) {
        std::cout << "TYPE ERROR : The left-hand of Box-Operation should be an Box" << std::endl;
        abort();
    } else if (R->getValueType() != ValueType::VTArrayValue and R->getValueType() != ValueType::VTInternalUnValue and R->getVarType() == ASTNode::uint1) {
        std::cout << "TYPE ERROR : The operated object of Box-Operation should be an array or a uints value" << std::endl;
        abort();
    }

    ValuePtr res = std::make_shared<InternalBinValue>("", L, R, ASTNode::Operator::BOXOP);
    res->set_symbol_value_f();
    res->setVarType(R->getVarType());
    interpreter.addToSequence(res);
    L->addParents(res);
    R->addParents(res);
    return res;
}


ValuePtr ASTNode::NAssignment::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr expValue = this->RHS->compute(interpreter);

    if (interpreter.getFromEnv(this->LHS->getName())->getVarType() != expValue->getVarType()) {
        ArrayValue* arrayValueL = dynamic_cast<ArrayValue*>(interpreter.getFromEnv(this->LHS->getName()).get());
        ArrayValue* arrayValueR = dynamic_cast<ArrayValue*>(expValue.get());
        ConcreteNumValue* expConcrete = dynamic_cast<ConcreteNumValue*>(expValue.get());
        if (((arrayValueL == nullptr and arrayValueR == nullptr) and (expConcrete == nullptr)) and expValue->getValueType() != VTInternalBinValue and
                !(this->LHS->getTypeName() == "NIdentifier" and this->RHS->getTypeName() == "NBinaryOperator")) {
            std::cout << "LHS type : " << interpreter.getFromEnv(this->LHS->getName())->getVarType() << std::endl;
            std::cout << "exp type : " << expValue->getVarType() << std::endl;
            std::cout << "TYPE ERROR : The assignment object is not of the same type as the assigned object"
                      << std::endl;
            abort();
        }
        else if (arrayValueL != nullptr and arrayValueR != nullptr) {
            if (arrayValueL->getArrayValue().size() != arrayValueR->getArrayValue().size()) {
                std::cout << "LHS type : " << interpreter.getFromEnv(this->LHS->getName())->getVarType() << std::endl;
                std::cout << "exp type : " << expValue->getVarType() << std::endl;
                std::cout << "TYPE ERROR : The array sizes of the two operated object are not same"
                          << std::endl;
                abort();
            }
        }
    }

    interpreter.addToEnv(this->LHS->getName(), expValue);
    if(expValue->getValueType() == ValueType::VTConcreteNumValue) {
    } else if(expValue->getValueType() == ValueType::VTInternalBinValue) {
        InternalBinValue* binExpValue = dynamic_cast<InternalBinValue*>(expValue.get());
        if (binExpValue->getRight()->getValueType() == ValueType::VTArrayValue) {
            std::vector<ValuePtr> arrayEles;
            auto op = binExpValue->getOp();
            if (op == ASTNode::Operator::BOXOP) {
                ArrayValue *binExpValueRight = dynamic_cast<ArrayValue *>(binExpValue->getRight().get());
                for (int i = 0; i < binExpValueRight->getArrayValue().size(); ++i) {
                    ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                    InternalBinValuePtr outbin = std::make_shared<InternalBinValue>(
                            this->LHS->getName() + "_" + std::to_string(i), expValue, index, ASTNode::Operator::SYMBOLINDEX);
                    outbin->setVarType(expValue->getVarType());
                    interpreter.addToSequence(outbin);
                    arrayEles.push_back(outbin);
                }
            }
            else if (op == ASTNode::Operator::ADD or op == ASTNode::Operator::MINUS or
                op == ASTNode::Operator::AND or op == ASTNode::Operator::OR or op == ASTNode::Operator::XOR) {
                ArrayValue *LarrayV = dynamic_cast<ArrayValue *>(binExpValue->getLeft().get());
                ArrayValue *RarrayV = dynamic_cast<ArrayValue *>(binExpValue->getRight().get());
                for (int i = 0; i < LarrayV->getArrayValue().size(); ++i) {
                    InternalBinValuePtr outbin = std::make_shared<InternalBinValue>(
                            this->LHS->getName() + "_" + std::to_string(i), LarrayV->getValueAt(i), RarrayV->getValueAt(i),
                            op);
                    interpreter.addToSequence(outbin);
                    arrayEles.push_back(outbin);
                }
            } else
                assert(false);
            ValuePtr valuePtr = std::make_shared<ArrayValue>(this->LHS->getName(), arrayEles);
            valuePtr->setVarType(interpreter.getFromEnv(this->LHS->getName())->getVarType());
            if (binExpValue->getOp() == ASTNode::Operator::BOXOP)
                valuePtr->setIsViewOrTouint();
            interpreter.addToEnv(this->LHS->getName(), valuePtr);
        }
        else {
            expValue->setName(this->LHS->getName());
            interpreter.addToEnv(this->LHS->getName(), expValue);
            return expValue;
        }
    } else if(expValue->getValueType() == ValueType::VTInternalUnValue) {
        InternalUnValue* unExpValue = dynamic_cast<InternalUnValue*>(expValue.get());
        if (unExpValue->getRand()->getValueType() == ValueType::VTArrayValue) {
            if (unExpValue->getOp() == ASTNode::Operator::TOUINT) {
                expValue->setIsViewOrTouint();
                expValue->setName(this->LHS->getName());
                interpreter.addToSequence(expValue);
                interpreter.addToEnv(this->LHS->getName(), expValue);
            } else
                assert(false);
            return expValue;
        } else
            assert(false);
    } else {
        expValue->setName(this->LHS->getName());
    }
    return expValue;
}


ValuePtr ASTNode::NArrayAssignment::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr right = this->expression->compute(interpreter);
    std::string name = this->arrayIndex->getArrayName()->getName();
    ValuePtr array = interpreter.getFromEnv(name);
    ArrayValue* arrayV = dynamic_cast<ArrayValue*>(array.get());
    assert(arrayV);

    std::vector<int> indexes = interpreter.computeIndex(name, this->arrayIndex->dimons);

    std::vector<ValuePtr> res;
    if(indexes.size() == 1) {
        int index = indexes[0];
        right->setName(name + "_" + std::to_string(index));
        ValuePtr valuePtr = interpreter.getEnv()[name];
        ASTNode::Type type = valuePtr->getVarType();
        right->setVarType(type);
        for(int i = 0; i < arrayV->getArrayValue().size(); i++) {
            if(i == index) {
                res.push_back(right);
            } else {
                res.push_back(arrayV->getArrayValue()[i]);
            }
        }
    } else {
        int arraySize = arrayV->getArrayValue().size();
        ArrayValue* rightArray = dynamic_cast<ArrayValue*>(right.get());
        if (!rightArray) {
            std::vector<ValuePtr> values;
            if (right->getValueType() == ValueType::VTInternalBinValue) {
                InternalBinValue* binRight = dynamic_cast<InternalBinValue*>(right.get());
                if (binRight->getOp() == ASTNode::Operator::BOXOP) {
                    for (int i = 0; i < arraySize; ++i) {
                        ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                        InternalBinValuePtr right2bin = std::make_shared<InternalBinValue>(
                                name + "_" + std::to_string(i), right,
                                index, ASTNode::Operator::SYMBOLINDEX);
                        right2bin->setVarType(right->getVarType());
                        values.push_back(right2bin);
                    }
                }
                else if (binRight->getOp() == ASTNode::Operator::XOR) {
                    if (binRight->getLeft()->getValueType() == ValueType::VTArrayValue and
                            binRight->getRight()->getValueType() == ValueType::VTArrayValue) {
                        ArrayValue *LarrayV = dynamic_cast<ArrayValue *>(binRight->getLeft().get());
                        ArrayValue *RarrayV = dynamic_cast<ArrayValue *>(binRight->getRight().get());
                        for (int i = 0; i < arraySize; ++i) {
                            InternalBinValuePtr right2bin = std::make_shared<InternalBinValue>(
                                    name + "_" + std::to_string(i), LarrayV->getValueAt(i),
                                    RarrayV->getValueAt(i), ASTNode::Operator::XOR);
                            right2bin->setVarType(LarrayV->getValueAt(i)->getVarType());
                            values.push_back(right2bin);
                        }
                    }
                    else
                        assert(false);
                }
                else if (binRight->getOp() == ASTNode::Operator::FFTIMES) {
                    if (binRight->getLeft()->getValueType() == ValueType::VTBoxValue) {
                        for (int i = 0; i < arraySize; ++i) {
                            ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                            InternalBinValuePtr right2bin = std::make_shared<InternalBinValue>(
                                    name + "_" + std::to_string(i), right,
                                    index, ASTNode::Operator::SYMBOLINDEX);
                            right2bin->setVarType(right->getVarType());
                            values.push_back(right2bin);
                        }
                    }
                    else
                        assert(false);
                }

                for (auto & value : values) {
                    interpreter.addToSequence(value);
                }
                rightArray = new ArrayValue("", values);
            } else
                assert(false);
        }

        assert(rightArray);
        assert(rightArray->getArrayValue().size() == indexes.size());
        for(int i = 0; i < arrayV->getArrayValue().size(); i++) {
            if(i == indexes[0]) {
                for(int j = 0; j < rightArray->getArrayValue().size(); j++) {
                    res.push_back(rightArray->getValueAt(j));
                    i++;
                }
                i = i - 1;
            } else
                res.push_back(arrayV->getArrayValue()[i]);
        }
    }

    std::string returnName = name + "_" + std::to_string(indexes[0]) + "_" + std::to_string(indexes[indexes.size() - 1]);
    ValuePtr valuePtr = std::make_shared<ArrayValue>(returnName, res);
    ValuePtr originArray = interpreter.getFromEnv(name);
    valuePtr->setVarType(originArray->getVarType());
    interpreter.addToEnv(name, valuePtr);
    return array;
}

ValuePtr ASTNode::NVariableDeclaration::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    if(this->type->isArray) {
        int arraySize = 1;
        std::vector<int> arrayBounds;

        bool needToDivide = false;
        for(auto bound : *(this->type->arraySize)) {
            ValuePtr ident = bound->compute(interpreter);
            if(NIdentifierPtr identifier = std::dynamic_pointer_cast<NIdentifier>(bound)) {
                if(identifier->getName() == "n" && !arrayBounds.empty()) {
                    needToDivide = false;
                }
            }
            int valueOfBound = interpreter.value_of(ident);
            arrayBounds.push_back(valueOfBound);
            arraySize *= valueOfBound;
        }

        if(needToDivide) {
            int newArraySize = 1;
            std::vector<int> newArrayBounds;
            newArrayBounds.push_back(arrayBounds[arrayBounds.size() - 1]);
            for(int i = 0; i < arrayBounds.size() - 1; i++) newArraySize *= arrayBounds[i];
            std::vector<ValuePtr> finalValues;
            for(int i = 0; i < newArraySize; i++) {
                interpreter.setArraySize(this->id->getName() + "_" + std::to_string(i), newArrayBounds);
                std::vector<ValuePtr> values;
                for (int j = 0; j < newArrayBounds[0]; j++) {
                    ValuePtr valuePtr = interpreter.getValue(this->type->getName(), this->id->getName() + "_" + std::to_string(i) + "_" + std::to_string(j),
                                                             this->isParameter);
                    values.push_back(valuePtr);
                }
                ValuePtr re = std::make_shared<ArrayValue>(this->id->getName() + "_" + std::to_string(i), values);
                interpreter.addToEnv(this->id->getName() + "_" + std::to_string(i), re);
                finalValues.push_back(re);

                if (this->isParameter) {
                    interpreter.addParameter(re);
                }
                if (this->assignmentExpr) {
                    NAssignment assignment(this->id, this->assignmentExpr);
                    return assignment.compute(interpreter);
                }
            }

            ValuePtr re = std::make_shared<ArrayValue>(this->id->getName(), finalValues);
            interpreter.addToEnv(this->id->getName(), re);

            if (this->isParameter) {
                interpreter.addParameter(re);
            }

        } else {
            interpreter.setArraySize(this->id->getName(), arrayBounds);
            std::vector<ValuePtr> values;
            for (int i = 0; i < arraySize; i++) {
                ValuePtr valuePtr = interpreter.getValue(this->type->getName(),
                                                         this->id->getName() + "_" + std::to_string(i),
                                                         this->isParameter);
                valuePtr->setVarType(Interpreter::VarTypeTrans(this->getMyType()));
                values.push_back(valuePtr);
            }
            ValuePtr re = std::make_shared<ArrayValue>(this->id->getName(), values);
            re->setVarType(Interpreter::VarTypeTrans(this->getMyType()));
            interpreter.addToEnv(this->id->getName(), re);

            if (this->assignmentExpr) {
                NExpressionPtr nExpressionPtr(this->assignmentExpr);
                NArrayIndexPtr nArrayIndex = std::make_shared<NArrayIndex>(this->id);
                NArrayAssignment nArrayAssignment(nArrayIndex, nExpressionPtr);
                nArrayAssignment.compute(interpreter);
            }

            if (this->isParameter) {
                interpreter.addParameter(re);
            }
        }
        return nullptr;
    }
    else {
        std::string name = this->id->getName();
        ValuePtr value = interpreter.getValue(this->type->getName(), name, this->isParameter);
        value->setVarType(interpreter.VarTypeTrans(this->getMyType()));
        interpreter.addToEnv(name, value);

        if(this->isParameter) {
            interpreter.addParameter(value);
        }

        if(this->assignmentExpr) {
            NAssignment assignment(this->id, this->assignmentExpr);
            ValuePtr valuePtr = assignment.compute(interpreter);
            valuePtr->setVarType(value->getVarType());
            interpreter.addToEnv(this->id->getName(), valuePtr);
            return valuePtr;
        }
        return nullptr;
    }
}


ValuePtr ASTNode::NArrayInitialization::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    this->declaration->compute(interpreter);
    NExpressionListPtr nExpressionListPtr = this->expressionList;
    std::vector<ValuePtr> result;
    for(auto & i : *nExpressionListPtr) {
        ValuePtr valuePtr = i->compute(interpreter);
        valuePtr->setVarType(Interpreter::VarTypeTrans(this->declaration->getMyType()));
        result.push_back(valuePtr);
    }
    std::string arrayName = this->declaration->getId()->getName();
    ValuePtr res = std::make_shared<ArrayValue>(arrayName, result);
    res->setVarType(Interpreter::VarTypeTrans(this->declaration->getMyType()));
    interpreter.addToEnv(arrayName, res);
    return nullptr;
}


ValuePtr ASTNode::NSboxInitialization::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::vector<int> sboxEles;
    this->declaration->compute(interpreter);
    NIntegerListPtr integerListPtr = this->nIntegerListPtr;
    std::vector<ValuePtr> result;
    for(auto & i : *integerListPtr) {
        ValuePtr valuePtr = i->compute(interpreter);
        valuePtr->setVarType(Interpreter::VarTypeTrans(this->declaration->getMyType()));
        result.push_back(valuePtr);

        auto* concreteNumValue = dynamic_cast<ConcreteNumValue*>(valuePtr.get());
        sboxEles.push_back(concreteNumValue->getNumer());
    }
    std::string arrayName = this->declaration->getId()->getName();
    int rowSize = integerListPtr->size();
    ValuePtr sbox = std::make_shared<BoxValue>(arrayName, "sbox", rowSize, result);
    sbox->setVarType(Interpreter::VarTypeTrans(this->declaration->getMyType()));
    interpreter.addToEnv(arrayName, sbox);
    allBox["sbox" + this->declaration->getId()->getName()] = sboxEles;
    return nullptr;
}


ValuePtr ASTNode::NPboxInitialization::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::vector<int> pboxEles;
    this->declaration->compute(interpreter);
    NIntegerListPtr integerListPtr = this->nIntegerListPtr;
    std::vector<ValuePtr> result;
    for(auto & i : *integerListPtr) {
        ValuePtr valuePtr = i->compute(interpreter);
        valuePtr->setVarType(Interpreter::VarTypeTrans(this->declaration->getMyType()));
        result.push_back(valuePtr);

        auto* concreteNumValue = dynamic_cast<ConcreteNumValue*>(valuePtr.get());
        pboxEles.push_back(concreteNumValue->getNumer());
    }
    std::string arrayName = this->declaration->getId()->getName();
    int rowSize = integerListPtr->size();
    ValuePtr pbox = std::make_shared<BoxValue>(arrayName, "pbox", rowSize, result);
    pbox->setVarType(Interpreter::VarTypeTrans(this->declaration->getMyType()));
    interpreter.addToEnv(arrayName, pbox);
    allBox["pbox" + this->declaration->getId()->getName()] = pboxEles;
    return nullptr;
}


ValuePtr ASTNode::NPboxmInitialization::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    this->declaration->compute(interpreter);
    NIntegerListListPtr integerListListPtr = this->nIntegerListListPtr;

    if (nIntegerListListPtr->size() != nIntegerListListPtr->at(0)->size()) {
        std::cout << "TYPE ERROR : The Pboxm should be a square matrices" << std::endl;
        abort();
    }

    std::vector<ValuePtr> result;
    for(auto & i : *nIntegerListListPtr) {
        for (auto & j : *i) {
            ValuePtr valuePtr = j->compute(interpreter);
            valuePtr->setVarType(Interpreter::VarTypeTrans(this->declaration->getMyType()));
            result.push_back(valuePtr);
        }
    }
    std::string arrayName = this->declaration->getId()->getName();
    int rowSize = nIntegerListListPtr->at(0)->size();
    ValuePtr pboxm = std::make_shared<BoxValue>(arrayName, "pboxm", rowSize, result);
    pboxm->setVarType(Interpreter::VarTypeTrans(this->declaration->getMyType()));
    interpreter.addToEnv(arrayName, pboxm);
    return nullptr;
}


ValuePtr ASTNode::NFfmInitialization::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr M = interpreter.getFromEnv(this->declaration->getId()->getName());
    this->declaration->compute(interpreter);
    interpreter.addToEnv(this->declaration->getId()->getName(), M);

    if (nIntegerListListPtr->size() != nIntegerListListPtr->at(0)->size()) {
        std::cout << "TYPE ERROR : The Ffm should be a square matrices" << std::endl;
        abort();
    }

    NIntegerListListPtr integerListListPtr = this->nIntegerListListPtr;
    std::vector<ValuePtr> result;
    for(auto & i : *nIntegerListListPtr) {
        for (auto & j : *i) {
            ValuePtr valuePtr = j->compute(interpreter);
            result.push_back(valuePtr);
        }
    }
    std::string arrayName = this->declaration->getId()->getName();
    int rowSize = nIntegerListListPtr->size();
    ValuePtr ffm = std::make_shared<BoxValue>(arrayName, "ffm", rowSize, result);
    ffm->setVarType(Interpreter::VarTypeTrans(this->declaration->getMyType()));
    interpreter.addToEnv(arrayName + "_ffm", ffm);
    return nullptr;
}


ValuePtr ASTNode::NBlock::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::vector<ValuePtr> block;
    ValuePtr last = nullptr;
    for(const auto& stmt : *(this->getStmtList())) {
        last = stmt->compute(interpreter);
        if(last && last->getValueType() == ValueType::VTProcValue)
            interpreter.addProc(last);
        block.push_back(last);
    }
    return last;
}


ValuePtr ASTNode::NFunctionDeclaration::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::string functionName = this->id->getName();
    std::map<std::string, ValuePtr> globalEnv = interpreter.getEnv();
    interpreter.pushBlock();
    for (const auto& env : globalEnv) {
        interpreter.addToEnv(env.first, env.second);
    }
    for(auto pa : *(this->var_list)) {
        pa->compute(interpreter);
    }

    std::vector<ValuePtr> parameters = interpreter.getParameter();
    ValuePtr body = this->block->compute(interpreter);
    interpreter.addToSequence(body);
    ValuePtr returns = interpreter.getCurrentReturnValue();
    ProcedurePtr proc = std::make_shared<Procedure>(functionName, parameters, interpreter.getEnv(),
                                                    interpreter.getSequence(), returns);
    return std::make_shared<ProcValue>(functionName, proc);
}


ValuePtr ASTNode::NRoundFunctionDeclaration::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::string functionName = this->id->getName();
    std::map<std::string, ValuePtr> globalEnv = interpreter.getEnv();
    interpreter.pushBlock();
    for (const auto& env : globalEnv) {
        interpreter.addToEnv(env.first, env.second);
    }
    this->round->compute(interpreter);
    this->sk->compute(interpreter);
    this->p->compute(interpreter);

    std::vector<ValuePtr> parameters = interpreter.getParameter();
    ValuePtr body = this->block->compute(interpreter);
    interpreter.addToSequence(body);
    ValuePtr returns = interpreter.getCurrentReturnValue();

    ProcedurePtr proc = std::make_shared<Procedure>(functionName, parameters, interpreter.getEnv(),
                                                    interpreter.getSequence(), returns);
    proc->setIsRndf();
    ValuePtr procRtn = std::make_shared<ProcValue>(functionName, proc);
    procRtn->setVarType(Interpreter::VarTypeTrans(this->type->getName()));
    return procRtn;
}


ValuePtr ASTNode::NKeyScheduleDeclaration::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::string functionName = this->id->getName();
    std::map<std::string, ValuePtr> globalEnv = interpreter.getEnv();
    interpreter.pushBlock();
    for (const auto& env : globalEnv) {
        interpreter.addToEnv(env.first, env.second);
    }
    this->key->compute(interpreter);

    std::vector<ValuePtr> parameters = interpreter.getParameter();
    ValuePtr body = this->block->compute(interpreter);
    interpreter.addToSequence(body);
    ValuePtr returns = interpreter.getCurrentReturnValue();

    ProcedurePtr proc = std::make_shared<Procedure>(functionName, parameters, interpreter.getEnv(),
                                                    interpreter.getSequence(), returns);
    proc->setIsKschd();
    ValuePtr procRtn = std::make_shared<ProcValue>(functionName, proc);
    procRtn->setVarType(Interpreter::VarTypeTrans(this->type->getName()));
    return procRtn;
}


ValuePtr ASTNode::NSboxFunctionDeclaration::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::string functionName = this->id->getName();
    std::map<std::string, ValuePtr> globalEnv = interpreter.getEnv();
    interpreter.pushBlock();
    for (const auto& env : globalEnv) {
        interpreter.addToEnv(env.first, env.second);
    }

    this->input->isParameter = true;
    this->input->compute(interpreter);

    std::vector<ValuePtr> parameters = interpreter.getParameter();
    ValuePtr body = this->block->compute(interpreter);
    interpreter.addToSequence(body);
    ValuePtr returns = interpreter.getCurrentReturnValue();

    ProcedurePtr proc = std::make_shared<Procedure>(functionName, parameters, interpreter.getEnv(),
                                                    interpreter.getSequence(), returns);
    proc->setIsSboxf();

    ValuePtr procRtn = std::make_shared<ProcValue>(functionName, proc);
    procRtn->setVarType(Interpreter::VarTypeTrans(this->type->getName()));
    return procRtn;
}


ValuePtr ASTNode::NCipherFunctionDeclaration::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::string functionName = this->id->getName();
    std::map<std::string, ValuePtr> globalEnv = interpreter.getEnv();
    interpreter.pushBlock();
    for (const auto& env : globalEnv) {
        interpreter.addToEnv(env.first, env.second);
    }

    this->k->compute(interpreter);
    this->p->compute(interpreter);

    std::vector<ValuePtr> parameters = interpreter.getParameter();
    ValuePtr body = this->block->compute(interpreter);
    interpreter.addToSequence(body);
    ValuePtr returns = interpreter.getCurrentReturnValue();
    ProcedurePtr proc = std::make_shared<Procedure>(functionName, parameters, interpreter.getEnv(),
                                                    interpreter.getSequence(), returns);
    proc->setIsFn();
    ValuePtr procRtn = std::make_shared<ProcValue>(functionName, proc);
    procRtn->setVarType(Interpreter::VarTypeTrans(this->type->getName()));
    return procRtn;
}


ValuePtr ASTNode::NExpressionStatement::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    return this->expr->compute(interpreter);
}


ValuePtr ASTNode::NReturnStatement::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr returns = this->expr->compute(interpreter);
    interpreter.setCurrentReturnValue(returns);
    return nullptr;
}


ValuePtr ASTNode::NForStatement::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr intiValue = this->from->compute(interpreter);
    interpreter.addToEnv(this->ident->getName(), intiValue);
    int lowerBound = this->from->getValue();
    int upperBound = this->to->getValue();
    bool cont = lowerBound <= upperBound;
    while(cont) {
        this->block->compute(interpreter);
        lowerBound++;
        interpreter.addToEnv(this->ident->getName(), std::make_shared<ConcreteNumValue>("", lowerBound));
        cont = lowerBound <= upperBound;
    }
    return nullptr;
}


ValuePtr ASTNode::NCipherNameDeclaration::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    return nullptr;
}
