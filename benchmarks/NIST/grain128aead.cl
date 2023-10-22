@cipher grain128aead

pbox uint[7] p = {2, 15, 36, 45, 64, 73, 89};
uint1[96] iv = {1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0};

# key -> lfsr    input -> nfsr
r_fn uint1[1] next_z(uint8 r, uint1[128] key, uint1[128] input) {
    # next_lfsr_bf
    uint1 lfsr_fb = key[96] ^ key[81] ^ key[70] ^ key[38] ^ key[7] ^ key[0];
    # next_nfsr_bf
    uint1 nfsr_fb =  input[96] ^ input[91] ^ input[56] ^ input[26] ^ input[0] ^ (input[84] & input[68]) ^
                    (input[67] & input[3]) ^ (input[65] & input[61]) ^ (input[59] & input[27]) ^
                    (input[48] & input[40]) ^ (input[18] & input[17]) ^ (input[13] & input[11]) ^
                    (input[82] & input[78] & input[70]) ^ (input[25] & input[24] & input[22]) ^
                    (input[95] & input[93] & input[92] & input[88]);
    # next_h
    uint1 h_out = (input[12] & key[8]) ^
                    (key[13] & key[20]) ^
                    (input[95] & key[42]) ^
                    (key[60] & key[79]) ^
                    (input[12] & input[95] & key[94]);
    uint1 nfsr_tmp;
    for (i from 0 to 6) {
        nfsr_tmp = nfsr_tmp ^ input[p[i]];
    }

    uint1 y = h_out ^ input[93] ^ nfsr_tmp;
    uint1[3] rtn = {y, lfsr_fb, nfsr_fb};
    return rtn;
}


# input -> initial vector
fn uint1[96] enc(uint1[128] key, uint1[256] input) {
    uint1[128] lfsr;
    uint1[128] nfsr = key;
    for (i from 0 to 95) {
        lfsr[i] = iv[i];
    }
    for (i from 96 to 127) {
        lfsr[i] = 1;
    }
    lfsr[127] = 0;

    uint1[64] auth_acc;
    uint1[64] auth_sr;
    for (i from 0 to 63) {
        auth_acc[i] = 0;
        auth_sr[i] = 0;
    }

    uint1[3] next_z1 = next_z(1, lfsr, nfsr);

    uint1 lfsr_out = lfsr[0];
    for (i from 0 to 126) {
        lfsr[i] = lfsr[i+1];
    }
    lfsr[127] = next_z1[1] ^ next_z1[0];
    for (i from 0 to 126) {
        nfsr[i] = nfsr[i+1];
    }
    nfsr[127] = next_z1[2] ^ lfsr_out ^ next_z1[0];

    for (i from 0 to 63) {
        uint1 addkey_0 = key[i];
        uint1 addkey_64 = key[64+i];
        uint1[3] next_z2 = next_z(1, lfsr, nfsr);
        uint1 lfsr_out1 = lfsr[0];
        for (j from 0 to 126) {
            lfsr[j] = lfsr[j+1];
        }
        lfsr[127] = next_z2[1] ^ next_z2[0] ^ addkey_64;
        for (j from 0 to 126) {
            nfsr[j] = nfsr[j+1];
        }
        nfsr[127] = next_z2[2] ^ lfsr_out1 ^ next_z2[0] ^ addkey_0;
    }

    for (i from 0 to 63) {
        uint1[3] next_z3 = next_z(1, lfsr, nfsr);
        uint1 lfsr_out2 = lfsr[0];
        for (j from 0 to 126) {
            lfsr[j] = lfsr[j+1];
        }
        lfsr[127] = next_z3[1] ^ next_z3[0];
        for (j from 0 to 126) {
            nfsr[j] = nfsr[j+1];
        }
        nfsr[127] = next_z3[2] ^ lfsr_out2 ^ next_z3[0];
        auth_acc[i] = next_z3[0];
    }

    for (i from 0 to 63) {
        uint1[3] next_z3 = next_z(1, lfsr, nfsr);
        uint1 lfsr_out2 = lfsr[0];
        for (j from 0 to 126) {
            lfsr[j] = lfsr[j+1];
        }
        lfsr[127] = next_z3[1] ^ next_z3[0];
        for (j from 0 to 126) {
            nfsr[j] = nfsr[j+1];
        }
        nfsr[127] = next_z3[2] ^ lfsr_out2 ^ next_z3[0];
        auth_sr[i] = next_z3[0];
    }

    uint1[256] rtn = input;
    for (i from 0 to 31) {
        for (j from 0 to 7) {
            uint1[3] next_z3 = next_z(1, lfsr, nfsr);
            rtn[32 * i + 7 - j] = next_z3[0] ^ input[i * 32 + j];
            j = j + 1;
        }
    }
	return rtn;
}





















