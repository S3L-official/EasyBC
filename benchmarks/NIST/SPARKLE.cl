# # the permutation of Romulus : ARX-based S-box Alzette
@cipher Alzette

pbox uint[32] p1 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0};
pbox uint[32] p2 = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0,1,2,3,4,5,6,7};
pbox uint[32] p3 = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
pbox uint[32] p4 = {17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

r_fn uint1[64] round_function1(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[32] l_input = View(input, 0, 31); # x
    uint1[32] r_input = View(input, 32, 63); # y

    uint1[32] p1_out = r_input >>> 31;
    l_input = l_input + p1_out;
    uint1[32] p2_out = l_input >>> 24; # x >>> 24
    r_input = r_input ^ p2_out;
    l_input = l_input ^ key;

    uint1[32] p3_out = r_input >>> 17;
    l_input = l_input + p3_out;
    uint1[32] p4_out = l_input >>> 17;
    r_input = r_input ^ p4_out;
    l_input = l_input ^ key;

    uint1[32] p5_out = r_input;
    l_input = l_input + p5_out;
    uint1[32] p6_out = l_input >>> 31;
    r_input = r_input ^ p6_out;
    l_input = l_input ^ key;

# 10
    uint1[32] p11_out1 = r_input >>> 24;
    l_input = l_input + p11_out1;
    uint1[32] p22_out1 = l_input >>> 16; # x >>> 24
    r_input = r_input ^ p22_out1;
    l_input = l_input ^ key;

    #uint1[32] p31_out = r_input >>> 31;
    #l_input = l_input + p31_out;
    #uint1[32] p32_out = l_input >>> 24; # x >>> 24
    #r_input = r_input ^ p32_out;
    #l_input = l_input ^ key;

    #uint1[32] p43_out = r_input >>> 17;
    #l_input = l_input + p43_out;
    #uint1[32] p44_out = l_input >>> 17;
    #r_input = r_input ^ p44_out;
    #l_input = l_input ^ key;


#18
    #uint1[32] p3_out1 = r_input >>> 17;
    #l_input = l_input + p3_out1;
    #uint1[32] p4_out1 = l_input >>> 17;
    #r_input = r_input ^ p4_out1;
    #l_input = l_input ^ key;

    uint1[64] rtn;
    for (i from 0 to 31) {
        rtn[i] = l_input[i];
        rtn[i + 32] = r_input[i];
    }
    return rtn;
}

fn uint1[64] enc(uint1[32] key, uint1[64] r_plaintext){
    r_plaintext = round_function1(1, key, r_plaintext);
    return r_plaintext;
}