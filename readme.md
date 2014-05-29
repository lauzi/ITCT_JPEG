# Usage
./main.exe takes a jgp file name as argument, transforms it into a bmp file, and then uses
the bmp file to generate another jpg file.

1. make
2. ./main.exe jpeg_file_name[.jpg]
3. A decoded .bmp file with the same filename will be generated at the current directory.
4. An encoded .jpg file ``jpeg _ file _ name _ from _ bmp.jpg'' will also be generated.

You can also "make test" to auto-test the 6 files appended.

# Implementation Details
## Decoder
- Reads whole file to memory.
- Uses 1D Fast IDCT to achieve IDCT.
- Uses 2-lebel 8-bit Huffman tables to decode Huffman codes.

## Encoder
- Buffered reading and writing.
- No options, can only transform to 4:2:0.
- Quantization tables and Huffman Tables are from the standard.
- Can only read 24-bit bmps with negative height (i.e. pixels are written from top-left to bottom-right).
