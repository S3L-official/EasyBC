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
    // 根据sbox描述输入与输出的关系
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < Ssize_; ++j) {
            smtLib_ += "(assert (= (select output" + std::to_string(i) + " (_ bv" + std::to_string(j) + " " + eleSizeS_ + ")) "
                       "(sbox (select input" + std::to_string(i) + " (_ bv" + std::to_string(j) + " " + eleSizeS_ + "))) ))\n";
        }
    }
    // 初始化并计算input difference和output difference
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

    // 初始化两个int类型的数组，保证其每个元素的值绑定两个difference数组的值
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

    // 需要加一个初始化约束，即输入差分大于等于1
    std::string initCon;
    for (int i = 0; i < Ssize_; ++i) {
        initCon += "(select inputBounder " + std::to_string(i) + ") ";
    }
    smtLib_ += "(assert (> (+ " + initCon + ") 1))\n";
    objMin_ += "(minimize (+ " + tt + "))\n";
    objMin_ += "(maximize (+ " + tt + "))\n";
}

void BranchN::WMsmtLib() {
    // 先创建4个array，分别作为2对输入输出的差分向量
    // 然后根据矩阵的Size确定输入输出的每个元素
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const input" + std::to_string(i) +
                   " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const output" + std::to_string(i) +
                   " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    // 计算每个输出array元素的值（矩阵乘法）
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
    // 初始化并计算input difference和output difference
    //smtLib_ += "(declare-const inputDiff (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    //smtLib_ += "(declare-const outputDiff (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    /*for (int i = 0; i < Msize_; ++i) {
        smtLib_ += "(assert (= (select inputDiff (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                "(bvxor (select input0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                "(select input1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + "))) ))\n";
    }
    for (int i = 0; i < Msize_; ++i) {
        smtLib_ += "(assert (= (select outputDiff (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                "(bvxor (select output0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                "(select output1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + "))) ))\n";
    }*/


    /*for (int i = 0; i < Msize_; ++i) {
        smtLib_ += "(assert (= (select inputDiff (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                "(bvxor (select input0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                  "(select input1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + "))) ))\n";
    }
    for (int i = 0; i < Msize_; ++i) {
        smtLib_ += "(assert (= (select outputDiff (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                 "(bvxor (select output0 (_ bv" + std::to_string(i) + " " + eleSizeS_ + ")) "
                "(select output1 (_ bv" + std::to_string(i) + " " + eleSizeS_ + "))) ))\n";
    }*/


    // 初始化两个int类型的数组，保证其每个元素的值绑定两个difference数组的值
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

    // 需要加一个初始化约束，即输入差分大于等于1
    std::string initCon;
    for (int i = 0; i < Msize_; ++i) {
        initCon += "(select inputBounder " + std::to_string(i) + ") ";
    }
    smtLib_ += "(assert (> (+ " + initCon + ") 0))\n";
    objMin_ += "(minimize (+ " + tt + "))\n";
    objMax_ += "(maximize (+ " + tt + "))\n";
}

void BranchN::BMsmtLib() {
    // 先创建4个array，分别作为2对输入输出的差分向量
    // 然后根据矩阵的Size确定输入输出的每个元素
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const input" + std::to_string(i) +
                   " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const output" + std::to_string(i) +
                   " (Array (_ BitVec " + eleSizeS_ + ") (_ BitVec " + eleSizeS_ + ")))\n";
    }
    // 计算每个输出array元素的值（矩阵乘法）
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

    // 初始化两个int类型的数组，保证其每个元素的值绑定两个difference数组的值
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

    // 需要加一个初始化约束，即输入差分大于等于1

    /*for (int i = 0; i < Msize_; ++i) {
        std::string initCon;
        for (int j = 0; j < eleSize_; ++j) {
            initCon += "(select inputBounder " + std::to_string(i*Msize_+j) + ") ";
        }
        smtLib_ += "(assert (> (+ " + initCon + ") 0))\n";
    }
    for (int i = 0; i < Msize_; ++i) {
        std::string initCon;
        for (int j = 0; j < eleSize_; ++j) {
            initCon += "(select outputBounder " + std::to_string(i*Msize_+j) + ") ";
        }
        smtLib_ += "(assert (> (+ " + initCon + ") 0))\n";
    }*/

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

// 这里的函数用于将bitvector的值和int类型的值进行绑定
// 其接受输入应该是一个bitvector，返回一个int类型
// 如果其接受的bitvector的值为0，那么返回0；否则返回1
/*
 *  (declare-const array (Array (_ BitVec 2) (Array (_ BitVec 2) (_ BitVec 2))))
    (assert (= (select (select array (_ bv0 2)) (_ bv0 2)) #b11))
    (assert (= (select (select array (_ bv0 2)) (_ bv1 2)) #b00))
    (declare-const a Int)
    (declare-const b Int)
    (declare-const c Int)
    (define-fun bounder ((x (_ BitVec 2))) Int
       (ite (= x #b01) 0 1)
    )
    (assert (= a (bounder (select (select array (_ bv1 2)) (_ bv1 2)))))
    (assert (= b (bounder (select (select array (_ bv0 2)) (_ bv1 2)))))
    (assert (= c (bounder (select (select array (_ bv1 2)) (_ bv0 2)))))
    (minimize (+ (+ a b) c))
    (check-sat)
    (get-model)
 */
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

/*
 * 这里我们需要定义有限域乘法的函数，有两种实现思路：
 * 1) 一种是直接定义二维Bit Vector，通过查找两个index即可返回对应的相乘结果:
 *  demo:
 *      (declare-const array (Array (_ BitVec 2) (Array (_ BitVec 2) (_ BitVec 2))))
 *      (assert (= (select (select array #b00) #b00) #b01))
 *
 * 2) 一种是定义一个一维的Bit Vector，通过两个数加起来得到相乘结果的index:
 *  Remark:
 *      通过输入的两个bitvector得到最终的index，但是bitvector好像没办法直接进行乘法，这个时候我们要怎么办呢。
 * */
// 这里给的size应该是矩阵中每个元素类型uints中的s
std::string BranchN::gfmuDef(int size) {
    std::string sizeS = std::to_string(size);
    std::string func = "(declare-const array (Array (_ BitVec " + sizeS + ") (Array (_ BitVec " + sizeS + ") (_ BitVec " + sizeS + "))))\n";
    int idx = 0;
    for (const auto& row : FFm_) {
        int idx2 = 0;
        for (auto ele : row) {
            // 先将FFm中的每个元素转化为长度为sizeS的二进制字符串
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

// word wise branch number of binary operators
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

    // 根据op选择inputA，inputB和output之间的关系
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

    // 根据op选择inputA，inputB和output之间的关系
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
    // 先创建4个array，分别作为2对输入输出的差分向量
    // 然后根据矩阵的Size确定输入输出的每个元素
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const input" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }
    for (int i = 0; i < 2; ++i) {
        smtLib_ += "(declare-const output" + std::to_string(i) + " (_ BitVec " + eleSizeS_ + "))\n";
    }
    smtLib_ += "(assert (= output0 (bvgfmul " + constantS + " input0)) )\n";
    smtLib_ += "(assert (= output1 (bvgfmul " + constantS + " input1)) )\n";

    // 初始化两个int类型的数组，保证其每个元素的值绑定两个difference数组的值
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

    // 需要加一个初始化约束，即输入差分大于等于1
    std::string initCon;
    for (int j = 0; j < eleSize_; ++j) {
        initCon += "(select inputBounder " + std::to_string(j) + ") ";
    }
    smtLib_ += "(assert (> (+ " + initCon + ") 0))\n";
    objMin_ += "(minimize (+ " + tt + "))\n";
    objMax_ += "(maximize (+ " + tt + "))\n";
}


// 此时的输入应该是一个vector变量，一个vector常量，相乘结果异或得到一个output
void BranchN::BVsmtLib() {
// 先创建4个array，分别作为2对输入输出的差分向量
    // 然后根据矩阵的Size确定输入输出的每个元素
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

    // 初始化两个int类型的数组，保证其每个元素的值绑定两个difference数组的值
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

    // 这里要保证每个input entry的input difference至少为1
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
