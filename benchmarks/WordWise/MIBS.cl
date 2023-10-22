@cipher MIBS_word_wise

sbox uint4[16] s = {4,15,3,8,13,10,12,0,11,5,7,14,2,6,1,9};
pbox uint[8] l_p = {2, 0, 3, 6, 7, 4, 5, 1};

r_fn uint4[16] round_function(uint8 r, uint4[8] key, uint4[16] input) {
    uint4[8] n_input_l = View(input, 0, 7);
    uint4[8] n_input_r = View(input, 8, 15);
    #for (i from 0 to 7) {
    #    n_input_l[i] = n_input_l[i] ^ key[i];
    #}

    uint4[8] s_out;
    for (i from 0 to 7) {
        s_out[i] = s<n_input_l[i]>;
    }

    uint4[8] x_outf;
    x_outf[0] = s_out[1] ^ s_out[2] ^ s_out[3] ^ s_out[4] ^ s_out[5] ^ s_out[6];
    x_outf[1] = s_out[0] ^ s_out[2] ^ s_out[3] ^ s_out[5] ^ s_out[6] ^ s_out[7];
    x_outf[2] = s_out[0] ^ s_out[1] ^ s_out[3] ^ s_out[4] ^ s_out[6] ^ s_out[7];
    x_outf[3] = s_out[0] ^ s_out[1] ^ s_out[2] ^ s_out[4] ^ s_out[5] ^ s_out[7];

    x_outf[4] = s_out[0] ^ s_out[1] ^ s_out[3] ^ s_out[4] ^ s_out[5];
    x_outf[5] = s_out[0] ^ s_out[1] ^ s_out[2] ^ s_out[5] ^ s_out[6];
    x_outf[6] = s_out[1] ^ s_out[2] ^ s_out[3] ^ s_out[6] ^ s_out[7];
    x_outf[7] = s_out[0] ^ s_out[2] ^ s_out[3] ^ s_out[4] ^ s_out[7];

    uint4[8] p_out = l_p<x_outf>;
    uint4[8] x_outc;
    for (i from 0 to 7) {
        x_outc[i] = n_input_r[i] ^ p_out[i];
    }

    uint4[16] rtn;
    for (i from 0 to 7) {
        rtn[i] = x_outc[i];
        rtn[i + 8] = input[i];
    }
	return rtn;
}

fn uint4[16] enc(uint4[256] key, uint4[16] r_plaintext){
    for (i from 1 to 32) {
        r_plaintext = round_function(i, View(key, (i - 1) * 8, i * 8 - 1), r_plaintext);
    }
    return r_plaintext;
}