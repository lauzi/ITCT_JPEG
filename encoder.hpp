#ifndef ENCODER_H
#define ENCODER_H

#include <string>
#include <vector>

typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;

class BMPReader {
public:
    int height, width;

    BMPReader(const char *file_name);
    ~BMPReader() { delete [] _bfr; fclose(_IN); }

    void read_line(double *y, double *cb, double *cr);

private:
    FILE *_IN;

    uint8 *_bfr;
    int _bfr_siz, _bfr_idx;
    int _line_left;

    void _read_header();
};


class HTable {
public:
    int key[256], len[256];

    HTable (uint8 L[], uint8 vals[]) {
        for (int i = 0, idx = 0, cval = 0; i < 16; ++i, cval <<= 1)
            for (int j = 0; j < L[i]; ++j, ++idx)
                key[vals[idx]] = cval++, len[vals[idx]] = i+1;
    }
};


const size_t bfr_size = 1024 * 1024;
class Encoder {
public:
    std::string in, out;

    Encoder (std::string i_in, std::string i_out): in(i_in), out(i_out), _reader(in.c_str()) {
        _height = _reader.height, _width = _reader.width;

        _OUT = fopen(out.c_str(), "wb");
        if (_OUT == NULL)
            throw "Encoder::Could not open output file";

        _bfr = new uint8 [bfr_size];
        _bfr_i = 0, _bfr_j = 8;
    }

    ~Encoder () { fclose(_OUT); delete [] _bfr; }

    bool solve();

private:
    BMPReader _reader;
    int _height, _width;

    std::vector<HTable> _hTable;

    FILE *_OUT;
    uint8 *_bfr; int _bfr_i, _bfr_j;

    void _write(const void *ptr, size_t size, size_t count);
    void _write_bits(int val, int len);

    std::vector<double> _cnl[3][16];
    double _block[8][8];
    int32 _DC_pred[3];

    void _write_DQTs();
    void _write_SOF0();
    void _write_DHTs();
    void _write_SOS();
    void _write_MCUs();
    void _write_MCU(int ox);
    void _write_block(int32 &DC_pred, int is_color);
    void _write_Huffman(int key, int val, int lg, HTable *table);
};

#endif
