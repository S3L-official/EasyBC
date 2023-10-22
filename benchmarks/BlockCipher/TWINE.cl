# the implementation without "touint" function,
# and the XOR operation subjects are uint1 arrays other than bits
@cipher TWINE_64

sbox uint4[16] s = {12, 0, 15, 10, 2, 11, 9, 5, 8, 3, 13, 7, 1, 14, 6, 4};
pbox uint[64] p = {20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 28, 29, 30, 31, 48, 49, 50, 51, 12, 13, 14, 15, 32, 33, 34, 35, 52, 53, 54, 55, 24, 25, 26, 27, 36, 37, 38, 39, 8, 9, 10, 11, 60, 61, 62, 63, 40, 41, 42, 43, 44, 45, 46, 47, 56, 57, 58, 59};

r_fn uint1[64] round_function1(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[64] n_input;
    for (i from 0 to 7) {
        for (j from 0 to 3) {
            n_input[i * 8 + j] = input[i * 8 + j] ^ key[i * 4 + j];
            n_input[i * 8 + j + 4] = input[i * 8 + j + 4];
        }
    }
    uint1[32] s_out;
    for (i from 0 to 7) {
        uint1[4] sbox_in = View(n_input, i*8, i*8+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i * 4 + 0] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
    }

    for (i from 0 to 7) {
        for (j from 0 to 3) {
            n_input[i * 8 + j + 4] = s_out[i * 4 + j] ^ n_input[i * 8 + j + 4];
            n_input[i * 8 + j] = input[i * 8 + j];
        }
    }
    uint1[64] p_out = p<n_input>;
	return p_out;
}

r_fn uint1[64] round_function2(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[64] n_input;
    for (i from 0 to 7) {
        for (j from 0 to 3) {
            n_input[i * 8 + j] = input[i * 8 + j] ^ key[i * 4 + j];
            n_input[i * 8 + j + 4] = input[i * 8 + j + 4];
        }
    }
    uint1[32] s_out;
    for (i from 0 to 7) {
        uint1[4] sbox_in = View(n_input, i*8, i*8+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i * 4 + 0] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
    }

    for (i from 0 to 7) {
        for (j from 0 to 3) {
            n_input[i * 8 + j + 4] = s_out[i * 4 + j] ^ n_input[i * 8 + j + 4];
            n_input[i * 8 + j] = input[i * 8 + j];
        }
    }
    return n_input;
}

fn uint1[64] enc(uint1[1152] key, uint1[64] r_plaintext){
    for (i from 1 to 35) {
        r_plaintext = round_function1(i, View(key, (i - 1) * 32, i * 32 - 1), r_plaintext);
    }
    r_plaintext = round_function2(36, View(key, 1120, 1151), r_plaintext);
    return r_plaintext;
}