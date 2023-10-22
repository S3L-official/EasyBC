//
// Created by Septi on 2/4/2023.
//

#ifndef EASYBC_BRANCHNUM_H
#define EASYBC_BRANCHNUM_H

#include "z3++.h"
#include "vector"
#include "string"
#include "fstream"
#include "utilities.h"

class BranchN {
private:
    z3::context context;
    z3::optimize optimize;

    int branchNumMin_, branchNumMax_;
    int eleSize_{};
    std::string eleSizeS_;

    // calculate branch number of sbox
    std::vector<int> sbox_;
    int Ssize_{};

    // calculate branch number of matrix
    std::vector<std::vector<int>> Matrix_;
    std::vector<std::vector<std::string>> MatrixBS_;
    std::vector<std::vector<int>> FFm_;
    int Msize_{}, Fsize_{};
    std::string MsizeS_, FsizeS_;

    // calculate branch number of constant
    int constant;
    std::string constantS;

    // calculate branch number of vector multi
    std::vector<int> inputCons;
    std::vector<std::string> inputConsS;
    int vecSize;

    // calculate branch number of binary operators
    // Calculating the branch of a binary operator only requires the number of bits and operator type of the object being manipulated
    std::string binOP_;
    std::string smtBinOP_;

    std::string smtLib_;
    std::string objMin_, objMax_;
    std::string modelMinPath_ = "../data/branchNum/bSMTLibMin.txt";
    std::string modelMaxPath_ = "../data/branchNum/bSMTLibMax.txt";

public:
    // Constructor of matrix
    // 这里我们接收的matrix和ffm都是一维的，需要将其转换成二维的
    BranchN(const std::vector<int>& Matrix, const std::vector<int>& FFm, std::string mode)
            : optimize(context) {
        // 一维matrix，ffm转换为二维
        // 因为matrix和ffm都是方阵，所以可以直接用平方根
        int rowSizeM = int(sqrt(Matrix.size())), rowSizeF = int(sqrt(FFm.size())), rowCounter = 0;
        std::vector<int> tempRow;
        for (int i : Matrix) {
            tempRow.push_back(i);
            rowCounter++;
            if (rowCounter == rowSizeM) {
                Matrix_.push_back(tempRow);
                tempRow.clear();
                rowCounter = 0;
            }
        }
        for (int i : FFm) {
            tempRow.push_back(i);
            rowCounter++;
            if (rowCounter == rowSizeF) {
                FFm_.push_back(tempRow);
                tempRow.clear();
                rowCounter = 0;
            }
        }
        eleSize_ = int(log2(FFm_.size()));
        eleSizeS_ = std::to_string(eleSize_);
        Msize_ = Matrix_.size();
        Fsize_ = FFm_.size();
        MsizeS_ = std::to_string(Msize_);
        FsizeS_ = std::to_string(Fsize_);
        // 这里要把矩阵转化为对应的二进制字符串格式，方便后面直接对元素进行操作
        for (const auto& row : Matrix_) {
            std::vector<std::string> tt;
            for (auto ele: row) {
                // 先将FFm中的每个元素转化为长度为sizeS的二进制字符串
                std::string eleB = std::to_string(utilities::d_to_b(ele));
                for (int i = eleB.size(); i < eleSize_; ++i) eleB = "0" + eleB;
                eleB = "#b" + eleB;
                tt.push_back(eleB);
            }
            MatrixBS_.push_back(tt);
        }
        smtLib_ += gfmuDef(eleSize_);
        if (mode == "b") {
            smtLib_ += bounder(1);
            BMsmtLib();
        } else if (mode == "w") {
            smtLib_ += bounder(eleSize_);
            WMsmtLib();
        }
    }

    // 和某个常数进行有限域乘法时的branch number
    BranchN(int constant, int eleSize, std::vector<int> FFm)
            : optimize(context), constant(constant), eleSize_(eleSize) {
        // 一维matrix，ffm转换为二维
        // 因为matrix和ffm都是方阵，所以可以直接用平方根
        int rowSizeF = int(sqrt(FFm.size())), rowCounter = 0;
        std::vector<int> tempRow;
        for (int i : FFm) {
            tempRow.push_back(i);
            rowCounter++;
            if (rowCounter == rowSizeF) {
                FFm_.push_back(tempRow);
                tempRow.clear();
                rowCounter = 0;
            }
        }
        eleSizeS_ = std::to_string(eleSize_);
        Fsize_ = FFm_.size();
        FsizeS_ = std::to_string(Fsize_);
        std::string eleB = std::to_string(utilities::d_to_b(constant));
        for (int i = eleB.size(); i < eleSize_; ++i) eleB = "0" + eleB;
        eleB = "#b" + eleB;
        constantS = eleB;
        smtLib_ += gfmuDef(eleSize_);
        //smtLib_ += bounder(eleSize_);
        //WMsmtLib();
        smtLib_ += bounder(1);
        BCsmtLib();
    }

    // 和一个vector变量和某个常数数组进行有限域乘法结果再进行xor时的branch number
    BranchN(std::vector<int> input, int eleSize, std::vector<int> FFm)
            : optimize(context), inputCons(input), eleSize_(eleSize) {
        // 一维matrix，ffm转换为二维
        // 因为matrix和ffm都是方阵，所以可以直接用平方根
        int rowSizeF = int(sqrt(FFm.size())), rowCounter = 0;
        std::vector<int> tempRow;
        for (int i : FFm) {
            tempRow.push_back(i);
            rowCounter++;
            if (rowCounter == rowSizeF) {
                FFm_.push_back(tempRow);
                tempRow.clear();
                rowCounter = 0;
            }
        }
        eleSizeS_ = std::to_string(eleSize_);
        Fsize_ = FFm_.size();
        FsizeS_ = std::to_string(Fsize_);
        vecSize = inputCons.size();
        for (auto ele : inputCons) {
            std::string eleB = std::to_string(utilities::d_to_b(ele));
            for (int i = eleB.size(); i < eleSize_; ++i)
                eleB = "0" + eleB;
            eleB = "#b" + eleB;
            inputConsS.push_back(eleB);
        }
        smtLib_ += gfmuDef(eleSize_);
        //smtLib_ += bounder(eleSize_);
        //WMsmtLib();
        smtLib_ += bounder(1);
        BVsmtLib();
    }

    // Constructor of lookup table sbox
    BranchN(std::vector<int> sbox) : optimize(context), sbox_(std::move(sbox)) {
        // 目前考虑的是输入输出位一样的情况，
        // 对于n位输入的sbox，其输入可能有2^n种，所以sbox的长度求log2即为输入输出长度
        eleSize_ = int(log2(sbox_.size()));
        eleSizeS_ = std::to_string(eleSize_);
        Ssize_ = sbox_.size();

        smtLib_ += sboxDef(eleSize_);
        smtLib_ += bounder(eleSize_);
        SsmtLib();
    }

    // Constructor of n-bits binary operators
    BranchN(int size, std::string op)
            : optimize(context), eleSize_(size), binOP_(std::move(op)) {
        eleSizeS_ = std::to_string(eleSize_);
        smtBinOP_ = smtBinOPget(binOP_);
        //smtLib_ += bounder(eleSize_);
        //WBsmtLib();
        smtLib_ += bounder(1);
        BBsmtLib();
    }

    void SsmtLib();

    // 矩阵计算branch number初始化约束
    void WMsmtLib();
    void BMsmtLib();
    // 有限域常量乘的branch number
    void BCsmtLib();

    // 和某个vector进行操作时的 branch number
    // 具体操作是，操作数和vector中的每个元素分别进行有限域乘法，然后结果之间在进行异或
    void BVsmtLib();

    // calculating the non-zero entries/bits of difference
    // when used for word-wise, the int size is the eleSize
    // when used for bit-wise, the int size is 1
    static std::string bounder(int size);

    // 将sbox定义为一维数组，将输入作为索引值，返回对应的value
    std::string sboxDef(int size);
    std::string gfmuDef(int size);

    // word wise branch number of binary operators
    void WBsmtLib();
    void BBsmtLib();
    std::string smtBinOPget(std::string op);

    std::vector<int> getBranchNum() {
        std::string modelMin = smtLib_ + objMin_, modelMax = smtLib_ + objMax_;
        std::ofstream smtWMin(this->modelMinPath_, std::ios::trunc);
        if (!smtWMin){
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            smtWMin << modelMin;
            smtWMin.close();
        }
        std::ofstream smtWMax(this->modelMaxPath_, std::ios::trunc);
        if (!smtWMax){
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            smtWMax << modelMax;
            smtWMax.close();
        }
        optimize.push();
        optimize.from_string(modelMin.c_str());

        z3::params p(context);
        p.set(":timeout", 5 * 1000u);
        optimize.set(p);
        //std::cout << "all params : " << std::endl;
        //std::cout << "all params : " << std::endl;


        //Z3_global_param_set("timeout", "1");
        //z3::set_param("solver.timeout", 10);
        optimize.check();
        //std::cout << optimize.check() << std::endl;
        //std::cout << optimize.get_model().eval(optimize.objectives()[0]) << std::endl;
        for (auto i : optimize.objectives()) {
            std::vector<std::string> tt = utilities::split(
                    optimize.get_model().eval(i).to_string(), "() ");
            std::string tmp;
            for (auto & k2 : tt) tmp += k2;
            branchNumMin_ = abs(stoi(tmp));
        }

        //std::cout << optimize.get_model().to_string() << std::endl;

        /*for (int i = 0; i < optimize.get_model().size(); ++i) {
            std::cout << optimize.get_model().get_const_decl(0).name() << " : ";
            std::cout << optimize.get_model().eval(optimize.get_model().get_const_interp(optimize.get_model().get_const_decl(i))) << std::endl;
        }*/

        //branchNumMax_ = 0;
        optimize.pop();
        optimize.from_string(modelMax.c_str());
        optimize.check();
        //std::cout << optimize.check() << std::endl;
        //std::cout << optimize.get_model().eval(optimize.objectives()[0]) << std::endl;
        for (auto i : optimize.objectives()) {
            std::vector<std::string> tt = utilities::split(
                    optimize.get_model().eval(i).to_string(), "() ");
            std::string tmp;
            for (auto & k2 : tt) tmp += k2;
            branchNumMax_ = abs(stoi(tmp));
        }
        return {branchNumMin_, branchNumMax_};
    }

};

#endif //EASYBC_BRANCHNUM_H
