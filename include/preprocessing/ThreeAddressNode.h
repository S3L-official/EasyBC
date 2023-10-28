//
// Created by Septi on 6/27/2022.
//

#ifndef PLBENCH_THREEADDRESSNODE_H
#define PLBENCH_THREEADDRESSNODE_H

#include <string>
#include "ASTNode.h"
#include <set>
using namespace std;
enum NodeType {
    PARAMETER,
    RETURN,
    FUNCTION,
    ARRAY,
    BOX,
    UINT,
    UINT1,
    UINT4,
    UINT6,
    UINT8,
    UINT12,
    UINT16,
    UINT32,
    UINT64,
    UINT128,
    UINT256,
};


class ThreeAddressNode {

private:
    ASTNode::Operator op;
    ThreeAddressNodePtr lhs;
    ThreeAddressNodePtr rhs;
    vector<ThreeAddressNodeWeakPtr> parents;
    int numberOfChild;

    string nodeName;
    int SSAIndex;
    int lineNumber;
    NodeType nodeType;
    int indexOfCall;

    int value;


public:
    ThreeAddressNode(string nodeName, const ThreeAddressNodePtr &lhs, const ThreeAddressNodePtr &rhs,
                     ASTNode::Operator op, NodeType nodeType) :
                     op(op),
                     lhs(lhs),
                     rhs(rhs),
                     nodeName(nodeName),
                     nodeType(nodeType) {
        SSAIndex = 0;
        lineNumber = 0;

        if(lhs == nullptr && rhs == nullptr)
            numberOfChild = 0;
        else if(lhs != nullptr && rhs == nullptr)
            numberOfChild = lhs->numberOfChild;
        else if(lhs != nullptr && rhs != nullptr)
            numberOfChild = lhs->numberOfChild + rhs->numberOfChild;
        else
            assert(false);
    };

    ThreeAddressNode(string nodeName, ThreeAddressNodePtr node) :
                     nodeName(nodeName),
                     lhs(node->lhs),
                     rhs(node->rhs),
                     op(node->op),
                     nodeType(node->nodeType) {
        lineNumber = 0;
        SSAIndex = 0;
        numberOfChild = node->numberOfChild;
    };

    ThreeAddressNode(ThreeAddressNodePtr& addressPtr, map<ThreeAddressNodePtr, ThreeAddressNodePtr>& nodeMap) {
        nodeName = addressPtr->nodeName;
        if(addressPtr->lhs != nullptr) {
            assert(nodeMap.count(addressPtr->lhs) != 0);
            lhs = nodeMap[addressPtr->lhs];
        }
        if(addressPtr->rhs != nullptr) {
            assert(nodeMap.count(addressPtr->rhs) != 0);
            rhs = nodeMap[addressPtr->rhs];
        }
        op = addressPtr->op;
        nodeType = addressPtr->nodeType;
        SSAIndex = addressPtr->SSAIndex;
        lineNumber = addressPtr->lineNumber;
        numberOfChild = 0;
    }

    ThreeAddressNode(ThreeAddressNodePtr& addressPtr) {
        nodeName = addressPtr->nodeName;
        op = addressPtr->op;
        nodeType = addressPtr->nodeType;
        SSAIndex = addressPtr->SSAIndex;
        lineNumber = addressPtr->lineNumber;
        numberOfChild = addressPtr->numberOfChild;
        lhs = nullptr;
        rhs = nullptr;
        indexOfCall = addressPtr->indexOfCall;
        numberOfChild = 0;

    }

    ThreeAddressNodePtr createNodePtr(string nodeName, const ThreeAddressNodePtr &lhs, const ThreeAddressNodePtr &rhs,
                                      ASTNode::Operator op, NodeType nodeType) {
        return make_shared<ThreeAddressNode>(nodeName, lhs, rhs, op, nodeType);
    }

    const string &getNodeName() const {
        return nodeName;
    }

    void setNodeName(const string &nodeName) {
        ThreeAddressNode::nodeName = nodeName;
    }

    NodeType getNodeType() const {
        return nodeType;
    }

    void setNodeType(NodeType nodeType) {
        ThreeAddressNode::nodeType = nodeType;
    }

    const ThreeAddressNodePtr &getRhs() const {
        return rhs;
    }

    const ThreeAddressNodePtr &getLhs() const {
        return lhs;
    }

    void setLhs(ThreeAddressNodePtr newPtr) {
        this->lhs = newPtr;
        int newNumber = (this->rhs == nullptr)? newPtr->numberOfChild : newPtr->numberOfChild + rhs->numberOfChild;
        if(newNumber != this->numberOfChild)
            this->numberOfChild = newNumber;
    }

    void setRhs(ThreeAddressNodePtr newPtr) {
        this->rhs = newPtr;
        int newNumber = newPtr->numberOfChild + lhs->numberOfChild;
        if(newNumber != this->numberOfChild)
            this->numberOfChild = newNumber;
    }

    ASTNode::Operator getOp() const {
        return op;
    }

    void setOp(ASTNode::Operator newOp) {
        this->op = newOp;
    }

    static ThreeAddressNodePtr copyNode(ThreeAddressNodePtr node, map<string, ThreeAddressNodePtr>& saved) {
        if(!node)
            return nullptr;
        if(saved.count(node->getNodeName()) != 0)
            return saved[node->getNodeName()];

        ThreeAddressNodePtr newNode = nullptr;
        if(node->getNodeType() == NodeType::UINT){
            newNode = make_shared<ThreeAddressNode>(node);
        }
        else if(node->getNodeType() == NodeType::PARAMETER) {
            newNode = make_shared<ThreeAddressNode>(node);
        }
        else if(node->getNodeType() == NodeType::FUNCTION) {
            newNode = make_shared<ThreeAddressNode>(node);
        }
        else if (node->getNodeType() == NodeType::UINT or node->getNodeType() == NodeType::UINT1 or
                node->getNodeType() == NodeType::UINT4 or node->getNodeType() == NodeType::UINT6 or
                node->getNodeType() == NodeType::UINT8 or node->getNodeType() == NodeType::UINT12 or
                node->getNodeType() == NodeType::UINT16 or node->getNodeType() == NodeType::UINT32 or
                node->getNodeType() == NodeType::UINT64 or node->getNodeType() == NodeType::UINT128 or
                node->getNodeType() == NodeType::UINT256) {
            newNode = make_shared<ThreeAddressNode>(node);
        }
        else {
            assert(false);
        }
        saved[node->getNodeName()] = newNode;

        return newNode;
    }

    static ThreeAddressNodePtr copyNodeWithPath(string sufix, ThreeAddressNodePtr node, map<string, ThreeAddressNodePtr>& saved, map<string, ThreeAddressNodePtr>& formalToActual) {
        if(!node)
            return nullptr;
        if(saved.count(node->getNodeName()) != 0 && node->getNodeType() != NodeType::PARAMETER)
            return saved[node->getNodeName()];

        ThreeAddressNodePtr newNode = nullptr;
        if(node->getNodeType() == NodeType::PARAMETER) {
            assert(formalToActual.count(node->getNodeName()) != 0);
            newNode = formalToActual[node->getNodeName()];
        }
        else if(node->getNodeType() == NodeType::FUNCTION) {
            newNode = make_shared<ThreeAddressNode>(node->getNodeName() + sufix, node);
        }
        else {
            assert(false);
        }
        saved[node->getNodeName()] = newNode;

        return newNode;
    }



    string printNodeType() {
        switch(this->getNodeType()) {
            case NodeType ::UINT:
                return "UINT";
            case NodeType ::UINT1:
                return "UINT1";
            case NodeType ::UINT4:
                return "UINT4";
            case NodeType ::UINT6:
                return "UINT6";
            case NodeType ::UINT8:
                return "UINT8";
            case NodeType ::UINT12:
                return "UINT12";
            case NodeType ::UINT16:
                return "UINT16";
            case NodeType ::UINT32:
                return "UINT32";
            case NodeType ::UINT64:
                return "UINT64";
            case NodeType ::UINT128:
                return "UINT128";
            case NodeType ::UINT256:
                return "UINT256";
            default:
                return "UINT";
        }
    }


    void prettyPrint() {
        if(nodeType == NodeType::UINT)
            return;

        if(lhs == nullptr && rhs == nullptr) {
            cout << getID() << "(" << printNodeType() << ")" << endl;
            return;
        }
        string result = getID() + " = " + lhs->getID();
        string oper;
        if(rhs != nullptr) {
            switch (op) {
                case ASTNode::Operator::ADD:
                    oper = "+";
                    break;
                case ASTNode::Operator::MINUS:
                    oper = "-";
                    break;
                case ASTNode::Operator::FFTIMES:
                    oper = "*";
                    break;
                case ASTNode::Operator::AND:
                    oper = "&";
                    break;
                case ASTNode::Operator::XOR:
                    oper = "^";
                    break;
                case ASTNode::Operator::OR:
                    oper = "|";
                    break;
                /*case ASTNode::Operator::LSL:
                    oper = "<<";
                    break;*/
                /*case ASTNode::Operator::NE:
                    oper = "!=";
                    break;*/
                case ASTNode::Operator ::RSH:
                    oper = ">>";
                    break;
                case ASTNode::Operator ::LSH:
                    oper = "<<";
                    break;
                /*case ASTNode::Operator ::POW2:
                    oper = "POW2";
                    break;
                case ASTNode::Operator ::POW4:
                    oper = "POW4";
                    break;
                case ASTNode::Operator ::POW16:
                    oper = "POW16";
                    break;
                case ASTNode::Operator ::AFFINE:
                    oper = "AFFINE";
                    break;*/
                default:
                    assert(false);
            }

            result += " " + oper + " " + rhs->getID() + "(" + printNodeType() + ")";
        } else {
            result += " " + oper;
        }

        cout << result << endl;
    }

    string prettyPrint4() {
        string result = getNodeName() + " = " + lhs->getNodeName();
        string oper;
        if(rhs != nullptr) {
            switch (op) {
                case ASTNode::Operator::ADD:
                    oper = "+";
                    break;
                case ASTNode::Operator::MINUS:
                    oper = "-";
                    break;
                case ASTNode::Operator::FFTIMES:
                    oper = "*";
                    break;
                case ASTNode::Operator::AND:
                    oper = "&";
                    break;
                case ASTNode::Operator::XOR:
                    oper = "^";
                    break;
                case ASTNode::Operator::OR:
                    oper = "|";
                    break;
                /*case ASTNode::Operator::LSL:
                    oper = "<<";
                    break;
                case ASTNode::Operator::NE:
                    oper = "!=";
                    break;*/
                case ASTNode::Operator ::RSH:
                    oper = ">>";
                    break;
                case ASTNode::Operator ::LSH:
                    oper = "<<";
                    break;
                /*case ASTNode::Operator ::POW2:
                    oper = "POW2";
                    break;
                case ASTNode::Operator ::POW4:
                    oper = "POW4";
                    break;
                case ASTNode::Operator ::POW16:
                    oper = "POW16";
                    break;
                case ASTNode::Operator ::AFFINE:
                    oper = "AFFINE";
                    break;*/
                default:
                    assert(false);
            }

            result += " " + oper + " " + rhs->getNodeName();
        } else {
            result += " " + oper;
        }

        return result + "\n";
    }

    string prettyPrint5() {

        if(nodeType == NodeType::FUNCTION) {
            return "CALL " + getNodeName() ;
        }
        if(!lhs)
            return getNodeName();
        string left =  lhs->prettyPrint5();
        string right = "";
        string result = "";
        string oper;

        switch (op) {
            case ASTNode::Operator::ADD:
                oper = "+";
                break;
            case ASTNode::Operator::MINUS:
                oper = "-";
                break;
            case ASTNode::Operator::FFTIMES:
                oper = "*";
                break;
            case ASTNode::Operator::AND:
                oper = "&";
                break;
            case ASTNode::Operator::XOR:
                oper = "^";
                break;
            case ASTNode::Operator::OR:
                oper = "|";
                break;
            /*case ASTNode::Operator::LSL:
                oper = "<<";
                break;
            case ASTNode::Operator::NE:
                oper = "!=";
                break;*/
            case ASTNode::Operator ::RSH:
                oper = ">>";
                break;
            case ASTNode::Operator ::LSH:
                oper = "<<";
                break;
            /*case ASTNode::Operator ::POW2:
                oper = "POW2";
                break;
            case ASTNode::Operator ::POW4:
                oper = "POW4";
                break;
            case ASTNode::Operator ::POW16:
                oper = "POW16";
                break;
            case ASTNode::Operator ::AFFINE:
                oper = "AFFINE";
                break;
            case ASTNode::Operator ::XTIMES:
                oper = "XTIMES";
                break;
            case ASTNode::Operator ::TRCON:
                oper = "TRCON";
                break;*/
            case ASTNode::Operator::NULLOP:
                oper = "";
                break;
            case ASTNode::Operator::CALL:
                oper = "[" + to_string(indexOfCall) + "](" + nodeName + ")";
                break;
            /*case ASTNode::Operator::TABLELUT:
                oper = "TABLUT";
                break;
            case ASTNode::Operator::POL:
                oper = "POL";
                break;
            case ASTNode::Operator::LIN:
                oper = "LIN";
                break;*/
            case ASTNode::Operator::NOT:
                oper = "~";
                break;
            /*case ASTNode::Operator::TABLELUT4:
                oper = "TABLUT4";
                break;*/
            // 新增两个操作符，TOUINT和BOXINDEX
            case ASTNode::Operator::TOUINT:
                oper = "touint";
                break;
            case ASTNode::Operator::BOXINDEX:
                oper = "boxindex";
                break;
            case ASTNode::Operator::BOXOP:
                oper = "boxop";
                break;
            case ASTNode::Operator::SYMBOLINDEX:
                oper = "symbolindex";
                break;
            default:
                assert(false);
        }

        if(rhs != nullptr) {
            right = rhs->prettyPrint5();
            result = "(" + left + " " + oper + " " + right + ")";
        } else {
            result = "(" + oper + " " + left + ")";
        }

        return result;
    }

    void setNumberOfChild(int n) {
        this->numberOfChild = n;
    }

    int getNumberOfChild() {
        return this->numberOfChild;
    }

    string prettyPrint2() {
        if(this->getOp() == ASTNode::Operator ::NOT)
            return  "!" + this->getLhs()->prettyPrint2();
        else if(this->getOp() == ASTNode::Operator ::ADD)
            return "(" + this->getLhs()->prettyPrint2()+ "+" + this->getRhs()->prettyPrint2() + ")";
        else if(this->getOp() == ASTNode::Operator ::XOR)
            return "(" + this->getLhs()->prettyPrint2() + "^" + this->getRhs()->prettyPrint2() + ")";
        else if(this->getOp() == ASTNode::Operator ::FFTIMES)
            return "(" + this->getLhs()->prettyPrint2() + "*" + this->getRhs()->prettyPrint2() + ")";
        else if(this->getOp() == ASTNode::Operator ::AND)
            return "(" + this->getLhs()->prettyPrint2() + "&" + this->getRhs()->prettyPrint2() + ")";
        /*else if(this->getOp() == ASTNode::Operator ::POW2)
            return "(POW2 " + this->getLhs()->prettyPrint2() + ")";
        else if(this->getOp() == ASTNode::Operator ::POW4)
            return "(POW4 " + this->getLhs()->prettyPrint2() + ")";
        else if(this->getOp() == ASTNode::Operator ::POW16)
            return "(POW16 " + this->getLhs()->prettyPrint2() + ")";
        else if(this->getOp() == ASTNode::Operator ::AFFINE)
            return "AFFINE" + this->getLhs()->prettyPrint2();
        else if(this->getOp() == ASTNode::Operator ::SBOX)
            return "Sbox" + this->getLhs()->prettyPrint2();
        else if(this->getOp() == ASTNode::Operator ::TABLELUT)
            return "TableLUT" + this->getLhs()->prettyPrint2();
        else if(this->getOp() == ASTNode::Operator ::TABLELUT4)
            return "TableLUT4" + this->getLhs()->prettyPrint2();
        else if(this->getOp() == ASTNode::Operator ::TRCON)
            return "RCON" + this->getLhs()->prettyPrint2();*/
        /*else if(this->getOp() == ASTNode::Operator ::XTIMES)
            return "XTIMES" + this->getLhs()->prettyPrint2();*/

        else
            assert(false);
    }

    int getSsaIndex() const {
        return SSAIndex;
    }

    void setSsaIndex(int ssaIndex) {
        SSAIndex = ssaIndex;
    }



    string getID() {
        if(lineNumber == 0) {
            return nodeName + "_" + to_string(SSAIndex);
        }
        else {
            return nodeName + "_" + to_string(SSAIndex) + "_" + to_string(lineNumber);
        }
    }

    string getIDWithOutLineNumber() {
        return nodeName + "_" + to_string(SSAIndex);
    }

    int getLineNumber() const {
        return lineNumber;
    }

    void setLineNumber(int lineNumber) {
        ThreeAddressNode::lineNumber = lineNumber;
    }

    void addParents(ThreeAddressNodePtr ptr) {
        this->parents.push_back(ptr);
    }

    /*void setParents(vector<ThreeAddressNodeWeakPtr> parents) {
        this->parents = parents;
    }*/

    /*vector<ThreeAddressNodeWeakPtr>& getParents() {
        return this->parents;
    }*/


    void replaceChild(ThreeAddressNodePtr A, ThreeAddressNodePtr B) {

        if(lhs == A) {
            lhs = B;
        } else if(rhs == A) {
            rhs = B;
        }
    }


    set<ThreeAddressNodePtr > randoms;
    set<ThreeAddressNodePtr > keys;

    void setIndexCall(int i) {
        indexOfCall = i;
    }

    int getIndexCall() {
        return indexOfCall;
    }

    bool operator<(const ThreeAddressNode objNode) const
    {
        return objNode.nodeName > nodeName;
    }

    static void getSupport(ThreeAddressNodePtr node, set<ThreeAddressNodePtr>& res) {
        if(!node)
            return;
        /*if(node->getNodeType() == NodeType::INTERNAL) {
            getSupport(node->getLhs(), res);
            getSupport(node->getRhs(), res);
        }*/
        else {
            res.insert(node);
        }
    }


    // used for interpreter
    void setThreeAddressNodeValue(int value) {
        this->value = value;
    }
};

#endif //PLBENCH_THREEADDRESSNODE_H
