//
// Created by Septi on 2/3/2023.
//

#ifndef EASYBC_SUPERBALLMGR_H
#define EASYBC_SUPERBALLMGR_H


#include "superball.h"
#include "SboxModel.h"


namespace superballMGR {
    void *get_task(void *paraall);


    int superballGS(SboxM sboxM);
}


#endif //EASYBC_SUPERBALLMGR_H
