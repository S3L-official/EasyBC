@cipher TWINE

sbox uint4[16] s = {12, 0, 15, 10, 2, 11, 9, 5, 8, 3, 13, 7, 1, 14, 6, 4};
pbox uint[16] p = {5,0,1,4,7,12,3,8,13,6,9,2,15,10,11,14};

r_fn uint4[16] round_function1(uint8 r, uint4[8] key, uint4[16] input) {
    uint4[16] n_input;
    for (i from 0 to 7) {
        n_input[i * 2] = input[i * 2] ^ key[i];
        n_input[i * 2 + 1] = input[i * 2 + 1];
    }

    uint4[8] s_out;
    for (i from 0 to 7) {
        s_out[i] = s<n_input[i * 2]>;
    }

    for (i from 0 to 7) {
        n_input[i * 2 + 1] = s_out[i] ^ n_input[i * 2 + 1];
        n_input[i * 2] = input[i * 2];
    }
    uint4[16] p_out = p<n_input>;
	return p_out;
}

r_fn uint4[16] round_function2(uint8 r, uint4[8] key, uint4[16] input) {
    uint4[16] n_input;
    for (i from 0 to 7) {
        n_input[i * 2] = input[i * 2] ^ key[i];
        n_input[i * 2 + 1] = input[i * 2 + 1];
    }

    uint4[8] s_out;
    for (i from 0 to 7) {
        s_out[i] = s<n_input[i * 2]>;
    }

    for (i from 0 to 7) {
        n_input[i * 2 + 1] = s_out[i] ^ n_input[i * 2 + 1];
        n_input[i * 2] = input[i * 2];
    }
    return n_input;
}

fn uint4[16] enc(uint4[288] key, uint4[16] r_plaintext){
    for (i from 1 to 35) {
        r_plaintext = round_function1(i, View(key, (i - 1) * 8, i * 8 - 1), r_plaintext);
    }
    r_plaintext = round_function2(36, View(key, 280, 287), r_plaintext);
    return r_plaintext;
}