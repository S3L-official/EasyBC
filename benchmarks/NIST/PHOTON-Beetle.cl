@cipher PHOTON256_permutation

sbox uint4[16] s = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};
pbox uint[256] p = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 32, 33, 34, 35, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 64, 65, 66, 67, 68, 69, 70, 71, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 216, 217, 218, 219, 220, 221, 222, 223, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 252, 253, 254, 255, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251};
pboxm uint4[8][8] M = {{2,  4,  2, 11,  2,  8,  5,  6},
                       	{12,  9,  8, 13,  7,  7,  5,  2},
                       	{ 4,  4, 13, 13,  9,  4, 13,  9},
                       	{ 1,  6,  5,  1, 12, 13, 15, 14},
                       	{15, 12,  9, 13, 14,  5, 14, 13},
                       	{ 9, 14,  5, 15,  4, 12,  9, 6},
                       	{12,  2,  2, 10,  3,  1,  1, 14},
                       	{15,  1, 13, 10,  5, 10,  2, 3}};
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

uint4[12] rc = {1, 3, 7, 14, 13, 11, 6, 12, 9, 2, 5, 10};
uint4[8] ic = {0, 1, 3, 7, 15, 14, 12, 8};

r_fn uint1[256] round_function(uint8 r, uint1[32] key, uint1[256] input) {
    #input[0] = input[0] ^ rc[r - 1][0] ^ ic[0][0];
    #input[1] = input[1] ^ rc[r - 1][1] ^ ic[0][1];
    #input[2] = input[2] ^ rc[r - 1][2] ^ ic[0][2];
    #input[3] = input[3] ^ rc[r - 1][3] ^ ic[0][3];

    #input[32] = input[32] ^ rc[r - 1][0] ^ ic[1][0];
    #input[33] = input[33] ^ rc[r - 1][1] ^ ic[1][1];
    #input[34] = input[34] ^ rc[r - 1][2] ^ ic[1][2];
    #input[35] = input[35] ^ rc[r - 1][3] ^ ic[1][3];

    #input[64] = input[64] ^ rc[r - 1][0] ^ ic[2][0];
    #input[65] = input[65] ^ rc[r - 1][1] ^ ic[2][1];
    #input[66] = input[66] ^ rc[r - 1][2] ^ ic[2][2];
    #input[67] = input[67] ^ rc[r - 1][3] ^ ic[2][3];

    #input[96] = input[96] ^ rc[r - 1][0] ^ ic[3][0];
    #input[97] = input[97] ^ rc[r - 1][1] ^ ic[3][1];
    #input[98] = input[98] ^ rc[r - 1][2] ^ ic[3][2];
    #input[99] = input[99] ^ rc[r - 1][3] ^ ic[3][3];

    #input[128] = input[128] ^ rc[r - 1][0] ^ ic[4][0];
    #input[129] = input[129] ^ rc[r - 1][1] ^ ic[4][1];
    #input[130] = input[130] ^ rc[r - 1][2] ^ ic[4][2];
    #input[131] = input[131] ^ rc[r - 1][3] ^ ic[4][3];

    #input[160] = input[160] ^ rc[r - 1][0] ^ ic[5][0];
    #input[161] = input[161] ^ rc[r - 1][1] ^ ic[5][1];
    #input[162] = input[162] ^ rc[r - 1][2] ^ ic[5][2];
    #input[163] = input[163] ^ rc[r - 1][3] ^ ic[5][3];

    #input[192] = input[192] ^ rc[r - 1][0] ^ ic[6][0];
    #input[193] = input[193] ^ rc[r - 1][1] ^ ic[6][1];
    #input[194] = input[194] ^ rc[r - 1][2] ^ ic[6][2];
    #input[195] = input[195] ^ rc[r - 1][3] ^ ic[6][3];

    #input[224] = input[224] ^ rc[r - 1][0] ^ ic[7][0];
    #input[225] = input[225] ^ rc[r - 1][1] ^ ic[7][1];
    #input[226] = input[226] ^ rc[r - 1][2] ^ ic[7][2];
    #input[227] = input[227] ^ rc[r - 1][3] ^ ic[7][3];

    uint1[256] s_out;
	for (i from 0 to 63) {
	    uint1[4] temp = View(input, i*4, i*4+3);
        uint1[4] sbox_out = s<temp>;
        s_out[i * 4 + 0] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
	}

    uint1[256] p_out = p<s_out>;

    uint1[256] m_out;
    uint4[8] tl;
    for (i from 0 to 7) {
        tl[i] = touint(p_out[32 * i + 3],p_out[32 * i + 2],p_out[32 * i + 1],p_out[32 * i + 0]);
    }
    uint4[8] tl_out = M * tl;
    uint4[8] tl2;
    for (i from 0 to 7) {
        tl2[i] = touint(p_out[32 * i + 7],p_out[32 * i + 6],p_out[32 * i + 5],p_out[32 * i + 4]);
    }
    uint4[8] tl_out2 = M * tl2;
    uint4[8] tl3;
    for (i from 0 to 7) {
        tl3[i] = touint(p_out[32 * i + 11],p_out[32 * i + 10],p_out[32 * i + 9],p_out[32 * i + 8]);
    }
    uint4[8] tl_out3 = M * tl3;
    uint4[8] tl4;
    for (i from 0 to 7) {
        tl4[i] = touint(p_out[32 * i + 15],p_out[32 * i + 14],p_out[32 * i + 13],p_out[32 * i + 12]);
    }
    uint4[8] tl_out4 = M * tl4;
    uint4[8] tl5;
    for (i from 0 to 7) {
        tl5[i] = touint(p_out[32 * i + 19],p_out[32 * i + 18],p_out[32 * i + 17],p_out[32 * i + 16]);
    }
    uint4[8] tl_out5 = M * tl5;
    uint4[8] tl6;
    for (i from 0 to 7) {
        tl6[i] = touint(p_out[32 * i + 23],p_out[32 * i + 22],p_out[32 * i + 21],p_out[32 * i + 20]);
    }
    uint4[8] tl_out6 = M * tl6;
    uint4[8] tl7;
    for (i from 0 to 7) {
        tl7[i] = touint(p_out[32 * i + 27],p_out[32 * i + 26],p_out[32 * i + 25],p_out[32 * i + 24]);
    }
    uint4[8] tl_out7 = M * tl7;
    uint4[8] tl8;
    for (i from 0 to 7) {
        tl8[i] = touint(p_out[32 * i + 31],p_out[32 * i + 30],p_out[32 * i + 29],p_out[32 * i + 28]);
    }
    uint4[8] tl_out8 = M * tl8;

    for (i from 0 to 3) {
        m_out[i] = tl_out[0][i];
        m_out[32+i] = tl_out[1][i];
        m_out[64+i] = tl_out[2][i];
        m_out[96+i] = tl_out[3][i];
        m_out[128+i] = tl_out[4][i];
        m_out[160+i] = tl_out[5][i];
        m_out[192+i] = tl_out[6][i];
        m_out[224+i] = tl_out[7][i];

        m_out[4+i] = tl_out2[0][i];
        m_out[36+i] = tl_out2[1][i];
        m_out[68+i] = tl_out2[2][i];
        m_out[100+i] = tl_out2[3][i];
        m_out[132+i] = tl_out2[4][i];
        m_out[164+i] = tl_out2[5][i];
        m_out[196+i] = tl_out2[6][i];
        m_out[228+i] = tl_out2[7][i];

        m_out[8+i] = tl_out3[0][i];
        m_out[40+i] = tl_out3[1][i];
        m_out[72+i] = tl_out3[2][i];
        m_out[104+i] = tl_out3[3][i];
        m_out[136+i] = tl_out3[4][i];
        m_out[168+i] = tl_out3[5][i];
        m_out[200+i] = tl_out3[6][i];
        m_out[232+i] = tl_out3[7][i];

        m_out[12+i] = tl_out4[0][i];
        m_out[44+i] = tl_out4[1][i];
        m_out[76+i] = tl_out4[2][i];
        m_out[108+i] = tl_out4[3][i];
        m_out[140+i] = tl_out4[4][i];
        m_out[172+i] = tl_out4[5][i];
        m_out[204+i] = tl_out4[6][i];
        m_out[236+i] = tl_out4[7][i];

        m_out[16+i] = tl_out5[0][i];
        m_out[48+i] = tl_out5[1][i];
        m_out[80+i] = tl_out5[2][i];
        m_out[112+i] = tl_out5[3][i];
        m_out[144+i] = tl_out5[4][i];
        m_out[176+i] = tl_out5[5][i];
        m_out[208+i] = tl_out5[6][i];
        m_out[240+i] = tl_out5[7][i];

        m_out[20+i] = tl_out6[0][i];
        m_out[52+i] = tl_out6[1][i];
        m_out[84+i] = tl_out6[2][i];
        m_out[116+i] = tl_out6[3][i];
        m_out[148+i] = tl_out6[4][i];
        m_out[180+i] = tl_out6[5][i];
        m_out[212+i] = tl_out6[6][i];
        m_out[244+i] = tl_out6[7][i];

        m_out[24+i] = tl_out7[0][i];
        m_out[56+i] = tl_out7[1][i];
        m_out[88+i] = tl_out7[2][i];
        m_out[120+i] = tl_out7[3][i];
        m_out[152+i] = tl_out7[4][i];
        m_out[184+i] = tl_out7[5][i];
        m_out[216+i] = tl_out7[6][i];
        m_out[248+i] = tl_out7[7][i];

        m_out[28+i] = tl_out8[0][i];
        m_out[60+i] = tl_out8[1][i];
        m_out[92+i] = tl_out8[2][i];
        m_out[124+i] = tl_out8[3][i];
        m_out[156+i] = tl_out8[4][i];
        m_out[188+i] = tl_out8[5][i];
        m_out[220+i] = tl_out8[6][i];
        m_out[252+i] = tl_out8[7][i];
    }
	return m_out;
}

fn uint1[256] enc(uint1[384] key, uint1[256] r_plaintext){
    for (i from 1 to 12) {
        r_plaintext = round_function(i, View(key, (i - 1) * 32, i * 32 - 1), r_plaintext);
    }
    return r_plaintext;
}