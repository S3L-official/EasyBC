//
// Created by Septi on 8/29/2022.
//
#include "SboxModel.h"

SboxM::SboxM(std::string name, std::vector<int> sbox, std::string mode) :
    name_(std::move(name)), sbox_(std::move(sbox)), mode_(std::move(mode)) {

    // the input size of the sbox is the size of the sbox
    this->ddtInputSize_ = this->sbox_.size();
    // the output size of the sbox is the value of the element whose value is the largest
    auto tmp = std::max_element(this->sbox_.begin(), this->sbox_.end());
    this->ddtOutputSize_ = this->sbox_[tmp - this->sbox_.begin()] + 1;
    this->sboxInputSize_ = int(log2(this->ddtInputSize_));
    this->sboxOutputSize_ = int(log2(this->ddtOutputSize_));

    if (mode_ == "AS")
        ddtPath_ = path_ + "AS/";
    else if (mode_ == "DC")
        ddtPath_ = path_ + "DC/";
    path_ = ddtPath_;
    ddtPath_ += "ddt/";
    ddt_gen();

    get_ext();
    pattern_ext();
    sage_ext();
    trueTableGen();
    possBintoHex(); // for superball
}

SboxM::SboxM(std::string name, std::string mode) : name_(std::move(name)), mode_(std::move(mode)) {
    path_ += "/ARX/";
    arxDiff();
}

void SboxM::ddt_gen() {
    int ddt[this->ddtInputSize_][this->ddtOutputSize_];
    for (int i = 0; i < this->ddtInputSize_; ++i)
        for (int j = 0; j < this->ddtOutputSize_; ++j)
            ddt[i][j] = 0;

    for (int i = 0; i < ddtInputSize_; ++i)
        for (int j = 0; j < this->ddtInputSize_; ++j) {
            int output1 = sbox_[i];
            int output2 = sbox_[j];
            // 就目前所有的benchmark，存在inputsize和outputsize不等的只有DES的sbox
            // 而DES有输入决定的输出是根据最低位和最高位组成的2bit数作为行号来确定output的
            // 所以我们需要对这个sbox进行特殊处理
            if (this->ddtInputSize_ != this->ddtOutputSize_) {
                int outputIndex1 = ((i / 32) * 2 + (i % 2)) * 16 +
                                   ((i % 32) / 2);
                int outputIndex2 = ((j / 32) * 2 + (j % 2)) * 16 +
                                   ((j % 32) / 2);

                output1 = sbox_[outputIndex1];
                output2 = sbox_[outputIndex2];
            }
            // debug
            //std::cout << i << ", " << j << " -> " << output1 << ", " << output2 << std::endl;
            ddt[i ^ j][output1 ^ output2]++;
            //ddt[i ^ j][sbox_[i] ^ sbox_[j]]++;
        }
    ddtPath_ += std::to_string(int(log2(ddtInputSize_))) + "bits/";
    system(("mkdir -p " + ddtPath_).c_str());
    ddtPath_ += name_ + ".txt";
    system(("touch " + ddtPath_).c_str());
    std::ofstream fs;
    fs.open(ddtPath_);
    for (int i = 0; i < ddtInputSize_; ++i) {
        fs << '[';
        for (int j = 0; j < ddtOutputSize_; ++j)
            fs << ddt[i][j] << "  ";
        fs << ']' << '\n';
    }
    fs.close();
}

void SboxM::lat_gen() {
    int lat_len = sbox_.size();
    int lat[lat_len][lat_len];
    for (int i = 0; i < lat_len; ++i) {
        for (int j = 0; j < lat_len; ++j) {
            int total = 0;
            for (int k = 0; k < lat_len; ++k) {
                int inputMasked = k & i;
                int outputMasked = sbox_[k] & j;
                if ((binCounter1(inputMasked) - binCounter1(outputMasked)) % 2 == 0)
                    total = total + 1;
            }
            lat[i][j] = total - ceil(lat_len / 2);
        }
    }

    latPath_ += std::to_string(int(log2(lat_len))) + "bits/";
    system(("mkdir -p " + latPath_).c_str());
    latPath_ += name_ + ".txt";
    system(("touch " + latPath_).c_str());
    std::ofstream fs;
    fs.open(latPath_);
    for (int i = 0; i < lat_len; ++i) {
        fs << '[';
        for (int j = 0; j < lat_len; ++j)
            fs << lat[i][j] << "  ";
        fs << ']' << '\n';
    }
    fs.close();
}

int SboxM::binCounter1(int input) {
    std::string r;
    while (input != 0){
        r += ( input % 2 == 0 ? "0" : "1" );
        input /= 2;
    }
    int rtn = 0;
    for (auto c : r) {
        if (c == '1')
            rtn++;
    }
    return rtn;
}

void SboxM::get_ext() {
    int len = log2(sbox_.size());
    std::unordered_map<std::string, int> poss_t;
    std::string file_path;
    file_path = ddtPath_;

    std::ifstream file_extract;
    file_extract.open(file_path);
    std::vector<std::string> imposs_p;
    std::vector<std::string> poss_p;
    int count = 0;
    std::string temp;

    while (getline(file_extract, temp)) {
        std::vector<std::string> line = utilities::split(temp, " []");
        for (auto & i : line) {
            if (std::find(ddtProb_.begin(), ddtProb_.end(), std::stoi(i)) == ddtProb_.end()) {
                ddtProb_.push_back(std::stoi(i));
            }
        }
    }
    ddtProbNum_ = ddtProb_.size();
}

void SboxM::pattern_ext() {
    int len = log2(sbox_.size());
    std::unordered_map<std::string, int> possT;
    std::string file_path;
    file_path = ddtPath_;

    std::vector<std::string> impossP;
    std::vector<std::string> possP;
    int count = 0;
    std::string temp;

    // 这里有一些现成的encode方法我们可以直接按照原有的使用，
    // 对于不符合的情况，我们再用general的方法通过maxSat求解
    bool usePreSet = false; // 用于判定是否使用预设定的 encode 方式，目前预设定三种。
    // 现根据获得的概率类型进行判断
    int st = 0;
    // 目前只对 4-bit 的 S-box 提供预设扩展位 encode
    if (len == 4) {
        if (ddtProbNum_ == 4) {
            // 判断是否概率是 0, 2, 4, 16
            for (auto p: {0, 2, 4, 16}) {
                for (auto ddtP: ddtProb_) {
                    if (p == ddtP) st++;
                }
            }
            extWeighted_ = {1, 2};
        } else if (ddtProbNum_ == 5) {
            for (auto p: {0, 2, 4, 6, 16}) {
                for (auto ddtP: ddtProb_) {
                    if (p == ddtP) st++;
                }
            }
            extWeighted_ = {600, 400, 283};
        }
        // 后续可以增加下面的概率情况的预扩展位
        /*else if (ddtProbNum_ == 6) {
            for (auto p: {0, 2, 4, 6, 12, 16}) {
                for (auto ddtP: ddtProb_) {
                    if (p == ddtP) st++;
                }
            }
        }*/
    }
    // ASCON test
    else if (len == 5) {
        if (ddtProbNum_ == 5) {
            for (auto p: {0, 2, 4, 8, 32}) {
                for (auto ddtP: ddtProb_) {
                    if (p == ddtP) st++;
                }
            }
            extWeighted_ = {1, 3};
        }
    }
        // 这里我们需要对DES进行特殊处理
    else if (len == 6) {
        if (ddtProbNum_ == 10) {
            // 判断是否概率是 64, 0, 6, 2, 4, 10, 12, 8, 14, 16
            for (auto p: {64, 0, 6, 2, 4, 10, 12, 8, 14, 16}) {
                for (auto ddtP: ddtProb_) {
                    if (p == ddtP) st++;
                }
            }
            extWeighted_ = {1, 3};
        }
    }
    if (st == ddtProbNum_)
        usePreSet = true;
    else {
        // 因为5bit及以上的sbox进行概率扩展时，实际上的模型基本不可解，
        // 所以我们只对4bit以及以下的sbox size进行处理
        if (len <= 4) {
            SboxExC sboxExC(name_, int(pow(2, int(sqrt(sbox_.size()))) / 2), ddtProb_);
            // 获取每个概率是如何 encode 的，并且获取对应的权重。
            extWeighted_ = sboxExC.getWeighted();
            std::vector<std::string> encodes = sboxExC.getEncodes();
            for (int i = 0; i < encodes.size(); ++i) {
                ddtProbEncode_[std::to_string(ddtProb_[i])] = encodes[i];
            }
        }
    }

    std::ifstream file_extract;
    file_extract.open(file_path);
    while (getline(file_extract, temp)) {
        std::vector<std::string> line = utilities::split(temp, " []");
        std::bitset<SBOX_LENGTH> xIdx = count;
        for (int i = 0; i < line.size(); ++i) {
            std::bitset<SBOX_LENGTH> yIdx = i;
            if (line[i] != "0"){
                /*std::string poss = xIdx.to_string().substr(SBOX_LENGTH - len, len) +
                                   yIdx.to_string().substr(SBOX_LENGTH - len, len);*/
                std::string poss = xIdx.to_string().substr(SBOX_LENGTH - this->sboxInputSize_, this->sboxInputSize_) +
                                   yIdx.to_string().substr(SBOX_LENGTH - this->sboxOutputSize_, this->sboxOutputSize_);

                possT[poss] = 1;

                if (mode_ == "AS")
                    possP.push_back(poss);
                else if (mode_ == "DC") {
                    if (usePreSet) {
                        // 使用预设的固定 encode
                        if (ddtProbNum_ == 4) {
                            if (line[i] == "16")
                                poss += "00";
                            else if (line[i] == "4") // 2 ^ -2
                                poss += "01";
                            else if (line[i] == "2") // 2 ^ -3
                                poss += "11";
                        } else if (ddtProbNum_ == 5 and len == 4) {
                            if (line[i] == "16")
                                poss += "000";
                            else if (line[i] == "6")
                                poss += "001";
                            else if (line[i] == "4")
                                poss += "010";
                            else if (line[i] == "2")
                                poss += "100";
                        }
                        // ASCON pre encode
                        else if (ddtProbNum_ == 5 and len == 5) {
                            if (line[i] == "32")
                                poss += "00";
                            else if (line[i] == "4" or line[i] == "8") // 2 ^ -3
                                poss += "01";
                            else if (line[i] == "2") // 2 ^ -4
                                poss += "11";
                        }
                        // DES pre encode
                        else if (ddtProbNum_ == 10) {
                            if (line[i] == "64")
                                poss += "00";
                            else if (line[i] == "16" or line[i] == "14" or line[i] == "12") // 2 ^ -1
                                poss += "10";
                            else if (line[i] == "10" or line[i] == "8" or line[i] == "6") // 2 ^ -3
                                poss += "01";
                            else if (line[i] == "4" or line[i] == "2") // 2 ^ -4
                                poss += "11";
                        }
                    } else {
                        // 使用 maxSat 求解得到的 encode
                        poss += ddtProbEncode_[line[i]];
                    }
                    possT[poss] = 1;
                    possP.push_back(poss);
                } else
                    assert(false);
            }
        }
        count++;
    }

    /*if (mode_ == "AS") {
        for (int i = 0; i < (1 << (len * 2)); i++) {
            std::bitset<SBOX_LENGTH * 2> enumer = i;
            std::string imposs = enumer.to_string().substr(SBOX_LENGTH * 2 - len * 2,
                                                           len * 2);
            if (possT[imposs] == 1)
                continue;
            else
                impossP.push_back(imposs);
        }
    } else {
        // 这里实际上已经确定了扩展位的位数，即weighted的size，根据这个来确定 impossP就可以了。
        for (int i = 0; i < (1 << (len * 2 + extWeighted_.size())); i++){
            std::bitset<SBOX_LENGTH * 2> enumer = i;
            std::string imposs = enumer.to_string().substr(SBOX_LENGTH * 2 - len * 2 - extWeighted_.size(),
                                                           len * 2 + extWeighted_.size());
            if (possT[imposs] == 1)
                continue;
            else
                impossP.push_back(imposs);
        }
    }*/
    if (mode_ == "AS") {
        for (int i = 0; i < (1 << (this->sboxInputSize_ + this->sboxOutputSize_)); i++) {
            std::bitset<SBOX_LENGTH * 2> enumer = i;
            std::string imposs = enumer.to_string().substr(SBOX_LENGTH * 2 - (this->sboxInputSize_ + this->sboxOutputSize_),
                                                           (this->sboxInputSize_ + this->sboxOutputSize_));
            if (possT[imposs] == 1)
                continue;
            else
                impossP.push_back(imposs);
        }
    } else {
        // 这里实际上已经确定了扩展位的位数，即weighted的size，根据这个来确定 impossP就可以了。
        for (int i = 0; i < (1 << ((this->sboxInputSize_ + this->sboxOutputSize_) + extWeighted_.size())); i++){
            std::bitset<SBOX_LENGTH * 2> enumer = i;
            std::string imposs = enumer.to_string().substr(SBOX_LENGTH * 2 - (this->sboxInputSize_ + this->sboxOutputSize_) - extWeighted_.size(),
                                                           (this->sboxInputSize_ + this->sboxOutputSize_) + extWeighted_.size());
            if (possT[imposs] == 1)
                continue;
            else
                impossP.push_back(imposs);
        }
    }

    sageInPath_ = path_ + "sage/input/" + std::to_string(this->ddtInputSize_) + "bits/";
    sageOutPath_ = path_ + "sage/output/" + std::to_string(this->ddtInputSize_) + "bits/";
    system(("mkdir -p " + sageInPath_).c_str());
    system(("mkdir -p " + sageOutPath_).c_str());
    sageInPath_ += name_ + ".sage";
    sageOutPath_ += name_ + ".txt";
    system(("touch " + sageInPath_).c_str());
    system(("touch " + sageOutPath_).c_str());

    impossPm_ = impossP;
    possPm_ = possP;

    std::ofstream file_save;
    file_save.open(sageInPath_);
    file_save << "points = [";
    for (int i = 0; i < possP.size(); ++i){
        file_save << '[';
        for (int j = 0; j < possP[i].size(); ++j){
            if (j != possP[i].size() - 1)
                file_save << possP[i][j] << ", ";
            else
                file_save << possP[i][j];
        }
        if (i != possP.size() - 1)
            file_save << ']' << ',' << "\n";
        else
            file_save << ']';
    }
    file_save << ']' << '\n';
    file_save << "poly = Polyhedron ( vertices = points)\n";
    file_save << "for l in poly.inequality_generator(): \n";
    file_save << "    print(l)\n";
    file_save.close();
}

void SboxM::sage_ext() {
    std::cout << "sage input start " << std::endl;
    std::system((std::string("sage ") + sageInPath_ + " > " + sageOutPath_).c_str());
    std::cout << "sage input finish " << std::endl;

    std::ifstream file_extract;
    file_extract.open(sageOutPath_);
    std::string temp;
    while (getline(file_extract, temp)) {
        std::string tt = "";
        bool fl = false;
        for (auto i : temp) {
            if (i == '(')
                fl = true;
            else if (i == ')')
                fl = false;
            if (fl && i != '(' && i != ','){
                tt = tt + i;
            }
            else if (i == 'x') {
                char sign = temp[temp.find(i) + 2];
                int a = temp.find(i) + 4;
                if (temp[a + 1] == ' ')
                    tt = tt + " " + sign + temp[a];
                else if (temp[a + 2] == ' ')
                    tt = tt + " " + sign + temp[a] + temp[a + 1];
                else if (temp[a + 3] == ' ')
                    tt = tt + " " + sign + temp[a] + temp[a + 1] + temp[a + 2];
                else if (temp[a + 4] == ' ')
                    tt = tt + " " + sign + temp[a] + temp[a + 1] + temp[a + 2] + temp[a + 3];
            }
        }
        std::vector<int> tiq;
        std::vector<std::string> ts = utilities::split(tt, " ");
        for (const auto& i : ts) {
            tiq.push_back(std::stoi(i));
        }
        sageIneqs_.push_back(tiq);
    }
}

void SboxM::arxDiff() {
    std::vector<std::string> poss_p, imposs_p;
    std::unordered_map<std::string, int> poss_t;
    int const vecLen = 6;
    int counter = 0, threshold = pow(2,vecLen);
    while (counter < threshold) {
        std::bitset<vecLen> tempVec = counter;
        std::string tempVecStr = tempVec.to_string();

        // 判断每个vec的值是否为possible differential pattern
        if (tempVecStr[0] == tempVecStr[1] and tempVecStr[0] == tempVecStr[2]) {
            int tXor = stoi(std::to_string(tempVecStr[3])) ^ stoi(std::to_string(tempVecStr[4])) ^ stoi(std::to_string(tempVecStr[5]));
            if (stoi(std::to_string(tempVecStr[0])) == tXor) {
                if (this->mode_ == "DC")
                    tempVecStr += "0";
                poss_p.push_back(tempVecStr);
                poss_t[tempVecStr] = 1;
            }
        } else {
            if (this->mode_ == "DC")
                tempVecStr += "1";
            poss_p.push_back(tempVecStr);
            poss_t[tempVecStr] = 1;
        }
        counter++;
    }

    if (this->mode_ == "AS") {
        for (int i = 0; i < (1 << (vecLen)); i++) {
            std::bitset<vecLen> enumer = i;
            std::string imposs = enumer.to_string();
            if (poss_t[imposs] == 1)
                continue;
            else
                imposs_p.push_back(imposs);
        }
    } else if (this->mode_ == "DC") {
        for (int i = 0; i < (1 << (vecLen + 1)); i++) {
            std::bitset<vecLen + 1> enumer = i;
            std::string imposs = enumer.to_string();
            if (poss_t[imposs] == 1)
                continue;
            else
                imposs_p.push_back(imposs);
        }
    }
    impossPm_ = imposs_p;
    possPm_ = poss_p;
    arxSageGen();
}

void SboxM::arxLinear() {
    std::string path = path_;
    system(("mkdir -p " + path).c_str());
    std::vector<std::string> poss_p, imposs_p;
    std::unordered_map<std::string, int> poss_t;
    int const vecLen = 5;
    int counter = 0, threshold = pow(2,vecLen);
    while (counter < threshold) {
        std::bitset<vecLen> tempVec = counter;
        std::string tempVecStr = tempVec.to_string();

        // 判断每个vec的值是否为possible linear transition
        int u = 4 * stoi(std::string(1, tempVecStr[1])) + 2 * stoi(std::string(1, tempVecStr[2])) + stoi(std::string(1, tempVecStr[3]));
        // 第一个元素是e0
        if (tempVecStr[0] == '0') {
            if (u == 7 and tempVecStr[4] == '1') {
                poss_p.push_back(tempVecStr);
                poss_t[tempVecStr] = 1;
            } else if (u == 0 and tempVecStr[4] == '0') {
                poss_p.push_back(tempVecStr);
                poss_t[tempVecStr] = 1;
            }
        // 第一个元素是e1
        } else if (tempVecStr[0] == '1') {
            if ((u == 0 or u == 3 or u == 5 or u == 6) and tempVecStr[4] == '1') {
                poss_p.push_back(tempVecStr);
                poss_t[tempVecStr] = 1;
            } else if ((u == 1 or u == 2 or u == 4 or u == 7) and tempVecStr[4] == '0') {
                poss_p.push_back(tempVecStr);
                poss_t[tempVecStr] = 1;
            }
        }
        counter++;
    }

    for (int i = 0; i < (1 << vecLen); i++) {
        std::bitset<vecLen> enumer = i;
        std::string imposs = enumer.to_string();
        if (poss_t[imposs] == 1)
            continue;
        else
            imposs_p.push_back(imposs);
    }
    impossPm_ = imposs_p;
    possPm_ = poss_p;
    arxSageGen();
}

void SboxM::arxSageGen() {
    std::string path = path_;
    system(("mkdir -p " + path).c_str());
    std::string sageIn = path + name_ + ".sage", sageOut = path + name_ + ".txt";
    system(("touch " + sageIn).c_str());
    system(("touch " + sageOut).c_str());
    std::ofstream file_save;
    file_save.open(sageIn);
    file_save << "points = [";
    for (int i = 0; i < possPm_.size(); ++i){
        file_save << '[';
        for (int j = 0; j < possPm_[i].size(); ++j){
            if (j != possPm_[i].size() - 1)
                file_save << possPm_[i][j] << ", ";
            else
                file_save << possPm_[i][j];
        }
        if (i != possPm_.size() - 1)
            file_save << ']' << ',' << "\n";
        else
            file_save << ']';
    }
    file_save << ']' << '\n';
    file_save << "poly = Polyhedron ( vertices = points)\n";
    file_save << "for l in poly.inequality_generator(): \n";
    file_save << "    print(l)\n";
    file_save.close();

    std::system((std::string("sage ") + sageIn + " > " + sageOut).c_str());

    std::vector<std::vector<int>> rtn;
    std::ifstream file_extract;
    file_extract.open(sageOut);
    std::string temp;
    while (getline(file_extract, temp)) {
        std::string tt = "";
        bool fl = false;
        for (auto i : temp) {
            if (i == '(')
                fl = true;
            else if (i == ')')
                fl = false;
            if (fl && i != '(' && i != ','){
                tt = tt + i;
            }
            if (i == '+'){
                int a = temp.find(i) + 2;
                if (temp[a + 1] == ' ')
                    tt = tt + " " + temp[a];
                else if (temp[a + 2] == ' ')
                    tt = tt + " " + temp[a] + temp[a + 1];
                else if (temp[a + 3] == ' ')
                    tt = tt + " " + temp[a] + temp[a + 1] + temp[a + 2];
                else if (temp[a + 4] == ' ')
                    tt = tt + " " + temp[a] + temp[a + 1] + temp[a + 2] + temp[a + 3];
            }
        }
        std::vector<int> tiq;
        std::vector<std::string> ts = utilities::split(tt, " ");
        for (const auto& i : ts) {
            tiq.push_back(std::stoi(i));
        }
        rtn.push_back(tiq);
    }
    this->sageIneqs_ = rtn;
}

void SboxM::trueTableGen() {
    std::vector<std::string> poss_p = possPm_;

    std::ofstream file;
    std::string file_path = path_ + "trueTable/" + std::to_string(int(log2(sbox_.size()))) + "bits/";
    std::string file_path2 = path_ + "cnf/" + std::to_string(int(log2(sbox_.size()))) + "bits/";
    system(("mkdir -p " + file_path).c_str());
    system(("mkdir -p " + file_path2).c_str());
    file_path += name_ + ".csv";
    file_path2 += name_ + ".txt";
    system(("touch " + file_path).c_str());
    system(("touch " + file_path2).c_str());

    file.open(file_path, std::ios::out);
    int dim = possPm_[0].size();
    for (int i = 0; i < dim; ++i) {
        file << char(i + 65) << ",";
    }
    file << "," << "F0" << std::endl;

    for (auto & i : poss_p) {
        for (char j : i) {
            if (j == '0')
                file << 0 << ",";
            else if (j == '1')
                file << 1 << ",";
            else
                file << 'X' << ",";
        }
        file << "," << 1 << std::endl;
    }
    file.close();
}

void SboxM::possBintoHex() {
    std::vector<std::string> possPmHex;
    for (auto & i : possPm_) {
        std::string hex;
        std::stringstream ss;
        ss << std::hex << stoi(i, nullptr, 2);
        ss >> hex;
        transform(hex.begin(), hex.end(), hex.begin(), ::toupper);
        possPmHex.push_back(hex);
    }

    std::ofstream file;
    std::string file_path = path_ + "possPmHex/" + std::to_string(int(log2(sbox_.size()))) + "bits/";
    system(("mkdir -p " + file_path).c_str());
    file_path += name_ + ".txt";
    system(("touch " + file_path).c_str());
    file.open(file_path, std::ios::out);
    for (auto hex : possPmHex) {
        file << hex << " ";
    }
    file.close();

    file_path = path_ + "poss/" + std::to_string(int(log2(sbox_.size()))) + "bits/";
    system(("mkdir -p " + file_path).c_str());
    file_path += name_ + ".txt";
    system(("touch " + file_path).c_str());
    file.open(file_path, std::ios::out);
    for (auto poss : possPm_) {
        file << poss << "\n";
    }
    file.close();

    file_path = path_ + "imposs/" + std::to_string(int(log2(sbox_.size()))) + "bits/";
    system(("mkdir -p " + file_path).c_str());
    file_path += name_ + ".txt";
    system(("touch " + file_path).c_str());
    file.open(file_path, std::ios::out);
    for (auto imposs : impossPm_) {
        file << imposs << "\n";
    }
    file.close();
}

