//
// Created by Septi on 5/22/2023.
//

#include "differential/DiffSBMILP.h"

extern std::map<std::string, std::vector<int>> allBox;
extern std::map<std::string, std::vector<int>> pboxM;
extern std::map<std::string, int> pboxMSize;
extern std::map<std::string, std::vector<int>> Ffm;
extern std::string cipherName;


DiffSBMILP::DiffSBMILP(std::vector<ProcedureHPtr> procedureHs, std::string target, std::string mode, int redMode)
        : procedureHs(std::move(procedureHs)), target(move(target)), mode(move(mode)), redMode(redMode) {
    this->Box = allBox;
    this->cipherName = ::cipherName;
}


void DiffSBMILP::MGR() {
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

    for (int i = this->startRound; i <= this->endRound; ++i) {
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
        this->xCounter = 1;
        this->dCounter = 1;
        this->ACounter = 1;
        this->PCounter = 1;
        this->fCounter = 1;
        this->yCounter = 1;
        this->lastRoundACounter = 1;
        this->lastRoundPCounter = 1;
        this->allRoundACounters.clear();
        this->allRoundPCounters.clear();
        this->differentialCharacteristic.clear();
        std::ofstream fResults(this->finalResultsPath, std::ios::trunc);
        if (!fResults){
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            if (this->target == "cryptanalysis") {
                fResults << " ***************** FINAL RESULTS ***************** \n\n";
                for (int j = 0; j < this->finalResults.size(); ++j) {
                    fResults << "Results Of " << j + 1 << " Rounds : \n";
                    fResults << "\tObj : " << this->finalResults[j] << "\n";
                    fResults << "\tclockTime : " << this->finalClockTime[j] << "\n";
                    fResults << "\ttimeTime : " << this->finalTimeTime[j] << "\n\n";
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
                        fResults << "\tnumber of target active sboxes for security bounding : " << this->targetNdiffOrPr << "\n";
                        fResults << "\tclockTime : " << this->finalClockTime[j] << "\n";
                        fResults << "\ttimeTime : " << this->finalTimeTime[j] << "\n\n";
                        fResults << "\ttotalRoundClockTime : " << finalTotalRoundClockTime[j] << "\n";
                        fResults << "\ttotalRoundTimeTime : " << finalTotalRoundTimeTime[j] << "\n\n\n";
                    } else {
                        fResults << "***************** Results Of " << j + 1 << " Rounds *****************\n";
                        fResults << "\tnumber of active sboxes : " << this->finalResults[j] << "\n";
                        fResults << "\tclockTime : " << this->finalClockTime[j] << "\n";
                        fResults << "\ttimeTime : " << this->finalTimeTime[j] << "\n\n";
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
                std::cout << "\tnumber of target active sboxes for security bounding : " << this->targetNdiffOrPr << "\n";
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


void DiffSBMILP::cryptanalysis() {
    programGenModel();

    std::ifstream file;
    file.open(this->modelPath);
    std::string model,line;
    while (getline(file, line)) {model += line + "\n";}
    file.close();

    if (model != "") {
        std::ofstream target(this->modelPath, std::ios::trunc);
        if (!target) {
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            target << "Minimize\n";
            if (this->mode == "AS") {
                for (int i = 1; i < this->ACounter; ++i) {
                    target << "A" << i;
                    if (i != this->ACounter - 1) target << " + ";
                }
            } else if (this->mode == "DC") {
                if (!this->sboxExtWeighted.empty()) {
                    for (int i = 1; i < this->PCounter; i += this->sboxExtWeighted.size()) {
                        int k = 0;
                        for (auto weight: this->sboxExtWeighted) {
                            if (weight != 1)
                                target << weight << " P" << i + k;
                            else
                                target << "P" << i + k;
                            if (i + k != this->PCounter - 1) target << " + ";
                            k++;
                        }
                    }
                } else {
                    for (int i = 1; i < this->PCounter; i ++) {
                        target << "P" << i;
                        if (i != this->PCounter - 1) target << " + ";
                    }
                }
            }
            target << "\n" << "Subject To\n";

            if (this->speedup1) {
                for (int j = 0; j < this->differentialCharacteristic.size() - 1; ++j) {
                    for (int k = 0; k < this->differentialCharacteristic[j].size(); ++k) {
                        if (k != this->differentialCharacteristic[j].size() - 1)
                            target << "x" << std::to_string(this->differentialCharacteristic[j][k]) << " + ";
                        else
                            target << "x" << std::to_string(this->differentialCharacteristic[j][k]);
                    }
                    if (this->differentialCharacteristic[j].size() > 0) target << " >= 1\n";
                }
            } else {
                for (int i = 0; i < this->blockSize; ++i) {
                    if (i != this->blockSize - 1)
                        target << "x" << std::to_string(i + 1) << " + ";
                    else
                        target << "x" << std::to_string(i + 1);
                }
                target << " >= 1\n";
            }

            if (this->speedup2) {
                if (this->mode == "AS") {
                    for (int i = 0; i < this->finalResults.size(); ++i) {
                        for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                            for (int k = 0; k < this->allRoundACounters[j].size(); ++k) {
                                target << "A" << this->allRoundACounters[j][k];
                                if (k != this->allRoundACounters[j].size() - 1) target << " + ";
                            }
                            if (j != this->currentRound - 1) target << " + ";
                        }
                        target << " - " << this->finalResults[i] << " >= 0\n";
                    }

                    for (int i = 0; i < this->finalResults.size(); ++i) {
                        for (int j = 0; j < i + 1; ++j) {
                            for (int k = 0; k < this->allRoundACounters[j].size(); ++k) {
                                target << "A" << this->allRoundACounters[j][k];
                                if (k != this->allRoundACounters[j].size() - 1) target << " + ";
                            }
                            if (j != i) target << " + ";
                        }
                        target << " - " << this->finalResults[i] << " >= 0\n";
                    }
                } else if (this->mode == "DC") {
                    if (!this->sboxExtWeighted.empty()) {
                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); k += this->sboxExtWeighted.size()) {
                                    int p = 0;
                                    for (auto weight: this->sboxExtWeighted) {
                                        if (weight != 1) {
                                            target << std::to_string(weight) << " P" << this->allRoundPCounters[j][k + p];
                                        } else {
                                            target << "P" << this->allRoundPCounters[j][k + p];
                                        }
                                        if (k + p != this->allRoundPCounters[j].size() - 1) target << " + ";
                                        p++;
                                    }
                                }
                                if (j != this->currentRound - 1) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }

                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = 0; j < i + 1; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); k += this->sboxExtWeighted.size()) {
                                    int p = 0;
                                    for (auto weight: this->sboxExtWeighted) {
                                        if (weight != 1) {
                                            target << std::to_string(weight) << " P" << this->allRoundPCounters[j][k + p];
                                        } else {
                                            target << "P" << this->allRoundPCounters[j][k + p];
                                        }
                                        if (k + p != this->allRoundPCounters[j].size() - 1) target << " + ";
                                        p++;
                                    }
                                }
                                if (j != i) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }
                    } else {


                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); ++k) {
                                    target << "P" << this->allRoundPCounters[j][k];
                                    if (k != this->allRoundPCounters[j].size() - 1) target << " + ";
                                }
                                if (j != this->currentRound - 1) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }

                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = 0; j < i + 1; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); ++k) {
                                    target << "P" << this->allRoundPCounters[j][k];
                                    if (k != this->allRoundPCounters[j].size() - 1) target << " + ";
                                }
                                if (j != i) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }


                    }
                }
            }

            target << model;
            target << "x0 = 0\n";

        }
        target.close();
    }
    std::ofstream binary(this->modelPath, std::ios::app);
    if (!binary) {
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        binary << "Binary\n";
        if (this->ILPFlag) {
            for (int i = 1; i < this->xCounter; ++i)
                binary << "x" << i << "\n";
        } else {
            for (int i = 1; i < blockSize + 1; ++i) {
                binary << "x" << i << "\n";
            }
        }
        for (int i = 1; i < this->dCounter; ++i)
            binary << "d" << i << "\n";
        if (this->mode == "AS") {
            for (int i = 1; i < this->ACounter; ++i)
                binary << "A" << i << "\n";
        } else if (this->mode == "DC") {
            for (int i = 1; i < this->PCounter; ++i)
                binary << "P" << i << "\n";
        }
        for (int i = 1; i < this->fCounter; ++i)
            binary << "f" << i << "\n";
        binary << "General\n";
        for (int i = 0; i < this->yCounter; ++i)
            binary << "y" << i << "\n";
        binary << "End";
        binary.close();
    }
}


void DiffSBMILP::evaluation() {
    if (this->evaluationSize == 0)
        this->evaluationSize = this->blockSize;

    if (this->mode == "AS") {
        this->targetNdiffOrPr = int(ceil(this->currentRound * ((this->evaluationSize / this->sboxMDP) / this->totalRoundNum))) - 1;
    } else if (this->mode == "DC") {
        this->targetNdiffOrPr = int(ceil(this->currentRound * (this->evaluationSize / this->totalRoundNum))) - 1;

        if (this->cipherName == "GIFT_128" or this->cipherName == "GIFT_64")
            this->targetNdiffOrPr = int(this->currentRound * this->evaluationSize / this->totalRoundNum * 200);
    }

    programGenModel();

    std::ifstream file;
    file.open(this->modelPath);
    std::string model,line;
    while (getline(file, line)) {model += line + "\n";}
    file.close();

    if (model != "") {
        std::ofstream target(this->modelPath, std::ios::trunc);
        if (!target) {
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            target << "Subject To\n";
            if (this->mode == "AS") {
                for (int i = 1; i < this->ACounter; ++i) {
                    target << "A" << i;
                    if (i != this->ACounter - 1) target << " + ";
                }
            } else if (this->mode == "DC") {
                if (!this->sboxExtWeighted.empty()) {
                    for (int i = 1; i < this->PCounter; i += this->sboxExtWeighted.size()) {
                        int k = 0;
                        for (auto weight: this->sboxExtWeighted) {
                            if (weight != 1)
                                target << weight << " P" << i + k;
                            else
                                target << "P" << i + k;
                            if (i + k != this->PCounter - 1) target << " + ";
                            k++;
                        }
                    }
                } else {
                    for (int i = 1; i < this->PCounter; i ++) {
                        target << "P" << i;
                        if (i != this->PCounter - 1) target << " + ";
                    }
                }
            }
            target << " <= " << this->targetNdiffOrPr << "\n";

            if (this->speedup1) {
                for (int j = 0; j < this->differentialCharacteristic.size() - 1; ++j) {
                    for (int k = 0; k < this->differentialCharacteristic[j].size(); ++k) {
                        if (k != this->differentialCharacteristic[j].size() - 1)
                            target << "x" << std::to_string(this->differentialCharacteristic[j][k]) << " + ";
                        else
                            target << "x" << std::to_string(this->differentialCharacteristic[j][k]);
                    }
                    if (this->differentialCharacteristic[j].size() > 0) target << " >= 1\n";
                }
            } else {
                for (int i = 0; i < this->blockSize; ++i) {
                    if (i != this->blockSize - 1)
                        target << "x" << std::to_string(i + 1) << " + ";
                    else
                        target << "x" << std::to_string(i + 1);
                }
                target << " >= 1\n";
            }

            if (this->speedup2) {
                if (this->mode == "AS") {
                    for (int i = 0; i < this->finalResults.size(); ++i) {
                        for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                            for (int k = 0; k < this->allRoundACounters[j].size(); ++k) {
                                target << "A" << this->allRoundACounters[j][k];
                                if (k != this->allRoundACounters[j].size() - 1) target << " + ";
                            }
                            if (j != this->currentRound - 1) target << " + ";
                        }
                        target << " - " << this->finalResults[i] << " >= 0\n";
                    }

                    for (int i = 0; i < this->finalResults.size(); ++i) {
                        for (int j = 0; j < i + 1; ++j) {
                            for (int k = 0; k < this->allRoundACounters[j].size(); ++k) {
                                target << "A" << this->allRoundACounters[j][k];
                                if (k != this->allRoundACounters[j].size() - 1) target << " + ";
                            }
                            if (j != i) target << " + ";
                        }
                        target << " - " << this->finalResults[i] << " >= 0\n";
                    }
                } else if (this->mode == "DC") {
                    if (!this->sboxExtWeighted.empty()) {
                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); k += this->sboxExtWeighted.size()) {
                                    int p = 0;
                                    for (auto weight: this->sboxExtWeighted) {
                                        if (weight != 1) {
                                            target << std::to_string(weight) << " P" << this->allRoundPCounters[j][k + p];
                                        } else {
                                            target << "P" << this->allRoundPCounters[j][k + p];
                                        }
                                        if (k + p != this->allRoundPCounters[j].size() - 1) target << " + ";
                                        p++;
                                    }
                                }
                                if (j != this->currentRound - 1) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }

                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = 0; j < i + 1; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); k += this->sboxExtWeighted.size()) {
                                    int p = 0;
                                    for (auto weight: this->sboxExtWeighted) {
                                        if (weight != 1) {
                                            target << std::to_string(weight) << " P" << this->allRoundPCounters[j][k + p];
                                        } else {
                                            target << "P" << this->allRoundPCounters[j][k + p];
                                        }
                                        if (k + p != this->allRoundPCounters[j].size() - 1) target << " + ";
                                        p++;
                                    }
                                }
                                if (j != i) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }
                    } else {


                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); ++k) {
                                    target << "P" << this->allRoundPCounters[j][k];
                                    if (k != this->allRoundPCounters[j].size() - 1) target << " + ";
                                }
                                if (j != this->currentRound - 1) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }

                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = 0; j < i + 1; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); ++k) {
                                    target << "P" << this->allRoundPCounters[j][k];
                                    if (k != this->allRoundPCounters[j].size() - 1) target << " + ";
                                }
                                if (j != i) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }


                    }

                }
            }

            target << model;
            target << "x0 = 0\n";

        }
        target.close();
    }
    std::ofstream binary(this->modelPath, std::ios::app);
    if (!binary) {
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        binary << "Binary\n";
        if (this->ILPFlag) {
            for (int i = 1; i < this->xCounter; ++i)
                binary << "x" << i << "\n";
        } else {
            for (int i = 1; i < blockSize + 1; ++i) {
                binary << "x" << i << "\n";
            }
        }
        for (int i = 1; i < this->dCounter; ++i)
            binary << "d" << i << "\n";
        if (this->mode == "AS") {
            for (int i = 1; i < this->ACounter; ++i)
                binary << "A" << i << "\n";
        } else if (this->mode == "DC") {
            for (int i = 1; i < this->PCounter; ++i)
                binary << "P" << i << "\n";
        }
        for (int i = 1; i < this->fCounter; ++i)
            binary << "f" << i << "\n";
        binary << "General\n";
        for (int i = 0; i < this->yCounter; ++i)
            binary << "y" << i << "\n";
        binary << "End";
        binary.close();
    }
}


void DiffSBMILP::solver() {
    clock_t startTime, endTime;
    startTime = clock();
    time_t star_time = 0, end_time;
    star_time = time(NULL);
    GRBEnv env = GRBEnv(true);

    env.set(GRB_IntParam_Threads, this->gurobiThreads);
    env.start();
    GRBModel model = GRBModel(env, modelPath);
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
            f << "obj is : " << this->targetNdiffOrPr << "\n";
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
            std::cout << "      number of active sboxes  : " << this->targetNdiffOrPr << std::endl;
            std::cout << "      clockTime is  : " << clockTime << std::endl;
            std::cout << "      timeTime is  : " << timeTime << std::endl;
            std::cout << "***********************************" << std::endl;
            this->finalResults.push_back(this->targetNdiffOrPr);
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


void DiffSBMILP::programGenModel() {
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
                    this->rndParamR = stoi(ele->getLhs()->getNodeName());
                    continue;
                }
                if (newRoundFlag and roundCounter > 0) {
                    if (ele->getNodeName() == "plaintext_push")
                        tempSizeCounter++;
                    if (ele->getOp() == ASTNode::CALL) {
                        this->blockSize = tempSizeCounter;
                        tempSizeCounter = 0;
                        roundFuncId = ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@"));
                        for (const auto& tproc : this->procedureHs) {
                            if (tproc->getName() == roundFuncId) {
                                roundFunctionGenModel(tproc);
                                if (this->mode == "AS") {
                                    int startACounter = this->lastRoundACounter;
                                    int endACounter = this->ACounter;
                                    this->lastRoundACounter = this->ACounter;
                                    std::vector<int> roundACounter;
                                    for (int i = startACounter; i < endACounter; ++i) {
                                        roundACounter.push_back(i);
                                    }
                                    this->allRoundACounters.push_back(roundACounter);
                                } else if (this->mode == "DC") {
                                    int startPCounter = this->lastRoundPCounter;
                                    int endPCounter = this->PCounter;
                                    this->lastRoundPCounter = this->PCounter;
                                    std::vector<int> roundPCounter;
                                    for (int i = startPCounter; i < endPCounter; ++i) {
                                        roundPCounter.push_back(i);
                                    }
                                    this->allRoundPCounters.push_back(roundPCounter);
                                }
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
                    file.open(this->modelPath);
                    std::string model, line;
                    while (getline(file, line)) {model += line + "\n";}
                    file.close();
                    if (model == "Subject To\n") {
                        // initial for next round;
                        this->tanNameMxIndex.clear();
                        this->rtnMxIndex.clear();
                        this->rtnIdxSave.clear();
                        this->xCounter = 1;
                        this->dCounter = 1;
                        this->ACounter = 1;
                        this->PCounter = 1;
                        this->fCounter = 1;
                        this->yCounter = 1;
                        this->lastRoundACounter = 1;
                        this->lastRoundPCounter = 1;
                        this->allRoundACounters.clear();
                        this->allRoundPCounters.clear();
                        roundFlag = false;
                        this->differentialCharacteristic.clear();
                    } else
                        modelCheckFlag = false;
                }
            }
            break;
        }
    }
}


void DiffSBMILP::roundFunctionGenModel(const ProcedureHPtr &procedureH) {
    this->constantTan.clear();
    this->consTanNameMxVal.clear();

    this->consTanNameMxVal[procedureH->getParameters().at(0).at(0)->getNodeName()] = this->rndParamR;
    this->constantTan.push_back(procedureH->getParameters().at(0).at(0)->getNodeName());

    std::string keyId = procedureH->getParameters().at(1).at(0)->getNodeName().substr(0, procedureH->getParameters().at(1).at(0)->getNodeName().find("0"));

    if (!this->rtnIdxSave.empty()) {
        for (int i = 0; i < this->rtnIdxSave.size(); ++i)
            this->tanNameMxIndex[procedureH->getParameters().at(2).at(i)->getNodeName()] = this->rtnIdxSave[i];
        this->rtnIdxSave.clear();
        this->rtnMxIndex.clear();
    } else {
        for (const auto & i : procedureH->getParameters().at(2)) {
            this->tanNameMxIndex[i->getNodeName()] = this->xCounter;
            this->xCounter++;
        }
        std::vector<int> inputC;
        for (int i = 1; i < this->xCounter; ++i) {
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
                XORGenModel(ele->getLhs(), ele->getRhs(), ele, false);
            }
        } else if (ele->getOp() == ASTNode::AND or ele->getOp() == ASTNode::OR) {
            functionCallFlag = true;
            ANDandORGenModel(ele->getLhs(), ele->getRhs(), ele, false);
        } else if (ele->getOp() == ASTNode::BOXOP) {
            functionCallFlag = true;
            if (ele->getLhs()->getNodeName().substr(0, 4) == "sbox") {
                SboxGenModel(ele->getLhs(), ele->getRhs(), ele);
            } else if (ele->getLhs()->getNodeName().substr(0, 4) == "pbox") {
                PboxGenModel(ele->getLhs(), ele->getRhs(), ele);
            }
        }
        else if (ele->getOp() == ASTNode::ADD) {
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
                if (ele != nullptr and ele->getOp() == ASTNode::ADD and ele->getNodeName().substr(0, outputName.size()) == outputName)
                    continue;
                else {
                    i--;
                    break;
                }
            }
            ADDandMINUSGenModel(input1, input2, output, 1, false);
        }
        else if (ele->getOp() == ASTNode::MINUS) {
            if (ele->getLhs()->getNodeType() == NodeType::UINT or ele->getRhs()->getNodeType() == NodeType::UINT) {
                continue;
            }

            if (isConstant(ele->getLhs()) or isConstant(ele->getRhs())) {

            } else {
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
                ADDandMINUSGenModel(input1, input2, output, 2, false);
            }
        } else if (ele->getOp() == ASTNode::PUSH) {
            functionCallFlag = true;
            std::vector<ThreeAddressNodePtr> input, output;
            while (ele->getNodeName() == "sbox_push") {
                input.push_back(ele->getLhs());
                i++;
                ele = procedureH->getBlock().at(i);
            }
            std::string sboxFuncId = ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@"));
            while (ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@")) == sboxFuncId) {
                output.push_back(ele);
                i++;
                ele = procedureH->getBlock().at(i);
            }
            i--;
            ele = procedureH->getBlock().at(i);
            for (const auto& tproc : this->procedureHs) {
                if (tproc->getName() == sboxFuncId) {
                    sboxFunctionGenModel(tproc, input, output);
                    break;
                }
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
                    if (outputNum == 0) i--;
                    left = procedureH->getBlock().at(i)->getLhs();
                    MatrixVectorGenModel(left->getLhs(), left->getRhs(), output);
                } else
                    assert(false);
            }
        } else if (ele->getOp() == ASTNode::TOUINT) {
            if (ele->getLhs()->getOp() == ASTNode::BOXINDEX) {
                ThreeAddressNodePtr left = ele->getLhs();
                std::vector<int> leftIdx = extIdxFromTOUINTorBOXINDEX(left);
                int size = transNodeTypeSize(ele->getNodeType());
                for (int j = 0; j < size; ++j) {
                    this->tanNameMxIndex[ele->getNodeName() + "_$B$_" + std::to_string(j)] = leftIdx[j];
                }
            }
        }
        else if (ele->getOp() == ASTNode::MOD) {
            int leftVal, rightVal;
            if (this->consTanNameMxVal.count(ele->getLhs()->getNodeName()) != 0)
                leftVal = this->consTanNameMxVal[ele->getLhs()->getNodeName()];
            else if (ele->getLhs()->getOp() == ASTNode::NULLOP)
                leftVal = stoi(ele->getLhs()->getNodeName());
            else
                assert(false);

            if (this->consTanNameMxVal.count(ele->getRhs()->getNodeName()) != 0)
                rightVal = this->consTanNameMxVal[ele->getRhs()->getNodeName()];
            else if (ele->getRhs()->getOp() == ASTNode::NULLOP)
                rightVal = stoi(ele->getRhs()->getNodeName());
            else
                assert(false);

            this->consTanNameMxVal[ele->getNodeName()] = leftVal % rightVal;
            this->constantTan.push_back(ele->getNodeName());
        } else if (ele->getOp() == ASTNode::NOT) {
            if (this->tanNameMxIndex.count(ele->getLhs()->getNodeName()) != 0)
                this->tanNameMxIndex[ele->getNodeName()] = this->tanNameMxIndex[ele->getLhs()->getNodeName()];
            else
                assert(false);
        } else
            assert(false);

        if (!functionCallFlag) {
            if (ele->getLhs()->getNodeType() == NodeType::ARRAY and ele->getOp() == ASTNode::SYMBOLINDEX) {
                std::string finderName = ele->getLhs()->getNodeName() + "_$B$_" + ele->getRhs()->getNodeName();
                for (const auto &pair: this->tanNameMxIndex) {
                    if (finderName == pair.first)
                        this->tanNameMxIndex[ele->getNodeName()] = pair.second;
                }
            }
            else {
                for (const auto &pair: this->tanNameMxIndex) {
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
    }

    for (const auto& rtn : procedureH->getReturns()) {
        for (const auto &pair: this->tanNameMxIndex) {
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
}



void DiffSBMILP::sboxFunctionGenModel(const ProcedureHPtr &procedureH, std::vector<ThreeAddressNodePtr> input, std::vector<ThreeAddressNodePtr> output) {
    this->sboxNameMxIndex.clear();
    std::vector<int> inputIdx;

    int idx = 0;
    for (const auto & i : procedureH->getParameters().at(0)) {
        this->sboxNameMxIndex[i->getNodeName()] = this->tanNameMxIndex[input[idx]->getNodeName()];
        inputIdx.push_back(this->tanNameMxIndex[input[idx]->getNodeName()]);
        idx++;
    }

    std::ofstream scons(this->modelPath, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        for (int i = 0; i < inputIdx.size(); ++i) {
            if (i != inputIdx.size() - 1)
                scons << "x" << inputIdx[i] << " + ";
            else
                scons << "x" << inputIdx[i];
        }
        scons << " - A" << ACounter << " >= 0\n";
        for (auto i: inputIdx)
            scons << "A" << ACounter << " - x" << i << " >= 0\n";
        ACounter++;
    }

    bool functionCallFlag;
    for (int i = 0; i < procedureH->getBlock().size(); ++i) {
        functionCallFlag = false;
        ThreeAddressNodePtr ele = procedureH->getBlock().at(i);
        if (ele->getOp() == ASTNode::XOR) {
            if (this->isConstant(ele->getLhs()) or this->isConstant(ele->getRhs())) {}
            else {
                functionCallFlag = true;
                XORGenModel(ele->getLhs(), ele->getRhs(), ele, true);
            }
        } else if (ele->getOp() == ASTNode::AND or ele->getOp() == ASTNode::OR) {
            functionCallFlag = true;
            ANDandORGenModel(ele->getLhs(), ele->getRhs(), ele, true);
        } else if (ele->getOp() == ASTNode::TOUINT) {
            if (ele->getLhs()->getOp() == ASTNode::BOXINDEX) {
                ThreeAddressNodePtr left = ele->getLhs();
                std::vector<int> leftIdx = extIdxFromTOUINTorBOXINDEX(left);
                int size = transNodeTypeSize(ele->getNodeType());
                for (int j = 0; j < size; ++j) {
                    this->sboxNameMxIndex[ele->getNodeName() + "_$B$_" + std::to_string(j)] = leftIdx[j];
                }
            }
        } else if (ele->getOp() == ASTNode::ADD) {
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
                if (ele != nullptr and ele->getOp() == ASTNode::ADD and ele->getNodeName().substr(0, outputName.size()) == outputName)
                    continue;
                else {
                    i--;
                    break;
                }
            }
            ADDandMINUSGenModel(input1, input2, output, 1, true);
        } else if (ele->getOp() == ASTNode::MINUS) {
            if (ele->getLhs()->getNodeType() == NodeType::UINT or ele->getRhs()->getNodeType() == NodeType::UINT) {
                continue;
            }

            if (isConstant(ele->getLhs()) or isConstant(ele->getRhs())) {

            } else {
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
                ADDandMINUSGenModel(input1, input2, output, 2, false);
            }
        } else
            assert(false);

        if (!functionCallFlag) {
            if (ele->getLhs()->getNodeType() == NodeType::ARRAY and ele->getOp() == ASTNode::SYMBOLINDEX) {
                std::string finderName = ele->getLhs()->getNodeName() + "_$B$_" + ele->getRhs()->getNodeName();
                for (const auto &pair: this->sboxNameMxIndex) {
                    if (finderName == pair.first)
                        this->sboxNameMxIndex[ele->getNodeName()] = pair.second;
                }
            } else {
                for (const auto &pair: this->sboxNameMxIndex) {
                    if (ele->getLhs() != nullptr)
                        if (ele->getLhs()->getNodeName() == pair.first)
                            this->sboxNameMxIndex[ele->getNodeName()] = pair.second;
                    if (ele->getRhs() != nullptr)
                        if (ele->getRhs()->getNodeName() == pair.first)
                            this->sboxNameMxIndex[ele->getNodeName()] = pair.second;
                }
            }
        } else
            functionCallFlag = false;
    }

    this->sboxRtnIdxSave.clear();
    for (const auto& rtn : procedureH->getReturns()) {
        for (const auto &pair: this->sboxNameMxIndex) {
            if (rtn->getNodeName() == pair.first) {
                this->sboxRtnIdxSave.push_back(pair.second);
            }
        }
    }

    for (int i = 0; i < output.size(); ++i) {
        this->tanNameMxIndex[output[i]->getNodeName()] = this->sboxRtnIdxSave[i];
    }
}



void DiffSBMILP::XORGenModel(const ThreeAddressNodePtr &left, const ThreeAddressNodePtr &right,
                             const ThreeAddressNodePtr &result, bool ifSboxFuncCall) {
    int inputIdx1 = 0, inputIdx2 = 0;
    if (ifSboxFuncCall) {
        for (const auto& pair : this->sboxNameMxIndex) {
            if (pair.first == left->getNodeName())
                inputIdx1 = pair.second;
            if (pair.first == right->getNodeName())
                inputIdx2 = pair.second;
        }
    } else {
        for (const auto& pair : this->tanNameMxIndex) {
            if (pair.first == left->getNodeName())
                inputIdx1 = pair.second;
            if (pair.first == right->getNodeName())
                inputIdx2 = pair.second;
        }
    }

    if (ifSboxFuncCall) {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        }

        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->sboxNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        }
    } else {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        }

        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->tanNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        }
    }


    if (ifSboxFuncCall) {
        if (inputIdx1 == 0) {
            if (!this->sboxRtnIdxSave.empty()) {
                inputIdx1 = this->sboxRtnIdxSave.front();
                this->sboxRtnIdxSave.erase(this->sboxRtnIdxSave.cbegin());
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
            } else {
                inputIdx1 = this->xCounter;
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
                this->xCounter++;
            }
        }
        if (inputIdx2 == 0) {
            if (!this->sboxRtnIdxSave.empty()) {
                inputIdx2 = this->sboxRtnIdxSave.front();
                this->sboxRtnIdxSave.erase(this->sboxRtnIdxSave.cbegin());
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx2;
            } else {
                inputIdx2 = this->xCounter;
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx2;
                this->xCounter++;
            }
        }
    } else {
        if (inputIdx1 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx1 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
            } else {
                inputIdx1 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                this->xCounter++;
            }
        }
        if (inputIdx2 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx2 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
            } else {
                inputIdx2 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
                this->xCounter++;
            }
        }
    }

    int outputIdx = this->xCounter;
    if (ifSboxFuncCall) {
        this->sboxNameMxIndex[result->getNodeName()] = outputIdx;
    } else {
        this->tanNameMxIndex[result->getNodeName()] = outputIdx;
    }
    this->xCounter++;
    switch (this->xorConSel) {
        case 1:
            DiffMILPcons::bXorC1(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->dCounter);
            break;
        case 2:
            DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
            break;
        case 3:
            DiffMILPcons::bXorC3(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->dCounter);
            break;
        default:
            DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
            break;
    }
}


void DiffSBMILP::ANDandORGenModel(const ThreeAddressNodePtr &left, const ThreeAddressNodePtr &right,
                                  const ThreeAddressNodePtr &result, bool ifSboxFuncCall) {
    int inputIdx1 = 0, inputIdx2 = 0;
    if (ifSboxFuncCall) {
        for (const auto& pair : this->sboxNameMxIndex) {
            if (pair.first == left->getNodeName())
                inputIdx1 = pair.second;
            if (pair.first == right->getNodeName())
                inputIdx2 = pair.second;
        }
    } else {
        for (const auto& pair : this->tanNameMxIndex) {
            if (pair.first == left->getNodeName())
                inputIdx1 = pair.second;
            if (pair.first == right->getNodeName())
                inputIdx2 = pair.second;
        }
    }

    if (ifSboxFuncCall) {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        } else if (left->getOp() == ASTNode::NOT) {
            if (this->sboxNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                this->sboxNameMxIndex[left->getNodeName()] = this->sboxNameMxIndex[left->getLhs()->getNodeName()];
                inputIdx1 = this->sboxNameMxIndex[left->getNodeName()];
            } else
                assert(false);
        }

        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->sboxNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        } else if (right->getOp() == ASTNode::NOT) {
            if (this->sboxNameMxIndex.count(right->getLhs()->getNodeName()) != 0) {
                this->sboxNameMxIndex[right->getNodeName()] = this->sboxNameMxIndex[right->getLhs()->getNodeName()];
                inputIdx2 = this->sboxNameMxIndex[right->getNodeName()];
            } else
                assert(false);
        }
    } else {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        } else if (left->getOp() == ASTNode::NOT) {
            if (this->tanNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                this->tanNameMxIndex[left->getNodeName()] = this->tanNameMxIndex[left->getLhs()->getNodeName()];
                inputIdx1 = this->tanNameMxIndex[left->getNodeName()];
            } else
                assert(false);
        }

        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->tanNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        } else if (right->getOp() == ASTNode::NOT) {
            if (this->tanNameMxIndex.count(right->getLhs()->getNodeName()) != 0) {
                this->tanNameMxIndex[right->getNodeName()] = this->tanNameMxIndex[right->getLhs()->getNodeName()];
                inputIdx2 = this->tanNameMxIndex[right->getNodeName()];
            } else
                assert(false);
        }
    }

    if (ifSboxFuncCall) {
        if (inputIdx1 == 0) {
            if (!this->sboxRtnIdxSave.empty()) {
                inputIdx1 = this->sboxRtnIdxSave.front();
                this->sboxRtnIdxSave.erase(this->sboxRtnIdxSave.cbegin());
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
            } else {
                inputIdx1 = this->xCounter;
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
                this->xCounter++;
            }
        }
        if (inputIdx2 == 0) {
            if (!this->sboxRtnIdxSave.empty()) {
                inputIdx2 = this->sboxRtnIdxSave.front();
                this->sboxRtnIdxSave.erase(this->sboxRtnIdxSave.cbegin());
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx2;
            } else {
                inputIdx2 = this->xCounter;
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx2;
                this->xCounter++;
            }
        }
    } else {
        if (inputIdx1 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx1 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
            } else {
                inputIdx1 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                this->xCounter++;
            }
        }
        if (inputIdx2 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx2 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
            } else {
                inputIdx2 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
                this->xCounter++;
            }
        }
    }

    int outputIdx = this->xCounter;
    this->tanNameMxIndex[result->getNodeName()] = outputIdx;
    this->xCounter++;
    if (mode == "AS")
        DiffMILPcons::bAndOrC(this->modelPath, inputIdx1, inputIdx2, outputIdx);
    else if (mode == "DC")
        DiffMILPcons::dAndOrC(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->PCounter);
    else
        assert(false);
}


void DiffSBMILP::SboxGenModel(const ThreeAddressNodePtr &sbox, const ThreeAddressNodePtr &input,
                              const ThreeAddressNodePtr &output) {
    std::vector<int> inputIdx = extIdxFromTOUINTorBOXINDEX(input);
    int outputSize = sboxOutputSize[sbox->getNodeName()];
    std::vector<int> outputIdx;
    for (int i = 0; i < outputSize; ++i) {
        outputIdx.push_back(this->xCounter);
        this->tanNameMxIndex[output->getNodeName() + "_$B$_" + std::to_string(i)] = this->xCounter;
        this->xCounter++;
    }

    if (this->mode == "AS") {
        switch (this->sboxConSel) {
            case 1:
                DiffMILPcons::bSboxC1(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->branchNumMin[sbox->getNodeName()], this->dCounter, this->ACounter, sboxIfInjective[sbox->getNodeName()]);
                break;
            case 2:
                DiffMILPcons::bSboxC2(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->ACounter, sboxIfInjective[sbox->getNodeName()]);
                break;
            case 3:
                DiffMILPcons::bSboxC3(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->ACounter);
                break;
            default:
                DiffMILPcons::bSboxC2(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->ACounter, sboxIfInjective[sbox->getNodeName()]);
        }
    } else if (this->mode == "DC") {
        switch (this->sboxConSel) {
            case 1:
                DiffMILPcons::dSboxC1(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->branchNumMin[sbox->getNodeName()], this->dCounter, this->PCounter, sboxIfInjective[sbox->getNodeName()], this->sboxExtWeighted.size());
                break;
            case 2:
                DiffMILPcons::dSboxC2(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->PCounter, sboxIfInjective[sbox->getNodeName()], this->sboxExtWeighted.size());
                break;
            case 3:
                DiffMILPcons::dSboxC3(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->PCounter, this->sboxExtWeighted.size());
                break;
            default:
                DiffMILPcons::dSboxC2(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->PCounter, sboxIfInjective[sbox->getNodeName()], this->sboxExtWeighted.size());
        }
    } else
        assert(false);
}

void DiffSBMILP::PboxGenModel(const ThreeAddressNodePtr &pbox, const ThreeAddressNodePtr &input,
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


void DiffSBMILP::MatrixVectorGenModel(const ThreeAddressNodePtr &pboxm, const ThreeAddressNodePtr &input,
                                      const vector<ThreeAddressNodePtr> &output) {
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

    std::vector<std::vector<int>> Matrix;
    std::vector<int> pboxmValue = pboxM[pboxm->getNodeName()];
    int rowSize = inputTAN.size(), rowCounter = 0;
    std::vector<int> tempRow;
    for (int & i : pboxmValue) {
        tempRow.push_back(i);
        rowCounter++;
        if (rowCounter == rowSize) {
            Matrix.push_back(tempRow);
            tempRow.clear();
            rowCounter = 0;
        }
    }
    tempRow.clear();
    rowCounter = 0;
    std::vector<int> ffmValue = Ffm[pboxm->getNodeName()];
    std::vector<std::vector<int>> Ffm;
    int ffmRowSize = int(sqrt(ffmValue.size()));
    for (int & i : ffmValue) {
        tempRow.push_back(i);
        rowCounter++;
        if (rowCounter == ffmRowSize) {
            Ffm.push_back(tempRow);
            tempRow.clear();
            rowCounter = 0;
        }
    }

    int eleSize;
    if (inputTAN[0]->getNodeType() == NodeType::PARAMETER)
        eleSize = 1;
    else
        eleSize = transNodeTypeSize(inputTAN[0]->getNodeType());

    if (eleSize != 1) {
        std::vector<std::vector<std::string>> MatrixStr;
        for (const auto& row : Matrix) {
            std::vector<std::string> tt;
            for (auto ele: row) {
                std::string eleB = std::to_string(utilities::d_to_b(ele));
                for (int i = eleB.size(); i < eleSize; ++i)
                    eleB = "0" + eleB;
                tt.push_back(eleB);
            }
            MatrixStr.push_back(tt);
        }
        int module = Ffm[2][int(pow(2, (eleSize - 1)))];
        std::string moduleStr = std::to_string(utilities::d_to_b(module));
        for (int i = moduleStr.size(); i < eleSize; ++i)
            moduleStr = "0" + moduleStr;
        std::vector<int> moduleVec;
        for (char i : moduleStr) {
            if (i == '0') moduleVec.push_back(0);
            else if (i == '1') moduleVec.push_back(1);
        }

        std::map<ThreeAddressNodePtr, std::vector<ThreeAddressNodePtr>> MoutMap;
        std::map<ThreeAddressNodePtr, std::vector<std::string>> MoutMapMatrix;
        for (int i = 0; i < output.size(); ++i) {
            std::vector<ThreeAddressNodePtr> MoutEleMap;
            std::vector<std::string> MoutEleMapMatrix;
            for (int j = 0; j < rowSize; ++j) {
                MoutEleMap.push_back(inputTAN[j]);
                MoutEleMapMatrix.push_back(MatrixStr[i][j]);
            }
            MoutMap[output[i]] = MoutEleMap;
            MoutMapMatrix[output[i]] = MoutEleMapMatrix;
        }

        std::map<ThreeAddressNodePtr, std::vector<int>> MoutIdxMap;
        std::map<ThreeAddressNodePtr, std::vector<int>> MintIdxMap;
        for (auto ele : inputTAN) {
            std::vector<int> inputIdx = extIdxFromTOUINTorBOXINDEX(ele);
            MintIdxMap[ele] = inputIdx;
        }
        for (auto ele : output) {
            std::vector<int> outputIdx;
            for (int i = 0; i < eleSize; ++i) {
                this->tanNameMxIndex[ele->getNodeName() + "_$B$_" + std::to_string(i)] = this->xCounter;
                outputIdx.push_back(this->xCounter);
                this->xCounter++;
            }
            MoutIdxMap[ele] = outputIdx;
        }

        std::map<std::vector<int>, std::vector<std::vector<int>>> MoutIdxIntIdxes;
        std::map<std::vector<int>, std::vector<int>> MoutIdxOverflowIdxes;

        auto item = MoutMap.begin();
        auto itemMoutMatrix = MoutMapMatrix.begin();
        for (int i = 0; i < MoutMap.size(); ++i) {
            std::vector<std::vector<int>> tMapIdxes;
            std::vector<int> tOverflows;
            for (int j = 0; j < itemMoutMatrix->second.size(); ++j) {
                std::vector<int> x_i = MintIdxMap[item->second[j]];
                std::string M_ij = itemMoutMatrix->second[j];
                int shiftNum = 0;

                for (int k = 0; k < eleSize; ++k) {
                    if (M_ij[eleSize - 1 - k] == '1') {
                        if (k > 0) {
                            for (int l = 0; l < k; ++l) {
                                auto finder = std::find(tOverflows.begin(), tOverflows.end(), x_i[l]);
                                if (finder != tOverflows.end())
                                    tOverflows.erase(finder);
                                else
                                    tOverflows.push_back(x_i[l]);
                            }
                            tMapIdxes.push_back(shiftVec(x_i, k));
                        } else {
                            tMapIdxes.push_back(x_i);
                        }
                    } else if (M_ij[eleSize - 1 - k] == '0')
                        continue;
                    shiftNum++;
                }
            }
            MoutIdxIntIdxes[MoutIdxMap[item->first]] = tMapIdxes;
            MoutIdxOverflowIdxes[MoutIdxMap[item->first]] = tOverflows;
            item++;
            itemMoutMatrix++;
        }

        auto itemMidx = MoutIdxIntIdxes.begin();
        auto itemMove = MoutIdxOverflowIdxes.begin();
        for (int i = 0; i < MoutIdxIntIdxes.size(); ++i) {
            if (matrixConSel == 1 or matrixConSel == 2) {
                DiffMILPcons::bMatrixEntryC12(this->modelPath, itemMidx->second, itemMidx->first, itemMove->second, moduleVec,
                                              this->xCounter, this->yCounter, this->fCounter, this->dCounter, this->matrixConSel);
            } else if (matrixConSel == 2) {
                DiffMILPcons::bMatrixEntryC3(this->modelPath, itemMidx->second, itemMidx->first, itemMove->second, moduleVec,
                                             this->xCounter, this->yCounter, this->fCounter);
            }
            itemMidx++;
            itemMove++;
        }
    } else {
        std::map<ThreeAddressNodePtr, std::vector<ThreeAddressNodePtr>> MoutMap;
        for (int i = 0; i < output.size(); ++i) {
            std::vector<ThreeAddressNodePtr> MoutEleMap;
            for (int j = 0; j < rowSize; ++j) {
                if (Ffm[Matrix[i][j]][1] == 1) {
                    MoutEleMap.push_back(inputTAN[j]);
                }
            }
            MoutMap[output[i]] = MoutEleMap;
        }

        for (auto pair : MoutMap) {
            if (pair.second.size() >= 2) {
                if (this->matrixConSel == 1 or this->matrixConSel == 2) {
                    multiXORGenModel12(pair.second, pair.first);
                } else if (this->matrixConSel == 3) {
                    multiXORGenModel3(pair.second, pair.first);
                }
            }
            else {
                int rrr = this->tanNameMxIndex[pair.second.at(0)->getNodeName()];
                this->tanNameMxIndex[pair.first->getNodeName()] = this->tanNameMxIndex[pair.second.at(0)->getNodeName()];
            }
        }
    }
}


void DiffSBMILP::multiXORGenModel12(std::vector<ThreeAddressNodePtr> input, ThreeAddressNodePtr result) {
    ThreeAddressNodePtr input1 = input[0], input2 = input[1];
    int inputIdx1 = 0, inputIdx2 = 0;
    for (const auto& pair : this->tanNameMxIndex) {
        if (pair.first == input1->getNodeName())
            inputIdx1 = pair.second;
        if (pair.first == input2->getNodeName())
            inputIdx2 = pair.second;
    }
    if (inputIdx1 == 0) {
        if (!this->rtnIdxSave.empty()) {
            inputIdx1 = this->rtnIdxSave.front();
            this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
            this->tanNameMxIndex[input1->getNodeName()] = inputIdx1;
        } else {
            inputIdx1 = this->xCounter;
            this->tanNameMxIndex[input1->getNodeName()] = inputIdx1;
            this->xCounter++;
        }
    }
    if (inputIdx2 == 0) {
        if (!this->rtnIdxSave.empty()) {
            inputIdx2 = this->rtnIdxSave.front();
            rtnIdxSave.erase(this->rtnIdxSave.cbegin());
            this->tanNameMxIndex[input2->getNodeName()] = inputIdx2;
        } else {
            inputIdx2 = this->xCounter;
            this->tanNameMxIndex[input2->getNodeName()] = inputIdx2;
            this->xCounter++;
        }
    }

    int outputIdx = this->xCounter;
    this->xCounter++;
    if (input.size() == 2)
        this->tanNameMxIndex[result->getNodeName()] = outputIdx;
    switch (matrixConSel) {
        case 1:
            DiffMILPcons::bXorC1(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->dCounter);
            break;
        case 2:
            DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
            break;
        default:
            DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
    }

    if (input.size() > 2) {
        for (int i = 2; i < input.size(); ++i) {
            inputIdx1 = outputIdx;
            ThreeAddressNodePtr inputNew = input[i];
            for (const auto& pair : this->tanNameMxIndex)
                if (pair.first == inputNew->getNodeName())
                    inputIdx2 = pair.second;

            if (inputIdx2 == 0) {
                if (!this->rtnIdxSave.empty()) {
                    inputIdx2 = this->rtnIdxSave.front();
                    this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                    this->tanNameMxIndex[inputNew->getNodeName()] = inputIdx2;
                } else {
                    inputIdx2 = this->xCounter;
                    this->tanNameMxIndex[inputNew->getNodeName()] = inputIdx2;
                    this->xCounter++;
                }
            }

            outputIdx = this->xCounter;
            this->xCounter++;
            if (i == input.size() - 1)
                this->tanNameMxIndex[result->getNodeName()] = outputIdx;
            switch (matrixConSel) {
                case 1:
                    DiffMILPcons::bXorC1(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->dCounter);
                    break;
                case 2:
                    DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
                    break;
                default:
                    DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
            }
        }
    }
}


void DiffSBMILP::multiXORGenModel3(std::vector<ThreeAddressNodePtr> input, ThreeAddressNodePtr result) {
    std::vector<int> inputIdx;
    for (auto ele : input) {
        bool finder = false;
        for (const auto& pair : this->tanNameMxIndex) {
            if (pair.first == ele->getNodeName()) {
                inputIdx.push_back(pair.second);
                finder = true;
            }
        }
        if (!finder) {
            inputIdx.push_back(0);
        }
    }

    for (int i = 0; i < inputIdx.size(); ++i) {
        if (inputIdx[i] == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx[i] = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[input[i]->getNodeName()] = inputIdx[i];
            } else {
                inputIdx[i] = this->xCounter;
                this->tanNameMxIndex[input[i]->getNodeName()] = inputIdx[i];
                this->xCounter++;
            }
        }
    }

    int outputIdx = this->xCounter;
    this->xCounter++;
    this->tanNameMxIndex[result->getNodeName()] = outputIdx;
    DiffMILPcons::bNXorC3(this->modelPath, inputIdx, outputIdx, this->yCounter);
}


void DiffSBMILP::ADDandMINUSGenModel(vector<ThreeAddressNodePtr> &input1, vector<ThreeAddressNodePtr> &input2,
                                     const vector<ThreeAddressNodePtr> &output, int AddOrMinus, bool ifSboxFuncCall) {
    std::vector<int> inputIdx1, inputIdx2;
    std::vector<std::vector<ThreeAddressNodePtr>> input = {input1, input2};
    if (ifSboxFuncCall) {
        for (auto in: input1) {
            if (this->sboxNameMxIndex.count(in->getNodeName()) != 0) {
                inputIdx1.push_back(this->sboxNameMxIndex.find(in->getNodeName())->second);
            } else if (!this->sboxRtnIdxSave.empty()) {
                this->sboxNameMxIndex[in->getNodeName()] = this->sboxRtnIdxSave.front();
                inputIdx1.push_back(this->sboxRtnIdxSave.front());
                this->sboxRtnIdxSave.erase(this->sboxRtnIdxSave.cbegin());
            } else {
                this->sboxNameMxIndex[in->getNodeName()] = this->xCounter;
                inputIdx1.push_back(this->xCounter);
                this->xCounter++;
            }
        }
    } else {
        for (auto in: input1) {
            if (this->tanNameMxIndex.count(in->getNodeName()) != 0) {
                inputIdx1.push_back(this->tanNameMxIndex.find(in->getNodeName())->second);
            } else if (!this->rtnIdxSave.empty()) {
                this->tanNameMxIndex[in->getNodeName()] = this->rtnIdxSave.front();
                inputIdx1.push_back(this->rtnIdxSave.front());
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
            } else {
                this->tanNameMxIndex[in->getNodeName()] = this->xCounter;
                inputIdx1.push_back(this->xCounter);
                this->xCounter++;
            }
        }
    }

    if (ifSboxFuncCall) {
        for (auto in: input2) {
            if (this->sboxNameMxIndex.count(in->getNodeName()) != 0) {
                inputIdx2.push_back(this->sboxNameMxIndex.find(in->getNodeName())->second);
            } else if (!this->sboxRtnIdxSave.empty()) {
                this->sboxNameMxIndex[in->getNodeName()] = this->sboxRtnIdxSave.front();
                inputIdx2.push_back(this->sboxRtnIdxSave.front());
                this->sboxRtnIdxSave.erase(this->sboxRtnIdxSave.cbegin());
            } else {
                this->sboxNameMxIndex[in->getNodeName()] = this->xCounter;
                inputIdx2.push_back(this->xCounter);
                this->xCounter++;
            }
        }
    } else {
        for (auto in: input2) {
            if (this->tanNameMxIndex.count(in->getNodeName()) != 0) {
                inputIdx2.push_back(this->tanNameMxIndex.find(in->getNodeName())->second);
            } else if (!this->rtnIdxSave.empty()) {
                this->tanNameMxIndex[in->getNodeName()] = this->rtnIdxSave.front();
                inputIdx2.push_back(this->rtnIdxSave.front());
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
            } else {
                this->tanNameMxIndex[in->getNodeName()] = this->xCounter;
                inputIdx2.push_back(this->xCounter);
                this->xCounter++;
            }
        }
    }

    std::vector<int> outputIdx;
    if (ifSboxFuncCall) {
        for (const auto& ele : output) {
            this->sboxNameMxIndex[ele->getNodeName()] = this->xCounter;
            outputIdx.push_back(this->xCounter);
            this->xCounter++;
        }
    } else {
        for (const auto& ele : output) {
            this->tanNameMxIndex[ele->getNodeName()] = this->xCounter;
            outputIdx.push_back(this->xCounter);
            this->xCounter++;
        }
    }

    if (this->mode == "AS") {
        if (AddOrMinus == 1)
            DiffMILPcons::bAddC(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->ARXineqs, dCounter, PCounter);
        else if (AddOrMinus == 2)
            DiffMILPcons::bAddC(this->modelPath, outputIdx, inputIdx2, inputIdx1, this->ARXineqs, dCounter, PCounter);
        else
            assert(false);
    } else if (this->mode == "DC") {
        if (AddOrMinus == 1)
            DiffMILPcons::dAddC(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->ARXineqs, dCounter, PCounter);
        else if (AddOrMinus == 2)
            DiffMILPcons::dAddC(this->modelPath, outputIdx, inputIdx2, inputIdx1, this->ARXineqs, dCounter, PCounter);
        else
            assert(false);
    } else
        assert(false);
}
