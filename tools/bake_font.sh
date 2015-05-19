#!/bin/sh

# Convert a 16x16 row-major font bitmap into a C include file with 8-bit
# grayscale data. Requires expr, ImageMagick and xxd.

if [ "$#" -ne 1 ]; then
    echo "Usage: `basename $0` infile.png"
    exit 1
fi

if !command -v xxd &> /dev/null; then
    echo "xxd not installed"
    exit 1
fi

if !command -v identify &> /dev/null; then
    echo "ImageMagick not installed"
    exit 1
fi

convert $1 $1.gray || exit 1
w=`identify -format '%w' $1`
h=`identify -format '%h' $1`
echo "{ `expr $w / 16`, `expr $h / 16`, `identify -format '%w' $1`,"
cat $1.gray | xxd -i
echo "}"

rm $1.gray
