//
// Created by Septi on 6/27/2022.
//

#ifndef PLBENCH_PROCEDUREH_H
#define PLBENCH_PROCEDUREH_H


#include "ThreeAddressNode.h"
#include "list"
#include "map"


typedef pair<vector<ThreeAddressNodePtr>, bool> ActualPara;
class ProcedureH;
typedef shared_ptr<ProcedureH> ProcedureHPtr;
class ProcedureH {
private:
    string name;
    vector<vector<ThreeAddressNodePtr>> parameters;
    vector<ThreeAddressNodePtr> block;
    vector<ThreeAddressNodePtr> returns;

    map<string, ProcedureHPtr> nameToProc;
public:
    const map<string, ProcedureHPtr> &getNameToProc() const;

private:
    map<ThreeAddressNodePtr, string> callToNodeName;

public:
    const map<ThreeAddressNodePtr, string> &getCallToNodeName() const;


public:
    ProcedureH(string, const vector<vector<ThreeAddressNodePtr>> &parameters, const vector<ThreeAddressNodePtr> &block,
               const vector<ThreeAddressNodePtr> &returns, const map<string, ProcedureHPtr>& nameToProc);
    string to_string();

    const string &getName() const;

    bool isSimple1() const;

    const vector<vector<ThreeAddressNodePtr>> &getParameters() const;

    const vector<ThreeAddressNodePtr> &getBlock() const;

    const vector<ThreeAddressNodePtr> &getReturns() const;

    void arrangeArguments(vector<ThreeAddressNodePtr>& functionCall, map<ThreeAddressNodePtr, vector<ActualPara>>& mapCallToArguments,
                          map<ThreeAddressNodePtr, set<ThreeAddressNodePtr>>& mapFuncCallArgumentsToCalls, map<ActualPara, set<ThreeAddressNodePtr>>& mapInputArgumentsToCalls,
                          map<ThreeAddressNodePtr, vector<ThreeAddressNodePtr>>& mapCallToReturnValues);
    void buildActualParas(vector<ActualPara>& actualParas, const vector<ThreeAddressNodePtr>& actualArguments,
                          ThreeAddressNodePtr first, map<ThreeAddressNodePtr, set<ThreeAddressNodePtr>>& rootcauseOfMine);

    void getAllRandomsOfANode(const ThreeAddressNodePtr& node , set<string>& allRandoms, set<ThreeAddressNodePtr>& saved);

    vector<vector<ThreeAddressNodePtr>> dealWithPushArguments(const vector<ThreeAddressNodePtr> & actualArgument, ThreeAddressNodePtr callNode);

    bool isShare(vector<ThreeAddressNodePtr>& actualPara, map<ThreeAddressNodePtr, set<ThreeAddressNodePtr>>& rootcauseOfMine, set<ThreeAddressNodePtr>& savedShare);

    bool isSNIShare(vector<ThreeAddressNodePtr>& actualPara, map<ThreeAddressNodePtr, set<ThreeAddressNodePtr>>& rootcauseOfMine, set<ThreeAddressNodePtr>& savedShare);

    static ThreeAddressNodePtr instantitateNode(ProcedureHPtr proc, string basename, ThreeAddressNodePtr node, map<ThreeAddressNodePtr, ThreeAddressNodePtr>& formalToActual, map<string, ThreeAddressNodePtr>& saved, map<ThreeAddressNodePtr, ThreeAddressNodePtr >& savedForFunction, string path, set<ThreeAddressNodePtr>& randomBackup);

    static vector<ThreeAddressNodePtr> instantitateProcedureCall(ProcedureHPtr proc, ThreeAddressNodePtr callInstruction,map<string, ThreeAddressNodePtr>& saved, map<ThreeAddressNodePtr, ThreeAddressNodePtr>& savedForFunction,
                                                                 map<ThreeAddressNodePtr, ThreeAddressNodePtr>& formalToActual, string path, set<ThreeAddressNodePtr>& backup );

    void ProcedureHInfoPrint();
};

#endif //PLBENCH_PROCEDUREH_H
