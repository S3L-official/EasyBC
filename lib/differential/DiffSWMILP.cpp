//
// Created by Septi on 6/6/2023.
//

#include "differential/DiffSWMILP.h"

extern std::map<std::string, std::vector<int>> allBox;
extern map<std::string, std::vector<int>> pboxM;
extern std::map<std::string, int> pboxMSize;
extern map<std::string, std::vector<int>> Ffm;
extern std::string cipherName;


DiffSWMILP::DiffSWMILP(const vector<ProcedureHPtr> &procedureHs, std::string target)
        : procedureHs(procedureHs), target(move(target))  {
    this->Box = allBox;
    this->cipherName = ::cipherName;
}

void DiffSWMILP::MGR() {
    preprocess(pboxM, Ffm);

    this->finalResults.clear();
    this->finalClockTime.clear();
    this->finalTimeTime.clear();

    this->pathPrefix += target + "/";
    this->modelPath = this->pathPrefix + "models/" + this->pathSuffix;
    this->resultsPath = this->pathPrefix + "results/" + this->pathSuffix;
    this->diffCharacteristicPath = this->pathPrefix + "differentialCharacteristics/" + this->pathSuffix;
    system(("mkdir -p " + this->modelPath).c_str());
    system(("mkdir -p " + this->resultsPath).c_str());
    system(("mkdir -p " + this->diffCharacteristicPath).c_str());
    this->finalResultsPath = this->resultsPath + this->cipherName + "_FinalResults.txt";
    system(("touch " + this->finalResultsPath).c_str());

    for (int i = this->startRound; i <= endRound; ++i) {
        if (this->securityFlag)
            break;

        this->currentRound = i;
        std::cout << "\n **************** CURRENT MAX ROUOND : " << this->currentRound << "  **************** \n" << std::endl;
        this->modelPath = this->pathPrefix + "models/" + this->pathSuffix + std::to_string(this->currentRound) + "_round_model.lp";
        this->resultsPath = this->pathPrefix + "results/" + this->pathSuffix + std::to_string(this->currentRound) + "_round_results.txt";
        system(("touch " + this->modelPath).c_str());
        system(("touch " + this->resultsPath).c_str());
        this->diffCharacteristicPath = this->pathPrefix + "differentialCharacteristics/" + this->pathSuffix + std::to_string(this->currentRound) + "_round_differential_characteristic.txt";
        system(("touch " + this->diffCharacteristicPath).c_str());

        system(("rm " + this->modelPath).c_str());
        if (this->target == "cryptanalysis")
            cryptanalysis();
        else if (this->target == "evaluation")
            evaluation();
        else
            assert(false);

        vector<double> finalTotalRoundClockTime;
        vector<double> finalTotalRoundTimeTime;
        clock_t startTime, endTime;
        startTime = clock();
        time_t star_time = 0, end_time;
        star_time = time(NULL);
        solver();
        endTime = clock();
        end_time = time(NULL);
        double clockTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
        double timeTime = difftime( end_time, star_time);
        finalTotalRoundClockTime.push_back(clockTime);
        finalTotalRoundTimeTime.push_back(timeTime);

        this->tanNameMxIndex.clear();
        this->rtnMxIndex.clear();
        this->rtnIdxSave.clear();
        this->dCounter = 1;
        this->ACounter = 1;
        this->differentialCharacteristic.clear();
        this->roundSboxIndexSave.clear();
        this->sboxFindFlag = false;
        this->sboxIndexSave.clear();
        std::ofstream fResults(this->finalResultsPath, std::ios::trunc);
        if (!fResults){
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            if (this->target == "cryptanalysis") {
                fResults << " ***************** FINAL RESULTS ***************** \n\n";
                for (int j = 0; j < finalResults.size(); ++j) {
                    fResults << "Results Of " << j + 1 << " Rounds : \n";
                    fResults << "\tObj : " << finalResults[j] << "\n";
                    fResults << "\tclockTime : " << finalClockTime[j] << "\n";
                    fResults << "\ttimeTime : " << finalTimeTime[j] << "\n\n";
                }
                fResults << " ***************** FINAL RESULTS ***************** \n";
            } else if (this->target == "evaluation") {
                fResults << "-------------------------- FINAL RESULTS -------------------------- \n\n";
                for (int j = 0; j <= this->finalResults.size(); ++j) {
                    if (j == this->securityRoundNumBound - 1) {
                        fResults << "\n****************************************************************\n";
                        fResults << "the block cipher is security enough when the round number is : " << j + 1 << "\n";
                        fResults << "****************************************************************\n\n\n\n";

                        fResults << "***************** Results Of " << j + 1 << " Rounds *****************\n";
                        // fResults << "\tnumber of active sboxes for security bounding : " << this->finalResults[j] << "\n";
                        fResults << "\tnumber of target active sboxes for security bounding : " << this->targetNdiff << "\n";
                        fResults << "\tclockTime : " << this->finalClockTime[j] << "\n";
                        fResults << "\ttimeTime : " << this->finalTimeTime[j] << "\n\n";
                        // 加上不可解模型求解时间的所有时间
                        fResults << "\ttotalRoundClockTime : " << finalTotalRoundClockTime[j] << "\n";
                        fResults << "\ttotalRoundTimeTime : " << finalTotalRoundTimeTime[j] << "\n\n\n";
                    } else {
                        fResults << "***************** Results Of " << j + 1 << " Rounds *****************\n";
                        fResults << "\tnumber of active sboxes : " << this->finalResults[j] << "\n";
                        fResults << "\tclockTime : " << this->finalClockTime[j] << "\n";
                        fResults << "\ttimeTime : " << this->finalTimeTime[j] << "\n\n";
                        // 加上不可解模型求解时间的所有时间
                        fResults << "\ttotalRoundClockTime : " << finalTotalRoundClockTime[j] << "\n";
                        fResults << "\ttotalRoundTimeTime : " << finalTotalRoundTimeTime[j] << "\n\n\n";
                    }
                }
                fResults << " -------------------------- FINAL RESULTS -------------------------- \n";
            }
            fResults.close();
        }
    }
    if (target == "cryptanalysis") {
        std::cout << "\n ***************** FINAL RESULTS ***************** " << std::endl;
        for (int i = 0; i < this->finalResults.size(); ++i) {
            std::cout << "Results Of " << i + 1 << " Rounds : " << std::endl;
            std::cout << "\tObj : " << this->finalResults[i] << std::endl;
            std::cout << "\tclockTime : " << this->finalClockTime[i] << std::endl;
            std::cout << "\ttimeTime : " << this->finalTimeTime[i] << std::endl << std::endl;
        }
        std::cout << " ***************** FINAL RESULTS ***************** \n" << std::endl;
    } else if (target == "evaluation") {
        std::cout << "\n ***************** FINAL RESULTS ***************** " << std::endl;
        for (int i = 0; i < this->finalResults.size(); ++i) {
            if (i == this->securityRoundNumBound - 1) {
                std::cout << "\n\n****************************************************************" << std::endl;
                std::cout << "the block cipher is security enough when the round number is : " << i + 1 << std::endl;
                std::cout << "****************************************************************\n\n\n" << std::endl;

                std::cout << "Results Of " << i + 1 << " Rounds : " << std::endl;
                std::cout << "\tnumber of target active sboxes for security bounding : " << this->targetNdiff << "\n";
                std::cout << "\tclockTime : " << this->finalClockTime[i] << std::endl;
                std::cout << "\ttimeTime : " << this->finalTimeTime[i] << std::endl << std::endl;
            } else {
                std::cout << "Results Of " << i + 1 << " Rounds : " << std::endl;
                std::cout << "\tnumber of active sboxes : " << this->finalResults[i] << std::endl;
                std::cout << "\tclockTime : " << this->finalClockTime[i] << std::endl;
                std::cout << "\ttimeTime : " << this->finalTimeTime[i] << std::endl << std::endl;
            }
        }
        std::cout << " ***************** FINAL RESULTS ***************** \n" << std::endl;
    }
}


void DiffSWMILP::cryptanalysis() {
    programGenModel();

    std::ifstream file;
    file.open(this->modelPath);
    std::string model,line;
    while (getline(file, line)) {model += line + "\n";}
    file.close();

    if (model != "") {
        std::ofstream target(this->modelPath, std::ios::trunc);
        if (!target){
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            target << "Minimize\n";
            for (int i = 0; i < sboxIndexSave.size(); ++i) {
                target << "A" << sboxIndexSave[i];
                if (i != sboxIndexSave.size() - 1) {
                    target << " + ";
                }
            }
            target << "\n" << "Subject To\n";

            if (this->speedup1) {
                for (int j = 0; j < this->differentialCharacteristic.size() - 1; ++j) {
                    for (int k = 0; k < this->differentialCharacteristic[j].size(); ++k) {
                        if (k != this->differentialCharacteristic[j].size() - 1)
                            target << "A" << std::to_string(this->differentialCharacteristic[j][k]) << " + ";
                        else
                            target << "A" << std::to_string(this->differentialCharacteristic[j][k]);
                    }
                    if (this->differentialCharacteristic[j].size() > 0) target << " >= 1\n";
                }
            } else {
                for (int i = 0; i < this->blockSize; ++i) {
                    if (i != this->blockSize - 1)
                        target << "A" << std::to_string(i + 1) << " + ";
                    else
                        target << "A" << std::to_string(i + 1);
                }
                target << " >= 1\n";
            }

            if (this->speedup2) {

                for (int i = 0; i < this->roundSboxIndexSave.size(); ++i) {
                    for (int j = this->roundSboxIndexSave.size() - i - 1; j < this->roundSboxIndexSave.size(); ++j) {
                        for (int k = 0; k < this->roundSboxIndexSave[j].size(); ++k) {
                            target << "A" << this->roundSboxIndexSave[j][k];
                            if (k != this->roundSboxIndexSave[j].size() - 1) target << " + ";
                        }
                        if (j != this->roundSboxIndexSave.size() - 1) target << " + ";
                    }
                    target << " - " << this->finalResults[i] << " >= 0\n";
                }

                for (int i = 0; i < this->roundSboxIndexSave.size(); ++i) {
                    for (int j = 0; j < i + 1; ++j) {
                        for (int k = 0; k < this->roundSboxIndexSave[j].size(); ++k) {
                            target << "A" << this->roundSboxIndexSave[j][k];
                            if (k != this->roundSboxIndexSave[j].size() - 1) target << " + ";
                        }
                        if (j != i) target << " + ";
                    }
                    target << " - " << this->finalResults[i] << " >= 0\n";
                }
            }

            target << model;
            target.close();
        }
    }
    std::ofstream binary(this->modelPath, std::ios::app);
    if (!binary){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        binary << "Binary\n";
        for (int i = 1; i < this->dCounter; ++i)
            binary << "d" << i << "\n";
        for (int i = 1; i < this->ACounter; ++i)
            binary << "A" << i << "\n";
        binary << "End";
        binary.close();
    }
}


void DiffSWMILP::evaluation() {
    if (this->evaluationSize == 0)
        this->evaluationSize = this->blockSize;
    this->targetNdiff = int(ceil(this->currentRound * ((this->evaluationSize / this->sboxMDP) / this->totalRoundNum))) - 1;

    programGenModel();

    std::ifstream file;
    file.open(this->modelPath);
    std::string model,line;
    while (getline(file, line)) {model += line + "\n";}
    file.close();

    if (model != "") {
        std::ofstream target(this->modelPath, std::ios::trunc);
        if (!target){
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            target << "Subject To\n";
            for (int i = 0; i < sboxIndexSave.size(); ++i) {
                target << "A" << sboxIndexSave[i];
                if (i != sboxIndexSave.size() - 1) {
                    target << " + ";
                }
            }
            target << " <= " << this->targetNdiff << "\n";

            if (this->speedup1) {
                for (int j = 0; j < this->differentialCharacteristic.size() - 1; ++j) {
                    for (int k = 0; k < this->differentialCharacteristic[j].size(); ++k) {
                        if (k != this->differentialCharacteristic[j].size() - 1)
                            target << "A" << std::to_string(this->differentialCharacteristic[j][k]) << " + ";
                        else
                            target << "A" << std::to_string(this->differentialCharacteristic[j][k]);
                    }
                    if (this->differentialCharacteristic[j].size() > 0) target << " >= 1\n";
                }
            } else {
                for (int i = 0; i < this->blockSize; ++i) {
                    if (i != this->blockSize - 1)
                        target << "A" << std::to_string(i + 1) << " + ";
                    else
                        target << "A" << std::to_string(i + 1);
                }
                target << " >= 1\n";
            }

            if (this->speedup2) {
                for (int i = 0; i < this->finalResults.size(); ++i) {
                    for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                        for (int k = 0; k < this->roundSboxIndexSave[j].size(); ++k) {
                            target << "A" << this->roundSboxIndexSave[j][k];
                            if (k != this->roundSboxIndexSave[j].size() - 1) target << " + ";
                        }
                        if (j != this->currentRound - 1) target << " + ";
                    }
                    target << " - " << this->finalResults[i] << " >= 0\n";
                }

                for (int i = 0; i < this->finalResults.size(); ++i) {
                    for (int j = 0; j < i + 1; ++j) {
                        for (int k = 0; k < this->roundSboxIndexSave[j].size(); ++k) {
                            target << "A" << this->roundSboxIndexSave[j][k];
                            if (k != this->roundSboxIndexSave[j].size() - 1) target << " + ";
                        }
                        if (j != i) target << " + ";
                    }
                    target << " - " << this->finalResults[i] << " >= 0\n";
                }
            }

            target << model;
            target.close();
        }
    }
    std::ofstream binary(this->modelPath, std::ios::app);
    if (!binary){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        binary << "Binary\n";
        for (int i = 1; i < dCounter; ++i)
            binary << "d" << i << "\n";
        for (int i = 1; i < ACounter; ++i)
            binary << "A" << i << "\n";
        binary << "End";
        binary.close();
    }
}


void DiffSWMILP::solver() {
    clock_t startTime, endTime;
    startTime = clock();
    time_t star_time = 0, end_time;
    star_time = time(NULL);
    GRBEnv env = GRBEnv(true);
    env.start();

    env.set(GRB_IntParam_Threads, this->gurobiThreads);
    env.set(GRB_IntParam_MIPFocus, 3);
    env.start();
    GRBModel model = GRBModel(env, modelPath);
    model.set(GRB_IntParam_MIPFocus, 2);
    model.set(GRB_DoubleParam_TimeLimit, this->gurobiTimer);
    model.optimize();
    endTime = clock();
    end_time = time(NULL);

    int optimstatus = model.get(GRB_IntAttr_Status);
    int result = 0;
    double clockTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    double timeTime = difftime( end_time, star_time);

    if (this->target == "cryptanalysis") {
        if (optimstatus == 2) {
            result = model.get(GRB_DoubleAttr_ObjVal);

            std::ofstream f(this->resultsPath, std::ios::trunc);
            f << "solving the MILP " << this->modelPath << "\n";
            f << "obj is : " << result << "\n";
            f << "clock time : " << clockTime << "s\n";
            f << "time time : " << timeTime << "s\n";

            map<std::string, int> dcVarMaps;
            GRBVar *a = model.getVars();
            int varNum = model.get(GRB_IntAttr_NumVars);
            for (int i = 0; i < varNum; ++i) {
                f << a[i].get(GRB_StringAttr_VarName) << " = " << a[i].get(GRB_DoubleAttr_X) << "\n";
                dcVarMaps[a[i].get(GRB_StringAttr_VarName)] = int(a[i].get(GRB_DoubleAttr_X));
            }
            f.close();

            std::ofstream dcs(this->diffCharacteristicPath, std::ios::trunc);
            if (!dcs){
                std::cout << "Wrong file path ! " << std::endl;
            } else {
                for (int j = 0; j < this->differentialCharacteristic.size(); ++j) {
                    if (j%2 == 0)
                        dcs << j/2 << "-th input difference\n";
                    else
                        dcs << (j-1)/2 << "-th output difference\n";
                    for (int k = 0; k < this->differentialCharacteristic[j].size(); ++k) {
                        dcs << dcVarMaps["x" + std::to_string(this->differentialCharacteristic[j][k])];
                        if ((k+1)%8 == 0)
                            dcs << " ";
                        if (k == this->differentialCharacteristic[j].size() - 1)
                            dcs << "\n";
                    }
                }
            }
            dcs.close();

            std::cout << "***********************************" << std::endl;
            std::cout << "      Obj is  : " << result << std::endl;
            std::cout << "***********************************" << std::endl;
            this->finalResults.push_back(result);
            this->finalClockTime.push_back(clockTime);
            this->finalTimeTime.push_back(timeTime);
        } else if (optimstatus == 3) {
            std::cout << "************************* THE CONSTRUCTED MODEL ARE INFEASIBLE *************************" << std::endl;
        }
    } else if (this->target == "evaluation") {
        if (optimstatus == 2) {
            std::ofstream f(this->resultsPath, std::ios::trunc);
            f << "solving the MILP " << this->modelPath << "\n";
            f << "obj is : " << this->targetNdiff << "\n";
            f << "clock time : " << clockTime << "s\n";
            f << "time time : " << timeTime << "s\n";

            map<std::string, int> dcVarMaps;
            GRBVar *a = model.getVars();
            int varNum = model.get(GRB_IntAttr_NumVars);
            for (int i = 0; i < varNum; ++i) {
                f << a[i].get(GRB_StringAttr_VarName) << " = " << a[i].get(GRB_DoubleAttr_X) << "\n";
                dcVarMaps[a[i].get(GRB_StringAttr_VarName)] = int(a[i].get(GRB_DoubleAttr_X));
            }
            f.close();

            std::ofstream dcs(this->diffCharacteristicPath, std::ios::trunc);
            if (!dcs){
                std::cout << "Wrong file path ! " << std::endl;
            } else {
                for (int j = 0; j < this->differentialCharacteristic.size(); ++j) {
                    if (j%2 == 0)
                        dcs << j/2 << "-th input difference\n";
                    else
                        dcs << (j-1)/2 << "-th output difference\n";
                    for (int k = 0; k < this->differentialCharacteristic[j].size(); ++k) {
                        dcs << dcVarMaps["x" + std::to_string(this->differentialCharacteristic[j][k])];
                        if ((k+1)%8 == 0)
                            dcs << " ";
                        if (k == this->differentialCharacteristic[j].size() - 1)
                            dcs << "\n";
                    }
                }
            }
            dcs.close();

            std::cout << "***********************************" << std::endl;
            std::cout << "      number of active sboxes  : " << this->targetNdiff << std::endl;
            std::cout << "      clockTime is  : " << clockTime << std::endl;
            std::cout << "      timeTime is  : " << timeTime << std::endl;
            std::cout << "***********************************" << std::endl;
            this->finalResults.push_back(this->targetNdiff);
            this->finalClockTime.push_back(clockTime);
            this->finalTimeTime.push_back(timeTime);
        }
        else if (optimstatus == 3) {
            this->securityFlag = true;
            this->securityRoundNumBound = this->currentRound;
            std::cout << "\n\n\n****************************************************************" << std::endl;
            std::cout << "the block cipher is security enough when the round number is : "
                      << this->currentRound << std::endl;
            std::cout << "****************************************************************\n\n\n" << std::endl;

            std::cout << "***********************************" << std::endl;
            std::cout << "      clockTime is  : " << clockTime << std::endl;
            std::cout << "      timeTime is  : " << timeTime << std::endl;
            std::cout << "***********************************" << std::endl;
            this->finalClockTime.push_back(clockTime);
            this->finalTimeTime.push_back(timeTime);

            std::ofstream f(this->resultsPath, std::ios::trunc);
            f << "clock time : " << clockTime << "s\n";
            f << "time time : " << timeTime << "s\n";
            f.close();
        }
        else
            std::cout << "optimstatus : " << optimstatus << std::endl;
    }
}


void DiffSWMILP::programGenModel() {
    int roundCounter = this->currentRound;
    for (const auto& proc : this->procedureHs) {
        if (proc->getName() == "main") {
            std::string roundFuncId;
            bool newRoundFlag = false;
            int tempSizeCounter = 0;
            int roundFlag = false, modelCheckFlag = true;
            for (const auto& ele : proc->getBlock()) {
                if (ele->getLhs()->getNodeType() == UINT) {
                    newRoundFlag = true;
                    continue;
                }
                if (newRoundFlag and roundCounter > 0) {
                    if (ele->getNodeName() == "plaintext_push")
                        tempSizeCounter++;
                    if (ele->getOp() == ASTNode::CALL) {
                        this->blockSize = tempSizeCounter;
                        tempSizeCounter = 0;
                        roundFuncId = ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@"));
                        for (const auto& tproc : procedureHs) {
                            if (tproc->getName() == roundFuncId) {
                                roundFunctionGenModel(tproc);
                                roundFlag = true;
                                break;
                            }
                        }
                        newRoundFlag = false;
                        roundCounter--;
                    }
                }
                if (roundFlag and modelCheckFlag) {
                    std::ifstream file;
                    file.open(modelPath);
                    std::string model, line;
                    while (getline(file, line)) model = model + line + "\n";
                    file.close();
                    if (model == "") {
                        // initial for next round;
                        this->tanNameMxIndex.clear();
                        this->rtnMxIndex.clear();
                        this->rtnIdxSave.clear();
                        this->dCounter = 1;
                        this->ACounter = 1;
                        roundFlag = false;
                        this->differentialCharacteristic.clear();
                        this->roundSboxIndexSave.clear();
                        this->sboxFindFlag = false;
                        this->sboxIndexSave.clear();
                    } else
                        modelCheckFlag = false;
                }
            }
            break;
        }
    }
}

void DiffSWMILP::roundFunctionGenModel(const ProcedureHPtr &procedureH) {
    std::string keyId = procedureH->getParameters().at(1).at(0)->getNodeName().substr(0, procedureH->getParameters().at(1).at(0)->getNodeName().find("0"));
    for (const auto & i : procedureH->getReturns()) {
        if (i->getNodeType() != NodeType::PARAMETER) {
            this->wordSize = Transformer::getNodeTypeSize(i->getNodeType());
            break;
        }
    }

    std::vector<int> currRoundSboxIndex;

    if (!this->rtnIdxSave.empty()) {
        for (int i = 0; i < this->rtnIdxSave.size(); ++i)
            this->tanNameMxIndex[procedureH->getParameters().at(2).at(i)->getNodeName()] = this->rtnIdxSave[i];
        this->rtnIdxSave.clear();
        this->rtnMxIndex.clear();
    } else {
        for (const auto & i : procedureH->getParameters().at(2)) {
            this->tanNameMxIndex[i->getNodeName()] = this->ACounter;
            this->ACounter++;
        }
        std::vector<int> inputC;
        for (int i = 1; i < this->ACounter; ++i) {
            inputC.push_back(i);
        }
        this->differentialCharacteristic.push_back(inputC);
    }

    bool functionCallFlag;
    for (int i = 0; i < procedureH->getBlock().size(); ++i) {
        functionCallFlag = false;
        ThreeAddressNodePtr ele = procedureH->getBlock().at(i);

        if (ele->getOp() == ASTNode::XOR) {
            if (ele->getLhs()->getNodeName().find(keyId) != std::string::npos or ele->getRhs()->getNodeName().find(keyId) != std::string::npos) {}
            else if (this->isConstant(ele->getLhs()) or this->isConstant(ele->getRhs())) {}
            else {
                functionCallFlag = true;
                XORGenModel(ele->getLhs(), ele->getRhs(), ele);
            }
        } else if (ele->getOp() == ASTNode::AND or ele->getOp() == ASTNode::OR) {
            functionCallFlag = true;
            ANDandORGenModel(ele->getLhs(), ele->getRhs(), ele);
        } else if (ele->getOp() == ASTNode::BOXOP) {
            if (ele->getLhs()->getNodeName().substr(0, 4) == "sbox") {
                sboxFindFlag = true;
            } else if (ele->getLhs()->getNodeName().substr(0, 4) == "pbox") {
                functionCallFlag = true;
                PboxGenModel(ele->getLhs(), ele->getRhs(), ele);
            }
        } else if (ele->getOp() == ASTNode::SYMBOLINDEX) {
            ThreeAddressNodePtr left = ele->getLhs();
            if (left->getOp() == ASTNode::FFTIMES) {
                if (left->getLhs()->getNodeName().substr(0, 5) == "pboxm") {
                    functionCallFlag = true;
                    std::vector<ThreeAddressNodePtr> output;
                    int pboxSize = pboxM[left->getLhs()->getNodeName()].size();
                    int outputNum = (int)sqrt(pboxSize);
                    while (outputNum > 0) {
                        output.push_back(ele);
                        i++;
                        if (i == procedureH->getBlock().size()) {
                            i--;
                            break;
                        } else {
                            ele = procedureH->getBlock().at(i);
                            left = ele->getLhs();
                            if (left != nullptr and left->getOp() == ASTNode::FFTIMES and
                                left->getLhs()->getNodeName().substr(0, 5) == "pboxm") {
                                outputNum--;
                                continue;
                            } else {
                                i--;
                                left = procedureH->getBlock().at(i)->getLhs();
                                break;
                            }
                        }
                    }
                    if (outputNum == 0)
                        i--;
                    left = procedureH->getBlock().at(i)->getLhs();
                    linearPermuteGenModel(left->getLhs(), left->getRhs(), output);
                } else
                    assert(false);
            }
        } else if (ele->getOp() == ASTNode::TOUINT) {}
        else if (ele->getOp() == ASTNode::INDEX) {}
        else if (ele->getOp() == ASTNode::ADD or ele->getOp() == ASTNode::MINUS) {
            functionCallFlag = true;
            std::string outputName = ele->getNodeName().substr(0, ele->getNodeName().find_last_of("_"));
            std::vector<ThreeAddressNodePtr> input1, input2, output;
            while (true) {
                input1.push_back(ele->getLhs());
                input2.push_back(ele->getRhs());
                output.push_back(ele);
                i++;
                if (i == procedureH->getBlock().size()) {
                    i--;
                    break;
                }
                ele = procedureH->getBlock().at(i);
                std::string  aaa = ele->getNodeName().substr(0, outputName.size());

                if (ele != nullptr and ele->getOp() == ASTNode::ADD and ele->getNodeName().substr(0, outputName.size()) == outputName)
                    continue;
                else {
                    i--;
                    break;
                }
            }
            AddGenModel(input1, input2, output);
        }
        else assert(false);

        if (!functionCallFlag) {
            if (ele->getLhs()->getNodeType() == NodeType::ARRAY and ele->getOp() == ASTNode::SYMBOLINDEX) {
                std::string finderName = ele->getLhs()->getNodeName() + "_$B$_" + ele->getRhs()->getNodeName();
                for (const auto &pair: this->tanNameMxIndex) {
                    if (finderName == pair.first)
                        this->tanNameMxIndex[ele->getNodeName()] = pair.second;
                }
            } else {
                for (const auto& pair : this->tanNameMxIndex) {
                    if (ele->getLhs() != nullptr)
                        if (ele->getLhs()->getNodeName() == pair.first)
                            this->tanNameMxIndex[ele->getNodeName()] = pair.second;
                    if (ele->getRhs() != nullptr)
                        if (ele->getRhs()->getNodeName() == pair.first)
                            this->tanNameMxIndex[ele->getNodeName()] = pair.second;
                }
            }
        } else
            functionCallFlag = false;

        if (this->sboxFindFlag) {
            this->sboxFindFlag = false;
            for (const auto& name : this->tanNameMxIndex) {
                if (name.first == ele->getNodeName()) {
                    this->sboxIndexSave.push_back(name.second);
                    currRoundSboxIndex.push_back(name.second);
                }
            }
        }
    }

    for (const auto& rtn : procedureH->getReturns()) {
        for (const auto& pair : this->tanNameMxIndex) {
            if (rtn->getNodeName() == pair.first) {
                this->rtnMxIndex[pair.first] = pair.second;
                this->rtnIdxSave.push_back(pair.second);
            }
        }
    }

    std::vector<int> outputC;
    for (int & i : this->rtnIdxSave) {
        outputC.push_back(i);
    }
    this->differentialCharacteristic.push_back(outputC);

    this->roundSboxIndexSave.push_back(currRoundSboxIndex);
}

void DiffSWMILP::XORGenModel(const ThreeAddressNodePtr &left, const ThreeAddressNodePtr &right,
                             const ThreeAddressNodePtr &result) {
    int inputIdx1 = 0, inputIdx2 = 0;
    for (const auto& pair : tanNameMxIndex) {
        if (pair.first == left->getNodeName())
            inputIdx1 = pair.second;
        if (pair.first == right->getNodeName())
            inputIdx2 = pair.second;
    }
    std::vector<int> inputIdx = {inputIdx1, inputIdx2};
    for (auto item : inputIdx) {
        if (item == 0) {
            if (!this->rtnIdxSave.empty()) {
                item = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = item;
            } else {
                item = this->ACounter;
                this->tanNameMxIndex[left->getNodeName()] = item;
                this->ACounter++;
            }
        }
    }
    int outputIdx = ACounter;
    tanNameMxIndex[result->getNodeName()] = outputIdx;
    ACounter++;
    switch (this->xorConSel) {
        case 1:
            DiffMILPcons::wXorC1(this->modelPath, inputIdx1, inputIdx2, outputIdx, dCounter);
            break;
        case 2:
            DiffMILPcons::wXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
            break;
        default:
            DiffMILPcons::wXorC1(this->modelPath, inputIdx1, inputIdx2, outputIdx, dCounter);
    }
}

void DiffSWMILP::ANDandORGenModel(const ThreeAddressNodePtr &left, const ThreeAddressNodePtr &right,
                                  const ThreeAddressNodePtr &result) {
    int inputIdx1 = 0, inputIdx2 = 0;
    for (const auto& pair : tanNameMxIndex) {
        if (pair.first == left->getNodeName())
            inputIdx1 = pair.second;
        if (pair.first == right->getNodeName())
            inputIdx2 = pair.second;
    }
    std::vector<int> inputIdx = {inputIdx1, inputIdx2};
    for (auto item : inputIdx) {
        if (item == 0) {
            if (!this->rtnIdxSave.empty()) {
                item = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = item;
            } else {
                item = this->ACounter;
                this->tanNameMxIndex[left->getNodeName()] = item;
                this->ACounter++;
            }
        }
    }
    int outputIdx = ACounter;
    tanNameMxIndex[result->getNodeName()] = outputIdx;
    ACounter++;
    DiffMILPcons::wAndOrC(this->modelPath, inputIdx1, inputIdx2, outputIdx);
}


void DiffSWMILP::PboxGenModel(const ThreeAddressNodePtr &pbox, const ThreeAddressNodePtr &input,
                              const ThreeAddressNodePtr &output) {
    std::vector<int> pboxValue = this->Box[pbox->getNodeName()];
    std::vector<int> inputIdx = extIdxFromTOUINTorBOXINDEX(input);
    std::vector<int> outputIdx;
    for (int i = 0; i < pboxValue.size(); ++i)
        outputIdx.push_back(0);
    for (int i = 0; i < pboxValue.size(); ++i)
        outputIdx[i] = inputIdx[pboxValue[i]];
    for (int i = 0; i < outputIdx.size(); ++i)
        this->tanNameMxIndex[output->getNodeName() + "_$B$_" + std::to_string(i)] = outputIdx[i];
}


void DiffSWMILP::linearPermuteGenModel(const ThreeAddressNodePtr &pbox, const ThreeAddressNodePtr &input,
                                       const vector<ThreeAddressNodePtr> &output) {
    std::vector<int> inputIdx;
    std::vector<ThreeAddressNodePtr> inputTAN;
    if (input->getOp() == ASTNode::BOXINDEX) {
        ThreeAddressNodePtr left = input;
        while (left->getOp() == ASTNode::BOXINDEX) {
            inputTAN.push_back(left->getLhs());
            left = left->getRhs();
        }
        inputTAN.push_back(left);
    } else
        assert(false);

    for (auto & i : inputTAN) {
        bool iFlag = false;
        for (const auto& pair : tanNameMxIndex) {
            if (pair.first == i->getNodeName()) {
                inputIdx.push_back(pair.second);
                iFlag = true;
            }
        }
        if (!iFlag) {
            inputIdx.push_back(ACounter);
            tanNameMxIndex[i->getNodeName()] = ACounter;
            ACounter++;
        }
    }

    std::vector<int> outputIdx;
    for (const auto& i : output) {
        outputIdx.push_back(ACounter);
        tanNameMxIndex[i->getNodeName()] = ACounter;
        ACounter++;
    }
    if (this->cipherName == "MIBS_word_wise")
        this->branchNumMin[pbox->getNodeName()] = 5;
    switch (this->linearConSel) {
        case 1:
            DiffMILPcons::wLinearC1(this->modelPath, inputIdx, outputIdx, dCounter, branchNumMin[pbox->getNodeName()], branchNumMax[pbox->getNodeName()]);
            break;
        case 2:
            DiffMILPcons::wLinearC2(this->modelPath, inputIdx, outputIdx, dCounter, branchNumMin[pbox->getNodeName()], branchNumMax[pbox->getNodeName()]);
            break;
        default:
            DiffMILPcons::wLinearC1(this->modelPath, inputIdx, outputIdx, dCounter, branchNumMin[pbox->getNodeName()], branchNumMax[pbox->getNodeName()]);
    }
}

void DiffSWMILP::AddGenModel(vector<ThreeAddressNodePtr> &input1, vector<ThreeAddressNodePtr> &input2,
                             const vector<ThreeAddressNodePtr> &output) {
    std::vector<int> inputIdx1, inputIdx2;
    for (auto in : input1) {
        if (tanNameMxIndex.count(in->getNodeName()) != 0) {
            inputIdx1.push_back(tanNameMxIndex.find(in->getNodeName())->second);
        } else if (!rtnIdxSave.empty()) {
            tanNameMxIndex[in->getNodeName()] = rtnIdxSave.front();
            inputIdx1.push_back(rtnIdxSave.front());
            rtnIdxSave.erase(rtnIdxSave.cbegin());
        } else {
            tanNameMxIndex[in->getNodeName()] = ACounter;
            inputIdx1.push_back(ACounter);
            ACounter++;
        }
    }
    for (auto in : input2) {
        if (tanNameMxIndex.count(in->getNodeName()) != 0) {
            inputIdx2.push_back(tanNameMxIndex.find(in->getNodeName())->second);
        } else if (!rtnIdxSave.empty()) {
            tanNameMxIndex[in->getNodeName()] = rtnIdxSave.front();
            inputIdx2.push_back(rtnIdxSave.front());
            rtnIdxSave.erase(rtnIdxSave.cbegin());
        } else {
            tanNameMxIndex[in->getNodeName()] = ACounter;
            inputIdx2.push_back(ACounter);
            ACounter++;
        }
    }

    std::vector<int> outputIdx;
    for (const auto& ele : output) {
        tanNameMxIndex[ele->getNodeName()] = ACounter;
        outputIdx.push_back(ACounter);
        ACounter++;
    }

    for (int i = 0; i < inputIdx1.size(); ++i) {
        switch (xorConSel) {
            case 1:
                DiffMILPcons::wXorC1(this->modelPath, inputIdx1[i], inputIdx2[i], outputIdx[i], dCounter);
                break;
            case 2:
                DiffMILPcons::wXorC2(this->modelPath, inputIdx1[i], inputIdx2[i], outputIdx[i]);
                break;
            default:
                DiffMILPcons::wXorC1(this->modelPath, inputIdx1[i], inputIdx2[i], outputIdx[i], dCounter);
        }
    }
}



bool DiffSWMILP::isConstant(ThreeAddressNodePtr threeAddressNodePtr) {
    ThreeAddressNodePtr left = threeAddressNodePtr->getLhs();
    ThreeAddressNodePtr right = threeAddressNodePtr->getRhs();

    if (threeAddressNodePtr->getNodeType() == UINT)
        return true;
    else if (threeAddressNodePtr->getNodeName() == this->roundID)
        return true;
    else if (left == nullptr and right == nullptr)
        return false;
    else if (left != nullptr and right != nullptr) {
        if (left->getNodeType() == UINT and right->getNodeType() == UINT)
            return true;
        else if (left->getNodeType() == UINT and right->getNodeName() == roundID)
            return true;
        else if (right->getNodeType() == UINT and left->getNodeName() == roundID)
            return true;
        else
            return false;
    } else if (left != nullptr and right == nullptr) {
        if (left->getNodeType() == UINT)
            return true;
        if (left->getNodeName() == this->roundID)
            return true;
        else if (isConstant(left))
            return true;
        else
            return false;
    } else if (left == nullptr and right != nullptr) {
        if (right->getNodeType() == UINT)
            return true;
        if (right->getNodeName() == this->roundID)
            return true;
        else if (isConstant(right))
            return true;
        else
            return false;
    } else
        assert(false);
}

