# Decode Loop Plan

This document lays out the theoretical decode pipeline for `p_codec`, mirroring
the exact structure of the existing encoder (`chroma` -> `dct` -> `entropy`).
It is a plan only — no decoder implementation exists yet.

## Encoder recap (relevant details for decoding)

- Height/width are padded to the next multiple of 16.
- `dct::performDCT` treats the buffer as full-resolution Y plus half-resolution
  Cb/Cr (4:2:0 layout: Y plane, then Cb plane, then Cr plane, concatenated).
- `dctOn8x8` performs forward DCT and quantization in a single step, writing
  the result back into the same matrix in place.
- `entropy::runLevel` processes **all Y blocks, then all Cb blocks, then all
  Cr blocks** — not interleaved per macroblock.
- `bitWriter::flush()` is called **once, at the very end** of `runLevel`, not
  per block. The entire compressed file is therefore one continuous
  bitstream with no per-block byte alignment.
- The DC coefficient is written as a raw signed 8-bit value (`char DC = dc`).
  If a quantized DC value falls outside -128..127, this truncates and is
  lossy — worth verifying against the quant tables before relying on it.
- AC coefficients use a variable-length Huffman/VLC scheme
  (`tcoeffTableClass::vlc_table`), keyed by `(last, run, level)`, plus a
  7-bit escape code `0000011` for triples not present in the table.
- There is currently **no header** in the `compressed` output file — original
  width/height/padding are not stored anywhere in the bitstream.

## 1. Bit-level reader (mirror of `bitWriter`)

Implement a `bitReader` that consumes the compressed buffer bit by bit.
Because flush only happens once at the end of encoding, the reader must
maintain a persistent bit cursor across the whole file — there is no
block-level realignment to reset on.

Needs:
- Read 1 bit at a time (for VLC prefix matching).
- Read N bits at a time (for fixed-width fields: DC, hasAC flag, escape
  fields).

## 2. Per-8x8-block decode loop

For each plane, in the same order the encoder used
(**all Y blocks -> all Cb blocks -> all Cr blocks**), for each block
position `(i, j)`:

1. **DC**: read 8 bits, interpret as a signed `int8_t` (mirrors
   `char DC = dc` on encode).
2. **hasAC flag**: read 1 bit.
   - If 0, the block has no AC energy — skip to step 5 with an all-zero
     AC array.
3. **AC run-level loop** (only if hasAC == 1). Repeat:
   - Read bits one at a time, checking the accumulated prefix against a
     reverse-built lookup of `vlc_table` (a map from `(bits, length)` to
     `(last, run, level)`), while also watching for the 7-bit escape
     prefix `0000011`.
   - If a normal VLC code matches: read 1 more bit for the sign, negate
     `level` if the sign bit is 1.
   - If the escape code matches: read 1 bit (`last`), 6 bits (`run`), 8
     bits (`level`, signed two's-complement).
   - Append `(last, run, level)` to this block's pair list.
   - Stop when a pair with `last == 1` has been read.
4. **Rebuild the 64-element zigzag array**: place DC at index 0, then for
   each `(last, run, level)` pair, insert `run` zeros followed by `level`,
   continuing until the array holds 64 entries.
5. **Inverse zigzag**: scatter the 64 values back into an 8x8 matrix using
   the exact traversal path `runLevelon8x8` used to scan it originally.
   Since that traversal is a custom (non-library) implementation, generate
   its (row, col) sequence once as a lookup table and reuse it in reverse
   for decoding.
6. **Dequantize**: multiply each cell element-wise by `luminanceTable` (for
   Y) or `chrominanceTabe` (for Cb/Cr) — the inverse of the divide-and-round
   done in `dctOn8x8`.
7. **Inverse DCT (IDCT)** on the dequantized 8x8 block to recover spatial
   residuals.
8. **Undo level shift**: add 128 back to every pixel (inverse of the `-128`
   applied in `performDCT`).
9. Write the reconstructed 8x8 block into the plane's output matrix at
   `[i][j]`.

## 3. Reassemble the YUV420p buffer

Concatenate the Y plane (height x width), then Cb, then Cr (each
height/2 x width/2) — matching the layout `chroma::rgbToyuv` produced.

## 4. Chroma upsampling (4:2:0 -> 4:4:4)

Each Cb/Cr sample was the average of a 2x2 luma-resolution block during
encoding. On decode, replicate (nearest-neighbor) or bilinearly interpolate
each chroma sample back across its corresponding 2x2 region.

## 5. YCbCr -> RGB

Invert the equations from `chroma::rgbToyuv`:

- `Y  = 0.299 R + 0.587 G + 0.114 B`
- `Cb = 128 + 0.564 (B - Y)`
- `Cr = 128 + 0.713 (R - Y)`

Solve algebraically for R, G, B per pixel and clamp to the 0-255 range.

## 6. Strip padding

Crop from the 16-aligned padded dimensions back down to the original
height/width.

## 7. Write raw RGB output

Mirror the raw file format `chroma::readFile` expects on the encode side.

## Open gaps to resolve before implementation

1. **No header in `compressed`.** The decoder has no way to recover the
   original width/height/padding from the file alone. A small fixed header
   (original width, original height) should be prepended to the bitstream
   before a standalone decoder can work without manual input.
2. **DC truncation to signed 8 bits.** If a quantized DC coefficient can
   exceed the -128..127 range given the current quant tables, that
   information is already lost at encode time and cannot be recovered on
   decode. Worth checking before investing decoder implementation time.
