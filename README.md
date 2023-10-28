# EasyBC
This repository is for parsing the input **[EasyDC]** programs of block ciphers and then performing the security analysis of block ciphers against differential cryptanalysis. It supports the following functions: 
- Interpreter of EasyBC.
- SMT-based method for determiningg branch numbers of various cryptographic operations.
- Modeling differential propagations in S-boxes.
- MaxSMT-based method for determining the encoding of all probabilities in a given S-box's differential distribution table(DDT).
- Security analysis of block ciphers writing in EasyBC against differential cryptanalysis.

## Quick Start
### Preparatory Environment:
[flex](https://github.com/westes/flex), [bison](https://www.gnu.org/software/bison/), [jsoncpp](https://github.com/open-source-parsers/jsoncpp), [Gurobi](https://www.gurobi.com/solutions/gurobi-optimizer/) and [Z3](https://github.com/Z3Prover/z3)
### Building
You should first prepare the preparatory environment, then set up your Gurobi environment in the 'CMakeLists.txt' file, and then run the following command to build the project:
```
    mkdir build
    cmake CMakeList.txt -B build
    cd build; make
```
### Usage
You can get the help information by executing:
```
./EasyBC -h
```

<!-- ### 1. Modeling possible differential propagations in DDTs of S-boxes
  - comand : "./EasyDC argv[1]  argv[2]  argv[3]  argv[4] " : 

    |  | argv[1] | argv[2] | argv[3] | argv[4] |
    | :-----: | :-----: | :----: | :----: | :----: |
    | Remark | name of S-box | S-box | mode of modelings, taking probabilities of possible differential propagations in DDTs into account or not | choosing reduction methods |
    | Options | | | "AS"; "DC" | "1" : T1; "2" : T2; "3" : T3; "4" : T4; "5" : T5; "6" : T6; "7" : T7; "8" : T8 |  
    
     *E.g.*,  `$ ./EasyDC Present 4,15,3,8,13,10,12,0,11,5,7,14,2,6,1,9 AS 1`
  - set paramters via *[parameters.txt](https://anonymous.4open.science/r/EasyDC-73BC/parameters.txt)* file.
    *E.g.*, *[parametersSboxDemo.txt](https://anonymous.4open.science/r/EasyDC-73BC/parametersSboxDemo.txt)*  
    
    ```
    $ cat parametersSboxDemo.txt  
    Present  
    4,15,3,8,13,10,12,0,11,5,7,14,2,6,1,9  
    AS  
    1
    ``` 

### 2. Security analysis of block ciphers against differential cryptanalysis
  - comand : "./EasyDC argv[1]  argv[2]  argv[3]  argv[4]  argv[5]  argv[6]  argv[7]  argv[8]  argv[9]  argv[10]  argv[11]  argv[12]  argv[13] " : 

    |  | argv[1] | argv[2] | argv[3] | argv[4] | argv[5] | argv[6]/argv[8]/argv[10]/argv[12] | argv[7]/argv[9]/argv[11]/argv[13] |
    | :-----: | :-----: | :----: | :----: | :----: | :----: | :----: | :----: |
    | Remark | num of parameters | **EasyDC** program file path | modeling approach | mode of modelings for S-boxes | choosing reduction methods for S-boxes | 
    | Options | | | "w" : word-wise approach; "b" : bit-wise approach; "d" : extended bit-wise approach | "AS"; "DC" | "1" : T1; "2" : T2; "3" : T3; "4" : T4; "5" : T5; "6" : T6; "7" : T7; "8" : T8 | startRound or allRounds or timer(second) or threadsNum | startRound or allRounds or timer(second) or threadsNum |
    
    *E.g.*,  `$ ./EasyDC 7 ../benchmarks/BlockCipher/PRESENT.cl b AS 1 allRounds 5`
    
  - set paramters via *[parameters.txt](https://anonymous.4open.science/r/EasyDC-73BC/parameters.txt)* file.
    *E.g.*, *[parametersMILPDemo.txt](https://anonymous.4open.science/r/EasyDC-73BC/parametersMILPDemo.txt)*  
    
    ```
    $ cat parametersMILPDemo.txt  
    7
    ../benchmarks/BlockCipher/PRESENT.cl
    b
    AS
    1
    allRounds
    5
    ```  -->

<!-- ## Example -->

## Full version
The full version of our paper is given [here](https://github.com/S3L-official/EasyBC/blob/main/POPL-full.pdf).
    
