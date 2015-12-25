#ifndef DECODER_H
#define DECODER_H

#include <cstdio>
#include <cstring>
#include <string>

#include "huffman.hpp"

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef char int8;
typedef short int16;
typedef unsigned int uint32;

class OptHTable;

class BMPWriter {
public:
    const int height, width;
    BMPWriter(int n_height, int n_width, const char *file_name):
        height(n_height), width(n_width),
        _bfr_len((3 * width + 3) / 4 * 4), _x(0),
        _out_bfr(NULL) {

        _OUT = fopen(file_name, "wb");
        if (_OUT == NULL) throw "BMPWriter::Could not open output file";

        _out_bfr = new uint8 [_bfr_len]();

        _write_header();
    }

    ~BMPWriter() { fclose(_OUT); delete [] _out_bfr; }

    void write_pxl(double y, double cb, double cr);
private:
    FILE *_OUT;
    const int _bfr_len;
    int _x;

    uint8 *_out_bfr;

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
                                                   _bfr(NULL), _out_bfr(NULL),
                                                   _count_a(0), _count_b(0),
                                                   _DC_predict(0),
                                                   _replaced_dht(false),
                                                   _pad_count(0) {}
    ~Decoder () { _close_files(); delete _bmp; }

    bool solve();
    std::vector<int> huffman_stats(int i, int j);
    void set_optimal_table(int i, int j, OptHTable *t) { _hs[i][j].opt = t; }
    int save_to_file(std::string out);

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

    int _bfr_size;
    uint8 *_bfr;
    int _bfr_idx;

    bool _auto_out;
    uint8 *_out_bfr;
    int _out_bfr_idx;

    int _count_a;
    int _count_b;

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

    bool _replaced_dht;

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

    void _write(const void *ptr, size_t size, size_t count, bool force=false);
    unsigned _pad_count;
    void _write_bits(uint32 i, int bits, bool force=false);
    void _write_byte(uint8 b, bool force=false) { _write(&b, 1, 1, force); }

    void _write_opt_tables();
};


#endif
