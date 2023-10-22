# the implementation without "touint" function,
# and the XOR operation subjects are uint1 arrays other than bits
@cipher MIBS_64

sbox uint4[16] s = {4,15,3,8,13,10,12,0,11,5,7,14,2,6,1,9};
pbox uint[32] l_p = {4, 5, 6, 7, 28, 29, 30, 31, 0, 1, 2, 3, 8, 9, 10, 11, 20, 21, 22, 23, 24, 25, 26, 27, 12, 13, 14, 15, 16, 17, 18, 19};

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

    uint1[32] x_outf;
    for (i from 0 to 3) {
        x_outf[0+i] = s_out[1*4+i] ^ s_out[2*4+i] ^ s_out[3*4+i] ^ s_out[4*4+i] ^ s_out[5*4+i] ^ s_out[6*4+i];
        x_outf[1*4+i] = s_out[0*4+i] ^ s_out[2*4+i] ^ s_out[3*4+i] ^ s_out[5*4+i] ^ s_out[6*4+i] ^ s_out[7*4+i];
        x_outf[2*4+i] = s_out[0*4+i] ^ s_out[1*4+i] ^ s_out[3*4+i] ^ s_out[4*4+i] ^ s_out[6*4+i] ^ s_out[7*4+i];
        x_outf[3*4+i] = s_out[0*4+i] ^ s_out[1*4+i] ^ s_out[2*4+i] ^ s_out[4*4+i] ^ s_out[5*4+i] ^ s_out[7*4+i];

        x_outf[4*4+i] = s_out[0*4+i] ^ s_out[1*4+i] ^ s_out[3*4+i] ^ s_out[4*4+i] ^ s_out[5*4+i];
        x_outf[5*4+i] = s_out[0*4+i] ^ s_out[1*4+i] ^ s_out[2*4+i] ^ s_out[5*4+i] ^ s_out[6*4+i];
        x_outf[6*4+i] = s_out[1*4+i] ^ s_out[2*4+i] ^ s_out[3*4+i] ^ s_out[6*4+i] ^ s_out[7*4+i];
        x_outf[7*4+i] = s_out[0*4+i] ^ s_out[2*4+i] ^ s_out[3*4+i] ^ s_out[4*4+i] ^ s_out[7*4+i];
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