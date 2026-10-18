# p_codec

p_codec is a still image codec primarily based on the JPEG specification, with some H.263 characteristics in its entropy coding.

## Usage

```
./p_codec encode -w <width> -h <height> -i <input> -o <output>
./p_codec decode -i <compressed>
./p_codec metrics -i <original> -d <decoded> -e <encoded>
```

- `encode` takes a raw RGB24 file (3 bytes per pixel, no header) and writes the compressed file.
- `decode` reads a compressed file and writes `decoded.rgb` (plus `decoded.yuv`) in the current directory.
- `metrics` prints the compression ratio and PSNR. The decoded file may be longer than the original (padding at the end is ignored).

To convert an image to raw RGB24:

```
ffmpeg -i image.png -pix_fmt rgb24 -f rawvideo image.rgb
```

## Build

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

Or directly with g++:

```
g++ -std=c++17 -O2 -Iheaders main.cpp implementations/*.cpp -o p_codec
```

## Design

The application is divided into three main classes:

1. **chroma**: converts raw RGB24 into YUV 4:2:0 subsampled data, and back.
2. **dct**: performs the discrete cosine transform and quantization on 8x8 blocks of the image.
3. **entropy**: run-level and Huffman encodes the transformed and quantized coefficients.

Each class also has the matching decode functions. The **coder** class combines them and orchestrates the encode and decode flow.

## Results on the Kodak image suite

Tested on the 24 images of the [Kodak Lossless True Color Image Suite](https://r0k.us/graphics/kodak/), converted to raw RGB24.

- Quantization: fixed standard JPEG quantization tables (no quality setting)
- Chroma subsampling: 4:2:0
- Ratio: raw RGB24 size divided by compressed file size
- PSNR: computed over all RGB bytes of the decoded image against the original

| Image | Resolution | Raw size (B) | Compressed (B) | Ratio | PSNR (dB) |
|---|---|---:|---:|---:|---:|
| kodim01 | 768x512 | 1,179,648 | 71,019 | 16.61x | 29.71 |
| kodim02 | 768x512 | 1,179,648 | 37,603 | 31.37x | 32.41 |
| kodim03 | 768x512 | 1,179,648 | 34,496 | 34.20x | 34.13 |
| kodim04 | 512x768 | 1,179,648 | 41,756 | 28.25x | 32.84 |
| kodim05 | 768x512 | 1,179,648 | 81,777 | 14.43x | 29.32 |
| kodim06 | 768x512 | 1,179,648 | 57,030 | 20.68x | 31.02 |
| kodim07 | 768x512 | 1,179,648 | 44,783 | 26.34x | 33.48 |
| kodim08 | 768x512 | 1,179,648 | 82,436 | 14.31x | 29.24 |
| kodim09 | 512x768 | 1,179,648 | 36,472 | 32.34x | 34.14 |
| kodim10 | 512x768 | 1,179,648 | 39,686 | 29.72x | 33.87 |
| kodim11 | 768x512 | 1,179,648 | 52,137 | 22.63x | 31.51 |
| kodim12 | 768x512 | 1,179,648 | 36,575 | 32.25x | 34.29 |
| kodim13 | 768x512 | 1,179,648 | 87,312 | 13.51x | 27.51 |
| kodim14 | 768x512 | 1,179,648 | 64,115 | 18.40x | 29.89 |
| kodim15 | 768x512 | 1,179,648 | 38,594 | 30.57x | 32.72 |
| kodim16 | 768x512 | 1,179,648 | 42,613 | 27.68x | 33.26 |
| kodim17 | 512x768 | 1,179,648 | 43,570 | 27.07x | 33.49 |
| kodim18 | 512x768 | 1,179,648 | 62,375 | 18.91x | 30.10 |
| kodim19 | 512x768 | 1,179,648 | 48,912 | 24.12x | 32.18 |
| kodim20 | 768x512 | 1,179,648 | 36,279 | 32.52x | 33.23 |
| kodim21 | 768x512 | 1,179,648 | 50,841 | 23.20x | 31.29 |
| kodim22 | 768x512 | 1,179,648 | 49,333 | 23.91x | 31.44 |
| kodim23 | 768x512 | 1,179,648 | 32,409 | 36.40x | 34.42 |
| kodim24 | 768x512 | 1,179,648 | 61,907 | 19.06x | 29.81 |
| **Average** | | | | **24.94x** | **31.89** |
