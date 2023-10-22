@cipher PHOTON256_permutation

sbox uint4[16] s = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};

pbox uint[8] p1 = {0, 1, 2, 3, 4, 5, 6, 7};
pbox uint[8] p2 = {1, 2, 3, 4, 5, 6, 7, 0};
pbox uint[8] p3 = {2, 3, 4, 5, 6, 7, 0, 1};
pbox uint[8] p4 = {3, 4, 5, 6, 7, 0, 1, 2};
pbox uint[8] p5 = {4, 5, 6, 7, 0, 1, 2, 3};
pbox uint[8] p6 = {5, 6, 7, 0, 1, 2, 3, 4};
pbox uint[8] p7 = {6, 7, 0, 1, 2, 3, 4, 5};
pbox uint[8] p8 = {7, 0, 1, 2, 3, 4, 5, 6};

pboxm uint4[8][8] M = {{2,4,2,11,2,8,5,6},
                       	{12,9,8,13,7,7,5,2},
                       	{4,4,13,13,9,4,13,9},
                       	{1,6,5,1,12,13,15,14},
                       	{15,12,9,13,14,5,14,13},
                       	{9,14,5,15,4,12,9,6},
                       	{12,2,2,10,3,1,1,14},
                       	{15,1,13,10,5,10,2,3}};
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

uint4[12] rc = {1, 3, 7, 14, 13, 11, 6, 12, 9, 2, 5, 10};
uint4[8] ic = {0, 1, 3, 7, 15, 14, 12, 8};

r_fn uint4[64] round_function(uint8 r, uint4[8] key, uint4[64] input) {
    #for (i from 0 to 7) {
    #    input[i*8] = input[i*8] ^ rc[r-1] ^ ic[i];
    #}

    uint4[64] s_out;
	for (i from 0 to 63) {
        s_out[i] = s<input[i]>;
	}

    #uint4[64] p_out = p<s_out>;

    uint4[8] p_in1;
    uint4[8] p_in2;
    uint4[8] p_in3;
    uint4[8] p_in4;
    uint4[8] p_in5;
    uint4[8] p_in6;
    uint4[8] p_in7;
    uint4[8] p_in8;
    for (i from 0 to 7) {
        p_in1[i] = s_out[i];
        p_in2[i] = s_out[i + 8];
        p_in3[i] = s_out[i + 16];
        p_in4[i] = s_out[i + 24];
        p_in5[i] = s_out[i + 32];
        p_in6[i] = s_out[i + 40];
        p_in7[i] = s_out[i + 48];
        p_in8[i] = s_out[i + 56];
    }

    uint4[8] p_out1;
    uint4[8] p_out2;
    uint4[8] p_out3;
    uint4[8] p_out4;
    uint4[8] p_out5;
    uint4[8] p_out6;
    uint4[8] p_out7;
    uint4[8] p_out8;

    p_out1 = p1<p_in1>;
    p_out2 = p2<p_in2>;
    p_out3 = p3<p_in3>;
    p_out4 = p4<p_in4>;
    p_out5 = p5<p_in5>;
    p_out6 = p6<p_in6>;
    p_out7 = p7<p_in7>;
    p_out8 = p8<p_in8>;

    uint4[8] m_in1 = {p_out1[0], p_out2[0], p_out3[0], p_out4[0], p_out5[0], p_out6[0], p_out7[0], p_out8[0]};
    uint4[8] m_in2 = {p_out1[1], p_out2[1], p_out3[1], p_out4[1], p_out5[1], p_out6[1], p_out7[1], p_out8[1]};
    uint4[8] m_in3 = {p_out1[2], p_out2[2], p_out3[2], p_out4[2], p_out5[2], p_out6[2], p_out7[2], p_out8[2]};
    uint4[8] m_in4 = {p_out1[3], p_out2[3], p_out3[3], p_out4[3], p_out5[3], p_out6[3], p_out7[3], p_out8[3]};
    uint4[8] m_in5 = {p_out1[4], p_out2[4], p_out3[4], p_out4[4], p_out5[4], p_out6[4], p_out7[4], p_out8[4]};
    uint4[8] m_in6 = {p_out1[5], p_out2[5], p_out3[5], p_out4[5], p_out5[5], p_out6[5], p_out7[5], p_out8[5]};
    uint4[8] m_in7 = {p_out1[6], p_out2[6], p_out3[6], p_out4[6], p_out5[6], p_out6[6], p_out7[6], p_out8[6]};
    uint4[8] m_in8 = {p_out1[7], p_out2[7], p_out3[7], p_out4[7], p_out5[7], p_out6[7], p_out7[7], p_out8[7]};

    uint4[8] m_out1 = M * m_in1;
    uint4[8] m_out2 = M * m_in2;
    uint4[8] m_out3 = M * m_in3;
    uint4[8] m_out4 = M * m_in4;
    uint4[8] m_out5 = M * m_in5;
    uint4[8] m_out6 = M * m_in6;
    uint4[8] m_out7 = M * m_in7;
    uint4[8] m_out8 = M * m_in8;

    uint4[64] m_out;
    for (i from 0 to 7) {
        m_out[0 + 8 * i] = m_out1[i];
        m_out[1 + 8 * i] = m_out2[i];
        m_out[2 + 8 * i] = m_out3[i];
        m_out[3 + 8 * i] = m_out4[i];
        m_out[4 + 8 * i] = m_out5[i];
        m_out[5 + 8 * i] = m_out6[i];
        m_out[6 + 8 * i] = m_out7[i];
        m_out[7 + 8 * i] = m_out8[i];
    }
	return m_out;
}

fn uint4[64] enc(uint4[96] key, uint4[64] r_plaintext){
    for (i from 1 to 12) {
        r_plaintext = round_function(i, View(key, (i - 1) * 8, i * 8 - 1), r_plaintext);
    }
    return r_plaintext;
}