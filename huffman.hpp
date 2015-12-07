#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>
#include <vector>

#include "opt_htable.hpp"

typedef unsigned char uint8;

class Huffman {
public:
    Huffman(): opt(NULL), cleanup_opt(false) {
        tbls.push_back(new int [256]);
        counts.resize(256);
        hb.resize(17);
        clear();
    }
    ~Huffman() {
        for (size_t i = 0; i < tbls.size(); ++i)
            delete [] tbls[i];
        if (cleanup_opt) delete opt;
    }
    void insert(int len, uint8 val);

    void clear() { _ins_tbl = _ins_idx = 0, _unit = 1<<7, _prev_len = 1; }

    std::vector<int*> tbls;
    std::vector<int> counts;

    void gen_opt();

    OptHTable* opt;
    bool cleanup_opt;
    HBook hb;
private:
    int _ins_tbl, _ins_idx, _unit, _prev_len;
};

#endif
