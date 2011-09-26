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
# * dv2dt;
# * pngtoppm (if outputing ppm format images).

# preview.sty and dvipng are part of the preview-latex project
# http://preview-latex.sourceforge.net/

# preview.sty can alternatively be obtained from
# CTAN/support/preview-latex/

# Example usage:
# lyxpreview2bitmap.py png 0lyxpreview.tex 128 000000 faf0e6

# This script takes six arguments:
# FORMAT:   The desired output format. Either 'png' or 'ppm'.
# TEXFILE:  the name of the .tex file to be converted.
# DPI:      a scale factor, used to ascertain the resolution of the
#           generated image which is then passed to gs.
# FG_COLOR: the foreground color as a hexadecimal string, eg '000000'.
# BG_COLOR: the background color as a hexadecimal string, eg 'faf0e6'.
# CONVERTER: the converter (optional). Default is latex.

# Decomposing TEXFILE's name as DIR/BASE.tex, this script will,
# if executed successfully, leave in DIR:
# * a (possibly large) number of image files with names
#   like BASE[0-9]+.png
# * a file BASE.metrics, containing info needed by LyX to position
#   the images correctly on the screen.

# What does this script do?
# 1) Call latex/pdflatex/xelatex/whatever (CONVERTER parameter)
# 2) If the output is a PDF fallback to legacy
# 3) Otherwise check each page of the DVI (with dv2dt) looking for 
#    PostScript literals, not well supported by dvipng. Pages 
#    containing them are passed to the legacy method in a new LaTeX file.
# 4) Call dvipng on the pages without PS literals
# 5) Join metrics info coming from both methods (legacy and dvipng)
#    and write them to file

# dvipng is fast but gives problem in several cases, like with
# PSTricks, TikZ and other packages using PostScript literals
# for all these cases the legacy route is taken (step 3).
# Moreover dvipng can't work with PDF files, so, if the CONVERTER
# paramter is pdflatex we have to fallback to legacy route (step 2).

import glob, os, re, string, sys

from legacy_lyxpreview2ppm import legacy_conversion, \
     legacy_conversion_step2, legacy_extract_metrics_info, filter_pages
     
from lyxpreview_tools import copyfileobj, error, find_exe, \
     find_exe_or_terminate, make_texcolor, mkstemp, run_command, warning, \
     write_metrics_info, join_metrics_and_rename


# Pre-compiled regular expressions.
latex_file_re = re.compile("\.tex$")

# PATH environment variable
path  = string.split(os.environ["PATH"], os.pathsep)

def usage(prog_name):
    return "Usage: %s <format> <latex file> <dpi> <fg color> <bg color>\n"\
           "\twhere the colors are hexadecimal strings, eg 'faf0e6'"\
           % prog_name
    
# Returns a list of tuples containing page number and ascent fraction
# extracted from dvipng output.
# Use write_metrics_info to create the .metrics file with this info
def extract_metrics_info(dvipng_stdout):
    # "\[[0-9]+" can match two kinds of numbers: page numbers from dvipng
    # and glyph numbers from mktexpk. The glyph numbers always match
    # "\[[0-9]+\]" while the page number never is followed by "\]". Thus:
    page_re = re.compile("\[([0-9]+)[^]]");
    metrics_re = re.compile("depth=(-?[0-9]+) height=(-?[0-9]+)")

    success = 0
    page = ""
    pos = 0
    results = []
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

        results.append((int(page), frac))
        pos = match.end() + 2

    if success == 0:
        error("Failed to extract metrics info from dvipng")
    
    return results


def color_pdf(latex_file, bg_color, fg_color):
    use_preview_pdf_re = re.compile("(\s*\\\\usepackage\[[^]]+)((pdftex|xetex)\]{preview})")

    tmp = mkstemp()
    
    fg = ""
    if fg_color != "0.000000,0.000000,0.000000":
        fg = '  \\AtBeginDocument{\\let\\oldpreview\\preview\\renewcommand\\preview{\\oldpreview\\color[rgb]{%s}}}\n' % (fg_color)
    
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
                  "%s" \
                  "%s\n" \
                  % (bg_color, fg, match.group()))
            continue

    except:
        # Unable to open the file, but do nothing here because
        # the calling function will act on the value of 'success'.
        warning('Warning in color_pdf! Unable to open "%s"' % latex_file)
        warning(`sys.exc_type` + ',' + `sys.exc_value`)

    if success:
        copyfileobj(tmp, open(latex_file,"wb"), 1)

    return success


def fix_latex_file(latex_file):
    documentclass_re = re.compile("(\\\\documentclass\[)(1[012]pt,?)(.+)")

    tmp = mkstemp()

    changed = 0
    for line in open(latex_file, 'r').readlines():
        match = documentclass_re.match(line)
        if match == None:
            tmp.write(line)
            continue

        changed = 1
        tmp.write("%s%s\n" % (match.group(1), match.group(3)))

    if changed:
        copyfileobj(tmp, open(latex_file,"wb"), 1)

    return


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

# Returns a tuple of:
# ps_pages: list of page indexes of pages containing PS literals
# page_count: total number of pages
# pages_parameter: parameter for dvipng to exclude pages with PostScript
def find_ps_pages(dvi_file):
    # latex failed
    # FIXME: try with pdflatex
    if not os.path.isfile(dvi_file):
        error("No DVI output.")
        
    # Check for PostScript specials in the dvi, badly supported by dvipng
    # This is required for correct rendering of PSTricks and TikZ
    dv2dt = find_exe_or_terminate(["dv2dt"], path)
    dv2dt_call = '%s "%s"' % (dv2dt, dvi_file)
 
    # The output from dv2dt goes to stdout
    dv2dt_status, dv2dt_output = run_command(dv2dt_call)
    psliteral_re = re.compile("^special[1-4] [0-9]+ '(\"|ps:)")

    # Parse the dtl file looking for PostScript specials.
    # Pages using PostScript specials are recorded in ps_pages and then
    # used to create a different LaTeX file for processing in legacy mode.
    page_has_ps = False
    page_index = 0
    ps_pages = []

    for line in dv2dt_output.split("\n"):
        # New page
        if line.startswith("bop"):
            page_has_ps = False
            page_index += 1

        # End of page
        if line.startswith("eop") and page_has_ps:
            # We save in a list all the PostScript pages
            ps_pages.append(page_index)

        if psliteral_re.match(line) != None:
            # Literal PostScript special detected!
            page_has_ps = True
    
    # Create the -pp parameter for dvipng
    pages_parameter = ""
    if len(ps_pages) > 0 and len(ps_pages) < page_index:
        # Don't process Postscript pages with dvipng by selecting the
        # wanted pages through the -pp parameter. E.g., dvipng -pp 4-12,14,64
        pages_parameter = " -pp "
        skip = True
        last = -1

        # Use page ranges, as a list of pages could exceed command line
        # maximum length (especially under Win32)
        for index in xrange(1, page_index + 1):
            if (not index in ps_pages) and skip:
                # We were skipping pages but current page shouldn't be skipped.
                # Add this page to -pp, it could stay alone or become the
                # start of a range.
                pages_parameter += str(index)
                # Save the starting index to avoid things such as "11-11"
                last = index
                # We're not skipping anymore
                skip = False
            elif (index in ps_pages) and (not skip):
                # We weren't skipping but current page should be skipped
                if last != index - 1:
                    # If the start index of the range is the previous page
                    # then it's not a range
                    pages_parameter += "-" + str(index - 1)

                # Add a separator
                pages_parameter += ","
                # Now we're skipping
                skip = True

        # Remove the trailing separator
        pages_parameter = pages_parameter.rstrip(",")
        # We've to manage the case in which the last page is closing a range
        if (not index in ps_pages) and (not skip) and (last != index):
                pages_parameter += "-" + str(index)

    return (ps_pages, page_index, pages_parameter)

def main(argv):
    # Parse and manipulate the command line arguments.
    if len(argv) != 6 and len(argv) != 7:
        error(usage(argv[0]))

    output_format = string.lower(argv[1])

    dir, latex_file = os.path.split(argv[2])
    if len(dir) != 0:
        os.chdir(dir)

    dpi = string.atoi(argv[3])
    fg_color = make_texcolor(argv[4], False)
    bg_color = make_texcolor(argv[5], False)

    fg_color_gr = make_texcolor(argv[4], True)
    bg_color_gr = make_texcolor(argv[5], True)

    # External programs used by the script.
    if len(argv) == 7:
        latex = argv[6]
    else:
        latex = find_exe_or_terminate(["latex", "pplatex", "platex", "latex2e"], path)

    # Omit font size specification in latex file.
    fix_latex_file(latex_file)

    # This can go once dvipng becomes widespread.
    dvipng = find_exe(["dvipng"], path)
    if dvipng == None:
        # The data is input to legacy_conversion in as similar
        # as possible a manner to that input to the code used in
        # LyX 1.3.x.
        vec = [ argv[0], argv[2], argv[3], argv[1], argv[4], argv[5], latex ]
        return legacy_conversion(vec)

    pngtopnm = ""
    if output_format == "ppm":
        pngtopnm = find_exe_or_terminate(["pngtopnm"], path)

    # Move color information for PDF into the latex file.
    if not color_pdf(latex_file, bg_color_gr, fg_color_gr):
        error("Unable to move color info into the latex file")

    # Compile the latex file.
    latex_call = '%s "%s"' % (latex, latex_file)

    latex_status, latex_stdout = run_command(latex_call)
    if latex_status != None:
        warning("%s had problems compiling %s" \
              % (os.path.basename(latex), latex_file))

    if latex == "xelatex":
        warning("Using XeTeX")
        # FIXME: skip unnecessary dvips trial in legacy_conversion_step2
        return legacy_conversion_step2(latex_file, dpi, output_format)

    # The dvi output file name
    dvi_file = latex_file_re.sub(".dvi", latex_file)

    # If there's no DVI output, look for PDF and go to legacy or fail
    if not os.path.isfile(dvi_file):
        # No DVI, is there a PDF?
        pdf_file = latex_file_re.sub(".pdf", latex_file)
        if os.path.isfile(pdf_file):
            warning("%s produced a PDF output, fallback to legacy." % \
                (os.path.basename(latex)))
            return legacy_conversion_step2(latex_file, dpi, output_format)
        else:
            error("No DVI or PDF output. %s failed." \
                % (os.path.basename(latex)))

    # Look for PS literals in DVI pages
    # ps_pages: list of page indexes of pages containing PS literals
    # page_count: total number of pages
    # pages_parameter: parameter for dvipng to exclude pages with PostScript
    (ps_pages, page_count, pages_parameter) = find_ps_pages(dvi_file)
    
    # If all pages need PostScript, directly use the legacy method.
    if len(ps_pages) == page_count:
        vec = [argv[0], argv[2], argv[3], argv[1], argv[4], argv[5], latex]
        return legacy_conversion(vec)

    # Run the dvi file through dvipng.
    dvipng_call = '%s -Ttight -depth -height -D %d -fg "%s" -bg "%s" %s "%s"' \
                  % (dvipng, dpi, fg_color, bg_color, pages_parameter, dvi_file)
    dvipng_status, dvipng_stdout = run_command(dvipng_call)

    if dvipng_status != None:
        warning("%s failed to generate images from %s... fallback to legacy method" \
              % (os.path.basename(dvipng), dvi_file))
        # FIXME: skip unnecessary dvips trial in legacy_conversion_step2
        return legacy_conversion_step2(latex_file, dpi, output_format)

    # Extract metrics info from dvipng_stdout.
    metrics_file = latex_file_re.sub(".metrics", latex_file)
    dvipng_metrics = extract_metrics_info(dvipng_stdout)

    # If some pages require PostScript pass them to legacy method
    if len(ps_pages) > 0:
        # Create a new LaTeX file just for the snippets needing
        # the legacy method
        legacy_latex_file = latex_file_re.sub("_legacy.tex", latex_file)
        filter_pages(latex_file, legacy_latex_file, ps_pages)

        # Pass the new LaTeX file to the legacy method
        vec = [ argv[0], latex_file_re.sub("_legacy.tex", argv[2]), \
                argv[3], argv[1], argv[4], argv[5], latex ]
        legacy_metrics = legacy_conversion(vec, True)[1]
        
        # Now we need to mix metrics data from dvipng and the legacy method
        original_bitmap = latex_file_re.sub("%d." + output_format, legacy_latex_file)
        destination_bitmap = latex_file_re.sub("%d." + output_format, latex_file)
        
        # Join metrics from dvipng and legacy, and rename legacy bitmaps
        join_metrics_and_rename(dvipng_metrics, legacy_metrics, ps_pages, 
            original_bitmap, destination_bitmap)

    # Convert images to ppm format if necessary.
    if output_format == "ppm":
        convert_to_ppm_format(pngtopnm, latex_file_re.sub("", latex_file))

    # Actually create the .metrics file
    write_metrics_info(dvipng_metrics, metrics_file)
    
    return (0, dvipng_metrics)

if __name__ == "__main__":
    sys.exit(main(sys.argv)[0])
