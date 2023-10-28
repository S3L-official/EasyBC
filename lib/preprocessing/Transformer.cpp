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
            for(int i = 0; i < procCallValuePtr->getArguments().size(); i++) {
                auto ele = procCallValuePtr->getArguments()[i];
                if(ArrayValuePtr eleArrayValuePtr = dynamic_pointer_cast<ArrayValue>(ele)) {
                    for(auto item : eleArrayValuePtr->getArrayValue()) {
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
        else {
            assert(false);
        }
    }

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

    if(ConcreteNumValuePtr concreteNumValuePtr = dynamic_pointer_cast<ConcreteNumValue>(internalBinValuePtr->getLeft())){
        if(saved.count(internalBinValuePtr->getLeft()) == 0) {
            left = make_shared<ThreeAddressNode>(to_string(concreteNumValuePtr->getNumer()), nullptr, nullptr,
                                                 ASTNode::Operator::NULLOP, getNodeType(concreteNumValuePtr->getVarType()));
            saved[internalBinValuePtr->getLeft()] = left;
        } else {
            left = saved[internalBinValuePtr->getLeft()];
        }
    }
    else if (shared_ptr<ArrayValueIndex> arrayValueIndex = dynamic_pointer_cast<ArrayValueIndex>(internalBinValuePtr->getLeft())) {
        left = transformArrayValueIndex(*arrayValueIndex, saved, nameCount);
    }
    else if (BoxValuePtr boxValuePtr = dynamic_pointer_cast<BoxValue>(internalBinValuePtr->getLeft())) {
        left = transformBoxValue(boxValuePtr, saved);
    }
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
                ThreeAddressNodePtr temp_left(new ThreeAddressNode(internalBinValuePtr->getLeft()->getName(),
                                                                   nullptr, nullptr, ASTNode::Operator::NULLOP,
                                                                   getNodeType(internalBinValuePtr->getVarType())));
                left = temp_left;
            }
        }
        assert(left);
    }

    if(ConcreteNumValuePtr concreteNumValuePtr = dynamic_pointer_cast<ConcreteNumValue>(internalBinValuePtr->getRight())){
        if(saved.count(internalBinValuePtr->getRight()) == 0) {
            right = make_shared<ThreeAddressNode>(to_string(concreteNumValuePtr->getNumer()), nullptr, nullptr,
                                                  ASTNode::Operator::NULLOP, getNodeType(concreteNumValuePtr->getVarType()));
            saved[internalBinValuePtr->getRight()] = right;
        } else {
            right = saved[internalBinValuePtr->getRight()];
        }
    }
    else if (shared_ptr<ArrayValueIndex> arrayValueIndex = dynamic_pointer_cast<ArrayValueIndex>(internalBinValuePtr->getRight())) {
        right = transformArrayValueIndex(*arrayValueIndex, saved, nameCount);
    }
    else if (ArrayValuePtr arrayValuePtr = dynamic_pointer_cast<ArrayValue>(internalBinValuePtr->getRight())) {
        right = transformArrayValue2oneTAN(arrayValuePtr, saved, nameCount);
    }
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
        right = saved[internalBinValuePtr->getRight()];
        assert(right);
    }
    else {
        assert(false);
    }

    string count;
    if (name == left->getNodeName() or name == right->getNodeName())
        count = "";
    else
        count = getCount(name, nameCount);

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

vector<ThreeAddressNodePtr> Transformer::transformArrayValue(ArrayValuePtr arrayValuePtr,
                                                             map<ValuePtr, ThreeAddressNodePtr>& saved, map<string, int>& nameCount) {
    vector<ThreeAddressNodePtr> res;
    for(auto val : arrayValuePtr->getArrayValue()) {
        if(ProcCallValueIndexPtr procCallValueIndexPtr = dynamic_pointer_cast<ProcCallValueIndex>(val)) {
            ProcCallValuePtr procCallValuePtr = dynamic_pointer_cast<ProcCallValue>(procCallValueIndexPtr->getProcCallValuePtr());
            for(int i = 0; i < procCallValuePtr->getArguments().size(); i++) {
                auto ele = procCallValuePtr->getArguments()[i];
                if(ArrayValuePtr tArrayValuePtr = dynamic_pointer_cast<ArrayValue>(ele)) {
                    for(auto item : tArrayValuePtr->getArrayValue()) {
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
                    saved[ele] = tempRes;
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
            assert(false);
        }
    }

    return res;
}


ThreeAddressNodePtr Transformer::transformArrayValue2oneTAN(ArrayValuePtr arrayValuePtr, map<ValuePtr, ThreeAddressNodePtr> &saved,
                                        map<string, int> &nameCount) {
    ThreeAddressNodePtr left = nullptr;
    ThreeAddressNodePtr right = nullptr;

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
        std::string name = internalBinValuePtr->getName();
        left = transformInternalBin(internalBinValuePtr, saved, nameCount);
        left->setNodeName(name);
    }
    else if (InternalUnValuePtr internalUnValue = dynamic_pointer_cast<InternalUnValue>(ele0)) {
        std::string name = internalUnValue->getName();
        left = transformInternalUn(internalUnValue, saved, nameCount);
        left->setNodeName(name);
    }
    else if (ParameterValuePtr parameterValuePtr = dynamic_pointer_cast<ParameterValue>(ele0)) {
        if(saved.count(parameterValuePtr) == 0) {
            string count = getCount(ele0->getName(), nameCount);
            left = make_shared<ThreeAddressNode>(ele0->getName() + count, nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::PARAMETER);
            saved[ele0] = left;
        } else {
            left = saved[parameterValuePtr];
        }
    }
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
    int sboxFuncIdx = 0;
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
    string name;
    if(arrayValueIndex.getName() == "")
        name = "t";
    else
        name = arrayValueIndex.getName();
    string count = getCount(name, nameCount);

    if (saved.count(arrayValueIndex.getArrayValuePtr()) == 0) {
        ThreeAddressNodePtr tempLeft(new ThreeAddressNode(arrayValueIndex.getArrayValuePtr()->getName(), nullptr, nullptr, ASTNode::Operator::NULLOP, NodeType::ARRAY));
        left = tempLeft;
        saved[arrayValueIndex.getArrayValuePtr()] = left;
    } else {
        left = saved[arrayValueIndex.getArrayValuePtr()];
    }

    if (InternalBinValuePtr internalBinValuePtr = dynamic_pointer_cast<InternalBinValue>(arrayValueIndex.getSymbolIndex())) {
        right = transformInternalBin(internalBinValuePtr, saved, nameCount);
    }
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


