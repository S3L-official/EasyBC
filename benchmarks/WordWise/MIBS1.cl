@cipher MIBS_word_wise

sbox uint4[16] s = {4,15,3,8,13,10,12,0,11,5,7,14,2,6,1,9};
pbox uint[8] l_p = {2, 0, 3, 6, 7, 4, 5, 1};

pboxm uint4[8][8] M = {{2,  4,  2, 11,  2,  8,  5,  6},
                       	{12,  9,  8, 13,  7,  7,  5,  2},
                       	{ 4,  4, 13, 13,  9,  4, 13,  9},
                       	{ 1,  6,  5,  1, 12, 13, 15, 14},
                       	{15, 12,  9, 13, 14,  5, 14, 13},
                       	{ 9, 14,  5, 15,  4, 12,  9, 6},
                       	{12,  2,  2, 10,  3,  1,  1, 14},
                       	{15,  1, 13, 10,  5, 10,  2, 3}};
ffm uint4[16][16] M = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                    {0, 2, 4, 6, 8, 10, 12, 14, 3, 1, 7, 5, 11, 9, 15, 13},
                    {0, 3, 6, 5, 12, 15, 10, 9, 11, 8, 13, 14, 7, 4, 1, 2},
                    {0, 4, 8, 12, 3, 7, 11, 15, 6, 2, 14, 10, 5, 1, 13, 9},
                    {0, 5, 10, 15, 7, 2, 13, 8, 14, 11, 4, 1, 9, 12, 3, 6},
                    {0, 6, 12, 10, 11, 13, 7, 1, 5, 3, 9, 15, 14, 8, 2, 4},
                    {0, 6, 12, 10, 11, 13, 7, 1, 5, 3, 9, 15, 14, 8, 2, 4},
                    {0, 8, 3, 11, 6, 14, 5, 13, 12, 4, 15, 7, 10, 2, 9, 1},
                    {0, 9, 1, 8, 2, 11, 3, 10, 4, 13, 5, 12, 6, 15, 7, 14},
                    {0, 10, 7, 13, 14, 4, 9, 3, 15, 5, 8, 2, 1, 11, 6, 12},
                    {0, 11, 5, 14, 10, 1, 15, 4, 7, 12, 2, 9, 13, 6, 8, 3},
                    {0, 12, 11, 7, 5, 9, 14, 2, 10, 6, 1, 13, 15, 3, 4, 8},
                    {0, 13, 9, 4, 1, 12, 8, 5, 2, 15, 11, 6, 3, 14, 10, 7},
                    {0, 14, 15, 1, 13, 3, 2, 12, 9, 7, 6, 8, 4, 10, 11, 5},
                    {0, 15, 13, 2, 9, 6, 4, 11, 1, 14, 12, 3, 8, 7, 5, 10}};

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

    uint4[8] x_outf = M * s_out;
    uint4[8] p_out = l_p<x_outf>;
    uint4[8] x_outc;
    for (i from 0 to 7) {
        x_outc[i] = n_input_r[i] ^ x_outf[i];
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