//
// Created by Septi on 9/6/2022.
//

#ifndef EASYBC_SYNTAXGUIDED_H
#define EASYBC_SYNTAXGUIDED_H

#include "z3++.h"
#include "string"
#include "vector"
#include "fstream"
#include "util/utilities.h"

class SGsyn {
private:
    z3::context context;
    z3::solver solver;

    z3::expr_vector xb;
    z3::expr_vector target_coeff;
    z3::expr pre_con;
    std::vector<std::vector<int>> crr_ineq;
    int dim;
    int target_num;
    int timer = 120;
    std::string round_results;
    int mode = 2;

    int solveTime;

    std::string smtPath1 = "../data/sbox/smt-lib/";
    std::string smtPath;

public:
    SGsyn(int timer, std::vector<std::vector<int>> ineq_int)
            : solver(context), xb(context), target_coeff(context),
              pre_con(context), timer(timer), crr_ineq(ineq_int) {
        dim = ineq_int[0].size() - 1;
        if (mode == 2)
            dichotomySyn();
        else
            minus1Syn();
    }

    void init() {
        for (int i = 1; i < dim + 1; i++)
            xb.push_back(context.int_const(("x" + std::to_string(i)).c_str()));

        pre_con = (xb[0] <= 1 && xb[0] >= 0);
        for (int i = 1; i < dim; i++)
            pre_con = pre_con and (xb[i] <= 1 && xb[i] >= 0);


        for (int j = 0; j < target_num; j++)
            for (int i = 0; i < dim + 1; i++)
                target_coeff.push_back(context.int_const(("c" + std::to_string((j + 1) * 100 + (i + 1))).c_str()));

        z3::expr targetInput(context);
        for (int i = 0; i < crr_ineq.size(); i++){
            z3::expr tExpr(context);
            tExpr = crr_ineq[i][0] * xb[0];
            for (int j = 1; j < dim; ++j) tExpr = tExpr + crr_ineq[i][j] * xb[j];
            tExpr = tExpr + crr_ineq[i][dim] >= 0;
            if (i == 0) targetInput = tExpr;
            else targetInput = targetInput and tExpr;
        }

        z3::expr targetMerge(context);
        for (int j = 0; j < target_num; j++){
            z3::expr target_inq(context);
            for (int i = 0; i < dim + 1; i++)
                if (i == 0)
                    target_inq = target_coeff[j * dim + j] * xb[i];
                else if (i < dim)
                    target_inq = target_inq + target_coeff[j * dim + i + j] * xb[i];
                else
                    target_inq = target_inq + target_coeff[j * dim + i + j] >= 0;
            if (j == 0) targetMerge = target_inq;
            else targetMerge = targetMerge and target_inq;
        }
        solver.push();
        solver.add(forall(xb, implies(pre_con, targetMerge == targetInput)));
    }

    void run() {
        init();
        z3::set_param("parallel.enable", true);
        z3::set_param("solver.timeout", timer * 1000);

        std::string aa = solver.to_smt2();
        //std::cout << "aa : " << aa << std::endl;
        std::ofstream file_save;
        file_save.open(smtPath);
        file_save << aa;
        file_save.close();

        std::string result = std::to_string(solver.check());
        if ( result != "1"){
            std::cout << "unsat" << std::endl;
            round_results = "unsat";
            return;
        }
        round_results = "sat";
        std::vector<std::vector<int>> tmpOut;
        for (int j = 0; j < target_num; j++){
            std::vector<int> targetCc;
            for (int i = 0; i < dim + 1; i++){
                std::vector<std::string> tmpcc = utilities::split(
                        solver.get_model().eval(target_coeff[j * dim + i + j]).to_string(), "() ");
                std::string tmpStr;
                for (auto & k2 : tmpcc) tmpStr += k2;
                targetCc.push_back(std::stoi(tmpStr));
            }
            tmpOut.push_back(targetCc);
        }
        std::cout << "current num : " << tmpOut.size() << std::endl;
        crr_ineq = tmpOut;
    }

    void dichotomySyn() {
        int lowerBound = 0, upperBound = crr_ineq.size();
        int counter = 1;
        time_t startTime, endTime;
        time(&startTime);
        while (lowerBound + 1 < upperBound) {
            target_num = ceil(((upperBound - lowerBound) / 2 + lowerBound));
            std::cout << "******** " << counter << " round ********" << std::endl;
            counter++;
            if (counter == 5)
                break;

            smtPath = smtPath1 + std::to_string(counter) + ".txt";
            time_t startTimeR, endTimeR;
            time(&startTimeR);
            run();
            time(&endTimeR);

            solver.reset();
            if (round_results == "sat") {
                upperBound = target_num;
            } else {
                lowerBound = target_num;
            }
            crrPrint();
            std::cout << std::endl;

            std::cout << "Round time : " << difftime(endTimeR, startTimeR) << "s" << std::endl << std::endl;
        }
        time(&endTime);
        solveTime = difftime(endTime, startTime);
        std::cout << "Reduction time : " << difftime(endTime, startTime) << "s" << std::endl << std::endl;
    }

    void minus1Syn() {
        int counter = 1;
        time_t startTime, endTime;
        time(&startTime);
        target_num = crr_ineq.size() / 8;
        while (true) {
            std::cout << "******** " << counter << " round ********" << std::endl;
            std::cout << "target num : " << target_num << std::endl;
            counter++;
            run();
            solver.reset();
            crrPrint();
            if (round_results == "unsat")
                break;
            target_num = crr_ineq.size() - 1;
        }
        time(&endTime);
        solveTime = difftime(endTime, startTime);
    }

    void crrPrint() {
        std::vector<std::vector<std::string>> tmp_out_cc;
        std::cout << "Coefficients of current results : " << std::endl;
        for (const auto& item : crr_ineq) {
            for (auto i: item) std::cout << i << " ";
            std::cout << std::endl;
        }
    }

    std::vector<std::vector<int>> getCrrIneq() {return crr_ineq;}
    int getSolveTime() {return solveTime;}
};

#endif //EASYBC_SYNTAXGUIDED_H
