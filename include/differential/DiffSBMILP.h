//
// Created by Septi on 5/22/2023.
//

#ifndef EASYBC_DIFFSBMILP_H
#define EASYBC_DIFFSBMILP_H

#include "ProcedureH.h"
#include <utility>
#include <cmath>
#include <util/utilities.h>
#include "Transformer.h"
#include "Interpreter.h"
#include "Reduction.h"
#include "SyntaxGuided.h"
#include "BranchNum.h"
#include "DiffMILPcons.h"
#include "SboxModel.h"


class DiffSBMILP {

private:
    std::string cipherName;
    vector<ProcedureHPtr> procedureHs;

    std::string target;
    std::string mode;
    int redMode;
    int gurobiTimer = 3600 * 24;
    int gurobiThreads = 8;
    int startRound = 1;
    int endRound = 5;
    int currentRound;

    int xorConSel = 2;
    int sboxConSel = 1;
    int matrixConSel = 2;


    bool speedup1 = false;
    bool speedup2 = false;

    bool ILPFlag = false; // ILP or MILP ?


    vector<int> finalResults;
    vector<double> finalClockTime;
    vector<double> finalTimeTime;

    std::string pathPrefix = std::string(DPATH) + "differential/BitWiseMILP/";
    std::string pathSuffix;
    std::string modelPath;
    std::string resultsPath;
    std::string diffCharacteristicPath;
    std::string finalResultsPath;

    std::map<std::string, std::vector<int>> Box;
    std::map<std::string, std::vector<std::vector<int>>> sboxIneqs;
    std::vector<std::vector<int>> ARXineqs;

    std::vector<int> sboxExtWeighted;
    std::map<std::string, bool> sboxIfInjective;
    std::map<std::string, int> sboxInputSize;
    std::map<std::string, int> sboxOutputSize;
    std::map<std::string, int> branchNumMin;
    std::map<std::string, int> branchNumMax;

    int keySize = 160;
    int totalRoundNum = 80;
    float sboxMDP;
    int targetNdiffOrPr;
    int evaluationSize = 0;

    int blockSize;

    bool securityFlag = false;
    int securityRoundNumBound;

    int xCounter = 1;
    int dCounter = 1;
    int ACounter = 1;
    int PCounter = 1;
    int fCounter = 1;
    int yCounter = 1;

    int lastRoundACounter = 1, lastRoundPCounter = 1;
    std::vector<std::vector<int>> allRoundACounters, allRoundPCounters;

    map<std::string, int> tanNameMxIndex;
    map<std::string, int> rtnMxIndex;
    std::vector<int> rtnIdxSave;

    map<std::string, int> sboxNameMxIndex;
    std::vector<int> sboxRtnIdxSave;

    std::vector<std::vector<int>> differentialCharacteristic;

    std::vector<std::string> constantTan;

    int rndParamR;
    map<std::string, int> consTanNameMxVal;

public:
    DiffSBMILP(std::vector<ProcedureHPtr> procedureHs, std::string target, std::string mode, int redMode);

    void setGurobiTimer(int timer) {this->gurobiTimer = timer;}
    void setGurobiThreads(int threads) {this->gurobiThreads = threads;}
    void setStartRound(int round) {this->startRound = round;}
    void setEndRound(int round) {this->endRound = round;}
    void setXorConSel(int select) {this->xorConSel = select;}
    void setSboxConSel(int select) {this->sboxConSel = select;}
    void setMatrixConSel(int select) {this->matrixConSel = select;}
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

    void sboxFunctionGenModel(const ProcedureHPtr& procedureH, std::vector<ThreeAddressNodePtr> input, std::vector<ThreeAddressNodePtr> output);

    void XORGenModel(const ThreeAddressNodePtr& left, const ThreeAddressNodePtr& right, const ThreeAddressNodePtr& result, bool ifSboxFuncCall);

    void ANDandORGenModel(const ThreeAddressNodePtr& left, const ThreeAddressNodePtr& right, const ThreeAddressNodePtr& result, bool ifSboxFuncCall);

    void SboxGenModel(const ThreeAddressNodePtr& sbox, const ThreeAddressNodePtr& input, const ThreeAddressNodePtr& output);

    void PboxGenModel(const ThreeAddressNodePtr& pbox, const ThreeAddressNodePtr& input, const ThreeAddressNodePtr& output);

    void MatrixVectorGenModel(const ThreeAddressNodePtr& pboxm, const ThreeAddressNodePtr& input, const std::vector<ThreeAddressNodePtr>& output);
    void multiXORGenModel12(std::vector<ThreeAddressNodePtr> input, ThreeAddressNodePtr result);
    void multiXORGenModel3(std::vector<ThreeAddressNodePtr> input, ThreeAddressNodePtr result);

    void ADDandMINUSGenModel(std::vector<ThreeAddressNodePtr>& input1, std::vector<ThreeAddressNodePtr>& input2,
                             const std::vector<ThreeAddressNodePtr>& output, int AddOrMinus, bool ifSboxFuncCall);


    void preprocess(std::map<std::string, std::vector<int>> pboxM, std::map<std::string, std::vector<int>> Ffm) {
        auto iterator = this->Box.begin();
        while (iterator != this->Box.end()) {
            if (iterator->first.substr(0, 4) == "sbox") {
                std::string sboxName = iterator->first;
                if (this->cipherName != "SKINNY_128_bitwise") {
                    SboxM sboxM(sboxName, iterator->second, this->mode);
                    this->sboxExtWeighted = sboxM.get_extWeighted();
                    this->sboxMDP = sboxM.getSboxMDP();
                    std::vector<std::vector<int>> ineq_set = Red::reduction(this->redMode, sboxM);
                    this->sboxIneqs[sboxName] = ineq_set;
                }
                this->sboxIfInjective[sboxName] = sboxInjectiveCheck(iterator->second);
                sboxSizeGet(sboxName, iterator->second);
            }
            this->branchNumMin[iterator->first] = Red::branch_num_of_sbox(iterator->second);
            iterator++;
        }

        SboxM sboxM(this->cipherName, this->mode);
        this->ARXineqs = Red::reduction(1, sboxM);

        auto iterM = pboxM.begin();
        while (iterM != pboxM.end()) {
            BranchN branchN(iterM->second, Ffm[iterM->first], "b");
            std::vector<int> branches = branchN.getBranchNum();
            this->branchNumMin[iterM->first] = branches[0];
            this->branchNumMax[iterM->first] = branches[1];
            iterM++;
        }

        this->pathSuffix = this->cipherName + "/" + this->mode + "/xor" + std::to_string(this->xorConSel) +
                           "_sbox" + std::to_string(this->sboxConSel) + "_matrix" + std::to_string(this->matrixConSel);
        switch (this->redMode) {
            case 1:
                this->pathSuffix += "_greedy_sun";
                break;
            case 2:
                this->pathSuffix += "_sub_milp";
                break;
            case 3:
                this->pathSuffix += "_convex_hull_tech";
                break;
            case 4:
                this->pathSuffix += "_logic_cond";
                break;
            case 5:
                this->pathSuffix += "_comb233";
                break;
            case 6:
                this->pathSuffix += "_superball";
                break;
            case 7:
                this->pathSuffix += "_cnf";
                break;
            default:
                this->pathSuffix += "_greedy_sun";
                break;
        }

        if (speedup1) this->pathSuffix += "_speedup1";
        if (speedup2) this->pathSuffix += "_speedup2";
        this->pathSuffix += "/";
    }

    static bool sboxInjectiveCheck(std::vector<int> sbox) {
        set<int> arr;
        pair<set<int>::iterator,bool> pr;
        for(auto it=sbox.begin();it<sbox.end();it++){
            pr = arr.insert(*it);
            if(!pr.second){
                return false;
            }
        }
        return true;
    }

    void sboxSizeGet(std::string name, std::vector<int> sbox) {
        sboxInputSize[name] = int(log2(sbox.size()));
        map<int, int> sboxMap;
        for (int & i : sbox) {
            sboxMap[i]++;
        }
        sboxOutputSize[name] = int(log2(sboxMap.size()));
    }

    bool isConstant(ThreeAddressNodePtr threeAddressNodePtr) {
        ThreeAddressNodePtr left = threeAddressNodePtr->getLhs();
        ThreeAddressNodePtr right = threeAddressNodePtr->getRhs();

        if (left and std::find(this->constantTan.begin(), this->constantTan.end(), left->getNodeName()) != this->constantTan.end()) {
            constantTan.push_back(threeAddressNodePtr->getNodeName());
            return true;
        }
        if (right and std::find(this->constantTan.begin(), this->constantTan.end(), right->getNodeName()) != this->constantTan.end()) {
            constantTan.push_back(threeAddressNodePtr->getNodeName());
            return true;
        }

        if (threeAddressNodePtr->getNodeName().find("_symbol_", 0) != std::string::npos) {
            this->constantTan.push_back(threeAddressNodePtr->getNodeName());
            return true;
        }

        int counter = 0;
        for (auto c : threeAddressNodePtr->getNodeName()) {
            if (48 <= c and c <= 57) {
                counter++;
            }
        }
        if (counter == threeAddressNodePtr->getNodeName().size()) {
            this->constantTan.push_back(threeAddressNodePtr->getNodeName());
            return true;
        }

        if (left) {
            if ((threeAddressNodePtr->getOp() == ASTNode::SYMBOLINDEX and left->getNodeType() == NodeType::PARAMETER)) {
                this->constantTan.push_back(threeAddressNodePtr->getNodeName());
                return true;
            }
        }

        return false;
    }

    std::vector<int> extIdxFromTOUINTorBOXINDEX(const ThreeAddressNodePtr& input) {
        std::vector<int> extIdx;
        if (input->getOp() == ASTNode::TOUINT) {
            ThreeAddressNodePtr left = input->getLhs();
            while (left->getOp() == ASTNode::BOXINDEX) {
                if (tanNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                    extIdx.push_back(tanNameMxIndex.find(left->getLhs()->getNodeName())->second);
                    left = left->getRhs();
                }
                else if (!rtnIdxSave.empty()) {
                    tanNameMxIndex[left->getLhs()->getNodeName()] = rtnIdxSave.front();
                    extIdx.push_back(rtnIdxSave.front());
                    rtnIdxSave.erase(rtnIdxSave.cbegin());
                    left = left->getRhs();
                } else {
                    tanNameMxIndex[left->getLhs()->getNodeName()] = xCounter;
                    extIdx.push_back(xCounter);
                    xCounter++;
                    left = left->getRhs();
                }
            }
            if (!rtnIdxSave.empty()) {
                tanNameMxIndex[left->getLhs()->getNodeName()] = rtnIdxSave.front();
                extIdx.push_back(rtnIdxSave.front());
                rtnIdxSave.erase(rtnIdxSave.cbegin());
            } else if (tanNameMxIndex.count(left->getNodeName()) != 0) {
                extIdx.push_back(tanNameMxIndex.find(left->getNodeName())->second);
            } else {
                tanNameMxIndex[left->getLhs()->getNodeName()] = xCounter;
                extIdx.push_back(xCounter);
                xCounter++;
            }
        } else if (input->getOp() == ASTNode::BOXINDEX) {
            ThreeAddressNodePtr left = input;
            while (left->getOp() == ASTNode::BOXINDEX) {
                if (tanNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                    extIdx.push_back(tanNameMxIndex.find(left->getLhs()->getNodeName())->second);
                    left = left->getRhs();
                }
                else if (left->getLhs() != nullptr and left->getRhs() != nullptr) {
                    if (left->getLhs()->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getLhs()->getNodeType() != UINT1) {
                        if (tanNameMxIndex.count(left->getLhs()->getLhs()->getNodeName() + "_$B$_" +
                                                 left->getLhs()->getRhs()->getNodeName()) != 0) {
                            extIdx.push_back(tanNameMxIndex.find(left->getLhs()->getLhs()->getNodeName() + "_$B$_" +
                                                                 left->getLhs()->getRhs()->getNodeName())->second);
                            tanNameMxIndex[left->getNodeName()] = tanNameMxIndex.find(left->getLhs()->getLhs()->getNodeName() + "_$B$_" +
                                                                                      left->getLhs()->getRhs()->getNodeName())->second;
                            left = left->getRhs();
                        } else
                            assert(false);
                    }
                }
                else if (!rtnIdxSave.empty()) {
                    tanNameMxIndex[left->getLhs()->getNodeName()] = rtnIdxSave.front();
                    extIdx.push_back(rtnIdxSave.front());
                    rtnIdxSave.erase(rtnIdxSave.cbegin());
                    left = left->getRhs();
                } else {
                    tanNameMxIndex[left->getLhs()->getNodeName()] = xCounter;
                    extIdx.push_back(xCounter);
                    xCounter++;
                    left = left->getRhs();
                }
            }
            if (!rtnIdxSave.empty()) {
                tanNameMxIndex[left->getNodeName()] = rtnIdxSave.front();
                extIdx.push_back(rtnIdxSave.front());
                rtnIdxSave.erase(rtnIdxSave.cbegin());
            } else if (tanNameMxIndex.count(left->getNodeName()) != 0) {
                extIdx.push_back(tanNameMxIndex.find(left->getNodeName())->second);
            } else {
                tanNameMxIndex[left->getNodeName()] = xCounter;
                extIdx.push_back(xCounter);
                xCounter++;
            }
        } else assert(false);
        return extIdx;
    }

    static std::vector<int> shiftVec(std::vector<int> input, int shiftNum) {
        std::vector<int> rtn;
        for (int i = shiftNum; i < input.size(); ++i) {
            rtn.push_back(input[i]);
        }
        for (int i = rtn.size(); i < input.size(); ++i) {
            rtn.push_back(0);
        }
        return rtn;
    }

    static int transNodeTypeSize(NodeType nodeType) {
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
        return 0;
    }

};


#endif //EASYBC_DIFFSBMILP_H






























