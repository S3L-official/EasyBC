# the implementation without "touint" function,
# and the XOR operation subjects are uint1 arrays other than bits
@cipher MIBS_64

sbox uint4[16] s = {4,15,3,8,13,10,12,0,11,5,7,14,2,6,1,9};
#pbox uint[8] l_p = {1, 7, 0, 2, 5, 6, 3, 4};
pbox uint[32] l_p = {4, 5, 6, 7, 28, 29, 30, 31, 0, 1, 2, 3, 8, 9, 10, 11, 20, 21, 22, 23, 24, 25, 26, 27, 12, 13, 14, 15, 16, 17, 18, 19};

pbox uint[32] x1 = {4, 5, 6, 7, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3};
pbox uint[32] x2 = {8, 9, 10, 11, 8, 9, 10, 11, 4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7, 8, 9, 10, 11, 8, 9, 10, 11};
pbox uint[32] x3 = {12, 13, 14, 15, 12, 13, 14, 15, 12, 13, 14, 15, 8, 9, 10, 11, 12, 13, 14, 15, 8, 9, 10, 11, 12, 13, 14, 15, 12, 13, 14, 15};
pbox uint[32] x4 = {16, 17, 18, 19, 20, 21, 22, 23, 16, 17, 18, 19, 16, 17, 18, 19, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 16, 17, 18, 19};
pbox uint[32] x5 = {20, 21, 22, 23, 24, 25, 26, 27, 24, 25, 26, 27, 20, 21, 22, 23, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 28, 29, 30, 31};
#pbox uint[16] x6 = {24, 25, 26, 27, 28, 29, 30, 31, 28, 29, 30, 31, 28, 29, 30, 31};
# 这里实际上置换的只有16个bit，所以pbox的index应该是0到15之间。
pbox uint[16] x6 = {8, 9, 10, 11, 12, 13, 14, 15, 12, 13, 14, 15, 12, 13, 14, 15};

r_fn uint1[64] round_function(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[32] n_input_l = View(input, 0, 31);
    uint1[32] n_input_r = View(input, 32, 63);
    n_input_l = n_input_l ^ key;

    uint1[32] s_out;
    for (i from 0 to 7) {
        uint1[4] sbox_in = View(n_input_l, i*4, i*4+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i * 4 + 0] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
    }

    uint1[32] x_out1 = x1<s_out>;
    uint1[32] x_out2 = x2<s_out>;
    uint1[32] x_out3 = x3<s_out>;
    uint1[32] x_out4 = x4<s_out>;
    uint1[32] x_out5 = x5<s_out>;
    uint1[16] x_out6 = x6<View(s_out, 16, 31)>;
    uint1[32] x_outf1 = x_out1 ^ x_out2;
    uint1[32] x_outf2 = x_outf1 ^ x_out3;
    uint1[32] x_outf3 = x_outf2 ^ x_out4;
    uint1[32] x_outf = x_outf3 ^ x_out5;
    for (i from 0 to 15) {
        x_outf[i] = x_outf[i] ^ x_out6[i];
    }
    uint1[32] p_out = l_p<x_outf>;
    uint1[32] x_outc = n_input_r ^ p_out;
    uint1[64] rtn;
    for (i from 0 to 31) {
        rtn[i] = x_outc[i];
        rtn[i + 32] = input[i];
    }
	return rtn;
}

fn uint1[64] enc(uint1[1024] key, uint1[64] r_plaintext){
    for (i from 1 to 32) {
        r_plaintext = round_function(i, View(key, (i - 1) * 32, i * 32 - 1), r_plaintext);
    }
    return r_plaintext;
}