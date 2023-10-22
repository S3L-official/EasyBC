@cipher TinyJAMBU_Keyed_Permutation

r_fn uint1[128] round_function(uint8 r, uint1[128] key, uint1[128] input) {
    uint1 temp = input[70] & input[85];
    uint8 i = r % 128;
    uint1 feedback = input[0] ^ input[47] ^ ~temp ^ input[91] ^ key[i];
    uint1[128] p_out = input <<< 1;
    p_out[127] = feedback;
	return p_out;
}

fn uint1[64] enc(uint1[128] key, uint1[128] r_plaintext){
    for (i from 1 to 1024) {
        r_plaintext = round_function(i, key, r_plaintext);
    }
    return r_plaintext;
}