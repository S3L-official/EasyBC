@cipher GIFT_64

sbox uint4[16] s = {1, 10, 4, 12, 6, 15, 3, 9, 2, 13, 11, 7, 5, 0, 8, 14};
pbox uint[64] p = {12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59, 8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55, 4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51, 0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63};
uint6[48] constants = {1, 3, 7, 15, 31, 62, 61, 59, 55, 47, 30, 60, 57, 51, 39, 14, 29, 58, 53, 43, 22, 44, 24, 48, 33, 2, 5, 11, 23, 46, 28, 56, 49, 35, 6, 13, 27, 54, 45, 26, 52, 41, 18, 36, 8, 17, 34, 4, 9, 19, 38, 12, 25, 50, 37, 10, 21, 42, 20, 40, 16, 32, 1};

r_fn uint1[64] round_function(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[64] s_out;
	for (i from 0 to 15) {
	    uint1[4] sbox_in = View(input, i*4, i*4+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i * 4] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
	}
    uint1[64] p_out = p<s_out>;
	for (i from 0 to 15) {
	    p_out[4 * i + 3] = p_out[4 * i + 3] ^ key[15-i];
	    p_out[4 * i + 2] = p_out[4 * i + 2] ^ key[31-i];
	}
    #p_out[63] = p_out[63] ^ 1;
    #p_out[23] = p_out[23] ^ constants[r][5];
    #p_out[19] = p_out[19] ^ constants[r][4];
    #p_out[15] = p_out[15] ^ constants[r][3];
    #p_out[11] = p_out[11] ^ constants[r][2];
    #p_out[7] = p_out[7] ^ constants[r][1];
    #p_out[3] = p_out[3] ^ constants[r][0];

	p_out[0] = p_out[0] ^ 1;
    p_out[40] = p_out[40] ^ constants[r][0];
    p_out[44] = p_out[44] ^ constants[r][1];
    p_out[48] = p_out[48] ^ constants[r][2];
    p_out[52] = p_out[52] ^ constants[r][3];
    p_out[56] = p_out[56] ^ constants[r][4];
    p_out[60] = p_out[60] ^ constants[r][5];
	return p_out;
}

fn uint1[64] enc(uint1[896] key, uint1[64] r_plaintext){
    for (i from 1 to 28) {
        r_plaintext = round_function(i, View(key, (i - 1) * 32, i * 32 - 1), r_plaintext);
    }
    return r_plaintext;
}