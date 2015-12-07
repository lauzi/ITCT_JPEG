#include "recoder.hpp"
#include "opt_htable.hpp"
#include "decoder.hpp"

bool Recoder::run() {
    _get_counts();

    _print_counts();

    Decoder dec(in, "/tmp/tmp.bmp");

    OptHTable* tabs[2][2];
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            tabs[i][j] = new OptHTable(_counts[i][j]);

            dec.set_optimal_table(i, j, tabs[i][j]);
        }
    }

    dec.solve();
    dec.save_to_file(out);

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            delete tabs[i][j];

    return true;
}

void Recoder::_get_counts() {
    Decoder first_pass(in, "/tmp/tmp.bmp");
    first_pass.solve();
    // first_pass.save_to_file("/tmp/tmp.jpg");

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            _counts[i][j] = first_pass.huffman_stats(i, j);
}

void Recoder::_print_counts() {
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            std::vector<std::pair<int, int>> ps;
            for (int k = 0; k < 256; ++k)
                ps.emplace_back(_counts[i][j][k], k);
            std::sort(std::begin(ps), std::end(ps));
            printf("Frequency count of %d, %d:\n", i, j);
            for (int i = 255; i >= 0 and ps[i].first > 0; --i)
                printf("%4d: %7d\n", ps[i].second, ps[i].first);
            puts("");
        }
    }
}
