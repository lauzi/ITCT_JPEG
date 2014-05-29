#include <cstdio>
#include <cstring>
#include <string>

#include "decoder.h"
#include "encoder.h"

void call(const char *file_name) {
    std::string file_in_name, file_out_name, file_out_out_name;

    int len = strlen(file_name);

    int point_idx = -1;
    for (int i = len-1; i >= 0; --i) {
        if (file_name[i] == '/' or
            file_name[i] == '\\') break;

        if (file_name[i] == '.') {
            point_idx = i;
            break;
        }
    }

    file_in_name = file_out_name = file_out_out_name = file_name;

    if (point_idx == -1) {
        file_in_name += ".jpg";
        file_out_name += ".bmp";
        file_out_out_name += "_from_bmp.jpg";
    } else {
        file_out_name.replace(point_idx, 4, ".bmp");
        file_out_out_name.replace(point_idx, 4, "_from_bmp.jpg");
    }

    printf("Input file name: %s\n", file_in_name.c_str());

    try {
        printf("Output jpeg->bmp file name: %s\n", file_out_name.c_str());
        Decoder(file_in_name, file_out_name).solve();

        printf("Output jpeg->bmp->jpeg file name: %s\n", file_out_out_name.c_str());
        Encoder(file_out_name, file_out_out_name).solve();
    } catch (const char *err_msg) {
        puts(err_msg);
    } catch (std::string str) {
        puts(str.c_str());
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        call("monalisa");
    } else {
        for (int i = 1; i < argc; ++i)
            call(argv[i]);
    }

    return 0;
}
