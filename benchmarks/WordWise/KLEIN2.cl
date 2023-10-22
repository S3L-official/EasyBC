@cipher KLEIN

sbox uint4[16] s = {7, 4, 10, 9, 1, 15, 11, 0, 12, 3, 2, 6, 8, 14, 13, 5};
pbox uint[16] p = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3};
pboxm uint4[4][4] M = {{2, 3, 1, 1},
                        {1, 2, 3, 1},
                        {1, 1, 2, 3},
                        {3, 1, 1, 2}};
ffm uint4[16][16] M = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                      {0, 2, 4, 6, 8, 10, 12, 14, 3, 1, 7, 5, 11, 9, 15, 13},
                      {0, 3, 6, 5, 12, 15, 10, 9, 11, 8, 13, 14, 7, 4, 1, 2},
                      {0, 4, 8, 12, 3, 7, 11, 15, 6, 2, 14, 10, 5, 1, 13, 9},
                      {0, 5, 10, 15, 7, 2, 13, 8, 14, 11, 4, 1, 9, 12, 3, 6},
                      {0, 6, 12, 10, 11, 13, 7, 1, 5, 3, 9, 15, 14, 8, 2, 4},
                      {0, 6, 12, 10, 11, 13, 7, 1, 5, 3, 9, 15, 14, 8, 2, 4},
                      {0, 8, 3, 11, 6, 14, 5, 13, 12, 4, 15, 7, 10, 2, 9, 1},
                      {0, 9, 1, 8, 2, 11, 3, 10, 4, 13, 5, 12, 6, 15, 7, 14},
                      {0, 10, 7, 13, 14, 4, 9, 3, 15, 5, 8, 2, 1, 11, 6, 12},
                      {0, 11, 5, 14, 10, 1, 15, 4, 7, 12, 2, 9, 13, 6, 8, 3},
                      {0, 12, 11, 7, 5, 9, 14, 2, 10, 6, 1, 13, 15, 3, 4, 8},
                      {0, 13, 9, 4, 1, 12, 8, 5, 2, 15, 11, 6, 3, 14, 10, 7},
                      {0, 14, 15, 1, 13, 3, 2, 12, 9, 7, 6, 8, 4, 10, 11, 5},
                      {0, 15, 13, 2, 9, 6, 4, 11, 1, 14, 12, 3, 8, 7, 5, 10}};

r_fn uint4[16] round_function1(uint8 r, uint4[16] key, uint4[16] input) {
	uint4[16] n_input = input ^ key;
    uint4[16] s_out;
    for (i from 0 to 15) {
        s_out[i] = s<n_input[i]>;
    }
    uint4[16] p_out = p<s_out>;

    uint4[4] m_in1;
    uint4[4] m_in2;
    uint4[4] m_in3;
    uint4[4] m_in4;
    for (i from 0 to 3) {
        m_in1[i] = p_out[i*2];
        m_in2[i] = p_out[i*2+1];
        m_in3[i] = p_out[i*2+8];
        m_in4[i] = p_out[i*2+9];
    }
    uint4[4] m_out1 = M * m_in1;
    uint4[4] m_out2 = M * m_in2;
    uint4[4] m_out3 = M * m_in3;
    uint4[4] m_out4 = M * m_in4;
    uint4[16] rtn;
    for (i from 0 to 3) {
        rtn[i*2] = m_out1[i];
        rtn[i*2+1] = m_out2[i];
        rtn[i*2+8] = m_out3[i];
        rtn[i*2+9] = m_out4[i];
    }
	return rtn;
}

r_fn uint4[16] round_function2(uint8 r, uint4[16] key, uint4[16] input) {
    input = input ^ key;
    return input;
}

fn uint4[16] enc(uint4[208] key, uint4[16] r_plaintext){
    for (i from 1 to 12) {
        r_plaintext = round_function1(i, View(key, (i - 1) * 16, i * 16 - 1), r_plaintext);
    }
    r_plaintext = round_function2(13, View(key, 192, 207), r_plaintext);
    return r_plaintext;
}