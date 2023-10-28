//
// Created by Septi on 6/28/2022.
//

#include "ProcedureH.h"

ProcedureH::ProcedureH(string name, const vector<vector<ThreeAddressNodePtr>> &parameters, const vector<ThreeAddressNodePtr> &block,
                       const vector<ThreeAddressNodePtr> &returns, const map<string, ProcedureHPtr>& nameToProc) : name(name), parameters(parameters), block(block),
                                                                                                                   returns(returns), nameToProc(nameToProc) {}

string ProcedureH::to_string() {
    string res = "";
    res += name;
    res += "(";
    for(auto ele : parameters) {
        for(auto ele1 : ele)
            res += ele1->getNodeName() + ",";
    }
    res += ") {\n";
    for(auto ele : block) {
        res += ele->prettyPrint4();
    }
    res += "return ";
    for(auto ele : returns) {
        res += ele->getNodeName() + ",";
    }
    res += "\n";
    return res;
}

const string &ProcedureH::getName() const {
    return name;
}

const vector<vector<ThreeAddressNodePtr>> &ProcedureH::getParameters() const {
    return parameters;
}

const vector<ThreeAddressNodePtr> &ProcedureH::getBlock() const {
    return block;
}

const vector<ThreeAddressNodePtr> &ProcedureH::getReturns() const {
    return returns;
}

void ProcedureH::arrangeArguments(vector<ThreeAddressNodePtr>& functionCall, map<ThreeAddressNodePtr, vector<ActualPara>>& mapCallToArguments,
                                  map<ThreeAddressNodePtr, set<ThreeAddressNodePtr>>& mapFuncCallArgumentsToCalls, map<ActualPara, set<ThreeAddressNodePtr>>& mapInputArgumentsToCalls,
                                  map<ThreeAddressNodePtr, vector<ThreeAddressNodePtr>>& mapCallToReturnValues) {
    list<ThreeAddressNodePtr> nodelist;
    std::copy(this->getBlock().begin(), this->getBlock().end(), std::back_inserter(nodelist));
    vector<ThreeAddressNodePtr> actualArguments;
    map<ThreeAddressNodePtr, vector<ThreeAddressNodePtr>> actualParaMap;

    vector<ThreeAddressNodePtr> return_value;

    vector<ActualPara> actualParas;

    set<string> nodeNames;

    map<ActualPara, set<ThreeAddressNodePtr>> mapArgumentsToCalls;

    map<ThreeAddressNodePtr, set<ThreeAddressNodePtr>> rootcauseOfMine;

    for (auto ele : this->getParameters()) {
        for (auto ele1 : ele) {
            rootcauseOfMine[ele1] = set<ThreeAddressNodePtr>{};
        }
    }

    while (!nodelist.empty()) {
        ThreeAddressNodePtr current = nodelist.front();
        nodelist.pop_front();
        if (current->getOp() == ASTNode::Operator::PUSH) {
            ThreeAddressNodePtr valueOfPush = current->getLhs();
            actualArguments.push_back(valueOfPush);
            if (!valueOfPush->getLhs()) {
                actualParaMap[nullptr].push_back(valueOfPush);
            } else {
                actualParaMap[valueOfPush->getLhs()].push_back(valueOfPush);
            }
        } else if (current->getOp() == ASTNode::Operator::CALL) {
            return_value.push_back(current);
            if (nodelist.empty() || (nodelist.front() && nodelist.front()->getOp() != ASTNode::Operator::CALL)) {

                buildActualParas(actualParas, actualArguments, current, rootcauseOfMine);

                for (auto ele : actualParas) {
                    mapArgumentsToCalls[ele].insert(current->getLhs());
                    if (ele.first[0]->getOp() == ASTNode::Operator::CALL) {
                        mapFuncCallArgumentsToCalls[ele.first[0]->getLhs()].insert(current->getLhs());
                    } else {
                        mapInputArgumentsToCalls[ele].insert(current->getLhs());
                    }
                }

                functionCall.push_back(current->getLhs());
                callToNodeName[current->getLhs()] = current->getNodeName();
                if (nodeNames.count(current->getNodeName()) == 0)
                    nodeNames.insert(current->getNodeName());
                else
                    assert(false);
                mapCallToArguments[current->getLhs()] = actualParas;
                mapCallToReturnValues[current->getLhs()] = return_value;

                actualParaMap.clear();
                return_value.clear();
                actualArguments.clear();
                actualParas.clear();
            }
        }
    }
}

void ProcedureH::buildActualParas(vector<ActualPara>& actualParas, const vector<ThreeAddressNodePtr>& actualArguments,
                                  ThreeAddressNodePtr first, map<ThreeAddressNodePtr, set<ThreeAddressNodePtr>>& rootcauseOfMine) {
    assert(actualParas.size() == 0);
    vector<vector<ThreeAddressNodePtr>> actualParaVector = dealWithPushArguments(actualArguments, first);
    vector<set<ThreeAddressNodePtr>> savedShares;

    string::size_type pos1 = first->getLhs()->getNodeName().find("@");
    string calleeName = first->getLhs()->getNodeName().substr(0, pos1);

    for (int i = 0; i < actualParaVector.size(); i++) {
        auto ele = actualParaVector[i];
        ThreeAddressNodePtr funcCall = ele[0]->getLhs();
        bool isFuncCall = (ele[0]->getOp() == ASTNode::Operator::CALL ? 1 : 0);
        if(!isFuncCall) {
            if(calleeName == "SecMult") {
                ActualPara actualPara(ele, false);
                actualParas.push_back(actualPara);
            } else {
                set<ThreeAddressNodePtr> savedShare;
                if (isShare(ele, rootcauseOfMine, savedShare)) {
                    ActualPara actualPara(ele, true);
                    actualParas.push_back(actualPara);
                    savedShares.push_back(savedShare);
                } else {
                    ActualPara actualPara(ele, false);
                    actualParas.push_back(actualPara);
                }
            }
        } else if (isFuncCall){
            string::size_type pos = funcCall->getNodeName().find("@");
            string funcName = funcCall->getNodeName().substr(0, pos);

            if(calleeName == "SecMult") {
                set<ThreeAddressNodePtr> savedShare;
                if(isSNIShare(ele, rootcauseOfMine, savedShare)) {
                    ActualPara actualPara(ele, true);
                    actualParas.push_back(actualPara);
                    savedShares.push_back(savedShare);
                } else {
                    ActualPara actualPara(ele, false);
                    actualParas.push_back(actualPara);
                }
            } else {
                set<ThreeAddressNodePtr> savedShare;
                if (isShare(ele, rootcauseOfMine, savedShare)) {
                    ActualPara actualPara(ele, true);
                    actualParas.push_back(actualPara);
                    savedShares.push_back(savedShare);
                } else {
                    ActualPara actualPara(ele, false);
                    actualParas.push_back(actualPara);
                }
            }
        } else {
            assert(false);
            ActualPara actualPara(ele, false);
            actualParas.push_back(actualPara);
        }
    }

}

bool ProcedureH::isShare(vector<ThreeAddressNodePtr>& actualPara, map<ThreeAddressNodePtr, set<ThreeAddressNodePtr>>& rootcauseOfMine, set<ThreeAddressNodePtr>& savedShare) {
    vector<set<ThreeAddressNodePtr>> tempRootcause;
    for(auto ele1 : actualPara) {
        tempRootcause.push_back(rootcauseOfMine[ele1]);
    }

    set<ThreeAddressNodePtr> temp = tempRootcause[0];
    if(temp.size() == 0)
        return false;
    for(int i = 1; i < tempRootcause.size(); i++) {
        if(tempRootcause[i] != temp )
            return false;
    }
    savedShare = temp;
    return true;

}

bool ProcedureH::isSNIShare(vector<ThreeAddressNodePtr>& actualPara, map<ThreeAddressNodePtr, set<ThreeAddressNodePtr>>& rootcauseOfMine, set<ThreeAddressNodePtr>& savedShare) {
//    vector<set<ThreeAddressNodePtr>> tempRootcause;
//    for(auto ele1 : actualPara) {
//        tempRootcause.push_back(rootcauseOfMine[ele1]);
//    }
//
//    set<ThreeAddressNodePtr> temp = tempRootcause[0];
//    if(temp.size() == 0)
//        return false;
//    for(int i = 1; i < tempRootcause.size(); i++) {
//        if(tempRootcause[i] != temp )
//            return false;
//    }
//    savedShare = temp;
//
//    for(auto ele : temp) {
//        string::size_type pos1 = ele->getNodeName().find("@");
//        string calleeName = ele->getNodeName().substr(0, pos1);
//        ProcedureHPtr callee = nameToProc[calleeName];
//        if(storedInfo[callee].second == CheckType::SNI)
//            return true;
//    }
    return false;
}



vector<vector<ThreeAddressNodePtr>> ProcedureH::dealWithPushArguments(const vector<ThreeAddressNodePtr> & actualArgument, ThreeAddressNodePtr callNode) {
    string::size_type pos = callNode->getLhs()->getNodeName().find("@");
    string funcName = callNode->getLhs()->getNodeName().substr(0, pos);
    vector<vector<ThreeAddressNodePtr>> formalParas = nameToProc[funcName]->getParameters();

    vector<vector<ThreeAddressNodePtr>> actualParas;
    list<ThreeAddressNodePtr> tempList;
    for(auto ele : actualArgument) {
        tempList.push_back(ele);
    }
    for(auto ele : formalParas) {
        vector<ThreeAddressNodePtr> actualPara;
        for(auto ele1 : ele) {
            ThreeAddressNodePtr first = tempList.front();
            tempList.pop_front();
            actualPara.push_back(first);
        }
        actualParas.push_back(actualPara);
    }
    return actualParas;

}


ThreeAddressNodePtr ProcedureH::instantitateNode(ProcedureHPtr proc, string basename, ThreeAddressNodePtr node, map<ThreeAddressNodePtr, ThreeAddressNodePtr>& formalToActual, map<string, ThreeAddressNodePtr>& saved, map<ThreeAddressNodePtr, ThreeAddressNodePtr >& savedForFunction, string path, set<ThreeAddressNodePtr>& randomBackup) {
    if(!node)
        return nullptr;

    if(formalToActual.count(node) != 0) {
        saved[node->getNodeName()] = formalToActual[node];
        return formalToActual[node];
    } else if(node->getNodeName() != "push" && saved.count(node->getNodeName()) != 0) {
        return saved[node->getNodeName()];
    } else if(node->getNodeName() == "push" && saved.count(node->getNodeName()) != 0 &&
              saved[node->getNodeName()]->getLhs()->getNodeName() == node->getLhs()->getNodeName())  {
        return saved[node->getNodeName()];
    } else if(savedForFunction.count(node) != 0) {
        return savedForFunction[node];
    }
    else {
        ThreeAddressNodePtr left = instantitateNode(proc, basename, node->getLhs(), formalToActual, saved, savedForFunction, path, randomBackup);
        ThreeAddressNodePtr right = instantitateNode(proc, basename, node->getRhs(), formalToActual, saved, savedForFunction, path, randomBackup);

        ThreeAddressNodePtr newNode = make_shared<ThreeAddressNode>(node->getNodeName() + path + "(" + basename + ")", left, right, node->getOp(), node->getNodeType());

        if(left && left->getNodeType() == NodeType::FUNCTION)
            newNode->setIndexCall(node->getIndexCall());
        if(left)
            left->addParents(newNode);
        if(right)
            right->addParents(newNode);

        if(node->getNodeName() != "push" && saved.count(node->getNodeName()) != 0)
            assert(false);
        if(node->getNodeType() == NodeType::FUNCTION) {
            savedForFunction[node] = newNode;
        } else {
            saved[node->getNodeName()] = newNode;
        }

        return newNode;
    }
}

const map<ThreeAddressNodePtr, string> &ProcedureH::getCallToNodeName() const {
    return callToNodeName;
}

const map<string, ProcedureHPtr> &ProcedureH::getNameToProc() const {
    return nameToProc;
}

void ProcedureH::ProcedureHInfoPrint() {
    std::cout << "name : " << name << std::endl;
    std::cout << "parameters : " << std::endl;
    for (auto param : parameters) {
        for (auto p : param) {
            std::cout <<  p->getNodeName() << " " << p->getNodeType() << " ";
        }
        std::cout << std::endl;
    }
    for (auto b : block) {
        std::cout << b->getNodeName() << " " << b->getLhs() << " \n";
        std::cout << b->prettyPrint4() << std::endl;
    }
    std::cout << std::endl;

}
