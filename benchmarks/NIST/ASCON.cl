@cipher Ascon_128_permutation_p12

sbox uint6[32] s = {4,11,31,20,26,21,9,2,27,5,8,18,29,3,6,28,30,19,7,14,0,13,17,24,16,12,1,25,22,10,15,23};

uint8[12] rc = {240,225,210,195,180,165,150,135,120,105,90,75};

r_fn uint1[320] round_function(uint8 r, uint1[32] key, uint1[320] input) {
    #for (i from 0 to 7) {
    #    input[184 + i] = input[184 + i] ^ rc[r - 1][7 - i];
    #}

    uint1[320] s_out;
    for (i from 0 to 63) {
        uint6 sbox_in = touint(input[i + 256], input[i + 192], input[i + 128], input[i + 64], input[i]);
        uint6 sbox_out = s<sbox_in>;
        s_out[i] = sbox_out[0];
        s_out[i + 64] = sbox_out[1];
        s_out[i + 128] = sbox_out[2];
        s_out[i + 192] = sbox_out[3];
        s_out[i + 256] = sbox_out[4];
    }

    uint1[64] x0;
    uint1[64] x1;
    uint1[64] x2;
    uint1[64] x3;
    uint1[64] x4;
    for (i from 0 to 63) {
        x0[i] = s_out[i];
        x1[i] = s_out[i + 64];
        x2[i] = s_out[i + 128];
        x3[i] = s_out[i + 192];
        x4[i] = s_out[i + 256];
    }
    uint1[64] x0_l19 = x0 >>> 19;
    uint1[64] x0_l28 = x0 >>> 28;

    uint1[64] x1_l61 = x1 >>> 61;
    uint1[64] x1_l39 = x1 >>> 39;

    uint1[64] x2_l1 = x2 >>> 1;
    uint1[64] x2_l6 = x2 >>> 6;

    uint1[64] x3_l10 = x3 >>> 10;
    uint1[64] x3_l17 = x3 >>> 17;

    uint1[64] x4_l7 = x4 >>> 7;
    uint1[64] x4_l41 = x4 >>> 41;

    uint1[320] rtn;
    for (i from 0 to 63) {
        rtn[i] = x0[i] ^ x0_l19[i] ^ x0_l28[i];
        rtn[i + 64] = x1[i] ^ x1_l61[i] ^ x1_l39[i];
        rtn[i + 128] = x2[i] ^ x2_l1[i] ^ x2_l6[i];
        rtn[i + 192] = x3[i] ^ x3_l10[i] ^ x3_l17[i];
        rtn[i + 256] = x4[i] ^ x4_l7[i] ^ x4_l41[i];
    }
    return rtn;
}

fn uint1[320] enc(uint1[32] key, uint1[320] r_plaintext){
    for (i from 1 to 12) {
        r_plaintext = round_function(i, key, r_plaintext);
    }
    return r_plaintext;
}