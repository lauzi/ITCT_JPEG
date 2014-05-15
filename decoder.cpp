#include <cstdio>
#include <cmath>

typedef unsigned char uint8;
typedef char int8;

// need huffman table
void ac_zigzag_to_block(int8 output[8][8], const int8 *input) {
    memset((T*)output + 1, sizeof T, 63);

    int input_i = 0;

    for (int i = 1; i < 64; ) {
        int runlength = (int)(*input) >> 4;
        int size = *input & 15;
        i += runlength;

        if (runlength == 0 and size == 0) break; // EOB

        output[zigzag_x[i]][zigzag_y[i]] = *(input++);
    }
}

void dequantize(int16 output[8][8], const int8 S_q[8][8], const int8 Q[8][8]) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            output[i][j] = (int16)S_q[i][j] * Q[i][j];
        }
    }
}

void idct(uint8 output[8][8], const int16 input[8][8]) {
    static double coeff[8][8][8][8];

    if (coeff[0][0][0][0] == 0.0) {
        double c[8];
        c[0] = M_SQRT1_2;
        for (int i = 1; i < 8; ++i)
            c[i] = 1.0;

        double coss[8][8];
        for (int i = 0; i < 8; ++i)
            for (int u = 0; u < 8; ++u)
                coss[i][u] = cos(M_PI * (2*i+1) * u / 16);

        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                for (int u = 0; u < 8; ++u)
                    for (int v = 0; v < 8; ++v)
                        coeff[i][j][u][v] = c[u] * c[v] * coss[i][u] * coss[j][v];
    }

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            double sum = 128.0;
            for (int u = 0; u < 8; ++u)
                for (int v = 0; v < 8; ++v)
                    sum += coeff[i][j][u][v] * input[u][v];
            output = (uint8) sum;
        }
    }
}

int main(int argc, char *argv[]) {
    const char *file_name = "monalisa.jpg";

    FILE *fin =

    return 0;
}
