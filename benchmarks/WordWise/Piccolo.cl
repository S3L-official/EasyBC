@cipher Piccolo

sbox uint4[16] s = {14, 4, 11, 2, 3, 8, 0, 9, 1, 10, 7, 15, 6, 12, 5, 13};

pbox uint[64] p = {4, 5, 14, 15, 8, 9, 2, 3, 12, 13, 6, 7, 0, 1, 10, 11};
# 2 7 4 1 6 3 0 5

pboxm uint4[4][4] M = {{2, 3, 1, 1}, {1, 2, 3, 1}, {1, 1, 2, 3}, {3, 1, 1, 2}};
ffm uint4[16][16] M = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                    {0, 2, 4, 6, 8, 10, 12, 14, 3, 1, 7, 5, 11, 9, 15, 13},
                    {0, 3, 6, 5, 12, 15, 10, 9, 11, 8, 13, 14, 7, 4, 1, 2},
                    {0, 4, 8, 12, 3, 7, 11, 15, 6, 2, 14, 10, 5, 1, 13, 9},
                    {0, 5, 10, 15, 7, 2, 13, 8, 14, 11, 4, 1, 9, 12, 3, 6},
                    {0, 6, 12, 10, 11, 13, 7, 1, 5, 3, 9, 15, 14, 8, 2, 4},
                    {0, 7, 14, 9, 15, 8, 1, 6, 13, 10, 3, 4, 2, 5, 12, 11},
                    {0, 8, 3, 11, 6, 14, 5, 13, 12, 4, 15, 7, 10, 2, 9, 1},
                    {0, 9, 1, 8, 2, 11, 3, 10, 4, 13, 5, 12, 6, 15, 7, 14},
                    {0, 10, 7, 13, 14, 4, 9, 3, 15, 5, 8, 2, 1, 11, 6, 12},
                    {0, 11, 5, 14, 10, 1, 15, 4, 7, 12, 2, 9, 13, 6, 8, 3},
                    {0, 12, 11, 7, 5, 9, 14, 2, 10, 6, 1, 13, 15, 3, 4, 8},
                    {0, 13, 9, 4, 1, 12, 8, 5, 2, 15, 11, 6, 3, 14, 10, 7},
                    {0, 14, 15, 1, 13, 3, 2, 12, 9, 7, 6, 8, 4, 10, 11, 5},
                    {0, 15, 13, 2, 9, 6, 4, 11, 1, 14, 12, 3, 8, 7, 5, 10}};

r_fn uint4[16] round_function1(uint8 r, uint4[8] key, uint4[16] input) {
    for (i from 0 to 3) {
        input[i] = input[i] ^ key[i];
        input[i + 8] = input[i + 8] ^ key[i + 4];
    }
    return input;
}

r_fn uint4[16] round_function2(uint8 r, uint4[8] key, uint4[16] input) {
    uint4[4] s_out1;
    uint4[4] s_out2;
    for (i from 0 to 3) {
        s_out1[i] = s<input[i]>;
        s_out2[i] = s<input[i+8]>;
    }

    uint4[4] m_out1 = M * s_out1;
    uint4[4] m_out2 = M * s_out2;

    uint4[4] s_out3;
    uint4[4] s_out4;
    for (i from 0 to 3) {
        s_out3[i] = s<m_out1[i]>;
        s_out4[i] = s<m_out2[i]>;
    }

    uint4[16] n_input = input;
    for (i from 0 to 3) {
        n_input[i+4] = s_out3[i] ^ input[i+4] ^ key[i];
    }
    for (i from 8 to 11) {
        n_input[i+4] = s_out4[i - 8] ^ input[i+4] ^ key[i - 4];
    }
    uint4[16] p_out = p<n_input>;
    return p_out;
}

r_fn uint4[16] round_function3(uint8 r, uint4[8] key, uint4[16] input) {
    uint4[4] s_out1;
    uint4[4] s_out2;
    for (i from 0 to 3) {
        s_out1[i] = s<input[i]>;
        s_out2[i] = s<input[i+8]>;
    }

    uint4[4] m_out1 = M * s_out1;
    uint4[4] m_out2 = M * s_out2;

    uint4[4] s_out3;
    uint4[4] s_out4;
    for (i from 0 to 3) {
        s_out3[i] = s<m_out1[i]>;
        s_out4[i] = s<m_out2[i]>;
    }

    uint4[16] n_input = input;
    for (i from 4 to 7) {
        n_input[i] = s_out3[i - 4] ^ input[i] ^ key[i - 4];
    }
    for (i from 12 to 15) {
        n_input[i] = s_out4[i - 12] ^ input[i] ^ key[i - 8];
    }
    return n_input;
}

fn uint4[16] enc(uint4[216] key, uint4[16] r_plaintext){
    #r_plaintext = round_function1(1, View(key, 0, 7), r_plaintext);
    for (i from 2 to 25) {
        r_plaintext = round_function2(i, View(key, (i - 1) * 8, i * 8 - 1), r_plaintext);
    }
    r_plaintext = round_function3(26, View(key, 200, 207), r_plaintext);
    r_plaintext = round_function1(27, View(key, 208, 215), r_plaintext);
    return r_plaintext;
}