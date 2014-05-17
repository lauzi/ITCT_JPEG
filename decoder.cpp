#include <cmath>
#include <algorithm>

#include "decoder.h"

template <class T>
T div_ceil(T a, T b) { return (a + b - 1) / b; }

void BMPWriter::write_pxl(double y, double cb, double cr) {
    cb -= 128, cr -= 128;

    double rgb[] = {
        y + 1.772 * cb,
        y - 0.34414 * cb - 0.71414 * cr,
        y + 1.402 * cr};
    uint8 rgbo[3];

    for (int i = 0; i < 3; ++i) {
        rgb[i] = round(rgb[i]);
        if (rgb[i] > 255) rgbo[i] = 255;
        else if (rgb[i] < 0) rgbo[i] = 0;
        else rgbo[i] = rgb[i];
    }

    _write(rgbo, 1, 3);

    static uint8 pad_zeroes[4] = {0};
    if (++_x == width) {
        _write(pad_zeroes, 1, _pad_len);
        _x = 0;
    }
}

void BMPWriter::_write_header() {
    _write("BM", 1, 2);
    int bfOffBits = 2+4+2+2+4+40;
    int size = bfOffBits + height*(width + _pad_len);
    int tmp = 0;

    _write(&size, 4, 1);
    _write(&tmp, 4, 1);
    _write(&bfOffBits, 4, 1);

    tmp = 40;

    _write(&tmp, 4, 1);
    _write(&width, 4, 1);
    int tmp_height = -height;
    _write(&tmp_height, 4, 1);

    int16 tmp16 = 1;
    _write(&tmp16, 2, 1);
    tmp16 = 24;
    _write(&tmp16, 2, 1);

    tmp = 0;
    for (int i = 0; i < 6; ++i)
        _write(&tmp, 4, 1);
}



const int Decoder::zigzag_x[] = {0,1,0,0,1,2,3,2,1,0,0,1,2,3,4,5,4,3,2,1,0,0,1,2,3,4,5,6,7,6,5,4,
                                 3,2,1,0,1,2,3,4,5,6,7,7,6,5,4,3,2,3,4,5,6,7,7,6,5,4,5,6,7,7,6,7};
const int Decoder::zigzag_y[] = {0,0,1,2,1,0,0,1,2,3,4,3,2,1,0,0,1,2,3,4,5,6,5,4,3,2,1,0,0,1,2,3,
                                 4,5,6,7,7,6,5,4,3,2,1,2,3,4,5,6,7,7,6,5,4,3,4,5,6,7,7,6,5,6,7,7};

bool Decoder::solve() {
    _open_files();

    while (_read_next_header());

    _close_files();

    return true;
}

// returns false if finished
bool Decoder::_read_next_header() {
    static uint8 _buf[16];
    if (_has_read_ff) {
        if (_has_read_mark)
            _buf[1] = _next_mark;
        else
            _read(_buf+1, 1, 1);
    } else {
        _read(_buf, 1, 2);
        if (_buf[0] != 0xFF) {
            fprintf(stderr, "Found 0x%02X\n", _buf[0]);

            throw "DAFUQ WHY NO 0xFF";
        }
    }
    _has_read_ff = false;
    _has_read_mark = false;

    switch (_buf[1]) {
    case 0xC0: // SOF0, baseline DCT
        _SOF0();
        break;
    case 0xC4: // DHT, Define Huffman Table
        _DHT();
        break;
    case 0xD8: // SOImage
        puts("SOI");
        break;
    case 0xD9: // EOImage
        puts("EOI");
        return false;
    case 0xDA: // SOScan
        _SOS();
        break;
    case 0xDB: // Define quantization table
        _DQT();
        break;
    case 0xDD: // DRI, define restart interval
        puts("DRI");
        _DRI();
        break;
    case 0xFF:
        _has_read_ff = true;
        break;
    default:
        if ((_buf[1] >> 3) == 0xFC) { // RSTm
        } else {
            if ((_buf[1] & 0xF0) == 0xE0) // APPn
                printf("APP ");
            else
                printf("Unknown HDR: %2X ", _buf[1]);

            uint16 L;
            _read(&L, 2, 1);
            printf("with length %d\n", (int)L);
            _rseek(L-2, SEEK_CUR);
        }
    }

    return true;
}

// baseline
void Decoder::_SOF0() {
    uint16 Lq;
    _read(&Lq, 2, 1);

    uint8 buf[Lq-2];
    _read(buf, 1, Lq-2);

    // p, y, x, Nf, Component specified params
    _Y = (uint16)buf[1]<<8 | buf[2];
    _X = (uint16)buf[3]<<8 | buf[4];

    _bmp = new BMPWriter(_Y, _X, out.c_str());

    _Nf = buf[5];

    printf("SOF0::Y = %d, X = %d, N_f = %d\n", (int)_Y, (int)_X, (int)_Nf);

    _max_H = _max_V = 1;
    for (int i = 6, j = 1; j <= _Nf; ++j) {
        uint8 c = (_cs[j] = buf[i++]);
        _H_sampling_factor[c] = (buf[i]>>4) & 15;
        _max_H = std::max(_max_H, _H_sampling_factor[c]);

        _V_sampling_factor[c] = buf[i++] & 15;
        _max_V = std::max(_max_V, _V_sampling_factor[c]);

        _Qtable_selector[c] = buf[i++];

        printf("SOF0::Component %d: H=%d, V=%d, T_q=%d\n", (int)c, (int)_H_sampling_factor[c],
               (int)_V_sampling_factor[c], (int)_Qtable_selector[c]);
    }
    printf("SOF0::H_max = %d, V_max = %d\n", _max_H, _max_V);
}

void Decoder::_DHT() {
    uint16 Lh;
    _read(&Lh, 2, 1);

    uint8 buf[Lh-2];
    _read(buf, 1, Lh-2);

    for (int i = 0; i < Lh-2;) {
        int Tc = buf[i]>>4, Th = buf[i++] & 15;
        printf("DHT::T_c = %d (%s), T_h = %d\n", Tc, Tc ? "AC" : "DC", Th);

        int Ls[17];
        printf("DHT::L_i = [");
        for (int j = 1; j <= 16; ++j) {
            Ls[j] = (int)buf[i++];
            printf("%d%s", Ls[j], j < 16 ? ", " : "]\n");
        }

        for (int j = 1; j <= 16; ++j)
            while (Ls[j]--)
                _hs[Tc][Th].insert(j, buf[i++]);
    }
}

void Decoder::_SOS() {
    uint16 Ls;
    _read(&Ls, 2, 1);

    uint8 buf[Ls-2];
    _read(buf, 1, Ls-2);

    _Ns = (int)buf[0];
    printf("SOS::N_s = %d\n", _Ns);

    int i = 1;
    for (int j = 1; j <= _Ns; ++j) {
        _scan_component_selector[j] = buf[i++];
        _DC_Huffman_selector[j] = (buf[i]>>4) & 15;
        _AC_Huffman_selector[j] = buf[i++] & 15;

        printf("SOS::C_s = %d, T_d = %d, T_a = %d\n", _scan_component_selector[j],
               _DC_Huffman_selector[j], _AC_Huffman_selector[j]);
    }

    fflush(stdout);

    _read_entropy_data();
}

void Decoder::_DQT() {
    uint16 Lq;
    _read(&Lq, 2, 1);

    uint8 buf[Lq];
    _read(buf, 1, Lq-2);

    for (int i = 0; i < Lq-2; ) {
        // P_q = 0, so buf[0] = T_q
        int Tq = (int)buf[i++];
        printf("DQT::T_q = %d\n", Tq);

        uint8 (*qt)[8] = _quantization_tables[Tq];
        const int *x = zigzag_x, *y = zigzag_y;

        for (int j = 0; j < 64; ++j)
            qt[y[j]][x[j]] = buf[i++];
    }
}

void Decoder::_DRI() {}

int16 Decoder::_read_DC() {
    int T = -1;

    for (Node *node = _hc_DC->root(); T == -1; _hc_i -= 1) {
        if (_hc_i < 0)
            _read_next_entropy_byte();

        node = node->cld[(_hc_input >> _hc_i) & 1];
        if (node->cld[0] == NULL and node->cld[1] == NULL)
            T = node->val;
    }

    if (T == 0) return _DC_predict;

    int base = 1<<(T-1), idx = _read_n_bits(T);
    int acc = idx >= base ? idx : -(1<<T)+1+idx;

    return _DC_predict += acc;
}

int16 Decoder::_read_AC(int &zz_idx) {
    int RS = -1;

    int len = 0;
    for (Node *node = _hc_AC->root(); RS == -1; _hc_i -= 1) {
        if (_hc_i < 0) _read_next_entropy_byte();

        len += 1;

        node = node->cld[(_hc_input >> _hc_i) & 1];
        if (node->cld[0] == NULL and node->cld[1] == NULL)
            RS = node->val;
    }

    int R = (RS>>4) & 15, S = RS & 15;
    zz_idx = R;

    if (S == 0) return 0;

    int base = 1<<(S-1), idx = _read_n_bits(S);
    int res = idx >= base ? idx : -(1<<S)+1+idx;

    return (int16)res;
}

int Decoder::_read_n_bits(int n) {
    int acc = 0;
    for (int bits; n > 0; n -= bits) {
        if (_hc_i < 0) _read_next_entropy_byte();

        int left_len = _hc_i+1 - n;

        if (left_len >= 0)
            bits = n;
        else
            bits = _hc_i + 1, left_len = 0;

        acc <<= bits;
        acc |= (_hc_input >> left_len) & ((1<<bits) - 1);
        _hc_i -= bits;
    }
    return acc;
}

bool Decoder::_read_next_entropy_byte() {
    _read(&_hc_input, 1, 1);
    if (_hc_input == 0xFF) {
        uint8 tmpc;
        _read(&tmpc, 1, 1);

        if ((tmpc & 0xF8) == 0xD0) // RSTn
            return _read_next_entropy_byte();

        if (tmpc != 0x00) {
            _has_read_ff = _has_read_mark = true, _next_mark = tmpc;
            return false;
        }
    }
    _hc_i = 7;
    return true;
}

void idct(double (*output)[8], const int16 (*input)[8]) {
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

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            double sum = 0.0;
            for (int u = 0; u < 8; ++u)
                for (int v = 0; v < 8; ++v)
                    sum += coeff[i][j][u][v] * input[u][v];
            output[i][j] = sum + 128.0;
        }
    }
}


void Decoder::_read_entropy_block(uint8 J, double out_block[8][8]) {
    uint8 c = _scan_component_selector[J];

    _hc_DC = &_hs[0][_DC_Huffman_selector[J]];
    _hc_AC = &_hs[1][_AC_Huffman_selector[J]];

    static int16 block[8][8];

    memset(block, 0, sizeof(block));

    block[0][0] = _read_DC();

    for (int idx = 1, run_length; idx < 64; ++idx) {
        int16 val = _read_AC(run_length);
        if (val == 0 && run_length == 0) break;
        idx += run_length;
        block[zigzag_y[idx]][zigzag_x[idx]] = val;
    }

    static int16 imm_block[8][8];

    int t = _Qtable_selector[c];
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            imm_block[i][j] = block[i][j] * _quantization_tables[t][i][j];

    idct(out_block, imm_block);
}


void Decoder::_read_entropy_data() {
    MCUArr *cnls[4];

    int width_MCUs = div_ceil(div_ceil((int)_X, 8), (int)_max_H);
    for (int i = 1; i <= 3; ++i)
        cnls[i] = new MCUArr(8 * _max_V * 2, width_MCUs * 8 * _max_H);

    int H_i[_Ns + 1], V_i[_Ns + 1];
    int pxl_w[_Ns + 1], pxl_h[_Ns + 1];
    int pxl_w_ord[_Ns + 1], pxl_h_ord[_Ns + 1];

    for (int c = 1; c <= _Ns; ++c) {
        int sel = _scan_component_selector[c];
        H_i[c] = _H_sampling_factor[sel];
        pxl_w[c] = _max_H / H_i[c];
        pxl_w_ord[c] = std::__lg(pxl_w[c]);

        V_i[c] = _V_sampling_factor[sel];
        pxl_h[c] = _max_V / V_i[c];
        pxl_h_ord[c] = std::__lg(pxl_h[c]);
    }

    _hc_i = -1;

    int16 DC_predict[4] = {0};

    static double block[8][8];
    for (int y = 0; y < _Y; y += 8 * _max_V) {
        for (int x = 0; x < _X; x += 8 * _max_H) {
            for (int c = 1; c <= _Ns; ++c) {
                _DC_predict = DC_predict[c];
                for (int y_cnt = 0; y_cnt < V_i[c]; ++y_cnt) {
                    for (int x_cnt = 0; x_cnt < H_i[c]; ++x_cnt) {
                        _read_entropy_block(c, block);

                        // TODO: now no fancy interpolation
                        int ny = y + y_cnt * pxl_h[c] * 8, nx = x + x_cnt * pxl_w[c] * 8;

                        for (int dy = 0; dy < pxl_h[c] * 8; ++dy) {
                            for (int dx = 0; dx < pxl_w[c] * 8; ++dx) {
                                int iy = ny + dy, ix = nx + dx;

                                cnls[c]->at(iy, ix) += block[dy>>pxl_h_ord[c]][dx>>pxl_w_ord[c]];
                            }
                        }
                    }
                }
                DC_predict[c] = _DC_predict;
            }  // close c
        }

        for (int dy = 0; dy < 8 * _max_V; ++dy) {
            for (int x = 0; x < _X; ++x)
                _bmp->write_pxl(cnls[1]->at(y+dy, x),
                                cnls[2]->at(y+dy, x),
                                cnls[3]->at(y+dy, x));

            for (int c = 1; c <= 3; ++c)
                cnls[c]->clear(y+dy);
        }
    }

    for (int i = 1; i <= 3; ++i)
        delete cnls[i];
}
