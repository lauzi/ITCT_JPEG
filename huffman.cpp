#include <cstdio>
#include "huffman.hpp"

void Huffman::insert(int len, uint8 val) {
    hb[len].push_back(val);

    if (len > 8) {
        if (_ins_tbl == 0)
            for (int tbl = -1; _ins_idx < 256; ) {
                tbls.push_back(new int [256]);
                tbls[0][_ins_idx++] = tbl--;
            }

        len -= 8;
    }

    if (_ins_idx >= 256)
        _ins_tbl += 1, _ins_idx = 0, _unit = 1<<7, _prev_len = 1;

    _unit >>= len - _prev_len;
    _prev_len = len;

    int new_idx = (len << 8) | val;
    for (int i = 0; i < _unit; ++i)
        tbls[_ins_tbl][_ins_idx++] = new_idx;
}

void Huffman::gen_opt() {
    opt = new OptHTable(hb);
    cleanup_opt = true;
}
