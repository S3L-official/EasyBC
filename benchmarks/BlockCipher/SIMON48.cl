@cipher SIMON_48_72

pbox uint[24] p1 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,0};
pbox uint[24] p2 = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,0,1,2,3,4,5,6,7};
pbox uint[24] p3 = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,0,1};

r_fn uint1[48] round_function(uint8 r, uint1[24] key, uint1[48] input) {
    uint1[24] l_input;
    uint1[24] r_input;
    for (i from 0 to 23) {
        l_input[i] = input[i];
        r_input[i] = input[i + 24];
    }
    uint1[24] p1_out = p1<l_input>;
    uint1[24] p2_out = p2<l_input>;
    uint1[24] p3_out = p3<l_input>;
    uint1[24] and_out;
    for (i from 0 to 23) {
        and_out[i] = p1_out[i] & p2_out[i];
    }
    uint1[24] l_out;
    for (i from 0 to 23) {
        l_out[i] = and_out[i] ^ p3_out[i] ^ r_input[i] ^ key[i];
    }
    uint1[48] rtn;
    for (i from 0 to 23) {
        rtn[i] = l_out[i];
        rtn[i + 24] = l_input[i];
    }
	return rtn;
}

fn uint1[48] enc(uint1[1008] key, uint1[48] r_plaintext) {
    for (i from 1 to 36) {
        r_plaintext = round_function(i, View(key, (i - 1) * 24, i * 24 - 1), r_plaintext);
    }
    return r_plaintext;
}