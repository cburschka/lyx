#!/bin/bash
#
# This command makes all files in "$ROOT_OUTDIR" writable by keytest.
#
# This command is useful if you run keytest under a seperate user "keytest" of group "keytest" 
. ./shared_variables.sh
find  $ROOT_OUTDIR | while read f ; do chmod g+rw $f ; chgrp keytest $f ;done
