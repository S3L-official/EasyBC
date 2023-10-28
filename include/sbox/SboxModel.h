//
// Created by Septi on 8/29/2022.
//

#ifndef EASYBC_SBOXMODEL_H
#define EASYBC_SBOXMODEL_H

#include <cassert>
#include <utility>
#include <cmath>
#include <bitset>
#include "SboxExtC.h"


class SboxM {

private:
    std::string name_;
    std::vector<int> sbox_;
    std::string mode_;

    int ddtInputSize_;
    int ddtOutputSize_;
    int sboxInputSize_;
    int sboxOutputSize_;

    int ddtProbNum_{};
    std::vector<int> ddtProb_{};
    std::vector<float> extWeighted_{};
    std::unordered_map<std::string, std::string> ddtProbEncode_;

    std::string path_ = std::string(DPATH) + "sbox/";
    std::string ddtPath_{}, latPath_{}, arxPath_{};

    std::vector<std::string> impossPm_;
    std::vector<std::string> possPm_;

    std::string sageInPath_;
    std::string sageOutPath_;
    std::vector<std::vector<int>> sageIneqs_;

    int sboxMDP;

public:
    SboxM(std::string name, std::vector<int> sbox, std::string mode);
    SboxM(std::string name, std::string mode);

    void ddt_gen();
    void lat_gen();
    static int binCounter1(int input);

    void get_ext();
    void pattern_ext();
    void sage_ext();

    void arxDiff();
    void arxLinear();
    void arxSageGen();

    void trueTableGen();

    void possBintoHex();

    void set_name(std::string name) { this->name_ = name;}
    std::string get_name() {return name_;}
    std::string get_ddt() {return ddtPath_;}
    std::string get_lat() {return latPath_;}
    std::string get_path() {return path_;}
    std::string get_mode() {return mode_;}
    int get_sbox_len() {return int(log2(sbox_.size()));}
    std::vector<std::string> get_poss() {return possPm_;}
    std::vector<std::string> get_imposs() {return impossPm_;}
    std::vector<std::vector<int>> get_sage_ineqs() {return sageIneqs_;}
    int get_dim() {return possPm_[0].size() + 1;}
    int get_ext_len() {return ddtProbNum_;}

    float getSboxMDP() {
        std::sort(ddtProb_.rbegin(), ddtProb_.rend());
        int maxProb = ddtProb_[1];
        float rtn = -log2(maxProb / pow(2, sqrt(sbox_.size())));
        return  rtn;
    }

    std::vector<int> get_extWeighted() {
        std::vector<int> rtn;
        std::vector<float> temp = extWeighted_;
        while (true) {
            int times = 10, counter = 0;
            bool allInt = false;
            for (auto &weight : temp) {
                if (weight - int(weight) == 0) {
                    counter++;
                }
            }
            if (counter == extWeighted_.size())
                break;
        }
        for (auto item : temp) {
            rtn.push_back(int(item));
        }
        return rtn;
    }
};

#endif //EASYBC_SBOXMODEL_H
