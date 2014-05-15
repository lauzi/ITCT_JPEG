#include <cstdio>
#include <string>



class Decoder {
public:
    std::string in, out;

    Decoder (std::string i_in, std::string i_out): in(i_in), out(i_out), _has_read_ff(false) {}
    ~Decoder () { _close_files(); }

    bool solve();

    /*         -> x
            y|
            v            */
    static const int zigzag_x[] = {0,1,0,0,1,2,3,2,1,0,0,1,2,3,4,5,4,3,2,1,0,0,1,2,3,4,5,6,7,6,5,4,
                                   3,2,1,0,1,2,3,4,5,6,7,7,6,5,4,3,2,3,4,5,6,7,7,6,5,4,5,6,7,7,6,7};
    static const int zigzag_y[] = {0,0,1,2,1,0,0,1,2,3,4,3,2,1,0,0,1,2,3,4,5,6,5,4,3,2,1,0,0,1,2,3,
                                   4,5,6,7,7,6,5,4,3,2,1,2,3,4,5,6,7,7,6,5,4,3,4,5,6,7,7,6,5,6,7,7};

private:
    FILE *_IN, *_OUT;

    bool _has_read_ff;

    void _open_files() {
        _IN = fopen(in, "r");
        _OUT = fopen(out, "w");

        if (_IN == NULL)
            throw "Could not open input file";
        if (_OUT == NULL)
            throw "Could not open output file";
    }

    void _close_files() {
        if (_IN != NULL)
            fclose(_IN), _IN = NULL;
        if (_OUT != NULL)
            fclose(_OUT), _OUT = NULL;
    }

    size_t _read(void *ptr, size_t size, size_t count) {
        return fread(ptr, size, count, _IN);
    }

    int _rseek(long int offset, int origin) {
        return fseek(_IN, offset, origin);
    }

    size_t _write(const void *ptr, size_t size, size_t count) {
        return fwrite(ptr, size, count);
    }

    char _buf[8 * 8 * 8 * 8];
    uint8 _cs[256];
    uint8 _quantization_tables[4][8][8];
    uint8 _H_sampling_factor[256];
    uint8 _V_sampling_factor[256];
    uint8 _Qtable_selector[256];
    uint8 _DC_Huffman_selector[256];
    uint8 _AC_Huffman_selector[256];

    uint8 _Nf;

    Huffman _hs[4];

    void _SOF0();
    void _DHT();
    void _SOS();
    void _DQT();
    void _DRI();
};

bool Decoder::solve() {
    _open_files();

    while (_read_next_header());

    _close_files();

    return true;
}

// returns false if finished
bool Decoder::_read_next_header() {
    if (_has_read_ff) {
        _read(_buf+1, 1, 1);
    } else {
        _read(_buf, 1, 2);
        if (_buf[0] != 0xFF) throw "DAFUQ WHY NO 0xFF";
    }
    _has_read_ff = false;

    switch (_buf[1]) {
    case 0xC0: // SOF0, baseline DCT
        puts("SOF0");
        _SOF0();
        break;
    case 0xC4: // DHT, Define Huffman Table
        puts("DHT");
        _DHT();
        break;
    case 0xD8: // SOImage
        puts("SOI");
        break;
    case 0xD9: // EOImage
        puts("EOI");
        return false;
    case 0xDA: // SOScan
        puts("SOS");
        _SOS();
        break;
    case 0xDB: // Define quantization table
        puts("DQT");
        _DQT();
        break;
    case 0xDD: // DRI, define restart interval
        puts("DRI");
        _DRI();
        break;
    default:
        if ((_buf[1] >> 3) == 0xFC) { // RSTm
        } else {
            if ((_buf[1] >> 4) == 0xFE) // APPn
                puts("APP");
            else
                printf("%2X", _buf[1]);

            uint16 L;
            _read(&L, 2, 1);
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
    uint16 Y = (uint16)buf[1]<<8 | buf[2];
    uint16 X = (uint16)buf[3]<<8 | buf[4];

    _Nf = buf[5];
    printf("SOF0::N_f = %d\n", (int)_Nf);

    for (int i = 6, j = 1; j <= _Nf; ) {
        uint8 c = (_cs[j] = buf[i++]);
        _H_sampling_factor[c] = (buf[i]>>4) & 15;
        _V_sampling_factor[c] = buf[i++] & 15;
        _Qtable_selector[c] = buf[i++];
    }

    // READ SHITLOAD OF BLOCKS
}

Void Decoder::_DHT() {
    uint16 Lh;
    _read(&Lh, 2, 1);

    uint8 buf[Lh-2];
    _read(buf, 1, Lh-2);

    for (int i = 0, t = 1; t <= n; ++t) {
        int Tc = buf[i]>>4, Th = buf[i++] & 15;
        printf("DHT::T_c = %d, %s\n", Tc, Tc ? "AC" : "DC");
        printf("DHT::T_h = %d\n", Th);

        int Ls[17];
        printf("DHT::L_i = [");
        for (int j = 1; j <= 16; ++j) {
            Ls[j] = (int)buf[i++];
            printf("%d%s", Ls[j], j < 16 ? ", " : "]\n");
        }

        for (int j = 1; j <= 16; ++j)
            while (Ls[j]--)
                _hs[Th].insert(j, buf[i++]);
    }
}

void Decoder::_SOS() {
    uint16 Ls;
    _read(&Ls, 2, 1);

    uint8 buf[Ls-2];
    _read(buf, 1, Ls-2);

    int Ns = (int)buf[0];
    printf("SOS::N_s = %d\n", Ns);

    int i = 1;
    while (Ns--) {
        int c = (int)buf[i++];

        _DC_Huffman_selector[c] = (buf[i]>>4) & 15;
        _AC_Huffman_selector[c] = buf[i] & 15;
    }
}

void Decoder::_DQT() {
    uint16 Lq;
    _read(&Lq, 2, 1);

    uint8 buf[Lq];
    _read(buf, 1, Lq-2);

    for (int i = 0, t = 1; t <= n; ++t) {
        // P_q = 0, so buf[0] = T_q
        int Tq = (int)buf[i++];
        printf("DQT::T_q = %d\n", Tq;

        uint8 qt[8][8] = _quantization_tables[Tq];
        const int *x = zigzag_x, *y = zigzag_y;

        for (int j = 0; j < 64; ++j)
            qt[y[j]][x[j]] = buf[i++];
    }
}

void Decoder::_DRI();
