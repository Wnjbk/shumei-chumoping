/* Vendor TEST_79505A_HKC7_LV_4Lane_60Hz_1280_Capless_V2_2026_01_30(1).txt. */
/* REGISTER length is decimal: the E0/E1 Gamma commands carry 27 bytes. */
#ifndef ILI79505A_TEST_CMDS_H
#define ILI79505A_TEST_CMDS_H
struct ili79505a_test_step {
    u8 packet[28];
    u8 packet_len;
    u16 delay_ms;
};
static const struct ili79505a_test_step ili79505a_test_init[] = {
    { .packet = { 0xff, 0x5a, 0xa5, 0x01 }, .packet_len = 4 }, /* vendor line 1 */
    { .packet = { 0x00, 0x50 }, .packet_len = 2 }, /* vendor line 3 */
    { .packet = { 0x01, 0x37 }, .packet_len = 2 }, /* vendor line 4 */
    { .packet = { 0x02, 0x00 }, .packet_len = 2 }, /* vendor line 5 */
    { .packet = { 0x03, 0x00 }, .packet_len = 2 }, /* vendor line 6 */
    { .packet = { 0x08, 0x8d }, .packet_len = 2 }, /* vendor line 9 */
    { .packet = { 0x09, 0x02 }, .packet_len = 2 }, /* vendor line 10 */
    { .packet = { 0x0a, 0xf6 }, .packet_len = 2 }, /* vendor line 11 */
    { .packet = { 0x0b, 0x00 }, .packet_len = 2 }, /* vendor line 12 */
    { .packet = { 0x0c, 0x09 }, .packet_len = 2 }, /* vendor line 13 */
    { .packet = { 0x0e, 0x00 }, .packet_len = 2 }, /* vendor line 14 */
    { .packet = { 0x16, 0x8d }, .packet_len = 2 }, /* vendor line 16 */
    { .packet = { 0x17, 0x02 }, .packet_len = 2 }, /* vendor line 17 */
    { .packet = { 0x18, 0xf6 }, .packet_len = 2 }, /* vendor line 18 */
    { .packet = { 0x19, 0x00 }, .packet_len = 2 }, /* vendor line 19 */
    { .packet = { 0x1a, 0x09 }, .packet_len = 2 }, /* vendor line 20 */
    { .packet = { 0x1c, 0x00 }, .packet_len = 2 }, /* vendor line 21 */
    { .packet = { 0x2c, 0x30 }, .packet_len = 2 }, /* vendor line 23 */
    { .packet = { 0x28, 0x9e }, .packet_len = 2 }, /* vendor line 25 */
    { .packet = { 0x29, 0x57 }, .packet_len = 2 }, /* vendor line 26 */
    { .packet = { 0x90, 0x9e }, .packet_len = 2 }, /* vendor line 28 */
    { .packet = { 0x91, 0x57 }, .packet_len = 2 }, /* vendor line 29 */
    { .packet = { 0xb9, 0x10 }, .packet_len = 2 }, /* vendor line 34 */
    { .packet = { 0x30, 0x07 }, .packet_len = 2 }, /* vendor line 38 */
    { .packet = { 0x31, 0x07 }, .packet_len = 2 }, /* vendor line 39 */
    { .packet = { 0x32, 0x22 }, .packet_len = 2 }, /* vendor line 40 */
    { .packet = { 0x33, 0x2c }, .packet_len = 2 }, /* vendor line 41 */
    { .packet = { 0x34, 0x00 }, .packet_len = 2 }, /* vendor line 42 */
    { .packet = { 0x35, 0x00 }, .packet_len = 2 }, /* vendor line 43 */
    { .packet = { 0x36, 0x01 }, .packet_len = 2 }, /* vendor line 44 */
    { .packet = { 0x37, 0x01 }, .packet_len = 2 }, /* vendor line 45 */
    { .packet = { 0x38, 0x28 }, .packet_len = 2 }, /* vendor line 46 */
    { .packet = { 0x39, 0x29 }, .packet_len = 2 }, /* vendor line 47 */
    { .packet = { 0x3a, 0x02 }, .packet_len = 2 }, /* vendor line 48 */
    { .packet = { 0x3b, 0x02 }, .packet_len = 2 }, /* vendor line 49 */
    { .packet = { 0x3c, 0x1e }, .packet_len = 2 }, /* vendor line 50 */
    { .packet = { 0x3d, 0x1c }, .packet_len = 2 }, /* vendor line 51 */
    { .packet = { 0x3e, 0x1a }, .packet_len = 2 }, /* vendor line 52 */
    { .packet = { 0x3f, 0x18 }, .packet_len = 2 }, /* vendor line 53 */
    { .packet = { 0x40, 0x16 }, .packet_len = 2 }, /* vendor line 54 */
    { .packet = { 0x41, 0x14 }, .packet_len = 2 }, /* vendor line 55 */
    { .packet = { 0x42, 0x12 }, .packet_len = 2 }, /* vendor line 56 */
    { .packet = { 0x43, 0x10 }, .packet_len = 2 }, /* vendor line 57 */
    { .packet = { 0x44, 0x08 }, .packet_len = 2 }, /* vendor line 58 */
    { .packet = { 0x45, 0x0a }, .packet_len = 2 }, /* vendor line 59 */
    { .packet = { 0x48, 0x07 }, .packet_len = 2 }, /* vendor line 63 */
    { .packet = { 0x49, 0x07 }, .packet_len = 2 }, /* vendor line 64 */
    { .packet = { 0x4a, 0x22 }, .packet_len = 2 }, /* vendor line 65 */
    { .packet = { 0x4b, 0x2c }, .packet_len = 2 }, /* vendor line 66 */
    { .packet = { 0x4c, 0x00 }, .packet_len = 2 }, /* vendor line 67 */
    { .packet = { 0x4d, 0x00 }, .packet_len = 2 }, /* vendor line 68 */
    { .packet = { 0x4e, 0x01 }, .packet_len = 2 }, /* vendor line 69 */
    { .packet = { 0x4f, 0x01 }, .packet_len = 2 }, /* vendor line 70 */
    { .packet = { 0x50, 0x28 }, .packet_len = 2 }, /* vendor line 71 */
    { .packet = { 0x51, 0x29 }, .packet_len = 2 }, /* vendor line 72 */
    { .packet = { 0x52, 0x02 }, .packet_len = 2 }, /* vendor line 73 */
    { .packet = { 0x53, 0x02 }, .packet_len = 2 }, /* vendor line 74 */
    { .packet = { 0x54, 0x1f }, .packet_len = 2 }, /* vendor line 75 */
    { .packet = { 0x55, 0x1d }, .packet_len = 2 }, /* vendor line 76 */
    { .packet = { 0x56, 0x1b }, .packet_len = 2 }, /* vendor line 77 */
    { .packet = { 0x57, 0x19 }, .packet_len = 2 }, /* vendor line 78 */
    { .packet = { 0x58, 0x17 }, .packet_len = 2 }, /* vendor line 79 */
    { .packet = { 0x59, 0x15 }, .packet_len = 2 }, /* vendor line 80 */
    { .packet = { 0x5a, 0x13 }, .packet_len = 2 }, /* vendor line 81 */
    { .packet = { 0x5b, 0x11 }, .packet_len = 2 }, /* vendor line 82 */
    { .packet = { 0x5c, 0x09 }, .packet_len = 2 }, /* vendor line 83 */
    { .packet = { 0x5d, 0x0b }, .packet_len = 2 }, /* vendor line 84 */
    { .packet = { 0x60, 0x07 }, .packet_len = 2 }, /* vendor line 87 */
    { .packet = { 0x61, 0x07 }, .packet_len = 2 }, /* vendor line 88 */
    { .packet = { 0x62, 0x22 }, .packet_len = 2 }, /* vendor line 89 */
    { .packet = { 0x63, 0x2c }, .packet_len = 2 }, /* vendor line 90 */
    { .packet = { 0x64, 0x00 }, .packet_len = 2 }, /* vendor line 91 */
    { .packet = { 0x65, 0x00 }, .packet_len = 2 }, /* vendor line 92 */
    { .packet = { 0x66, 0x01 }, .packet_len = 2 }, /* vendor line 93 */
    { .packet = { 0x67, 0x01 }, .packet_len = 2 }, /* vendor line 94 */
    { .packet = { 0x68, 0x28 }, .packet_len = 2 }, /* vendor line 95 */
    { .packet = { 0x69, 0x29 }, .packet_len = 2 }, /* vendor line 96 */
    { .packet = { 0x6a, 0x02 }, .packet_len = 2 }, /* vendor line 97 */
    { .packet = { 0x6b, 0x02 }, .packet_len = 2 }, /* vendor line 98 */
    { .packet = { 0x6c, 0x11 }, .packet_len = 2 }, /* vendor line 99 */
    { .packet = { 0x6d, 0x13 }, .packet_len = 2 }, /* vendor line 100 */
    { .packet = { 0x6e, 0x15 }, .packet_len = 2 }, /* vendor line 101 */
    { .packet = { 0x6f, 0x17 }, .packet_len = 2 }, /* vendor line 102 */
    { .packet = { 0x70, 0x19 }, .packet_len = 2 }, /* vendor line 103 */
    { .packet = { 0x71, 0x1b }, .packet_len = 2 }, /* vendor line 104 */
    { .packet = { 0x72, 0x1d }, .packet_len = 2 }, /* vendor line 105 */
    { .packet = { 0x73, 0x1f }, .packet_len = 2 }, /* vendor line 106 */
    { .packet = { 0x74, 0x0b }, .packet_len = 2 }, /* vendor line 107 */
    { .packet = { 0x75, 0x09 }, .packet_len = 2 }, /* vendor line 108 */
    { .packet = { 0x78, 0x07 }, .packet_len = 2 }, /* vendor line 110 */
    { .packet = { 0x79, 0x07 }, .packet_len = 2 }, /* vendor line 111 */
    { .packet = { 0x7a, 0x22 }, .packet_len = 2 }, /* vendor line 112 */
    { .packet = { 0x7b, 0x2c }, .packet_len = 2 }, /* vendor line 113 */
    { .packet = { 0x7c, 0x00 }, .packet_len = 2 }, /* vendor line 114 */
    { .packet = { 0x7d, 0x00 }, .packet_len = 2 }, /* vendor line 115 */
    { .packet = { 0x7e, 0x01 }, .packet_len = 2 }, /* vendor line 116 */
    { .packet = { 0x7f, 0x01 }, .packet_len = 2 }, /* vendor line 117 */
    { .packet = { 0x80, 0x28 }, .packet_len = 2 }, /* vendor line 118 */
    { .packet = { 0x81, 0x29 }, .packet_len = 2 }, /* vendor line 119 */
    { .packet = { 0x82, 0x02 }, .packet_len = 2 }, /* vendor line 120 */
    { .packet = { 0x83, 0x02 }, .packet_len = 2 }, /* vendor line 121 */
    { .packet = { 0x84, 0x10 }, .packet_len = 2 }, /* vendor line 122 */
    { .packet = { 0x85, 0x12 }, .packet_len = 2 }, /* vendor line 123 */
    { .packet = { 0x86, 0x14 }, .packet_len = 2 }, /* vendor line 124 */
    { .packet = { 0x87, 0x16 }, .packet_len = 2 }, /* vendor line 125 */
    { .packet = { 0x88, 0x18 }, .packet_len = 2 }, /* vendor line 126 */
    { .packet = { 0x89, 0x1a }, .packet_len = 2 }, /* vendor line 127 */
    { .packet = { 0x8a, 0x1c }, .packet_len = 2 }, /* vendor line 128 */
    { .packet = { 0x8b, 0x1e }, .packet_len = 2 }, /* vendor line 129 */
    { .packet = { 0x8c, 0x0a }, .packet_len = 2 }, /* vendor line 130 */
    { .packet = { 0x8d, 0x08 }, .packet_len = 2 }, /* vendor line 131 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x01 }, .packet_len = 4 }, /* vendor line 135 */
    { .packet = { 0xc1, 0x00 }, .packet_len = 2 }, /* vendor line 136 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x02 }, .packet_len = 4 }, /* vendor line 139 */
    { .packet = { 0x06, 0xb3 }, .packet_len = 2 }, /* vendor line 140 */
    { .packet = { 0x07, 0x00 }, .packet_len = 2 }, /* vendor line 141 */
    { .packet = { 0x39, 0x01 }, .packet_len = 2 }, /* vendor line 142 */
    { .packet = { 0x3a, 0x57 }, .packet_len = 2 }, /* vendor line 143 */
    { .packet = { 0x3b, 0x00 }, .packet_len = 2 }, /* vendor line 144 */
    { .packet = { 0x3c, 0x34 }, .packet_len = 2 }, /* vendor line 145 */
    { .packet = { 0x3d, 0x00 }, .packet_len = 2 }, /* vendor line 146 */
    { .packet = { 0x3e, 0x96 }, .packet_len = 2 }, /* vendor line 147 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x02 }, .packet_len = 4 }, /* vendor line 148 */
    { .packet = { 0x43, 0x00 }, .packet_len = 2 }, /* vendor line 149 */
    { .packet = { 0x1b, 0x00 }, .packet_len = 2 }, /* vendor line 150 */
    { .packet = { 0x1c, 0xd3 }, .packet_len = 2 }, /* vendor line 151 */
    { .packet = { 0x48, 0x01 }, .packet_len = 2 }, /* vendor line 154 */
    { .packet = { 0x08, 0x86 }, .packet_len = 2 }, /* vendor line 155 */
    { .packet = { 0x49, 0x86 }, .packet_len = 2 }, /* vendor line 157 */
    { .packet = { 0x51, 0x34 }, .packet_len = 2 }, /* vendor line 158 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x03 }, .packet_len = 4 }, /* vendor line 161 */
    { .packet = { 0x20, 0x01 }, .packet_len = 2 }, /* vendor line 162 */
    { .packet = { 0x22, 0xff }, .packet_len = 2 }, /* vendor line 163 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x14 }, .packet_len = 4 }, /* vendor line 166 */
    { .packet = { 0x44, 0x00 }, .packet_len = 2 }, /* vendor line 167 */
    { .packet = { 0x45, 0x8a }, .packet_len = 2 }, /* vendor line 168 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x05 }, .packet_len = 4 }, /* vendor line 169 */
    { .packet = { 0x54, 0x97 }, .packet_len = 2 }, /* vendor line 170 */
    { .packet = { 0x55, 0x97 }, .packet_len = 2 }, /* vendor line 171 */
    { .packet = { 0x5a, 0xa1 }, .packet_len = 2 }, /* vendor line 172 */
    { .packet = { 0x61, 0xa7 }, .packet_len = 2 }, /* vendor line 173 */
    { .packet = { 0x67, 0x8d }, .packet_len = 2 }, /* vendor line 174 */
    { .packet = { 0x6d, 0x7f }, .packet_len = 2 }, /* vendor line 175 */
    { .packet = { 0x51, 0x87 }, .packet_len = 2 }, /* vendor line 178 */
    { .packet = { 0x02, 0x13 }, .packet_len = 2 }, /* vendor line 179 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x06 }, .packet_len = 4 }, /* vendor line 183 */
    { .packet = { 0x35, 0x20 }, .packet_len = 2 }, /* vendor line 184 */
    { .packet = { 0xd9, 0x0f }, .packet_len = 2 }, /* vendor line 185 */
    { .packet = { 0xc0, 0x00 }, .packet_len = 2 }, /* vendor line 186 */
    { .packet = { 0xc1, 0x15 }, .packet_len = 2 }, /* vendor line 187 */
    { .packet = { 0xc3, 0x0e }, .packet_len = 2 }, /* vendor line 188 */
    { .packet = { 0x0d, 0x8a }, .packet_len = 2 }, /* vendor line 189 */
    { .packet = { 0x0e, 0x67 }, .packet_len = 2 }, /* vendor line 190 */
    { .packet = { 0xd6, 0x55 }, .packet_len = 2 }, /* vendor line 192 */
    { .packet = { 0xdc, 0x88 }, .packet_len = 2 }, /* vendor line 193 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x08 }, .packet_len = 4 }, /* vendor line 196 */
    { .packet = { 0xe0, 0x00, 0x00, 0x6f, 0xad, 0xf5, 0x55, 0x2d, 0x57, 0x88, 0xaf, 0xa9, 0xeb, 0x1a, 0x45, 0x6d, 0xea, 0x97, 0xc8, 0xe7, 0x0d, 0xff, 0x2d, 0x55, 0x85, 0xac, 0x03, 0xd7 }, .packet_len = 28 }, /* vendor line 197 */
    { .packet = { 0xe1, 0x00, 0x00, 0x6f, 0xad, 0xf5, 0x55, 0x2d, 0x57, 0x88, 0xaf, 0xa9, 0xeb, 0x1a, 0x45, 0x6d, 0xea, 0x97, 0xc8, 0xe7, 0x0d, 0xff, 0x2d, 0x55, 0x85, 0xac, 0x03, 0xd7 }, .packet_len = 28 }, /* vendor line 198 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x0a }, .packet_len = 4 }, /* vendor line 201 */
    { .packet = { 0xe0, 0x01 }, .packet_len = 2 }, /* vendor line 202 */
    { .packet = { 0xe1, 0x0b }, .packet_len = 2 }, /* vendor line 203 */
    { .packet = { 0xe2, 0x01 }, .packet_len = 2 }, /* vendor line 204 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x0b }, .packet_len = 4 }, /* vendor line 207 */
    { .packet = { 0xa6, 0xc5 }, .packet_len = 2 }, /* vendor line 208 */
    { .packet = { 0xa7, 0x9f }, .packet_len = 2 }, /* vendor line 209 */
    { .packet = { 0xa8, 0x04 }, .packet_len = 2 }, /* vendor line 210 */
    { .packet = { 0xa9, 0x04 }, .packet_len = 2 }, /* vendor line 211 */
    { .packet = { 0xaa, 0x8c }, .packet_len = 2 }, /* vendor line 212 */
    { .packet = { 0xab, 0x8c }, .packet_len = 2 }, /* vendor line 213 */
    { .packet = { 0xbd, 0x22 }, .packet_len = 2 }, /* vendor line 214 */
    { .packet = { 0xbe, 0xe0 }, .packet_len = 2 }, /* vendor line 215 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x0e }, .packet_len = 4 }, /* vendor line 218 */
    { .packet = { 0x14, 0x07 }, .packet_len = 2 }, /* vendor line 219 */
    { .packet = { 0x10, 0x02 }, .packet_len = 2 }, /* vendor line 220 */
    { .packet = { 0x15, 0x09 }, .packet_len = 2 }, /* vendor line 221 */
    { .packet = { 0x00, 0xa0 }, .packet_len = 2 }, /* vendor line 222 */
    { .packet = { 0xff, 0x5a, 0xa5, 0x00 }, .packet_len = 4 }, /* vendor line 224 */
    { .packet = { 0x35, 0x00 }, .packet_len = 2 }, /* vendor line 226 */
    { .packet = { 0x11 }, .packet_len = 1 }, /* vendor line 227 */
    { .delay_ms = 120 }, /* vendor line 228 */
    { .packet = { 0x29 }, .packet_len = 1 }, /* vendor line 229 */
    { .delay_ms = 120 }, /* vendor line 230 */
};
#endif
