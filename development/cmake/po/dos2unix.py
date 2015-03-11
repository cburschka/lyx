#! /usr/bin/env python

###############
import sys

for fname in sys.argv[1:]:
    infile = open( fname, "r" )
    instr = infile.read()
    infile.close()
    outstr = instr.replace( "\r\n", "\n" ).replace( "\r", "\n" )

    if outstr == instr:
        continue
    
    outfile = open( fname , "w" )
    outfile.write( outstr )
    outfile.close()
