//
// Created by Septi on 2021/7/1.
//

#ifndef SBOX_MILP_PURESOLVERALG_UTILITIES_H
#define SBOX_MILP_PURESOLVERALG_UTILITIES_H

#include "fstream"
#include "vector"
#include "string"
#include <omp.h>
#include <valarray>
#include <iostream>

namespace utilities{

    std::vector<std::string> split(const std::string &s, const std::string &seperator);

    int lcm(int num1, int num2);

    int b_to_d(long long n);

    long long d_to_b(long long n);

    std::string po_xor(std::string a, std::string b);

    bool sub_prec_ck(std::vector<std::string> us, std::vector<std::string> sub);

    std::vector<int> sv_to_iv(std::vector<std::string> tt);

    std::vector<std::string> iv_to_sv(std::vector<int> tt);

    int weight(std::string a);
}

#endif //SBOX_MILP_PURESOLVERALG_UTILITIES_H
