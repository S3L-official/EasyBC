@cipher SKINNY_64_tn

sbox uint4[16] s = {12, 6, 9, 0, 1, 10, 2, 11, 3, 8, 5, 13, 4, 14, 7, 15};
pbox uint[64] p = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,28,29,30,31,16,17,18,19,20,21,22,23,24,25,26,27,40,41,42,43,44,45,46,47,32,33,34,35,36,37,38,39,52,53,54,55,56,57,58,59,60,61,62,63,48,49,50,51};

pboxm uint1[4][4] M = {{1,0,1,1},{1,0,0,0},{0,1,1,0},{1,0,1,0}};
ffm uint1[2][2] M = {{0,0},{0,1}};

uint6[62] rc = {1,3,7,15,31,62,61,59,55,47,30,60,57,51,39,14,29,58,53,43,22,44,24,48,33,2,5,11,23,46,28,56,49,35,6,13,27,54,45,26,52,41,18,36,8,17,34,4,9,19,38,12,25,50,37,10,21,42,20,30,10,32};

r_fn uint1[64] round_function(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[64] s_out;
    for (i from 0 to 15) {
        uint1[4] temp = View(input, i*4, i*4+3);
        uint1[4] sbox_out = s<temp>;
        s_out[i * 4 + 0] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
    }
    #uint6 c = rc[r];
    #uint4 c0 = touint(c[2], c[3], c[4], c[5]);
    #uint4 c1 = touint(0, 0, c[0], c[1]);
    #uint4 c2 = 2;
    #for (i from 0 to 3) {
    #    s_out[i] = s_out[i] ^ c0[3-i];
    #    s_out[i + 16] = s_out[i + 16] ^ c1[3-i];
    #    s_out[i + 32] = s_out[i + 32] ^ c2[i];
    #}

    uint1[64] t_out = s_out;
    for (i from 0 to 31) {
        t_out[i] = s_out[i] ^ key[i];
    }

    uint1[64] p_out = p<t_out>;

    uint1[64] m_out;
    for (i from 0 to 15) {
        # 实际上进行矩阵乘法是每一列，所以取每一列的元素不能用View连续取四位，而是一位一位取。
        # uint1[4] mtemp = View(input, i*4, i*4+3);
        # uint1[4] mtemp = {p_out[i + 48], p_out[i + 32], p_out[i + 16], p_out[i]};
        uint1[4] mtemp = {p_out[i], p_out[i + 16], p_out[i + 32], p_out[i + 48]};
        uint1[4] sm_out = M * mtemp;
        # for (j from 0 to 3) {
        #     m_out[i*4+j] = sm_out[j];
        # }

        m_out[i] = sm_out[0];
        m_out[i + 16] = sm_out[1];
        m_out[i + 32] = sm_out[2];
        m_out[i + 48] = sm_out[3];
    }
    return m_out;
}

fn uint1[64] enc(uint1[1024] key, uint1[64] r_plaintext){
    for (i from 1 to 32) {
        r_plaintext = round_function(i, View(key, (i - 1) * 32, i * 32 - 1), r_plaintext);
    }
    return r_plaintext;
}
