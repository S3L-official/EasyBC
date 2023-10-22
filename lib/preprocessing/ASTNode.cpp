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


/*
 * ArrayIndex的compute逻辑：
 *  1）当dimons有一个元素的时候，即一维访问，直接进行compute即可;
 *  2）当dimons有两个元素的时候，先访问第一个元素所表示的结果，再访问第二个元素的结果.
 *  注：尽管我们的语法支持二维数组，但是二维数组只能是pboxm或者ffm，而这两个是不会直接进行数组访问的。
 *     能直接进行数组访问的只有一维数组，所以我们分为一维和二维的两种元素访问方式
 * */
ValuePtr ASTNode::NArrayIndex::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    std::string name = this->arrayName->getName();

    // 当dimons由两个元素时，先compute第一个元素，再处理第二个元素
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

    /*
     * 当arrayV == nullptr 时，说明是对uints的整数取某位bit。那么就要将其转化为对应该uints中s个数的size的ArrayValue类型
     * 根据 operation 分为三种情况：
     *  1) InternalBinValue : boxop, ffm乘法的symbolIndex (其他的二元操作符可以直接在 evaluate 时，分解成为 bit 对 bit 的操作）
     *  2) InternalUnValue : touint
     *  3) ConcreteNumValue
     *  4) ParameterValue
     * */
    if (arrayV == nullptr) {
        // 不用对 value 的type进行区分，当array为 nullptr 时，意味着时对uints中的某个bit取位，那么直接用symbolindex来标记即可
        int size;
        std::vector<ValuePtr> res;
        if (InternalBinValue* array2Bin = dynamic_cast<InternalBinValue*>(array.get())) {
            if (array2Bin->getOp() == ASTNode::Operator::BOXOP) {
                // boxop 的 rightHand 就直接是数组
                if (ArrayValue *tArray = dynamic_cast<ArrayValue*>(array2Bin->getRight().get())) {
                    // 这里处理是按照boxop的输入输出都是一样的size，因此由输入input的size来决定output的size
                    size = tArray->getArrayValue().size();
                }
                // boxop 的 rightHand 是 touint
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
            // 当value本身是symbolIndex，并且类型不是uint1时，可以对其中某一个bit取位
            else if (array2Bin->getOp() == ASTNode::Operator::SYMBOLINDEX and array2Bin->getVarType() != ASTNode::uint1) {
                // symbolIndex的整体是数组
                size = this->transType2Int(array->getVarType());
                for (int i = 0; i < size; ++i) {
                    ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                    InternalBinValuePtr tArray2bin = std::make_shared<InternalBinValue>(
                            array->getName() + "_" + std::to_string(i), array,
                            index, ASTNode::Operator::SYMBOLINDEX);
                    // 当对一个uint4拆分成一个数组时，每个元素都会变成uint1类型
                    tArray2bin->setVarType(uint1);
                    res.push_back(tArray2bin);
                }
            }
            else
                assert(false);
        }
        else if (InternalUnValue *array2Un = dynamic_cast<InternalUnValue*>(array.get())) {
            if (array2Un->getOp() == ASTNode::Operator::TOUINT) {
                // touint 的 rand 就直接是数组
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
            // array2Num 的 valType 可以确定构建数组的 size
            size = interpreter.getVarTypeSize(array2Num->getVarType());
            for (int i = 0; i < size; ++i) {
                ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                InternalBinValuePtr temp2bin = std::make_shared<InternalBinValue>(
                        name + "_" + std::to_string(i), array,
                        index, ASTNode::Operator::SYMBOLINDEX);
                // 当array为concreteNumValue时, 被访问的每个元素的类型被设置为uint1
                temp2bin->setVarType(ASTNode::uint1);
                res.push_back(temp2bin);
            }
        }
        else if (ParameterValue *array2Param = dynamic_cast<ParameterValue*>(array.get())) {
            // 首先从env中根据array的名字取出其对应的value对象，以提取它的valType，从而确定构建数组的size
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
        // 当对元素为uints的数组的某个元组的某一位进行取值时，可能存在array为ArrayValueIndex的情况
        // 如 GIFT_64.txt 中，对 constants 数组的某个元素的某一位进行取值
        else if (ArrayValueIndex *array2arrIdx = dynamic_cast<ArrayValueIndex*>(array.get())) {
            // 首先从env中根据array的名字取出其对应的value对象，以提取它的valType，从而确定构建数组的size
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
        // 当转化为数组以后，还需要更新数组至env，因为后续计算index会在env中查询数组的size
        interpreter.addToEnv(name, tArrayOut);
        std::vector<int> resArraySize;
        resArraySize.push_back(res.size());
        interpreter.setArraySize(name, resArraySize);
    }

    /*
     * 在这里我们要计算index，在这里我们分为两种情况：
     * 1. index是包含输入参数的，此时index具体的值不能被计算出来，只能用symbol方式表示出来；
     * 2. index没有包含输入参数，index具体值可以被计算出，此时计算index的具体值又分为三种情况：
     *      1）当数组是一维的时候，index的值直接计算即可；
     *      2）当数组是二维的时候，index的值需要由两个值来决定，具体的，
     *         数组a中，a[i1][i2]的具体index值需要求解的是 i1 * rowSize + i2
     *      3）当数组是一维，取到数组某一个元素时，又对该元素的某位bit取值，如 rc[r-1][i]
     * */
    std::vector<ValuePtr> indexesSymbol = interpreter.computeIndexWithSymbol(name, this->dimons);

    bool indexHasSymbol = false;
    for(auto ele : indexesSymbol) {
        if(!ValueCommon::isNoParameter(ele)) {
            indexHasSymbol = true;
            interpreter.getEnv()[name]->set_symbol_value_f();
        }
    }

    /*
     * 如果没有symbol，index是具体的确定的，分为两种情况：
     *  1）当arrayV->getValueAt(indexes[0])是一个concreteNumValue时，会直接返回一个具体的数值，
     *     此时的返回值除了具体值以外是没有其他任何信息的，但是如果我们需要进一步对该具体的数值取某一位时，就不能继续往下进行了，
     *     因为我们还需要知道这个具体的数值在内存中存储时是按照多少位来存储的。
     *     所以，这里我们首先判断arrayV->getValueAt(indexes[0])是否是concreteNumValue，如果是，
     *     那我们构建一个Value对象，将该具体的存储进去，并且还需要根据数组arrayV的varType，来确定arrayV->getValueAt(indexes[0])的VarType
     *  2）当arrayV->getValueAt(indexes[0])不是具体的数值，直接返回该位的元素，
     *     此时返回的的应该是一个SYMBOLINDEX的形式或者和parameter相关的index形式
     * */
    if(!indexHasSymbol and !interpreter.getEnv()[name]->get_symbol_value_f()) {
        std::vector<int> indexes;
        for (auto ele : indexesSymbol) {
            // 如果ele本身是不可以evaluate的，那么就需要直接使用其表达式形式
            int res = ele->value_of(interpreter.getEnv());
            indexes.push_back(res);
        }

        if(indexes.size() == 1) {
            if (arrayV->getValueAt(indexes[0])->getValueType() == ValueType::VTConcreteNumValue) {
                ValuePtr con = arrayV->getValueAt(indexes[0]);
                // 这里我们还将该具体值的value对象命名为 “所取自数组名 + '_' +索引值”
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
        // 如果有symbol，我们将其命名位name+SYMBOLINDEX，从而不会覆盖env中原有的name标记的value
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

    // 函数的形参替换为调用的实参，是数组就直接放数组
    std::vector<ValuePtr> realParameters;
    for (auto arg : *(this->arguments)) {
        ValuePtr argPtr = arg->compute(interpreter);
        realParameters.push_back(argPtr);
    }

    // find function body
    ValuePtr proc = interpreter.getProc(functionName);
    ProcedurePtr procedurePtr = nullptr;
    if(auto* procValue = dynamic_cast<ProcValue*>(proc.get())) {
        procedurePtr = procValue->getProcedurePtr();
    } else {
        assert(false);
    }

    // 对于一个函数调用，其返回值如果返回是一个具体的值，就是那个值，如果是一个vector，就应该是一个vector
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
        // 如果返回值是一个数组
        std::vector<ValuePtr> returnArray;
        for (int i = 0; i < arrayValue->getArrayValue().size(); i++) {
            ValuePtr value = std::make_shared<ProcCallValueIndex>(procCallValuePtr->getName() + "_" + std::to_string(i),
                                                                  procCallValuePtr, i);
            value->setVarType(proc->getVarType());
            // 函数调用生成value时直接被添加至sequence
            // interpreter.addToSequence(value);
            returnArray.push_back(value);
        }
        res = std::make_shared<ArrayValue>(procCallValuePtr->getName() + "_ret", returnArray);
        res->setVarType(proc->getVarType());
        // ProcCallValue的parent是各个返回值
        for (auto ele : returnArray) {
            procCallValuePtr->addParents(ele);
        }
    } else {
        // 如果返回值是单个值
        res = std::make_shared<ProcCallValueIndex>(procCallValuePtr->getName() + "_" + std::to_string(0), procCallValuePtr, 0);
        res->setVarType(proc->getVarType());
        // 函数调用生成value时直接被添加至sequence
        // interpreter.addToSequence(res);
        procCallValuePtr->addParents(res);
    }

    // 实际参数的parent是ProcCallValue
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
    // 这里存在两种情况，1.函数调用返回数组；2.函数调用返回单个元素
    // 刚开始重构的处理方式是，若返回的是单个元素的value，就将该value放入sequence
    // 若返回的是数组的value，就将数组中的每个元素的value依次放入sequence
    // 但是这样会引入一个问题，后续在进行三地址转换以及分析时，没办法直接得到返回数组的size，
    // 这就导致进而分析时需要确定的返回值对应的xCounter及其他建模信息没办法确定
    // 所以现在，我们不分别对待，不管是不是数组，都直接将元素整体放入sequence
    interpreter.addToSequence(res);
    return res;
}


ValuePtr ASTNode::NBinaryOperator::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr L = this->lhs->compute(interpreter);
    assert(L);
    ValuePtr R = this->rhs->compute(interpreter);
    assert(R);

    // TYPE SYSTEM
    // 这里的type system实际上有个问题，就是当 binary operator 连接了多个操作对象时，如何对每个操作对象的类型都进行判断呢？
    /*if (L->getVarType() != R->getVarType()) {
        ConcreteNumValue* concreteNumValue = dynamic_cast<ConcreteNumValue*>(R.get());
        if (concreteNumValue == nullptr) {
            std::cout << "TYPE ERROR : Inconsistent object types for binary operations" << std::endl;
            abort();
        }
    }
*/
    ValuePtr res = std::make_shared<InternalBinValue>("", L, R, this->op);
    if (L->getVarType() != ASTNode::null)
        res->setVarType(L->getVarType());
    else if (R->getVarType() != ASTNode::null)
        res->setVarType(R->getVarType());

    // the concrete value should be evaluated : operators : + - * / %
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
        } else {
            assert(false);
        }
    }

    // circulating shift operators should be evaluated
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

    // pboxm
    BoxValue* boxValue = dynamic_cast<BoxValue*>(L.get());
    if (boxValue != nullptr) {
        if (boxValue->getBoxType() == "pboxm") {
            // 这里需要进行矩阵乘向量的计算，需要先获取env中的ffm，从而知道对应的有限域乘法计算规则，
            // 然后按照计算规则，依次将矩阵的元素和对应的vector的每个元素相乘
            std::string mName = boxValue->getName();
            ValuePtr mFfm = interpreter.getEnv()[mName + "_ffm"];
            BoxValue* boxFfm = dynamic_cast<BoxValue*>(mFfm.get());
            ArrayValue* rVector = dynamic_cast<ArrayValue*>(R.get());
            // 根据ffm中的一个元素的类型，或者对应优先于计算规则来判断是哪种计算
            if (boxValue->getValueAt(0)->getVarType() == ASTNode::uint1) {
                // 现在觉得好像没有必要在这里进行规则判断，而是应该将对应的pboxm和对应的ffm一起保存起来，这样在进行模型转换时直接统一进行计算即可。
            }

            // 直接存储对应的pboxm和ffm
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
            // 这里ffm保存的名字需要和pboxm中的一致，方便后续查找
            Ffm["pboxm" + mName] = ffmValue;
        }
        else {
            assert(false);
        }
    }

    L->addParents(res);
    R->addParents(res);

    // 如果 L 和 R 都不是数组类型的，那么就直接将这两个的操作加入到sequence
    // 否则，应该将数组对应每个元素的操作都加入到sequence
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
    // TYPING SYSTEM : 所有的整数需要大于等于0
    // 后面考虑使用yylineno打印出错的行数
    if (this->value < 0) {
        std::cout << "TYPE ERROR : Integer value less than 0" << std::endl;
        abort();
    }
    ValuePtr valuePtr = std::make_shared<ConcreteNumValue>("", this->value);
    return std::make_shared<ConcreteNumValue>("", this->value);
}


ValuePtr ASTNode::NViewArray::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    // 从当前的Env中找到View操作的数组对象
    ValuePtr array = interpreter.getEnv()[this->arrayName->getName()];
    auto *arrayV = dynamic_cast<ArrayValue *>(array.get());

    // type system
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
    //返回一个VTInternalBinValue类型，由操作符TOUINT链接标识符和数组
    std::vector<ValuePtr> expListTrans;
    // 当NToUint对象的实例由expressionListPtr初始化
    int size = 0;
    if (this->expressionListPtr != nullptr) {
        for (auto exp : *expressionListPtr) {
            ValuePtr expValue = exp->compute(interpreter);

            // touint的连接对象可以是常数
            ConcreteNumValue* expConcrete = dynamic_cast<ConcreteNumValue*>(expValue.get());
            // type system
            if (expValue->getVarType() != ASTNode::Type::uint1 and expConcrete == nullptr) {
                std::cout << "TYPE ERROR : The type operated object of ToUint-Operation should be 'uint1'" << std::endl;
                abort();
            }

            name = expValue->getName();
            expListTrans.push_back(expValue);
            size++;
        }
    }
    // 当NToUint对象的实例由arrayName初始化, 从env中获取对应数组，并将对应元素用于构造返回的value
    else if (this->arrayName != nullptr) {
        ValuePtr array = interpreter.getEnv()[this->arrayName->getName()];
        auto *arrayV = dynamic_cast<ArrayValue *>(array.get());
        assert(arrayV);
        for (auto ele : arrayV->getArrayValue()) {

            // type system
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
    // touint需要添加至sequence
    // interpreter.addToSequence(rtn);
    return rtn;
}


ValuePtr ASTNode::NBoxOperation::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    ValuePtr L = boxname->compute(interpreter);
    ValuePtr R = expressionPtr->compute(interpreter);

    // type system
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

    // type system
    if (interpreter.getFromEnv(this->LHS->getName())->getVarType() != expValue->getVarType()) {
        ArrayValue* arrayValueL = dynamic_cast<ArrayValue*>(interpreter.getFromEnv(this->LHS->getName()).get());
        ArrayValue* arrayValueR = dynamic_cast<ArrayValue*>(expValue.get());
        ConcreteNumValue* expConcrete = dynamic_cast<ConcreteNumValue*>(expValue.get());
        // 赋值语句时，如果左右两个都为数组，或者右边为具体值，则类型正确，否则类型错误
        if ((arrayValueL == nullptr and arrayValueR == nullptr) and (expConcrete == nullptr)) {
            std::cout << "LHS type : " << interpreter.getFromEnv(this->LHS->getName())->getVarType() << std::endl;
            std::cout << "exp type : " << expValue->getVarType() << std::endl;
            std::cout << "TYPE ERROR : The assignment object is not of the same type as the assigned object"
                      << std::endl;
            abort();
        }
        // 左右两个数组的size应该相同
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
        // ConcreteNumValue不需要加入到sequence中去
    } else if(expValue->getValueType() == ValueType::VTInternalBinValue) {
        /*
         * 表达式的操作对象不是array时，在RHS->compute时就已经被添加紧sequence
         * 当操作对象是array时，要在这里按照array的size展开，并依次放入sequence
         * */
        InternalBinValue* binExpValue = dynamic_cast<InternalBinValue*>(expValue.get());
        // 首先判断被操作对象是否是数组
        if (binExpValue->getRight()->getValueType() == ValueType::VTArrayValue) {
            // 如果被操作对象是数组，那么也新建一个数组对象用以存放被操作的结果，后续添加到env中
            std::vector<ValuePtr> arrayEles;
            auto op = binExpValue->getOp();
            if (op == ASTNode::Operator::BOXOP) {
                // 当operator为BOXOP时，输出array的size和输入Right的size相同
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
            // 如果op是bin operators，输出value中的每个元素，是被操作对象对应的index进行bin operators的结果
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
        // 当被操作对象不是数组，那么输出对象也不应该是数组
        else {
            expValue->setName(this->LHS->getName());
            // 若binExpValue的op是boxop，比如touint为操作对象的uints，不需要被添加至sequences
            // boxop 已经正在boxop时被添加至sequence

            // added in 2023.7.31
            // 非数组类型的bin操作已经在nbinaryoperation class的compute中已经被处理，所以这里我们就不再添加至sequence中
            /*if (binExpValue->getOp() != BOXOP)
                interpreter.addToSequence(expValue);*/
            interpreter.addToEnv(this->LHS->getName(), expValue);
            return expValue;
        }
    } else if(expValue->getValueType() == ValueType::VTInternalUnValue) {
        // 当expValue是VTInternalUnValue
        InternalUnValue* unExpValue = dynamic_cast<InternalUnValue*>(expValue.get());
        // 首先判断被操作对象是否是数组
        if (unExpValue->getRand()->getValueType() == ValueType::VTArrayValue) {
            // 根据被操作对象和操作符，输出对象来决定输出对象的类型，然后加到env中
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
        // interpreter.addToSequence(expValue);
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

    //对数组元素的覆盖也算吧
    std::vector<ValuePtr> res;
    if(indexes.size() == 1) {
        // 只有一个元素
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
        /*
         * 这里根据right具体类型分为两种情况：
         *   1）right是数组，此时可以直接将对应的元素值依次赋值给array对应的index。
         *     并且此时不需要将对应的元素依次放到sequence中，因为只是同一个value换了不同的标识符，并没有涉及具体的运算关系
         *   2）right是数组的操作结果，此时需要先将操作结果转换为数组，再将对应的元素值依次赋值给array对应的index。
         *     并且此时也需要将操作结果的对应元素依次放到sequence中，因为right的value是具体的操作结果，并且在具体操作的compute函数
         *     中没有被放到sequence中。
         * */
        int arraySize = arrayV->getArrayValue().size();
        ArrayValue* rightArray = dynamic_cast<ArrayValue*>(right.get());
        // 2.right是数组的操作结果，
        if (!rightArray) {
            std::vector<ValuePtr> values;
            if (right->getValueType() == ValueType::VTInternalBinValue) {
                // 如果right类型是VTInternalBinValue，则要先将其转化成该类型，然后根据具体的operator进行不同的处理
                InternalBinValue* binRight = dynamic_cast<InternalBinValue*>(right.get());
                // 如果op是BOXOP，则输出value的每个元素仍然是VTInternalBinValue，并且每个元素都需要增加一个索引；
                // 整个数组的所有元素集合也是一个VTInternalBinValue类型
                if (binRight->getOp() == ASTNode::Operator::BOXOP) {
                    for (int i = 0; i < arraySize; ++i) {
                        ConcreteNumValuePtr index = std::make_shared<ConcreteNumValue>("index", i);
                        // 因为后面就会把各个value元素赋值给对应的left-hand，所以这里每个value元素没有名字
                        InternalBinValuePtr right2bin = std::make_shared<InternalBinValue>(
                                name + "_" + std::to_string(i), right,
                                index, ASTNode::Operator::SYMBOLINDEX);
                        right2bin->setVarType(right->getVarType());
                        values.push_back(right2bin);
                    }
                }
                // 这里我们只支持由一个xor连接的两个数组，不支持多个数组连续使用多个xor的操作连接
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
                // 如果是 pboxm * uints[n], 则返回uints[n]类型
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

                // 每个生成的valuePtr都需要放在sequence
                for (auto & value : values) {
                    interpreter.addToSequence(value);
                }
                rightArray = new ArrayValue("", values);
            } else
                assert(false);
        }
        /*else {
            std::vector<ValuePtr> tArray = rightArray->getArrayValue();
            for (int i = 0; i < tArray.size(); ++i) {
                auto ele = tArray[i];
                ele->setName(name + "_" + std::to_string(i));
                interpreter.addToSequence(ele);
            }
        }*/

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

    // 这里好像只要是对数组中元素的赋值，都是返回的是
    std::string returnName = name + "_" + std::to_string(indexes[0]) + "_" + std::to_string(indexes[indexes.size() - 1]);
    ValuePtr valuePtr = std::make_shared<ArrayValue>(returnName, res);
    ValuePtr originArray = interpreter.getFromEnv(name);
    valuePtr->setVarType(originArray->getVarType());
    interpreter.addToEnv(name, valuePtr);
    return array;
}


/*
 * NVariableDeclaration 的 compute 中，如果直接跟着一个赋值语句，
 * 那么就用 NAssignment 或者 NArrayAssignment 进行赋值处理，因此
 * 该函数中不需要添加至sequence中的处理
 * */
ValuePtr ASTNode::NVariableDeclaration::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    // type is an array
    if(this->type->isArray) {
        int arraySize = 1;
        std::vector<int> arrayBounds;

        bool needToDivide = false;
        for(auto bound : *(this->type->arraySize)) {
            ValuePtr ident = bound->compute(interpreter);
            if(NIdentifierPtr identifier = std::dynamic_pointer_cast<NIdentifier>(bound)) {
                // 如果此时数组有bound是n，并且此时arrayBounds不是空，那么证明此时这不是一个一维数组，需要将其拆开
                if(identifier->getName() == "n" && !arrayBounds.empty()) {
                    needToDivide = false;
                }
            }
            int valueOfBound = interpreter.value_of(ident);
            arrayBounds.push_back(valueOfBound);
            arraySize *= valueOfBound;
        }

        if(needToDivide) {
            // 如果需要拆开，那么需要新建立除了最后一个
            int newArraySize = 1;
            std::vector<int> newArrayBounds;
            newArrayBounds.push_back(arrayBounds[arrayBounds.size() - 1]);
            for(int i = 0; i < arrayBounds.size() - 1; i++) newArraySize *= arrayBounds[i];
            // 现在需要建立newArraySize个长度为n的参数
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
                // 在环境中，数组中的单个元素没有拥有姓名
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

            // 当新声明的数据类型为数组，且需要赋值，那么就直接用 NArrayAssignment 处理
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
        // 对于声明的非数组类型的value，直接根据其类型等信息构造一个新的value
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


/*
 * 1. 先计算声明，在环境中开辟一席之地
 * 2. 把值读出来，每个值计算value，并放到vector中
 * 3. 重新放到env中
 * */
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
    // 存储sbox, 存储每个box时，需要将该box的类型也存储，这样可以在后续处理时选择合适的对应方式
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
    // 存储pbox
    allBox["pbox" + this->declaration->getId()->getName()] = pboxEles;
    return nullptr;
}


ValuePtr ASTNode::NPboxmInitialization::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    this->declaration->compute(interpreter);
    NIntegerListListPtr integerListListPtr = this->nIntegerListListPtr;

    // type system
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
    // 二维的 pboxm 的 rowSize 定义为每一行的 size，这里取 nIntegerListListPtr 的第一个元素的 size
    int rowSize = nIntegerListListPtr->at(0)->size();
    ValuePtr pboxm = std::make_shared<BoxValue>(arrayName, "pboxm", rowSize, result);
    pboxm->setVarType(Interpreter::VarTypeTrans(this->declaration->getMyType()));
    interpreter.addToEnv(arrayName, pboxm);
    return nullptr;
}


ValuePtr ASTNode::NFfmInitialization::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    // 在ffm的declaration进行compute时，先将对应的pboxm取出来。因为compute会计算出相同ident的M，覆盖pboxm
    ValuePtr M = interpreter.getFromEnv(this->declaration->getId()->getName());
    this->declaration->compute(interpreter);
    // 在compute结束之后，重新添加之前的pboxm至env
    interpreter.addToEnv(this->declaration->getId()->getName(), M);

    // type system
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
    // 对于每一个新声明的函数，都需要向其添加所有的全局变量作为该函数env的一部分。
    std::map<std::string, ValuePtr> globalEnv = interpreter.getEnv();
    interpreter.pushBlock();
    for (const auto& env : globalEnv) {
        interpreter.addToEnv(env.first, env.second);
    }
    // arguments compute
    for(auto pa : *(this->var_list)) {
        pa->compute(interpreter);
    }

    std::vector<ValuePtr> parameters = interpreter.getParameter();
    // body compute
    ValuePtr body = this->block->compute(interpreter);
    interpreter.addToSequence(body);
    ValuePtr returns = interpreter.getCurrentReturnValue();
    // construct ProcValue for function
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
    // process arguments
    this->round->compute(interpreter);
    this->sk->compute(interpreter);
    this->p->compute(interpreter);

    std::vector<ValuePtr> parameters = interpreter.getParameter();
    // body compute
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
    // process arguments
    this->key->compute(interpreter);

    std::vector<ValuePtr> parameters = interpreter.getParameter();
    // body compute
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

    // process arguments
    this->input->isParameter = true;
    this->input->compute(interpreter);

    std::vector<ValuePtr> parameters = interpreter.getParameter();
    // body compute
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
    // 将初始时i以及目前i的值添加至env，以便在block进行处理时，可以根据i的值来计算index的值。
    interpreter.addToEnv(this->ident->getName(), intiValue);
    int lowerBound = this->from->getValue();
    int upperBound = this->to->getValue();
    bool cont = lowerBound <= upperBound;
    while(cont) {
        this->block->compute(interpreter);
        lowerBound++;
        // 将i以及目前i的值添加至env，以便在block进行处理时，可以根据i的值来计算index的值。
        interpreter.addToEnv(this->ident->getName(), std::make_shared<ConcreteNumValue>("", lowerBound));
        cont = lowerBound <= upperBound;
    }
    return nullptr;
}


ValuePtr ASTNode::NCipherNameDeclaration::compute(Interpreter &interpreter) {
    ASTNodeCounter++;
    return nullptr;
}
