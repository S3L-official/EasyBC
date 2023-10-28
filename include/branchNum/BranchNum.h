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

    std::vector<int> sbox_;
    int Ssize_{};

    std::vector<std::vector<int>> Matrix_;
    std::vector<std::vector<std::string>> MatrixBS_;
    std::vector<std::vector<int>> FFm_;
    int Msize_{}, Fsize_{};
    std::string MsizeS_, FsizeS_;

    int constant;
    std::string constantS;

    std::vector<int> inputCons;
    std::vector<std::string> inputConsS;
    int vecSize;

    std::string binOP_;
    std::string smtBinOP_;

    std::string smtLib_;
    std::string objMin_, objMax_;
    std::string modelMinPath_ = "../data/branchNum/bSMTLibMin.txt";
    std::string modelMaxPath_ = "../data/branchNum/bSMTLibMax.txt";

public:
    BranchN(const std::vector<int>& Matrix, const std::vector<int>& FFm, std::string mode)
            : optimize(context) {
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
        for (const auto& row : Matrix_) {
            std::vector<std::string> tt;
            for (auto ele: row) {
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

    BranchN(int constant, int eleSize, std::vector<int> FFm)
            : optimize(context), constant(constant), eleSize_(eleSize) {
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
        smtLib_ += bounder(1);
        BCsmtLib();
    }

    BranchN(std::vector<int> input, int eleSize, std::vector<int> FFm)
            : optimize(context), inputCons(input), eleSize_(eleSize) {
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
        smtLib_ += bounder(1);
        BVsmtLib();
    }

    BranchN(std::vector<int> sbox) : optimize(context), sbox_(std::move(sbox)) {
        eleSize_ = int(log2(sbox_.size()));
        eleSizeS_ = std::to_string(eleSize_);
        Ssize_ = sbox_.size();

        smtLib_ += sboxDef(eleSize_);
        smtLib_ += bounder(eleSize_);
        SsmtLib();
    }

    BranchN(int size, std::string op)
            : optimize(context), eleSize_(size), binOP_(std::move(op)) {
        eleSizeS_ = std::to_string(eleSize_);
        smtBinOP_ = smtBinOPget(binOP_);
        smtLib_ += bounder(1);
        BBsmtLib();
    }

    void SsmtLib();

    void WMsmtLib();
    void BMsmtLib();
    void BCsmtLib();

    void BVsmtLib();

    static std::string bounder(int size);

    std::string sboxDef(int size);
    std::string gfmuDef(int size);

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
        optimize.check();
        for (auto i : optimize.objectives()) {
            std::vector<std::string> tt = utilities::split(
                    optimize.get_model().eval(i).to_string(), "() ");
            std::string tmp;
            for (auto & k2 : tt) tmp += k2;
            branchNumMin_ = abs(stoi(tmp));
        }

        optimize.pop();
        optimize.from_string(modelMax.c_str());
        optimize.check();
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
