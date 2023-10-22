//
// Created by Septi on 2021/7/1.
//

#include "include/util/utilities.h"

std::vector<std::string> utilities::split(const std::string &s, const std::string &seperator) {
    std::vector<std::string> result;
    typedef std::string::size_type str_size;
    str_size i = 0;

    while (i != s.size()) {
        int flag = 0;
        while (i != s.size() && flag == 0) {
            flag = 1;
            for (str_size x = 0; x < seperator.size(); ++x)
                if (s[i] == seperator[x]) {
                    ++i;
                    flag = 0;
                    break;
                }
        }
        flag = 0;
        str_size j = i;
        while (j != s.size() && flag == 0) {
            for (str_size x = 0; x < seperator.size(); ++x)
                if (s[j] == seperator[x]) {
                    flag = 1;
                    break;
                }
            if (flag == 0)
                ++j;
        }
        if (i != j) {
            result.push_back(s.substr(i, j - i));
            i = j;
        }
    }
    return result;
}

int utilities::lcm(int num1, int num2) {
    int max = (num1 > num2) ? num1 : num2;
    do {
        if (max % num1 == 0 && max % num2 == 0){
            return max;
        }else
            max ++;
    } while (true);
}

int utilities::b_to_d(long long int n) {
    int decimalNumber = 0, i = 0, remainder;
    while (n!=0) {
        remainder = n % 10;
        n /= 10;
        decimalNumber += remainder * pow(2, i);
        i ++;
    }
    return decimalNumber;
}

long long utilities::d_to_b(long long int n) {
    long long rtn = 0, t = n, j = 1;
    while(t) {
        rtn = rtn + j * (t % 2);
        t = t / 2;
        j = j * 10;
    }
    return rtn;
}

std::string utilities::po_xor(std::string a, std::string b) {
    std::string f = "";
    for (int i = 0; i < a.size(); ++i)
        if (a[i] != b[i])
            f += "1";
        else
            f += "0";
    return f;
}

bool utilities::sub_prec_ck(std::vector<std::string> us, std::vector<std::string> sub) {
    int counter = 0;
    for (int i = 0; i < sub.size(); ++i) {
        __gnu_cxx::__normal_iterator<std::basic_string<char> *, std::vector<std::basic_string<char>>> it = std::find(us.begin(), us.end(), sub[i]);
        if (it != us.end())
            counter ++;
        else
            break;
    }

    if (counter == sub.size())
        return true;
    else
        return false;
}

std::vector<int> utilities::sv_to_iv(std::vector<std::string> tt) {
    std::vector<int> rtn;
    for (auto i : tt)
        rtn.push_back(std::stoi(i));
    return rtn;
}

std::vector<std::string> utilities::iv_to_sv(std::vector<int> tt) {
    std::vector<std::string> co;
    for (auto i: tt) {
        co.push_back(std::to_string(i));
    }
    return co;
}

int utilities::weight(std::string a) {
    int w = 0;
    for (char i : a) if (i == '1') w++;
    return w;
}




