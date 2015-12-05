#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>
#include <string>

#include "encoder.h"

#ifndef M_SQRT2
const double M_SQRT2 = sqrt(2);
#endif

#ifndef M_PI
const double M_PI = 4 * atan(1);
#endif

BMPReader::BMPReader(const char *file_name) {
    _IN = fopen(file_name, "rb");
    if (_IN == NULL) throw "BMPReader::Could not open input file";

    _read_header();

    printf("BMPReader::width = %d, height = %d\n", width, height);

    int line_size = (3 * width + 3) / 4 * 4;
    _line_left = line_size - width * 3;

    _bfr_siz = 1024 * 1024;
    _bfr = new uint8 [_bfr_siz];
    _bfr_idx = _bfr_siz;
}

void BMPReader::read_line(double *y, double *cb, double *cr) {
    if (_bfr_idx + width * 3 >= _bfr_siz) {
        int leftover = _bfr_siz - _bfr_idx;

        memcpy(_bfr, _bfr + _bfr_idx, leftover);

        fread(_bfr + leftover, 1, _bfr_siz - leftover, _IN), _bfr_idx = 0;
    }

    for (int i = 0; i < width; ++i) {
        double b = _bfr[_bfr_idx++];
        double g = _bfr[_bfr_idx++];
        double r = _bfr[_bfr_idx++];

        y[i]  =  0.299 * r + 0.587 * g + 0.114 * b;
        cb[i] = -0.168 * r - 0.331 * g + 0.500 * b + 128;
        cr[i] =  0.500 * r - 0.419 * g - 0.081 * b + 128;
    }

    _bfr_idx += _line_left;
}

void BMPReader::_read_header() {
    const int shit = 2 + 4 + 2 + 2 + 4 + 4;
    const int read_len = shit + 4 + 4 + 2 + 2 + 4 * 6;

    uint8 bfr[read_len];
    fread(bfr, 1, read_len, _IN);

    memcpy(&width, bfr + shit, 4);
    memcpy(&height, bfr + shit + 4, 4);
    height *= -1;
}



const int zigzag_x[] = {0,1,0,0,1,2,3,2,1,0,0,1,2,3,4,5,4,3,2,1,0,0,1,2,3,4,5,6,7,6,5,4,
                        3,2,1,0,1,2,3,4,5,6,7,7,6,5,4,3,2,3,4,5,6,7,7,6,5,4,5,6,7,7,6,7};
const int zigzag_y[] = {0,0,1,2,1,0,0,1,2,3,4,3,2,1,0,0,1,2,3,4,5,6,5,4,3,2,1,0,0,1,2,3,
                        4,5,6,7,7,6,5,4,3,2,1,2,3,4,5,6,7,7,6,5,4,3,4,5,6,7,7,6,5,6,7,7};

const uint8 quantize_tables[2][8][8] = {
    {
        {16,	11,	10,	16,	124,	140,	151,	161},
        {12,	12,	14,	19,	126,	158,	160,	155},
        {14,	13,	16,	24,	140,	157,	169,	156},
        {14,	17,	22,	29,	151,	187,	180,	162},
        {18,	22,	37,	56,	168,	109,	103,	177},
        {24,	35,	55,	64,	181,	104,	113,	192},
        {49,	64,	78,	87,	103,	121,	120,	101},
        {72,	92,	95,	98,	112,	100,	103,	199}
    },
    {
        {17,	18,	24,	47,	99,	99,	99,	99},
        {18,	21,	26,	66,	99,	99,	99,	99},
        {24,	26,	56,	99,	99,	99,	99,	99},
        {47,	66,	99,	99,	99,	99,	99,	99},
        {99,	99,	99,	99,	99,	99,	99,	99},
        {99,	99,	99,	99,	99,	99,	99,	99},
        {99,	99,	99,	99,	99,	99,	99,	99},
        {99,	99,	99,	99,	99,	99,	99,	99}
    }
};

namespace Huffman {
    int16 L_h[] = {31, 181, 31, 181};
    int8 T_c[] = {0, 1, 0, 1};
    int8 T_h[] = {0, 0, 1, 1};

    uint8 L_i0[] = {0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
    uint8 V_ij0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    uint8 L_i1[] = {0, 2, 1, 3, 3, 2, 3, 3, 6, 5, 4, 4, 0, 0, 1, 125};
    uint8 V_ij1[] = {1, 2, 3, 0, 4, 17, 5, 18, 33, 49, 65, 6, 19, 81, 97, 7, 34, 113, 20, 50,
                     129, 145, 161, 8, 35, 66, 177, 193, 21, 82, 209, 240, 36, 51, 98, 114, 130,
                     9, 10, 22, 23, 24, 25, 26, 37, 38, 39, 40, 41, 42, 52, 53, 54, 55, 56, 57,
                     58, 67, 68, 69, 70, 71, 72, 73, 74, 83, 84, 85, 86, 87, 88, 89, 90, 99, 100,
                     101, 102, 103, 104, 105, 106, 115, 116, 117, 118, 119, 120, 121, 122, 131,
                     132, 133, 134, 135, 136, 137, 138, 146, 147, 148, 149, 150, 151, 152, 153,
                     154, 162, 163, 164, 165, 166, 167, 168, 169, 170, 178, 179, 180, 181, 182,
                     183, 184, 185, 186, 194, 195, 196, 197, 198, 199, 200, 201, 202, 210, 211,
                     212, 213, 214, 215, 216, 217, 218, 225, 226, 227, 228, 229, 230, 231, 232,
                     233, 234, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250};

    uint8 L_i2[] = {0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
    uint8 V_ij2[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    uint8 L_i3[] = {0, 2, 1, 2, 4, 3, 4, 3, 8, 5, 4, 4, 0, 1, 2, 119};
    uint8 V_ij3[] = {0, 1, 2, 3, 17, 4, 5, 33, 49, 6, 18, 65, 81, 7, 97, 113, 19, 34, 50, 129,
                     8, 20, 66, 145, 161, 177, 193, 9, 35, 51, 82, 240, 21, 98, 114, 209, 10, 22,
                     36, 52, 225, 37, 241, 23, 24, 25, 26, 38, 39, 40, 41, 42, 53, 54, 55, 56, 57,
                     58, 67, 68, 69, 70, 71, 72, 73, 74, 83, 84, 85, 86, 87, 88, 89, 90, 99, 100,
                     101, 102, 103, 104, 105, 106, 115, 116, 117, 118, 119, 120, 121, 122, 130,
                     131, 132, 133, 134, 135, 136, 137, 138, 146, 147, 148, 149, 150, 151, 152,
                     153, 154, 162, 163, 164, 165, 166, 167, 168, 169, 170, 178, 179, 180, 181,
                     182, 183, 184, 185, 186, 194, 195, 196, 197, 198, 199, 200, 201, 202, 210,
                     211, 212, 213, 214, 215, 216, 217, 218, 226, 227, 228, 229, 230, 231, 232,
                     233, 234, 242, 243, 244, 245, 246, 247, 248, 249, 250};

    uint8 *L_i[] = {L_i0, L_i1, L_i2, L_i3};
    uint8 *V_ij[] = {V_ij0, V_ij1, V_ij2, V_ij3};
}


void Encoder::_write_DQTs() {
    _write("\xFF\xDB", 1, 2); // DQT

    int16 L_q = 2 + (1 + 8 * 8) * 2;
    _write(&L_q, 2, 1);

    for (int id = 0; id < 2; ++id) {
        int8 P_qT_q = id;
        _write(&P_qT_q, 1, 1);

        for (int idx = 0; idx < 64; ++idx)
            _write(&quantize_tables[id][zigzag_y[idx]][zigzag_x[idx]], 1, 1);
    }
}

void Encoder::_write_SOF0() {
    _write("\xFF\xC0", 1, 2); // SOF0

    int8 N_f = 3;
    int16 L_f = 8 + 3 * N_f;
    int8 P = 8;
    int16 Y = _height, X = _width;

    _write(&L_f, 2, 1);
    _write(&P, 1, 1);
    _write(&Y, 2, 1);
    _write(&X, 2, 1);
    _write(&N_f, 1, 1);

    for (int i = 0; i < 3; ++i) {
        int8 C = i+1;
        int sample_factor = i == 0 ? 2 : 1;
        int8 HV = sample_factor << 4 | sample_factor;
        int8 T_q = i == 0 ? 0 : 1;

        _write(&C, 1, 1);
        _write(&HV, 1, 1);
        _write(&T_q, 1, 1);
    }
}

void Encoder::_write_DHTs() {
    for (int T_h = 0; T_h < 2; ++T_h) {
        for (int T_c = 0; T_c < 2; ++T_c) {
            _write("\xFF\xC4", 1, 2); // DHT

            int idx = T_h << 1 | T_c;

            int16 L_h = Huffman::L_h[idx];
            _write(&L_h, 2, 1);

            int8 T_cT_h = Huffman::T_c[idx] << 4 | Huffman::T_h[idx];
            _write(&T_cT_h, 1, 1);

            _write(Huffman::L_i[idx], 1, 16);
            _write(Huffman::V_ij[idx], 1, L_h - 2 - 1 - 16);

            _hTable.push_back(HTable(Huffman::L_i[idx], Huffman::V_ij[idx]));
        }
    }
}

void Encoder::_write_SOS() {
    _write("\xFF\xDA", 1, 2); // SOS

    int8 N_s = 3;
    int16 L_s = 6 + 2 * N_s;
    _write(&L_s, 2, 1);
    _write(&N_s, 1, 1);

    for (int i = 0; i < 3; ++i) {
        int8 Cs = i + 1;
        int8 TdTa = i == 0 ? 0 : 1 << 4 | 1;
        _write(&Cs, 1, 1);
        _write(&TdTa, 1, 1);
    }

    int8 Ss = 0, Se = 63, AhAl = 0;
    _write(&Ss, 1, 1);
    _write(&Se, 1, 1);
    _write(&AhAl, 1, 1);
}

void Encoder::_write_MCUs() {
    int ext_width  = (_width  + 15) / 16 * 16;

    for (int j = 0; j < 3; ++j)
        for (int i = 0; i < 16; ++i)
            _cnl[j][i].resize(ext_width);

    _DC_pred[0] = _DC_pred[1] = _DC_pred[2] = 0;

    for (int i = 0; i < _height; i += 16) {
        for (int j = 0; j < 16; ++j) {
            int k = i + j;

            if (k >= _height) {
                for (int a = 0; a < ext_width; ++a)
                    _cnl[0][j][a] = _cnl[0][j-1][a],
                        _cnl[1][j][a] = _cnl[1][j-1][a],
                        _cnl[2][j][a] = _cnl[2][j-1][a];
            } else {
                _reader.read_line(&_cnl[0][j][0], &_cnl[1][j][0], &_cnl[2][j][0]);

                for (int a = _width; a < ext_width; ++a)
                    _cnl[0][j][a] = _cnl[0][j][a-1],
                        _cnl[1][j][a] = _cnl[1][j][a-1],
                        _cnl[2][j][a] = _cnl[2][j][a-1];
            }
        }

        for (int j = 0; j < _width; j += 16)
            _write_MCU(j);
    }
}

void Encoder::_write_MCU(int ox) {
    for (int y = 0; y < 16; y += 8) {
        for (int dx = 0; dx < 16; dx += 8) {
            for (int ddy = 0; ddy < 8; ++ddy)
                for (int ddx = 0; ddx < 8; ++ddx)
                    _block[ddy][ddx] = _cnl[0][y+ddy][ox+dx+ddx] - 128;

            _write_block(_DC_pred[0], 0);
        }
    }

    for (int t = 1; t < 3; ++t) {
        for (int y = 0; y < 8; ++y)
            for (int dx = 0; dx < 8; ++dx) {
                int xx = ox + dx * 2, yy = y * 2;

                double sum = _cnl[t][yy][xx] + _cnl[t][yy][xx+1] +
                             _cnl[t][yy+1][xx] + _cnl[t][yy+1][xx+1];

                _block[y][dx] = sum / 4 - 128;
            }

        _write_block(_DC_pred[t], 1);
    }
}

void dct_qt(int32 out[8][8], double in[8][8], const uint8 qt[8][8]) {
    static double coeff[8][8][8][8];

    if (coeff[0][0][0][0] == 0.0) {
        double c[8];
        c[0] = M_SQRT1_2 / 2;
        for (int i = 1; i < 8; ++i)
            c[i] = 1.0 / 2;

        double coss[8][8];
        for (int i = 0; i < 8; ++i)
            for (int u = 0; u < 8; ++u)
                coss[i][u] = cos(M_PI * (2*i+1) * u / 16);

        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                for (int u = 0; u < 8; ++u)
                    for (int v = 0; v < 8; ++v)
                        coeff[i][j][u][v] = c[u] * c[v] * coss[i][u] * coss[j][v];
    }

    for (int u = 0; u < 8; ++u) {
        for (int v = 0; v < 8; ++v) {
            double sum = 0.0;
            for (int i = 0; i < 8; ++i)
                for (int j = 0; j < 8; ++j)
                    sum += coeff[i][j][u][v] * in[i][j];

            out[u][v] = round(sum / qt[u][v]);
        }
    }
}

int32 lgp(int32 val) {
    int32 cnt = 0;
    for (val = std::abs(val); val; val >>= 1) cnt += 1;
    return cnt;
}

void Encoder::_write_block(int32 &DC_pred, int is_color) {
    static int32 dct_buf[8][8];

    dct_qt(dct_buf, _block, quantize_tables[is_color]);

    dct_buf[0][0] -= DC_pred;

    if (std::abs(dct_buf[0][0]) >> 12)
        dct_buf[0][0] = dct_buf[0][0] > 0 ? 2047 : -2047;

    DC_pred += dct_buf[0][0];

    int DC_lg = lgp(dct_buf[0][0]);
    _write_Huffman(DC_lg, dct_buf[0][0], DC_lg, &_hTable[is_color << 1]);

    int zeroes = 0;
    for (int i = 1; i < 64; ++i) {
        int32 &cval = dct_buf[zigzag_y[i]][zigzag_x[i]];
        if (cval == 0) {
            zeroes += 1;
        } else {
            for (; zeroes >= 16; zeroes -= 16)
                _write_Huffman(15 << 4, 0, 0, &_hTable[is_color << 1 | 1]);

            int lg = lgp(cval);
            if (lg > 10)
                lg = 10, cval = cval > 0 ? 1023 : -1023;

            _write_Huffman(zeroes << 4 | lg, cval, lg, &_hTable[is_color << 1 | 1]);

            zeroes = 0;
        }
    }

    if (zeroes > 0)
        _write_Huffman(0, 0, 0, &_hTable[is_color << 1 | 1]);
}

void Encoder::_write_Huffman(int key, int val, int lg, HTable *table) {
    _write_bits(table->key[key], table->len[key]);

    if (--lg < 0) return ;

    _write_bits(val > 0 ? 1 : 0, 1);

    val = val > 0 ? val - (1 << lg) : val + (3 << lg) - 1;
    _write_bits(val, lg);
}

void Encoder::_write(const void *ptr, size_t size, size_t count) {
    const uint8 *p = (const uint8*)ptr;

    for (size_t idx = 0; count--; idx += size) {
        for (int j = size-1; j >= 0; --j) {
            _bfr[_bfr_i++] = p[idx + j];

            if (_bfr_i == bfr_size) {
                fwrite(_bfr, 1, bfr_size, _OUT);
                _bfr_i = 0;
            }
        }
    }
}

void Encoder::_write_bits(int val, int len) {
    static int bits[] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191,
                         16383, 32767, 65535};
    for (int left_len = len, w_len; left_len > 0; left_len -= w_len) {
        w_len = std::min(left_len, _bfr_j);
        _bfr[_bfr_i] <<= w_len;
        _bfr[_bfr_i] |= (val >> (left_len - w_len)) & bits[w_len];
        _bfr_j -= w_len;

        if (_bfr_j == 0) {
            bool is_FF = _bfr[_bfr_i] == 0xFF;

            _bfr_i += 1, _bfr_j = 8;

            if (_bfr_i == bfr_size) {
                fprintf(stderr, "FLUSH SHIT");
                fwrite(_bfr, 1, bfr_size, _OUT);
                _bfr_i = 0;
            }

            if (is_FF) _write_bits(0, 8);
        }
    }
}

bool Encoder::solve() {
    _write("\xFF\xD8", 1, 2); // SOI

    _write_DQTs();
    _write_SOF0();
    _write_DHTs();
    _write_SOS();
    _write_MCUs();

    _write_bits(0, _bfr_j);

    _write("\xFF\xD9", 1, 2); // EOI

    fwrite(_bfr, 1, _bfr_i, _OUT);

    return true;
}
