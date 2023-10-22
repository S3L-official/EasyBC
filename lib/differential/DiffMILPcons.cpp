//
// Created by Septi on 12/14/2022.
//
#include "differential/DiffMILPcons.h"

void DiffMILPcons::wXorC1(std::string path, int inputIdx1, int inputIdx2, int outputIdx, int &dCounter) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of wXORc1 ! " << std::endl;
    } else {
        scons << "A" << inputIdx1 << " + A" << inputIdx2 << " + A" << outputIdx << " - 2 d" << dCounter << " >= 0\n";
        scons << "d" << dCounter << " - A" << inputIdx1 << " >= 0\n";
        scons << "d" << dCounter << " - A" << inputIdx2 << " >= 0\n";
        scons << "d" << dCounter << " - A" << outputIdx << " >= 0\n";
        dCounter++;
    }
    scons.close();
}

void DiffMILPcons::wXorC2(std::string path, int inputIdx1, int inputIdx2, int outputIdx) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of wXORc2 ! " << std::endl;
    } else {
        scons << "A" << inputIdx1 << " + A" << inputIdx2 << " - A" << outputIdx << " >= 0\n";
        scons << "A" << inputIdx1 << " - A" << inputIdx2 << " + A" << outputIdx << " >= 0\n";
        scons << "- A" << inputIdx1 << " + A" << inputIdx2 << " + A" << outputIdx << " >= 0\n";
    }
    scons.close();
}

void DiffMILPcons::wAndOrC(std::string path, int inputIdx1, int inputIdx2, int outputIdx) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of wAndOrC ! " << std::endl;
    } else {
        scons << "A" << inputIdx1 << " + A" << inputIdx2 << " - A" << outputIdx << " >= 0\n";
    }
    scons.close();
}

void DiffMILPcons::wLinearC1(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx, int &dCounter,
                             int branchMin, int branchMax) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of wLinearC1 ! " << std::endl;
    } else {
        for (int i : inputIdx)
            scons << "A" << i << " + ";
        for (int i = 0; i < outputIdx.size(); ++i) {
            scons << "A" << outputIdx[i];
            if (i < outputIdx.size() - 1)
                scons << " + ";
        }
        scons << " - " << branchMin << " d" << dCounter << " >= 0\n";
        for (int i : inputIdx)
            scons << "A" << i << " + ";
        for (int i = 0; i < outputIdx.size(); ++i) {
            scons << "A" << outputIdx[i];
            if (i < outputIdx.size() - 1)
                scons << " + ";
        }
        scons << " <= " << branchMax << "\n";

        for (int i : inputIdx)
            scons << "d" << dCounter << " - A" << i << " >= 0\n";
        dCounter++;

        int omega = outputIdx.size();
        for (int i = 0; i < inputIdx.size(); ++i) {
            if (i != inputIdx.size() - 1)
                scons << omega << " A" << inputIdx[i] << " + ";
            else
                scons << omega << " A" << inputIdx[i];
        }
        for (auto i : outputIdx) {
            scons << " - A" << i;
        }
        scons << " >= 0\n";
    }
    scons.close();
}

void DiffMILPcons::wLinearC2(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx, int &dCounter,
                             int branchMin, int branchMax) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of wLinearC2 ! " << std::endl;
    } else {
        for (int i : inputIdx)
            scons << "A" << i << " + ";
        for (int i = 0; i < outputIdx.size(); ++i) {
            scons << "A" << outputIdx[i];
            if (i < outputIdx.size() - 1)
                scons << " + ";
        }
        scons << " - " << branchMin << " d" << dCounter << " >= 0\n";
        for (int i : inputIdx)
            scons << "A" << i << " + ";
        for (int i = 0; i < outputIdx.size(); ++i) {
            scons << "A" << outputIdx[i];
            if (i < outputIdx.size() - 1)
                scons << " + ";
        }
        scons << " <= " << branchMax << "\n";

        int omega1 = inputIdx.size();
        scons << omega1 << " d" << dCounter;
        for (int i : inputIdx)
            scons << " - A" << i;
        scons << " >= 0\n";
        dCounter++;

        int omega2 = outputIdx.size();
        for (int i = 0; i < inputIdx.size(); ++i) {
            if (i != inputIdx.size() - 1)
                scons << omega2 << " A" << inputIdx[i] << " + ";
            else
                scons << omega2 << " A" << inputIdx[i];
        }
        for (auto i : outputIdx) {
            scons << " - A" << i;
        }
        scons << " >= 0\n";
    }
    scons.close();
}

void DiffMILPcons::bXorC1(std::string path, int inputIdx1, int inputIdx2, int outputIdx, int &dCounter) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of bXORc1 ! " << std::endl;
    } else {
        scons << "x" << inputIdx1 << " + x" << inputIdx2 << " + x" << outputIdx << " - 2 d" << dCounter << " >= 0\n";
        scons << "d" << dCounter << " - x" << inputIdx1 << " >= 0\n";
        scons << "d" << dCounter << " - x" << inputIdx2 << " >= 0\n";
        scons << "d" << dCounter << " - x" << outputIdx << " >= 0\n";
        scons << "x" << inputIdx1 << " + x" << inputIdx2 << " + x" << outputIdx << " <= 2\n";
        dCounter++;
    }
    scons.close();
}

void DiffMILPcons::bXorC2(std::string path, int inputIdx1, int inputIdx2, int outputIdx) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of bXORc2 ! " << std::endl;
    } else {
        scons << "x" << inputIdx1 << " + x" << inputIdx2 << " + x" << outputIdx << " <= 2\n";
        scons << "x" << inputIdx1 << " + x" << inputIdx2 << " - x" << outputIdx << " >= 0\n";
        scons << "x" << inputIdx1 << " - x" << inputIdx2 << " + x" << outputIdx << " >= 0\n";
        scons << "-x" << inputIdx1 << " + x" << inputIdx2 << " + x" << outputIdx << " >= 0\n";
    }
    scons.close();
}

void DiffMILPcons::bXorC3(std::string path, int inputIdx1, int inputIdx2, int outputIdx, int &dCounter) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of bXORc3 ! " << std::endl;
    } else {
        scons << "x" << inputIdx1 << " + x" << inputIdx2 << " + x" << outputIdx << " - 2 d" << dCounter << " = 0\n";
        dCounter++;
    }
    scons.close();
}

// without introduce extra variables
void DiffMILPcons::bMatrixEntryC12(std::string path, std::vector<std::vector<int>> inputIdx, std::vector<int> outputIdx,
                                   std::vector<int> overflows, std::vector<int> module, int &xCounter, int &yCounter,
                                   int &fCounter, int &dCounter, int chooser) {
    // y_i 映射的 x_j 的异或最终结果用单独的 output 来记录，
    // 还需要处理完 溢出位决定要不要跟模 异或以后，在将输出赋值给 y_i
    std::vector<int> input1 = inputIdx[0];
    std::vector<int> input2 = inputIdx[1];
    std::vector<int> output;
    for (int i = 0; i < input1.size(); ++i) {
        output.push_back(xCounter);
        xCounter++;
    }
    //MultibXORC(path, input1, input2, output);
    for (int i = 0; i < input1.size(); ++i) {
        if (chooser == 1)
            bXorC1(path, input1[i], input2[i], output[i], dCounter);
        else if (chooser == 2)
            bXorC2(path, input1[i], input2[i], output[i]);
    }
    std::vector<int> lastOut;
    if (inputIdx.size() > 2) {
        for (int i = 2; i < inputIdx.size(); ++i) {
            input1 = std::move(inputIdx[i]);
            input2 = std::move(output);
            output.clear();
            for (int j = 0; j < input1.size(); ++j) {
                output.push_back(xCounter);
                xCounter++;
            }
            //MultibXORC(path, input1, input2, output);
            for (int j = 0; j < input1.size(); ++j) {
                if (chooser == 1)
                    bXorC1(path, input1[j], input2[j], output[j], dCounter);
                else if (chooser == 2)
                    bXorC2(path, input1[j], input2[j], output[j]);
            }
            if (i == inputIdx.size() - 1)
                lastOut = std::move(output);
        }
    }

    // 处理 overflow 的最终值，结果应该是其中所有值的异或
    // 没有溢出位
    int fIdx = fCounter;
    fCounter++;
    bool fFlag = true;
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        if (overflows.size() == 0) {
            fFlag = false;
            for (int i = 0; i < lastOut.size(); ++i) {
                scons << "x" << outputIdx[i] << " - x" << lastOut[i] << " = 0\n";
            }
        }
            // 溢出位为1时，标识溢出位的最终值即为该溢出位的值
        else if (overflows.size() == 1) {
            scons << "f" << fIdx << " - x" << overflows[0] << " = 0\n";
        } else if (overflows.size() >= 2) {
            int XORout = xCounter;
            xCounter++;
            if (chooser == 1)
                bXorC1(path, overflows[0], overflows[1], XORout, dCounter);
            else if (chooser == 2)
                bXorC2(path, overflows[0], overflows[1], XORout);
            if (overflows.size() > 2) {
                for (int i = 2; i < overflows.size(); ++i) {
                    int in1 = overflows[i], in2 = XORout;
                    XORout = xCounter;
                    xCounter++;
                    if (chooser == 1)
                        bXorC1(path, in1, in2, XORout, dCounter);
                    else if (chooser == 2)
                        bXorC2(path, in1, in2, XORout);
                }
            }
            scons << "f" << fIdx << " - x" << XORout << " = 0\n";
        }
    }
    // 如果有溢出位，根据 溢出位异或的最终值，添加 indicator constrain, 描述是否需要和模进行异或
    if (fFlag) {
        for (int i = 0; i < module.size(); ++i) {
            if (module[module.size()-1-i] == 0) {
                scons << "f" << fIdx << " == 1 -> " << "x" << outputIdx[module.size()-1-i] << " - x" << lastOut[i] << " = 0\n";
                scons << "f" << fIdx << " == 0 -> " << "x" << outputIdx[module.size()-1-i] << " - x" << lastOut[i] << " = 0\n";
            } else if (module[module.size()-1-i] == 1) {
                scons << "f" << fIdx << " == 1 -> " << "x" << outputIdx[module.size()-1-i] << " + x" << lastOut[i] << " = 0\n";
                scons << "f" << fIdx << " == 0 -> " << "x" << outputIdx[module.size()-1-i] << " - x" << lastOut[i] << " = 0\n";
            }
        }
    }
}

void DiffMILPcons::bNXorC3(std::string path, std::vector<int> inputIdx, int outputIdx, int &yCounter) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of bNXorC3 ! " << std::endl;
    } else {
        int x0Counter = 0;
        for (int i = 0; i < inputIdx.size() - 1; ++i) {
            scons << "x" << inputIdx[i] << " + ";
            if (inputIdx[i] == 0)
                x0Counter++;
        }
        if (inputIdx[inputIdx.size() - 1] == 0)
            x0Counter++;
        scons << "x" << inputIdx[inputIdx.size() - 1] << " - 2 y" << yCounter << " - x" << outputIdx << " = 0\n";
        scons << "y" << yCounter << " >= 0\n";
        scons << "y" << yCounter << " - " << (inputIdx.size() - x0Counter)/2 << " <= 0\n";
        yCounter++;
    }
    scons.close();
}

void DiffMILPcons::bMatrixEntryC3(std::string path, std::vector<std::vector<int>> inputIdx, std::vector<int> outputIdx,
                                  std::vector<int> overflows, std::vector<int> module, int &xCounter, int &yCounter,
                                  int &fCounter) {
    std::vector<int> output;
    for (int i = 0; i < outputIdx.size(); ++i) {
        output.push_back(xCounter);
        xCounter++;
    }
    for (int i = 0; i < inputIdx[0].size(); ++i) {
        std::vector<int> tInput;
        for (int j = 0; j < inputIdx.size(); ++j) {
            tInput.push_back(inputIdx[j][i]);
        }
        bNXorC3(path, tInput, output[i], yCounter);
    }

    // 处理 overflow 的最终值，结果应该是其中所有值的异或
    int fIdx = fCounter;
    fCounter++;
    bool fFlag = true;
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        if (overflows.empty()) {
            fFlag = false;
            for (int i = 0; i < output.size(); ++i) {
                scons << "x" << outputIdx[i] << " - x" << output[i] << " = 0\n";
            }
        }
            // 溢出位为1时，标识溢出位的最终值即为该溢出位的值
        else if (overflows.size() == 1) {
            scons << "f" << fIdx << " - x" << overflows[0] << " = 0\n";
        } else if (overflows.size() >= 2) {
            // 多个溢出位时，这里直接写入约束，不再调用 bNXorC
            for (int i = 0; i < overflows.size() - 1; ++i) {
                scons << "x" << overflows[i] << " + ";
            }
            scons << "x" << overflows[overflows.size() - 1] << " - 2 y" << yCounter << " + f" << fIdx << " = 0\n";
            scons << "y" << yCounter << " >= 0\n";
            scons << "y" << yCounter << " - " << (overflows.size()+1)/2 << " <= 0\n";
            yCounter++;
        }
    }
    // 如果有溢出位，根据 溢出位异或的最终值，添加 indicator constrain, 描述是否需要和模进行异或
    if (fFlag) {
        for (int i = 0; i < module.size(); ++i) {
            // 这里是根据 module 的每位值，直接建立output对应位于outputIdx对应位的关系了
            // 逻辑上，若fIdx为1时，output 与 module 异或后的值是 outputIdx 对应位的值
            if (module[module.size()-1-i] == 0) {
                scons << "f" << fIdx << " == 1 -> " << "x" << outputIdx[module.size()-1-i] << " - x" << output[i] << " = 0\n";
                scons << "f" << fIdx << " == 0 -> " << "x" << outputIdx[module.size()-1-i] << " - x" << output[i] << " = 0\n";
            } else if (module[module.size()-1-i] == 1) {
                scons << "f" << fIdx << " == 1 -> " << "x" << outputIdx[module.size()-1-i] << " + x" << output[i] << " = 0\n";
                scons << "f" << fIdx << " == 0 -> " << "x" << outputIdx[module.size()-1-i] << " - x" << output[i] << " = 0\n";
            }
        }
    }
}

void DiffMILPcons::bSboxC1(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                           std::vector<std::vector<int>> ineqs, int branchMin, int &dCounter, int &ACounter, bool ifInjective) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        // input
        /*for (auto i : inputIdx)
            scons << "x" << i << " + ";
        // output
        for (int i = 0; i < outputIdx.size(); ++i) {
            if (i != outputIdx.size() - 1)
                scons << "x" << outputIdx[i] << " + ";
            else
                scons << "x" << outputIdx[i];
        }
        scons << " - " << branchMin << " d" << dCounter << " >= 0\n";
        for (auto i : inputIdx)
            scons << "d" << dCounter << " - x" << i << " >= 0\n";
        for (auto i : outputIdx)
            scons << "d" << dCounter << " - x" << i << " >= 0\n";
        dCounter++;*/

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

        if (ifInjective) {
            int omega = inputIdx.size();
            for (int i = 0; i < inputIdx.size(); ++i) {
                if (i != inputIdx.size() - 1)
                    scons << omega << " x" << inputIdx[i] << " + ";
                else
                    scons << omega << " x" << inputIdx[i];
            }
            for (auto i : outputIdx) {
                scons << " - x" << i;
            }
            scons << " >= 0\n";
            for (int i = 0; i < outputIdx.size(); ++i) {
                if (i != outputIdx.size() - 1)
                    scons << omega << " x" << outputIdx[i] << " + ";
                else
                    scons << omega << " x" << outputIdx[i];
            }
            for (auto i : inputIdx) {
                scons << " - x" << i;
            }
            scons << " >= 0\n";
        }

        // sbox modeling ineqs
        for (auto ineq : ineqs) {
            int idx = 0;
            for (auto i : inputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            for (auto i : outputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            if ((ineq[idx]) > 0)
                scons << " >= -" << abs((ineq[idx])) << "\n";
            else
                scons << " >= " << abs((ineq[idx])) << "\n";
        }
    }
    scons.close();
}

void DiffMILPcons::bSboxC2(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                           std::vector<std::vector<int>> ineqs, int &ACounter, bool ifInjective) {
    std::ofstream scons(path, std::ios::app);
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

        if (ifInjective) {
            int omega = inputIdx.size();
            for (int i = 0; i < inputIdx.size(); ++i) {
                if (i != inputIdx.size() - 1)
                    scons << omega << " x" << inputIdx[i] << " + ";
                else
                    scons << omega << " x" << inputIdx[i];
            }
            for (auto i : outputIdx) {
                scons << " - x" << i;
            }
            scons << " >= 0\n";
            for (int i = 0; i < outputIdx.size(); ++i) {
                if (i != outputIdx.size() - 1)
                    scons << omega << " x" << outputIdx[i] << " + ";
                else
                    scons << omega << " x" << outputIdx[i];
            }
            for (auto i : inputIdx) {
                scons << " - x" << i;
            }
            scons << " >= 0\n";
        }

        // sbox modeling ineqs
        for (auto ineq : ineqs) {
            int idx = 0;
            for (auto i : inputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            for (auto i : outputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            if ((ineq[idx]) > 0)
                scons << " >= -" << abs((ineq[idx])) << "\n";
            else
                scons << " >= " << abs((ineq[idx])) << "\n";
        }
    }
    scons.close();
}

void DiffMILPcons::bSboxC3(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                           std::vector<std::vector<int>> ineqs, int &ACounter) {
    std::ofstream scons(path, std::ios::app);
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

        // sbox modeling ineqs
        for (auto ineq : ineqs) {
            int idx = 0;
            for (auto i : inputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            for (auto i : outputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            if ((ineq[idx]) > 0)
                scons << " >= -" << abs((ineq[idx])) << "\n";
            else
                scons << " >= " << abs((ineq[idx])) << "\n";
        }
    }
    scons.close();
}

void DiffMILPcons::bAddC(std::string path, std::vector<int> inputIdx1, std::vector<int> inputIdx2, std::vector<int> outputIdx,
                         std::vector<std::vector<int>> ineqs, int &dCounter, int &PCounter) {
    int addSize = inputIdx1.size();
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        // pCounter = 1;
        // Theorem 1
        scons << "x" << inputIdx1[addSize - 1] << " + x" << inputIdx2[addSize - 1] << " + x" << outputIdx[addSize - 1] << " <= 2\n";
        scons << "x" << inputIdx1[addSize - 1] << " + x" << inputIdx2[addSize - 1] << " + x" << outputIdx[addSize - 1] << " - 2 d" << dCounter << " >= 0\n";
        scons << "d" << dCounter << " - x" << inputIdx1[addSize - 1] << " >= 0\n";
        scons << "d" << dCounter << " - x" << inputIdx2[addSize - 1] << " >= 0\n";
        scons << "d" << dCounter << " - x" << outputIdx[addSize - 1] << " >= 0\n";
        dCounter++;

        // Theorem 2
        for (int i = addSize - 1; i > 0; --i) {
            for (auto ineq : ineqs) {
                int idx = 0;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << inputIdx1[i];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << inputIdx1[i];
                idx++;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << inputIdx2[i];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << inputIdx2[i];
                idx++;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << outputIdx[i];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << outputIdx[i];
                idx++;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << inputIdx1[i - 1];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << inputIdx1[i - 1];
                idx++;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << inputIdx2[i - 1];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << inputIdx2[i - 1];
                idx++;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << outputIdx[i - 1];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << outputIdx[i - 1];
                idx++;

                if (ineq[idx] > 0)
                    scons << " >= -" << abs(ineq[idx]) << "\n";
                else
                    scons << " >= +" << abs(ineq[idx]) << "\n";
            }
        }
    }
    scons.close();
}

void DiffMILPcons::bAndOrC(std::string path, int inputIdx1, int inputIdx2, int outputIdx) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of bAndC1 ! " << std::endl;
    } else {
        scons << "x" << inputIdx1 << " + x" << inputIdx2 << " - x" << outputIdx << " >= 0\n";
    }
    scons.close();
}

void DiffMILPcons::dSboxC1(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                           std::vector<std::vector<int>> ineqs, int branchMin, int &dCounter, int &PCounter,
                           bool ifInjective, int extLen) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        // input
        for (auto i : inputIdx)
            scons << "x" << i << " + ";
        // output
        for (int i = 0; i < outputIdx.size(); ++i) {
            if (i != outputIdx.size() - 1)
                scons << "x" << outputIdx[i] << " + ";
            else
                scons << "x" << outputIdx[i];
        }
        scons << " - " << branchMin << " d" << dCounter << " >= 0\n";
        for (auto i : inputIdx)
            scons << "d" << dCounter << " - x" << i << " >= 0\n";
        for (auto i : outputIdx)
            scons << "d" << dCounter << " - x" << i << " >= 0\n";
        dCounter++;

        if (ifInjective) {
            int omega = inputIdx.size();
            for (int i = 0; i < inputIdx.size(); ++i) {
                if (i != inputIdx.size() - 1)
                    scons << omega << " x" << inputIdx[i] << " + ";
                else
                    scons << omega << " x" << inputIdx[i];
            }
            for (auto i : outputIdx) {
                scons << " - x" << i;
            }
            scons << " >= 0\n";
            for (int i = 0; i < outputIdx.size(); ++i) {
                if (i != outputIdx.size() - 1)
                    scons << omega << " x" << outputIdx[i] << " + ";
                else
                    scons << omega << " x" << outputIdx[i];
            }
            for (auto i : inputIdx) {
                scons << " - x" << i;
            }
            scons << " >= 0\n";
        }

        // sbox modeling ineqs
        for (auto ineq : ineqs) {
            int idx = 0;
            for (auto i : inputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            for (auto i : outputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            for (int i = 0; i < extLen; ++i) {
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " P" << i + PCounter;
                else
                    scons << " - " << abs(ineq[idx]) << " P" << i + PCounter;
                idx++;
            }
            if ((ineq[idx]) > 0)
                scons << " >= -" << abs((ineq[idx])) << "\n";
            else
                scons << " >= " << abs((ineq[idx])) << "\n";
        }
        PCounter += extLen;
    }
    scons.close();
}

void DiffMILPcons::dSboxC2(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                           std::vector<std::vector<int>> ineqs, int &PCounter, bool ifInjective, int extLen) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        if (ifInjective) {
            int omega = inputIdx.size();
            for (int i = 0; i < inputIdx.size(); ++i) {
                if (i != inputIdx.size() - 1)
                    scons << omega << " x" << inputIdx[i] << " + ";
                else
                    scons << omega << " x" << inputIdx[i];
            }
            for (auto i : outputIdx) {
                scons << " - x" << i;
            }
            scons << " >= 0\n";
            for (int i = 0; i < outputIdx.size(); ++i) {
                if (i != outputIdx.size() - 1)
                    scons << omega << " x" << outputIdx[i] << " + ";
                else
                    scons << omega << " x" << outputIdx[i];
            }
            for (auto i : inputIdx) {
                scons << " - x" << i;
            }
            scons << " >= 0\n";
        }

        // sbox modeling ineqs
        for (auto ineq : ineqs) {
            int idx = 0;
            for (auto i : inputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            for (auto i : outputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            for (int i = 0; i < extLen; ++i) {
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " P" << i + PCounter;
                else
                    scons << " - " << abs(ineq[idx]) << " P" << i + PCounter;
                idx++;
            }
            if ((ineq[idx]) > 0)
                scons << " >= -" << abs((ineq[idx])) << "\n";
            else
                scons << " >= " << abs((ineq[idx])) << "\n";
        }
        PCounter += extLen;
    }
    scons.close();
}

void DiffMILPcons::dSboxC3(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                           std::vector<std::vector<int>> ineqs, int &PCounter, int extLen) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        // sbox modeling ineqs
        for (auto ineq : ineqs) {
            int idx = 0;
            for (auto i : inputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            for (auto i : outputIdx) {
                if ((ineq[idx]) >= 0)
                    scons << " + " << abs((ineq[idx])) << " x" << i;
                else
                    scons << " - " << abs((ineq[idx])) << " x" << i;
                idx++;
            }
            for (int i = 0; i < extLen; ++i) {
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " P" << i + PCounter;
                else
                    scons << " - " << abs(ineq[idx]) << " P" << i + PCounter;
                idx++;
            }
            if ((ineq[idx]) > 0)
                scons << " >= -" << abs((ineq[idx])) << "\n";
            else
                scons << " >= " << abs((ineq[idx])) << "\n";
        }
        PCounter += extLen;
    }
    scons.close();
}

void DiffMILPcons::dAddC(std::string path, std::vector<int> inputIdx1, std::vector<int> inputIdx2, std::vector<int> outputIdx,
                         std::vector<std::vector<int>> ineqs, int &dCounter, int &PCounter) {
    int addSize = inputIdx1.size();
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        // pCounter = 1;
        // Theorem 1
        scons << "x" << inputIdx1[addSize - 1] << " + x" << inputIdx2[addSize - 1] << " + x" << outputIdx[addSize - 1] << " <= 2\n";
        scons << "x" << inputIdx1[addSize - 1] << " + x" << inputIdx2[addSize - 1] << " + x" << outputIdx[addSize - 1] << " - 2 d" << dCounter << " >= 0\n";
        scons << "d" << dCounter << " - x" << inputIdx1[addSize - 1] << " >= 0\n";
        scons << "d" << dCounter << " - x" << inputIdx2[addSize - 1] << " >= 0\n";
        scons << "d" << dCounter << " - x" << outputIdx[addSize - 1] << " >= 0\n";
        dCounter++;

        // Theorem 2
        for (int i = addSize - 1; i > 0; --i) {
            for (auto ineq : ineqs) {
                int idx = 0;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << inputIdx1[i];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << inputIdx1[i];
                idx++;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << inputIdx2[i];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << inputIdx2[i];
                idx++;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << outputIdx[i];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << outputIdx[i];
                idx++;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << inputIdx1[i - 1];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << inputIdx1[i - 1];
                idx++;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << inputIdx2[i - 1];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << inputIdx2[i - 1];
                idx++;
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << outputIdx[i - 1];
                else
                    scons << " - " << abs(ineq[idx]) << " x" << outputIdx[i - 1];
                idx++;

                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " P" << PCounter;
                else
                    scons << " - " << abs(ineq[idx]) << " P" << PCounter;
                idx++;

                if (ineq[idx] > 0)
                    scons << " >= -" << abs(ineq[idx]) << "\n";
                else
                    scons << " >= +" << abs(ineq[idx]) << "\n";
            }
            PCounter++;
        }
    }
    scons.close();
}

void DiffMILPcons::dAndOrC(std::string path, int inputIdx1, int inputIdx2, int outputIdx, int &PCounter) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of bAndC1 ! " << std::endl;
    } else {
        scons << "- x" << inputIdx1 << " - x" << inputIdx2 << " - x" << outputIdx << " + 3 P" << PCounter << " >= 0\n";
        scons << "x" << inputIdx1 << " + x" << inputIdx2 << " - P" << PCounter << " >= 0\n";
        PCounter++;
    }
    scons.close();
}




