//
// Created by Septi on 2/4/2023.
//
#include "BranchNum.h"

void BranchN::SsmtLib() {
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const input" + std::to_string(i) + " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const output" + std::to_string(i) + " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < Ssize_; ++j) {
            smtLib_ += "(assert (= (select output" + std::to_string(i) + " (_ bv" + std::to_string(j) + " " + eleSizeS_ + ")) "
                       "(sbox (select input" + std::to_string(i) + " (_ bv" + std::to_string(j) + " " + eleSizeS_ + "))) ))\n";
        }
    }
    smtLib_ += "(declare-const inputDiff (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    smtLib_ += "(declare-const outputDiff (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    for (int i = 0; i < Ssize_; ++i) {
        smtLib_ += "(assert (= (select inputDiff (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                   "(bvxor (select input0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                                                                                                                                                                      "(select input1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + "))) ))\n";
    }
    for (int i = 0; i < Ssize_; ++i) {
        smtLib_ += "(assert (= (select outputDiff (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                   "(bvxor (select output0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                                                                                                                                                                        "(select output1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + "))) ))\n";
    }

    smtLib_ += "(declare-const inputBounder (Array Int Int))\n";
    smtLib_ += "(declare-const outputBounder (Array Int Int))\n";
    for (int i = 0; i < Ssize_; ++i) {
        smtLib_ += "(assert (= (select inputBounder " + std::to_string(i) +
                   ") (bounder (select inputDiff (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")))  ))\n";
    }
    for (int i = 0; i < Ssize_; ++i) {
        smtLib_ += "(assert (= (select outputBounder " + std::to_string(i) +
                   ") (bounder (select outputDiff (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")))  ))\n";
    }
    std::string tt;
    for (int i = 0; i < Ssize_; ++i) {
        tt += "(select inputBounder " + std::to_string(i) + ") ";
    }
    for (int i = 0; i < Ssize_; ++i) {
        tt += "(select outputBounder " + std::to_string(i) + ") ";
    }

    std::string initCon;
    for (int i = 0; i < Ssize_; ++i) {
        initCon += "(select inputBounder " + std::to_string(i) + ") ";
    }
    smtLib_ += "(assert (> (+ " + initCon + ") 1))\n";
    objMin_ += "(minimize (+ " + tt + "))\n";
    objMin_ += "(maximize (+ " + tt + "))\n";
}

void BranchN::WMsmtLib() {
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const input" + std::to_string(i) +
                   " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const output" + std::to_string(i) +
                   " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    for (int i = 0; i < 2; ++i) {
        int idx = 0;
        for (const auto& row : MatrixBS_) {
            std::string tt = "(bvgfmul " + row[0] + " (select input" + std::to_string(i) +
                             " (_ bv" + std::to_string(0) + " " + eleSizeS_ + "))) ";
            for (int j = 1; j < row.size(); ++j) {
                tt += "(bvgfmul " + row[j] + " (select input" + std::to_string(i) +
                      " (_ bv" + std::to_string(j) + " " + eleSizeS_ + "))) ";
            }
            smtLib_ += "(assert (= (select output" + std::to_string(i) +
                       " (_ bv" + std::to_string(idx) + " " + eleSizeS_ + ")) (bvxor " + tt + ")))\n";
            idx++;
        }
    }

    smtLib_ += "(declare-const inputBounder (Array Int Int))\n";
    smtLib_ += "(declare-const outputBounder (Array Int Int))\n";
    for (int i = 0; i < Msize_; ++i) {
        smtLib_ += "(assert (= (select inputBounder " + std::to_string(i) +
                   ") (bounder (bvxor (select input0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                                                                                                    " (select input1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) ))  ))\n";
    }
    for (int i = 0; i < Msize_; ++i) {
        smtLib_ += "(assert (= (select outputBounder " + std::to_string(i) +
                   ") (bounder (bvxor (select output0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                                                                                                     " (select output1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) ))  ))\n";
    }
    std::string tt;
    for (int i = 0; i < Msize_; ++i) {
        tt += "(select inputBounder " + std::to_string(i) + ") ";
    }
    for (int i = 0; i < Msize_; ++i) {
        tt += "(select outputBounder " + std::to_string(i) + ") ";
    }

    std::string initCon;
    for (int i = 0; i < Msize_; ++i) {
        initCon += "(select inputBounder " + std::to_string(i) + ") ";
    }
    smtLib_ += "(assert (> (+ " + initCon + ") 0))\n";
    objMin_ += "(minimize (+ " + tt + "))\n";
    objMax_ += "(maximize (+ " + tt + "))\n";
}

void BranchN::BMsmtLib() {
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const input" + std::to_string(i) +
                   " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const output" + std::to_string(i) +
                   " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    for (int i = 0; i < 2; ++i) {
        int idx = 0;
        for (const auto& row : MatrixBS_) {
            std::string tt = "(bvgfmul " + row[0] + " (select input" + std::to_string(i) +
                             " (_ bv" + std::to_string(0) + " " + eleSizeS_ + "))) ";
            for (int j = 1; j < row.size(); ++j) {
                tt += "(bvgfmul " + row[j] + " (select input" + std::to_string(i) +
                      " (_ bv" + std::to_string(j) + " " + eleSizeS_ + "))) ";
            }
            smtLib_ += "(assert (= (select output" + std::to_string(i) +
                       " (_ bv" + std::to_string(idx) + " " + eleSizeS_ + ")) (bvxor " + tt + ")))\n";
            idx++;
        }
    }

    smtLib_ += "(declare-const inputBounder (Array Int Int))\n";
    smtLib_ += "(declare-const outputBounder (Array Int Int))\n";
    for (int i = 0; i < Msize_; ++i) {
        for (int j = 0; j < eleSize_; ++j) {
            smtLib_ += "(assert (= (select inputBounder " + std::to_string(i*Msize_+j) +
                       ") (bounder ((_ extract " + std::to_string(j) + " " + std::to_string(j) + ")"
                                                                                                 "(bvxor (select input0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                                                                                                                                                                       " (select input1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) )))  ))\n";
        }
    }
    for (int i = 0; i < Msize_; ++i) {
        for (int j = 0; j < eleSize_; ++j) {
            smtLib_ += "(assert (= (select outputBounder " + std::to_string(i*Msize_+j) +
                       ") (bounder ((_ extract " + std::to_string(j) + " " + std::to_string(j) + ")"
                                                                                                 "(bvxor (select output0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                                                                                                                                                                        " (select output1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) )))  ))\n";
        }
    }
    std::string tt;
    for (int i = 0; i < Msize_; ++i) {
        for (int j = 0; j < eleSize_; ++j) {
            tt += "(select inputBounder " + std::to_string(i*Msize_+j) + ") ";
        }
    }
    for (int i = 0; i < Msize_; ++i) {
        for (int j = 0; j < eleSize_; ++j) {
            tt += "(select outputBounder " + std::to_string(i*Msize_+j) + ") ";
        }
    }

    std::string initCon;
    for (int i = 0; i < Msize_; ++i) {
        for (int j = 0; j < eleSize_; ++j) {
            initCon += "(select inputBounder " + std::to_string(i*Msize_+j) + ") ";
        }
    }
    smtLib_ += "(assert (> (+ " + initCon + ") 0))\n";

    objMin_ += "(minimize (+ " + tt + "))\n";
    objMax_ += "(maximize (+ " + tt + "))\n";
}

std::string BranchN::bounder(int size) {
    std::string sizeS = std::to_string(size), zeroBS = "#b";
    for (int i = 0; i < size; ++i)
        zeroBS = zeroBS + "0";
    return "(define-fun bounder ((x (_ BitVec " + sizeS + "))) Int\n"
                                                          "(ite (= x " + zeroBS + ") 0 1)  )\n";
}

std::string BranchN::sboxDef(int size) {
    std::string sizeS = std::to_string(size);
    std::string func = "(declare-const array (Array (_ BitVec " + sizeS + ") (_ BitVec " + sizeS + ")))\n";
    int idx = 0;
    for (auto ele : sbox_) {
        std::string eleB = std::to_string(utilities::d_to_b(ele));
        for (int i = eleB.size(); i < size; ++i) eleB = "0" + eleB;
        eleB = "#b" + eleB;
        func += "(assert (= (select array (_ bv" + std::to_string(idx) + " " + sizeS + ")) " + eleB + "))\n";
        idx++;
    }
    func += "(define-fun sbox ((x (_ BitVec " + sizeS + "))) (_ BitVec " + sizeS + ")\n"
                                                                                   "   (select array x)"
                                                                                   ")\n";
    return func;
}

std::string BranchN::gfmuDef(int size) {
    std::string sizeS = std::to_string(size);
    std::string func = "(declare-const array (Array (_ BitVec " + sizeS + ") (Array (_ BitVec " + sizeS + ") (_ BitVec " + sizeS + "))))\n";
    int idx = 0;
    for (const auto& row : FFm_) {
        int idx2 = 0;
        for (auto ele : row) {
            std::string eleB = std::to_string(utilities::d_to_b(ele));
            for (int i = eleB.size(); i < size; ++i) eleB = "0" + eleB;
            eleB = "#b" + eleB;
            func += "(assert (= (select (select array (_ bv" + std::to_string(idx) + " " + sizeS + ")) "
                                                                                                   "(_ bv" + std::to_string(idx2) + " " + sizeS + ")) " + eleB + "))\n";
            idx2++;
        }
        idx++;
    }
    func += "(define-fun bvgfmul ((x (_ BitVec " + sizeS + ")) (y (_ BitVec " + sizeS + ")) )(_ BitVec " + sizeS + ")\n"
                                                                                                                   "   (select (select array x) y)"
                                                                                                                   ")\n";
    return func;
}

void BranchN::WBsmtLib() {
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const inputA" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const inputB" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const output" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }

    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(assert (= output" + std::to_string(i) +
                   "( " + smtBinOP_ + " inputA" + std::to_string(i) + " inputB" + std::to_string(i) + ")))\n";
    }

    smtLib_ += "(declare-const inputDiffA (_ BitVec " + eleSizeS_ + "))\n";
    smtLib_ += "(declare-const inputDiffB (_ BitVec " + eleSizeS_ + "))\n";
    smtLib_ += "(declare-const outputDiff (_ BitVec " + eleSizeS_ + "))\n";
    smtLib_ += "(assert (= inputDiffA (bvxor inputA0 inputA1)))\n";
    smtLib_ += "(assert (= inputDiffB (bvxor inputB0 inputB1)))\n";
    smtLib_ += "(assert (= outputDiff (bvxor output0 output1)))\n";

    smtLib_ += "(declare-const inputBounderA Int)\n";
    smtLib_ += "(declare-const inputBounderB Int)\n";
    smtLib_ += "(declare-const outputBounder Int)\n";
    smtLib_ += "(assert (= inputBounderA (bounder inputDiffA)))\n";
    smtLib_ += "(assert (= inputBounderB (bounder inputDiffB)))\n";
    smtLib_ += "(assert (= outputBounder (bounder outputDiff)))\n";

    smtLib_ += "(assert (>= (+ inputBounderA inputBounderB) 1))\n";
    objMin_ += "(minimize (+ inputBounderA inputBounderB outputBounder))\n";
    objMax_ += "(maximize (+ inputBounderA inputBounderB outputBounder))\n";
}


void BranchN::BBsmtLib() {
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const inputA" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const inputB" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const output" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }

    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(assert (= output" + std::to_string(i) +
                   "( " + smtBinOP_ + " inputA" + std::to_string(i) + " inputB" + std::to_string(i) + ")))\n";
    }

    smtLib_ += "(declare-const inputDiffA (_ BitVec " + eleSizeS_ + "))\n";
    smtLib_ += "(declare-const inputDiffB (_ BitVec " + eleSizeS_ + "))\n";
    smtLib_ += "(declare-const outputDiff (_ BitVec " + eleSizeS_ + "))\n";
    smtLib_ += "(assert (= inputDiffA (bvxor inputA0 inputA1)))\n";
    smtLib_ += "(assert (= inputDiffB (bvxor inputB0 inputB1)))\n";
    smtLib_ += "(assert (= outputDiff (bvxor output0 output1)))\n";

    smtLib_ += "(declare-const inputBounderA (Array Int Int))\n";
    smtLib_ += "(declare-const inputBounderB (Array Int Int))\n";
    smtLib_ += "(declare-const outputBounder (Array Int Int))\n";
    for (int i = 0; i < eleSize_; ++i) {
        smtLib_ += "(assert (= (select inputBounderA " + std::to_string(i) + ") "
                                                                             "(bounder ((_ extract " + std::to_string(i) + " " + std::to_string(i) + ") inputDiffA))))\n";
        smtLib_ += "(assert (= (select inputBounderB " + std::to_string(i) + ") "
                                                                             "(bounder ((_ extract " + std::to_string(i) + " " + std::to_string(i) + ") inputDiffB))))\n";
        smtLib_ += "(assert (= (select outputBounder " + std::to_string(i) + ") "
                                                                             "(bounder ((_ extract " + std::to_string(i) + " " + std::to_string(i) + ") outputDiff))))\n";
    }

    std::string tt;
    for (int i = 0; i < eleSize_; ++i) {
        tt += "(select inputBounderA " + std::to_string(i) + ") ";
    }
    for (int i = 0; i < eleSize_; ++i) {
        tt += "(select inputBounderB " + std::to_string(i) + ") ";
    }
    smtLib_ += "(assert (>= (+ " + tt + ") 1))\n";
    for (int i = 0; i < eleSize_; ++i) {
        tt += "(select outputBounder " + std::to_string(i) + ") ";
    }
    objMin_ += "(minimize (+ " + tt + "))\n";
    objMax_ += "(maximize (+ " + tt + "))\n";
}


std::string BranchN::smtBinOPget(std::string op) {
    if (op == "XOR")
        return "bvxor";
    else if (op == "AND")
        return "bvand";
    else if (op == "OR")
        return "bvor";
    else if (op == "ADD")
        return "bvadd";
    else if (op == "SUB")
        return "bvsub";

}

void BranchN::BCsmtLib() {
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const input" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const output" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }
    smtLib_ += "(assert (= output0 (bvgfmul " + constantS + " input0)) )\n";
    smtLib_ += "(assert (= output1 (bvgfmul " + constantS + " input1)) )\n";

    smtLib_ += "(declare-const inputBounder (Array Int Int))\n";
    smtLib_ += "(declare-const outputBounder (Array Int Int))\n";
    for (int i = 0; i < eleSize_; ++i) {
        smtLib_ += "(assert (= (select inputBounder " + std::to_string(i) +
                   ") (bounder ((_ extract " + std::to_string(i) + " " + std::to_string(i) + ")"
                                                                                             "(bvxor input0 input1)) ) ))\n";
    }
    for (int i = 0; i < eleSize_; ++i) {
        smtLib_ += "(assert (= (select outputBounder " + std::to_string(i) +
                   ") (bounder ((_ extract " + std::to_string(i) + " " + std::to_string(i) + ")"
                                                                                             "(bvxor output0 output1)) ) ))\n";
    }

    std::string tt;
    for (int j = 0; j < eleSize_; ++j) {
        tt += "(select inputBounder " + std::to_string(j) + ") ";
    }
    for (int j = 0; j < eleSize_; ++j) {
        tt += "(select outputBounder " + std::to_string(j) + ") ";
    }

    std::string initCon;
    for (int j = 0; j < eleSize_; ++j) {
        initCon += "(select inputBounder " + std::to_string(j) + ") ";
    }
    smtLib_ += "(assert (> (+ " + initCon + ") 0))\n";
    objMin_ += "(minimize (+ " + tt + "))\n";
    objMax_ += "(maximize (+ " + tt + "))\n";
}


void BranchN::BVsmtLib() {
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const input" + std::to_string(i) + " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const output" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }

    for (int i = 0; i < 2; ++i) {
        std::string te;
        for (int j = 0; j < vecSize; ++j) {
            te += "(bvgfmul (select input" + std::to_string(i) + " (_ bv" + std::to_string(j) + " " + eleSizeS_ + ")) "
                  + inputConsS[j] + ")";
        }
        smtLib_ += "(assert (= output" + std::to_string(i) + " (bvxor " + te + " ) ) )\n";
    }

    smtLib_ += "(declare-const inputBounder (Array Int Int))\n";
    smtLib_ += "(declare-const outputBounder (Array Int Int))\n";
    for (int i = 0; i < vecSize; ++i) {
        for (int j = 0; j < eleSize_; ++j) {
            smtLib_ += "(assert (= (select inputBounder " + std::to_string(i*eleSize_+j) +
                       ") (bounder ((_ extract " + std::to_string(j) + " " + std::to_string(j) + ")"
                                                                                                 "(bvxor (select input0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                                                                                                                                                                       "(select input1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) )) ) ))\n";
        }
    }
    for (int i = 0; i < eleSize_; ++i) {
        smtLib_ += "(assert (= (select outputBounder " + std::to_string(i) +
                   ") (bounder ((_ extract " + std::to_string(i) + " " + std::to_string(i) + ")"
                                                                                             "(bvxor output0 output1)) ) ))\n";
    }

    std::string tt;
    for (int j = 0; j < eleSize_ * vecSize; ++j) {
        tt += "(select inputBounder " + std::to_string(j) + ") ";
    }
    for (int j = 0; j < eleSize_; ++j) {
        tt += "(select outputBounder " + std::to_string(j) + ") ";
    }

    for (int i = 0; i < vecSize; ++i) {
        std::string initCon;
        for (int j = 0; j < eleSize_; ++j) {
            initCon += "(select inputBounder " + std::to_string(i*eleSize_+j) + ") ";
        }
        smtLib_ += "(assert (> (+ " + initCon + ") 0))\n";
    }
    objMin_ += "(minimize (+ " + tt + "))\n";
    objMax_ += "(maximize (+ " + tt + "))\n";
}
