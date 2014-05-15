#include <cstdio>
#include <cstring>
#include <string>

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

    FILE *file_in = fopen(file_in_name.c_str(), "r");
    if (file_in == NULL) {
        puts("Could not open input file!!");
        return -1;
    }

    FILE *file_out = fopen(file_out_name.c_str(), "w");
    if (file_out == NULL) {
        puts("Could not open output file!!");
        return -1;
    }

    decode(file_in, file_out);

    fclose(file_in);
    fclose(file_out)

    return 0;
}
