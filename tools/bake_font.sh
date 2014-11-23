#!/bin/sh

# Convert a font bitmap into a C include file with 8-bit grayscale data.
# Requires ImageMagick and xxd.

if [ "$#" -ne 1 ]; then
    echo "Usage: `basename $0` infile.png"
    exit 1
fi

convert $1 $1.gray || exit 1
echo "const int default_font_w = `identify -format '%w' $1` / 16;"
echo "const int default_font_h = `identify -format '%h' $1` / 16;"
echo "const uint8_t default_font[] = {"
cat $1.gray | xxd -i
echo "};"

rm $1.gray
