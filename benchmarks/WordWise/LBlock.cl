@cipher LBlock_word_wise

sbox uint4[16] s1 = {13, 10, 15, 0, 14, 4, 9, 11, 2, 1, 8, 3, 7, 5, 12, 6};
sbox uint4[16] s2 = {11, 9, 4, 14, 0, 15, 10, 13, 6, 12, 5, 7, 3, 8, 1, 2};
sbox uint4[16] s3 = {2, 13, 11, 12, 15, 14, 0, 9, 7, 10, 6, 3, 1, 8, 4, 5};
sbox uint4[16] s4 = {14, 5, 15, 0, 7, 2, 12, 13, 1, 8, 4, 9, 11, 10, 6, 3};
sbox uint4[16] s5 = {7, 6, 8, 11, 0, 15, 3, 14, 9, 10, 12, 13, 5, 2, 4, 1};
sbox uint4[16] s6 = {1, 14, 7, 12, 15, 13, 0, 6, 11, 5, 9, 3, 2, 4, 8, 10};
sbox uint4[16] s7 = {4, 11, 14, 9, 15, 13, 0, 10, 7, 12, 5, 6, 2, 8, 1, 3};
sbox uint4[16] s8 = {14, 9, 15, 0, 13, 4, 10, 11, 1, 2, 8, 3, 7, 6, 12, 5};

pbox uint[8] p_l = {2,0,3,1,6,4,7,5};
pbox uint[8] p_r = {2,3,4,5,6,7,0,1};

r_fn uint4[16] round_function(uint8 r, uint4[8] key, uint4[16] input) {
	uint4[8] n_input;
	for (i from 0 to 7) {
	    n_input[i] = input[i] ^ key[i];
	}
    uint4[8] s_out;
    s_out[0] = s1<n_input[0]>;
    s_out[1] = s2<n_input[1]>;
    s_out[2] = s3<n_input[2]>;
    s_out[3] = s4<n_input[3]>;
    s_out[4] = s5<n_input[4]>;
    s_out[5] = s6<n_input[5]>;
    s_out[6] = s7<n_input[6]>;
    s_out[7] = s8<n_input[7]>;

    uint4[8] l_p_out = p_l<s_out>;
    uint4[8] r_p_out = p_r<View(input, 8, 15)>;
    uint4[8] x_out;
    for (i from 0 to 7) {
        x_out[i] = r_p_out[i] ^ l_p_out[i];
    }

    uint4[16] rtn;
	for (i from 0 to 7) {
        rtn[i] = x_out[i];
        rtn[8 + i] = input[i];
	}
	return rtn;
}

fn uint4[16] enc(uint4[256] key, uint4[16] r_plaintext){
    for (i from 1 to 32) {
        r_plaintext = round_function(i, View(key, (i - 1) * 8, i * 8 - 1), r_plaintext);
    }
    return r_plaintext;
}