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

/*
 * Word-wise MILP used to calculate the minimal number of differential active S-boxes
 * */

class DiffSWMILP {

private:
    std::string cipherName;
    std::vector<ProcedureHPtr> procedureHs;


    // **************** setup ****************
    /*
     * "cryptanalysis" -> calculate minimal number of active S-boxes, or maximal differential characteristic probability
     * "evaluation" -> evaluate the security of block ciphers by number of active S-boxes, or differential characteristic probability
     * */
    std::string target;
    int gurobiTimer = 3600 * 12; // the timer of gurobi solver
    int gurobiThreads = 8; // the number of threads of gurobi solver
    int startRound = 1; // the start round of "cryptanalysis" or "evaluation"
    int endRound = 5; // the end round of "cryptanalysis" or "evaluation"
    int currentRound; // the current round of "cryptanalysis" or "evaluation"
    /*
     * the selections of constraints for XOR, S-boxes and matrix-vector product
     * */
    int xorConSel = 1; // 1-2
    int linearConSel = 1; // 1-2
    /*
     * speed up techniques
     * speedup1 -> the sum of input difference is larger than 1
     * speedup2 -> all sum of difference that fixed the searched results is no less than the current one which will be searched
     * */
    bool speedup1 = false;
    bool speedup2 = false;

    bool ILPFlag = false; // ILP or MILP ?


    // **************** results ****************
    /*
     * "cryptanalysis" -> minimal number of active S-boxes, or maximal differential characteristic probability of each round
     * "evaluation" -> number of active S-boxes, or differential characteristic probability of each round for evaluating the security
     * */
    vector<int> finalResults;
    // "cryptanalysis" or "evaluation" time of each round
    vector<double> finalClockTime;
    vector<double> finalTimeTime;


    // **************** analysis structure ****************
    std::string pathPrefix = std::string(DPATH) + "differential/WordWiseMILP/";
    std::string pathSuffix;
    std::string modelPath;
    std::string resultsPath;
    std::string diffCharacteristicPath; // save differential characteristics
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
    std::vector<int> sboxIndexSave;     // index of variable of S-boxes used for objective function
    std::vector<std::vector<int>> roundSboxIndexSave;     // speedup constraints auxiliary variables


    std::string roundID;

    std::vector<std::vector<int>> differentialCharacteristic;


public:

    DiffSWMILP(const vector<ProcedureHPtr>& procedureHs, std::string target);


    // parameters  setup
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
        // obtain branch number of all boxes
        auto iterator = Box.begin();
        while (iterator != Box.end()) {
            if (iterator->first.substr(0, 4) == "sbox") {
                std::string sboxName = iterator->first;
                // 只对5bit以及以下的sbox建模
                if (iterator->second.size() <= 32) {
                    SboxM sboxM(sboxName, iterator->second, "AS"); // word-wise 时需要的MDP通过AS mode来建模获取
                    // 获取sboxMDP
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

    // extract index form the ThreeAddressNode instance whose op is touint or boxindex
    std::vector<int> extIdxFromTOUINTorBOXINDEX(const ThreeAddressNodePtr& input) {
        std::vector<int> extIdx;
        if (input->getOp() == ASTNode::TOUINT) {
            ThreeAddressNodePtr left = input->getLhs();
            while (left->getOp() == ASTNode::BOXINDEX) {
                // Before the specific treatment, we need to add a judgment to  whether the input three-address instances
                // have been stored in the map of tanNameMxIndex.
                // If there is, there is no need to repeat the processing and directly end the operation of the function
                if (this->tanNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                    extIdx.push_back(this->tanNameMxIndex.find(left->getLhs()->getNodeName())->second);
                    left = left->getRhs();
                }
                    // If it is not the first round, i.e. rtnRecorder is not empty,
                    // the first element of rtnRecorder is taken each time as the ordinal number of x
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
                // tanNameMxIndex.count(left->getLhs()->getNodeName()) == 0说明该元素还没有对应的xCounter的map
                // 但是其可能是某个uints的某一位，而这个uints已经被map，所以需要寻找该uints的对应某一位的map
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
