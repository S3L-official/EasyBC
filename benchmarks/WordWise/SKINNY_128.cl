@cipher SKINNY_128_wordwise

sbox uint8[16] s = {101,76,106,66,75,99,67,107,85,117,90,122,83,115,91,123,53,140,58,129,137,51,128,59,149,37,152,42,144,35,153,43,229,204,232,193,201,224,192,233,213,245,216,248,208,240,217,249,165,28,168,18,27,160,19,169,5,181,10,184,3,176,11,185,50,136,60,133,141,52,132,61,145,34,156,44,148,36,157,45,98,74,108,69,77,100,68,109,82,114,92,124,84,116,93,125,161,26,172,21,29,164,20,173,2,177,12,188,4,180,13,189,225,200,236,197,205,228,196,237,209,241,220,252,212,244,221,253,54,142,56,130,139,48,131,57,150,38,154,40,147,32,155,41,102,78,104,65,73,96,64,105,86,118,88,120,80,112,89,121,166,30,170,17,25,163,16,171,6,182,8,186,0,179,9,187,230,206,234,194,203,227,195,235,214,246,218,250,211,243,219,251,49,138,62,134,143,55,135,63,146,33,158,46,151,39,159,47,97,72,110,70,79,103,71,111,81,113,94,126,87,119,95,127,162,24,174,22,31,167,23,175,1,178,14,190,7,183,15,191,226,202,238,198,207,231,199,239,210,242,222,254,215,247,223,255};
pbox uint[64] p = {0, 1, 2, 3, 7, 4, 5, 6, 10, 11, 8, 9, 13, 14, 15, 12};

pboxm uint8[4][4] M = {{1,0,1,1},{1,0,0,0},{0,1,1,0},{1,0,1,0}};
ffm uint1[2][2] M = {{0,0},{0,1}};

uint6[40] rc = {1,3,7,15,31,62,61,59,55,47,30,60,57,51,39,14,29,58,53,43,22,44,24,48,33,2,5,11,23,46,28,56,49,35,6,13,27,54,45,26};

r_fn uint8[16] round_function(uint8 r, uint8[48] key, uint8[16] input) {
    uint8[16] s_out;
    for (i from 0 to 15) {
        s_out[i] = s<input[i]>;
    }

    uint6 c = rc[r];
    uint8 c0 = touint(0,0,0,0,c[0], c[1], c[2], c[3]);
    uint8 c1 = touint(0,0,0,0,0,0,c[5], c[4]);
    uint8 c2 = 2;
    #s_out[0] = s_out[0] ^ c0;
    #s_out[4] = s_out[4] ^ c1;
    #s_out[8] = s_out[8] ^ c2;

    uint8[16] t_out = s_out;
    for (i from 0 to 15) {
        t_out[i] = s_out[i] ^ key[i] ^ key[i+16] ^ key[i+32];
    }

    uint8[16] p_out = p<t_out>;

    uint8[16] m_out;
    for (i from 0 to 3) {
        uint8[4] mtemp = {input[i*4], input[i*4+1], input[i*4+2], input[i*4+3]};
        uint8[4] sm_out = M * mtemp;
        for (j from 0 to 3) {
            m_out[i*4+j] = sm_out[j];
        }
    }
    return m_out;
}

fn uint8[16] enc(uint8[1920] key, uint8[16] r_plaintext){
    for (i from 1 to 40) {
        r_plaintext = round_function(i, View(key, (i - 1) * 48, i * 48 - 1), r_plaintext);
    }
    return r_plaintext;
}
