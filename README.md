# EasyBC
This repository is for parsing the input **[EasyDC]** programs of block ciphers and then performing the security analysis of block ciphers against differential cryptanalysis. It supports the following functions: 
- Interpreter of EasyBC.
- SMT-based method for determiningg branch numbers of various cryptographic operations.
- Modeling differential propagations in S-boxes.
- MaxSMT-based method for determining the encoding of all probabilities in a given S-box's differential distribution table(DDT).
- Security analysis of block ciphers writing in EasyBC against differential cryptanalysis.

[//]: # (# Quick Start)
## Preparatory Environment:
[flex](https://github.com/westes/flex), [bison](https://www.gnu.org/software/bison/), [jsoncpp](https://github.com/open-source-parsers/jsoncpp), [Gurobi](https://www.gurobi.com/solutions/gurobi-optimizer/) and [Z3](https://github.com/Z3Prover/z3)
## Building
You should first prepare the preparatory environment, then set up your Gurobi environment in the 'CMakeLists.txt' file, and then run the following commands to build the project:
```
mkdir build
cmake CMakeList.txt -B build
cd build; make
```
## Usage
- ### Use command line: 
    You can get the help information by executing:
    ``` 
    ./EasyBC -h
    ```
- ### Use the parameter file:
    You can set the specific parameters in the parameter file *[parameters.txt](https://anonymous.4open.science/r/EasyDC-73BC/parameters.txt)* file.
    
    *E.g.*, *[parametersMILPDemo.txt](https://anonymous.4open.science/r/EasyDC-73BC/parametersMILPDemo.txt)*

   ```
   $ cat parametersMILPDemo.txt  
   7    (remark: number of parameters)
   ../benchmarks/BlockCipher/PRESENT.cl    (remark: path of benchmark)
   b    (remark: word-wise, bit-wise or extended bit-wise, i.e., "w", "b" or "d")
   AS   (remark: the security analysis is by calculating minimal number of active S-boxes, or you can perform the security analysis by calculating MaxEDCP, i.e., "AS" -> "DC")
   1    (remark: reduction method of S-boxes, the value ranges from 1 to 8, corresponding to T1 to T8 which mentioned in our paper)
   startRound    
   1
   endRound
   5
   ```  
  
[//]: # (   ```)

[//]: # (   $ cat parametersMILPDemo.txt  )

[//]: # (   7    &#40;remark: number of parameters&#41;)

[//]: # (   ../benchmarks/BlockCipher/PRESENT.cl    &#40;remark: path of benchmark&#41;)

[//]: # (   b    &#40;remark: bit-wise or word-wise, i.e., "b" or "w"&#41;)

[//]: # (   AS   &#40;remark: the security analysis is by calculating minimal number of active S-boxes, or you can perform the security analysis by calculating MaxEDCP, i.e., "AS" -> "DC"&#41;)

[//]: # (   1    &#40;remark: reduction method of S-boxes, the value ranges from 1 to 8, corresponding to T1 to T8 which mentioned in our paper&#41;)

[//]: # (   allRounds    &#40;remark: round number&#41;)

[//]: # (   5)

[//]: # (   ```  )

 <!--
##### 1. Modeling possible differential propagations in DDTs of S-boxes
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

      ##### 2. Security analysis of block ciphers against differential cryptanalysis
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
    
- ### Adding cryptographic primitives
  
    Use EasyBC syntax to implement the primitives and place its implementation file in the **[benchmarks](https://github.com/S3L-official/EasyBC/blob/main/benchmarks)** directory.
    
    - **[benchmarks/BlockCipher](https://github.com/S3L-official/EasyBC/blob/main/benchmarks/BlockCipher)** the implementations of block ciphers
    - **[benchmarks/WordWise](https://github.com/S3L-official/EasyBC/blob/main/benchmarks/WordWise)** the word-wise implementations
    - **[benchmarks/NIST](https://github.com/S3L-official/EasyBC/blob/main/benchmarks/NIST)** the implementations of underlying primitives 
 
    Specify the path of the file corresponding to the primitives in the command line or parameter file, to analyze the security of the primitive.

- ### Modify modeling constraints for operations

    Modify the function corresponding to this operation in the 
    **[lib/differential/DiffMILPcons.cpp](https://github.com/S3L-official/EasyBC/blob/main/lib/differential/DiffMILPcons.cpp)**
    file.
  
    *E.g.,* for XOR in word-wise approach,
    the function of the modeling constraints 
  

    $$
    \{b'\geq b_0, b'\geq b_1, b'\geq b_2, \sum_{i=0}^2b_i\geq 2b'\} 
    $$
    
    is shown in the following:   

    ```
    void DiffMILPcons::wXorC1(std::string path, int inputIdx1, int inputIdx2, int outputIdx, int &dCounter) {
    std::ofstream scons(path, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path of wXORc1 ! " << std::endl;
    } else {
        scons << "A" << inputIdx1 << " + A" << inputIdx2 << " + A" << outputIdx << " - 2 d" << dCounter << " >= 0\n";
        scons << "d" << dCounter << " - A" << inputIdx1 << " >= 0\n";
        scons << "d" << dCounter << " - A" << inputIdx2 << " >= 0\n";
        scons << "d" << dCounter << " - A" << outputIdx << " >= 0\n";
        dCounter++;
    }
    scons.close();
    }
    ```
    
    
<!-- ## Example -->

## Full version
The full version of our paper is given [here](https://github.com/S3L-official/EasyBC/blob/main/POPL-full.pdf).
    
