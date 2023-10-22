//
// Created by Septi on 2/2/2023.
//

#ifndef EASYBC_SBOXEXTC_H
#define EASYBC_SBOXEXTC_H


#include <z3++.h>
#include <fstream>
#include <map>
#include "utilities.h"
#include "setup.h"
#include "iomanip"


class SboxExC {

private:
    z3::context context_;
    z3::optimize optimize_;

    int maxExtSize_ = 8;
    int timer_ = 3600 * 12;
    int precisionBits_ = 10; // 控制概率取以2为底后的对数后的精度，现在是保留10位有效数字

    // 2, 4, 6, 8, 10, 12, 14, 16
    // Take the logarithm of the probability value with base 2
    //std::vector<float> probabilities_ = {-3, -2, -1.415, -1, -0.6781, -0.415, -0.1926, 0};
    std::vector<float> pLog2_ = {-3, -2, -1, 0};
    std::vector<std::string> c_;
    std::vector<std::vector<std::string>> p_;
    std::string smtLib_;

    std::string path_ = std::string(DPATH) + "sbox/DC/extBits/";
    std::string smtModelPath_, resultPath_;

    std::vector<float> weighted_{};
    std::vector<int> weightedIdx_{};

public:

    SboxExC(const std::string& name, int maxExtSize, const std::vector<int>& probabilities):
        optimize_(context_), maxExtSize_(maxExtSize) {
        int sboxSize = int(sqrt(maxExtSize * 2));
        for (auto p : probabilities)
            pLog2_.push_back(log2(p / pow(2, sboxSize)));

        for (int i = 0; i < pLog2_.size(); ++i) {
            std::string ci = "c" + std::to_string(i);
            c_.push_back(ci);
            std::vector<std::string> pi;
            for (int j = 0; j < maxExtSize_; ++j) {
                std::string pij = "p" + std::to_string(i) + "_" + std::to_string(j);
                pi.push_back(pij);
            }
            p_.push_back(pi);
        }

        smtModelPath_ += std::to_string(sboxSize) + "bits/" + name + "_smtLib.txt";
        resultPath_ += std::to_string(sboxSize) + "bits/" + name + "_results.txt";
        modelConstruct();
    }

    void modelConstruct() {
        for (const auto& ci : c_) {
            smtLib_ += "(declare-const " + ci + " Real)\n";
        }
        for (const auto& pi : p_) {
            for (const auto& pij : pi) {
                smtLib_ += "(declare-const " + pij + " Int)\n";
            }
        }

        // 所有 Pij 的取值只能是 0 或 1
        for (int i = 0; i < maxExtSize_; ++i) {
            for (int j = 0; j < maxExtSize_; ++j) {
                smtLib_ += "(assert (>= " + p_[i][j] + " 0))\n";
                smtLib_ += "(assert (<= " + p_[i][j] + " 1))\n";
            }
        }

        // hard constraints
        for (int i = 0; i < pLog2_.size(); ++i) {
            std::string cons;
            for (int j = 0; j < maxExtSize_; ++j) {
                cons += "(* " + c_[j] + " " + p_[i][j] + ")";
            }
            smtLib_ += "(assert (= " + std::to_string(pLog2_[i]).substr(0, precisionBits_) + " (+ " + cons + ")) )\n";
        }

        // soft constraints
        for (const auto& ci : c_) {
            smtLib_ += "(assert-soft (= " + ci + " 0) :weight 1)\n";
        }

        // debug : print constructed model
        std::cout << smtLib_ << std::endl;
        solving();
    }

    void solving() {
        std::cout << "timer : " << timer_ << std::endl;
        std::ofstream smtW(this->smtModelPath_, std::ios::trunc);
        if (!smtW){
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            smtW << smtLib_;
            smtW.close();
        }
        optimize_.push();
        optimize_.from_string(smtLib_.c_str());

        z3::params p(context_);
        p.set(":timeout", timer_ * 1000u);
        optimize_.set(p);

        clock_t startTime, endTime;
        startTime = clock();
        time_t star_time = 0, end_time;
        star_time = time(NULL);

        optimize_.check();
        //std::cout << optimize_.check() << std::endl;

        endTime = clock();
        end_time = time(NULL);
        double clockTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
        double timeTime = difftime( end_time, star_time);

        std::ofstream f(resultPath_, std::ios::trunc);
        f << "clock time : " << clockTime << "s\n";
        f << "time time : " << timeTime << "s\n";

        for (int i = 0; i < optimize_.get_model().size(); ++i) {
            std::cout << optimize_.get_model().get_const_decl(i).name() << " : ";
            f << optimize_.get_model().get_const_decl(i).name() << " : ";
            std::cout << optimize_.get_model().eval(optimize_.get_model().get_const_interp(optimize_.get_model().get_const_decl(i))) << std::endl;
            f << optimize_.get_model().eval(optimize_.get_model().get_const_interp(optimize_.get_model().get_const_decl(i))) << std::endl;
        }
        f.close();
    }

    std::vector<float> getWeighted() {
        std::ifstream file_extract;
        file_extract.open(resultPath_);
        std::string temp;
        while (getline(file_extract, temp)) {
            std::vector<std::string> line = utilities::split(temp, " ");
            for (const auto& ci : c_) {
                if (line[0] == ci) {
                    if (line[2] != "0.0") {
                        weighted_.push_back(std::stof(line[2]));
                        // The indexes of weighted are ci's sub-strings from the second char.
                        weightedIdx_.push_back(std::stoi(line[0].substr(1, line[0].size())));
                        break;
                    }
                }
            }
        }
        return weighted_;
    }

    // 这里返回的 encode 结果应该和 构造函数 中输入的 probabilities 是一一对应的。
    std::vector<std::string> getEncodes() {
        std::unordered_map<std::string, std::string> piAndValues;
        std::ifstream file_extract;
        file_extract.open(resultPath_);
        std::string temp;
        while (getline(file_extract, temp)) {
            // 先提取出所有 pi 及其对应值
            std::vector<std::string> line = utilities::split(temp, " ");
            if (line[0][0] == 'p')
                piAndValues[line[0]] = line[2];
        }

        std::vector<std::string> encodes;
        for (int i = 0; i < pLog2_.size(); ++i) {
            std::string piEncode;
            for (int j = 0; j < maxExtSize_; ++j) {
                std::string pij = "p" + std::to_string(i) + "_" + std::to_string(j);
                for (auto pair : piAndValues) {
                    if (pair.first == pij) {
                        piEncode += pair.second;
                        break;
                    }
                }
            }
            encodes.push_back(piEncode);
        }
        return encodes;
    }

};


#endif //EASYBC_SBOXEXTC_H
