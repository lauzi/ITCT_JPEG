#include <cstdio>

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

void Node::print(std::string path) const {
    if (cld[0] == NULL && cld[1] == NULL)
        printf("%s -> 0x%02x\n", path.c_str(), (int)val);

    for (int i = 0; i < 2; ++i)
        if (cld[i] != NULL)
            cld[i]->print(path + (char)('0' + i), subtree_size>>1);
}


class Huffman {
public:
    Huffman(): _root(new Node) {}
    ~Huffman() { delete _root; }

    void insert(int len, uint8 val);
    void print() const { _root->print(); }

    void clear() { delete _root; _root = new Node; }
private:
    Node *_root;
};

void Huffman::insert(int len, uint8 val) {
    int size_needed = 1 << (16-len);
    Node *cur = _root;
    int sub_max_size = 1<<15;
    for (int i = 0; i < len; ++i, sub_max_size >>= 1) {
        cur->cnt += size_needed;

        if (cur->cld[0] == NULL) cur->cld[0] = new Node;

        int pick = cur->cld[0]->cnt + size_needed <= sub_max_size ? 0 : 1;

        if (cur->cld[pick] == NULL) cur->cld[pick] = new Node;
        cur = cur->cld[pick];
    }
    cur->cnt += size_needed;
    cur->val = val;
}


int main() {
    int li[] = {0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125};
    uint8 vi[] = {
        1, 2, 3, 0, 4, 17, 5, 18, 33, 49, 65, 6, 19, 81, 97, 7, 34, 113, 20, 50, 129, 145, 161, 8, 35, 66, 177, 193, 21, 82, 209, 240, 36, 51, 98, 114, 130, 9, 10, 22, 23, 24, 25, 26, 37, 38, 39, 40, 41, 42, 52, 53, 54, 55, 56, 57, 58, 67, 68, 69, 70, 71, 72, 73, 74, 83, 84, 85, 86, 87, 88, 89, 90, 99, 100, 101, 102, 103, 104, 105, 106, 115, 116, 117, 118, 119, 120, 121, 122, 131, 132, 133, 134, 135, 136, 137, 138, 146, 147, 148, 149, 150, 151, 152, 153, 154, 162, 163, 164, 165, 166, 167, 168, 169, 170, 178, 179, 180, 181, 182, 183, 184, 185, 186, 194, 195, 196, 197, 198, 199, 200, 201, 202, 210, 211, 212, 213, 214, 215, 216, 217, 218, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250
    };

    int j;
    for (j = 0; li[j] == 0; ++j);

    Huffman tree;
    for (int i = 0, cnt = li[j]; i < sizeof vi; ++i) {
        tree.insert(j+1, vi[i]);
        if (--cnt == 0) cnt = li[++j];
    }
    tree.print();
    return 0;
}
