# Usage
./main takes a jgp file name as argument, and optimzes its Huffman table to produce smaller files.

1. make
2. ./main jpeg_file_name[.jpg]
3. An optimized .jpg file ``jpeg _ file _ name _ optimized.jpg'' will also be generated.

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

## Recoder
- Shitty hacks
- Uses the shitty algorithm in annex K to keep codeword lengths less than 16.
- Two-pass, and uses ``/tmp/tmp.bmp''.
