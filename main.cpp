#include <cstdio>
#include <cstring>
#include <string>

#include "recoder.hpp"

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
        file_out_out_name += "_optimized.jpg";
    } else {
        file_out_name.replace(point_idx, 4, ".bmp");
        file_out_out_name.replace(point_idx, 4, "_optimized.jpg");
    }

    printf("Input file name: %s\n", file_in_name.c_str());

    try {
        printf("Output jpeg->optimized jpeg file name: %s\n", file_out_out_name.c_str());
        Recoder(file_in_name, file_out_out_name).run();
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
