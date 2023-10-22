@cipher grain128aead

# grain的key是128位，nonce是96位，这里我们把他们统一作为key处理，为224bits
r_fn uint1[8] round_function(uint8 r, uint1[224] key, uint1[64] input) {
    # init grain
    uint1[128] lfsr;
    for (i from 0 to 95) {
        lfsr[i] = key[i + 128];
    }
    for (i from 96 to 126) {
        lfsr[i] = 1;
    }
    lfsr[127] = 0;

    uint1[128] nfsr;
    for (i from 0 to 127) {
        nfsr[i] = key[i];
    }

    uint1[32] auth_acc;
    uint1[32] auth_sr;
    for (i from 0 to 31) {
        auth_acc[i] = 0;
        auth_sr[i] = 0;
    }

    # init data
    # 这里我们不对date进行初始化，默认给定的input就是符合输入类型的data

    # next_z
    for (i from 0 to 255) {
        # grain->lfsr[96] ^ grain->lfsr[81] ^ grain->lfsr[70] ^ grain->lfsr[38] ^ grain->lfsr[7] ^ grain->lfsr[0];
        uint1 lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        # grain->nfsr[96] ^ grain->nfsr[91] ^ grain->nfsr[56] ^ grain->nfsr[26] ^ grain->nfsr[0] ^ (grain->nfsr[84] & grain->nfsr[68]) ^
        #             (grain->nfsr[67] & grain->nfsr[3]) ^ (grain->nfsr[65] & grain->nfsr[61]) ^ (grain->nfsr[59] & grain->nfsr[27]) ^
        #             (grain->nfsr[48] & grain->nfsr[40]) ^ (grain->nfsr[18] & grain->nfsr[17]) ^ (grain->nfsr[13] & grain->nfsr[11]) ^
        #             (grain->nfsr[82] & grain->nfsr[78] & grain->nfsr[70]) ^ (grain->nfsr[25] & grain->nfsr[24] & grain->nfsr[22]) ^
        #             (grain->nfsr[95] & grain->nfsr[93] & grain->nfsr[92] & grain->nfsr[88]);
        uint1 nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^
                                   (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                                   (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                                   (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                                   (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        #define x0 grain->nfsr[12]	// bi+12
        #define x1 grain->lfsr[8]		// si+8
        #define x2 grain->lfsr[13]	// si+13
        #define x3 grain->lfsr[20]	// si+20
        #define x4 grain->nfsr[95]	// bi+95
        #define x5 grain->lfsr[42]	// si+42
        #define x6 grain->lfsr[60]	// si+60
        #define x7 grain->lfsr[79]	// si+79
        #define x8 grain->lfsr[94]	// si+94
        # uint8_t h_out = (x0 & x1) ^ (x2 & x3) ^ (x4 & x5) ^ (x6 & x7) ^ (x0 & x4 & x8);
        uint1 h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        uint1 nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        uint1 y = h_out ^ lfsr[93] ^ nfsr_tmp;

        #lfsr_out = shift(grain->lfsr, lfsr_fb ^ y);
        #shift(grain->nfsr, nfsr_fb ^ lfsr_out ^ y);
        uint1 lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb ^ y;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out ^ y;
    }

    # generate_keystream
    for (i from 0 to 31) {
        # grain->lfsr[96] ^ grain->lfsr[81] ^ grain->lfsr[70] ^ grain->lfsr[38] ^ grain->lfsr[7] ^ grain->lfsr[0];
        uint1 lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        # grain->nfsr[96] ^ grain->nfsr[91] ^ grain->nfsr[56] ^ grain->nfsr[26] ^ grain->nfsr[0] ^ (grain->nfsr[84] & grain->nfsr[68]) ^
        #             (grain->nfsr[67] & grain->nfsr[3]) ^ (grain->nfsr[65] & grain->nfsr[61]) ^ (grain->nfsr[59] & grain->nfsr[27]) ^
        #             (grain->nfsr[48] & grain->nfsr[40]) ^ (grain->nfsr[18] & grain->nfsr[17]) ^ (grain->nfsr[13] & grain->nfsr[11]) ^
        #             (grain->nfsr[82] & grain->nfsr[78] & grain->nfsr[70]) ^ (grain->nfsr[25] & grain->nfsr[24] & grain->nfsr[22]) ^
        #             (grain->nfsr[95] & grain->nfsr[93] & grain->nfsr[92] & grain->nfsr[88]);
        uint1 nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^
                                   (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                                   (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                                   (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                                   (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        #define x0 grain->nfsr[12]	// bi+12
        #define x1 grain->lfsr[8]		// si+8
        #define x2 grain->lfsr[13]	// si+13
        #define x3 grain->lfsr[20]	// si+20
        #define x4 grain->nfsr[95]	// bi+95
        #define x5 grain->lfsr[42]	// si+42
        #define x6 grain->lfsr[60]	// si+60
        #define x7 grain->lfsr[79]	// si+79
        #define x8 grain->lfsr[94]	// si+94
        # uint8_t h_out = (x0 & x1) ^ (x2 & x3) ^ (x4 & x5) ^ (x6 & x7) ^ (x0 & x4 & x8);
        uint1 h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        uint1 nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        uint1 y = h_out ^ lfsr[93] ^ nfsr_tmp;

        #lfsr_out = shift(grain->lfsr, lfsr_fb ^ y);
        #shift(grain->nfsr, nfsr_fb ^ lfsr_out ^ y);
        uint1 lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        auth_acc[i] = y;
    }
    for (i from 0 to 31) {
        uint1 lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        uint1 nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                                   (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                                   (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                                   (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        #define x0 grain->nfsr[12]	// bi+12
        #define x1 grain->lfsr[8]		// si+8
        #define x2 grain->lfsr[13]	// si+13
        #define x3 grain->lfsr[20]	// si+20
        #define x4 grain->nfsr[95]	// bi+95
        #define x5 grain->lfsr[42]	// si+42
        #define x6 grain->lfsr[60]	// si+60
        #define x7 grain->lfsr[79]	// si+79
        #define x8 grain->lfsr[94]	// si+94
        # uint8_t h_out = (x0 & x1) ^ (x2 & x3) ^ (x4 & x5) ^ (x6 & x7) ^ (x0 & x4 & x8);
        uint1 h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        uint1 nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        uint1 y = h_out ^ lfsr[93] ^ nfsr_tmp;

        #lfsr_out = shift(grain->lfsr, lfsr_fb ^ y);
        #shift(grain->nfsr, nfsr_fb ^ lfsr_out ^ y);
        uint1 lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        auth_sr[i] = y;
    }


    # generate keystream line 231
    uint1[8] ks;
    uint1[8] ms;
    uint1 z_next;
    for (i from 0 to 7) {
        # 0
        uint1 lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        uint1 nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                            (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                            (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                            (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        #define x0 grain->nfsr[12]	// bi+12
        #define x1 grain->lfsr[8]		// si+8
        #define x2 grain->lfsr[13]	// si+13
        #define x3 grain->lfsr[20]	// si+20
        #define x4 grain->nfsr[95]	// bi+95
        #define x5 grain->lfsr[42]	// si+42
        #define x6 grain->lfsr[60]	// si+60
        #define x7 grain->lfsr[79]	// si+79
        #define x8 grain->lfsr[94]	// si+94
        # uint8_t h_out = (x0 & x1) ^ (x2 & x3) ^ (x4 & x5) ^ (x6 & x7) ^ (x0 & x4 & x8);
        uint1 h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        uint1 nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        uint1 y = h_out ^ lfsr[93] ^ nfsr_tmp;

        #lfsr_out = shift(grain->lfsr, lfsr_fb ^ y);
        #shift(grain->nfsr, nfsr_fb ^ lfsr_out ^ y);
        uint1 lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ks[0] = z_next;

        # 1
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                             (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                             (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                             (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ms[0] = z_next;

        auth_sr = auth_sr <<< 1;
        auth_sr[31] = z_next;

        # 2
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                             (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                             (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                             (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ks[1] = z_next;

        # 3
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                             (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                             (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                             (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ms[1] = z_next;
        auth_acc = auth_acc ^ auth_sr;
        auth_sr = auth_sr <<< 1;
        auth_sr[31] = z_next;

        # 4
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                            (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                            (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                            (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ks[2] = z_next;

        # 5
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                             (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                             (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                             (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ms[2] = z_next;
        auth_sr = auth_sr <<< 1;
        auth_sr[31] = z_next;

        # 6
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                            (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                            (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                            (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ks[3] = z_next;

        # 7
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                             (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                             (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                             (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ms[3] = z_next;
        auth_sr = auth_sr <<< 1;
        auth_sr[31] = z_next;

        # 8
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                            (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                            (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                            (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ks[4] = z_next;

        # 9
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                             (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                             (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                             (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ms[4] = z_next;
        auth_sr = auth_sr <<< 1;
        auth_sr[31] = z_next;

        # 10
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                            (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                            (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                            (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ks[5] = z_next;

        # 11
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                             (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                             (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                             (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ms[5] = z_next;
        auth_sr = auth_sr <<< 1;
        auth_sr[31] = z_next;

        # 12
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                            (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                            (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                            (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ks[6] = z_next;

        # 13
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                             (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                             (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                             (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ms[6] = z_next;
        auth_sr = auth_sr <<< 1;
        auth_sr[31] = z_next;


        # 14
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                            (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                            (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                            (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ks[7] = z_next;

        # 15
        lfsr_fb = lfsr[96] ^ lfsr[81] ^ lfsr[70] ^ lfsr[38] ^ lfsr[7] ^ lfsr[0];
        nfsr_fb = nfsr[96] ^ nfsr[91] ^ nfsr[56] ^ nfsr[26] ^ nfsr[0] ^ (nfsr[84] & nfsr[68]) ^ (nfsr[67] & nfsr[3]) ^ (nfsr[65] & nfsr[61]) ^ (nfsr[59] & nfsr[27]) ^
                             (nfsr[48] & nfsr[40]) ^ (nfsr[18] & nfsr[17]) ^ (nfsr[13] & nfsr[11]) ^
                             (nfsr[82] & nfsr[78] & nfsr[70]) ^ (nfsr[25] & nfsr[24] & nfsr[22]) ^
                             (nfsr[95] & nfsr[93] & nfsr[92] & nfsr[88]);

        h_out = (nfsr[12] & lfsr[8]) ^ (lfsr[13] & lfsr[20]) ^ (nfsr[95] & lfsr[42]) ^ (lfsr[60] & lfsr[79]) ^ (nfsr[12] & nfsr[12] & lfsr[94]);

        nfsr_tmp = 0;
        nfsr_tmp = nfsr_tmp ^ nfsr[2];
        nfsr_tmp = nfsr_tmp ^ nfsr[15];
        nfsr_tmp = nfsr_tmp ^ nfsr[36];
        nfsr_tmp = nfsr_tmp ^ nfsr[45];
        nfsr_tmp = nfsr_tmp ^ nfsr[64];
        nfsr_tmp = nfsr_tmp ^ nfsr[73];
        nfsr_tmp = nfsr_tmp ^ nfsr[89];

        y = h_out ^ lfsr[93] ^ nfsr_tmp;

        lfsr_out = lfsr[0];
        lfsr = lfsr <<< 1;
        lfsr[127] = lfsr_fb;
        nfsr = nfsr <<< 1;
        nfsr[127] = nfsr_fb ^ lfsr_out;
        z_next = y;
        ms[7] = z_next;
        auth_sr = auth_sr <<< 1;
        auth_sr[31] = z_next;
    }

    return ms;
}


fn uint1[8] enc(uint1[224] key, uint1[64] input) {
    uint1[8] rtn = round_function(1, key, input);
	return rtn;
}







