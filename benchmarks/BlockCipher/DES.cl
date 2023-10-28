@cipher DES

sbox uint4[64] s1 = {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7,
                                        0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8,
                                        4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0,
                                        15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13};
sbox uint4[64] s2 = {15 , 1 , 8 , 14 , 6 , 11 , 3 , 4 , 9 , 7 , 2 , 13 , 12 , 0 , 5 , 10 ,
                                    3 , 13 , 4 , 7 , 15 , 2 , 8 , 14 , 12 , 0 , 1 , 10 , 6 , 9 , 11 , 5 ,
                                    0 , 14 , 7 , 11 , 10 , 4 , 13 , 1 , 5 , 8 , 12 , 6 , 9 , 3 , 2 , 15 ,
                                    13 , 8 , 10 , 1 , 3 , 15 , 4 , 2 , 11 , 6 , 7 , 12 , 0 , 5 , 14 , 9};
sbox uint4[64] s3 = {10 , 0 , 9 , 14 , 6 , 3 , 15 , 5 , 1 , 13 , 12 , 7 , 11 , 4 , 2 , 8 ,
                                    13 , 7 , 0 , 9 , 3 , 4 , 6 , 10 , 2 , 8 , 5 , 14 , 12 , 11 , 15 , 1 ,
                                    13 , 6 , 4 , 9 , 8 , 15 , 3 , 0 , 11 , 1 , 2 , 12 , 5 , 10 , 14 , 7 ,
                                    1 , 10 , 13 , 0 , 6 , 9 , 8 , 7 , 4 , 15 , 14 , 3 , 11 , 5 , 2 , 12};

sbox uint4[64] s4 = {7 , 13 , 14 , 3 , 0 , 6 , 9 , 10 , 1 , 2 , 8 , 5 , 11 , 12 , 4 , 15 ,
                                    13 , 8 , 11 , 5 , 6 , 15 , 0 , 3 , 4 , 7 , 2 , 12 , 1 , 10 , 14 , 9 ,
                                    10 , 6 , 9 , 0 , 12 , 11 , 7 , 13 , 15 , 1 , 3 , 14 , 5 , 2 , 8 , 4 ,
                                    3 , 15 , 0 , 6 , 10 , 1 , 13 , 8 , 9 , 4 , 5 , 11 , 12 , 7 , 2 , 14};

sbox uint4[64] s5 = {2 , 12 , 4 , 1 , 7 , 10 , 11 , 6 , 8 , 5 , 3 , 15 , 13 , 0 , 14 , 9 ,
                                     14 , 11 , 2 , 12 , 4 , 7 , 13 , 1 , 5 , 0 , 15 , 10 , 3 , 9 , 8 , 6 ,
                                     4 , 2 , 1 , 11 , 10 , 13 , 7 , 8 , 15 , 9 , 12 , 5 , 6 , 3 , 0 , 14 ,
                                     11 , 8 , 12 , 7 , 1 , 14 , 2 , 13 , 6 , 15 , 0 , 9 , 10 , 4 , 5 , 3};

sbox uint4[64] s6 = {12 , 1 , 10 , 15 , 9 , 2 , 6 , 8 , 0 , 13 , 3 , 4 , 14 , 7 , 5 , 11 ,
                                     10 , 15 , 4 , 2 , 7 , 12 , 9 , 5 , 6 , 1 , 13 , 14 , 0 , 11 , 3 , 8 ,
                                     9 , 14 , 15 , 5 , 2 , 8 , 12 , 3 , 7 , 0 , 4 , 10 , 1 , 13 , 11 , 6 ,
                                     4 , 3 , 2 , 12 , 9 , 5 , 15 , 10 , 11 , 14 , 1 , 7 , 6 , 0 , 8 , 13};

sbox uint4[64] s7 = {4 , 11 , 2 , 14 , 15 , 0 , 8 , 13 , 3 , 12 , 9 , 7 , 5 , 10 , 6 , 1 ,
                                     13 , 0 , 11 , 7 , 4 , 9 , 1 , 10 , 14 , 3 , 5 , 12 , 2 , 15 , 8 , 6 ,
                                     1 , 4 , 11 , 13 , 12 , 3 , 7 , 14 , 10 , 15 , 6 , 8 , 0 , 5 , 9 , 2 ,
                                     6 , 11 , 13 , 8 , 1 , 4 , 10 , 7 , 9 , 5 , 0 , 15 , 14 , 2 , 3 , 12};

sbox uint4[64] s8 = {13 , 2 , 8 , 4 , 6 , 15 , 11 , 1 , 10 , 9 , 3 , 14 , 5 , 0 , 12 , 7 ,
                                     1 , 15 , 13 , 8 , 10 , 3 , 7 , 4 , 12 , 5 , 6 , 11 , 0 , 14 , 9 , 2 ,
                                     7 , 11 , 4 , 1 , 9 , 12 , 14 , 2 , 0 , 6 , 10 , 13 , 15 , 3 , 5 , 8 ,
                                     2 , 1 , 14 , 7 , 4 , 10 , 8 , 13 , 15 , 12 , 9 , 0 , 3 , 5 , 6 , 11};

pbox uint[48] expansion = {31, 0, 1, 2, 3, 4, 3, 4, 5, 6, 7, 8, 7, 8, 9, 10, 11, 12, 11, 12, 13, 14, 15, 16, 15, 16, 17, 18, 19, 20, 19, 20, 21, 22, 23, 24, 23, 24, 25, 26, 27, 28, 27, 28, 29, 30, 31, 0};
pbox uint[32] p = {15, 6, 19, 20, 28, 11, 27, 16, 0, 14, 22, 25, 4, 17, 30, 9, 1, 7, 23, 13, 31, 26, 2, 8, 18, 12, 29, 5, 21, 10, 3, 24};
pbox uint[64] ipr_table = {39, 7, 47, 15, 55, 23, 63, 31, 38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29, 36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27, 34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41, 9, 49, 17, 57, 25, 32, 0, 40, 8, 48, 16, 56, 24};

r_fn uint1[64] round_function(uint8 r, uint1[48] key, uint1[64] input) {
    uint1[32] l_input = View(input, 0, 31);
    uint1[32] r_input = View(input, 32, 63);
    uint1[48] ext = expansion<r_input>;
    ext = ext ^ key;

    uint1[32] s_out;
    uint1[4] ts_out1 = s1<View(ext, 0, 5)>;
    uint1[4] ts_out2 = s2<View(ext, 6, 11)>;
    uint1[4] ts_out3 = s3<View(ext, 12, 17)>;
    uint1[4] ts_out4 = s4<View(ext, 18, 23)>;
    uint1[4] ts_out5 = s5<View(ext, 24, 29)>;
    uint1[4] ts_out6 = s6<View(ext, 30, 35)>;
    uint1[4] ts_out7 = s7<View(ext, 36, 41)>;
    uint1[4] ts_out8 = s8<View(ext, 42, 47)>;
    for (i from 0 to 3) {
        s_out[i] = ts_out1[3-i];
        s_out[i + 4] = ts_out2[3-i];
        s_out[i + 8] = ts_out3[3-i];
        s_out[i + 12] = ts_out4[3-i];
        s_out[i + 16] = ts_out5[3-i];
        s_out[i + 20] = ts_out6[3-i];
        s_out[i + 24] = ts_out7[3-i];
        s_out[i + 28] = ts_out8[3-i];
    }
    uint1[32] p_out = p<s_out>;
    uint1[32] x_out = p_out ^ l_input;
    uint1[64] rtn;
    for (i from 0 to 31) {
        rtn[i] = r_input[i];
        rtn[i + 32] = x_out[i];
    }
    return rtn;
}

r_fn uint1[64] round_function2(uint8 r, uint1[48] key, uint1[64] input) {
    uint1[32] l_input = View(input, 0, 31);
    uint1[32] r_input = View(input, 32, 63);
    uint1[48] ext = expansion<r_input>;
    ext = ext ^ key;

    uint1[32] s_out;
    uint1[4] ts_out1 = s1<View(ext, 0, 5)>;
    uint1[4] ts_out2 = s2<View(ext, 6, 11)>;
    uint1[4] ts_out3 = s3<View(ext, 12, 17)>;
    uint1[4] ts_out4 = s4<View(ext, 18, 23)>;
    uint1[4] ts_out5 = s5<View(ext, 24, 29)>;
    uint1[4] ts_out6 = s6<View(ext, 30, 35)>;
    uint1[4] ts_out7 = s7<View(ext, 36, 41)>;
    uint1[4] ts_out8 = s8<View(ext, 42, 47)>;
    for (i from 0 to 3) {
        s_out[i] = ts_out1[3-i];
        s_out[i + 4] = ts_out2[3-i];
        s_out[i + 8] = ts_out3[3-i];
        s_out[i + 12] = ts_out4[3-i];
        s_out[i + 16] = ts_out5[3-i];
        s_out[i + 20] = ts_out6[3-i];
        s_out[i + 24] = ts_out7[3-i];
        s_out[i + 28] = ts_out8[3-i];
    }
    uint1[32] p_out = p<s_out>;
    uint1[32] x_out = p_out ^ l_input;
    uint1[64] rtn;
    for (i from 0 to 31) {
        rtn[i] = r_input[i];
        rtn[i + 32] = x_out[i];
    }
    rtn = ipr_table<rtn>;
    return rtn;
}

fn uint1[64] enc(uint1[768] key, uint1[64] r_plaintext){
    for (i from 1 to 15) {
        r_plaintext = round_function(i, View(key, (i - 1) * 48, i * 48 - 1), r_plaintext);
    }
    r_plaintext = round_function2(i, View(key, 720, 747), r_plaintext);
    return r_plaintext;
}