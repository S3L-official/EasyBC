@cipher grain128aead

# grain的key是128位，nonce是96位，这里我们把他们统一作为key处理，为224bits
r_fn uint1[64] round_function(uint8 r, uint1[64] key, uint1[64] input) {
    uint1[64] rtn;
    for (i from 0 to 63) {
        rtn[i] = 0;
    }
    for (i from 0 to 63) {
        rtn[i] = rtn[i] | (input[i] ^ key[i]);
    }
    return rtn;
}


fn uint1[64] enc(uint1[64] key, uint1[64] input) {
    uint1[64] rtn = round_function(1, key, input);
	return rtn;
}







