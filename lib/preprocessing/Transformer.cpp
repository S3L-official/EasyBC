//
// Created by Septi on 6/27/2022.
//

#include "Transformer.h"


ProcedureHPtr Transformer::transform(ProcValuePtr procValuePtr) {
    vector<vector<ThreeAddressNodePtr>> parameters;
    vector<ThreeAddressNodePtr> block;
    vector<ThreeAddressNodePtr> returns;

    map<ValuePtr, ThreeAddressNodePtr> saved;
    map<string, int> nameCount;

    // 进行参数的转换
    for(auto ele : procValuePtr->getParameters()) {
        if(ArrayValuePtr arrayValuePtr = dynamic_pointer_cast<ArrayValue>(ele)) {
            vector<ThreeAddressNodePtr> paras;
            for(auto ele1 : arrayValuePtr->getArrayValue()) {
                ThreeAddressNodePtr para(new ThreeAddressNode(ele1->getName(), nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::PARAMETER));
                paras.push_back(para);
                saved[ele1] = para;
            }
            parameters.push_back(paras);
        }
        else {
            vector<ThreeAddressNodePtr> paras;
            ThreeAddressNodePtr para(new ThreeAddressNode(ele->getName(), nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::PARAMETER));
            paras.push_back(para);
            saved[ele] = para;
            parameters.push_back(paras);
        }
    }

    // 进行block的转换
    // 根据sequence中不同的value类型，而选择对应的transform函数
    for (int k = 0; k < procValuePtr->getProcedurePtr()->getBlock().size(); ++k) {
        ValuePtr ele = procValuePtr->getProcedurePtr()->getBlock().at(k);
        if (InternalBinValuePtr internalBinValuePtr = dynamic_pointer_cast<InternalBinValue>(ele)) {
            block.push_back(transformInternalBin(internalBinValuePtr, saved, nameCount));
        }
        else if (InternalUnValuePtr internalUnValue = dynamic_pointer_cast<InternalUnValue>(ele)) {
            block.push_back(transformInternalUn(internalUnValue, saved, nameCount));
        }
        else if (ArrayValuePtr arrayValuePtr = dynamic_pointer_cast<ArrayValue>(ele)) {
            vector<ThreeAddressNodePtr> res = transformArrayValue(arrayValuePtr, saved, nameCount);
            block.insert(block.end(), res.begin(), res.end());
        }
        else if (shared_ptr<ArrayValueIndex> arrayValueIndex = dynamic_pointer_cast<ArrayValueIndex>(ele)) {
            block.push_back(transformArrayValueIndex(*arrayValueIndex, saved, nameCount));
        }
        else if (ProcCallValueIndexPtr procCallValueIndexPtr = dynamic_pointer_cast<ProcCallValueIndex>(ele)) {
            ProcCallValuePtr procCallValuePtr = dynamic_pointer_cast<ProcCallValue>(procCallValueIndexPtr->getProcCallValuePtr());
            // 这里是处理实参，所以实参的类型应该和对应的 ASTNode 一样，即 uint 或者 uints
            for(int i = 0; i < procCallValuePtr->getArguments().size(); i++) {
                auto ele = procCallValuePtr->getArguments()[i];
                if(ArrayValuePtr eleArrayValuePtr = dynamic_pointer_cast<ArrayValue>(ele)) {
                    for(auto item : eleArrayValuePtr->getArrayValue()) {
                        // 如果被调用的对象是轮函数，则需要对每个参数的名字标记处理
                        // sbox function 和 kschd function 的接收参数都只有数组，因此只在数组这种情况下进行区分
                        std::string name = "push";
                        if (procCallValuePtr->getProcedurePtr()->getIsRndf()) {
                            if (i == 1)
                                name = "key_push";
                            else if (i == 2)
                                name = "plaintext_push";
                        } else if (procCallValuePtr->getProcedurePtr()->getIsSboxf()) {
                            name = "sbox_push";
                        } else if (procCallValuePtr->getProcedurePtr()->getIsKschd()) {
                            name = "kschd_push";
                        }
                        if(ConcreteNumValuePtr concreteNumValue = dynamic_pointer_cast<ConcreteNumValue>(item)) {
                            auto tempRes = make_shared<ThreeAddressNode>(to_string(concreteNumValue->getNumer()),
                                                                         nullptr, nullptr,
                                                                         ASTNode::Operator::NULLOP, getNodeType(concreteNumValue->getVarType()));
                            saved[item] = tempRes;
                            ThreeAddressNodePtr threeAddressNodePtr(
                                    new ThreeAddressNode(name, saved[item], nullptr, ASTNode::Operator::PUSH,
                                                         getNodeType(concreteNumValue->getVarType())));
                            block.push_back(threeAddressNodePtr);
                        }
                        else if(ConstantValuePtr constantValue = dynamic_pointer_cast<ConstantValue>(item)) {
                            auto tempRes = make_shared<ThreeAddressNode>(constantValue->getName(),
                                                                         nullptr, nullptr,
                                                                         ASTNode::Operator::NULLOP, getNodeType(constantValue->getVarType()));
                            saved[item] = tempRes;
                            ThreeAddressNodePtr threeAddressNodePtr(
                                    new ThreeAddressNode(name, saved[item], nullptr, ASTNode::Operator::PUSH,
                                                         getNodeType(constantValue->getVarType())));
                            block.push_back(threeAddressNodePtr);
                        }
                        else {
                            assert(saved[item]);
                            ThreeAddressNodePtr threeAddressNodePtr(
                                    new ThreeAddressNode(name, saved[item], nullptr, ASTNode::Operator::PUSH,
                                                         getNodeType(item->getVarType())));
                            block.push_back(threeAddressNodePtr);
                        }
                    }
                } else if(ConcreteNumValuePtr concreteNumValue = dynamic_pointer_cast<ConcreteNumValue>(ele)) {
                    auto tempRes = make_shared<ThreeAddressNode>(to_string(concreteNumValue->getNumer()), nullptr, nullptr,
                                                                 ASTNode::Operator::NULLOP, getNodeType(ele->getVarType()));
                    saved[ele] = tempRes;
                    // 这里因为不是arrayvalue，所以在处理参数时，并没有用到push操作，
                    // 但是后续进行inline时，是根据push操作来识别参数的
                    // 所以这里增加一层，改为在res中存储新增的带有push操作的threeAddressNodePtr，而非tempRes
                    ThreeAddressNodePtr threeAddressNodePtr(new ThreeAddressNode
                                                                    ("push", saved[ele], nullptr, ASTNode::Operator::PUSH,
                                                                     getNodeType(ele->getVarType())));
                    block.push_back(threeAddressNodePtr);
                } else {
                    assert(saved[ele]);
                    ThreeAddressNodePtr threeAddressNodePtr(new ThreeAddressNode
                                                                    ("push", saved[ele], nullptr, ASTNode::Operator::PUSH,
                                                                     getNodeType(ele->getVarType())));
                    block.push_back(threeAddressNodePtr);
                }
            }

            std::string name = procCallValueIndexPtr->getName();
            ThreeAddressNodePtr func = nullptr;
            if(saved.count(procCallValueIndexPtr->getProcCallValuePtr()) == 0) {
                ThreeAddressNodePtr function(new ThreeAddressNode(procCallValueIndexPtr->getProcCallValuePtr()->getName(), nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::FUNCTION));
                saved[procCallValueIndexPtr->getProcCallValuePtr()] = function;
                func = function;
            } else {
                func = saved[procCallValueIndexPtr->getProcCallValuePtr()];
            }
            // function call 有三种情况：roundFunction， sboxFunction 和 kschdFunction
            if (procCallValuePtr->getProcedurePtr()->getIsRndf() or procCallValuePtr->getProcedurePtr()->getIsSboxf() or procCallValuePtr->getProcedurePtr()->getIsKschd()) {
                while (true) {
                    name = procCallValueIndexPtr->getName();
                    auto index = make_shared<ThreeAddressNode>(std::to_string(procCallValueIndexPtr->getNumber()),
                                                               nullptr, nullptr, ASTNode::Operator::NULLOP,
                                                               getNodeType(procCallValueIndexPtr->getVarType()));
                    auto threeAddressNodePtr = make_shared<ThreeAddressNode>(name, func, index, ASTNode::Operator::CALL,
                                                                             getNodeType(procCallValueIndexPtr->getProcCallValuePtr()->getVarType()));
                    saved[procCallValueIndexPtr] = threeAddressNodePtr;
                    block.push_back(threeAddressNodePtr);
                    k++;
                    ele = procValuePtr->getProcedurePtr()->getBlock().at(k);
                    procCallValueIndexPtr = dynamic_pointer_cast<ProcCallValueIndex>(ele);
                    if (!procCallValueIndexPtr)
                        break;
                }
                k--;
                ele = procValuePtr->getProcedurePtr()->getBlock().at(k);
            } else {
                auto index = make_shared<ThreeAddressNode>(std::to_string(procCallValueIndexPtr->getNumber()),
                                                           nullptr, nullptr, ASTNode::Operator::NULLOP,
                                                           getNodeType(procCallValueIndexPtr->getVarType()));
                auto threeAddressNodePtr = make_shared<ThreeAddressNode>(name, func, index, ASTNode::Operator::CALL,
                                                                         NodeType::UINT);
                saved[procCallValueIndexPtr] = threeAddressNodePtr;
                block.push_back(threeAddressNodePtr);
            }

        }
        else if (!ele) {
            continue;
        }
        // 暂时没有覆盖到的value类型的转换
        else {
            assert(false);
        }
    }


    // 进行return的转换
    if(ArrayValuePtr arrayValuePtr = dynamic_pointer_cast<ArrayValue>(procValuePtr->getProcedurePtr()->getReturns())) {
        for(auto ele1 : arrayValuePtr->getArrayValue()) {
            if(saved.count(ele1) != 0) {
                returns.push_back(saved[ele1]);
            } else {
                if (InternalBinValuePtr internalBinValuePtr = dynamic_pointer_cast<InternalBinValue>(ele1)) {
                    returns.push_back(transformInternalBin(internalBinValuePtr, saved, nameCount));
                } else if (InternalUnValuePtr internalUnValue = dynamic_pointer_cast<InternalUnValue>(ele1)) {
                    returns.push_back(transformInternalUn(internalUnValue, saved, nameCount));
                } else if(ConcreteNumValuePtr concreteNumValue = dynamic_pointer_cast<ConcreteNumValue>(ele1)){
                    auto res = make_shared<ThreeAddressNode>(to_string(concreteNumValue->getNumer()), nullptr, nullptr,
                                                             ASTNode::Operator::NULLOP, getNodeType(concreteNumValue->getVarType()));
                    returns.push_back(res);
                    saved[ele1] = res;
                } else {
                    assert(false);
                }
            }
        }
    } else {
        returns.push_back(saved[procValuePtr->getProcedurePtr()->getReturns()]);
    }

    map<string, ProcedureHPtr> nameToProc;
    for(auto ele : procedureHs) {
        nameToProc[ele->getName()] = ele;
    }

    auto newProc = ProcedureHPtr(new ProcedureH(procValuePtr->getName(), parameters, block, returns, nameToProc));
    return newProc;
}


ThreeAddressNodePtr Transformer::transformInternalBin(InternalBinValuePtr internalBinValuePtr, map<ValuePtr, ThreeAddressNodePtr>& saved, map<string, int>& nameCount) {
    ThreeAddressNodePtr left = nullptr;
    ThreeAddressNodePtr right = nullptr;
    string name = "";
    if(internalBinValuePtr->getName() == "")
        name = "t";
    else
        name = internalBinValuePtr->getName();

    // left
    if(ConcreteNumValuePtr concreteNumValuePtr = dynamic_pointer_cast<ConcreteNumValue>(internalBinValuePtr->getLeft())){
        if(saved.count(internalBinValuePtr->getLeft()) == 0) {
            left = make_shared<ThreeAddressNode>(to_string(concreteNumValuePtr->getNumer()), nullptr, nullptr,
                                                 ASTNode::Operator::NULLOP, getNodeType(concreteNumValuePtr->getVarType()));
            saved[internalBinValuePtr->getLeft()] = left;
        } else {
            left = saved[internalBinValuePtr->getLeft()];
        }
    }
    // 存在数组的index为symbol的情况，所以左右孩子节点都有可能包含是包含symbol的arrayValueIndex的类型，因此需要对左右孩子节点都进行这样的判断，
    else if (shared_ptr<ArrayValueIndex> arrayValueIndex = dynamic_pointer_cast<ArrayValueIndex>(internalBinValuePtr->getLeft())) {
        left = transformArrayValueIndex(*arrayValueIndex, saved, nameCount);
    }
    // 当left是BoxValue时，对应的internalBin为BOXOP，如： uint4 sbox_out = sbox<sbox_in>;
    else if (BoxValuePtr boxValuePtr = dynamic_pointer_cast<BoxValue>(internalBinValuePtr->getLeft())) {
        left = transformBoxValue(boxValuePtr, saved);
    }
    // 当左孩子也为 InternalBinValuePtr 时，递归调用
    else if (InternalBinValuePtr tInternalBinValuePtr = dynamic_pointer_cast<InternalBinValue>(internalBinValuePtr->getLeft())) {
        if(saved.count(tInternalBinValuePtr) == 0) {
            left = transformInternalBin(tInternalBinValuePtr, saved, nameCount);
        } else {
            left = saved[tInternalBinValuePtr];
        }
    }
    else if (ProcCallValueIndexPtr procCallValueIndexPtr = dynamic_pointer_cast<ProcCallValueIndex>(internalBinValuePtr->getLeft())) {
        std::string name = procCallValueIndexPtr->getName();
        ThreeAddressNodePtr func = nullptr;
        if(saved.count(procCallValueIndexPtr) == 0) {
            ThreeAddressNodePtr function(new ThreeAddressNode(procCallValueIndexPtr->getProcCallValuePtr()->getName(), nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::FUNCTION));
            saved[procCallValueIndexPtr->getProcCallValuePtr()] = function;
            func = function;
            auto index = make_shared<ThreeAddressNode>(std::to_string(procCallValueIndexPtr->getNumber()),
                                                       nullptr, nullptr, ASTNode::Operator::NULLOP, getNodeType(procCallValueIndexPtr->getVarType()));
            left = make_shared<ThreeAddressNode>(name, func, index, ASTNode::Operator::CALL, getNodeType(procCallValueIndexPtr->getVarType()));
            left->setNodeName(name);
        } else {
            left = saved[procCallValueIndexPtr];
        }
    }
    else if (ParameterValuePtr parameterValuePtr = dynamic_pointer_cast<ParameterValue>(internalBinValuePtr->getLeft())) {
        // 参数已经在 transform function 中被放到 saved 中
        left = saved[internalBinValuePtr->getLeft()];
        assert(left);
    }
    else if (InternalUnValuePtr internalUnValuePtr = dynamic_pointer_cast<InternalUnValue>(internalBinValuePtr->getLeft())) {
        left = transformInternalUn(internalUnValuePtr, saved, nameCount);
    }
    else {
        assert(false);

        left = saved[internalBinValuePtr->getLeft()];
        if (left == nullptr) {
            if (internalBinValuePtr->getLeft()->toString().find("[SYMBOL]") != string::npos) {
                // 此时left的值是不可计算的，因为可能与输入相关，那么就直接返回表达式表示的形式。
                // 所以此时不能使用存储在saved中的具体数值，而是需要以表达式的方式存储。
                ThreeAddressNodePtr temp_left(new ThreeAddressNode(internalBinValuePtr->getLeft()->getName(),
                                                                   nullptr, nullptr, ASTNode::Operator::NULLOP,
                                                                   getNodeType(internalBinValuePtr->getVarType())));
                left = temp_left;
            }
        }
        assert(left);
    }

    // right
    if(ConcreteNumValuePtr concreteNumValuePtr = dynamic_pointer_cast<ConcreteNumValue>(internalBinValuePtr->getRight())){
        if(saved.count(internalBinValuePtr->getRight()) == 0) {
            right = make_shared<ThreeAddressNode>(to_string(concreteNumValuePtr->getNumer()), nullptr, nullptr,
                                                  ASTNode::Operator::NULLOP, getNodeType(concreteNumValuePtr->getVarType()));
            saved[internalBinValuePtr->getRight()] = right;
        } else {
            right = saved[internalBinValuePtr->getRight()];
        }
    }
    // 同样的，我们也增加对right的判断，看是否其是包含symbol的数组index访问
    else if (shared_ptr<ArrayValueIndex> arrayValueIndex = dynamic_pointer_cast<ArrayValueIndex>(internalBinValuePtr->getRight())) {
        right = transformArrayValueIndex(*arrayValueIndex, saved, nameCount);
    }
    // 当 left 为 boxValue 时，right为arrayValue类型，即对arrayValue整体进行的操作，如 sbox substitution
    // 此时，需要将arrayValue转化成internalBinValuePtr模式的三地址
    else if (ArrayValuePtr arrayValuePtr = dynamic_pointer_cast<ArrayValue>(internalBinValuePtr->getRight())) {
        right = transformArrayValue2oneTAN(arrayValuePtr, saved, nameCount);
    }
    // 当 left 为 boxValue 时，right为也可以为internalUnValue类型（touint)，即对uints整体进行的操作，
    // 如 sbox substitution
    else if (InternalUnValuePtr internalUnValuePtr = dynamic_pointer_cast<InternalUnValue>(internalBinValuePtr->getRight())) {
        right = transformInternalUn(internalUnValuePtr, saved, nameCount);
    }
    else if (InternalBinValuePtr tInternalBinValuePtr = dynamic_pointer_cast<InternalBinValue>(internalBinValuePtr->getRight())) {
        if(saved.count(tInternalBinValuePtr) == 0) {
            right = transformInternalBin(tInternalBinValuePtr, saved, nameCount);
        } else {
            right = saved[tInternalBinValuePtr];
        }
    }
    else if (ProcCallValueIndexPtr procCallValueIndexPtr = dynamic_pointer_cast<ProcCallValueIndex>(internalBinValuePtr->getRight())) {
        std::string name = procCallValueIndexPtr->getName();
        ThreeAddressNodePtr func = nullptr;
        if(saved.count(procCallValueIndexPtr) == 0) {
            ThreeAddressNodePtr function(new ThreeAddressNode(procCallValueIndexPtr->getProcCallValuePtr()->getName(), nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::FUNCTION));
            saved[procCallValueIndexPtr->getProcCallValuePtr()] = function;
            func = function;
            auto index = make_shared<ThreeAddressNode>(std::to_string(procCallValueIndexPtr->getNumber()),
                                                       nullptr, nullptr, ASTNode::Operator::NULLOP, getNodeType(procCallValueIndexPtr->getVarType()));
            right = make_shared<ThreeAddressNode>(name, func, index, ASTNode::Operator::CALL, getNodeType(procCallValueIndexPtr->getVarType()));
            right->setNodeName(name);
        } else {
            right = saved[procCallValueIndexPtr];
        }

    }
    else if (ParameterValuePtr parameterValuePtr = dynamic_pointer_cast<ParameterValue>(internalBinValuePtr->getRight())) {
        // 参数已经在 transform function 中被放到 saved 中
        right = saved[internalBinValuePtr->getRight()];
        assert(right);
    }
    else {
        assert(false);
    }

    /*
     * 无论name等于何值，都需要在后面加上count，防止三地址实例名称重复。
     * 此外，在实际代码中我们可能对同一个变量进行多次赋值，此时在进行转换时，
     * 直接在后面添加一个counter会产生变量标识符重复覆盖的情况，如 p_out[0] = sbox<sbox_in>[0]; p_out[0] = p_out[0] ^ r;
     * 此时两次出现的 p_out[0] 的变量标识符分别为 p_out0 和 p_out01, 貌似没有问题，但是当 p_out[i] 中 i 取 5 呢？
     * 就有两个变量标识符 p_out5 和 p_out51， 会覆盖掉原来名为 p_out51 的变量，所以我们需要设计一种可以避免重复赋值情况下出现重复覆盖的情况
     * 具体的做法是 getCount()会返回一个前面增加一个"_"的counter，以对有些情况下变量名会重复的区分
     * */

    // 检查本节点的name是否和其中一个孩子节点的name相同，如果相同，说明是对一个变量的重复赋值，用 name + "_n" 标记避免重复
    string count;
    if (name == left->getNodeName() or name == right->getNodeName())
        count = "";
    else
        count = getCount(name, nameCount);

    // 根据varType选择NodeType，从而确定返回值的NodeType，分析时可以确定uints的s
    NodeType nodeType = getNodeType(internalBinValuePtr->getVarType());
    if (right->getNodeType() == NodeType::ARRAY)
        nodeType = NodeType::ARRAY;
    ThreeAddressNodePtr rtn(new ThreeAddressNode(name + count, left, right, internalBinValuePtr->getOp(), nodeType));
    left->addParents(rtn);
    right->addParents(rtn);
    saved[internalBinValuePtr] = rtn;
    return rtn;
}


ThreeAddressNodePtr Transformer::transformInternalUn(InternalUnValuePtr internalUnValuePtr, map<ValuePtr, ThreeAddressNodePtr>& saved, map<string, int>& nameCount) {
    string name = "";
    if(internalUnValuePtr->getName() == "")
        name = "t";
    else
        name = internalUnValuePtr->getName();

    ThreeAddressNodePtr operation = saved.count(internalUnValuePtr->getRand()) == 0 ? nullptr : saved[internalUnValuePtr->getRand()];

    if(operation == nullptr) {
        if(ConcreteNumValuePtr concreteNumValuePtr = dynamic_pointer_cast<ConcreteNumValue>(internalUnValuePtr->getRand())) {
            operation =  make_shared<ThreeAddressNode>(to_string(concreteNumValuePtr->getNumer()),
                                                       nullptr, nullptr, ASTNode::Operator::NULLOP, getNodeType(concreteNumValuePtr->getVarType()));
            saved[internalUnValuePtr->getRand()] = operation;
        }
        // 当rand为arrayValue类型时，说明是对arrayValue整体进行的操作，如 touint
        // 此时，需要将arrayValue转化成internalBinValuePtr模式的三地址
        else if (ArrayValuePtr arrayValuePtr = dynamic_pointer_cast<ArrayValue>(internalUnValuePtr->getRand())) {
            operation = transformArrayValue2oneTAN(arrayValuePtr, saved, nameCount);
        }
        else {
            assert(false);
        }
    }

    string count = getCount(name, nameCount);
    NodeType nodeType = getNodeType(internalUnValuePtr->getVarType());
    ThreeAddressNodePtr rtn(new ThreeAddressNode(name + count, operation, nullptr, internalUnValuePtr->getOp(), nodeType));
    operation->addParents(rtn);
    saved[internalUnValuePtr] = rtn;
    return rtn;
}


/*
 * 在ASTNode处理后的结构中，为 arrayValue 的只有 function call，
 *      1. roundFunction call
 *      2. sboxFunction call
 *      3. schdFunction call
 * 因为实际上是一次函数调用，返回一个数组，所以在转化为三地址形式时，
 * 我们只需要将所有的实参转化一次，然后将数组中的每个元素再分别转化为三地址形式
 * */
vector<ThreeAddressNodePtr> Transformer::transformArrayValue(ArrayValuePtr arrayValuePtr,
                                                             map<ValuePtr, ThreeAddressNodePtr>& saved, map<string, int>& nameCount) {
    vector<ThreeAddressNodePtr> res;
    // 实参的三地址转化
    for(auto val : arrayValuePtr->getArrayValue()) {
        if(ProcCallValueIndexPtr procCallValueIndexPtr = dynamic_pointer_cast<ProcCallValueIndex>(val)) {
            ProcCallValuePtr procCallValuePtr = dynamic_pointer_cast<ProcCallValue>(procCallValueIndexPtr->getProcCallValuePtr());
            for(int i = 0; i < procCallValuePtr->getArguments().size(); i++) {
                auto ele = procCallValuePtr->getArguments()[i];
                if(ArrayValuePtr tArrayValuePtr = dynamic_pointer_cast<ArrayValue>(ele)) {
                    for(auto item : tArrayValuePtr->getArrayValue()) {
                        // 如果被调用的对象是轮函数，则需要对每个参数的名字标记处理
                        std::string name = "push";
                        if (procCallValuePtr->getProcedurePtr()->getIsRndf()) {
                            if (i == 1)
                                name = "key_push";
                            else if (i == 2)
                                name = "plaintext_push";
                        } else if (procCallValuePtr->getProcedurePtr()->getIsSboxf()) {
                            name = "sbox_push";
                        } else if (procCallValuePtr->getProcedurePtr()->getIsKschd()) {
                            name = "kschd_push";
                        }
                        if(ConcreteNumValuePtr concreteNumValue = dynamic_pointer_cast<ConcreteNumValue>(item)) {
                            auto tempRes = make_shared<ThreeAddressNode>(to_string(concreteNumValue->getNumer()),
                                                                         nullptr, nullptr,
                                                                         ASTNode::Operator::NULLOP, getNodeType(concreteNumValue->getVarType()));
                            saved[item] = tempRes;
                            ThreeAddressNodePtr threeAddressNodePtr(
                                    new ThreeAddressNode(name, saved[item], nullptr, ASTNode::Operator::PUSH,
                                                         getNodeType(concreteNumValue->getVarType())));
                            res.push_back(threeAddressNodePtr);
                        }
                        else if(ConstantValuePtr constantValue = dynamic_pointer_cast<ConstantValue>(item)) {
                            auto tempRes = make_shared<ThreeAddressNode>(constantValue->getName(),
                                                                         nullptr, nullptr,
                                                                         ASTNode::Operator::NULLOP, getNodeType(constantValue->getVarType()));
                            saved[item] = tempRes;
                            ThreeAddressNodePtr threeAddressNodePtr(
                                    new ThreeAddressNode(name, saved[item], nullptr, ASTNode::Operator::PUSH,
                                                         getNodeType(constantValue->getVarType())));
                            res.push_back(threeAddressNodePtr);
                        }
                        else {
                            assert(saved[item]);
                            ThreeAddressNodePtr threeAddressNodePtr(
                                    new ThreeAddressNode(name, saved[item], nullptr, ASTNode::Operator::PUSH,
                                                         getNodeType(item->getVarType())));
                            res.push_back(threeAddressNodePtr);
                        }
                    }
                } else if(ConcreteNumValuePtr concreteNumValue = dynamic_pointer_cast<ConcreteNumValue>(ele)) {
                    auto tempRes = make_shared<ThreeAddressNode>(to_string(concreteNumValue->getNumer()), nullptr, nullptr,
                                                                 ASTNode::Operator::NULLOP, getNodeType(ele->getVarType()));
                    // res.push_back(tempres);
                    saved[ele] = tempRes;
                    // 这里因为不是arrayvalue，所以在处理参数时，并没有用到push操作，但是后续进行inline时，是根据push操作来识别参数的
                    // 所以这里增加一层，改为在res中存储新增的带有push操作的threeAddressNodePtr，而非tempres
                    ThreeAddressNodePtr threeAddressNodePtr(new ThreeAddressNode
                                                                    ("push", saved[ele], nullptr, ASTNode::Operator::PUSH,
                                                                     getNodeType(ele->getVarType())));
                    res.push_back(threeAddressNodePtr);
                } else {
                    assert(saved[ele]);
                    ThreeAddressNodePtr threeAddressNodePtr(new ThreeAddressNode
                                                                    ("push", saved[ele], nullptr, ASTNode::Operator::PUSH,
                                                                     getNodeType(ele->getVarType())));
                    res.push_back(threeAddressNodePtr);
                }
            }
        }
        break;
    }

    // 返回数组的每个元素的三地址转化
    for(int i = 0; i < arrayValuePtr->getArrayValue().size(); i++) {
        auto ele = arrayValuePtr->getArrayValue()[i];
        if(ProcCallValueIndexPtr procCallValueIndexPtr = dynamic_pointer_cast<ProcCallValueIndex>(ele)) {
            ThreeAddressNodePtr func = nullptr;
            if(saved.count(procCallValueIndexPtr->getProcCallValuePtr()) == 0) {
                ThreeAddressNodePtr function(new ThreeAddressNode(procCallValueIndexPtr->getProcCallValuePtr()->getName(), nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::FUNCTION));
                saved[procCallValueIndexPtr->getProcCallValuePtr()] = function;
                func = function;
            } else {
                func = saved[procCallValueIndexPtr->getProcCallValuePtr()];
            }

            string name = "t";
            string count = getCount(name, nameCount);
            ThreeAddressNodePtr threeAddressNodePtr(new ThreeAddressNode(name + count, func, nullptr, ASTNode::Operator::CALL,
                                                                         getNodeType(ele->getVarType())));
            threeAddressNodePtr->setIndexCall(i);
            res.push_back(threeAddressNodePtr);
            saved[ele] = threeAddressNodePtr;
        } else {
            // 非 function call 的 value 报错
            assert(false);
        }
    }

    return res;
}


// 本函数用于处理直接进行整体操作的ArrayValue，将arrayValue的各个元素用操作符BOXINDEX连接，转换成对应internalBinValue的对应三地址形式
ThreeAddressNodePtr Transformer::transformArrayValue2oneTAN(ArrayValuePtr arrayValuePtr, map<ValuePtr, ThreeAddressNodePtr> &saved,
                                        map<string, int> &nameCount) {
    ThreeAddressNodePtr left = nullptr;
    ThreeAddressNodePtr right = nullptr;

    // 对每个元素，按照其对象类型，生成对应类型的三地址形式
    ValuePtr ele0 = arrayValuePtr->getValueAt(0);
    if(ConcreteNumValuePtr concreteNumValuePtr = dynamic_pointer_cast<ConcreteNumValue>(ele0)){
        if(saved.count(ele0) == 0) {
            left = make_shared<ThreeAddressNode>(to_string(concreteNumValuePtr->getNumer()), nullptr, nullptr,
                                                  ASTNode::Operator::NULLOP, getNodeType(concreteNumValuePtr->getVarType()));
            saved[ele0] = left;
        } else {
            left = saved[ele0];
        }
    }
    else if (shared_ptr<ArrayValueIndex> arrayValueIndex = dynamic_pointer_cast<ArrayValueIndex>(ele0)) {
        std::string name = arrayValueIndex->getName();
        left = transformArrayValueIndex(*arrayValueIndex, saved, nameCount);
        left->setNodeName(name);
    }
    else if (InternalBinValuePtr internalBinValuePtr = dynamic_pointer_cast<InternalBinValue>(ele0)) {
        // 因为transformInternalBin会更改node的名字（后面增加一个counter），所以备份初始的名字，并且后面也更新一下
        std::string name = internalBinValuePtr->getName();
        left = transformInternalBin(internalBinValuePtr, saved, nameCount);
        left->setNodeName(name);
    }
    else if (InternalUnValuePtr internalUnValue = dynamic_pointer_cast<InternalUnValue>(ele0)) {
        std::string name = internalUnValue->getName();
        left = transformInternalUn(internalUnValue, saved, nameCount);
        left->setNodeName(name);
    }
    // box的操作对象可以直接是参数
    else if (ParameterValuePtr parameterValuePtr = dynamic_pointer_cast<ParameterValue>(ele0)) {
        if(saved.count(parameterValuePtr) == 0) {
            string count = getCount(ele0->getName(), nameCount);
            left = make_shared<ThreeAddressNode>(ele0->getName() + count, nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::PARAMETER);
            saved[ele0] = left;
        } else {
            left = saved[parameterValuePtr];
        }
    }
    // sbox function call -> ProcCallValueIndex
    else if (ProcCallValueIndexPtr procCallValueIndexPtr = dynamic_pointer_cast<ProcCallValueIndex>(ele0)) {
        std::string name = procCallValueIndexPtr->getName();
        ThreeAddressNodePtr func = nullptr;
        if(saved.count(procCallValueIndexPtr) == 0) {
            ThreeAddressNodePtr function(new ThreeAddressNode(procCallValueIndexPtr->getProcCallValuePtr()->getName(), nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::FUNCTION));
            saved[procCallValueIndexPtr->getProcCallValuePtr()] = function;
            func = function;
            auto index = make_shared<ThreeAddressNode>(std::to_string(procCallValueIndexPtr->getNumber()),
                                                       nullptr, nullptr, ASTNode::Operator::NULLOP, getNodeType(procCallValueIndexPtr->getVarType()));
            left = make_shared<ThreeAddressNode>(name, func, index, ASTNode::Operator::CALL, getNodeType(procCallValueIndexPtr->getVarType()));
            left->setNodeName(name);
        } else {
            left = saved[procCallValueIndexPtr];
        }
    }
    else if(ConstantValuePtr constantValuePtr = dynamic_pointer_cast<ConstantValue>(ele0)){
        if(saved.count(ele0) == 0) {
            left = make_shared<ThreeAddressNode>(constantValuePtr->getName(), nullptr, nullptr,
                                                 ASTNode::Operator::NULLOP, getNodeType(constantValuePtr->getVarType()));
            saved[ele0] = left;
        } else {
            left = saved[ele0];
        }
    }
    else {
        assert(false);
    }

    std::vector<ValuePtr> leftoverArray;
    for (int i = 1; i < arrayValuePtr->getArrayValue().size(); ++i) {
        leftoverArray.push_back(arrayValuePtr->getValueAt(i));
    }
    if (!leftoverArray.empty()) {
        ArrayValuePtr leftover = make_shared<ArrayValue>("leftover", leftoverArray);
        right = transformArrayValue2oneTAN(leftover, saved, nameCount);

        ThreeAddressNodePtr rtn = make_shared<ThreeAddressNode>(arrayValuePtr->getName() + std::to_string(leftoverCounter), left, right,
                                                                ASTNode::Operator::BOXINDEX, NodeType::ARRAY);
        leftoverCounter++;
        return rtn;
    }
    else {
        return left;
    }
}


void Transformer::transformProcedures() {
    int sboxFuncIdx = 0; // There may be multiple functions for sboxes
    for(auto ele : procedures) {
        if (ele->getProcedurePtr()->getIsFn()) {
            ele->setName("main");
        }
        if (ele->getProcedurePtr()->getIsSboxf()) {
            sboxFuncIdx++;
        }
        ProcedureHPtr temp = transform(ele);
        nameToProc[ele->getName()] = temp;
        procedureHs.push_back(temp);
    }
}


string Transformer::getCount(string name, map<string, int>& nameCount) {
    if(nameCount.count(name) == 0) {
        nameCount[name] = 0;
        return "";
    } else {
        nameCount[name]++;
        //return to_string(nameCount[name]);
        // 前面增加一个"_"作以对有些情况下，变量名会重复的区分
        return "_#" + to_string(nameCount[name]);
    }
}


const vector<ProcedureHPtr> &Transformer::getProcedureHs() const {
    return procedureHs;
}


ThreeAddressNodePtr Transformer::transformArrayValueIndex(ArrayValueIndex arrayValueIndex, map<ValuePtr, ThreeAddressNodePtr> &saved,
                                                          map<string, int> &nameCount) {
    ThreeAddressNodePtr left = nullptr;
    ThreeAddressNodePtr right = nullptr;
    //这里ArrayValueIndex转换成三地址形式，左孩子为数组名，右孩子为表达式，即寻找index的表达式，同时本节点的名字为t
    //若 arrayValueIndex name 为空，命名为 t，否则就沿用原名
    string name;
    if(arrayValueIndex.getName() == "")
        name = "t";
    else
        name = arrayValueIndex.getName();
    string count = getCount(name, nameCount);

    // 左孩子节点存储对应的数组的三地址形式
    if (saved.count(arrayValueIndex.getArrayValuePtr()) == 0) {
        ThreeAddressNodePtr tempLeft(new ThreeAddressNode(arrayValueIndex.getArrayValuePtr()->getName(), nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::ARRAY));
        left = tempLeft;
        saved[arrayValueIndex.getArrayValuePtr()] = left;
    } else {
        left = saved[arrayValueIndex.getArrayValuePtr()];
    }

    // 右孩子节点存储访问index的对应表达式的三地址形式
    if (InternalBinValuePtr internalBinValuePtr = dynamic_pointer_cast<InternalBinValue>(arrayValueIndex.getSymbolIndex())) {
        right = transformInternalBin(internalBinValuePtr, saved, nameCount);
    }
    // 可能直接将参数作为symbol index
    else if (ParameterValuePtr parameterValuePtr = dynamic_pointer_cast<ParameterValue>(arrayValueIndex.getSymbolIndex())) {
        if(saved.count(parameterValuePtr) == 0) {
            string tCount = getCount(arrayValueIndex.getSymbolIndex()->getName(), nameCount);
            right = make_shared<ThreeAddressNode>(arrayValueIndex.getSymbolIndex()->getName() + tCount, nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::PARAMETER);
            saved[arrayValueIndex.getSymbolIndex()] = right;
        } else {
            right = saved[parameterValuePtr];
        }
    }
    else {
        assert(false);
    }
    right->setNodeName("SYMBOL");
    ThreeAddressNodePtr rtn(new ThreeAddressNode(name + count, left, right,
                                                 ASTNode::Operator::INDEX, getNodeType(arrayValueIndex.getVarType())));
    return rtn;
}


ThreeAddressNodePtr Transformer::transformBoxValue(BoxValuePtr boxValuePtr, map<ValuePtr, ThreeAddressNodePtr> &saved) {
    ThreeAddressNodePtr left = nullptr;
    ThreeAddressNodePtr right = nullptr;
    // 这里BoxValue转换成三地址形式，左右孩子均为空，Box的具体值在ASTNode时另外存储，后续建模过程中单独处理，
    // 转换后的三地址形式用于索引查找对应Box的具体取值
    // 这里用boxType和box标识符连接作为标识符，可以根据boxType来判断对应的操作选择
    string name = boxValuePtr->getBoxType() + boxValuePtr->getName();
    ThreeAddressNodePtr rtn(new ThreeAddressNode(name, nullptr,
                                                       nullptr, ASTNode::Operator::NULLOP, NodeType::BOX));
    saved[boxValuePtr] = rtn;
    return rtn;
}


NodeType Transformer::getNodeType(ASTNode::Type type) {
    if (type == ASTNode::uint) {
        return UINT;
    }
    else if (type == ASTNode::uint1) {
        return UINT1;
    }
    else if (type == ASTNode::uint4) {
        return UINT4;
    }
    else if (type == ASTNode::uint6) {
        return UINT6;
    }
    else if (type == ASTNode::uint8) {
        return UINT8;
    }
    else if (type == ASTNode::uint12) {
        return UINT12;
    }
    else if (type == ASTNode::uint16) {
        return UINT16;
    }
    else if (type == ASTNode::uint32) {
        return UINT32;
    }
    else if (type == ASTNode::uint64) {
        return UINT64;
    }
    else if (type == ASTNode::uint128) {
        return UINT128;
    }
    else if (type == ASTNode::uint256) {
        return UINT256;
    }
    return UINT;
}


int Transformer::getNodeTypeSize(NodeType nodeType) {
    if (nodeType == UINT) {
        return 0;
    }
    else if (nodeType == UINT1) {
        return 1;
    }
    else if (nodeType == UINT4) {
        return 4;
    }
    else if (nodeType == UINT6) {
        return 6;
    }
    else if (nodeType == UINT8) {
        return 8;
    }
    else if (nodeType == UINT12) {
        return 12;
    }
    else if (nodeType == UINT16) {
        return 16;
    }
    else if (nodeType == UINT32) {
        return 32;
    }
    else if (nodeType == UINT64) {
        return 64;
    }
    else if (nodeType == UINT128) {
        return 128;
    }
    else if (nodeType == UINT256) {
        return 256;
    }
    else
        assert(false);
    return 0;
}


