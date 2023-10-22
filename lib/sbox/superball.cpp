//
// Created by Septi on 2/3/2023.
//

#include "superball.h"

Input::Input(int argc, std::vector<std::string> argv) {

    // points_filename, dim, n_workers, n_threads_gurobi
    if (argc < 3) {
        printf("Please input points_filename, dim, n_workers, n_threads_gurobi, and try agian.\n");
        exit(1);
    }

    this->points_filename = argv[1];
    this->dim = stoi(argv[2]);
    this->n_workers = (argc > 3) ? stoi(argv[3]) : NUM_THREADS_WORKERS;
    this->n_threads_gurobi = (argc > 4) ? stoi(argv[4]) : NUM_THREADS_GUROBI;

    /*{
        wind_t i = 0;
        while (this->points_filename[i] != '.') {
            this->ineqs_filename += this->points_filename[i++];
        }
    }*/
    this->ineqs_filename = argv[6];
    this->ineqs_filename += ALG;
    this->result_filename = argv[6];


    if (strcmp(ALG, "SIZE_then_STRENGTH") == 0) {
        if (argc < 7) {
            printf("Please input points_filename, dim, n_workers, n_threads_gurobi, min_strength, candidate_dir1, ...and try agian.\n");
            exit(1);
        }

        this->min_strength = stoi(argv[5]);
        for (wind_t i = 6; i < argc; i++) {
            this->source_dirs.push_back(argv[i]);
        }
        this->ineqs_filename += "_minstrength" + to_string(this->min_strength);

    } else {
        this->min_strength = -1;
        for (wind_t i = 5; i < argc; i++) {
            this->source_dirs.push_back(argv[i] + ALG);
            system((std::string("mkdir -p ") + argv[i] + ALG).c_str());
        }
    }

    if (this->dim >= sizeof(p_t) * 8) {
        printf("Dimension is too large, %d vs. %ld.\n", this->dim, sizeof(p_t) * 8);
        exit(1);
    }



    printf("\n++++++++++++++++++++++++++++ parameter +++++++++++++++++++++++++++++++++\n\n");

    printf("Algorithm: %s\n", ALG);

    printf("Worker number: %d\n", this->n_workers);
    printf("Gurobi threads: %d\n\n", this->n_threads_gurobi);

    printf("Coefficient limits: %d\n\n", SC_COEFF);

    printf("GENERATE_NUM: %d\n", GENERATE_NUM);
    printf("GENERATE_TIMELIMIT: %d\n", GENERATE_TIMELIMIT);
    printf("GENERATE_DURABLE: %d\n\n", GENERATE_DURABLE);

    printf("Points filename: %s\n", this->points_filename.c_str());
    printf("Dimension: %d\n\n", this->dim);

    printf("Inequality dirname: %s\n\n", this->ineqs_filename.c_str());
    mkdir(this->ineqs_filename.c_str(), S_IRWXU);

    printf("Program: %s\n\n", argv[0].c_str());

    if (this->min_strength > 0) {
        printf("Min_strength: %d\n\n", this->min_strength);
    }

    if (this->source_dirs.size()) {
        printf("Source dirs:\n");
        for (auto st : this->source_dirs) {
            printf("\t%s\n", st.c_str());
        }
        printf("\n\n");
    }

    cout << getCurrentSystemTime() << endl;

    printf("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");

    this->load_points();
    this->calculate_weight_list();
}


void Input::load_points() {

    PRINTF_STAMP("loading points...\n");

    std::string fp = this->points_filename;
    FILE *fin = fopen((fp).c_str(), "r");
    this->inc.clear();
    p_t poi;
//    while (fscanf(fin, "%x", &poi) > 0) {
    while (fscanf(fin, "%x", &poi) > 0) {
        this->inc.push_back(poi);
    }
    fclose(fin);

    PRINTF_STAMP("to include points num: %ld\n", this->inc.size());


    // table
    vector<char> table(1 << this->dim);
    for (auto poi : this->inc) {
        table[poi] = 1;
    }
    this->table = table;


    // exclusive points
    this->exc.clear();
    for (p_t i = 0; i < (1 << this->dim); i++) {
        if (table[i] == 0) {
            this->exc.push_back(i);
        }
    }
    PRINTF_STAMP("to exclude points num: %ld\n", this->exc.size());

    PRINTF_STAMP("points are loaded\n\n");

}


void Input::calculate_weight_list() {

    vector<uint64_t> table(1 << this->dim);
    for (p_t diff = 0; diff < (1 << this->dim); diff++) {
        table[diff] = uint64_t(diff) + (weight(diff) << this->dim);
    }
    sort(table.begin(), table.end());

    const p_t mask = (1 << this->dim) - 1;
    wind_t w = 0;
    points_t temp;
    for (auto poi : table) {
        if ((poi >> this->dim) == w) {
            temp.push_back(mask & poi);
        } else {
            this->weight_list.push_back(temp);
            temp.clear();
            w++;
            temp.push_back(mask & poi);
        }
    }
    assert(w == this->dim);
    this->weight_list.push_back(temp);

    PRINTF_STAMP("weight table is calculated\n\n");

}



// return true if not in.
flag_t Inequality::point_notin_equ_normal(p_t diff) {

    p_t tp = diff;
    wind_t j = 0;
    wind_t dim = this->coeff.size() - 1;
    coeff_t ans = -this->coeff[dim]; // in mormal form

    while (tp) {
        if (tp & 1) {
            ans += this->coeff[j];
        }
        tp >>= 1;
        j++;
    }

    if (ans > 0) {
        return FLAG_STRICT_INC;
    } else if (ans == 0) {
        return FLAG_ON;
    } else {
        return FLAG_EXC;
    }
}



void Inequality::update_inc_exc(Region &reg) {
    assert(this->inc_index.size() == 0);
    assert(this->exc_diff.size() == 0);
    for (wind_t idx = 0; idx < reg.region.size(); idx++) {
        if (this->point_notin_equ_normal(reg.region[idx]) != FLAG_EXC) {
            this->inc_index.push_back(idx);
        } else {
            this->exc_diff.push_back(reg.region[idx]);
        }
    }
}


void Inequality::update_noton_on(Region &reg) {
    assert(this->noton_index.size() == 0);
    assert(this->on_diff.size() == 0);
    for (wind_t idx = 0; idx < reg.border.size(); idx++) {
        if (this->point_notin_equ_normal(reg.border[idx]) != FLAG_ON) {
            this->noton_index.push_back(idx);
        } else {
            this->on_diff.push_back(reg.border[idx]);
        }
    }
}



void Inequality::simplify() {
    assert(this->coeff.size() > 1);

    coeff_t g = this->coeff[0];

    for (wind_t j = 1; j < this->coeff.size(); j++) {
        g = __gcd(g, this->coeff[j]);
    }
    g = g > 0 ? g : -g;
    assert(g > 0);
    if (g == 1) {
        return;
    } else {
        for (wind_t j = 0; j < this->coeff.size(); j++) {
            this->coeff[j] /= g;
        }
    }
}




string Inequality::tostring() {
    string result;
    coeff_t cnst = 0;
    for (wind_t j = 0; j < this->coeff.size() - 1; j++) {
        if (j) {
            result += "+";
        }
        if ((this->center >> j) & 1) {
            result += to_string(-this->coeff[j]);
            cnst += this->coeff[j];
        } else {
            result += to_string(this->coeff[j]);
        }
    }
    result += ">=" + to_string(cnst - this->coeff[this->coeff.size() - 1]);

    return result;
}


wind_t Inequality::zerocoeff() {
    wind_t res = 0;
    for (wind_t j = 0; j < this->coeff.size() - 1; j++) {
        if (this->coeff[j] == 0) {
            res++;
        }
    }
    return res;
}





void Inequality::display() {
    coeff_t cnst = 0;
    for (wind_t j = 0; j < this->coeff.size() - 1; j++) {
        if ((this->center >> j) & 1) {
            printf("%5d, ", -this->coeff[j]);
            cnst += this->coeff[j];
        } else {
            printf("%5d, ", this->coeff[j]);
        }
    }
    printf("%5d [exc %5ld, on %5ld]", cnst - this->coeff[this->coeff.size() - 1], this->exc_diff.size(), this->on_diff.size());
    cout << endl;
}




void Inequality::display_detail() {
    coeff_t cnst = 0;
    for (wind_t j = 0; j < this->coeff.size() - 1; j++) {
        if ((this->center >> j) & 1) {
            printf("%d, ", -this->coeff[j]);
            cnst += this->coeff[j];
        } else {
            printf("%d, ", this->coeff[j]);
        }
    }
    printf("%d [on %ld:", cnst - this->coeff[this->coeff.size() - 1], this->on_diff.size());
    for (auto diff : this->on_diff) {
        printf(" %x", diff);
    }
    printf("], {exc %ld:", this->exc_diff.size());
    for (auto diff : this->exc_diff) {
        printf(" %x", diff);
    }
    printf("}");
    cout << endl;
}

void Inequality::write(FILE *fout) {
    coeff_t cnst = 0;
    for (wind_t j = 0; j < this->coeff.size() - 1; j++) {
        if ((this->center >> j) & 1) {
            fprintf(fout, "%d ", -this->coeff[j]);
            cnst += this->coeff[j];
        } else {
            fprintf(fout, "%d ", this->coeff[j]);
        }
    }
    fprintf(fout, "%d\n", (cnst - this->coeff[this->coeff.size() - 1]));
}

void Inequality::write_detail(FILE *fout) {
    coeff_t cnst = 0;
    for (wind_t j = 0; j < this->coeff.size() - 1; j++) {
        if ((this->center >> j) & 1) {
            fprintf(fout, "%d, ", -this->coeff[j]);
            cnst += this->coeff[j];
        } else {
            fprintf(fout, "%d, ", this->coeff[j]);
        }
    }
    fprintf(fout, "%d::[%ld:", cnst - this->coeff[this->coeff.size() - 1], this->on_diff.size());
    for (auto diff : this->on_diff) {
        fprintf(fout, " %x", diff);
    }
    fprintf(fout, "]:::{%ld:", this->exc_diff.size());
    for (auto diff : this->exc_diff) {
        fprintf(fout, " %x", diff);
    }
    fprintf(fout, "}\n");
}




bool Inequality::read(FILE *fin, p_t center, wind_t dim) {

    assert(this->coeff.size() == 0);
    assert(this->exc_diff.size() == 0);

    this->center = center;

    coeff_t cnst = 0;
    coeff_t c;
    if (fscanf(fin, "%d, ", &c) > 0) {
        if (c < 0) {
            assert(center & p_t(1));
            c = -c;
            cnst += c;
        }
        this->coeff.push_back(c);
    } else {
        return false;
    }

    for (wind_t j = 1; j < dim; j++) {
        if (fscanf(fin, "%d, ", &c) <= 0) {
            exit(1);
        }
        if (c < 0) {
            assert(center & (p_t(1) << j));
            c = -c;
            cnst += c;
        }

        this->coeff.push_back(c);
    }
    wind_t len;
    if (fscanf(fin, "%d::[%d:", &c, &len) <= 0) {
        exit(1);
    }
    c = -c + cnst;
    assert(c > 0);
    this->coeff.push_back(c);

    p_t on;
    for (wind_t i = 0; i < len; i++) {
        if (fscanf(fin, " %x", &on) <= 0) {
            exit(1);
        }
        this->on_diff.push_back(on);
    }

    if (fscanf(fin, "]:::{%d:", &len) <= 0) {
        exit(1);
    }

    p_t ex;
    for (wind_t i = 0; i < len - 1; i++) {
        if (fscanf(fin, " %x", &ex) <= 0) {
            exit(1);
        }
        this->exc_diff.push_back(ex);
    }
    if (fscanf(fin, " %x}\n", &ex) <= 0) {
        exit(1);
    }
    this->exc_diff.push_back(ex);

    return true;
}



void Inequality::clear() {
    this->coeff.clear();

    this->noton_index.clear();
    this->on_diff.clear();

    this->inc_index.clear();
    this->exc_diff.clear();
}




Dyn_task::Dyn_task(Input &in) {

    for (wind_t idx = 0; idx < in.exc.size(); idx++) {

        Task cur_task;

        cur_task.type = TASK_TYPE_POINT;
        cur_task.center = in.exc[idx];

        cur_task.task_id = idx;
        cur_task.thread_idx = -1;

        this->task_array.push_back(cur_task);
    }

    this->n_done = 0;
    this->working_threads = in.n_workers;

    PRINTF_STAMP("initialized %ld tasks, type: TASK_TYPE_POINT\n\n", this->task_array.size());
}





void Region::generate(Input *p_in) {

    for (auto poi : p_in->exc) {
        this->diff_to_type[this->center ^ poi] = REGION_EXC_CANNOT;
    }

    // deal with weight, can break is no candidates.
    this->max_dist = 0;
    for (wind_t w = 0; w <= p_in->dim; w++) {
        bool has_outter = false;

        for (p_t diff : p_in->weight_list[w]) {
            points_t subsetpoints;
            wind_t j = 0;
            p_t td = diff;
            while (td) {
                if (td & p_t(1)) {
                    subsetpoints.push_back(diff ^ (p_t(1) << j));
                }
                td >>= 1;
                j++;
            }

            if (this->diff_to_type[diff] == REGION_INC) {
                this->diff_to_type[diff] = REGION_INC_MINIBORDER;
                for (auto subspoi : subsetpoints) {
                    if (this->diff_to_type[subspoi] <= REGION_EXC_CANNOT) {
                        this->diff_to_type[diff] = REGION_INC;
                        break;
                    }
                }

                if (this->diff_to_type[diff] == REGION_INC) {
                    for (auto subspoi : subsetpoints) {
                        if (this->diff_to_type[subspoi] > REGION_EXC_CANNOT) {
                            this->diff_to_type[diff] = REGION_INC_BORDER;
                            break;
                        }
                    }
                }
            } else {
                assert(this->diff_to_type[diff] == REGION_EXC_CANNOT);
                this->diff_to_type[diff] = REGION_EXC_OUTTER;
                for (auto subspoi : subsetpoints) {
                    if (this->diff_to_type[subspoi] <= REGION_EXC_CANNOT) {
                        this->diff_to_type[diff] = REGION_EXC_CANNOT;
                        break;
                    }
                }

                if (this->diff_to_type[diff] == REGION_EXC_OUTTER) {
                    has_outter = true;
                    for (auto subspoi : subsetpoints) {
                        assert(this->diff_to_type[subspoi] >= REGION_EXC_INNER);
                        this->diff_to_type[subspoi] = REGION_EXC_INNER;
                    }
                }
            }
        }

        if (!has_outter) {
            this->max_dist = w - 1;
            assert(this->max_dist >= 0);
            break;
        }
    }


    // update border and region
    this->miniborder.clear();
    this->region.clear();
    this->outter.clear();
    for (wind_t w = this->max_dist + 1; w >= 0; w--) {
        for (p_t diff : p_in->weight_list[w]) {
            switch (this->diff_to_type[diff]) {
                case REGION_INC_BORDER: {
                    this->border.push_back(diff);
                    break;
                }
                case REGION_INC_MINIBORDER: {
                    this->border.push_back(diff);
                    this->miniborder.push_back(diff);
                    break;
                }
                case REGION_EXC_INNER: {
                    this->region.push_back(diff);
                    break;
                }
                case REGION_EXC_OUTTER: {
                    this->region.push_back(diff);
                    this->outter.push_back(diff);
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }
}






Pool::Pool(Input &in) {

    this->candidate_set.clear();
    char filename[255];
    assert(in.dim > 0);

    wind_t missing = 0;
    wind_t count = 0;
    wind_t ineq_count = 0;

    unordered_map<string, Inequality> umap;

    for (auto dir_name : in.source_dirs) {
        for (auto center : in.exc) {
            sprintf(filename, "%s/%d.txt", dir_name.c_str(), center);
            FILE *fin = fopen(filename, "r");
            if (!fin) {
                missing++;
                count++;
                continue;
            }

            if (cout && count % 10000 == 0) {
                PRINTF_STAMP("read %d / %ld files...\n", count, in.exc.size());
            }
            count++;

            while (true) {
                Inequality ineq;
                if (!ineq.read(fin, center, in.dim)) {
                    break;
                }
                ineq.simplify();
                umap[ineq.tostring()] = ineq;
                ineq_count++;
            }
            fclose(fin);
        }
    }


    for (auto it : umap) {
        this->candidate_set.push_back(it.second);
    }


    this->cover_borders = 0;

    PRINTF_STAMP("Loaded %d candidate inequalities from %ld centers, simplified to %ld, the other %d are missing\n\n", ineq_count, in.exc.size() - missing, this->candidate_set.size(), missing);
}






void Pool::display() {

    printf("#inequalities: %ld, cover borders: %d\n", this->selection.size(), this->cover_borders);
    printf("Selection: \n");
    for (wind_t i = 0; i < this->sel_index.size(); i++) {
        printf("eq%4d: ", this->sel_index[i]);
        this->selection[i].display();
    }
    cout << endl;
}


void Pool::write(Input &in) {
    char filename[255];
    sprintf(filename, "%s",
            (in.result_filename.replace(in.result_filename.size() - 1, 1, "") + ".txt").c_str());

    FILE *fout = fopen(filename, "w");

    /*fprintf(fout, "%ld", this->sel_index.size());
    for (auto idx : this->sel_index) {
        fprintf(fout, " %d", idx);
    }
    fprintf(fout, "\n\n")*/;

    for (auto ineq : this->selection) {
        ineq.write(fout);
    }
    fclose(fout);
}



void Pool::clear() {

    this->cover_borders = 0;

    this->sel_index.clear();
    this->selection.clear();

}





// compute an inequality, i.e. excludes points as many as possible.
flag_t Task::generate_inequality(Input *p_in) {

    assert(this->first.coeff.size() == 0);
    GRBEnv env = GRBEnv();
    if (p_in->n_workers != 1) {
        env.set(GRB_IntParam_LogToConsole, 0);
    }
    env.set(GRB_IntParam_Threads, p_in->n_threads_gurobi);
    env.set(GRB_IntParam_MIPFocus, 3);
    GRBModel model = GRBModel(env);

    // variables
    wind_t coeff_limit = wind_t((SC_COEFF - 100) / (this->reg.max_dist + 1));
    wind_t constant_limit = coeff_limit * (this->reg.max_dist + 1);

    const wind_t dim = p_in->dim;
    vector<GRBVar> z(dim + 1);
    for (wind_t j = 0; j < dim; j++) {
        z[j] = model.addVar(0, coeff_limit, 0, GRB_INTEGER);
    }
    z[dim] = model.addVar(1, constant_limit, 0, GRB_INTEGER);


#ifdef REGION_then_BORDER

    const wind_t weight_region = this->reg.border.size() + 1;
    const wind_t weight_border = 1;

#endif

#ifdef BORDER_then_REGION

    const wind_t weight_region = 1;
    const wind_t weight_border = this->reg.region.size() + 1;

#endif

#ifdef WEIGHTED_ONE

    const wind_t weight_region = 1;
    const wind_t weight_border = 1;

#endif

    // build model
    // Constraint 1: border
    GRBLinExpr obj = 0;
    vector<GRBVar> nb;
    for (auto diff : this->reg.border) {
        INEQ_L(diff, z, dim)
        model.addConstr(l >= 0);

        GRBVar b = model.addVar(0, 1, 0, GRB_BINARY);
        nb.push_back(b);
        l += -SC_COEFF * (1 - b);
        model.addConstr(l <= 0);
        obj += b * weight_border;
    }

    // Constraint 2: region points
    vector<GRBVar> rg;
    for (wind_t r_i = 0; r_i < this->reg.region.size(); r_i++) {
        p_t diff = this->reg.region[r_i];
        INEQ_L(diff, z, dim)

        GRBVar b = model.addVar(0, 1, 0, GRB_BINARY);
        rg.push_back(b);
        l += -SC_COEFF * (1 - b);
        model.addConstr(l <= -1);
        obj += b * weight_region;
    }

    // Constraint 3: exclude previous computed inequalities.
    for (auto ineq : this->ineq_set) {
        if (ineq.noton_index.size()) {
            GRBLinExpr exclude = 0;
            for (auto idx : ineq.noton_index) {
                exclude += nb[idx];
            }
            model.addConstr(exclude >= 1);
        }
    }
    for (auto ineq : this->ineq_set) {
        if (ineq.inc_index.size()) {
            GRBLinExpr exclude = 0;
            for (auto idx : ineq.inc_index) {
                exclude += rg[idx];
            }
            model.addConstr(exclude >= 1);
        }
    }


    model.setObjective(obj, GRB_MAXIMIZE);

    double this_time = 0;
    for (wind_t durable = 0; durable < GENERATE_DURABLE; durable++) {
        model.set(GRB_DoubleParam_TimeLimit, GENERATE_TIMELIMIT);
        model.optimize();
        this_time += model.get(GRB_DoubleAttr_Runtime);
        if (model.get(GRB_IntAttr_SolCount) || model.get(GRB_IntAttr_Status) == GRB_INFEASIBLE) {
            break;
        }
    }
    this->time += this_time;

    if (model.get(GRB_IntAttr_SolCount) == 0) {
        return MODEL_INFESIBLE;
    }


    flag_t result = (model.get(GRB_IntAttr_Status) == GRB_TIME_LIMIT) ? MODEL_TIMELIMIT : MODEL_OPTIMAL;

    model.set(GRB_IntParam_SolutionNumber, 0);

    wind_t most_on = 0;
    for (auto b : nb) {
        if (round(b.get(GRB_DoubleAttr_Xn)) == 1) {
            most_on++;
        }
    }
    wind_t best_weight = 0;
    for (auto b : rg) {
        if (round(b.get(GRB_DoubleAttr_Xn))) {
            best_weight++;
        }
    }

    this->first.center = this->center;
    for (wind_t j = 0; j < dim + 1; j++) {
        this->first.coeff.push_back(round(z[j].get(GRB_DoubleAttr_Xn)));
    }
    this->first.update_inc_exc(this->reg);
    this->first.update_noton_on(this->reg);

    assert(this->first.exc_diff.size() >= best_weight);
    assert(this->first.on_diff.size() >= most_on);


    if (this->first.inc_index.size() == 0 && this->first.noton_index.size() == 0) {
        result = MODEL_INFESIBLE;
    }

    this->ineq_set.push_back(this->first);
    this->first.clear();

    return result;
}

void Task::superball_type_point(Input *p_in, Dyn_task *dyn) {

    // step 0: check prevous files
    char filename[255];
    sprintf(filename, "%s/%d.txt", p_in->ineqs_filename.c_str(), this->center);
    FILE *test = fopen(filename, "r");
    if (test) {
        PRINTF_STAMP("file '%s' exists, no needs of re-computation\n", filename);
        fclose(test);
        return;
    }

    // step 1: build diff_to_type table.
    this->reg.center = this->center;
    vector<flag_t> diff_to_type(1 << p_in->dim);
    this->reg.diff_to_type = diff_to_type;
    this->time = 0;

    // step 2: generate region.
    this->reg.generate(p_in);

    // step 3: compute GENERATE_NUM inequality
    for (wind_t i = 0; i < GENERATE_NUM; i++) {
        // compute an inequality, the inequality is stored in this->first.
        if (this->generate_inequality(p_in) == MODEL_INFESIBLE) {
            break;
        }
    }

    // step 6: output the inequalities in this->ineq_set to files
    FILE *fout = fopen(filename, "w");
    for (auto ineq : this->ineq_set) {
        ineq.write_detail(fout);
    }
    fclose(fout);


    cout << endl;
    PRINTF_STAMP("[%d]: center %d [%x] is DONE\n", \
        this->thread_idx, this->center, this->center);

    printf("\t\t#Region %3s %ld\n", ":", this->reg.region.size());
    printf("\t\t#Outter %3s %ld\n", ":", this->reg.outter.size());
    printf("\t\t#Border %3s %ld\n", ":", this->reg.border.size());
    printf("\t\t#mBorder %2s %ld\n", ":", this->reg.miniborder.size());
    printf("\t\tMax-dist %2s %d\n", ":", this->reg.max_dist);
    for (wind_t i = 0; i < this->ineq_set.size(); i++) {
        printf("\t\t%2d-th %5s exc %ld/%ld, on %ld/%ld\n", i, ":", \
            this->ineq_set[i].exc_diff.size(), this->reg.region.size(), \
            this->ineq_set[i].on_diff.size(), this->reg.border.size());
    }
    printf("\t\tTime %6s %5.3f\n\n", ":", this->time);

}








void Pool::find_min_size(Input &in) {

    GRBEnv env = GRBEnv();
    env.set(GRB_IntParam_Threads, in.n_threads_gurobi);
    env.set(GRB_IntParam_MIPFocus, 3);
    GRBModel model = GRBModel(env);


    PRINTF_STAMP("Start building model for %ld candidates\n\n", this->candidate_set.size());


    // all contained points
    vector<GRBVar> ineq_var;
    vector<GRBVar> on_var;

    GRBLinExpr obj = 0;
    for (auto ineq : this->candidate_set) {
        GRBVar v = model.addVar(0, 1, 0, GRB_BINARY);
        ineq_var.push_back(v);
        obj += v * (in.inc.size() + 1);
    }

    vector<GRBLinExpr> exc_point_expr(1 << in.dim);
    vector<GRBLinExpr> on_point_expr(1 << in.dim);
    vector<bool> on_point_flag(1 << in.dim);

    for (auto ic : in.inc) {
        GRBVar v = model.addVar(0, 1, 0, GRB_BINARY);
        on_var.push_back(v);
        obj -= v;
    }

    for (wind_t inq_i = 0; inq_i < this->candidate_set.size(); inq_i++) {
        if (inq_i && inq_i % 10000 == 0) {
            PRINTF_STAMP("%d inequalities are done\n", inq_i);
        }
        for (auto diff : this->candidate_set[inq_i].exc_diff) {
            exc_point_expr[diff ^ this->candidate_set[inq_i].center] += ineq_var[inq_i];
        }
        for (auto diff : this->candidate_set[inq_i].on_diff) {
            on_point_expr[diff ^ this->candidate_set[inq_i].center] += ineq_var[inq_i];
            on_point_flag[diff ^ this->candidate_set[inq_i].center] = true;
        }
    }

    for (auto ex : in.exc) {
        model.addConstr(exc_point_expr[ex] >= 1);
    }

    for (wind_t i = 0; i < in.inc.size(); i++) {
        if (on_point_flag[in.inc[i]]) {
            model.addConstr(on_point_expr[in.inc[i]] >= in.min_strength);
            model.addConstr(on_point_expr[in.inc[i]] >= on_var[i]);
        } else {
            model.addConstr(on_var[i] == 0);
        }
    }


    if (this->sel_index.size() > 0) {
        model.addConstr(obj <= this->sel_index.size() - 1);
    }


    PRINTF_STAMP("Building is done\n\n");


    // start optimization
    model.setObjective(obj, GRB_MINIMIZE);

    model.optimize();
    this->time += model.get(GRB_DoubleAttr_Runtime);

    if (model.get(GRB_IntAttr_Status) == GRB_INFEASIBLE) {
        return;
    }

    assert(model.get(GRB_IntAttr_SolCount) > 0);
    model.set(GRB_IntParam_SolutionNumber, 0);

    this->clear();
    on_point_flag.clear();
    for (wind_t i = 0; i < ineq_var.size(); i++) {
        if (round(ineq_var[i].get(GRB_DoubleAttr_Xn)) == 1) {
            this->sel_index.push_back(i);
            this->selection.push_back(this->candidate_set[i]);

            for (auto diff : this->candidate_set[i].on_diff) {
                on_point_flag[diff ^ this->candidate_set[i].center] = true;
            }
        }
    }
    for (auto ic : in.inc) {
        if (on_point_flag[ic]) {
            this->cover_borders++;
        }
    }

    this->write(in);

    PRINTF_STAMP("best result %ld, cover borders %d\n\n", this->sel_index.size(), this->cover_borders);

}
