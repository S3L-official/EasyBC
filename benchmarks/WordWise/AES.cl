@cipher AES_128_128

sbox uint8[256] s = {99, 124, 119, 123, 242, 107, 111, 197, 48, 1, 103, 43, 254, 215, 171, 118, 202, 130, 201, 125, 250, 89, 71, 240, 173, 212, 162, 175, 156, 164, 114, 192, 183, 253, 147, 38, 54, 63, 247, 204, 52, 165, 229, 241, 113, 216, 49, 21, 4, 199, 35, 195, 24, 150, 5, 154, 7, 18, 128, 226, 235, 39, 178, 117, 9, 131, 44, 26, 27, 110, 90, 160, 82, 59, 214, 179, 41, 227, 47, 132, 83, 209, 0, 237, 32, 252, 177, 91, 106, 203, 190, 57, 74, 76, 88, 207, 208, 239, 170, 251, 67, 77, 51, 133, 69, 249, 2, 127, 80, 60, 159, 168, 81, 163, 64, 143, 146, 157, 56, 245, 188, 182, 218, 33, 16, 255, 243, 210, 205, 12, 19, 236, 95, 151, 68, 23, 196, 167, 126, 61, 100, 93, 25, 115, 96, 129, 79, 220, 34, 42, 144, 136, 70, 238, 184, 20, 222, 94, 11, 219, 224, 50, 58, 10, 73, 6, 36, 92, 194, 211, 172, 98, 145, 149, 228, 121, 231, 200, 55, 109, 141, 213, 78, 169, 108, 86, 244, 234, 101, 122, 174, 8, 186, 120, 37, 46, 28, 166, 180, 198, 232, 221, 116, 31, 75, 189, 139, 138, 112, 62, 181, 102, 72, 3, 246, 14, 97, 53, 87, 185, 134, 193, 29, 158, 225, 248, 152, 17, 105, 217, 142, 148, 155, 30, 135, 233, 206, 85, 40, 223, 140, 161, 137, 13, 191, 230, 66, 104, 65, 153, 45, 15, 176, 84, 187, 22};
pbox uint[16] p = {0,5,10,15,4,9,14,3,8,13,2,7,12,1,6,11};
pboxm uint8[4][4] M = {{2, 3, 1, 1},
                        {1, 2, 3, 1},
                        {1, 1, 2, 3},
                        {3, 1, 1, 2}};
ffm uint8[16][16] M = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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

r_fn uint8[16] round_function1(uint8 r, uint8[16] key, uint8[16] input) {
    uint8[16] rtn;
    for (i from 0 to 15) {
        rtn[i] = input[i] ^ key[i];
    }
	return rtn;
}

r_fn uint8[16] round_function2(uint8 r, uint8[16] key, uint8[16] input) {
    uint8[16] byte_s_out;
	for (i from 0 to 15) {
	    byte_s_out[i] = s<input[i]>;
	}
    uint8[16] byte_p_out = p<byte_s_out>;
    uint8[16] byte_m_out;
    for (i from 0 to 3) {
        # uint8[4] a = {byte_p_out[i], byte_p_out[i+4], byte_p_out[i+8], byte_p_out[i+12]};
        uint8[4] a = {byte_p_out[i*4], byte_p_out[i*4+1], byte_p_out[i*4+2], byte_p_out[i*4+3]};
        uint8[4] byte_t_out = M * a;
        # for (j from 0 to 3) {
        #     byte_m_out[i*4+j] = byte_t_out[j];
        # }
        byte_m_out[i*4] = byte_t_out[0];
        byte_m_out[i*4+1] = byte_t_out[1];
        byte_m_out[i*4+2] = byte_t_out[2];
        byte_m_out[i*4+3] = byte_t_out[3];
    }

    uint8[16] rtn;
    for (i from 0 to 15) {
        rtn[i] = byte_m_out[i] ^ key[i];
    }
	return rtn;
}

r_fn uint8[16] round_function3(uint8 r, uint8[16] key, uint8[16] input) {
    uint8[16] byte_s_out;
	for (i from 0 to 15) {
	    byte_s_out[i] = s<input[i]>;
	}
    uint8[16] byte_p_out = p<byte_s_out>;
    uint8[16] rtn;
    for (i from 0 to 15) {
        rtn[i] = byte_p_out[i] ^ key[i];
    }
	return rtn;
}

fn uint8[16] enc(uint8[224] key, uint8[16] r_plaintext){
    # 现在还遗留一个小问题，需要注释掉第一个轮函数的调用
    r_plaintext = round_function1(1, View(key, 0, 16), r_plaintext);
    for (i from 2 to 13) {
        r_plaintext = round_function2(i, View(key, (i - 1) * 16, i * 16 - 1), r_plaintext);
    }
    r_plaintext = round_function3(14, View(key, 208, 223), r_plaintext);
    return r_plaintext;
}