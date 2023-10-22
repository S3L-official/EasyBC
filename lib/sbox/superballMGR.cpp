//
// Created by Septi on 2/3/2023.
//
#include "superballMGR.h"

void *superballMGR::get_task(void *paraall) {
    pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
#define DATA_LOCK() {do {pthread_mutex_lock(&data_mutex); } while (0);}
#define DATA_UNLOCK() {do {pthread_mutex_unlock(&data_mutex); } while (0);}

    parameters_t *par = (parameters_t *) paraall;
    Dyn_task *dyn = par->p_dyn;

    Task tk;
    bool i_am_working = true;

    while (true) {
        DATA_LOCK()
        if (dyn->task_array.size() == 0) {
            if (dyn->working_threads == 0) {
                // all works are done
                DATA_UNLOCK()
                return NULL;

            } else {
                // still has workers
                if (i_am_working) {
                    dyn->working_threads--;
                    i_am_working = false;
                }
                DATA_UNLOCK()
                if (dyn->task_array.size()) {
                    sleep(SLEEP_TIME); // in seconds.
                }
                continue;
            }
        }

        if (!i_am_working) {
            dyn->working_threads++;
            i_am_working = true;
        }

        // assign task
        tk = dyn->task_array[dyn->task_array.size() - 1];
        tk.task_id = dyn->task_array.size() - 1;
        dyn->task_array.pop_back();
        tk.thread_idx = par->thread_idx;
        DATA_UNLOCK()


        switch (tk.type) {
            case TASK_TYPE_POINT: {

                tk.superball_type_point(par->p_in, dyn);

                break;
            }
            default:
                break;
        }



        DATA_LOCK()
        dyn->n_done += 1;
        DATA_UNLOCK()
    }

    return NULL;
}

int superballMGR::superballGS(SboxM sboxM) {
    /*
         * Generate Inequalities
         * */
    std::vector<std::string> sboxes = {"SKINNY_4", "GIFT", "Elephant", "PRESENT", "KLEIN", "TWINE", "PRINCE", "Piccolo", "MIBS",
                                       "LBlock_0", "LBlock_1", "LBlock_2", "LBlock_3", "LBlock_4", "LBlock_5", "LBlock_6",
                                       "LBlock_7", "LBlock_8", "LBlock_9",
                                       "SERPENT_S0", "SERPENT_S1", "SERPENT_S2", "SERPENT_S3", "SERPENT_S4", "SERPENT_S5",
                                       "SERPENT_S6", "SERPENT_S7",
                                       "Midori_Sb0", "Midori_Sb1", "Rectangle", "Pride"};
    int argc = 7;
    std::string pro = sboxM.get_name();
    std::string filePath = sboxM.get_path() + "possPmHex/4bits/" + sboxM.get_name() + ".txt";
    std::string dim = std::to_string(sboxM.get_sbox_len() * 2);
    std::string nWorkers = "32";
    std::string nThreads = "4";
    /*std::string sourcePath1 = sboxM.get_path() + "superball/" + std::to_string(sboxM.get_sbox_len()) + "bits/" + sboxM.get_name() + "/BORDER_then_REGION";
    std::string sourcePath2 = sboxM.get_path() + "superball/" + std::to_string(sboxM.get_sbox_len()) + "bits/" + sboxM.get_name() + "/REGION_then_BORDER";
    std::string sourcePath3 = sboxM.get_path() + "superball/" + std::to_string(sboxM.get_sbox_len()) + "bits/" + sboxM.get_name() + "/WEIGHTED_ONE";*/
    std::string sourcePath1 = sboxM.get_path() + "superball/" + std::to_string(sboxM.get_sbox_len()) + "bits/" + sboxM.get_name() + "/";
    std::vector<std::string> argvs = {pro, filePath, dim, nWorkers, nThreads, "-1", sourcePath1};

    Input in(argc, argvs);

    PRINTF_STAMP("Start computing superball in parallel\n\n");

    Dyn_task dyn_task(in);
    // initialize workers
    pthread_t *threads = new pthread_t[in.n_workers];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    parameters_t *para = new parameters_t[in.n_workers];

    for (wind_t th = 0; th < in.n_workers; th++) {

        para[th].p_in = &in;
        para[th].p_dyn = &dyn_task;

        para[th].thread_idx = th;

        pthread_create(threads + th, &attr, get_task, (void *) (para + th));
    }

    for (wind_t th = 0; th < in.n_workers; th++) {
        pthread_join(threads[th], NULL);
    }

    delete threads;
    delete para;

    PRINTF_STAMP("All done ~~~~~\n\n");

    cout << getCurrentSystemTime() << endl;


    /*
     * Selected Inequalities
     * */
    PRINTF_STAMP("Select inequalities from candidate set\n\n");

    // load candidate inequalities.
    Pool pool(in);

    // find the minimial number of generators.
    pool.find_min_size(in);


    PRINTF_STAMP("All done ~~~~~\n\n");

    cout << getCurrentSystemTime() << endl;
}

