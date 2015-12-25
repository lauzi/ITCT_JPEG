#include <map>
#include <list>
#include <climits>
#include <cassert>

#include "opt_htable.hpp"

// not optimal, but still acceptable
std::vector<int> adjust_lens(std::vector<int> lens) {
    for (int i = lens.size()-1, j = i-2; i > 16; ) {
        if (lens[i] == 0) {
            i -= 1, j = std::min(j, i-2);
            continue;
        }

        while (lens[j] == 0) --j;
        int n = std::min(lens[i]/2, lens[j]);
        lens[i] -= 2*n, lens[i-1] += n, lens[j] -= n, lens[j+1] += 2*n;
        j = std::min(j+1, i-2);
    }

    lens.resize(1+16);

    for (int i = 16; i >= 1; --i) {
        if (lens[i] > 0) {
            lens[i] -= 1;
            break;
        }
    }

    return lens;
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

std::vector<int> huffman_lens(const std::vector<int> &arr) {
    std::vector<int> queue, tree;
    for (auto x: arr)
        if (x > 0) queue.push_back(x);
    queue.push_back(0);
    std::sort(queue.begin(), queue.end());

    std::vector<std::pair<int, int>> children;
    const int LEAF = -1;

    for (unsigned iq = 0, it = 0; ; ) {
        std::pair<int, int> best{INT_MAX, -1}; // value, mode

        if (iq+1 < queue.size())
            best = std::min(best, {queue[iq]+queue[iq+1], 0});

        if (iq < queue.size() and it < tree.size())
            best = std::min(best, {queue[iq]+tree[it], 1});

        if (it+1 < tree.size())
            best = std::min(best, {tree[it]+tree[it+1], 2});

        if (best.second == -1) break;
        else if (best.second == 0) children.emplace_back(LEAF, LEAF), iq += 2;
        else if (best.second == 1) children.emplace_back(LEAF, it), iq += 1, it += 1;
        else if (best.second == 2) children.emplace_back(it, it+1), it += 2;

        tree.push_back(best.first);
    }

    std::vector<int> lens(16+1);
    std::vector<unsigned> depths(children.size()); // depths[-1] == 0

    for (int i = children.size()-1; i >= 0; --i) {
        const unsigned newd = depths[i]+1;
        if (newd >= lens.size())
            lens.resize(newd+1);

#define DO(x) if ((x) == LEAF) lens[newd] += 1; else depths[x] = newd;

        DO(children[i].first);
        DO(children[i].second);
    }

    return lens;
}

HBook OptHTable::_optimize_table(const std::vector<int> &arr, bool opt) {
    if (opt) return _opt_optimize_table(arr);

    return make_book(arr, adjust_lens(huffman_lens(arr)));
}


using Width = int;
using Weight = std::pair<int, int>; // (actual weight, index)
using item_t = std::pair<Width, Weight>;
using mitem_t = std::pair<Weight, std::list<item_t>>;

inline int __lg(int __n) {
    return sizeof(int) * __CHAR_BIT__  - 1 - __builtin_clz(__n);
}

template <typename T>
inline
void add_to(std::vector<T> &xs, const std::vector<T> &ys) {
    xs.insert(xs.end(), ys.begin(), ys.end());
}

template <typename T>
inline
void add_to(std::list<T> &xs, std::list<T> &ys) {
    // This empties ys and should be in constant time according to cppreference.com
    xs.splice(xs.begin(), ys);
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

std::vector<mitem_t> pm_pair(std::vector<mitem_t> arr) {
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
            for (const auto &x : arr[d][0].second)
                ans.push_back(x);
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

    return make_book(arr, adjust_lens(lens)); // adjust_lens to remove the 0
}


HMap OptHTable::_make_map(const HBook &b) {
    HMap m;
    for (int lvl = 1, cval = 0; lvl <= 16; ++lvl, cval <<= 1)
        for (auto val : b[lvl])
            m[val] = {cval++, lvl};

    printf("Optimized table:\n");
    for (int i = 1; i <= 16; ++i) {
        if (b[i].size() == 0) continue;

        printf("%2d:", i);
        for (auto x : b[i])
            printf(" %3d,", x);
        puts("\b ");
    }

    return m;
}
