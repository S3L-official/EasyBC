//
// Created by Septi on 8/30/2022.
//

#ifndef EASYBC_SETUP_H
#define EASYBC_SETUP_H

#include <unordered_map>
#include "vector"

#define SBOX_LENGTH 16
#define CK_THD 16

#define DPATH "../data/"

#endif //EASYBC_SETUP_H


namespace setup{

    static std::unordered_map<std::string, int> sbox_len
            {{"AES",8}, {"Anubis", 8}, {"ARIA_s2", 8}, {"BelT", 8}, {"Camellia", 8},{"SKINNY_8", 8},
             {"Wage", 7},
             {"Fides_6", 6}, {"APN_6", 6}, {"SC2000_6", 6},
             {"Ascon", 5}, {"DryGASCON128", 5}, {"Fides_5", 5}, {"SC2000_5", 5}, {"Shamash", 5}, {"SYCON", 5},
             {"Elephant", 4}, {"KNOT", 4}, {"Pyjamask_4", 4}, {"SATURNIN_0", 4}, {"Spook", 4}, {"TRIFLE", 4}, {"SKINNY_4", 4},
             {"GIFT", 4}, {"KLEIN", 4}, {"LBlock_0", 4}, {"LBlock_1", 4}, {"LBlock_2", 4}, {"LBlock_3", 4}, {"LBlock_4", 4}, {"LBlock_5", 4},
             {"LBlock_6", 4}, {"LBlock_7", 4}, {"LBlock_8", 4}, {"LBlock_9", 4}, {"MIBS", 4}, {"Midori_Sb0", 4}, {"Midori_Sb1", 4}, {"Piccolo", 4},
             {"PRESENT", 4}, {"Pride", 4},{"PRINCE", 4},{"Rectangle", 4},{"SERPENT_S0", 4},{"SERPENT_S1", 4},{"SERPENT_S2", 4},{"SERPENT_S3", 4},
             {"SERPENT_S4", 4},{"SERPENT_S5", 4},{"SERPENT_S6", 4},{"SERPENT_S7", 4},{"TWINE", 4},
             {"SEA", 3}, {"PRINTcipher", 3}, {"Pyjamask_3", 3},
             {"test", 3}};

    static std::unordered_map<std::string, int> ddt_len
            {{"AES",256}, {"Anubis", 256}, {"ARIA_s2", 256}, {"BelT", 256}, {"Camellia", 256},{"SKINNY_8", 256},
             {"Wage", 128},
             {"Fides_6", 64}, {"APN_6", 64}, {"SC2000_6", 64},
             {"Ascon", 32}, {"DryGASCON128", 32}, {"Fides_5", 32}, {"SC2000_5", 32}, {"Shamash", 32}, {"SYCON", 32},
             {"Elephant", 16}, {"KNOT", 16}, {"Pyjamask_4", 16}, {"SATURNIN_0", 16}, {"Spook", 16}, {"TRIFLE", 16}, {"SKINNY_4", 16},
             {"GIFT", 16}, {"KLEIN", 16}, {"LBlock_0", 16}, {"LBlock_1", 16}, {"LBlock_2", 16}, {"LBlock_3", 16}, {"LBlock_4", 16}, {"LBlock_5", 16},
             {"LBlock_6", 16}, {"LBlock_7", 16}, {"LBlock_8", 16}, {"LBlock_9", 16}, {"MIBS", 16}, {"Midori_Sb0", 16}, {"Midori_Sb1", 16}, {"Piccolo", 16},
             {"PRESENT", 16}, {"Pride", 16},{"PRINCE", 16},{"Rectangle", 16},{"SERPENT_S0", 16},{"SERPENT_S1", 16},{"SERPENT_S2", 16},{"SERPENT_S3", 16},
             {"SERPENT_S4", 16},{"SERPENT_S5", 16},{"SERPENT_S6", 16},{"SERPENT_S7", 16},{"TWINE", 16},
             {"SEA", 8}, {"PRINTcipher", 8}, {"Pyjamask_3", 8},
             {"test", 8}};

    static std::unordered_map<std::string, std::vector<int>> sbox_dict
            { // 3 bits
                    {"SEA", {0, 5, 6, 7, 4, 3, 1, 2}},
                    // 4 bits
                    {"SKINNY_4", {0xc,0x6,0x9,0x0,0x1,0xa,0x2,0xb,0x3,0x8,0x5,0xd,0x4,0xe,0x7,0xf}},
                    {"GIFT", {0x1,0xa,0x4,0xc,0x6,0xf,0x3,0x9,0x2,0xd,0xb,0x7,0x5,0x0,0x8,0xe}},
                    {"Elephant", {0xE, 0xD, 0xB, 0x0, 0x2, 0x1, 0x4, 0xF, 0x7, 0xA, 0x8, 0x5, 0x9, 0xC, 0x3, 0x6}},
                    {"PRESENT", {0xC,0x5,0x6,0xB,0x9,0x0,0xA,0xD,0x3,0xE,0xF,0x8,0x4,0x7,0x1,0x2}},
                    {"KLEIN", {0x7,0x4,0xA,0x9,0x1,0xF,0xB,0x0,0xC,0x3,0x2,0x6,0x8,0xE,0xD,0x5}},
                    {"TWINE", {0xC,0x0,0xF,0xA,0x2,0xB,0x9,0x5,0x8,0x3,0xD,0x7,0x1,0xE,0x6,0x4}},
                    {"PRINCE", {0xB,0xF,0x3,0x2,0xA,0xC,0x9,0x1,0x6,0x7,0x8,0x0,0xE,0x5,0xD,0x4}},
                    {"Piccolo", {0xe,0x4,0xb,0x2,0x3,0x8,0x0,0x9,0x1,0xa,0x7,0xf,0x6,0xc,0x5,0xd}},
                    {"MIBS", {4,15,3,8,13,10,12,0,11,5,7,14,2,6,1,9}},
                    {"LBlock_0", {14, 9, 15, 0, 13, 4, 10, 11, 1, 2, 8, 3, 7, 6, 12, 5}},
                    {"LBlock_1", {4, 11, 14, 9, 15, 13, 0, 10, 7, 12, 5, 6, 2, 8, 1, 3}},
                    {"LBlock_2", {1, 14, 7, 12, 15, 13, 0, 6, 11, 5, 9, 3, 2, 4, 8, 10}},
                    {"LBlock_3", {7, 6, 8, 11, 0, 15, 3, 14, 9, 10, 12, 13, 5, 2, 4, 1}},
                    {"LBlock_4", {14, 5, 15, 0, 7, 2, 12, 13, 1, 8, 4, 9, 11, 10, 6, 3}},
                    {"LBlock_5", {2, 13, 11, 12, 15, 14, 0, 9, 7, 10, 6, 3, 1, 8, 4, 5}},
                    {"LBlock_6", {11, 9, 4, 14, 0, 15, 10, 13, 6, 12, 5, 7, 3, 8, 1, 2}},
                    {"LBlock_7", {13, 10, 15, 0, 14, 4, 9, 11, 2, 1, 8, 3, 7, 5, 12, 6}},
                    {"LBlock_8", {8, 7, 14, 5, 15, 13, 0, 6, 11, 12, 9, 10, 2, 4, 1, 3}},
                    {"LBlock_9", {11, 5, 15, 0, 7, 2, 9, 13, 4, 8, 1, 12, 14, 10, 3, 6}},
                    {"SERPENT_S0", {3,8,15,1,10,6,5,11,14,13,4,2,7,0,9,12}},
                    {"SERPENT_S1", {15,12,2,7,9,0,5,10,1,11,14,8,6,13,3,4}},
                    {"SERPENT_S2", {8,6,7,9,3,12,10,15,13,1,14,4,0,11,5,2}},
                    {"SERPENT_S3", {0,15,11,8,12,9,6,3,13,1,2,4,10,7,5,14}},
                    {"SERPENT_S4", {1,15,8,3,12,0,11,6,2,5,4,10,9,14,7,13}},
                    {"SERPENT_S5", {15,5,2,11,4,10,9,12,0,3,14,8,13,6,7,1}},
                    {"SERPENT_S6", {7,2,12,5,8,4,6,11,14,9,1,15,13,3,10,0}},
                    {"SERPENT_S7", {1,13,15,0,14,8,2,11,7,4,12,10,9,3,5,6}},
                    {"Midori_Sb0", {0xc,0xa,0xd,0x3,0xe,0xb,0xf,0x7,0x8,0x9,0x1,0x5,0x0,0x2,0x4,0x6}},
                    {"Midori_Sb1", {0x1,0x0,0x5,0x3,0xe,0x2,0xf,0x7,0xd,0xa,0x9,0xb,0xc,0x8,0x4,0x6}},
                    {"Rectangle", {0x6,0x5,0xC,0xA,0x1,0xE,0x7,0x9,0xB,0x0,0x3,0xD,0x8,0xF,0x4,0x2}},
                    {"Pride", {0x0,0x4,0x8,0xf,0x1,0x5,0xe,0x9,0x2,0x7,0xa,0xc,0xb,0xd,0x6,0x3}},
                    // 5 bits
                    {"Ascon", {0x04,0x0b,0x1f,0x14,0x1a,0x15,0x09,0x02,0x1b,0x05,0x08,0x12,0x1d,0x03,0x06,0x1c,
                                                                               0x1e,0x13,0x07,0x0e,0x00,0x0d,0x11,0x18,0x10,0x0c,0x01,0x19,0x16,0x0a,0x0f,0x17}},
                    {"DryGASCON128", {0x04, 0x0f, 0x1b, 0x01, 0x0b, 0x00, 0x17, 0x0d, 0x1f, 0x1c, 0x02, 0x10, 0x12,
                                    0x11, 0x0c, 0x1e, 0x1a, 0x19, 0x14, 0x06, 0x15, 0x16, 0x18, 0x0a, 0x05, 0x0e,
                                                                                                                                 0x09, 0x13, 0x08, 0x03, 0x07, 0x1d}},
                    {"Fides_5", {0x01,0x00,0x19,0x1a,0x11,0x1d,0x15,0x1b,0x14,0x05,0x04,0x17,0x0e,0x12,0x02,0x1c,
                                                                               0x0f,0x08,0x06,0x03,0x0d,0x07,0x18,0x10,0x1e,0x09,0x1f,0x0a,0x16,0x0c,0x0b,0x13}},
                    {"SC2000_5", {20,26,7,31,19,12,10,15,22,30,13,14,4,24,9,18,27,11,1,21,6,16,2,28,23,5,8,3,0,17,29,25}},
                    {"Shamash", {16, 14, 13, 2, 11, 17, 21, 30, 7, 24, 18, 28, 26, 1, 12, 6,
                                                                               31, 25, 0, 23, 20, 22, 8, 27, 4, 3, 19, 5, 9, 10, 29, 15}},
                    {"SYCON", {8, 19, 30, 7, 6, 25, 16, 13, 22, 15, 3, 24, 17, 12, 4, 27,
                                                                               11, 0, 29, 20, 1, 14, 23, 26, 28, 21, 9, 2, 31, 18, 10, 5}},
                    // 6 bits
                    {"APN_6", {0x0,0x36,0x30,0xd,0xf,0x12,0x35,0x23,0x19,0x3f,0x2d,0x34,0x3,0x14,0x29,0x21,
                                                                               0x3b,0x24,0x2,0x22,0xa,0x8,0x39,0x25,0x3c,0x13,0x2a,0xe,0x32,0x1a,0x3a,0x18,
                                    0x27,0x1b,0x15,0x11,0x10,0x1d,0x1,0x3e,0x2f,0x28,0x33,0x38,0x7,0x2b,0x2c,0x26,
                                    0x1f,0xb,0x4,0x1c,0x3d,0x2e,0x5,0x31,0x9,0x6,0x17,0x20,0x1e,0xc,0x37,0x16}},
                    {"Fides_6", {0x36,0x00,0x30,0x0d,0x0f,0x12,0x23,0x35,0x3f,0x19,0x2d,0x34,0x03,0x14,0x21,0x29,
                                                                               0x08,0x0a,0x39,0x25,0x3b,0x24,0x22,0x02,0x1a,0x32,0x3a,0x18,0x3c,0x13,0x0e,0x2a,
                                    0x2e,0x3d,0x05,0x31,0x1f,0x0b,0x1c,0x04,0x0c,0x1e,0x37,0x16,0x09,0x06,0x20,0x17,
                                    0x1b,0x27,0x15,0x11,0x10,0x1d,0x3e,0x01,0x28,0x2f,0x33,0x38,0x07,0x2b,0x26,0x2c}},
                    {"SC2000_6", {47,59,25,42,15,23,28,39,26,38,36,19,60,24,29,56,
                                                                               37,63,20,61,55,2,30,44,9,10,6,22,53,48,51,11,
                                    62,52,35,18,14,46,0,54,17,40,27,4,31,8,5,12,
                                    3,16,41,34,33,7,45,49,50,58,1,21,43,57,32,13}},
                    // 8 bits
                    {"AES", {0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
                                                                               0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
                                    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
                                    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
                                    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
                                    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
                                    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
                                    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
                                    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
                                    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
                                    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
                                    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
                                    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
                                    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
                                    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
                                    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16}},
                    {"SKINNY_8", {0x65,0x4c,0x6a,0x42,0x4b,0x63,0x43,0x6b,0x55,0x75,0x5a,0x7a,0x53,0x73,0x5b,0x7b,
                                    0x35,0x8c,0x3a,0x81,0x89,0x33,0x80,0x3b,0x95,0x25,0x98,0x2a,0x90,0x23,0x99,0x2b,
                                    0xe5,0xcc,0xe8,0xc1,0xc9,0xe0,0xc0,0xe9,0xd5,0xf5,0xd8,0xf8,0xd0,0xf0,0xd9,0xf9,
                                    0xa5,0x1c,0xa8,0x12,0x1b,0xa0,0x13,0xa9,0x05,0xb5,0x0a,0xb8,0x03,0xb0,0x0b,0xb9,
                                    0x32,0x88,0x3c,0x85,0x8d,0x34,0x84,0x3d,0x91,0x22,0x9c,0x2c,0x94,0x24,0x9d,0x2d,
                                    0x62,0x4a,0x6c,0x45,0x4d,0x64,0x44,0x6d,0x52,0x72,0x5c,0x7c,0x54,0x74,0x5d,0x7d,
                                    0xa1,0x1a,0xac,0x15,0x1d,0xa4,0x14,0xad,0x02,0xb1,0x0c,0xbc,0x04,0xb4,0x0d,0xbd,
                                    0xe1,0xc8,0xec,0xc5,0xcd,0xe4,0xc4,0xed,0xd1,0xf1,0xdc,0xfc,0xd4,0xf4,0xdd,0xfd,
                                    0x36,0x8e,0x38,0x82,0x8b,0x30,0x83,0x39,0x96,0x26,0x9a,0x28,0x93,0x20,0x9b,0x29,
                                    0x66,0x4e,0x68,0x41,0x49,0x60,0x40,0x69,0x56,0x76,0x58,0x78,0x50,0x70,0x59,0x79,
                                    0xa6,0x1e,0xaa,0x11,0x19,0xa3,0x10,0xab,0x06,0xb6,0x08,0xba,0x00,0xb3,0x09,0xbb,
                                    0xe6,0xce,0xea,0xc2,0xcb,0xe3,0xc3,0xeb,0xd6,0xf6,0xda,0xfa,0xd3,0xf3,0xdb,0xfb,
                                    0x31,0x8a,0x3e,0x86,0x8f,0x37,0x87,0x3f,0x92,0x21,0x9e,0x2e,0x97,0x27,0x9f,0x2f,
                                    0x61,0x48,0x6e,0x46,0x4f,0x67,0x47,0x6f,0x51,0x71,0x5e,0x7e,0x57,0x77,0x5f,0x7f,
                                    0xa2,0x18,0xae,0x16,0x1f,0xa7,0x17,0xaf,0x01,0xb2,0x0e,0xbe,0x07,0xb7,0x0f,0xbf,
                                    0xe2,0xca,0xee,0xc6,0xcf,0xe7,0xc7,0xef,0xd2,0xf2,0xde,0xfe,0xd7,0xf7,0xdf,0xff}},
                    {"", {}}};

    // GREEDY_SUN, SUB_MILP, CONVEX_HULL, LOGIC_CONDITION, DISTORTED_BALLS, COMBINE233, Syntax-guided
    static int ALGS[] = {1, 2, 3, 4, 5, 6, 7};

    static std::string CIPHERS4[] = {"SKINNY_4", "GIFT", "Elephant", "PRESENT", "KLEIN", "TWINE", "PRINCE", "Piccolo", "MIBS",
                                     "LBlock_0", "LBlock_1", "LBlock_2", "LBlock_3", "LBlock_4", "LBlock_5", "LBlock_6",
                                     "LBlock_7", "LBlock_8", "LBlock_9",
                                     "SERPENT_S0", "SERPENT_S1", "SERPENT_S2", "SERPENT_S3", "SERPENT_S4", "SERPENT_S5",
                                     "SERPENT_S6", "SERPENT_S7",
                                     "Midori_Sb0", "Midori_Sb1", "Rectangle", "Pride"};

    static std::string CIPHERS5[] = {"Ascon", "DryGASCON128", "Fides_5", "SC2000_5", "Shamash", "SYCON"};

    static std::string CIPHERS6[] = {"APN_6", "Fides_6", "SC2000_6"};

    static std::string CIPHERS8[] = {"AES", "SKINNY_8"};

    static std::unordered_map<std::string, std::string> cryptPrimitiveMap
            {{"AES","AES"}, {"DES", "DES"}, {"GIFT-64", "GIFT_64"}, {"KLEIN", "KLEIN2"},
             {"LBlock", "LBlock"}, {"MIBS", "MIBS1"}, {"Piccolo", "Piccolo"},
             {"PRESENT", "PRESENT"}, {"Rectangle", "Rectangle"},
             {"SIMON32", "SIMON32"}, {"SIMON48", "SIMON48"}, {"SIMON64", "SIMON"},
             {"SKINNY-64", "SKINNY_64tn"}, {"TWINE", "TWINE"},
             {"ASCON", "ASCON"}, {"Elephant", "Elephant_Dumbo"}, {"GIFT-COFB", "GIFT-COFB"},
             {"GRAIN", "grain128aead_enc"}, {"ISAP", "ISAP-KECCAK-p"}, {"Photon", "PHOTON-Beetle"},
             {"Romulus", "Romulus"}, {"SPARKLE", "SPARKLE"},
             {"TinyJAMBU", "TinyJAMBU"}, {"Xoodyak", "Xoodyak"}};

    static std::unordered_map<std::string, std::string> cryptPrimitiveSetMap
            {{"AES","WordWise"}, {"DES", "BlockCipher"}, {"GIFT-64", "BlockCipher"},
             {"KLEIN", "WordWise"},
             {"LBlock", "WordWise"}, {"MIBS", "WordWise"}, {"Piccolo", "WordWise"},
             {"PRESENT", "BlockCipher"}, {"Rectangle", "BlockCipher"},
             {"SIMON32", "BlockCipher"}, {"SIMON48", "BlockCipher"}, {"SIMON64", "BlockCipher"},
             {"SKINNY-64", "BlockCipher"}, {"TWINE", "WordWise"},
             {"ASCON", "NIST"}, {"Elephant", "NIST"}, {"GIFT-COFB", "NIST"},
             {"GRAIN", "NIST"}, {"ISAP", "NIST"}, {"Photon", "WordWise"},
             {"Romulus", "NIST"}, {"SPARKLE", "NIST"},
             {"TinyJAMBU", "NIST"}, {"Xoodyak", "NIST"}};

    static std::unordered_map<std::string, std::string> cryptPrimitiveSetMapSup
            {{"AES","w"}, {"DES", "b"}, {"GIFT-64", "b"},
             {"KLEIN", "w"},
             {"LBlock", "w"}, {"MIBS", "w"}, {"Piccolo", "w"},
             {"PRESENT", "b"}, {"Rectangle", "b"},
             {"SIMON32", "d"}, {"SIMON48", "d"}, {"SIMON64", "d"},
             {"SKINNY-64", "b"}, {"TWINE", "w"},
             {"ASCON", "b"}, {"Elephant", "b"}, {"GIFT-COFB", "d"},
             {"GRAIN", "n"}, {"ISAP", "d"}, {"Photon", "w"},
             {"Romulus", "b"}, {"SPARKLE", "d"},
             {"TinyJAMBU", "d"}, {"Xoodyak", "d"}};

static std::unordered_map<std::string, std::string> cryptPrimitiveInterMap
        {{"AES","BlockCipher"}, {"DES", "BlockCipher"}, {"GIFT-64", "BlockCipher"},
         {"KLEIN", "BlockCipher"},
         {"LBlock", "BlockCipher"}, {"MIBS", "BlockCipher"}, {"Piccolo", "BlockCipher"},
         {"PRESENT", "BlockCipher"}, {"Rectangle", "BlockCipher"},
         {"SIMON32", "BlockCipher"}, {"SIMON48", "BlockCipher"}, {"SIMON64", "BlockCipher"},
         {"SKINNY-64", "BlockCipher"}, {"TWINE", "BlockCipher"},
         {"ASCON", "NIST"}, {"Elephant", "NIST"}, {"GIFT-COFB", "NIST"},
         {"GRAIN", "NIST"}, {"ISAP", "NIST"}, {"Photon", "NIST"},
         {"Romulus", "NIST"}, {"SPARKLE", "NIST"},
         {"TinyJAMBU", "NIST"}, {"Xoodyak", "NIST"}};
}
