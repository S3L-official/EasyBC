@cipher Elephant_Dumbo_permutation

sbox uint4[16] s = {14, 13, 11, 0, 2, 1, 4, 15, 7, 10, 8, 5, 9, 12, 3, 6};
pbox uint[160] p = {0,40,80,120,1,41,81,121,2,42,82,122,3,43,83,123,4,44,84,124,5,45,85,125,6,46,86,126,7,47,87,127,8,48,88,128,9,49,89,129,10,50,90,130,11,51,91,131,12,52,92,132,13,53,93,133,14,54,94,134,15,55,95,135,16,56,96,136,17,57,97,137,18,58,98,138,19,59,99,139,20,60,100,140,21,61,101,141,22,62,102,142,23,63,103,143,24,64,104,144,25,65,105,145,26,66,106,146,27,67,107,147,28,68,108,148,29,69,109,149,30,70,110,150,31,71,111,151,32,72,112,152,33,73,113,153,34,74,114,154,35,75,115,155,36,76,116,156,37,77,117,157,38,78,118,158,39,79,119,159};

r_fn uint1[160] round_function(uint8 r, uint1[32] key, uint1[160] input) {
    uint1[160] icounter_out;
    for (i from 0 to 159) {
        icounter_out[i] = 0;
    }
    icounter_out[153] = r[6];
    icounter_out[154] = r[5];
    icounter_out[159] = r[0];

    uint1[160] ricounter_out;
    for (i from 0 to 159) {
        ricounter_out[i] = icounter_out[159 - i];
    }

    uint1[160] c_out;
    for (i from 0 to 159) {
        c_out[i] = input[i] ^ icounter_out[i] ^ ricounter_out[i];
    }

    uint1[160] s_out;
    for (i from 0 to 39) {
        uint1[4] sbox_in = View(c_out, i*4, i*4+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i * 4 + 0] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
    }
    uint1[160] p_out = p<s_out>;
    return p_out;
}

fn uint1[160] enc(uint1[32] key, uint1[160] r_plaintext){
    for (i from 1 to 80) {
        r_plaintext = round_function(i, key, r_plaintext);
    }
    return r_plaintext;
}