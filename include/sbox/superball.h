//
// Created by Septi on 2/3/2023.
//

#ifndef EASYBC_SUPERBALL_H
#define EASYBC_SUPERBALL_H


#include <limits.h>
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <bitset>
#include <vector>
#include <map>
#include <unordered_map>
#include <cmath>
#include <fstream>
#include <chrono>
#include <string.h>
#include <cstdlib>
#include "sys/time.h"


// pthread
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// gubori
#include "gurobi_c++.h"

using namespace std;


// ********************** controllors **********************


//! controllors

// model basic
#define NUM_THREADS_WORKERS 32
#define NUM_THREADS_GUROBI 4

#define SLEEP_TIME 3

#define GENERATE_NUM 5
#define GENERATE_TIMELIMIT 30
#define GENERATE_DURABLE 5


// ********************** constants **********************


#define SC_COEFF 39999

// for region
#define REGION_INC 0
#define REGION_INC_BORDER 1
#define REGION_INC_MINIBORDER 2
#define REGION_EXC_CANNOT 3
#define REGION_EXC_INNER 4
#define REGION_EXC_OUTTER 5



#define MODEL_OPTIMAL 1
#define MODEL_INFESIBLE 2
#define MODEL_TIMELIMIT 3

#define TASK_TYPE_NONE 0
#define TASK_TYPE_POINT 1



#define FLAG_STRICT_INC 0
#define FLAG_ON 1
#define FLAG_EXC 2




#define REGION_then_BORDER

// ! type redefinition
typedef int coeff_t;
typedef int wind_t;
typedef uint32_t p_t;
typedef vector<p_t> points_t;
typedef vector<wind_t> index_t;
typedef char flag_t;

#ifdef REGION_then_BORDER
#define ALG "REGION_then_BORDER"
#endif

#ifdef BORDER_then_REGION
#define ALG "BORDER_then_REGION"
#endif

#ifdef WEIGHTED_ONE
#define ALG "WEIGHTED_ONE"
#endif

#ifdef SIZE_then_STRENGTH
#define ALG "SIZE_then_STRENGTH"
#endif



//pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
//#define DATA_LOCK() {do {pthread_mutex_lock(&data_mutex); } while (0);}
//#define DATA_UNLOCK() {do {pthread_mutex_unlock(&data_mutex); } while (0);}


//! useful functions
#define SB_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SB_MIN(a, b) ((a) < (b) ? (a) : (b))


#define INEQ_L(diff, z, dim) \
    GRBLinExpr l = 0; \
    p_t tp = diff; \
    wind_t j = 0; \
    while (tp) { \
        if (tp & 1) { \
            l += z[j]; \
        } \
        tp >>= 1; \
        j++; \
    } \
    l -= z[dim];


/* convert timeval to miliseconds */
#define TIMEVAL2F(stamp) ((stamp).tv_sec * 1000.0 + (stamp).tv_usec / 1000.0)

/* get timestamp to the precision of miliseconds since the program starts */
inline double get_timestamp() {
    static double __init_stamp = -1;
    static struct timeval __cur_time;
    if (-1 == __init_stamp) {
        gettimeofday(&__cur_time, NULL);
        __init_stamp = TIMEVAL2F(__cur_time);
    }

    gettimeofday(&__cur_time, NULL);
    return ((TIMEVAL2F(__cur_time) - __init_stamp) / 1000.0);
}

/* print msg with timestamp */
#define PRINTF_STAMP(format, ...) \
do { \
    flockfile(stdout); \
    printf("%12.2f - ", get_timestamp()); \
    printf(format, ##__VA_ARGS__); \
    fflush(stdout); \
    funlockfile(stdout); \
} while(0)


inline string getCurrentSystemTime() {
    auto tt = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime(&tt);
    char date[60] = { 0 };
    sprintf(date, "%d-%02d-%02d-%02d:%02d:%02d", (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday, (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return string(date);
}

















class Input {

public:

    points_t inc;
    points_t exc;
    vector<flag_t> table; // 1 means inclusive; 0 means exclusive.

    wind_t dim;

    wind_t min_strength;

    wind_t n_workers;
    wind_t n_threads_gurobi;

    string points_filename;
    string ineqs_filename;

    // save result
    std::string result_filename;

    vector<string> source_dirs;

    vector<vector<p_t>> weight_list;

    Input(int argc, std::vector<std::string> argv);
    void load_points();
    void calculate_weight_list();

};

class Region {

public:

    vector<flag_t> diff_to_type;
    index_t diff_to_index;

    p_t center;
    wind_t max_dist;

    points_t border;
    points_t miniborder;
    points_t region;
    points_t outter;

    void generate(Input *p_in);

};




class Inequality {
public:
    p_t center;

    vector<coeff_t> coeff;
    index_t inc_index;
    points_t on_diff;
    index_t noton_index;
    points_t exc_diff;

    flag_t point_notin_equ_normal(p_t diff);

    void update_inc_exc(Region &reg);
    void update_noton_on(Region &reg);

    void simplify();
    string tostring();
    wind_t zerocoeff();

    void display();
    void display_detail();
    void write(FILE *fout);
    void write_detail(FILE *fout);
    bool read(FILE *fin, p_t center, wind_t dim);
    void clear();
};





class Dyn_task;

class Task {

public:

    wind_t type;
    p_t center;

    wind_t task_id;
    wind_t thread_idx;

    double time;

    Region reg;

    Inequality first;
    vector<Inequality> ineq_set;


    void superball_type_point(Input *p_in, Dyn_task *dyn);
    flag_t generate_inequality(Input *p_in);

};



class Dyn_task {

public:

    vector<Task> task_array;
    wind_t n_done;
    wind_t working_threads;

    Dyn_task(Input &in);

};


struct parameters_t {

    Input *p_in;
    Dyn_task *p_dyn;
    wind_t thread_idx;

};




class Pool {

public:

    double time;

    wind_t cover_borders;

    vector<Inequality> candidate_set;

    index_t sel_index;
    vector<Inequality> selection;

    Pool(Input &in);

    void find_min_size(Input &in);

    void display();
    void write(Input &in);
    void clear();


};









// ********************** functions **********************








inline wind_t weight(p_t a) {
    p_t c = a;
    wind_t res = 0;
    while (c != 0) {
        if (c & 1) {
            res += 1;
        }
        c >>= 1;
    }
    return res;
}





#endif //EASYBC_SUPERBALL_H
