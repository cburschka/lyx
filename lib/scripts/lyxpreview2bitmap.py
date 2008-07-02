#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file lyxpreview2bitmap.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Angus Leeming
# with much advice from members of the preview-latex project:
# David Kastrup, dak@gnu.org and
# Jan-Åke Larsson, jalar@mai.liu.se.

# Full author contact details are available in file CREDITS

# This script takes a LaTeX file and generates a collection of
# png or ppm image files, one per previewed snippet.

# Pre-requisites:
# * A latex executable;
# * preview.sty;
# * dvipng;
# * pngtoppm (if outputing ppm format images).

# preview.sty and dvipng are part of the preview-latex project
# http://preview-latex.sourceforge.net/

# preview.sty can alternatively be obtained from
# CTAN/support/preview-latex/

# Example usage:
# lyxpreview2bitmap.py png 0lyxpreview.tex 128 000000 faf0e6

# This script takes five arguments:
# FORMAT:   The desired output format. Either 'png' or 'ppm'.
# TEXFILE:  the name of the .tex file to be converted.
# DPI:      a scale factor, used to ascertain the resolution of the
#           generated image which is then passed to gs.
# FG_COLOR: the foreground color as a hexadecimal string, eg '000000'.
# BG_COLOR: the background color as a hexadecimal string, eg 'faf0e6'.

# Decomposing TEXFILE's name as DIR/BASE.tex, this script will,
# if executed successfully, leave in DIR:
# * a (possibly large) number of image files with names
#   like BASE[0-9]+.png
# * a file BASE.metrics, containing info needed by LyX to position
#   the images correctly on the screen.

import glob, os, re, string, sys

from legacy_lyxpreview2ppm import legacy_conversion, \
     legacy_conversion_step2

from lyxpreview_tools import copyfileobj, error, find_exe, \
     find_exe_or_terminate, make_texcolor, mkstemp, run_command, warning


# Pre-compiled regular expressions.
latex_file_re = re.compile("\.tex$")


def usage(prog_name):
    return "Usage: %s <format> <latex file> <dpi> <fg color> <bg color>\n"\
           "\twhere the colors are hexadecimal strings, eg 'faf0e6'"\
           % prog_name


def extract_metrics_info(dvipng_stdout, metrics_file):
    metrics = open(metrics_file, 'w')
# "\[[0-9]+" can match two kinds of numbers: page numbers from dvipng
# and glyph numbers from mktexpk. The glyph numbers always match
# "\[[0-9]+\]" while the page number never is followed by "\]". Thus:
    page_re = re.compile("\[([0-9]+)[^]]");
    metrics_re = re.compile("depth=(-?[0-9]+) height=(-?[0-9]+)")

    success = 0
    page = ""
    pos = 0
    while 1:
        match = page_re.search(dvipng_stdout, pos)
        if match == None:
            break
        page = match.group(1)
        pos = match.end()
        match = metrics_re.search(dvipng_stdout, pos)
        if match == None:
            break
        success = 1

        # Calculate the 'ascent fraction'.
        descent = string.atof(match.group(1))
        ascent  = string.atof(match.group(2))

        frac = 0.5
        if ascent >= 0 or descent >= 0:
            if abs(ascent + descent) > 0.1:
                frac = ascent / (ascent + descent)

	    # Sanity check
            if frac < 0:
                frac = 0.5

        metrics.write("Snippet %s %f\n" % (page, frac))
        pos = match.end() + 2

    return success


def color_pdf(latex_file, bg_color):
    use_preview_pdf_re = re.compile("(\s*\\\\usepackage\[[^]]+)(pdftex\]{preview})")

    tmp = mkstemp()

    success = 0
    try:
        for line in open(latex_file, 'r').readlines():
            match = use_preview_pdf_re.match(line)
            if match == None:
                tmp.write(line)
                continue
            success = 1
            tmp.write("  \\usepackage{color}\n" \
                  "  \\pagecolor[rgb]{%s}\n" \
                  "%s\n" \
                  % (bg_color, match.group()))
            continue

    except:
        # Unable to open the file, but do nothing here because
        # the calling function will act on the value of 'success'.
        warning('Warning in color_pdf! Unable to open "%s"' % latex_file)
        warning(`sys.exc_type` + ',' + `sys.exc_value`)

    if success:
        copyfileobj(tmp, open(latex_file,"wb"), 1)

    return success


def convert_to_ppm_format(pngtopnm, basename):
    png_file_re = re.compile("\.png$")

    for png_file in glob.glob("%s*.png" % basename):
        ppm_file = png_file_re.sub(".ppm", png_file)

        p2p_cmd = '%s "%s"' % (pngtopnm, png_file)
        p2p_status, p2p_stdout = run_command(p2p_cmd)
        if p2p_status != None:
            error("Unable to convert %s to ppm format" % png_file)

        ppm = open(ppm_file, 'w')
        ppm.write(p2p_stdout)
        os.remove(png_file)


def main(argv):
    # Parse and manipulate the command line arguments.
    if len(argv) != 6:
        error(usage(argv[0]))

    output_format = string.lower(argv[1])

    dir, latex_file = os.path.split(argv[2])
    if len(dir) != 0:
        os.chdir(dir)

    dpi = string.atoi(argv[3])
    fg_color = make_texcolor(argv[4], False)
    bg_color = make_texcolor(argv[5], False)

    bg_color_gr = make_texcolor(argv[5], True)

    # External programs used by the script.
    path = string.split(os.environ["PATH"], os.pathsep)
    latex = find_exe_or_terminate(["latex", "pplatex", "platex", "latex2e"], path)

    # This can go once dvipng becomes widespread.
    dvipng = find_exe(["dvipng"], path)
    if dvipng == None:
        # The data is input to legacy_conversion in as similar
        # as possible a manner to that input to the code used in
        # LyX 1.3.x.
        vec = [ argv[0], argv[2], argv[3], argv[1], argv[4], argv[5] ]
        return legacy_conversion(vec)

    pngtopnm = ""
    if output_format == "ppm":
        pngtopnm = find_exe_or_terminate(["pngtopnm"], path)

    # Move color information for PDF into the latex file.
    if not color_pdf(latex_file, bg_color_gr):
        error("Unable to move color info into the latex file")

    # Compile the latex file.
    latex_call = '%s "%s"' % (latex, latex_file)

    latex_status, latex_stdout = run_command(latex_call)
    if latex_status != None:
        error("%s failed to compile %s" \
              % (os.path.basename(latex), latex_file))

    # Run the dvi file through dvipng.
    dvi_file = latex_file_re.sub(".dvi", latex_file)
    dvipng_call = '%s -Ttight -depth -height -D %d -fg "%s" -bg "%s" "%s"' \
                  % (dvipng, dpi, fg_color, bg_color, dvi_file)

    dvipng_status, dvipng_stdout = run_command(dvipng_call)
    if dvipng_status != None:
        warning("%s failed to generate images from %s ... looking for PDF" \
              % (os.path.basename(dvipng), dvi_file))
        return legacy_conversion_step2(latex_file, dpi, output_format)

    # Extract metrics info from dvipng_stdout.
    metrics_file = latex_file_re.sub(".metrics", latex_file)
    if not extract_metrics_info(dvipng_stdout, metrics_file):
        error("Failed to extract metrics info from dvipng")

    # Convert images to ppm format if necessary.
    if output_format == "ppm":
        convert_to_ppm_format(pngtopnm, latex_file_re.sub("", latex_file))

    return 0


if __name__ == "__main__":
    main(sys.argv)
