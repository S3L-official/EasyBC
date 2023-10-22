//
// Created by Septi on 6/27/2022.
//

#ifndef PLBENCH_TRANSFORMER_H
#define PLBENCH_TRANSFORMER_H

#include "Value.h"
#include "list"
#include "ProcedureH.h"
#include "ThreeAddressNode.h"
#include "Interpreter.h"


class Transformer {
private:
    vector<ProcValuePtr> procedures;
    vector<ProcedureHPtr> procedureHs;

    // 在transformArrayValue2oneTAN中作为counter使用
    int leftoverCounter = 0;
public:
    const vector<ProcedureHPtr> &getProcedureHs() const;

private:
    map<string, ProcedureHPtr> nameToProc;

public:
    Transformer(const vector<ProcValuePtr>& procedures) {
        this->procedures = procedures;
    }

    void transformProcedures();

    ThreeAddressNodePtr
    transformInternalBin(InternalBinValuePtr internalBinValuePtr, map<ValuePtr, ThreeAddressNodePtr> &saved,
                         map<string, int> &nameCount);

    ThreeAddressNodePtr
    transformInternalUn(InternalUnValuePtr internalUnValuePtr, map<ValuePtr, ThreeAddressNodePtr> &saved,
                        map<string, int> &nameCount);

    vector<ThreeAddressNodePtr>
    transformArrayValue(ArrayValuePtr arrayValuePtr, map<ValuePtr, ThreeAddressNodePtr> &saved,
                        map<string, int> &nameCount);

    ThreeAddressNodePtr
    transformArrayValue2oneTAN(ArrayValuePtr arrayValuePtr, map<ValuePtr, ThreeAddressNodePtr> &saved,
                        map<string, int> &nameCount);

    ThreeAddressNodePtr
    transformArrayValueIndex(ArrayValueIndex arrayValueIndex, map<ValuePtr, ThreeAddressNodePtr> &saved,
                             map<string, int> &nameCount);

    ThreeAddressNodePtr
    transformBoxValue(BoxValuePtr boxValuePtr, map<ValuePtr, ThreeAddressNodePtr> &saved);

    ProcedureHPtr transform(ProcValuePtr procValuePtr);

    string getCount(string name, map<string, int> &nameCount);

    static NodeType getNodeType(ASTNode::Type type);

    static int getNodeTypeSize(NodeType nodeType);
};


#endif //PLBENCH_TRANSFORMER_H
