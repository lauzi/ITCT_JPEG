#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>

typedef unsigned char uint8;

struct Node {
    Node *cld[2];
    uint8 val;
    int cnt;

    Node (): cld(), val(-1), cnt(0) {}
    ~Node() { delete cld[0]; delete cld[1]; }

    void print(std::string path = "") const;
};


class Huffman {
public:
    Huffman(): _root(new Node) {}
    ~Huffman() { delete _root; }

    void insert(int len, uint8 val);
    void print() const { _root->print(); }

    Node* root() { return _root; }

    void clear() { delete _root; _root = new Node; }
private:
    Node *_root;
};


#endif
