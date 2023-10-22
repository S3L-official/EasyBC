//
// Created by Septi on 9/7/2022.
//
#include "Reduction.h"

void Red::rtn_save(SboxM sboxM, const std::vector<std::vector<int>> &rtn, double time, std::string alg) {
    std::string rtn_save = sboxM.get_path() + "reduction/" + std::to_string(sboxM.get_sbox_len()) + "bits/";
    std::string rtn_stat = sboxM.get_path() + "statistics/" + std::to_string(sboxM.get_sbox_len()) + "bits/";
    system(("mkdir -p " + rtn_save).c_str());
    system(("mkdir -p " + rtn_stat).c_str());
    rtn_save += sboxM.get_name() + ".txt";
    rtn_stat += sboxM.get_name() + ".txt";
    system(("touch " + rtn_save).c_str());
    system(("touch " + rtn_stat).c_str());
    std::ofstream fs;
    fs.open(rtn_save);
    std::cout << "the results of " + alg + " : " << std::endl;
    for (auto & i : rtn) {
        for (int j : i) {
            std::cout << j << " ";
            fs << j << " ";
        }
        std::cout << std::endl;
        fs << std::endl;
    }
    fs.close();
    std::cout << "Total num of " + alg + "'s results : " << rtn.size() << std::endl;
    std::cout << "Reduction time : " << time << "s" << std::endl << std::endl;
    // save the information of reduction process
    std::ofstream stat_s;
    stat_s.open(rtn_stat, std::ios::app);
    stat_s << alg + " : " << std::endl;
    stat_s << "#constrains : " << rtn.size() << std::endl;
    stat_s << "time : " << time << "s\n\n" << std::endl;
    stat_s.close();
}

std::vector<std::vector<int>> Red::greedy_sun(SboxM sboxM, std::vector<std::vector<int>> ineq_int) {
    std::vector<std::vector<int>> rtn;
    std::vector<Check::Ineq> ineqs = Check::ineq_obt(ineq_int);
    std::vector<int> ref_ans = Check::ref_imposs_init(sboxM);
    int dim = ineqs[0].get_dim();
    time_t startTime, endTime;
    time(&startTime);
    // init bool for solution
    bool bref[int(pow(2, dim))];
    for (int i = 0; i < int(pow(2, dim)); ++i) bref[i] = false;
    for (int ref_an : ref_ans) bref[ref_an] = true;
    // iteration and greedy
    int po_n = ref_ans.size();
    while (true){
        int counter = 0;
        std::vector<int> re, ineq;
        for (auto & i : ineqs) {
            std::vector<int> rm;
            int it = 0;
            for (int & ref_an : ref_ans)
                // ref_ans[j] does not satisfy ineq[i]
                if (bref[ref_an] and !i(ref_an)){
                    rm.push_back(ref_an);
                    it++;
                    i.add_mv_point(ref_an);
                }
            if (counter <= it){
                counter = it;
                re = rm;
                ineq = i.get_ineq();
            }
        }
        rtn.push_back(ineq);
        po_n -= counter;
        /*if (po_n <= 0) break;*/
        for (int i : re) bref[i] = false;
        for (auto & i : ineqs) i.reset_mv_points();
        // replace the if statement which judge "po_n <= 0"
        int yy = 0;
        for (int i = 0; i < int(pow(2, dim)); ++i) if (bref[i] == 0) yy ++;
        if (yy == int(pow(2, dim))) break;
    }
    time(&endTime);
    rtn_save(sboxM, rtn, difftime(endTime, startTime), "greedy_sun");
    return rtn;
}

std::vector<Red::ImPoint> Red::ref_impoint_init(SboxM sboxM) {
    std::vector<ImPoint> ref_ans;
    std::vector<std::string> ref_initial = sboxM.get_imposs();
    for (const auto& i : ref_initial){
        ref_ans.push_back(ImPoint(utilities::b_to_d(stol(i))));
    }
    std::cout << "totally " << ref_ans.size() << " impossible points objects" << std::endl;
    return ref_ans;
}

std::vector<std::vector<int>> Red::milp_red(std::vector<Check::Ineq> ineqs, std::vector<ImPoint> ref_ans) {
    std::vector<std::vector<int>> rtn;
    // initial process
    for (auto & ref_an : ref_ans)
        for (int j = 0; j < ineqs.size(); ++j)
            if (!ineqs[j](ref_an.get_d())) {
                ref_an.rmd_add(ineqs[j]);
                ref_an.rmd_indx_add(j);
            }
    // Create an environment
    GRBEnv env = GRBEnv(true);
    env.start();
    GRBModel model = GRBModel(env);
    GRBVar *Elem;
    int vs = ineqs.size();
    // Create variables
    Elem = model.addVars(vs, GRB_BINARY);
    for (int i = 0; i < vs; ++i) {
        std::ostringstream vname;
        vname << "inq_" << i;
        Elem[i].set(GRB_StringAttr_VarName, vname.str());
    }

    for (auto & ref_an : ref_ans) {
        GRBLinExpr cons = 0;
        for (int j = 0; j < ref_an.get_rmd_idx().size(); ++j)
            cons += Elem[ref_an.get_rmd_idx()[j]];
        model.addConstr(cons >= 1);
    }

    GRBLinExpr obj = 0;
    for (int i = 0; i < vs; ++i)
        obj += Elem[i];
    model.setObjective(obj, GRB_MINIMIZE);
    model.optimize();

    for (int i = 0; i < vs; ++i)
        if (Elem[i].get(GRB_DoubleAttr_X) == 1)
            rtn.push_back(ineqs[std::stoi((Elem[i].get(GRB_StringAttr_VarName)).substr(4))].get_ineq());
    return rtn;
}

std::vector<std::vector<int>> Red::sub_milp(SboxM sboxM, std::vector<std::vector<int>> ineq_int) {
    std::vector<Check::Ineq> ineqs = Check::ineq_obt(ineq_int);
    std::vector<ImPoint> ref_ans = ref_impoint_init(sboxM);
    time_t startTime, endTime;
    time(&startTime);
    std::vector<std::vector<int>> rtn = milp_red(ineqs, ref_ans);
    time(&endTime);
    rtn_save(sboxM, rtn, difftime(endTime, startTime), "sub_milp");
    return rtn;
}

std::vector<std::vector<int>> Red::convex_hull_tech(SboxM sboxM, std::vector<std::vector<int>> ineq_int) {
    std::vector<Check::Ineq> ineqs = Check::ineq_obt(ineq_int);
    std::vector<ImPoint> ref_ans = ref_impoint_init(sboxM);
    std::vector<int> ref_ans_int = Check::ref_imposs_init(sboxM);
    time_t startTime, endTime;
    time(&startTime);

    std::set<std::vector<int>> mvd_pos;
    for (auto & ineq : ineqs) {
        for (int j : ref_ans_int)
            if (!ineq(j)) ineq.add_mv_point(j);
        if (mvd_pos.find(ineq.get_mv_points()) == mvd_pos.end()) mvd_pos.insert(ineq.get_mv_points());
    }
    std::cout << "Start traversing nodes in convex hull algorithm : " << std::endl;
    // k = 2
    std::vector<Check::Ineq> n_ineqs;
    for (int l = 0; l < ref_ans.size(); ++l) {
        for (int i = 0; i < ineqs.size(); ++i) {
            for (int j = 0; j < ineqs.size(); ++j) {
                if (ineqs[i](ref_ans[l].get_d()) && ineqs[j](ref_ans[l].get_d()) && !ineqs[i].equal_to(ineqs[j])){
                    // calculate the new ineq
                    std::vector<int> t1 = ineqs[i].get_cos();
                    std::vector<int> t2 = ineqs[j].get_cos();
                    std::vector<int> ma;
                    for (int m = 0; m < t1.size(); ++m)
                        ma.push_back(t1[m] + t2[m]);
                    int mb = ineqs[i].get_nco() + ineqs[j].get_nco();
                    Check::Ineq ti = Check::Ineq(ma, mb);

                    for (int m : ref_ans_int)
                        if (!ti(m)) ti.add_mv_point(m);

                    if (mvd_pos.find(ti.get_mv_points()) == mvd_pos.end()) {
                        mvd_pos.insert(ti.get_mv_points());
                        n_ineqs.push_back(ti);
                    }
                }
            }
        }
        std::cout << l << " points" << std::endl;
    }
    for (auto & n_ineq : n_ineqs) ineqs.push_back(n_ineq);

    std::vector<std::vector<int>> rtn = milp_red(ineqs, ref_ans);
    time(&endTime);
    rtn_save(sboxM, rtn, difftime(endTime, startTime), "convex_hull");
    return rtn;
}

std::vector<int> Red::supp(std::string a) {
    std::vector<int> rtn;
    for (int i = 0; i < a.size(); ++i) if (a[i] == '1') rtn.push_back(i);
    return rtn;
}

bool Red::supp_intersec(std::vector<int> a, std::vector<int> b) {
    if (a.size() <= b.size()){
        for (int & i : a)
            if (std::find(b.begin(), b.end(), i) != b.end()) return false;
        return true;
    } else {
        for (int & i : b)
            if (std::find(a.begin(), a.end(), i) != a.end()) return false;
        return true;
    }
}

std::vector<int> Red::prec_to_ineq(SboxM sboxM, Red::IcPrec ic) {
    std::vector<int> supp_a = supp(ic.get_a());
    std::vector<int> supp_u = supp(ic.get_u());
    std::vector<int> ii;
    int dim = sboxM.get_dim() - 1;
    for (int i = 0; i < dim; ++i) ii.push_back(i);
    for (int & i : supp_a) ii.erase(std::find(ii.begin(), ii.end(), i));
    for (int & i : supp_u) ii.erase(std::find(ii.begin(), ii.end(), i));

    std::vector<int> cos;
    for (int i = 0; i < dim; ++i) {
        auto it_a = std::find(supp_a.begin(), supp_a.end(), i);
        auto it_ii = std::find(ii.begin(), ii.end(), i);
        if (it_a != supp_a.end() && it_ii == ii.end()) cos.push_back(-1);
        else if (it_a == supp_a.end() && it_ii != ii.end()) cos.push_back(1);
        else cos.push_back(0);
    }
    cos.push_back(utilities::weight(ic.get_a()) - 1);
    return cos;
}

std::vector<std::string> Red::ck_po(SboxM sboxM, std::vector<int> ineq) {
    std::vector<std::string> pos = sboxM.get_poss();
    int b = ineq[ineq.size() - 1];
    ineq.pop_back();
    Check::Ineq t = Check::Ineq(ineq, b);
    std::vector<std::string> pos_rtn;
    for (auto & po : pos)
        if (!t(utilities::b_to_d(std::stoll(po)))) pos_rtn.push_back(po);
    return pos_rtn;
}

std::vector<std::string> Red::obt_impo(SboxM sboxM, std::vector<int> ineq) {
    std::vector<std::string> impos = sboxM.get_imposs();
    int b = ineq[ineq.size() - 1];
    ineq.pop_back();
    Check::Ineq t = Check::Ineq(ineq, b);
    std::vector<std::string> impos_rtn;
    for (auto & impo : impos)
        if (!t(utilities::b_to_d(std::stoll(impo)))) impos_rtn.push_back(impo);
    return impos_rtn;
}

std::vector<std::vector<int>> Red::logic_cond(SboxM sboxM, bool ifComb233) {
    std::vector<std::string> ref_ans = sboxM.get_imposs();
    std::vector<IcPrec> ff;
    int dim = sboxM.get_dim();
    time_t startTime, endTime;
    time(&startTime);

    // for all a in P : refans[i] -> a
    for (int i = 0; i < ref_ans.size(); ++i) {
        // s_interesting
        std::vector<IcPrec> is;
        std::vector<std::vector<IcPrec>> ws;
        std::vector<std::vector<std::string>> wu;

        for (int j = 0; j < dim; ++j) {
            // initial si and ui
            std::vector<IcPrec> ti;
            ws.push_back(ti);
            std::vector<std::string> ts;
            wu.push_back(ts);
        }

        // fir all p in P
        for (int j = 0; j < ref_ans.size(); ++j) {
            std::string u = utilities::po_xor(ref_ans[i], ref_ans[j]);
            if (supp_intersec(supp(ref_ans[i]), supp(u))){
                IcPrec t = IcPrec(ref_ans[i], u);
                std::vector<int> tq = prec_to_ineq(sboxM, t);
                std::vector<std::string> pos_rtn = ck_po(sboxM, tq);
                if (pos_rtn.empty()) wu[utilities::weight(u)].push_back(u);
            }
        }

        if (wu[1].empty())
            for (auto & j : wu[0]) {
                IcPrec t = IcPrec(ref_ans[i], j);
                is.push_back(t);
            }
        else
            for (auto & j : wu[1]) {
                IcPrec t = IcPrec(ref_ans[i], j);
                is.push_back(t);
            }

        for (auto & j : wu[0]) {
            IcPrec t = IcPrec(ref_ans[i], j);
            ws[0].push_back(t);
        }
        for (auto & j : wu[1]) {
            IcPrec t = IcPrec(ref_ans[i], j);
            ws[1].push_back(t);
        }

        // k[2, m]
        for (int j = 2; j < dim; ++j) {
            // for all uk[i]
            for (int k = 0; k < wu[j].size(); ++k) {
                for (int l = 0; l < wu[j - 1].size(); ++l) {
                    IcPrec t = IcPrec(ref_ans[i], wu[j - 1][l]);
                    auto it = std::find(ws[j - 1].begin(), ws[j - 1].end(), t);
                    if (it == ws[j - 1].end()) break;
                }

                IcPrec t3 = IcPrec(ref_ans[i], wu[j][k]);
                ws[j].push_back(t3);
                std::vector<int> tq3 = prec_to_ineq(sboxM, t3);
                std::vector<std::string> impos_rtn3 = obt_impo(sboxM, tq3);

                for (auto & l : wu[j - 1]) {
                    IcPrec t4 = IcPrec(ref_ans[i], l);
                    std::vector<int> tq4 = prec_to_ineq(sboxM, t4);
                    std::vector<std::string> impos_rtn4 = obt_impo(sboxM, tq4);
                    auto it = std::find(is.begin(), is.end(), t4);
                    if (it != is.end() && utilities::sub_prec_ck(impos_rtn3, impos_rtn4)) is.erase(it);
                }
            }
            is.insert(is.end(), ws[j].begin(), ws[j].end());
        }
        ff.insert(ff.end(), is.begin(), is.end());
        std::cout << i << std::endl;
    }
    std::vector<std::vector<int>> aprec_rtn;
    for (auto & i : ff) aprec_rtn.push_back(prec_to_ineq(sboxM, i));
    std::cout << "aprec find over " << std::endl;

    if (!ifComb233) {
        std::vector<Check::Ineq> ineqs = Check::ineq_obt(aprec_rtn);
        std::vector<ImPoint> ref_ans_impoint = ref_impoint_init(sboxM);
        std::vector<std::vector<int>> rtn = milp_red(ineqs, ref_ans_impoint);
        time(&endTime);
        rtn_save(sboxM, rtn, difftime(endTime, startTime), "logic_cond");
        return rtn;
    } else return aprec_rtn;
}

int Red::hamming_distance(std::string a, std::string b) {
    int sum = 0;
    if (a.size() == b.size()){
        for (int i = 0; i < a.size(); i++) if (a[i] != b[i]) sum ++;
        return sum;
    }
    else return -1;
}

std::vector<std::string> Red::ball_points(SboxM sboxM, int d, std::string c) {
    std::vector<std::string> rtn, ref_ans = sboxM.get_poss();
    for (const auto& tmp : ref_ans) {
        if (hamming_distance(tmp, c) <= d && hamming_distance(tmp, c) != -1)
            rtn.push_back(tmp);
    }
    return rtn;
}

std::vector<int> Red::ball_ineq(SboxM sboxM, std::string c, int d, std::vector<std::string> bq) {
    std::vector<int> rtn;
    std::bitset<SBOX_LENGTH> it = 0;

    std::string q = it.to_string().substr(0, sboxM.get_dim() - 1);
    for (auto & i : bq)
        for (int j = 0; j < i.size(); ++j) {
            std::string tb_c = utilities::po_xor(i, c);
            if (tb_c[j] == '1' && q[j] != '1') q.replace(j, 1, "1");
        }
    int ct = 0;
    for (int i = 0; i < c.size(); ++i) {
        if ((c[i] - 48) == 0){
            if (q[i] - 48 == 1)
                rtn.emplace_back(2);
            else
                rtn.emplace_back(1);
        } else if ((c[i] - 48) == 1){
            if (q[i] - 48 == 1){
                rtn.emplace_back(-2);
                ct = ct + 2;
            } else {
                rtn.emplace_back(-1);
                ct ++;
            }
        } else{
            rtn.emplace_back(0);
            continue;
        }
    }
    rtn.emplace_back(ct - 1 - d);
    return rtn;
}

std::vector<std::vector<int>> Red::distorted_balls(SboxM sboxM, bool ifComb233) {
    std::vector<std::vector<int>> ball_rtn;
    std::vector<std::string> impos = sboxM.get_imposs();
    time_t startTime, endTime;
    time(&startTime);

    // a <- impos[i]; b <- impos[j]; c <- impos[k]
    for (int i = 0; i < impos.size(); ++i) {
        for (int j = 0; j < impos.size(); ++j) {
            for (int k = 0; k < impos.size(); ++k) {
                if (utilities::weight(utilities::po_xor(impos[i], impos[j])) == 1 && utilities::weight(utilities::po_xor(impos[i], impos[k])) == 1 && impos[j] != impos[k]){
                    std::string a = utilities::po_xor(utilities::po_xor(impos[i], impos[j]),  impos[k]);
                    auto it = std::find(impos.begin(), impos.end(), a);
                    if (it != impos.end()) {
                        std::vector<std::string> pa = ball_points(sboxM, 1, impos[i]);
                        pa.push_back(impos[k]);
                        std::vector<std::string> ra = pa;
                        pa.erase(pa.end());
                        std::vector<std::string> pb = ball_points(sboxM, 1, impos[j]);
                        std::vector<std::string> rb = pb;
                        std::vector<std::string> rc = ball_points(sboxM, 1, impos[k]);

                        for (auto & l : pa) {
                            std::string ab = utilities::po_xor(utilities::po_xor(l, impos[i]), impos[j]);
                            rb.push_back(ab);
                            std::string ac = utilities::po_xor(utilities::po_xor(l, impos[i]), impos[k]);
                            rc.push_back(ac);
                        }

                        for (auto & l : pb) {
                            std::string tt = utilities::po_xor(utilities::po_xor(l, impos[j]), impos[k]);
                            rc.push_back(tt);
                        }

                        std::vector<int> ta = ball_ineq(sboxM, impos[i], 1, ra), tb = ball_ineq(sboxM, impos[j], 1, rb), tc = ball_ineq(sboxM, impos[k], 1, rc);
                        std::vector<int> cn;
                        for (int l = 0; l < ta.size(); ++l) {
                            if (l == ta.size() - 1) {
                                cn.push_back(ta[l] + tb[l] + tc[l] - 2);
                            } else {
                                cn.push_back(ta[l] + tb[l] + tc[l]);
                            }
                        }
                        ball_rtn.push_back(cn);
                    }
                }
            }
        }
    }

    if (!ifComb233) {
        std::vector<Check::Ineq> ineqs = Check::ineq_obt(ball_rtn);
        std::vector<ImPoint> ref_ans_impoint = ref_impoint_init(sboxM);
        std::vector<std::vector<int>> rtn = milp_red(ineqs, ref_ans_impoint);
        time(&endTime);
        rtn_save(sboxM, rtn, difftime(endTime, startTime), "distorted_balls");
        return rtn;
    } else return ball_rtn;
}

std::vector<std::vector<int>> Red::comb233(SboxM sboxM) {
    time_t startTime, endTime;
    time(&startTime);
    std::vector<std::vector<int>> lc = logic_cond(sboxM, true);
    std::vector<std::vector<int>> db = distorted_balls(sboxM, true);
    lc.insert(lc.begin(), db.begin(), db.end());
    std::vector<Check::Ineq> ineqs = Check::ineq_obt(lc);
    std::vector<ImPoint> ref_ans_impoint = ref_impoint_init(sboxM);
    std::vector<std::vector<int>> rtn = milp_red(ineqs, ref_ans_impoint);
    time(&endTime);
    rtn_save(sboxM, rtn, difftime(endTime, startTime), "comb233");
    return rtn;
}

std::vector<std::vector<int>> Red::reduction(int chooser, SboxM sboxM) {
    std::vector<std::vector<int>> ineq_int = sboxM.get_sage_ineqs(), rtn;
    switch (chooser) {
        case 1:
            rtn = greedy_sun(sboxM, ineq_int);
            Check::check(sboxM, rtn);
            break;
        case 2:
            rtn = sub_milp(sboxM, ineq_int);
            Check::check(sboxM, rtn);
            break;
        case 3:
            rtn = convex_hull_tech(sboxM, ineq_int);
            Check::check(sboxM, rtn);
            break;
        case 4:
            rtn = logic_cond(sboxM, false);
            break;
        case 5:
            rtn = comb233(sboxM);
            break;
        case 6: {
            // superball
            time_t startTime, endTime;
            time(&startTime);
            superballMGR::superballGS(sboxM);
            std::string path = std::string(DPATH) + "/sbox/" + sboxM.get_mode() + "/superball/" +
                    std::to_string(sboxM.get_sbox_len()) + "bits/" + sboxM.get_name() + ".txt";
            std::ifstream file;
            file.open(path);
            std::string model, line;
            while (getline(file, line)) {
                std::vector<std::string> coffS = utilities::split(line, " ");
                std::vector<int> coff;
                // 因为superball的结果是按照逆序存储的，所以这里要特殊处理
                for (int i = coffS.size() - 2; i >= 0; --i) {
                    coff.push_back(std::stoi(coffS[i]));
                }
                coff.push_back(std::stoi(coffS[coffS.size() - 1]));
                rtn.push_back(coff);
            }
            file.close();
            time(&endTime);
            auto diff = difftime(endTime, startTime);
            std::cout << "superball time : " << diff << std::endl;
            Check::check(sboxM, rtn);
            break;
        }
        // 这里最后应该是读取外部结果的不等式组，可以是cnf，也可以是 Udovenko 的方法
        case 7: {
            std::string path = std::string(DPATH) + "/sbox/" + sboxM.get_mode() + "/external/" +
                               std::to_string(sboxM.get_sbox_len()) + "bits/" + sboxM.get_name() + ".txt";
            std::ifstream file;
            file.open(path);
            std::string model, line;
            while (getline(file, line)) {
                std::vector<std::string> coffS = utilities::split(line, " ");
                std::vector<int> coff;
                for (auto & j : coffS) {
                    coff.push_back(std::stoi(j));
                }
                rtn.push_back(coff);
            }
            file.close();
            Check::check(sboxM, rtn);
            break;
        }
        default:
            rtn = greedy_sun(sboxM, ineq_int);
            break;
    }
    return rtn;
}

int Red::branch_num_of_sbox(std::vector<int> sbox) {
    int e = 100;
    for (int i = 0; i < sbox.size(); ++i) {
        for (int j = 0; j < sbox.size(); ++j) {
            int a = i ^ j;
            int b = sbox[i] ^ sbox[j];
            int c = utilities::weight(std::to_string(utilities::d_to_b(a)));
            int d = utilities::weight(std::to_string(utilities::d_to_b(b)));
            int f = c + d;
            if (e >= f && i != j){
                e = f;
            }
        }
    }
    return e;
}

// It seems that the branch number cannot be solved with MILP ??
// calculate branch number of linear permutation
int Red::branch_num_of_matrix(std::vector<std::vector<int>> m, std::vector<std::vector<int>> ffm) {
    int inputSize = m.size();
    // Create an environment
    GRBEnv env = GRBEnv(true);
    env.start();
    GRBModel model = GRBModel(env);
    GRBVar *InputI, *OutputI;
    InputI = model.addVars(inputSize, GRB_INTEGER);
    // 确定每个变量的上下界
    GRBLinExpr initial = 0;
    for (int i = 0; i < inputSize; ++i) {
        GRBLinExpr bound = 0;
        bound = InputI[i];
        model.addConstr(bound <= ffm.size());
        model.addConstr(bound >= 0);
        initial += InputI[i];
    }
    model.addConstr(initial >= 1);
    OutputI = model.addVars(inputSize, GRB_INTEGER);
    for (int i = 0; i < inputSize; ++i) {
        GRBLinExpr consL = 0, consR = 0;
        consL += OutputI[i];
        for (int j = 0; j < m[i].size(); ++j) {
            consL += -m[i][j] * InputI[j];
        }
        model.addConstr(consL >= 0);
        model.addConstr(consL <= 0);
    }
    GRBLinExpr obj = 0;
    for (int i = 0; i < inputSize; ++i) {
        obj += OutputI[i];
    }
    model.setObjective(obj, GRB_MINIMIZE);
    model.optimize();

    int optimstatus = model.get(GRB_IntAttr_Status);
    int result;
    // GRB_OPTIMAL
    if (optimstatus == 2) {
        result = model.get(GRB_DoubleAttr_ObjVal);
        std::cout << "all results : " << std::endl;
        GRBVar *a = model.getVars();
        int numvars = model.get(GRB_IntAttr_NumVars);
        for (int i = 0; i < numvars; ++i) {
            std::cout << a[i].get(GRB_StringAttr_VarName) << " = " << a[i].get(GRB_DoubleAttr_X) << "\n";
        }

    }
    else if (optimstatus == 3) {
        result = 25600;
    }
    return result;
}



void Red::sboxModelBench(std::string name, std::vector<int> sbox, std::string target, std::string mode) {
    // SboxM sboxM("Present", {0xC,0x5,0x6,0xB,0x9,0x0,0xA,0xD,0x3,0xE,0xF,0x8,0x4,0x7,0x1,0x2}, "D", "AS");
    SboxM sboxM(std::move(name), std::move(sbox), std::move(mode));
    Check::check(sboxM, sboxM.get_sage_ineqs());
    // test for greedy sun
    // std::vector<std::vector<int>> greedRed = Red::greedy_sun(sboxM, sboxM.get_sage_ineqs());
    // Check::check(sboxM, greedRed);
    // test for subMILP
    // std::vector<std::vector<int>> subRed = Red::sub_milp(sboxM, sboxM.get_sage_ineqs());
    // Check::check(sboxM, subRed);
    // test for convexHull
    //std::vector<std::vector<int>> convexHullRed = Red::convex_hull_tech(sboxM, sboxM.get_sage_ineqs());
    //Check::check(sboxM, convexHullRed);
    // test for logic condition
    std::vector<std::vector<int>> logicCondRed = Red::logic_cond(sboxM, false);
    Check::check(sboxM, logicCondRed);
    // test for distorted_balls
    // std::vector<std::vector<int>> distortedBallsRed = Red::distorted_balls(sboxM, false);
    // Check::check(sboxM, distortedBallsRed);
    // test for comb233
    // std::vector<std::vector<int>> comb233Red = Red::comb233(sboxM);
    // Check::check(sboxM, comb233Red);
    // test for syntax-guided
    // SGsyn sGsyn(300, greedRed);
    // rtn_save(sboxM, sGsyn.getCrrIneq(), sGsyn.getSolveTime(), "syntax-guided");
    // Check::check(sboxM, sGsyn.getCrrIneq());
    // cnf
    // std::vector<std::vector<int>> cnfRed = Red::cnfGen(sboxM);
    // Check::check(sboxM, cnfRed);
}

std::vector<std::vector<int>> Red::cnfGen(SboxM sboxM) {
    time_t startTime, endTime;
    time(&startTime);
    std::string path = sboxM.get_path() + "cnf/" + std::to_string(sboxM.get_sbox_len()) + "bits/" + sboxM.get_name() + ".txt";
    std::ifstream file_extract;
    file_extract.open(path);
    std::stringstream data;
    data << file_extract.rdbuf();
    std::string cnf = data.str();
    std::vector<std::string> cnf_s = utilities::split(cnf, "();");

    std::vector<std::string> cnf_str_save;
    for (auto & cnf_ : cnf_s) {
        if (cnf_[1] == '+' or cnf_[2] == '+'){
            std::string cnf_str;
            for (int j = 0; j < sboxM.get_dim() - 1; ++j) cnf_str = cnf_str + "-";
            std::vector<std::string> tmp_cnf = utilities::split(cnf_, "+");
            for (auto & j : tmp_cnf) {
                if (j.size() == 1) cnf_str.replace((int)j[0] - 65, 1, "0");
                else if (j.size() > 1) cnf_str.replace((int)j[0] - 65, 1, "1");
            }
            cnf_str_save.push_back(cnf_str);
        }
    }

    std::vector<std::vector<int>> rtn;
    for (const auto& str : cnf_str_save) {
        std::vector<int> coeff;
        int const_term = 0;
        for (char j : str){
            if ((j - 48) == 0){
                coeff.emplace_back(1);
            } else if ((j - 48) == 1){
                coeff.emplace_back(-1);
                const_term ++;
            } else{
                coeff.emplace_back(0);
                continue;
            }
        }
        coeff.emplace_back(const_term - 1);
        rtn.push_back(coeff);
    }

    time(&endTime);
    rtn_save(sboxM, rtn, difftime(endTime, startTime), "cnf");
    return rtn;
}

