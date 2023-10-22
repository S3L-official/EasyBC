@cipher Keccak_p400

#uint10[5][5] rho_val = {{0, 36, 3, 105, 210}, {1, 300, 10, 45, 66}, {190, 6, 171, 15, 253},{28, 55, 153, 21, 120},{91, 276, 231, 136, 78}};

uint16[24] rc = {1,32898,32906,32768,32907,1,30897,32777,138,136,32777,10,32907,139,32905,32771,32770,128,32778,10,32897,32896,1,32776};

pbox uint[80] theta_pbox = {31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 47, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 63, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 79, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

r_fn uint1[400] round_function1(uint8 r, uint1[32] key, uint1[400] input) {
    #### theta
    uint1[400] theta_out;

    uint1[80] input0 = View(input, 0, 79); # x = (0 1 2 3 4) 0
    uint1[80] input1 = View(input, 80, 159); # x 1
    uint1[80] input2 = View(input, 160, 239); # x 2
    uint1[80] input3 = View(input, 240, 319); # x 3
    uint1[80] input4 = View(input, 320, 399); # x 4

    uint1[80] xy1z0 = input0 <<< 64; # x = (4 0 1 2 3) 0
    uint1[80] xy1z1 = input1 <<< 64; # x = (4 0 1 2 3) 1
    uint1[80] xy1z2 = input2 <<< 64; # x = (4 0 1 2 3) 2
    uint1[80] xy1z3 = input3 <<< 64; # x = (4 0 1 2 3) 3
    uint1[80] xy1z4 = input4 <<< 64; # x = (4 0 1 2 3) 4

    uint1[80] xy2z0 = theta_pbox<input0>; # x = (1 2 3 4 0) 0 z = (15, 0, 1, 2 ... , 14)
    uint1[80] xy2z1 = theta_pbox<input1>; # x = (1 2 3 4 0) 0 z = (15, 0, 1, 2 ... , 14)
    uint1[80] xy2z2 = theta_pbox<input2>;
    uint1[80] xy2z3 = theta_pbox<input3>;
    uint1[80] xy2z4 = theta_pbox<input4>;

    for (i from 0 to 79) {
        theta_out[i] = input[i] ^ xy1z0[i] ^ xy1z1[i] ^ xy1z2[i] ^ xy1z3[i] ^ xy1z4[i] ^ xy2z0[i] ^ xy2z1[i] ^ xy2z2[i] ^ xy2z3[i] ^ xy2z4[i];
    }
    for (i from 0 to 79) {
        theta_out[i + 80] = input[i + 80] ^ xy1z0[i] ^ xy1z1[i] ^ xy1z2[i] ^ xy1z3[i] ^ xy1z4[i] ^ xy2z0[i] ^ xy2z1[i] ^ xy2z2[i] ^ xy2z3[i] ^ xy2z4[i];
    }
    for (i from 0 to 79) {
        theta_out[i + 160] = input[i + 160] ^ xy1z0[i] ^ xy1z1[i] ^ xy1z2[i] ^ xy1z3[i] ^ xy1z4[i] ^ xy2z0[i] ^ xy2z1[i] ^ xy2z2[i] ^ xy2z3[i] ^ xy2z4[i];
    }
    for (i from 0 to 79) {
        theta_out[i + 240] = input[i + 240] ^ xy1z0[i] ^ xy1z1[i] ^ xy1z2[i] ^ xy1z3[i] ^ xy1z4[i] ^ xy2z0[i] ^ xy2z1[i] ^ xy2z2[i] ^ xy2z3[i] ^ xy2z4[i];
    }
    for (i from 0 to 79) {
        theta_out[i + 320] = input[i + 320] ^ xy1z0[i] ^ xy1z1[i] ^ xy1z2[i] ^ xy1z3[i] ^ xy1z4[i] ^ xy2z0[i] ^ xy2z1[i] ^ xy2z2[i] ^ xy2z3[i] ^ xy2z4[i];
    }

    #### rho
    uint1[16] theta_out0 = View(theta_out, 0, 15); # x y -> 0 0
    uint1[16] theta_out1 = View(theta_out, 16, 31); # 1 0
    uint1[16] theta_out2 = View(theta_out, 32, 47); # 2 0
    uint1[16] theta_out3 = View(theta_out, 48, 63); # 3 0
    uint1[16] theta_out4 = View(theta_out, 64, 79); # 4 0
    uint1[16] theta_out5 = View(theta_out, 80, 95); # 0 1
    uint1[16] theta_out6 = View(theta_out, 96, 111); # 1 1
    uint1[16] theta_out7 = View(theta_out, 112, 127); # 2 1
    uint1[16] theta_out8 = View(theta_out, 128, 143); # 3 1
    uint1[16] theta_out9 = View(theta_out, 144, 159); # 4 1
    uint1[16] theta_out10 = View(theta_out, 160, 175); # 0 2
    uint1[16] theta_out11 = View(theta_out, 176, 191); # 1 2
    uint1[16] theta_out12 = View(theta_out, 192, 207); # 2 2
    uint1[16] theta_out13 = View(theta_out, 208, 223); # 3 2
    uint1[16] theta_out14 = View(theta_out, 224, 239); # 4 2
    uint1[16] theta_out15 = View(theta_out, 240, 255); # 0 3
    uint1[16] theta_out16 = View(theta_out, 256, 271); # 1 3
    uint1[16] theta_out17 = View(theta_out, 272, 287); # 2 3
    uint1[16] theta_out18 = View(theta_out, 288, 303); # 3 3
    uint1[16] theta_out19 = View(theta_out, 304, 319); # 4 3
    uint1[16] theta_out20 = View(theta_out, 320, 335); # 0 4
    uint1[16] theta_out21 = View(theta_out, 336, 351); # 1 4
    uint1[16] theta_out22 = View(theta_out, 352, 367); # 2 4
    uint1[16] theta_out23 = View(theta_out, 368, 383); # 3 4
    uint1[16] theta_out24 = View(theta_out, 384, 399); # 4 4

    uint1[16] rho_out0 = theta_out0;  # 0 0
    uint1[16] rho_out1 = theta_out1 >>> 1; # 1 0
    uint1[16] rho_out2 = theta_out2 >>> 14; # 2 0
    uint1[16] rho_out3 = theta_out3 >>> 12; # 3 0
    uint1[16] rho_out4 = theta_out4 >>> 11; # 4 0
    uint1[16] rho_out5 = theta_out5 >>> 4; # 0 1
    uint1[16] rho_out6 = theta_out6 >>> 12; # 1 1
    uint1[16] rho_out7 = theta_out7 >>> 6; # 2 1
    uint1[16] rho_out8 = theta_out8 >>> 7; # 3 1
    uint1[16] rho_out9 = theta_out9 >>> 4; # 4 1
    uint1[16] rho_out10 = theta_out10 >>> 3; # 0 2
    uint1[16] rho_out11 = theta_out11 >>> 10; # 1 2
    uint1[16] rho_out12 = theta_out12 >>> 11; # 2 2
    uint1[16] rho_out13 = theta_out13 >>> 9; # 3 2
    uint1[16] rho_out14 = theta_out14 >>> 7; # 4 2
    uint1[16] rho_out15 = theta_out15 >>> 9; # 0 3
    uint1[16] rho_out16 = theta_out16 >>> 13; # 1 3
    uint1[16] rho_out17 = theta_out17 >>> 15; # 2 3
    uint1[16] rho_out18 = theta_out18 >>> 5; # 3 3
    uint1[16] rho_out19 = theta_out19 >>> 8; # 4 3
    uint1[16] rho_out20 = theta_out20 >>> 2; # 0 4
    uint1[16] rho_out21 = theta_out21 >>> 2; # 1 4
    uint1[16] rho_out22 = theta_out22 >>> 13; # 2 4
    uint1[16] rho_out23 = theta_out23 >>> 8; # 3 4
    uint1[16] rho_out24 = theta_out24 >>> 14; # 4 4

    uint1[400] rho_out;
    for (i from 0 to 15) { # 0 0
        rho_out[i] = rho_out0[i];
    }
    for (i from 16 to 31) { # 1 0
        rho_out[i] = rho_out1[i - 16];
    }
    for (i from 32 to 47) { # 2 0
        rho_out[i] = rho_out2[i - 32];
    }
    for (i from 48 to 63) { # 3 0
        rho_out[i] = rho_out3[i - 48];
    }
    for (i from 64 to 79) { # 4 0
        rho_out[i] = rho_out4[i - 64];
    }
    for (i from 80 to 95) { # 0 1
        rho_out[i] = rho_out5[i - 80];
    }
    for (i from 96 to 111) { # 1 1
        rho_out[i] = rho_out6[i - 96];
    }
    for (i from 112 to 127) { # 2 1
        rho_out[i] = rho_out7[i - 112];
    }
    for (i from 128 to 143) { # 3 1
        rho_out[i] = rho_out8[i - 128];
    }
    for (i from 144 to 159) { # 4 1
        rho_out[i] = rho_out9[i - 144];
    }
    for (i from 160 to 175) { # 0 2
        rho_out[i] = rho_out10[i - 160];
    }
    for (i from 176 to 191) { # 1 2
        rho_out[i] = rho_out11[i - 176];
    }
    for (i from 192 to 207) { # 2 2
        rho_out[i] = rho_out12[i - 192];
    }
    for (i from 208 to 223) { # 3 2
        rho_out[i] = rho_out13[i - 208];
    }
    for (i from 224 to 239) { # 4 2
        rho_out[i] = rho_out14[i - 224];
    }
    for (i from 240 to 255) { # 0 3
        rho_out[i] = rho_out15[i - 240];
    }
    for (i from 256 to 271) { # 1 3
        rho_out[i] = rho_out16[i - 256];
    }
    for (i from 272 to 287) { # 2 3
        rho_out[i] = rho_out17[i - 272];
    }
    for (i from 288 to 303) { # 3 3
        rho_out[i] = rho_out18[i - 288];
    }
    for (i from 304 to 319) { # 4 3
        rho_out[i] = rho_out19[i - 304];
    }
    for (i from 320 to 335) { # 0 4
        rho_out[i] = rho_out20[i - 320];
    }
    for (i from 336 to 351) { # 1 4
        rho_out[i] = rho_out21[i - 336];
    }
    for (i from 352 to 367) { # 2 4
        rho_out[i] = rho_out22[i - 352];
    }
    for (i from 368 to 383) { # 3 4
        rho_out[i] = rho_out23[i - 368];
    }
    for (i from 384 to 399) { # 4 4
        rho_out[i] = rho_out24[i - 384];
    }

    ### pi
    uint1[400] pi_out;
    for (i from 0 to 15) { # 0 0
        pi_out[i] = rho_out[i];
    }
    for (i from 16 to 31) { # 1 0
        pi_out[i] = rho_out[i + 80];
    }
    for (i from 32 to 47) { # 2 0
        pi_out[i] = rho_out[i + 160];
    }
    for (i from 48 to 63) { # 3 0
        pi_out[i] = rho_out[i + 240];
    }
    for (i from 64 to 79) { # 4 0
        pi_out[i] = rho_out[i + 320];
    }
    for (i from 80 to 95) { # 0 1
        pi_out[i] = rho_out[i - 32];
    }
    for (i from 96 to 111) { # 1 1
        pi_out[i] = rho_out[i + 48];
    }
    for (i from 112 to 127) { # 2 1
        pi_out[i] = rho_out[i + 48];
    }
    for (i from 128 to 143) { # 3 1
        pi_out[i] = rho_out[i + 128];
    }
    for (i from 144 to 159) { # 4 1
        pi_out[i] = rho_out[i + 208];
    }
    for (i from 160 to 175) { # 0 2
        pi_out[i] = rho_out[i - 144];
    }
    for (i from 176 to 191) { # 1 2
        pi_out[i] = rho_out[i - 64];
    }
    for (i from 192 to 207) { # 2 2
        pi_out[i] = rho_out[i + 16];
    }
    for (i from 208 to 223) { # 3 2
        pi_out[i] = rho_out[i + 96];
    }
    for (i from 224 to 239) { # 4 2
        pi_out[i] = rho_out[i + 96];
    }
    for (i from 240 to 255) { # 0 3
        pi_out[i] = rho_out[i - 176];
    }
    for (i from 256 to 271) { # 1 3
        pi_out[i] = rho_out[i - 176];
    }
    for (i from 272 to 287) { # 2 3
        pi_out[i] = rho_out[i - 96];
    }
    for (i from 288 to 303) { # 3 3
        pi_out[i] = rho_out[i - 16];
    }
    for (i from 304 to 319) { # 4 3
        pi_out[i] = rho_out[i + 64];
    }
    for (i from 320 to 335) { # 0 4
        pi_out[i] = rho_out[i - 288];
    }
    for (i from 336 to 351) { # 1 4
        pi_out[i] = rho_out[i - 208];
    }
    for (i from 352 to 367) { # 2 4
        pi_out[i] = rho_out[i - 128];
    }
    for (i from 368 to 383) { # 3 4
        pi_out[i] = rho_out[i - 128];
    }
    for (i from 384 to 399) { # 4 4
        pi_out[i] = rho_out[i - 48];
    }

    ### chi
    uint1[400] chi_out;
    for (i from 0 to 15) { # 0 x
        chi_out[i] = pi_out[i] ^ pi_out[i + 16] & pi_out[i + 32];
        chi_out[i + 80] = pi_out[i + 80] ^ pi_out[i + 96] & pi_out[i + 112];
        chi_out[i + 160] = pi_out[i + 160] ^ pi_out[i + 176] & pi_out[i + 192];
        chi_out[i + 240] = pi_out[i + 240] ^ pi_out[i + 256] & pi_out[i + 272];
        chi_out[i + 320] = pi_out[i + 320] ^ pi_out[i + 336] & pi_out[i + 352];
    }
    for (i from 16 to 31) { # 1 x
        chi_out[i] = pi_out[i] ^ pi_out[i + 16] & pi_out[i + 32];
        chi_out[i + 80] = pi_out[i + 80] ^ pi_out[i + 96] & pi_out[i + 112];
        chi_out[i + 160] = pi_out[i + 160] ^ pi_out[i + 176] & pi_out[i + 192];
        chi_out[i + 240] = pi_out[i + 240] ^ pi_out[i + 256] & pi_out[i + 272];
        chi_out[i + 320] = pi_out[i + 320] ^ pi_out[i + 336] & pi_out[i + 352];
    }
    for (i from 32 to 47) { # 2 x
        chi_out[i] = pi_out[i] ^ pi_out[i + 16] & pi_out[i + 32];
        chi_out[i + 80] = pi_out[i + 80] ^ pi_out[i + 96] & pi_out[i + 112];
        chi_out[i + 160] = pi_out[i + 160] ^ pi_out[i + 176] & pi_out[i + 192];
        chi_out[i + 240] = pi_out[i + 240] ^ pi_out[i + 256] & pi_out[i + 272];
        chi_out[i + 320] = pi_out[i + 320] ^ pi_out[i + 336] & pi_out[i + 352];
    }
    for (i from 48 to 63) { # 3 x
        chi_out[i] = pi_out[i] ^ pi_out[i + 16] & pi_out[i - 48];
        chi_out[i + 80] = pi_out[i + 80] ^ pi_out[i + 96] & pi_out[i + 32];
        chi_out[i + 160] = pi_out[i + 160] ^ pi_out[i + 176] & pi_out[i + 112];
        chi_out[i + 240] = pi_out[i + 240] ^ pi_out[i + 256] & pi_out[i + 192];
        chi_out[i + 320] = pi_out[i + 320] ^ pi_out[i + 336] & pi_out[i + 272];
    }
    for (i from 64 to 79) { # 4 0
        chi_out[i] = pi_out[i] ^ pi_out[i - 64] & pi_out[i - 48];
        chi_out[i + 80] = pi_out[i + 80] ^ pi_out[i + 16] & pi_out[i + 32];
        chi_out[i + 160] = pi_out[i + 160] ^ pi_out[i + 96] & pi_out[i + 112];
        chi_out[i + 240] = pi_out[i + 240] ^ pi_out[i + 176] & pi_out[i + 192];
        chi_out[i + 320] = pi_out[i + 320] ^ pi_out[i + 256] & pi_out[i + 272];
    }

    #for (i from 0 to 15) {
    #    chi_out[i] = chi_out[i] ^ rc[r-1][16-i];
    #}
    return chi_out;
}

fn uint1[400] enc(uint1[32] key, uint1[400] r_plaintext){
    for (i from 1 to 20) {
        r_plaintext = round_function1(i, key, r_plaintext);
    }
    return r_plaintext;
}