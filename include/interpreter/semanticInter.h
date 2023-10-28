//
// Created by Septi on 9/18/2023.
//

#ifndef EASYBC_SEMANTICINTER_H
#define EASYBC_SEMANTICINTER_H

#include "ProcedureH.h"
#include <utility>
#include <cmath>
#include <util/utilities.h>
#include "Transformer.h"
#include "Interpreter.h"
#include "Reduction.h"
#include "SyntaxGuided.h"
#include "BranchNum.h"
#include "SboxModel.h"

class EasyBCInter {

private:
    std::string cipherName;
    vector<ProcedureHPtr> procedureHs;
    std::vector<int> plaintext;
    std::vector<int> ciphertext;
    std::vector<std::vector<int>> subkeys;
    std::map<std::string, std::vector<int>> Box;
    int sboxInputSize, sboxOutputSize;

    std::map<std::string, int> mainTanNameMVal;

    std::map<std::string, int> rndTanNameMVal;
    std::map<std::string, std::vector<int>> rndParamMVal;
    std::map<std::string, std::string> pboxTanNameMStr;
    std::map<std::string, std::vector<int>> matVecProTanNameMStr;

public:
    EasyBCInter(std::vector<ProcedureHPtr> procedureHs, std::vector<int> plaintext, std::vector<std::vector<int>> subkeys);

    std::vector<int> getCiphertext() {return this->ciphertext;};

    void interpreter();

    std::vector<int> roundFuncEnc(const ProcedureHPtr& procedureH);

    int xorEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output);
    int andEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output);
    int orEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output);

    int sboxEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output);
    int pboxEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output);

    int ffTimesEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output);

    int symbolIndexEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output);

    int touintEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output);

    std::vector<int> modularAddEnc(std::vector<ThreeAddressNodePtr> input1, std::vector<ThreeAddressNodePtr> input2, std::vector<ThreeAddressNodePtr> output);

    std::vector<int> modularMinusEnc(std::vector<ThreeAddressNodePtr> input1, std::vector<ThreeAddressNodePtr> input2, std::vector<ThreeAddressNodePtr> output);

    int minusEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output);

    int modeEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output);

    int notEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr output);

    int desSbox(std::string inputStr) {
        int row, col;
        std::string rowStr, colStr;
        rowStr = inputStr.substr(inputStr.size()-1,1) + inputStr.substr(0,1);
        colStr = inputStr.substr(4,1) + inputStr.substr(3,1) + inputStr.substr(2,1) + inputStr.substr(1,1);
        row = utilities::b_to_d(stoi(rowStr));
        col = utilities::b_to_d(stoi(colStr));
        return row*16+col;
    }


    std::vector<int> extValueFromTOUINTorBOXINDEX(const ThreeAddressNodePtr& input) {
        std::vector<int> extVal;
        if (input->getOp() == ASTNode::TOUINT) {
            ThreeAddressNodePtr left = input->getLhs();
            while (left->getOp() == ASTNode::BOXINDEX) {
                if (this->rndTanNameMVal.find(left->getLhs()->getNodeName()) == this->rndTanNameMVal.end()) {
                    if (left->getLhs()->getOp() == ASTNode::SYMBOLINDEX) {
                        this->rndTanNameMVal[left->getLhs()->getNodeName()] =
                                symbolIndexEnc(left->getLhs()->getLhs(), left->getLhs()->getRhs(), left->getLhs());
                    } else if (left->getLhs()->getOp() == ASTNode::NULLOP) {
                        this->rndTanNameMVal[left->getLhs()->getNodeName()] = std::stoi(left->getLhs()->getNodeName());
                    } else
                        assert(false);
                }
                extVal.push_back(this->rndTanNameMVal[left->getLhs()->getNodeName()]);
                left = left->getRhs();
            }
            if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
                if (left->getLhs()->getOp() == ASTNode::SYMBOLINDEX) {
                    this->rndTanNameMVal[left->getNodeName()] =
                            symbolIndexEnc(left->getLhs(), left->getRhs(), left);
                } else if (left->getOp() == ASTNode::NULLOP) {
                    this->rndTanNameMVal[left->getNodeName()] = std::stoi(left->getNodeName());
                } else
                    assert(false);
            }
            extVal.push_back(this->rndTanNameMVal[left->getNodeName()]);
        } else if (input->getOp() == ASTNode::BOXINDEX) {
            ThreeAddressNodePtr left = input;
            while (left->getOp() == ASTNode::BOXINDEX) {
                if (this->rndTanNameMVal.find(left->getLhs()->getNodeName()) == this->rndTanNameMVal.end()) {
                    if (left->getLhs()->getOp() == ASTNode::SYMBOLINDEX) {
                        this->rndTanNameMVal[left->getLhs()->getNodeName()] = symbolIndexEnc(left->getLhs()->getLhs(), left->getLhs()->getRhs(), left->getLhs());
                    } else if (left->getLhs()->getOp() == ASTNode::TOUINT) {
                        this->rndTanNameMVal[left->getLhs()->getNodeName()] = touintEnc(left->getLhs()->getLhs(), left->getLhs()->getRhs(), left->getLhs());
                    }
                    else if (left->getLhs()->getOp() == ASTNode::NULLOP) {
                        this->rndTanNameMVal[left->getLhs()->getNodeName()] = std::stoi(left->getLhs()->getNodeName());
                    } else
                        assert(false);
                }
                extVal.push_back(this->rndTanNameMVal[left->getLhs()->getNodeName()]);
                left = left->getRhs();
            }
            if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
                if (left->getOp() == ASTNode::SYMBOLINDEX) {
                    this->rndTanNameMVal[left->getNodeName()] =
                            symbolIndexEnc(left->getLhs(), left->getRhs(), left);
                } else if (left->getOp() == ASTNode::TOUINT) {
                    this->rndTanNameMVal[left->getNodeName()] = touintEnc(left->getLhs(), left->getRhs(), left);
                } else if (left->getOp() == ASTNode::NULLOP) {
                    this->rndTanNameMVal[left->getNodeName()] = std::stoi(left->getNodeName());
                } else
                    assert(false);
            }
            extVal.push_back(this->rndTanNameMVal[left->getNodeName()]);
        } else
            assert(false);
        return extVal;
    }

    static int getNodeTypeSize(NodeType type) {
        if (type == NodeType::UINT) {
            return 0;
        }
        else if (type == NodeType::UINT1) {
            return 1;
        }
        else if (type == NodeType::UINT4) {
            return 4;
        }
        else if (type == NodeType::UINT6) {
            return 6;
        }
        else if (type == NodeType::UINT8) {
            return 8;
        }
        else if (type == NodeType::UINT12) {
            return 12;
        }
        else if (type == NodeType::UINT16) {
            return 16;
        }
        else if (type == NodeType::UINT32) {
            return 32;
        }
        else if (type == NodeType::UINT64) {
            return 64;
        }
        else if (type == NodeType::UINT128) {
            return 128;
        }
        else if (type == NodeType::UINT256) {
            return 256;
        }
        return 0;
    }
};

#endif //EASYBC_SEMANTICINTER_H
