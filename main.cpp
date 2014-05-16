#include <cstdio>
#include <cstring>
#include <string>

#include "decoder.h"

int main(int argc, char *argv[]) {
    std::string file_in_name, file_out_name;

    if (argc >= 2) {
        int len = strlen(argv[1]);

        int point_idx = -1;
        for (int i = len-1; i >= 0; --i) {
            if (argv[1][i] == '/' or
                argv[1][i] == '\\') break;

            if (argv[1][i] == '.') {
                point_idx = i;
                break;
            }
        }

        file_in_name = file_out_name = argv[1];

        if (point_idx == -1) {
            file_in_name += ".jpg";
            file_out_name += ".bmp";
        } else {
            file_out_name.replace(point_idx, 4, ".bmp");
        }
    } else {
        file_in_name = "monalisa.jpg";
        file_out_name = "monalisa.bmp";
    }

    try {
        Decoder(file_in_name, file_out_name).solve();
    } catch (const char *err_msg) {
        puts(err_msg);
    } catch (std::string str) {
        puts(str.c_str());
    }

    return 0;
}
