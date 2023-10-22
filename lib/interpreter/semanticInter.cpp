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
            /*
             * 对于一个给定input plaintext和每轮subkey值的implementation，我们evaluation得到最终加密ciphertext的流程应该是：
             * 1. 初始化main函数的plaintext的值，并将其保存在main函数中维护的ThreeAddressNode实例到其值的map 'mainTanNameMVal'，后续
             *    每轮的加密都是加密该plaintext或者其每轮被加密后的结果；
             * 2. 处理完main函数的参数以后，开始处理main函数的函数体，需要依次寻找到每次的round function call；
             * 3. 对于每次round function call， 其首先进行的操作是实参的 PUSH，那么我们就需要维护一个保存每轮实参值的map 'rndParamMVal',
             *    然后根据 PUSH 对应的三地址实例，寻找所有的实参ThreeAddressNode实例，并通过main函数中维护的mainTanNameMVal，构造函数中
             *    初始化的subkey的值和轮数r的PUSH值来获取所有实参对应的值；
             * 4. 在实参 PUSH 操作时候是 function call，其op是 CALL，说明这里是真正的轮函数调用，也是轮函数的返回值的被赋值对象，所以此时我
             *    们需要找到所有op为 CALL 的实例并保存在 roundFunctionRtnTan，这些即是 round function 的返回三地址实例，然后调用round
             *    function的加密处理函数roundFuncEnc来获取被加密后的值，并将获取的vector<int>保存在roundFuncRtnVal中，最后我们根据上述
             *    寻找到的 roundFunctionRtnTan 和调用 roundFuncEnc 获取的 roundFuncRtnVal 来更新 mainTanNameMVal；
             * 5. 当所有的round function call都被分析完，并且到main函数中最终的三地址实例时，说明整个implementation已经被分析完毕并，那么
             *    implementation中的main函数的返回三地址实例对应的value值，即为最终的加密结果ciphertext.
             * */

            // 初始化参数值
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
                // 如果找到 round function 第一个参数，那么就直接按照相同方式把剩下所有 round func 的参数处理结束
                if (ele->getNodeName() == "push" and ele->getOp() == ASTNode::PUSH) {
                    // 用于存放参数value的vector
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
                            /*
                             * 对于singleKey的interpreter，需要记录round function的被调用次数，也就是第几轮round，然后
                             * 根据round function num来选择对应的subkey, 此外，为了寻找对应index的subkey的值，我们需要
                             * 维护一个subKeyValIdx来保证每次可以寻找到对应的subkey值
                             * */
                            paramSK.push_back(subkeys[paramR[0]-1][subKeyValIdx]);
                            subKeyValIdx++;
                        } else if (ele->getNodeName() == "plaintext_push" and ele->getOp() == ASTNode::PUSH) {
                            // plaintext每个元素对应的value只需要访问tanNameMval中对应的mapping条目即可
                            paramP.push_back(mainTanNameMVal[ele->getLhs()->getNodeName()]);
                        }
                    }
                    this->rndParamMVal["r"] = paramR;
                    this->rndParamMVal["sk"] = paramSK;
                    this->rndParamMVal["p"] = paramP;
                }

                /*
                 * 当寻找到call的op，说明开始round function call，此时就需要根据上述提取的每轮round function的参数来进行每轮单独的加密。
                 * 具体的，我们首先将op为call的ele作为return value的ThreeAddressNode提取出来，然后调用roundFuncEnc函数获取加密后的明文
                 * 结果，并与return value的ThreeAddressNode作一一匹配，更新tanNameMval的mapping
                 * */
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

                    // debug
                    /*std::cout << "output : " << std::endl;
                    for (int i = 0; i < roundFuncRtnVal.size(); ++i) {
                        std::cout << roundFuncRtnVal[i];
                    }
                    std::cout << std::endl << std::endl;*/

                    for (int i = 0; i < roundFuncRtnVal.size(); ++i) {
                        this->mainTanNameMVal[roundFuncRtnTan[i]] = roundFuncRtnVal[i];
                    }

                    // debug
                    /*std::cout << "round results : " << std::endl;
                    int ccc1 = 0;
                    for (int i = 0; i < roundFuncRtnVal.size(); ++i) {
                        ccc1++;
                        std::cout << roundFuncRtnVal[i];
                        *//*if (ccc1 % 32 == 0)
                            cout << "\n";*//*
                    }
                    std::cout << std::endl;*/
                }
            }

            for (auto c : proc->getReturns()) {
                this->ciphertext.push_back(this->mainTanNameMVal[c->getNodeName()]);
            }
        }
    }

    // debug
    // encryption finished
    /*std::cout << "plaintext : " << std::endl << "0b";
    for (auto p : this->plaintext) std::cout << p;
    std::cout << std::endl;

    std::cout << "ciphertext : " << std::endl << "0b";
    int ccc = 0;
    for (auto c : this->ciphertext) {
        ccc++;
        std::cout << c;
        if (ccc % 32 == 0)
            cout << "\n";
    }
    std::cout << std::endl;*/
}

std::vector<int> EasyBCInter::roundFuncEnc(const ProcedureHPtr& procedureH) {

    // debug
    /*std::cout << "============================== round " << this->rndParamMVal["r"][0] << " ==============================" << std::endl;
    std::cout << "input : " << std::endl;
    for (int i = 0; i < this->rndParamMVal["p"].size(); ++i) {
        std::cout << this->rndParamMVal["p"][i];
    }
    std::cout << std::endl;
    std::cout << "subkey : " << std::endl;
    for (int i = 0; i < this->rndParamMVal["sk"].size(); ++i) {
        std::cout << this->rndParamMVal["sk"][i];
    }
    std::cout << std::endl;*/


    this->rndTanNameMVal.clear();
    // 初始化round function参数值
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
            // 所有的boxop都需要找到所有的被操作对象，然后将操作结果对应的一一赋值
            if (ele->getLhs()->getNodeName().substr(0, 4) == "sbox") {
                this->rndTanNameMVal[ele->getNodeName()] = sboxEnc(ele->getLhs(), ele->getRhs(), ele);
            } else if (ele->getLhs()->getNodeName().substr(0, 4) == "pbox") {
                this->rndTanNameMVal[ele->getNodeName()] = pboxEnc(ele->getLhs(), ele->getRhs(), ele);
            }
        } else if (ele->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[ele->getNodeName()] = symbolIndexEnc(ele->getLhs(), ele->getRhs(), ele);
        } else if (ele->getOp() == ASTNode::ADD) {
            // added in 2023.9.26
            // 这里需要考虑C++的二进制有限域加法
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

            // added in 2023.9.26
            // C++的二进制有限域减法
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
            // 无论left还是right的op是ADD（modular addition），都不需要任何操作，因为ADD操作对象是对整个数组，理论上这里碰到时已经完成了evaluation
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
            // left->right是index表达式，我们需要区分其所有情况，目前所碰到的只有left->right本身是round function参数的round number r，
            // 所以直接通过保存的参数值来确定即可
            // added in 2023.10.9
            // 目前碰到了第二种情况，left->right本身可以evaluate，而left->left是已经evaluate的一个数组
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

    // debug
//    std::cout << "xor : " << left->getNodeName() << " -> " << this->rndTanNameMVal[left->getNodeName()] << std::endl;
//    std::cout << "xor : " << right->getNodeName() << " -> " << this->rndTanNameMVal[right->getNodeName()] << std::endl;
//    std::cout << "xor : " << output->getNodeName() << " -> " << outputVal << "\n\n";

    return outputVal;
}

int EasyBCInter::andEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
        if (left->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[left->getNodeName()] = symbolIndexEnc(left->getLhs(), left->getRhs(), left);
        } else if (left->getOp() == ASTNode::NOT) {
            this->rndTanNameMVal[left->getNodeName()] = notEnc(left->getLhs(), left);
        } else
            assert(false);
    }
    if (this->rndTanNameMVal.find(right->getNodeName()) == this->rndTanNameMVal.end()) {
        if (right->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[right->getNodeName()] = symbolIndexEnc(right->getLhs(), right->getRhs(), right);
        } else if (right->getOp() == ASTNode::NOT) {
            this->rndTanNameMVal[right->getNodeName()] = notEnc(right->getLhs(), right);
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
        } else
            assert(false);
    }
    if (this->rndTanNameMVal.find(right->getNodeName()) == this->rndTanNameMVal.end()) {
        if (right->getOp() == ASTNode::SYMBOLINDEX) {
            this->rndTanNameMVal[right->getNodeName()] = symbolIndexEnc(right->getLhs(), right->getRhs(), right);
        } else
            assert(false);
    }
    int outputVal = this->rndTanNameMVal[left->getNodeName()] or this->rndTanNameMVal[right->getNodeName()];
    output->setThreeAddressNodeValue(outputVal);
    return outputVal;
}

int EasyBCInter::sboxEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    std::vector<int> inputVal = extValueFromTOUINTorBOXINDEX(right);
    // 将input values整体转换成对应的整数值，然后获取sbox substitution后的结果
    std::string inputStr;
    for (auto b : inputVal) inputStr += to_string(b);

    std::vector<int> sbox = this->Box[left->getNodeName()];
    this->sboxInputSize = int(log2(sbox.size()));
    this->sboxOutputSize = int(log2(sbox[std::max_element(sbox.begin(), sbox.end()) - sbox.begin()] + 1));
    // substitution
    int inputDec;
    if (this->cipherName != "DES") {
        inputDec = utilities::b_to_d(std::stoi(inputStr));
    } else {
        inputDec = desSbox(inputStr);
    }
    int outputDec = this->Box[left->getNodeName()][inputDec];
    // debug
    //std::cout << inputDec << " -> " << outputDec << std::endl;

    output->setThreeAddressNodeValue(outputDec);
    return outputDec;
}

int EasyBCInter::pboxEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    std::vector<int> inputVal = extValueFromTOUINTorBOXINDEX(right);

    // debug
    /*std::cout << "pbox input : " << std::endl;
    for (int i : inputVal) {
        std::cout << i;
    }
    std::cout << std::endl;
    for (int i : inputVal) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;*/

    // 因为存在并非双射的pbox，所以pbox的output size需要由pbox本身的size来决定
    int outSize = this->Box[left->getNodeName()].size();

    std::vector<int> outputVal;
    for (int j = 0; j < outSize; ++j)
        outputVal.push_back(0);
    for (int j = 0; j < outSize; ++j) {
        // debug
        //std::cout << ele->getLhs()->getNodeName() << " : " << this->Box[ele->getLhs()->getNodeName()][j] <<
        //        " : " << input[this->Box[ele->getLhs()->getNodeName()][j]] << std::endl;
        //outputVal[this->Box[left->getNodeName()][j]] = inputVal[j];
        /*
         * 注意，因为不同的block cipher的pbox的置换基准是不同的，
         * 比如对于PRESENT，其pbox置换为 output[pbox[i]] = input[i];
         * 而对于SKINNY64，其pbox置换为 output[i] = input[pbox[i]]。
         * 而本实现中统一采用 output[i] = input[pbox[i]], 所以对于置换规则不一样的block cipher，对应实现中的pbox需要有所调整。
         * 以PRESENT为例，由output[pbox1[i]] = input[i]的pbox1转换到output[i] = input[pbox2[i]]中的pbox2的方法为：
         *
         * int pbox[64] = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6,
         *                 22, 38, 54, 7, 23, 39, 55, 8, 24, 40, 56, 9, 25, 41, 57, 10, 26, 42, 58, 11, 27, 43, 59, 12,
         *                 28, 44, 60, 13, 29, 45, 61, 14, 30, 46, 62, 15, 31, 47, 63};
         * int out[64] = {0}, input[64] = {0};
         * for (int i = 0; i < 64; ++i) {input[i] = i;}
         * for (int i = 0; i < 64; ++i) {out[pbox[i]] = input[i];}
         * for (int i = 0; i < 64; ++i) {std::cout << out[i] << ", ";}
         * std::cout << std::endl;
         *
         * 上述代码最终输出的out数组即为满足output[i] = input[pbox2[i]]且与原pbox置换效果相同的数组。
         * */
        outputVal[j] = inputVal[this->Box[left->getNodeName()][j]];
    }
    std::string outputBinStr;
    //for (auto b : outputVal) outputBinStr += to_string(b);
    for (int i = 0; i < outputVal.size(); ++i) {
        outputBinStr += to_string(outputVal[i]);
        if (i < outputVal.size() - 1)
            outputBinStr += ",";
    }
    /*
     * 这里有一个实现上的问题，即对于每个ThreeAddressNode的实例，我们希望可以通过map来维护其所有被evaluate后的值，
     * 但是对于pbox而言，我开始的设想是把置换后的每一位bit都用字符串连接起来，后续再碰到SYMBOLINDEX时再根据index来
     * 依次访问对应的bit再转化为int类型的0或者1即可，但是pbox的置换往往会是32bit，64bit或者128bit，那么转换为字符
     * 串以后再传换成int类型存储到map 'rndTanNameMVal'中是不可行的，因为会溢出，超出了int或者longlong的表达范围，
     * 那这个时候我们如何解决呢？
     *
     * 目前一个直观的设想是，再维护一个map，这个map存储了每轮的所有pbox的对应置换后的bit所连接而成的字符串，即
     * string->string的map，而pbox对应到rndTanNameMVal中的值，我们需要预设一个特定的数，那么在碰到SYMBOLINDEX时，
     * 只要碰到这个特定的数，我们就可以判定时pbox下的SYMBOLINDEX，从而再根据string->string的map来寻找到对应的bit.
     *
     * 对于特定的数，因为其他的ThreeAddressNode对应的value都是属于集合{0，1}^n, 所以我们可以给负数或者非由01组成的任
     * 意自然数，目前选择数为 -2
     *
     * remark in 2023.9.23
     * 发现了一个新的问题，即pbox的permutation的对象并非一定是bit流，可能是uints表达的正整数，那么如果我们按照之前的
     * 做法，即把所有的output转换为string，后续再在symbolindex中提取对应的某一个bit是不可行的，因为置换的每一个元素
     * 都是uints，那么其对应的置换结果也是uints，不能仅仅提取某一个bit，此时我们应该如何处理呢？
     *
     * 一个直观的想法是，对于所有的permutation结果，我们依然采用string将其表示在一个新的map中，但是每个置换的结果之间
     * 我们用","相连，后续在symbolindex提取某个元素时，只需要先将pbox的结果按照","分割为一个vector<string>，然后再
     * 根据index提取出vector<string>中的对应string，然后再使用stoi获取对应的value，这种方法可以同时兼容bit和int。
     *
     * */
    // debug
    /*std::cout << "pbox out : \n" << outputBinStr << std::endl;
    for (int i = 0; i < outputVal.size(); ++i) {
        std::cout << outputVal[i];
    }
    std::cout << std::endl;*/

    this->pboxTanNameMStr[output->getNodeName()] = outputBinStr;
    output->setThreeAddressNodeValue(-2);
    return -2;
}

int EasyBCInter::ffTimesEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    /*
     * 对于矩阵乘法，我希望每一次矩阵乘法的计算只有一次，但是实际上的SYMBOLINDEX可能有多个，那么我就需要在第一次矩阵乘法被计算以后，后续
     * 其他的SYMBOLINDEX只需要寻找之前的计算结果即可取出对应的bit了。
     * 此时我们的处理可以类似于上述pbox的evaluate，即另外维护一个map，用于存储所有矩阵乘法的结果，那么在碰到SYMBOLINDEX时，只需要查找
     * 对应的map中存储的矩阵乘法的结果，即可获取对应的bit位信息。
     * 还有需要注意的点是，我们需要给定一个特定的数，用于标记对应的矩阵乘法已经被计算，用以标识来判定是否需要从新的维护的map中来寻找对应的矩
     * 阵乘法的结果。
     *
     * 这里我们新维护的map为 matVecProTanNameMStr，存储op为FFMTIMES的TAN的name到对应的FFMTIMES计算结果的映射，
     * 计算结果为std::vector<int>类型，
     * 类似于pbox且和pbox作为区分，存储在rndTanNameMVal中对应的FFMTIMES的TAN的值为 -3
     * */

    /*
     * remark in 2023.9.23
     * 这里我们现在遗留一个问题，即AES的矩阵乘法虽然也是有限域乘法，但是乘数本身是要超出FFM很多的，那么此时我们该如何取余计算呢？
     * 这就需要比较详细的了解有限域乘法的计算规则了。
     * */

    std::vector<int> inputVal = extValueFromTOUINTorBOXINDEX(right);
    // 初始化 matrix 和 ffm, 直接用用 vector<vector<int>> 来表示对应方阵
    std::vector<std::vector<int>> Matrix; // matrix
    std::vector<int> pboxmValue = pboxM[left->getNodeName()];
    int rowSize = inputVal.size(), rowCounter = 0;
    std::vector<int> tempRow;
    // Reassemble pboxm into a matrix
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
    std::vector<std::vector<int>> Ffm; // ffm
    int ffmRowSize = int(sqrt(ffmValue.size())); // 因为ffm一定是个方阵，所以求平方根以后就是ffm每行的size
    for (int & i : ffmValue) {
        tempRow.push_back(i);
        rowCounter++;
        if (rowCounter == ffmRowSize) {
            Ffm.push_back(tempRow);
            tempRow.clear();
            rowCounter = 0;
        }
    }

    // debug
    /*std::cout << "matrix vector product input : " << std::endl;
    for (auto ele : inputVal) {
        std::cout << ele << " ";
    }*/

    std::vector<int> outputVal;
    // 因为所有的矩阵都是方阵，所以可以统一按照input size来访问所有的矩阵元素
    for (int j = 0; j < inputVal.size(); ++j) {
        int t = 0;
        // debug
        //std::cout << "the " << j << "-th results : " << std::endl;
        for (int k = 0; k < inputVal.size(); ++k) {
            //std::cout << Ffm[Matrix[j][k]][inputVal[k]] << ", ";
            t ^= Ffm[Matrix[j][k]][inputVal[k]];
        }
        //std::cout << std::endl;
        outputVal.push_back(t);
    }

    // debug
    /*std::cout << "matrix vector product output : " << std::endl;
    for (auto ele : outputVal) {
        std::cout << ele << " ";
    }
    std::cout << std::endl;*/

    this->matVecProTanNameMStr[output->getNodeName()] = outputVal;
    output->setThreeAddressNodeValue(-3);
    return -3;
}

int EasyBCInter::symbolIndexEnc(ThreeAddressNodePtr left, ThreeAddressNodePtr right, ThreeAddressNodePtr output) {
    /*
     * 在op为 SYMBOLINDEX 的ThreeAddressNode实例中，存在两种情况：
     * 1. 左孩子为矩阵乘法，此时矩阵乘法还没有被evaluate，所以需要先evaluate，再提取对应的bit的值；
     * 2. 左孩子为box操作的结果，此时box操作的结果应该已经被evaluate，那么就可以直接提取对应的bit的值
     * */
    int outputVal;
    if (left->getOp() == ASTNode::FFTIMES) {
        // 如果左孩子结点中的FFM还没有被evaluate，先evaluate然后再更新值，
        // 若左孩子结点的值为-3，即ffTimesEnc的返回值，那么就说明已经被evaluate了。
        if (this->rndTanNameMVal[left->getNodeName()] != -3) {
            this->rndTanNameMVal[left->getNodeName()] = ffTimesEnc(left->getLhs(), left->getRhs(), left);
        }
        outputVal = this->matVecProTanNameMStr[left->getNodeName()][std::stoi(right->getNodeName())];
        output->setThreeAddressNodeValue(outputVal);
    } else if (left->getOp() == ASTNode::BOXOP) {
        // remark in 2023.9.23
        // 既然我们对pbox进行了修改使之可以适配所有uints的permutation，那么我们就需要对pbox和sbox作分别处理了
        /*std::string leftBinStr;
        if (this->rndTanNameMVal[left->getNodeName()] != -2)
            leftBinStr = std::to_string(utilities::d_to_b(this->rndTanNameMVal[left->getNodeName()])); // sbox op
        else
            leftBinStr = this->pboxTanNameMStr[left->getNodeName()]; // pbox op
        // 如果leftBinStr小于sbox输出的size，则说明目前sbox的输出值的string不够对应输出位数，需要在string之前补0，使得其长度等于sbox的输出位数目
        if (leftBinStr.size() < this->sboxOutputSize) {
            std::string tmp;
            for (int j = 0; j < this->sboxOutputSize - leftBinStr.size(); ++j) tmp += "0";
            leftBinStr = tmp + leftBinStr;
        }
        outputVal = stoi(leftBinStr.substr(stoi(right->getNodeName()), 1));
        output->setThreeAddressNodeValue(outputVal);*/

        // recode in 2023.9.23
        if (this->rndTanNameMVal[left->getNodeName()] != -2) {
            // sbox op
            std::string leftBinStr = std::to_string(utilities::d_to_b(this->rndTanNameMVal[left->getNodeName()]));
            // 如果leftBinStr小于sbox输出的size，则说明目前sbox的输出值的string不够对应输出位数，需要在string之前补0，使得其长度等于sbox的输出位数目
            if (leftBinStr.size() < this->sboxOutputSize) {
                std::string tmp;
                for (int j = 0; j < this->sboxOutputSize - leftBinStr.size(); ++j) tmp += "0";
                leftBinStr = tmp + leftBinStr;
            }
            outputVal = stoi(leftBinStr.substr(stoi(right->getNodeName()), 1));
        }
        else {
            // pbox op
            std::vector<std::string> leftPboxOutStrs = utilities::split(this->pboxTanNameMStr[left->getNodeName()], ",");
            outputVal = stoi(leftPboxOutStrs[stoi(right->getNodeName())]);
        }
        output->setThreeAddressNodeValue(outputVal);

    } else if (left->getOp() == ASTNode::INDEX) {
        /*
         * left op为INDEX时，left->left为一个数组，left->right为数组某个index的表达式
         * 如果是变量数组，那么其每个元素的evaluate一定是在其他语句函数中进行的，
         * 所以这里涉及的数组一定是常量数组，所以只需要在allBox里寻找就行了。
         * 此外，由于SYMBOLINDEX每次取位只取一个，因此我们在第一次evaluate出INDEX的value时，
         * 我们就将其保存，后续只需要检车其是否被evaluate，然后根据检查结果再决定是否需要evaluate
         * */
        if (this->rndTanNameMVal.find(left->getNodeName()) == this->rndTanNameMVal.end()) {
            // left->right是index表达式，我们需要区分其所有情况，目前所碰到的只有left->right本身是round function参数的round number r，
            // 所以直接通过保存的参数值来确定即可
            int leftVal, index;
            if (left->getRhs()->getNodeType() == PARAMETER) {
                // 因为round number是从1开始，那么实际的index应该是 round number - 1
                //index = this->rndParamMVal[left->getRhs()->getNodeName()][0] - 1;
                index = this->rndParamMVal["r"][0] - 1;
            } else if (left->getRhs()->getOp() == ASTNode::MINUS) {
                index = minusEnc(left->getRhs()->getLhs(), left->getRhs()->getRhs(), left->getRhs());
            } else
                assert(false);
            leftVal = this->Box[left->getLhs()->getNodeName()][index];
            this->rndTanNameMVal[left->getNodeName()] = leftVal;
        }
        int leftValSize = getNodeTypeSize(left->getNodeType());
        // 接下来，我们就需要根据leftVal的uints的s来确定binStr，然后再取其中某一个bit
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
        // 若左孩子节点是NULLOP，这说明左孩子本身就是一个常数，就需要根据左孩子结点的uints来对应的取对应于右孩子结点index的bit
        int leftValSize = getNodeTypeSize(left->getNodeType());
        std::string leftBinStr = to_string(utilities::d_to_b(std::stoi(left->getNodeName())));
        std::string pre;
        for (int i = 0; i < leftValSize - leftBinStr.size(); ++i) { pre += "0";}
        leftBinStr = pre + leftBinStr;

        outputVal = stoi(leftBinStr.substr(stoi(right->getNodeName()), 1));
        output->setThreeAddressNodeValue(outputVal);
    } else if (left->getNodeType() == PARAMETER) {
        // 若左孩子节点是PARAMTER，这说明左孩子本身r,而r我们统一作为8bit存储，
        // 那么就需要根据左孩子结点的uints来对应的取对应于右孩子结点index的bit
        int leftValSize = 8;
        std::string leftBinStr = to_string(utilities::d_to_b(this->rndTanNameMVal[left->getNodeName()]));
        std::string pre;
        for (int i = 0; i < leftValSize - leftBinStr.size(); ++i) { pre += "0";}
        leftBinStr = pre + leftBinStr;

        outputVal = stoi(leftBinStr.substr(stoi(right->getNodeName()), 1));
        output->setThreeAddressNodeValue(outputVal);
    } else if (left->getOp() == ASTNode::SYMBOLINDEX) {
        // remark in 2023.9.25
        // 可能存在左孩子节点为SYMBOLINDEX，即对SYMBOLINDEX取其中某一个bit,
        // 那么就需要递归调用，先处理左孩子结点的SYMBOLINDEX了。
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
    // touint 连接的对象从left中提取
    std::vector<int> inputVal = extValueFromTOUINTorBOXINDEX(left);
    // sort(inputVal.rbegin(), inputVal.rend());
    // touint的返回值是所有的input对应的bit值所拼接成的int值
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
        // 这里我们需要获取出每个三地址实例对应的evaluate值，然后将其添加到vector中
        if (this->rndTanNameMVal.find(ele->getNodeName()) == this->rndTanNameMVal.end()) {
            // 暂时不考虑在modular addition之前，没有evaluate出input的情况，所以这里暂时给的是assert false
            assert(false);
        }
        input1Val.push_back(this->rndTanNameMVal[ele->getNodeName()]);
    }
    for (auto & ele : input2) {
        // 这里我们需要获取出每个三地址实例对应的evaluate值，然后将其添加到vector中
        if (this->rndTanNameMVal.find(ele->getNodeName()) == this->rndTanNameMVal.end()) {
            // 暂时不考虑在modular addition之前，没有evaluate出input的情况，所以这里暂时给的是assert false
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
        // 这里我们需要获取出每个三地址实例对应的evaluate值，然后将其添加到vector中
        if (this->rndTanNameMVal.find(ele->getNodeName()) == this->rndTanNameMVal.end()) {
            // 暂时不考虑在modular addition之前，没有evaluate出input的情况，所以这里暂时给的是assert false
            assert(false);
        }
        input1Val.push_back(this->rndTanNameMVal[ele->getNodeName()]);
    }
    for (auto & ele : input2) {
        // 这里我们需要获取出每个三地址实例对应的evaluate值，然后将其添加到vector中
        if (this->rndTanNameMVal.find(ele->getNodeName()) == this->rndTanNameMVal.end()) {
            // 暂时不考虑在modular addition之前，没有evaluate出input的情况，所以这里暂时给的是assert false
            if (ele->getOp() == ASTNode::NULLOP) {
                this->rndTanNameMVal[ele->getNodeName()] = stoi(ele->getNodeName());
            } else
                assert(false);
        }
        input2Val.push_back(this->rndTanNameMVal[ele->getNodeName()]);
    }
    // 获取减数的补码
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














