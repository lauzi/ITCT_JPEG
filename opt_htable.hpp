#ifndef OPT_HTABLE_H
#define OPT_HTABLE_H

#include <vector>
#include <unordered_map>

using HBook = std::vector<std::vector<int>>;
using HPair = std::pair<int, int>;
using HMap = std::unordered_map<int, HPair>;

class OptHTable {
    public:
    OptHTable (const HBook &b): book(b), map(_make_map(b)) {}
    OptHTable (const std::vector<int> &v, bool opt):
        book(_optimize_table(v, opt)),
        map(_make_map(book)) {}

    const HBook book;
    const HMap map;

    private:
    static HBook _optimize_table(const std::vector<int> &arr, bool opt);
    static HBook _opt_optimize_table(const std::vector<int> &arr);
    static HMap _make_map(const HBook&);
};


#endif
