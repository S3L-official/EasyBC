# the implementation without "touint" function,
# and the XOR operation subjects are uint1 arrays other than bits
@cipher Piccolo_64

sbox uint4[16] s = {14, 4, 11, 2, 3, 8, 0, 9, 1, 10, 7, 15, 6, 12, 5, 13};
pbox uint[64] p = {16, 17, 18, 19, 20, 21, 22, 23, 56, 57, 58, 59, 60, 61, 62, 63, 32, 33, 34, 35, 36, 37, 38, 39, 8, 9, 10, 11, 12, 13, 14, 15, 48, 49, 50, 51, 52, 53, 54, 55, 24, 25, 26, 27, 28, 29, 30, 31, 0, 1, 2, 3, 4, 5, 6, 7, 40, 41, 42, 43, 44, 45, 46, 47};
#pbox uint[64] p = {47, 46, 45, 44, 43, 42, 41, 40, 7, 6, 5, 4, 3, 2, 1, 0, 31, 30, 29, 28, 27, 26, 25, 24, 55, 54, 53, 52, 51, 50, 49, 48, 15, 14, 13, 12, 11, 10, 9, 8, 39, 38, 37, 36, 35, 34, 33, 32, 63, 62, 61, 60, 59, 58, 57, 56, 23, 22, 21, 20, 19, 18, 17, 16};
#pbox uint[64] p = {16, 17, 18, 19, 20, 21, 22, 23, 56, 57, 58, 59, 60, 61, 62, 63, 48, 49, 50, 51, 52, 53, 54, 55, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 40, 41, 42, 43, 44, 45, 46, 47};

pboxm uint4[4][4] M = {{2, 3, 1, 1}, {1, 2, 3, 1}, {1, 1, 2, 3}, {3, 1, 1, 2}};
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

r_fn uint1[64] round_function1(uint8 r, uint1[32] key, uint1[64] input) {
    for (i from 0 to 15) {
        input[i] = input[i] ^ key[i];
        input[i + 32] = input[i + 32] ^ key[i + 16];
    }
    return input;
}

r_fn uint1[64] round_function2(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[16] s_out1;
    uint1[16] s_out2;
    for (i from 0 to 3) {
        uint1[4] sbox_in = View(input, i*4, i*4+3);
        # uint1[4] sbox_in = {input[i*4+3], input[i*4+2], input[i*4+1], input[i*4]};
        uint1[4] sbox_out = s<sbox_in>;
        s_out1[i * 4 + 0] = sbox_out[0];
        s_out1[i * 4 + 1] = sbox_out[1];
        s_out1[i * 4 + 2] = sbox_out[2];
        s_out1[i * 4 + 3] = sbox_out[3];
    }
    for (i from 0 to 3) {
        uint1[4] sbox_in = View(input, 32 + (i*4), 32 + (i*4)+3);
        # uint1[4] sbox_in = {input[32 + (i*4)+3], input[32 + (i*4)+2], input[32 + (i*4)+1], input[32 + (i*4)]};
        uint1[4] sbox_out = s<sbox_in>;
        s_out2[i * 4 + 0] = sbox_out[0];
        s_out2[i * 4 + 1] = sbox_out[1];
        s_out2[i * 4 + 2] = sbox_out[2];
        s_out2[i * 4 + 3] = sbox_out[3];
    }

    uint4[4] t_s_out1;
    t_s_out1[0] = touint(s_out1[3], s_out1[2], s_out1[1], s_out1[0]);
    t_s_out1[1] = touint(s_out1[7], s_out1[6], s_out1[5], s_out1[4]);
    t_s_out1[2] = touint(s_out1[11], s_out1[10], s_out1[9], s_out1[8]);
    t_s_out1[3] = touint(s_out1[15], s_out1[14], s_out1[13], s_out1[12]);

    #t_s_out1[0] = touint(s_out1[0], s_out1[4], s_out1[8], s_out1[12]);
    #t_s_out1[1] = touint(s_out1[1], s_out1[5], s_out1[9], s_out1[13]);
    #t_s_out1[2] = touint(s_out1[2], s_out1[6], s_out1[10], s_out1[14]);
    #t_s_out1[3] = touint(s_out1[3], s_out1[7], s_out1[11], s_out1[15]);

    uint4[4] t_s_out2;
    t_s_out2[0] = touint(s_out2[3], s_out2[2], s_out2[1], s_out2[0]);
    t_s_out2[1] = touint(s_out2[7], s_out2[6], s_out2[5], s_out2[4]);
    t_s_out2[2] = touint(s_out2[11], s_out2[10], s_out2[9], s_out2[8]);
    t_s_out2[3] = touint(s_out2[15], s_out2[14], s_out2[13], s_out2[12]);

    #t_s_out2[0] = touint(s_out2[0], s_out2[4], s_out2[8], s_out2[12]);
    #t_s_out2[1] = touint(s_out2[1], s_out2[5], s_out2[9], s_out2[13]);
    #t_s_out2[2] = touint(s_out2[2], s_out2[6], s_out2[10], s_out2[14]);
    #t_s_out2[3] = touint(s_out2[3], s_out2[7], s_out2[11], s_out2[15]);

    uint4[4] t_m_out1 = M * t_s_out1;
    uint4[4] t_m_out2 = M * t_s_out2;

    uint1[16] m_out1;
    uint1[16] m_out2;
    for (i from 0 to 3) {
        m_out1[i] = t_m_out1[0][3 - i];
        m_out2[i] = t_m_out2[0][3 - i];
    }
    for (i from 0 to 3) {
        m_out1[i+4] = t_m_out1[1][3 - i];
        m_out2[i+4] = t_m_out2[1][3 - i];
    }
    for (i from 0 to 3) {
        m_out1[i+8] = t_m_out1[2][3 - i];
        m_out2[i+8] = t_m_out2[2][3 - i];
    }
    for (i from 0 to 3) {
        m_out1[i+12] = t_m_out1[3][3 - i];
        m_out2[i+12] = t_m_out2[3][3 - i];
    }

    uint1[16] s_out3;
    uint1[16] s_out4;
    for (i from 0 to 3) {
        uint1[4] sbox_in = View(m_out1, i*4, i*4+3);
        # uint1[4] sbox_in = {m_out1[i*4+3], m_out1[i*4+2], m_out1[i*4+1], m_out1[i*4]};
        uint1[4] sbox_out = s<sbox_in>;
        s_out3[i * 4 + 0] = sbox_out[0];
        s_out3[i * 4 + 1] = sbox_out[1];
        s_out3[i * 4 + 2] = sbox_out[2];
        s_out3[i * 4 + 3] = sbox_out[3];
    }
    for (i from 0 to 3) {
        uint1[4] sbox_in = View(m_out2, i*4, i*4+3);
        # uint1[4] sbox_in = {m_out2[i*4+3], m_out2[i*4+2], m_out2[i*4+1], m_out2[i*4]};
        uint1[4] sbox_out = s<sbox_in>;
        s_out4[i * 4 + 0] = sbox_out[0];
        s_out4[i * 4 + 1] = sbox_out[1];
        s_out4[i * 4 + 2] = sbox_out[2];
        s_out4[i * 4 + 3] = sbox_out[3];
    }

    uint1[64] n_input = input;
    for (i from 0 to 15) {
        n_input[i + 16] = s_out3[i] ^ input[i + 16] ^ key[i];
    }
    for (i from 32 to 47) {
        n_input[i + 16] = s_out4[i - 32] ^ input[i + 16] ^ key[i - 16];
    }
    uint1[64] p_out = p<n_input>;
    return p_out;
}

r_fn uint1[64] round_function3(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[16] s_out1;
    uint1[16] s_out2;
    for (i from 0 to 3) {
        uint1[4] sbox_in = View(input, i*4, i*4+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out1[i * 4 + 0] = sbox_out[0];
        s_out1[i * 4 + 1] = sbox_out[1];
        s_out1[i * 4 + 2] = sbox_out[2];
        s_out1[i * 4 + 3] = sbox_out[3];
    }
    for (i from 0 to 3) {
        uint1[4] sbox_in = View(input, 32 + (i*4), 32 + (i*4)+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out2[i * 4 + 0] = sbox_out[0];
        s_out2[i * 4 + 1] = sbox_out[1];
        s_out2[i * 4 + 2] = sbox_out[2];
        s_out2[i * 4 + 3] = sbox_out[3];
    }

    uint4[4] t_s_out1;
    t_s_out1[0] = touint(s_out1[3], s_out1[2], s_out1[1], s_out1[0]);
    t_s_out1[1] = touint(s_out1[7], s_out1[6], s_out1[5], s_out1[4]);
    t_s_out1[2] = touint(s_out1[11], s_out1[10], s_out1[9], s_out1[8]);
    t_s_out1[3] = touint(s_out1[15], s_out1[14], s_out1[13], s_out1[12]);
    uint4[4] t_s_out2;
    t_s_out2[0] = touint(s_out2[3], s_out2[2], s_out2[1], s_out2[0]);
    t_s_out2[1] = touint(s_out2[7], s_out2[6], s_out2[5], s_out2[4]);
    t_s_out2[2] = touint(s_out2[11], s_out2[10], s_out2[9], s_out2[8]);
    t_s_out2[3] = touint(s_out2[15], s_out2[14], s_out2[13], s_out2[12]);

    uint4[4] t_m_out1 = M * t_s_out1;
    uint4[4] t_m_out2 = M * t_s_out2;

    uint1[16] m_out1;
    uint1[16] m_out2;
    for (i from 0 to 3) {
        for (j from 0 to 3) {
            m_out1[i * 4 + j] = t_m_out1[i][3 - j];
            m_out2[i * 4 + j] = t_m_out2[i][3 - j];
        }
    }

    uint1[16] s_out3;
    uint1[16] s_out4;
    for (i from 0 to 3) {
        uint1[4] sbox_in = View(m_out1, i*4, i*4+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out3[i * 4 + 0] = sbox_out[0];
        s_out3[i * 4 + 1] = sbox_out[1];
        s_out3[i * 4 + 2] = sbox_out[2];
        s_out3[i * 4 + 3] = sbox_out[3];
    }
    for (i from 0 to 3) {
        uint1[4] sbox_in = View(m_out2, i*4, i*4+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out4[i * 4 + 0] = sbox_out[0];
        s_out4[i * 4 + 1] = sbox_out[1];
        s_out4[i * 4 + 2] = sbox_out[2];
        s_out4[i * 4 + 3] = sbox_out[3];
    }

    uint1[64] n_input = input;
    for (i from 16 to 31) {
        n_input[i] = s_out3[i - 16] ^ input[i] ^ key[i - 16];
    }
    for (i from 48 to 63) {
        n_input[i] = s_out4[i - 48] ^ input[i] ^ key[i - 32];
    }
    return n_input;
}

fn uint1[64] enc(uint1[864] key, uint1[64] r_plaintext){
    #r_plaintext = round_function1(1, View(key, 0, 32), r_plaintext);
    for (i from 1 to 25) {
        r_plaintext = round_function2(i, View(key, (i - 1) * 32, i * 32 - 1), r_plaintext);
    }
    r_plaintext = round_function3(26, View(key, 800, 831), r_plaintext);
    r_plaintext = round_function1(27, View(key, 832, 863), r_plaintext);
    return r_plaintext;
}