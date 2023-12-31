//
// Created by Septi on 4/18/2023.
//

#include "ValueCommon.h"

bool ValueCommon::isNoParameter(ValuePtr valuePtr) {
    std::set<ValuePtr> visited;
    bool noPara = true;
    isNoParameterHelper(std::move(valuePtr), noPara, visited);
    return noPara;
}

void ValueCommon::isNoParameterHelper(ValuePtr valuePtr, bool &noPara, std::set<ValuePtr> &visited) {
    if(!noPara)
        return;
    if(valuePtr->getValueType() == ValueType::VTParameterValue) {
        noPara = false;
        return;
    } else if(valuePtr->getValueType() == ValueType::VTConstantValue) {
        noPara = false;
        return;
    }
    else if(valuePtr->getValueType() == ValueType::VTProcCallValueIndex) {
        return;
    } else if(InternalBinValue* value = dynamic_cast<InternalBinValue*>(valuePtr.get())) {
        if(visited.find(valuePtr) == visited.end()) {
            isNoParameterHelper(value->getLeft(), noPara, visited);
            if (noPara)
                isNoParameterHelper(value->getRight(), noPara, visited);
            visited.insert(valuePtr);
        }
    } else if(InternalUnValue* value = dynamic_cast<InternalUnValue*>(valuePtr.get())) {
        if(visited.find(valuePtr) == visited.end()) {
            isNoParameterHelper(value->getRand(), noPara, visited);
            visited.insert(valuePtr);
        }
    } else if(ArrayValue* value = dynamic_cast<ArrayValue*>(valuePtr.get())) {
        for(auto ele : value->getArrayValue())
            if (noPara && visited.find(ele) == visited.end()) {
                isNoParameterHelper(ele, noPara, visited);
                visited.insert(ele);
            }
    }
    else if (ArrayValueIndex* value = dynamic_cast<ArrayValueIndex*>(valuePtr.get())) {
        if (value->getSymbolIndex() != nullptr)
            noPara = false;
        return;
    }
    else {
        return;
    }
}