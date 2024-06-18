#include <iostream>
#include <ASTNode.h>
#include "Value.h"
#include "Interpreter.h"
#include "Transformer.h"
#include "differential/DiffSBMILP.h"
#include "differential/DiffSWMILP.h"
#include "interpreter/semanticInter.h"


extern int yyparse();
extern int yydebug;
extern FILE *yyin;
extern int yylineno;
extern std::shared_ptr<ASTNode::NBlock> programRoot;

int ASTNodeCounter = 0;

std::map<std::string, std::vector<int>> allBox = {};
std::map<std::string, std::vector<int>> pboxM = {};
std::map<std::string, int> pboxMSize = {};
std::map<std::string, std::vector<int>> Ffm = {};
std::string cipherName;

void paramProcess(std::string mode, std::string cipherName);
void SboxModelingMGR(std::vector<std::string> params);
void MILPMGR(std::vector<std::string> params);
void EasyBCInterpreter(std::vector<std::string> params);
std::vector<double> EasyBCInterpreterDebug(std::vector<std::string> params);

int main(int argc, const char* argv[]) {
    std::vector<std::string> params;
    for (int i = 1; i < argc; ++i) {
        params.emplace_back(argv[i]);
    }

    // print help
    if (argc > 1 and params[0] == "-h") {
        std::cout << "Welcome to EasyBC!\n"
                     "Available commands are:\n"
                     " -benchmark : list all block ciphers and key-less permutation in our benchmarks.\n"
                     " -s cipherName : perform security analysis of the cryptographic primitive against differential cryptanalysis.\n"
                     " -i cipherName : encrypt cryptographic primitive implemented in EasyBC by EasyBC interpreter.\n"
                     "You can find the output data in the 'data' folder!" << std::endl;
    } else if (argc > 1 and params[0] == "-benchmark") {
        std::cout << "block ciphers: \n"
                     "AES, DES, GIFT-64, KLEIN, LBlock, MIBS, Piccolo, PRESENT, Rectangle, SIMON32, SIMON48, SIMON64, SKINNY-64, TWINE\n"
                     "key-less permutations: \n"
                     "ASCON, Elephant, GIFT-COFB, GRAIN, ISAP, Photon, Romulus, SPARKLE, TinyJAMBU, Xoodyak" << std::endl;
    } else if (argc > 1 and params[0] == "-block ciphers") {
        std::cout << "block ciphers: \n"
                     "AES, DES, GIFT-64, KLEIN, LBlock, MIBS, Piccolo, PRESENT, Rectangle, SIMON32, SIMON48, SIMON64, SKINNY-64, TWINE" << std::endl;
    } else if (argc > 1 and params[0] == "-nist") {
        std::cout << "key-less permutations: \n"
                     "ASCON, Elephant, GIFT-COFB, GRAIN, ISAP, Photon, Romulus, SPARKLE, TinyJAMBU, Xoodyak" << std::endl;
    } else if (argc > 1 and (params[0] == "-s" or params[0] == "-i")) {
        paramProcess(params[0], params[1]);
    } else {
        params.clear();
        std::string path = "../parameters.txt";
        std::ifstream file;
        file.open(path);
        std::string model, line;
        std::string whiteSpaces = " \n\r\t\f\v";
        while (getline(file, line)) {
            // trim and save parameters
            size_t first_non_space = line.find_first_not_of(whiteSpaces);
            line.erase(0, first_non_space);
            size_t last_non_space = line.find_last_not_of(whiteSpaces);
            line.erase(last_non_space + 1);
            params.push_back(line);
        }
        file.close();
        int paramNum = params.size();
        if (paramNum == 4) {
            SboxModelingMGR(params);
        } else if (paramNum >= 5) {
            MILPMGR(params);
        } else {
            std::cout << "No input parameters !" << std::endl;
            assert(false);
        }
    }
    return 0;
}

void paramProcess(std::string mode, std::string cipherName) {
    std::string RunCipherName = setup::cryptPrimitiveMap[cipherName];
    std::vector<std::string> params;
    params.clear();
    std::string path = "../data/param/parameters.txt";

    std::ofstream fParamWrite(path, std::ios::trunc);
    if (!fParamWrite){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        if (mode == "-s") {
            fParamWrite << "9\n";
            fParamWrite << "../benchmarks/" + setup::cryptPrimitiveSetMap[cipherName] + "/" + RunCipherName + ".cl\n";
            fParamWrite << setup::cryptPrimitiveSetMapSup[cipherName] << "\n";
            if (setup::cryptPrimitiveSetMapSup[cipherName] == "n") {
                std::cout << "the security analysis of stream cipher is not supported now !" << std::endl;
                assert(false);
            }
            fParamWrite << "cryptanalysis\n";
            fParamWrite << "1\n";
            fParamWrite << "startRound\n";
            fParamWrite << "1\n";
            fParamWrite << "endRound\n";
            fParamWrite << "10";
        } else if (mode == "-i") {
            fParamWrite << setup::cryptPrimitiveInterMap[cipherName] << "\n";
            fParamWrite << RunCipherName;
        }
    }
    fParamWrite.close();

    std::ifstream file;
    file.open(path);
    std::string model, line;
    std::string whiteSpaces = " \n\r\t\f\v";
    while (getline(file, line)) {
        // trim and save parameters
        size_t first_non_space = line.find_first_not_of(whiteSpaces);
        line.erase(0, first_non_space);
        size_t last_non_space = line.find_last_not_of(whiteSpaces);
        line.erase(last_non_space + 1);
        params.push_back(line);
    }
    file.close();

    int paramNum = params.size();
    if (params[1] == "plaintext" and params[3] == "subkey") {
        EasyBCInterpreterDebug(params);
    } else if (paramNum == 2) {
        EasyBCInterpreter(params);
    } else if (paramNum == 4) {
        SboxModelingMGR(params);
    } else if (paramNum >= 5) {
        MILPMGR(params);
    } else
        assert(false);
}

void SboxModelingMGR(std::vector<std::string> params) {
    std::string sboxName = params[0];
    std::vector<string> sboxStr = utilities::split(params[1], ",");
    std::vector<int> sbox;
    for (const auto &ele: sboxStr) { sbox.push_back(std::stoi(ele)); };
    std::string mode = params[2];
    int redMd = std::stoi(params[3]);
    SboxM sboxM(sboxName, sbox, mode);
    std::vector<std::vector<int>> redResults = Red::reduction(redMd, sboxM);
}

void MILPMGR(std::vector<std::string> params) {
    /*
     * argv[0] -> the number of parameters
     * argv[1] -> EasyBC implementation file path;
     * argv[2] -> word-wise or bit-wise or extended bit-wise
     *            techChoose = "w" -> word-wise
     *            techChoose = "b" -> bit-wise
     *            techChoose = "d" -> extended bit-wise
     *            the modeling mode for S-boxes, same as the "case 1", is depended on "w", "b" or "d"
     * argv[3] -> "cryptanalysis" or "evaluation"
     * argv[4] -> reduction methods for S-boxes; same as the "case 1";
     *
     * optional parameters :
     * argv[5]/argv[7]/argv[9]/argv[11]/argv[13]/argv[15]
     *            -> startRound or allRounds or timer(second) or threadsNum or totalRoundNum or evaluationSize
     * argv[6]/argv[8]/argv[10]/argv[12]/argv[14]/argv[16]
     *            -> startRound or allRounds or timer(second) or threadsNum or totalRoundNum or evaluationSize
     * */
    std::string filePath = params[1];
    yyin = fopen(filePath.c_str(), "r");
    if (!yyin) {
        std::cout << "Wrong Path : \n" << filePath << std::endl;
        assert(false);
    }
    yydebug = 0;
    yylineno = 1;
    std::vector<ProcValuePtr> res;
    Interpreter interpreter;
    if (!yyparse()) {
        std::cout << "Parsing complete\n" << std::endl;
    } else {
        std::cout << "Hint : wrong syntax at line " << yylineno << std::endl;
        assert(false);
    }
    interpreter.generateCode(*programRoot);
    res = interpreter.getProcs();

    Transformer transformer(res);
    transformer.transformProcedures();

    int startRound = 1, endRound = 10, timer = 3600 * 24, threadsNum = 16, totalRoundNum = 0;
    int evaluationSize = 0;
    for (int i = 5; i < std::stoi(params[0]); i = i + 2) {
        if (params[i] == "startRound") {
            startRound = std::stoi(params[i + 1]);
        } else if (params[i] == "endRound") {
            endRound = std::stoi(params[i + 1]);
        } else if (params[i] == "timer") {
            timer = std::stoi(params[i + 1]);
        } else if (params[i] == "threadsNum") {
            threadsNum = std::stoi(params[i + 1]);
        } else if (params[i] == "totalRoundNum") {
            totalRoundNum = std::stoi(params[i + 1]);
        } else if (params[i] == "evaluationSize") {
            evaluationSize = std::stoi(params[i + 1]);
        } else
            assert(false);
    }

    std::string sboxModelingMode;
    if (params[2] == "w" or params[2] == "b") sboxModelingMode = "AS";
    else if (params[2] == "d") sboxModelingMode = "DC";
    else
        assert(false);

    if (params[3] == "evaluation" and totalRoundNum == 0) {
        std::cout << "the parameters 'totalRoundNum' should be given when the target is 'evaluation' !" << std::endl;
        assert(false);
    }

    if (params[2] == "w") {
        DiffSWMILP sw(transformer.getProcedureHs(), params[3]);
        sw.setGurobiTimer(timer);
        sw.setGurobiThreads(threadsNum);
        sw.setStartRound(startRound);
        sw.setEndRound(endRound);
        if (params[3] == "cryptanalysis") {
            sw.setILP();
        }
        if (evaluationSize != 0)
            sw.setEvaluationSize(evaluationSize);
        sw.setTotalRoundNum(totalRoundNum);
        sw.MGR();
    }
    // bit-wise or extended bit-wise
    else if (params[2] == "b" or params[2] == "d") {
        DiffSBMILP sb(transformer.getProcedureHs(), params[3], sboxModelingMode, std::stoi(params[4]));
        sb.setGurobiTimer(timer);
        sb.setGurobiThreads(threadsNum);
        sb.setStartRound(startRound);
        sb.setEndRound(endRound);
        if (params[3] == "cryptanalysis") {
            sb.setSpeedUp1();
            sb.setSpeedUp2();
            sb.setILP();
        }
        if (evaluationSize != 0)
            sb.setEvaluationSize(evaluationSize);
        sb.setTotalRoundNum(totalRoundNum);
        sb.MGR();
    } else {
        std::cout << "the fourth argument should be 'w', 'b' or 'd' !" << std::endl;
        assert(false);
    }
}

void EasyBCInterpreter(std::vector<std::string> params) {
    int testNum = 100;
    std::string type = params[0]; // blockCipher or NIST
    std::string cipherName = params[1];
    std::string path = "../data/interpreterInput/" + type + "/" + cipherName + "/";
    std::vector<std::vector<double>> allEncryptionTime;
    for (int i = 0; i < testNum; ++i) {
        std::cout << " ======================= TEST CASE " << i << " ======================= " << std::endl;
        std::string inputFilePath = path + std::to_string(i) + ".txt";
        std::ifstream file;
        file.open(inputFilePath);
        if (!file){
            std::cout << "Wrong input file path : " << inputFilePath << std::endl;
        } else {
            std::string model, line;
            std::string whiteSpaces = " \n\r\t\f\v";
            std::vector<std::string> caseParams;
            while (getline(file, line)) {
                size_t first_non_space = line.find_first_not_of(whiteSpaces);
                line.erase(0, first_non_space);
                size_t last_non_space = line.find_last_not_of(whiteSpaces);
                line.erase(last_non_space + 1);
                caseParams.push_back(line);
            }
            file.close();
            std::vector<double> encryptionTime = EasyBCInterpreterDebug(caseParams);
            allEncryptionTime.push_back(encryptionTime);
            std::cout << " =============== TEST CASE " << i << " ENCRYPTION FINISH =============== \n" << std::endl;
        }
    }

    std::string encTimePath = path + "encryptionTime/";
    system(("mkdir -p " + encTimePath).c_str());
    encTimePath += "encTime.txt";
    system(("touch " + encTimePath).c_str());

    std::ofstream f(encTimePath, std::ios::trunc);
    double AverageClockTime = 0, AverageTimeTime = 0;
    for (int i = 0; i < allEncryptionTime.size(); ++i) {
        f << " ============== test case " << i << "  ============== \n";
        f << "clock time : " << allEncryptionTime[i][0] << "s\n";
        f << "time time : " << allEncryptionTime[i][1] << "s\n\n";
        AverageClockTime += allEncryptionTime[i][0];
        AverageTimeTime += allEncryptionTime[i][1];
    }
    f << "average clock time : " << AverageClockTime/testNum << "s\n";
    f << "average time time : " << AverageTimeTime/testNum << "s\n";
    f.close();
    std::cout << "Print all encryption time to : " << encTimePath << std::endl;
}

std::vector<double> EasyBCInterpreterDebug(std::vector<std::string> params) {
    /*
     * argv[0] -> EasyBC implementation file path;
     * argv[1] -> "plaintext"
     * argv[2] -> the value of plaintext in binary
     * argv[3] -> "subkey"
     * argv[4] -> the subkey size
     * argv[5]~ -> the value of subkeys
     * */
    std::string filePath = params[0];
    yyin = fopen(filePath.c_str(), "r");
    if (!yyin) {
        std::cout << "Wrong Path : \n" << filePath << std::endl;
        assert(false);
    }
    yydebug = 0;
    yylineno = 1;
    std::vector<ProcValuePtr> res;
    Interpreter interpreter;
    if (!yyparse()) {
        std::cout << "Parsing complete\n" << std::endl;
    } else {
        std::cout << "Hint : wrong syntax at line " << yylineno << std::endl;
        assert(false);
    }

    interpreter.generateCode(*programRoot);
    res = interpreter.getProcs();

    for (auto array : interpreter.getblockStack()[0]->env) {
        if (array.second->getValueType() == VTArrayValue) {
            ArrayValue *arrayV = dynamic_cast<ArrayValue *>(array.second.get());
            std::vector<int> arrayEle;
            for (auto ele : arrayV->getArrayValue()) {
                auto* concreteNumValue = dynamic_cast<ConcreteNumValue*>(ele.get());
                arrayEle.push_back(concreteNumValue->getNumer());
            }
            allBox[array.first] = arrayEle;
        }
    }

    Transformer transformer(res);
    transformer.transformProcedures();

    std::string plaintextStr = params[2].substr(2, params[2].size()-2);
    std::vector<int> plaintext;
    for (auto p : plaintextStr) {
        if (p == '1')
            plaintext.push_back(1);
        else if (p == '0')
            plaintext.push_back(0);
    }

    int subKeySize = std::stoi(params[4]);
    std::vector<std::vector<int>> subKeys;
    for (int i = 5; i < params.size() - 2; ++i) {
        std::string subkeyStr = params[i].substr(2, params[i].size()-2);
        std::string preZeroBits;
        for (int j = subkeyStr.size(); j < subKeySize; ++j) {
            preZeroBits += "0";
        }
        subkeyStr = preZeroBits + subkeyStr;

        std::vector<int> subkeyBit;
        for (auto k : subkeyStr) {
            if (k == '1')
                subkeyBit.push_back(1);
            else if (k == '0')
                subkeyBit.push_back(0);
        }
        subKeys.push_back(subkeyBit);
    }

    std::vector<int> ciphertext;
    std::string ciphertextStr = params[params.size()-1];
    for (auto c : ciphertextStr) {
        if (c == '1')
            ciphertext.push_back(1);
        else if (c == '0')
            ciphertext.push_back(0);
    }

    EasyBCInter easyBcInter(transformer.getProcedureHs(), plaintext, subKeys);

    clock_t startTime, endTime;
    startTime = clock();
    time_t star_time = 0, end_time;
    star_time = time(NULL);

    easyBcInter.interpreter();

    endTime = clock();
    end_time = time(NULL);
    double clockTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    double timeTime = difftime( end_time, star_time);

    std::vector<int> encryptResult = easyBcInter.getCiphertext();

    std::cout << "enc : \n";
    for (int i = 0; i < encryptResult.size(); ++i) {
        std::cout << encryptResult[i];
    }
    std::cout << std::endl;

    std::vector<double> encryptTime;
    encryptTime.push_back(clockTime);
    encryptTime.push_back(timeTime);
    return encryptTime;
}
