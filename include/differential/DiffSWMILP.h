//
// Created by Septi on 6/6/2023.
//

#ifndef EASYBC_DIFFSWMILP_H
#define EASYBC_DIFFSWMILP_H

#include "ProcedureH.h"
#include <utility>
#include <utilities.h>
#include "Transformer.h"
#include "SboxModel.h"
#include "gurobi_c++.h"
#include "Reduction.h"
#include "BranchNum.h"
#include "DiffMILPcons.h"


class DiffSWMILP {

private:
    std::string cipherName;
    std::vector<ProcedureHPtr> procedureHs;

    std::string target;
    int gurobiTimer = 3600 * 12;
    int gurobiThreads = 8;
    int startRound = 1;
    int endRound = 5;
    int currentRound;

    int xorConSel = 1; // 1-2
    int linearConSel = 1; // 1-2

    bool speedup1 = false;
    bool speedup2 = false;

    bool ILPFlag = false; // ILP or MILP ?

    vector<int> finalResults;
    vector<double> finalClockTime;
    vector<double> finalTimeTime;

    std::string pathPrefix = std::string(DPATH) + "differential/WordWiseMILP/";
    std::string pathSuffix;
    std::string modelPath;
    std::string resultsPath;
    std::string diffCharacteristicPath;
    std::string finalResultsPath;

    std::map<std::string, std::vector<int>> Box;
    std::map<std::string, int> branchNumMin;
    std::map<std::string, int> branchNumMax;

    int keySize;
    int totalRoundNum;
    float sboxMDP;
    int targetNdiff;
    int blockSize;
    int wordSize;
    int evaluationSize = 0;

    bool securityFlag = false;
    int securityRoundNumBound;

    int dCounter = 1;
    int ACounter = 1;

    std::map<std::string, int> tanNameMxIndex;
    std::map<std::string, int> rtnMxIndex;
    std::vector<int> rtnIdxSave;
    bool sboxFindFlag = false;
    std::vector<int> sboxIndexSave;
    std::vector<std::vector<int>> roundSboxIndexSave;

    std::string roundID;

    std::vector<std::vector<int>> differentialCharacteristic;

public:

    DiffSWMILP(const vector<ProcedureHPtr>& procedureHs, std::string target);

    void setGurobiTimer(int timer) {this->gurobiTimer = timer;}
    void setGurobiThreads(int threads) {this->gurobiThreads = threads;}
    void setStartRound(int round) {this->startRound = round;}
    void setEndRound(int round) {this->endRound = round;}
    void setXorConSel(int select) {this->xorConSel = select;}
    void setLinearConSel(int select) {this->linearConSel = select;}
    void setSpeedUp1() {this->speedup1 = true;}
    void setSpeedUp2() {this->speedup2 = true;}
    void setILP() {this->ILPFlag = true;}
    void setTotalRoundNum(int num) {this->totalRoundNum = num;}
    void setEvaluationSize(int evaluationSize) {this->evaluationSize = evaluationSize;}

    void MGR();

    void cryptanalysis();

    void evaluation();

    void solver();

    void programGenModel();

    void roundFunctionGenModel(const ProcedureHPtr& procedureH);

    void XORGenModel(const ThreeAddressNodePtr& left, const ThreeAddressNodePtr& right, const ThreeAddressNodePtr& result);

    void ANDandORGenModel(const ThreeAddressNodePtr& left, const ThreeAddressNodePtr& right, const ThreeAddressNodePtr& result);

    void PboxGenModel(const ThreeAddressNodePtr& pbox, const ThreeAddressNodePtr& input, const ThreeAddressNodePtr& output);

    void linearPermuteGenModel(const ThreeAddressNodePtr& pbox, const ThreeAddressNodePtr& input, const std::vector<ThreeAddressNodePtr>& output);

    void AddGenModel(std::vector<ThreeAddressNodePtr>& input1, std::vector<ThreeAddressNodePtr>& input2, const std::vector<ThreeAddressNodePtr>& output);

    void preprocess(std::map<std::string, std::vector<int>> pboxM, std::map<std::string, std::vector<int>> Ffm) {
        auto iterator = Box.begin();
        while (iterator != Box.end()) {
            if (iterator->first.substr(0, 4) == "sbox") {
                std::string sboxName = iterator->first;
                if (iterator->second.size() <= 32) {
                    SboxM sboxM(sboxName, iterator->second, "AS");
                    this->sboxMDP = sboxM.getSboxMDP();
                } else if (this->cipherName == "AES_128_128") {
                    this->sboxMDP = 6;
                }
            }
            this->branchNumMin[iterator->first] = Red::branch_num_of_sbox(iterator->second);
            iterator++;
        }
        auto iterM = pboxM.begin();
        while (iterM != pboxM.end()) {
            BranchN branchN(iterM->second, Ffm[iterM->first], "w");
            std::vector<int> branches = branchN.getBranchNum();
            this->branchNumMin[iterM->first] = branches[0];
            this->branchNumMax[iterM->first] = branches[1];
            iterM++;
        }

        this->pathSuffix = this->cipherName + "/xor" + std::to_string(this->xorConSel) +
                           "_linear" + std::to_string(this->linearConSel);
        if (this->speedup1) this->pathSuffix += "_speedup1";
        if (this->speedup2) this->pathSuffix += "_speedup2";
        this->pathSuffix += "/";
    }

    bool isConstant(ThreeAddressNodePtr threeAddressNodePtr);

    std::vector<int> extIdxFromTOUINTorBOXINDEX(const ThreeAddressNodePtr& input) {
        std::vector<int> extIdx;
        if (input->getOp() == ASTNode::TOUINT) {
            ThreeAddressNodePtr left = input->getLhs();
            while (left->getOp() == ASTNode::BOXINDEX) {
                if (this->tanNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                    extIdx.push_back(this->tanNameMxIndex.find(left->getLhs()->getNodeName())->second);
                    left = left->getRhs();
                }
                else if (!this->rtnIdxSave.empty()) {
                    this->tanNameMxIndex[left->getLhs()->getNodeName()] = this->rtnIdxSave.front();
                    extIdx.push_back(this->rtnIdxSave.front());
                    this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                    left = left->getRhs();
                } else {
                    this->tanNameMxIndex[left->getLhs()->getNodeName()] = this->ACounter;
                    extIdx.push_back(this->ACounter);
                    this->ACounter++;
                    left = left->getRhs();
                }
            }
            if (!this->rtnIdxSave.empty()) {
                this->tanNameMxIndex[left->getLhs()->getNodeName()] = this->rtnIdxSave.front();
                extIdx.push_back(this->rtnIdxSave.front());
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
            } else if (this->tanNameMxIndex.count(left->getNodeName()) != 0) {
                extIdx.push_back(this->tanNameMxIndex.find(left->getNodeName())->second);
            } else {
                this->tanNameMxIndex[left->getLhs()->getNodeName()] = this->ACounter;
                extIdx.push_back(this->ACounter);
                this->ACounter++;
            }
        } else if (input->getOp() == ASTNode::BOXINDEX) {
            ThreeAddressNodePtr left = input;
            while (left->getOp() == ASTNode::BOXINDEX) {
                if (this->tanNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                    extIdx.push_back(this->tanNameMxIndex.find(left->getLhs()->getNodeName())->second);
                    left = left->getRhs();
                }
                else if (left->getLhs() != nullptr and left->getRhs() != nullptr) {
                    if (left->getLhs()->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getLhs()->getNodeType() != UINT1) {
                        if (this->tanNameMxIndex.count(left->getLhs()->getLhs()->getNodeName() + "_$B$_" +
                                                 left->getLhs()->getRhs()->getNodeName()) != 0) {
                            extIdx.push_back(this->tanNameMxIndex.find(left->getLhs()->getLhs()->getNodeName() + "_$B$_" +
                                                                 left->getLhs()->getRhs()->getNodeName())->second);
                            this->tanNameMxIndex[left->getNodeName()] = this->tanNameMxIndex.find(left->getLhs()->getLhs()->getNodeName() + "_$B$_" +
                                                                                      left->getLhs()->getRhs()->getNodeName())->second;
                            left = left->getRhs();
                        } else
                            assert(false);
                    }
                }
                else if (!this->rtnIdxSave.empty()) {
                    this->tanNameMxIndex[left->getLhs()->getNodeName()] = this->rtnIdxSave.front();
                    extIdx.push_back(this->rtnIdxSave.front());
                    this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                    left = left->getRhs();
                } else {
                    this->tanNameMxIndex[left->getLhs()->getNodeName()] = this->ACounter;
                    extIdx.push_back(this->ACounter);
                    this->ACounter++;
                    left = left->getRhs();
                }
            }
            if (!this->rtnIdxSave.empty()) {
                this->tanNameMxIndex[left->getNodeName()] = this->rtnIdxSave.front();
                extIdx.push_back(this->rtnIdxSave.front());
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
            } else if (this->tanNameMxIndex.count(left->getNodeName()) != 0) {
                extIdx.push_back(this->tanNameMxIndex.find(left->getNodeName())->second);
            } else {
                this->tanNameMxIndex[left->getNodeName()] = this->ACounter;
                extIdx.push_back(this->ACounter);
                this->ACounter++;
            }
        } else assert(false);
        return extIdx;
    }

};

#endif //EASYBC_DIFFSWMILP_H
