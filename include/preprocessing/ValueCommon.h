//
// Created by Septi on 4/18/2023.
//

#ifndef EASYBC_VALUECOMMON_H
#define EASYBC_VALUECOMMON_H

#include "Value.h"

namespace ValueCommon {

    bool isFullComputation(ValuePtr valuePtr);

    bool isNoParameter(ValuePtr valuePtr);

    void isNoParameterHelper(ValuePtr valuePtr, bool& noPara, std::set<ValuePtr>& visited);

    bool isNoRandoms(ValuePtr valuePtr);
    void isNoRandomsHelper(ValuePtr valuePtr, bool& noRands, std::set<ValuePtr>& visited);

    bool isNoProcCall(ValuePtr valuePtr);
    void isNoProcCallHelper(ValuePtr valuePtr, bool& noProcCall, std::set<ValuePtr>& visited);

    bool isNoArrayIndex(ValuePtr valuePtr);
    void isNoArrayIndexHelper(ValuePtr valuePtr, bool& noProcCall, std::set<ValuePtr>& visited);

    ValuePtr compactExpression(ValuePtr valuePtr);


    bool containsProcCall(ValuePtr valuePtr, ValuePtr& res);

    void containsProcCallHelper(ValuePtr valuePtr, bool& flag, ValuePtr& res, std::set<ValuePtr>& visited);


    void isNoSameFunctionCall(ValuePtr valuePtr, ValuePtr target, bool& noSameFunctionCall, std::set<ValuePtr>& visited);


    void getProcCallNPara(ValuePtr valuePtr, std::set<ValuePtr>& visited, std::set<ProcedurePtr>& procs, std::set<ValuePtr>& parameters);

    bool noSameCall(ValuePtr left, ValuePtr right);
}

#endif //EASYBC_VALUECOMMON_H
