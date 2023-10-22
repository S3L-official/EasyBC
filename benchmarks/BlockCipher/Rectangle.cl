# the implementation without "touint" function,
# and the XOR operation subjects are uint1 arrays other than bits
@cipher Rectangle_64

sbox uint4[16] s = {6, 5, 12, 10, 1, 14, 7, 9, 11, 0, 3, 13, 8, 15, 4, 2};
#pbox uint[64] p = {13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,28,29,30,31,16,17,18,19,20,21,22,23,24,25,26,27,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,32,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63};
#pbox uint[64] p = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
#                   30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31,
#                   35, 34, 33, 32, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36,
#                   50, 49, 48, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51};
pbox uint[64] p = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                   31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                   36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 32, 33, 34, 35,
                   51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 48, 49, 50};

r_fn uint1[64] round_function1(uint8 r, uint1[64] key, uint1[64] input) {
	uint1[64] n_input = input ^ key;
    uint1[64] s_out;
	for (i from 0 to 15) {
    	# uint1 a = View(n_input, i + 48, i + 48)[0];
        # uint1 b = View(n_input, i + 32, i + 32)[0];
        # uint1 c = View(n_input, i + 16, i + 16)[0];
        # uint1 d = View(n_input, i, i)[0];
        # uint1[4] sbox_in = {a, b, c, d};

        uint1[4] sbox_in = {n_input[i], n_input[i+16], n_input[i+32], n_input[i+48]};

        # uint1[4] sbox_in = {View(n_input, i + 48, i + 48)[0], View(n_input, i + 32, i + 32)[0], View(n_input, i + 16, i + 16)[0], View(n_input, i, i)[0]};
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i] = sbox_out[0];
        s_out[i + 16] = sbox_out[1];
        s_out[i + 32] = sbox_out[2];
        s_out[i + 48] = sbox_out[3];
    }
    uint1[64] rtn = p<s_out>;
	return rtn;
}

r_fn uint1[64] round_function2(uint8 r, uint1[64] key, uint1[64] input) {
    uint1[64] rtn = input ^ key;
    return rtn;
}

fn uint1[64] enc(uint1[1664] key, uint1[64] r_plaintext){
    for (i from 1 to 25) {
        r_plaintext = round_function1(i, View(key, (i - 1) * 64, i * 64 - 1), r_plaintext);
    }
    r_plaintext = round_function2(26, View(key, 1600, 1663), r_plaintext);
    return r_plaintext;
}