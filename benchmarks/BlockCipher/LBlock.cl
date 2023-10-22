# the implementation's XOR operation subjects are uint1 arrays other than bits
@cipher LBlock

sbox uint4[16] s1 = {13,10,15,0,14,4,9,11,2,1,8,3,7,5,12,6};
sbox uint4[16] s2 = {11,9,4,14,0,15,10,13,6,12,5,7,3,8,1,2};
sbox uint4[16] s3 = {2,13,11,12,15,14,0,9,7,10,6,3,1,8,4,5};
sbox uint4[16] s4 = {14,5,15,0,7,2,12,13,1,8,4,9,11,10,6,3};
sbox uint4[16] s5 = {7,6,8,11,0,15,3,14,9,10,12,13,5,2,4,1};
sbox uint4[16] s6 = {1,14,7,12,15,13,0,6,11,5,9,3,2,4,8,10};
sbox uint4[16] s7 = {4,11,14,9,15,13,0,10,7,12,5,6,2,8,1,3};
sbox uint4[16] s8 = {14,9,15,0,13,4,10,11,1,2,8,3,7,6,12,5};

pbox uint[32] p_r = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0,1,2,3,4,5,6,7};
pbox uint[32] p_l = {4, 5, 6, 7, 12, 13, 14, 15, 0, 1, 2, 3, 8, 9, 10, 11, 20, 21, 22, 23, 28, 29, 30, 31, 16, 17, 18, 19, 24, 25, 26, 27};

r_fn uint1[64] round_function(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[32] kk = View(input, 0, 31);
	uint1[32] n_input = kk ^ key;

    uint1[32] s_out;
    uint1[4] a = View(n_input, 0, 3);
    uint1[4] sbox_out1 = s1<a>;
    uint1[4] b = View(n_input, 4, 7);
    uint1[4] sbox_out2 = s2<b>;
    uint1[4] c = View(n_input, 8, 11);
    uint1[4] sbox_out3 = s3<c>;
    uint1[4] d = View(n_input, 12, 15);
    uint1[4] sbox_out4 = s4<d>;
    uint1[4] e = View(n_input, 16, 19);
    uint1[4] sbox_out5 = s5<e>;
    uint1[4] f = View(n_input, 20, 23);
    uint1[4] sbox_out6 = s6<f>;
    uint1[4] g = View(n_input, 24, 27);
    uint1[4] sbox_out7 = s7<g>;
    uint1[4] h = View(n_input, 28, 31);
    uint1[4] sbox_out8 = s8<h>;

    for (i from 0 to 3) {
        s_out[i] = sbox_out1[i];
        s_out[i + 4] = sbox_out2[i];
        s_out[i + 8] = sbox_out3[i];
        s_out[i + 12] = sbox_out4[i];
        s_out[i + 16] = sbox_out5[i];
        s_out[i + 20] = sbox_out6[i];
        s_out[i + 24] = sbox_out7[i];
        s_out[i + 28] = sbox_out8[i];
    }
    uint1[32] l_p_out = p_l<s_out>;
    uint1[32] r_p_out = p_r<View(input, 32, 63)>;
    uint1[32] x_out = l_p_out ^ r_p_out;

    uint1[64] rtn;
	for (i from 0 to 31) {
        rtn[i] = x_out[i];
        rtn[32 + i] = input[i];
	}
	return rtn;
}

r_fn uint1[64] round_function2(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[32] kk = View(input, 0, 31);
	uint1[32] n_input = kk ^ key;

    uint1[32] s_out;
    uint1[4] a = View(n_input, 0, 3);
    uint1[4] sbox_out1 = s1<a>;
    uint1[4] b = View(n_input, 4, 7);
    uint1[4] sbox_out2 = s2<b>;
    uint1[4] c = View(n_input, 8, 11);
    uint1[4] sbox_out3 = s3<c>;
    uint1[4] d = View(n_input, 12, 15);
    uint1[4] sbox_out4 = s4<d>;
    uint1[4] e = View(n_input, 16, 19);
    uint1[4] sbox_out5 = s5<e>;
    uint1[4] f = View(n_input, 20, 23);
    uint1[4] sbox_out6 = s6<f>;
    uint1[4] g = View(n_input, 24, 27);
    uint1[4] sbox_out7 = s7<g>;
    uint1[4] h = View(n_input, 28, 31);
    uint1[4] sbox_out8 = s8<h>;

    for (i from 0 to 3) {
        s_out[i] = sbox_out1[i];
        s_out[i + 4] = sbox_out2[i];
        s_out[i + 8] = sbox_out3[i];
        s_out[i + 12] = sbox_out4[i];
        s_out[i + 16] = sbox_out5[i];
        s_out[i + 20] = sbox_out6[i];
        s_out[i + 24] = sbox_out7[i];
        s_out[i + 28] = sbox_out8[i];
    }
    uint1[32] l_p_out = p_l<s_out>;
    uint1[32] r_p_out = p_r<View(input, 32, 63)>;
    uint1[32] x_out = l_p_out ^ r_p_out;

    uint1[64] rtn;
	for (i from 0 to 31) {
        rtn[i] = input[i];
        rtn[32 + i] = x_out[i];
	}
	return rtn;
}

fn uint1[64] enc(uint1[1024] key, uint1[64] r_plaintext){
    for (i from 1 to 31) {
        r_plaintext = round_function(i, View(key, (i - 1) * 32, i * 32 - 1), r_plaintext);
    }
    r_plaintext = round_function2(i, View(key, 992, 1023), r_plaintext);
    return r_plaintext;
}