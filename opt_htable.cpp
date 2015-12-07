#include <queue>
#include <cassert>

#include "opt_htable.hpp"

struct Node {
    int w, label; Node *lft, *rgt;
    Node (int nw, int lbl, Node *nlft=NULL, Node *nrgt=NULL):
        w(nw), label(lbl), lft(nlft), rgt(nrgt) {}
    ~Node () {
        delete lft;
        delete rgt;
    }
};

void node_dfs(Node *n, unsigned lvl, std::vector<int> &lens) {
    if (n == NULL) return ;
    if (n->label != -1) {
        if (lvl >= lens.size()) lens.resize(lvl+1);
        lens[lvl] += 1;
    } else {
        node_dfs(n->lft, lvl+1, lens);
        node_dfs(n->rgt, lvl+1, lens);
    }
}

// not optimal, but still acceptable
void adjust_lens(std::vector<int> &lens) {
    while (lens.size()-1 > 16) {
        if (lens.back() == 0) {
            lens.pop_back();
            continue;
        }
        int i = lens.size()-1, j = i-2;
        while (lens[j] == 0) --j;
        lens[i] -= 2, lens[i-1] += 1, lens[j] -= 1, lens[j+1] += 2;
    }

    for (int i = 16; i >= 1; --i) {
        if (lens[i] > 0) {
            lens[i] -= 1;
            break;
        }
    }
}

HBook OptHTable::_optimize_table(const std::vector<int> &arr) {
    std::priority_queue<std::pair<int, Node*>> heap; // max heap
    std::vector<int> lens(17);

    for (unsigned i = 0; i < arr.size(); ++i)
        if (arr[i] > 0)
            heap.emplace(-arr[i], new Node(arr[i], i));
    heap.emplace(0, new Node(0, 257)); // to prevent all 1s

    while (heap.size() > 1) {
        auto x = heap.top(); heap.pop();
        auto y = heap.top(); heap.pop();
        int w = -(x.first + y.first);
        heap.emplace(-w, new Node(w, -1, x.second, y.second));
    }

    node_dfs(heap.top().second, 1, lens);
    adjust_lens(lens);
    delete heap.top().second;

    HBook nums(17);

    std::vector<std::pair<int, int>> ps;
    for (unsigned i = 0; i < arr.size(); ++i)
        if (arr[i] > 0)
            ps.emplace_back(arr[i], i);

    std::sort(std::begin(ps), std::end(ps));
    for (int i = 1, idx = ps.size()-1; i <= 16; ++i)
        for (int j = 0; j < lens[i]; ++j)
            nums[i].push_back(ps[idx--].second);

    printf("Optimized table:\n");
    for (int i = 1; i <= 16; ++i) {
        printf("%2d: ", i);
        for (auto x : nums[i])
            printf("%3d, ", x);
        puts("");
    }

    return nums;
}

HMap OptHTable::_make_map(const HBook &b) {
    HMap m;
    for (int lvl = 1, cval = 0; lvl <= 16; ++lvl, cval <<= 1)
        for (auto val : b[lvl])
            m[val] = {cval++, lvl};
    return m;
}
