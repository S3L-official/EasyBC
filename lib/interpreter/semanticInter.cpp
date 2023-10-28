//
// Created by Septi on 9/18/2023.
//
#include "interpreter/semanticInter.h"

extern std::map<std::string, std::vector<int>> allBox;
extern std::map<std::string, std::vector<int>> pboxM;
extern std::map<std::string, int> pboxMSize;
extern std::map<std::string, std::vector<int>> Ffm;
extern std::string cipherName;

EasyBCInter::EasyBCInter(std::vector<ProcedureHPtr> procedureHs, std::vector<int> plaintext,
                         std::vector<std::vector<int>> subkeys)
        : procedureHs(std::move(procedureHs)), plaintext(std::move(plaintext)), subkeys(std::move(subkeys)) {
    this->Box = allBox;
    this->cipherName = ::cipherName;

    std::cout << "\nthe encryption of " << this->cipherName << std::endl;
}

void EasyBCInter::interpreter() {
    for (const auto& proc : this->procedureHs) {
        if (proc->getName() == "main") {
            if (this->plaintext.size() != proc->getParameters().at(1).size()) {
                std::cout << "the given plaintext is illegal !" << std::endl;
                assert(false);
            } else {
                for (int i = 0; i < proc->getParameters().at(1).size(); ++i) {
                    proc->getParameters().at(1)[i]->setThreeAddressNodeValue(this->plaintext[i]);
                    this->mainTanNameMVal[proc->getParameters().at(1)[i]->getNodeName()] = this->plaintext[i];
                }
            }

            for (int idx = 0; idx < proc->getBlock().size(); ++idx) {
                ThreeAddressNodePtr ele = proc->getBlock().at(idx);
                if (ele->getNodeName() == "push" and ele->getOp() == ASTNode::PUSH) {
                    std::vector<int> paramR, paramSK, paramP;
                    paramR.push_back(std::stoi(ele->getLhs()->getNodeName()));
                    int subKeyValIdx = 0;
                    while (true) {
                        idx++;
                        ele = proc->getBlock().at(idx);
                        if (ele->getOp() == ASTNode::CALL) {
                            idx--;
                            ele = proc->getBlock().at(idx);
                            break;
                        } else if (ele->getNodeName() == "key_push" and ele->getOp() == ASTNode::PUSH) {
                            paramSK.push_back(subkeys[paramR[0]-1][subKeyValIdx]);
                            subKeyValIdx++;
                        } else if (ele->getNodeName() == "plaintext_push" and ele->getOp() == ASTNode::PUSH) {
                            paramP.push_back(mainTanNameMVal[ele->getLhs()->getNodeName()]);
                        }
                    }
                    this->rndParamMVal["r"] = paramR;
                    this->rndParamMVal["sk"] = paramSK;
                    this->rndParamMVal["p"] = paramP;
                }

                if (ele->getOp() == ASTNode::CALL) {
                    std::string roundFuncId = ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@"));
                    std::vector<std::string> roundFuncRtnTan;
                    while (true) {
                        roundFuncRtnTan.push_back(ele->getNodeName());
                        idx++;
                        if (idx < proc->getBlock().size())
                            ele = proc->getBlock().at(idx);
                        else
                            break;
                        if (ele->getOp() != ASTNode::CALL) {
                            idx--;
                            ele = proc->getBlock().at(idx);
                            break;
                        }
                    }
                    std::vector<int> roundFuncRtnVal;
                    for (const auto& tproc : this->procedureHs) {
                        if (tproc->getName() == roundFuncId) {
                            roundFuncRtnVal = roundFuncEnc(tproc);
                            break;
                        }
                    }

                    for (int i = 0; i < roundFuncRtnVal.size(); ++i) {
                        this->mainTanNameMVal[roundFuncRtnTan[i]] = roundFuncRtnVal[i];
                    }
                }
            }

            for (auto c : proc->getReturns()) {
                this->ciphertext.push_back(this->mainTanNameMVal[c->getNodeName()]);
            }
        }
    }
}

std::vector<int> EasyBCInter::roundFuncEnc(const ProcedureHPtr& procedureH) {

    this->rndTanNameMVal.clear();
    this->rndTanNameMVal[procedureH->getParameters().at(0)[0]->getNodeName()] =
            this->rndParamMVal["r"][0];
    for (int i = 0; i < procedureH->getParameters().at(1).size(); ++i) {
        this->rndTanNameMVal[procedureH->getParameters().at(1)[i]->getNodeName()] =
                this->rndParamMVal["sk"][i];
    }
    for (int i = 0; i < procedureH->getParameters().at(2).size(); ++i) {
        this->rndTanNameMVal[procedureH->getParameters().at(2)[i]->getNodeName()] =
                this->rndParamMVal["p"][i];
    }

    for (int i = 0; i < procedureH->getBlock().size(); ++i) {
        ThreeAddressNodePtr ele = procedureH->getBlock().at(i);
        if (ele->getOp() == ASTNode::XOR) {
            this->rndTanNameMVal[ele->getNodeName()] = xorEnc(ele->getLhs(), ele->getRhs(), ele);
        } else if (ele->getOp() == ASTNode::AND) {
            this->rndTanNameMVal[ele->getNodeName()] = andEnc(ele->getLhs(), ele->getRhs(), ele);
        } else if (ele->getOp() == ASTNode::OR) {
            this->rndTanNameMVal[ele->getNodeName()] = orEnc(ele->getLhs(), ele->getRhs(), ele);
        } else if (ele->getOp() == ASTNode::BOXOP) {
            if (ele->getLhs()->getNodeName().substr(0, 4) == "sbox") {
                this->rndTanNameMVal[ele->getNodeName()] = sboxEnc(ele->getLhs(), ele->getRhs(), ele);
            } else if (ele->getLhs()->getNodeName().substr(0, 4) == "pbox") {
                this->rndTanNameMVal[ele->getNodeName()] = pboxEnc(ele->getLhs(), ele->getRhs(), ele);
            }
        } else if (ele->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[ele->getNodeName()] = symbolIndexEnc(ele->getLhs(), ele->getRhs(), ele);
        } else if (ele->getOp() == ASTNode::ADD) {
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
            std::vector<int> outputVal = modularAddEnc(input1, input2, output);
            for (int j = 0; j < output.size(); ++j) {
                this->rndTanNameMVal[output[j]->getNodeName()] = outputVal[j];
            }

        } else if (ele->getOp() == ASTNode::MINUS) {
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
            std::vector<int> outputVal = modularMinusEnc(input1, input2, output);
            for (int j = 0; j < output.size(); ++j) {
                this->rndTanNameMVal[output[j]->getNodeName()] = outputVal[j];
            }

        } else if (ele->getOp() == ASTNode::TOUINT) {
            this->rndTanNameMVal[ele->getNodeName()] = touintEnc(ele->getLhs(), ele->getRhs(), ele);
        } else if (ele->getOp() == ASTNode::MOD) {
            this->rndTanNameMVal[ele->getNodeName()] = modeEnc(ele->getLhs(), ele->getRhs(), ele);
        } else if (ele->getOp() == ASTNode::NOT) {
            this->rndTanNameMVal[ele->getNodeName()] = notEnc(ele->getLhs(), ele);
        } else if (ele->getNodeType() == PARAMETER) {

        } else
            assert(false);
    }

    std::vector<int> rtnVal;
    for (const auto & i : procedureH->getReturns()) {
        if (this->rndTanNameMVal.find(i->getNodeName()) != this->rndTanNameMVal.end())
            rtnVal.push_back(this->rndTanNameMVal[i->getNodeName()]);
        else if (i->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[i->getNodeName()] = symbolIndexEnc(i->getLhs(), i->getRhs(), i);
            rtnVal.push_back(this->rndTanNameMVal[i->getNodeName()]);
        }
        else {
            std::cout << "Round encryption error : an non-01 value is thrown : " <<
                 i->getNodeName() << " -> " << this->rndTanNameMVal[i->getNodeName()] << std::endl;
            assert(false);
        }
    }
    return rtnVal;
}

int EasyBCInter::xorEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
        if (left->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[left->getNodeName()] = symbolIndexEnc(left->getLhs(), left->getRhs(), left);
        } else if (left->getOp() == ASTNode::NULLOP and left->getNodeType() != PARAMETER) {
            this->rndTanNameMVal[left->getNodeName()] = std::stoi(left->getNodeName());
        } else if (left->getOp() == ASTNode::ADD) {
        } else if (left->getNodeType() == PARAMETER) {

        } else if (left->getOp() == ASTNode::NOT) {
            this->rndTanNameMVal[left->getNodeName()] = notEnc(left->getLhs(), left);
        } else
            assert(false);
    }
    if (this->rndTanNameMVal.find(right->getNodeName()) == this->rndTanNameMVal.end() or right->getOp() == ASTNode::INDEX) {
        if (right->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[right->getNodeName()] = symbolIndexEnc(right->getLhs(), right->getRhs(), right);
        } else if (right->getOp() == ASTNode::NULLOP and right->getNodeType() != PARAMETER) {
            this->rndTanNameMVal[right->getNodeName()] = std::stoi(right->getNodeName());
        } else if (right->getOp() == ASTNode::ADD) {

        } else if (right->getNodeType() == PARAMETER) {

        } else if (right->getOp() == ASTNode::NOT) {
            this->rndTanNameMVal[right->getNodeName()] = notEnc(right->getLhs(), right);
        } else if (right->getOp() == ASTNode::INDEX) {
            int index;
            if (right->getRhs()->getOp() == ASTNode::MOD) {
                index = modeEnc(right->getRhs()->getLhs(), right->getRhs()->getRhs(), right->getRhs());
            } else if (right->getRhs()->getOp() == ASTNode::MINUS) {
                index = minusEnc(right->getRhs()->getLhs(), right->getRhs()->getRhs(), right->getRhs());
            } else
                assert(false);
            std::string indexStr = right->getLhs()->getNodeName() + "_" + to_string(index);
            this->rndTanNameMVal[right->getNodeName()] = this->rndTanNameMVal[indexStr];
        } else
            assert(false);
    }

    int outputVal = this->rndTanNameMVal[left->getNodeName()] xor this->rndTanNameMVal[right->getNodeName()];
    output->setThreeAddressNodeValue(outputVal);
    return outputVal;
}

int EasyBCInter::andEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
        if (left->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[left->getNodeName()] = symbolIndexEnc(left->getLhs(), left->getRhs(), left);
        } else if (left->getOp() == ASTNode::NOT) {
            this->rndTanNameMVal[left->getNodeName()] = notEnc(left->getLhs(), left);
        } else if (left->getOp() == ASTNode::NULLOP) {
            this->rndTanNameMVal[left->getNodeName()] = stoi(left->getNodeName());
        } else
            assert(false);
    }
    if (this->rndTanNameMVal.find(right->getNodeName()) == this->rndTanNameMVal.end()) {
        if (right->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[right->getNodeName()] = symbolIndexEnc(right->getLhs(), right->getRhs(), right);
        } else if (right->getOp() == ASTNode::NOT) {
            this->rndTanNameMVal[right->getNodeName()] = notEnc(right->getLhs(), right);
        } else if (right->getOp() == ASTNode::NULLOP) {
            this->rndTanNameMVal[right->getNodeName()] = stoi(right->getNodeName());
        } else
            assert(false);
    }
    int outputVal = this->rndTanNameMVal[left->getNodeName()] and this->rndTanNameMVal[right->getNodeName()];
    output->setThreeAddressNodeValue(outputVal);
    return outputVal;
}

int EasyBCInter::orEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
        if (left->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[left->getNodeName()] = symbolIndexEnc(left->getLhs(), left->getRhs(), left);
        } else if (left->getOp() == ASTNode::NOT) {
            this->rndTanNameMVal[left->getNodeName()] = notEnc(left->getLhs(), left);
        } else if (left->getOp() == ASTNode::NULLOP) {
            this->rndTanNameMVal[left->getNodeName()] = stoi(left->getNodeName());
        } else
            assert(false);
    }
    if (this->rndTanNameMVal.find(right->getNodeName()) == this->rndTanNameMVal.end()) {
        if (right->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[right->getNodeName()] = symbolIndexEnc(right->getLhs(), right->getRhs(), right);
        } else if (right->getOp() == ASTNode::NOT) {
            this->rndTanNameMVal[right->getNodeName()] = notEnc(right->getLhs(), right);
        } else if (right->getOp() == ASTNode::NULLOP) {
            this->rndTanNameMVal[right->getNodeName()] = stoi(right->getNodeName());
        } else
            assert(false);
    }
    int outputVal = this->rndTanNameMVal[left->getNodeName()] or this->rndTanNameMVal[right->getNodeName()];
    output->setThreeAddressNodeValue(outputVal);
    return outputVal;
}

int EasyBCInter::sboxEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    std::vector<int> inputVal = extValueFromTOUINTorBOXINDEX(right);
    std::string inputStr;
    for (auto b : inputVal) inputStr += to_string(b);

    std::vector<int> sbox = this->Box[left->getNodeName()];
    this->sboxInputSize = int(log2(sbox.size()));
    this->sboxOutputSize = int(log2(sbox[std::max_element(sbox.begin(), sbox.end()) - sbox.begin()] + 1));
    int inputDec;
    if (this->cipherName != "DES") {
        inputDec = utilities::b_to_d(std::stoi(inputStr));
    } else {
        inputDec = desSbox(inputStr);
    }
    int outputDec = this->Box[left->getNodeName()][inputDec];
    output->setThreeAddressNodeValue(outputDec);
    return outputDec;
}

int EasyBCInter::pboxEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    std::vector<int> inputVal = extValueFromTOUINTorBOXINDEX(right);

    int outSize = this->Box[left->getNodeName()].size();

    std::vector<int> outputVal;
    for (int j = 0; j < outSize; ++j)
        outputVal.push_back(0);
    for (int j = 0; j < outSize; ++j) {
        outputVal[j] = inputVal[this->Box[left->getNodeName()][j]];
    }
    std::string outputBinStr;
    for (int i = 0; i < outputVal.size(); ++i) {
        outputBinStr += to_string(outputVal[i]);
        if (i < outputVal.size() - 1)
            outputBinStr += ",";
    }

    this->pboxTanNameMStr[output->getNodeName()] = outputBinStr;
    output->setThreeAddressNodeValue(-2);
    return -2;
}

int EasyBCInter::ffTimesEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    std::vector<int> inputVal = extValueFromTOUINTorBOXINDEX(right);
    std::vector<std::vector<int>> Matrix; // matrix
    std::vector<int> pboxmValue = pboxM[left->getNodeName()];
    int rowSize = inputVal.size(), rowCounter = 0;
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
    std::vector<int> ffmValue = Ffm[left->getNodeName()];
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

    std::vector<int> outputVal;
    for (int j = 0; j < inputVal.size(); ++j) {
        int t = 0;
        for (int k = 0; k < inputVal.size(); ++k) {
            t ^= Ffm[Matrix[j][k]][inputVal[k]];
        }
        outputVal.push_back(t);
    }

    this->matVecProTanNameMStr[output->getNodeName()] = outputVal;
    output->setThreeAddressNodeValue(-3);
    return -3;
}

int EasyBCInter::symbolIndexEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    int outputVal;
    if (left->getOp() == ASTNode::FFTIMES) {
        if (this->rndTanNameMVal[left->getNodeName()] != -3) {
            this->rndTanNameMVal[left->getNodeName()] = ffTimesEnc(left->getLhs(), left->getRhs(), left);
        }
        outputVal = this->matVecProTanNameMStr[left->getNodeName()][std::stoi(right->getNodeName())];
        output->setThreeAddressNodeValue(outputVal);
    } else if (left->getOp() == ASTNode::BOXOP) {
        if (this->rndTanNameMVal[left->getNodeName()] != -2) {
            std::string leftBinStr = std::to_string(utilities::d_to_b(this->rndTanNameMVal[left->getNodeName()]));
            if (leftBinStr.size() < this->sboxOutputSize) {
                std::string tmp;
                for (int j = 0; j < this->sboxOutputSize - leftBinStr.size(); ++j) tmp += "0";
                leftBinStr = tmp + leftBinStr;
            }
            outputVal = stoi(leftBinStr.substr(stoi(right->getNodeName()), 1));
        }
        else {
            std::vector<std::string> leftPboxOutStrs = utilities::split(this->pboxTanNameMStr[left->getNodeName()], ",");
            outputVal = stoi(leftPboxOutStrs[stoi(right->getNodeName())]);
        }
        output->setThreeAddressNodeValue(outputVal);

    } else if (left->getOp() == ASTNode::INDEX) {
        if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
            int leftVal, index;
            if (left->getRhs()->getNodeType() == PARAMETER) {
                index = this->rndParamMVal["r"][0] - 1;
            } else if (left->getRhs()->getOp() == ASTNode::MINUS) {
                index = minusEnc(left->getRhs()->getLhs(), left->getRhs()->getRhs(), left->getRhs());
            } else
                assert(false);
            leftVal = this->Box[left->getLhs()->getNodeName()][index];
            this->rndTanNameMVal[left->getNodeName()] = leftVal;
        }
        int leftValSize = getNodeTypeSize(left->getNodeType());
        std::string leftBinStr = to_string(utilities::d_to_b(this->rndTanNameMVal[left->getNodeName()]));
        std::string pre;
        for (int i = 0; i < leftValSize - leftBinStr.size(); ++i) { pre += "0";}
        leftBinStr = pre + leftBinStr;

        outputVal = stoi(leftBinStr.substr(stoi(right->getNodeName()), 1));
        output->setThreeAddressNodeValue(outputVal);
    } else if (left->getOp() == ASTNode::TOUINT) {
        if (this->rndTanNameMVal.find(left->getNodeName() + "_touint") == this->rndTanNameMVal.end()) {
            this->rndTanNameMVal[left->getNodeName() + "_touint"] = touintEnc(left->getLhs(), left->getRhs(), left);
        }
        int leftValSize = getNodeTypeSize(left->getNodeType());
        std::string leftBinStr = to_string(utilities::d_to_b(this->rndTanNameMVal[left->getNodeName() + "_touint"]));
        std::string pre;
        for (int i = 0; i < leftValSize - leftBinStr.size(); ++i) { pre += "0";}
        leftBinStr = pre + leftBinStr;

        outputVal = stoi(leftBinStr.substr(stoi(right->getNodeName()), 1));
        output->setThreeAddressNodeValue(outputVal);
    } else if (left->getOp() == ASTNode::NULLOP and left->getNodeType() != PARAMETER) {
        int leftValSize = getNodeTypeSize(left->getNodeType());
        std::string leftBinStr = to_string(utilities::d_to_b(std::stoi(left->getNodeName())));
        std::string pre;
        for (int i = 0; i < leftValSize - leftBinStr.size(); ++i) { pre += "0";}
        leftBinStr = pre + leftBinStr;

        outputVal = stoi(leftBinStr.substr(stoi(right->getNodeName()), 1));
        output->setThreeAddressNodeValue(outputVal);
    } else if (left->getNodeType() == PARAMETER) {
        int leftValSize = 8;
        std::string leftBinStr = to_string(utilities::d_to_b(this->rndTanNameMVal[left->getNodeName()]));
        std::string pre;
        for (int i = 0; i < leftValSize - leftBinStr.size(); ++i) { pre += "0";}
        leftBinStr = pre + leftBinStr;

        outputVal = stoi(leftBinStr.substr(stoi(right->getNodeName()), 1));
        output->setThreeAddressNodeValue(outputVal);
    } else if (left->getOp() == ASTNode::SYMBOLINDEX) {
        if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
            this->rndTanNameMVal[left->getNodeName()] = symbolIndexEnc(left->getLhs(), left->getRhs(), left);
        }
        int leftValSize = getNodeTypeSize(left->getNodeType());
        std::string leftBinStr = to_string(utilities::d_to_b(this->rndTanNameMVal[left->getNodeName()]));
        std::string pre;
        for (int i = 0; i < leftValSize - leftBinStr.size(); ++i) { pre += "0";}
        leftBinStr = pre + leftBinStr;

        outputVal = stoi(leftBinStr.substr(stoi(right->getNodeName()), 1));
        output->setThreeAddressNodeValue(outputVal);
    }
    else
        assert(false);
    return outputVal;
}

int EasyBCInter::touintEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    std::vector<int> inputVal = extValueFromTOUINTorBOXINDEX(left);
    std::string outputBinStr;
    for (auto val : inputVal) outputBinStr = to_string(val) + outputBinStr;
    int outputVal = utilities::b_to_d(stoi(outputBinStr));
    output->setThreeAddressNodeValue(outputVal);
    return outputVal;
}

std::vector<int> EasyBCInter::modularAddEnc(std::vector<ThreeAddressNodePtr> input1,
                        std::vector<ThreeAddressNodePtr> input2, std::vector<ThreeAddressNodePtr> output) {
    std::vector<int> input1Val, input2Val;
    for (auto & ele : input1) {
        if (this->rndTanNameMVal.find(ele->getNodeName()) == this->rndTanNameMVal.end()) {
            assert(false);
        }
        input1Val.push_back(this->rndTanNameMVal[ele->getNodeName()]);
    }
    for (auto & ele : input2) {
        if (this->rndTanNameMVal.find(ele->getNodeName()) == this->rndTanNameMVal.end()) {
            assert(false);
        }
        input2Val.push_back(this->rndTanNameMVal[ele->getNodeName()]);
    }
    std::map<int, int> idxCarriesMap;
    std::vector<int> outputValInv;
    outputValInv.push_back(input1Val[input1Val.size()-1] ^ input2Val[input2Val.size()-1]);
    idxCarriesMap[input1Val.size()-2] = input1Val[input1Val.size()-1] & input2Val[input2Val.size()-1];

    for (int i = input1Val.size()-2; i >= 0; --i) {
        outputValInv.push_back(input1Val[i] ^ input2Val[i] ^ idxCarriesMap[i]);
        if (input1Val[i] + input2Val[i] + idxCarriesMap[i] >= 2)
            idxCarriesMap[i-1] = 1;
        else
            idxCarriesMap[i-1] = 0;
    }
    std::vector<int> outputVal;
    for (int i = outputValInv.size()-1; i >= 0; --i) {
        outputVal.push_back(outputValInv[i]);
    }
    return outputVal;
}

std::vector<int> EasyBCInter::modularMinusEnc(std::vector<ThreeAddressNodePtr> input1,
                        std::vector<ThreeAddressNodePtr> input2, std::vector<ThreeAddressNodePtr> output) {
    std::vector<int> input1Val, input2Val;
    for (auto & ele : input1) {
        if (this->rndTanNameMVal.find(ele->getNodeName()) == this->rndTanNameMVal.end()) {
            assert(false);
        }
        input1Val.push_back(this->rndTanNameMVal[ele->getNodeName()]);
    }
    for (auto & ele : input2) {
        if (this->rndTanNameMVal.find(ele->getNodeName()) == this->rndTanNameMVal.end()) {
            if (ele->getOp() == ASTNode::NULLOP) {
                this->rndTanNameMVal[ele->getNodeName()] = stoi(ele->getNodeName());
            } else
                assert(false);
        }
        input2Val.push_back(this->rndTanNameMVal[ele->getNodeName()]);
    }
    std::vector<int> input2CompVal;
    for (int i : input2Val) {
        if (i == 0) input2CompVal.push_back(1);
        else if (i == 1) input2CompVal.push_back(0);
        else assert(false);
    }
    std::map<int, int> idxCarriesMap;
    std::vector<int> outputValInv;
    outputValInv.push_back(input1Val[input1Val.size()-1] ^ input2CompVal[input2CompVal.size()-1]);
    idxCarriesMap[input1Val.size()-2] = input1Val[input1Val.size()-1] & input2CompVal[input2CompVal.size()-1];
    for (int i = input1Val.size()-2; i > 0; --i) {
        outputValInv.push_back(input1Val[i] ^ input2CompVal[i] ^ idxCarriesMap[i]);
        if (input1Val[i] + input2CompVal[i] + idxCarriesMap[i] >= 2)
            idxCarriesMap[i-1] = 1;
        else
            idxCarriesMap[i-1] = 0;
    }
    std::vector<int> outputVal;
    for (int i = outputValInv.size()-1; i >= 0; --i) {
        outputVal.push_back(outputValInv[i]);
    }
    return outputVal;
}

int EasyBCInter::minusEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
        if (left->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[left->getNodeName()] = symbolIndexEnc(left->getLhs(), left->getRhs(), left);
        } else if (left->getOp() == ASTNode::NULLOP) {
            this->rndTanNameMVal[left->getNodeName()] = stoi(left->getNodeName());
        } else
            assert(false);
    }
    if (this->rndTanNameMVal.find(right->getNodeName()) == this->rndTanNameMVal.end()) {
        if (right->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[right->getNodeName()] = symbolIndexEnc(right->getLhs(), right->getRhs(), right);
        } else if (right->getOp() == ASTNode::NULLOP) {
            this->rndTanNameMVal[right->getNodeName()] = stoi(right->getNodeName());
        } else
            assert(false);
    }
    int outputVal = this->rndTanNameMVal[left->getNodeName()] - this->rndTanNameMVal[right->getNodeName()];
    output->setThreeAddressNodeValue(outputVal);
    return outputVal;
    return 0;
}


int EasyBCInter::modeEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
        if (left->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[left->getNodeName()] = symbolIndexEnc(left->getLhs(), left->getRhs(), left);
        } else if (left->getOp() == ASTNode::NULLOP) {
            this->rndTanNameMVal[left->getNodeName()] = stoi(left->getNodeName());
        } else
            assert(false);
    }
    if (this->rndTanNameMVal.find(right->getNodeName()) == this->rndTanNameMVal.end()) {
        if (right->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[right->getNodeName()] = symbolIndexEnc(right->getLhs(), right->getRhs(), right);
        } else if (right->getOp() == ASTNode::NULLOP) {
            this->rndTanNameMVal[right->getNodeName()] = stoi(right->getNodeName());
        } else
            assert(false);
    }
    int outputVal = this->rndTanNameMVal[left->getNodeName()] % this->rndTanNameMVal[right->getNodeName()];
    output->setThreeAddressNodeValue(outputVal);
    return outputVal;
}

int EasyBCInter::notEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr output) {
    if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
        if (left->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[left->getNodeName()] = symbolIndexEnc(left->getLhs(), left->getRhs(), left);
        } else if (left->getOp() == ASTNode::NULLOP) {
            this->rndTanNameMVal[left->getNodeName()] = stoi(left->getNodeName());
        } else
            assert(false);
    }
    int t = this->rndTanNameMVal[left->getNodeName()];
    if (t == 1) {
        output->setThreeAddressNodeValue(0);
        return 0;
    } else if (t == 0) {
        output->setThreeAddressNodeValue(1);
        return 1;
    } else
        assert(false);
}














