# the permutation of Romulus : block cipher SKINNY128
@cipher SKINNY_128_bitwise

sbox uint8[256] s = {101,76,106,66,75,99,67,107,85,117,90,122,83,115,91,123,53,140,58,129,137,51,128,59,149,37,152,42,144,35,153,43,229,204,232,193,201,224,192,233,213,245,216,248,208,240,217,249,165,28,168,18,27,160,19,169,5,181,10,184,3,176,11,185,50,136,60,133,141,52,132,61,145,34,156,44,148,36,157,45,98,74,108,69,77,100,68,109,82,114,92,124,84,116,93,125,161,26,172,21,29,164,20,173,2,177,12,188,4,180,13,189,225,200,236,197,205,228,196,237,209,241,220,252,212,244,221,253,54,142,56,130,139,48,131,57,150,38,154,40,147,32,155,41,102,78,104,65,73,96,64,105,86,118,88,120,80,112,89,121,166,30,170,17,25,163,16,171,6,182,8,186,0,179,9,187,230,206,234,194,203,227,195,235,214,246,218,250,211,243,219,251,49,138,62,134,143,55,135,63,146,33,158,46,151,39,159,47,97,72,110,70,79,103,71,111,81,113,94,126,87,119,95,127,162,24,174,22,31,167,23,175,1,178,14,190,7,183,15,191,226,202,238,198,207,231,199,239,210,242,222,254,215,247,223,255};
pbox uint[128] p = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,56,57,58,59,60,61,62,63,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,96,97,98,99,100,101,102,103};

pboxm uint1[4][4] M = {{1,0,1,1},{1,0,0,0},{0,1,1,0},{1,0,1,0}};
ffm uint1[2][2] M = {{0,0},{0,1}};

uint6[40] rc = {1,3,7,15,31,62,61,59,55,47,30,60,57,51,39,14,29,58,53,43,22,44,24,48,33,2,5,11,23,46,28,56,49,35,6,13,27,54,45,26};

r_fn uint1[128] round_function(uint8 r, uint1[64] key, uint1[128] input) {
    uint1[128] s_out;
    for (i from 0 to 15) {
        uint1[8] temp = View(input, i*8, i*8+7);
        #uint8 sbox_in = touint(temp[0], temp[1], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7]);
        #uint8 sbox_out = s<sbox_in>;
        uint1[8] sbox_out = s<temp>;
        s_out[i * 8 + 0] = sbox_out[0];
        s_out[i * 8 + 1] = sbox_out[1];
        s_out[i * 8 + 2] = sbox_out[2];
        s_out[i * 8 + 3] = sbox_out[3];
        s_out[i * 8 + 4] = sbox_out[4];
        s_out[i * 8 + 5] = sbox_out[5];
        s_out[i * 8 + 6] = sbox_out[6];
        s_out[i * 8 + 7] = sbox_out[7];
    }
    uint6 c = rc[r];
    uint8 c0 = touint(0,0,0,0,c[2], c[3], c[4], c[5]);
    uint8 c1 = touint(0,0,0,0,0,0,c[0], c[1]);
    uint8 c2 = 2;
    for (i from 0 to 7) {
        s_out[i] = s_out[i] ^ c0[7-i];
        s_out[i + 32] = s_out[i + 32] ^ c1[7-i];
        s_out[i + 64] = s_out[i + 64] ^ c2[i];
    }

    uint1[128] t_out = s_out;
    for (i from 0 to 63) {
        t_out[i] = s_out[i] ^ key[i];
    }

    uint1[128] p_out = p<t_out>;

    uint1[128] m_out;
    for (i from 0 to 31) {
        # 实际上进行矩阵乘法是每一列，所以取每一列的元素不能用View连续取四位，而是一位一位取。
        uint1[4] mtemp = {p_out[i], p_out[i + 32], p_out[i + 64], p_out[i + 96]};
        uint1[4] sm_out = M * mtemp;

        m_out[i] = sm_out[0];
        m_out[i + 32] = sm_out[1];
        m_out[i + 64] = sm_out[2];
        m_out[i + 96] = sm_out[3];
    }
    return m_out;
}

fn uint1[128] enc(uint1[2560] key, uint1[128] r_plaintext){
    for (i from 1 to 40) {
        r_plaintext = round_function(i, View(key, (i - 1) * 64, i * 64 - 1), r_plaintext);
    }
    return r_plaintext;
}
