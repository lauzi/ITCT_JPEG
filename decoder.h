#ifndef DECODER_H
#define DECODER_H

#include <cstdio>
#include <string>

#include "huffman.h"

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef char int8;
typedef short int16;
typedef unsigned int uint32;

class BMPWriter {
public:
    const int height, width;
    BMPWriter(int n_height, int n_width, const char *file_name):
        height(n_height), width(n_width),
        _pad_len((3 * width + 3) / 4 * 4 - 3 * width), _x(0) {

        _OUT = fopen(file_name, "wb");
        if (_OUT == NULL) throw "Could not open output file";

        _write_header();
    }

    ~BMPWriter() { fclose(_OUT); }

    void write_pxl(double y, double cb, double cr);
private:
    FILE *_OUT;
    const int _pad_len;
    int _x;

    size_t _write(const void *ptr, size_t size, size_t count) {
        return fwrite(ptr, size, count, _OUT);
    }

    void _write_header();
};


class MCUArr {
public:
    const int height, width;
    MCUArr (int n_height, int n_width):
        height(n_height), width(n_width), _mod(height-1) {

        for (int i = 0; i < height; ++i)
            _arr[i] = new double [width];
    }

    ~MCUArr () {
        for (int i = 0; i < height; ++i)
            delete [] _arr[i];
    }

    double& at(int y, int x) { return _arr[y&_mod][x]; }

    void clear(int y) { memset(_arr[y&_mod], 0, sizeof(double) * width); }

private:
    const int _mod;

    double *_arr[64];
};

class Decoder {
public:
    std::string in, out;

    Decoder (std::string i_in, std::string i_out): in(i_in), out(i_out),
                                                   _IN(NULL), _bmp(NULL),
                                                   _has_read_ff(false), _has_read_mark(false),
                                                   _bfr(NULL), _DC_predict(0) {}
    ~Decoder () { _close_files(); delete _bmp; }

    bool solve();

    /*         --> x
            y|
            v            */
    static const int zigzag_x[];
    static const int zigzag_y[];
private:
    FILE *_IN;
    BMPWriter *_bmp;

    bool _has_read_ff;
    bool _has_read_mark;
    uint8 _next_mark;

    uint8 *_bfr;
    int _bfr_idx;

    void _open_files();
    void _close_files();
    size_t _read(void *ptr, size_t size, size_t count);
    int _rseek(long int offset, int origin);

    uint16 _Y, _X;

    uint8 _cs[256];
    uint8 _quantization_tables[4][8][8];
    uint8 _H_sampling_factor[256]; uint8 _max_H;
    uint8 _V_sampling_factor[256]; uint8 _max_V;
    uint8 _Qtable_selector[256];

    uint8 _Ns;
    uint8 _scan_component_selector[5];
    uint8 _DC_Huffman_selector[5];
    uint8 _AC_Huffman_selector[5];

    uint8 _Nf;

    Huffman _hs[2][4];

    int16 _DC_predict;

    uint32 *_hc_data;
    int _hc_bfr_idx;
    int _hc_i;

    Huffman *_hc_DC, *_hc_AC;

    void _SOF0();
    void _DHT();
    void _SOS();
    void _DQT();
    void _DRI();

    bool _read_next_header();

    int _read_Huffman(Huffman *h);

    int16 _read_DC();
    int16 _read_AC(int &zz_idx);
    int _read_n_bits(int n);
    void _read_entropy_bytes();
    void _read_entropy_block(uint8 c, double out_block[8][8]);
    void _read_entropy_data();
};


#endif
