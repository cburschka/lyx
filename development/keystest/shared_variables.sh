DIRNAME0=`dirname "$0"`
OUT_NAME=out/3
ROOT_OUTDIR="$DIRNAME0/$OUT_NAME"
SRC_DIR=lyx/src
EXE_TO_TEST=$SRC_DIR/lyx

EXE_TO_TEST=`readlink -f "$EXE_TO_TEST"` # softlinks can confuse "ps"
