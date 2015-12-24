#include <set>
#include <map>
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

HBook make_book(const std::vector<int> &arr, const std::vector<int> &lens) {
    HBook nums(17);

    std::vector<std::pair<int, int>> ps;
    for (unsigned i = 0; i < arr.size(); ++i)
        if (arr[i] > 0)
            ps.emplace_back(arr[i], i);

    std::sort(std::begin(ps), std::end(ps));
    for (int i = 1, idx = ps.size()-1; i <= 16; ++i)
        for (int j = 0; j < lens[i]; ++j)
            nums[i].push_back(ps[idx--].second);

    return nums;
}

HBook OptHTable::_optimize_table(const std::vector<int> &arr, bool opt) {
    if (opt) return _opt_optimize_table(arr);

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

    node_dfs(heap.top().second, 0, lens);
    adjust_lens(lens);
    delete heap.top().second;

    return make_book(arr, lens);
}


using Width = int;
using Weight = std::pair<int, int>; // (actual weight, index)
using item_t = std::pair<Width, Weight>;
using mitem_t = std::pair<Weight, std::vector<item_t>>;

inline int __lg(int __n) {
    return sizeof(int) * __CHAR_BIT__  - 1 - __builtin_clz(__n); 
}

template <typename T>
inline
void add_to(std::vector<T> &xs, const std::vector<T> &ys) {
    xs.insert(xs.end(), ys.begin(), ys.end());
}

template <typename T>
std::vector<T> pm_merge(const std::vector<T> &xs, const std::vector<T> &ys) {
    std::vector<T> ans;

    auto itx = xs.begin(), ity = ys.begin();
    while (itx != xs.end() and ity != ys.end()) {
        if (*itx < *ity)
            ans.push_back(*itx), ++itx;
        else
            ans.push_back(*ity), ++ity;
    }

    ans.insert(ans.end(), itx, xs.end());
    ans.insert(ans.end(), ity, ys.end());

    return ans;
}

template <typename A, typename B>
inline
std::pair<A, B> pm_add(const std::pair<A, B> &x, const std::pair<A, B> &y) {
    return {x.first+y.first, x.second+y.second};
}

std::vector<mitem_t> pm_pair(const std::vector<mitem_t> arr) {
    std::vector<mitem_t> ans;

    for (auto ita = arr.begin(); ita != arr.end(); ita += 2) {
        auto itb = ita + 1;
        if (itb == arr.end()) break;

        ans.emplace_back(pm_add(ita->first, itb->first), ita->second);
        add_to(ans.back().second, itb->second);
    }

    return ans;
}

std::vector<item_t> package_merge(std::vector<item_t> arr_in, Width X) {
    std::sort(arr_in.begin(), arr_in.end());

    std::vector<item_t> ans;

    const int L = __lg(X);
    std::vector<std::vector<mitem_t>> arr(L+3);

    for (const item_t &p : arr_in)
        arr[__lg(p.first)].push_back(mitem_t{p.second, {p}});

    for (int d = 0; X > 0; ++d) {
        const int min_width = X & -X;

        while (arr[d].size() == 0)
            if (++d >= (int)arr.size())
                throw "No solution: sum width < X";

        const int r = 1<<d;
        if (r > min_width) throw "No solution: too few shit";
        if (r == min_width) {
            add_to(ans, arr[d][0].second);
            arr[d].erase(arr[d].begin());
            X -= r;
        }

        arr[d+1] = pm_merge(arr[d+1], pm_pair(arr[d]));
    }

    return ans;
}


using weight = std::pair<int, int>;

HBook OptHTable::_opt_optimize_table(const std::vector<int> &arr) {
    std::vector<int> freqs(arr);
    freqs.push_back(0);
    std::sort(freqs.begin(), freqs.end());

    std::vector<item_t> noteset;
    int n = 0;
    for (unsigned i = 0; i < freqs.size(); ++i) {
        if (freqs[i] == 0 and i > 0) continue; // to prevent all 0s

        n += 1;
        for (int l = 0; l < 16; ++l)
            noteset.push_back({1<<l, {freqs[i], (i+1)<<4|l}});
    }

    auto ans = package_merge(noteset, (n-1)<<16);

    std::vector<int> cnts(260);
    for (const item_t &p: ans)
        cnts[p.second.second >> 4] += 1;

    std::vector<int> lens(16+1);
    for (const int x: cnts) lens[x] += 1;

    for (int i = 16; i >= 1; --i) { // remove the fucking 0-chance codeword
        if (lens[i] == 0) continue;

        lens[i] -= 1;
        break;
    }

    return make_book(arr, lens);
}


HMap OptHTable::_make_map(const HBook &b) {
    HMap m;
    for (int lvl = 1, cval = 0; lvl <= 16; ++lvl, cval <<= 1)
        for (auto val : b[lvl])
            m[val] = {cval++, lvl};

    printf("Optimized table:\n");
    for (int i = 1; i <= 16; ++i) {
        if (b[i].size() == 0) continue;
        if (false) {
            printf("%2d: \n", i);
            for (auto x : b[i]) {
                printf("  ");
                for (int j = i-1; j >= 0; --j)
                    printf("%d", m[x].first>>j&1);
                printf(": %3d\n", x);
            }
            puts("");
        } else {
            printf("%2d: ", i);
            for (auto x : b[i])
                printf("%3d, ", x);
            puts("");
        }
    }

    return m;
}
