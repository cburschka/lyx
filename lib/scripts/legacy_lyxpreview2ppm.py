#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file legacy_lyxpreview2ppm.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Angus Leeming
# Full author contact details are available in file CREDITS

# with much advice from members of the preview-latex project:
#   David Kastrup, dak@gnu.org and
#   Jan-Åke Larsson, jalar@mai.liu.se.
# and with much help testing the code under Windows from
#   Paul A. Rubin, rubin@msu.edu.

# This script takes a LaTeX file and generates a collection of
# png or ppm image files, one per previewed snippet.
# Example usage:
# legacy_lyxpreview2bitmap.py 0lyxpreview.tex 128 ppm 000000 faf0e6

# This script takes five arguments:
# TEXFILE:       the name of the .tex file to be converted.
# SCALEFACTOR:   a scale factor, used to ascertain the resolution of the
#                generated image which is then passed to gs.
# OUTPUTFORMAT:  the format of the output bitmap image files.
#                This particular script can produce only "ppm" format output.
# FG_COLOR:      the foreground color as a hexadecimal string, eg '000000'.
# BG_COLOR:      the background color as a hexadecimal string, eg 'faf0e6'.

# Decomposing TEXFILE's name as DIR/BASE.tex, this script will,
# if executed successfully, leave in DIR:
# * a (possibly large) number of image files with names
#   like BASE[0-9]+.(ppm|png)
# * a file BASE.metrics, containing info needed by LyX to position
#   the images correctly on the screen.

# The script uses several external programs and files:
# * A latex executable;
# * preview.sty;
# * dvips;
# * gs;
# * pdflatex (optional);
# * pnmcrop (optional).

# preview.sty is part of the preview-latex project
#   http://preview-latex.sourceforge.net/
# Alternatively, it can be obtained from
#   CTAN/support/preview-latex/

# What does this script do?
# [legacy_conversion]
# 1) Call latex to create a DVI file from LaTeX
# [legacy_conversion_step2]
# 2) Call dvips to create one PS file for each DVI page
# 3) If dvips fails look for PDF and call gs to produce bitmaps
# 4) Otherwise call gs on each PostScript file to produce bitmaps
# [legacy_conversion_pdflatex]
# 5) Keep track of pages on which gs failed and pass them to pdflatex
# 6) Call gs on the PDF output from pdflatex to produce bitmaps
# 7) Extract and write to file (or return to lyxpreview2bitmap) 
#    metrics from both methods (standard and pdflatex)

# The script uses the old dvi->ps->png conversion route,
# which is good when using PSTricks, TikZ or other packages involving
# PostScript literals (steps 1, 2, 4).
# This script also generates bitmaps from PDF created by a call to
# lyxpreview2bitmap.py passing "pdflatex" to the CONVERTER parameter
# (step 3).
# Finally, there's also has a fallback method based on pdflatex, which 
# is required in certain cases, if hyperref is active for instance,
# (step 5, 6).
# If possible, dvipng should be used, as it's much faster.

import glob, os, pipes, re, string, sys

from lyxpreview_tools import copyfileobj, error, find_exe, \
     find_exe_or_terminate, make_texcolor, mkstemp, run_command, warning, \
     write_metrics_info, filter_pages, join_metrics_and_rename

# Pre-compiled regular expression.
latex_file_re = re.compile("\.tex$")

# PATH environment variable
path  = string.split(os.environ["PATH"], os.pathsep)

def usage(prog_name):
    return "Usage: %s <latex file> <dpi> ppm <fg color> <bg color>\n"\
           "\twhere the colors are hexadecimal strings, eg 'faf0e6'"\
           % prog_name

# Returns a list of tuples containing page number and ascent fraction
# extracted from dvipng output.
# Use write_metrics_info to create the .metrics file with this info
def legacy_extract_metrics_info(log_file):

    log_re = re.compile("Preview: ([ST])")
    data_re = re.compile("(-?[0-9]+) (-?[0-9]+) (-?[0-9]+) (-?[0-9]+)")

    tp_ascent  = 0.0
    tp_descent = 0.0

    success = 0
    results = []
    try:
        for line in open(log_file, 'r').readlines():
            match = log_re.match(line)
            if match == None:
                continue

            snippet = (match.group(1) == 'S')
            success = 1
            match = data_re.search(line)
            if match == None:
                error("Unexpected data in %s\n%s" % (log_file, line))

            if snippet:
                ascent  = string.atoi(match.group(2))
                descent = string.atoi(match.group(3))

                frac = 0.5
                if ascent >= 0 and descent >= 0:
                    ascent = float(ascent) + tp_ascent
                    descent = float(descent) - tp_descent

                    if abs(ascent + descent) > 0.1:
                        frac = ascent / (ascent + descent)

                    # Sanity check
                    if frac < 0 or frac > 1:
                            frac = 0.5

                results.append((int(match.group(1)), frac))

            else:
                tp_descent = string.atof(match.group(2))
                tp_ascent  = string.atof(match.group(4))

    except:
        # Unable to open the file, but do nothing here because
        # the calling function will act on the value of 'success'.
        warning('Warning in legacy_extract_metrics_info! Unable to open "%s"' % log_file)
        warning(`sys.exc_type` + ',' + `sys.exc_value`)
		
    if success == 0:
        error("Failed to extract metrics info from %s" % log_file)
        
    return results

def extract_resolution(log_file, dpi):
    fontsize_re = re.compile("Preview: Fontsize")
    magnification_re = re.compile("Preview: Magnification")
    extract_decimal_re = re.compile("([0-9\.]+)")
    extract_integer_re = re.compile("([0-9]+)")

    found_fontsize = 0
    found_magnification = 0

    # Default values
    magnification = 1000.0
    fontsize = 10.0

    try:
        for line in open(log_file, 'r').readlines():
            if found_fontsize and found_magnification:
                break

            if not found_fontsize:
                match = fontsize_re.match(line)
                if match != None:
                    match = extract_decimal_re.search(line)
                    if match == None:
                        error("Unable to parse: %s" % line)
                    fontsize = string.atof(match.group(1))
                    found_fontsize = 1
                    continue

            if not found_magnification:
                match = magnification_re.match(line)
                if match != None:
                    match = extract_integer_re.search(line)
                    if match == None:
                        error("Unable to parse: %s" % line)
                    magnification = string.atof(match.group(1))
                    found_magnification = 1
                    continue

    except:
        warning('Warning in extract_resolution! Unable to open "%s"' % log_file)
        warning(`sys.exc_type` + ',' + `sys.exc_value`)

    # This is safe because both fontsize and magnification have
    # non-zero default values.
    return dpi * (10.0 / fontsize) * (1000.0 / magnification)


def legacy_latex_file(latex_file, fg_color, bg_color, bg_color_gr):
    use_preview_dvi_re = re.compile("(\s*\\\\usepackage\[[^]]+)(dvips\]{preview})")
    use_preview_pdf_re = re.compile("(\s*\\\\usepackage\[[^]]+)(pdftex\]{preview})")

    tmp = mkstemp()

    success = 0
    try:
        for line in open(latex_file, 'r').readlines():
            match = use_preview_dvi_re.match(line)
            if match == None:
                match = use_preview_pdf_re.match(line)
                if match == None:
                    tmp.write(line)
                    continue
                success = 1
                tmp.write("  \\usepackage{color}\n" \
                      "  \\pagecolor[rgb]{%s}\n" \
                      "%s\n" \
                      % (bg_color_gr, match.group()))
                continue

            success = 1
            tmp.write("%stightpage,%s\n" \
                      "  \\AtBeginDocument{\\AtBeginDvi{%%\n" \
                      "  \\special{!userdict begin/bop-hook{//bop-hook exec\n" \
                      "  <%s%s>{255 div}forall setrgbcolor\n" \
                      "  clippath fill setrgbcolor}bind def end}}}\n" \
                      % (match.group(1), match.group(2), fg_color, bg_color))

    except:
        # Unable to open the file, but do nothing here because
        # the calling function will act on the value of 'success'.
        warning('Warning in legacy_latex_file! Unable to open "%s"' % latex_file)
        warning(`sys.exc_type` + ',' + `sys.exc_value`)

    if success:
        copyfileobj(tmp, open(latex_file,"wb"), 1)

    return success


def crop_files(pnmcrop, basename):
    t = pipes.Template()
    t.append('%s -left' % pnmcrop, '--')
    t.append('%s -right' % pnmcrop, '--')

    for file in glob.glob("%s*.ppm" % basename):
        tmp = mkstemp()
        new = t.open(file, "r")
        copyfileobj(new, tmp)
        if not new.close():
            copyfileobj(tmp, open(file,"wb"), 1)


def legacy_conversion(argv, skipMetrics = False):
    latex_commands = ["latex", "pplatex", "platex", "latex2e"]
    # Parse and manipulate the command line arguments.
    if len(argv) == 7:
        latex_commands = [argv[6]]
    elif len(argv) != 6:
        error(usage(argv[0]))

    dir, latex_file = os.path.split(argv[1])
    if len(dir) != 0:
        os.chdir(dir)

    dpi = string.atoi(argv[2])

    output_format = argv[3]

    fg_color = argv[4]
    bg_color = argv[5]
    bg_color_gr = make_texcolor(argv[5], True)

    # External programs used by the script.
    latex = find_exe_or_terminate(latex_commands, path)

    # Move color information into the latex file.
    if not legacy_latex_file(latex_file, fg_color, bg_color, bg_color_gr):
        error("Unable to move color info into the latex file")

    # Compile the latex file.
    latex_call = '%s "%s"' % (latex, latex_file)

    latex_status, latex_stdout = run_command(latex_call)
    if latex_status != None:
        warning("%s had problems compiling %s" \
              % (os.path.basename(latex), latex_file))

    return legacy_conversion_step2(latex_file, dpi, output_format, skipMetrics)

# Creates a new LaTeX file from the original with pages specified in 
# failed_pages, pass it through pdflatex and updates the metrics
# from the standard legacy route
def legacy_conversion_pdflatex(latex_file, failed_pages, legacy_metrics, gs, 
    gs_device, gs_ext, alpha, resolution, output_format):
        
    # Search for pdflatex executable
    pdflatex = find_exe(["pdflatex"], path)
    if pdflatex == None:
        warning("Can't find pdflatex. Some pages failed with all the possible routes.")
    else:
        # Create a new LaTeX file from the original but only with failed pages
        pdf_latex_file = latex_file_re.sub("_pdflatex.tex", latex_file)
        filter_pages(latex_file, pdf_latex_file, failed_pages)
            
        # pdflatex call
        pdflatex_call = '%s "%s"' % (pdflatex, pdf_latex_file)
        pdflatex_status, pdflatex_stdout = run_command(pdflatex_call)
            
        pdf_file = latex_file_re.sub(".pdf", pdf_latex_file)
            
        # GhostScript call to produce bitmaps
        gs_call = '%s -dNOPAUSE -dBATCH -dSAFER -sDEVICE=%s ' \
                    '-sOutputFile="%s%%d.%s" ' \
                    '-dGraphicsAlphaBit=%d -dTextAlphaBits=%d ' \
                    '-r%f "%s"' \
                    % (gs, gs_device, latex_file_re.sub("", pdf_latex_file), \
                        gs_ext, alpha, alpha, resolution, pdf_file)
        gs_status, gs_stdout = run_command(gs_call)
        if gs_status != None:
            # Give up!
            warning("Some pages failed with all the possible routes")
        else:
            # We've done it!
            pdf_log_file = latex_file_re.sub(".log", pdf_latex_file)
            pdf_metrics = legacy_extract_metrics_info(pdf_log_file)
                
            original_bitmap = latex_file_re.sub("%d." + output_format, pdf_latex_file)
            destination_bitmap = latex_file_re.sub("%d." + output_format, latex_file)
                
            # Join the metrics with the those from dvips and rename the bitmap images
            join_metrics_and_rename(legacy_metrics, pdf_metrics, failed_pages, 
                original_bitmap, destination_bitmap)


def legacy_conversion_step2(latex_file, dpi, output_format, skipMetrics = False):
    # External programs used by the script.
    dvips   = find_exe_or_terminate(["dvips"], path)
    gs      = find_exe_or_terminate(["gswin32c", "gswin64c", "gs"], path)
    pnmcrop = find_exe(["pnmcrop"], path)

    # Run the dvi file through dvips.
    dvi_file = latex_file_re.sub(".dvi", latex_file)
    ps_file  = latex_file_re.sub(".ps",  latex_file)
    pdf_file  = latex_file_re.sub(".pdf", latex_file)

    dvips_call = '%s -i -o "%s" "%s"' % (dvips, ps_file, dvi_file)
    dvips_failed = False

    dvips_status, dvips_stdout = run_command(dvips_call)
    if dvips_status != None:
        warning('Failed: %s %s ... looking for PDF' \
            % (os.path.basename(dvips), dvi_file))
        dvips_failed = True

    # Extract resolution data for gs from the log file.
    log_file = latex_file_re.sub(".log", latex_file)
    resolution = extract_resolution(log_file, dpi)

    # Older versions of gs have problems with a large degree of
    # anti-aliasing at high resolutions
    alpha = 4
    if resolution > 150:
        alpha = 2

    gs_device = "png16m"
    gs_ext = "png"
    if output_format == "ppm":
        gs_device = "pnmraw"
        gs_ext = "ppm"

    # Extract the metrics from the log file
    legacy_metrics = legacy_extract_metrics_info(log_file)
    
    # List of pages which failed to produce a correct output
    failed_pages = []
    
    # Generate the bitmap images
    if dvips_failed:
        # dvips failed, maybe there's a PDF, try to produce bitmaps
        gs_call = '%s -dNOPAUSE -dBATCH -dSAFER -sDEVICE=%s ' \
                  '-sOutputFile="%s%%d.%s" ' \
                  '-dGraphicsAlphaBit=%d -dTextAlphaBits=%d ' \
                  '-r%f "%s"' \
                  % (gs, gs_device, latex_file_re.sub("", latex_file), \
                     gs_ext, alpha, alpha, resolution, pdf_file)

        gs_status, gs_stdout = run_command(gs_call)
        if gs_status != None:
            error("Failed: %s %s" % (os.path.basename(gs), ps_file))
    else:
        # Model for calling gs on each file
        gs_call = '%s -dNOPAUSE -dBATCH -dSAFER -sDEVICE=%s ' \
                  '-sOutputFile="%s%%d.%s" ' \
                  '-dGraphicsAlphaBit=%d -dTextAlphaBits=%d ' \
                  '-r%f "%%s"' \
                  % (gs, gs_device, latex_file_re.sub("", latex_file), \
                     gs_ext, alpha, alpha, resolution)
        
        i = 0
        # Collect all the PostScript files (like *.001, *.002, ...)
        ps_files = glob.glob("%s.[0-9][0-9][0-9]" % latex_file_re.sub("", latex_file))
        ps_files.sort()
        
        # Call GhostScript for each file
        for file in ps_files:
            i = i + 1
            gs_status, gs_stdout = run_command(gs_call % (i, file))
            if gs_status != None:
                # gs failed, keep track of this
                failed_pages.append(i)
    
    # Pass failed pages to pdflatex
    if len(failed_pages) > 0:
        legacy_conversion_pdflatex(latex_file, failed_pages, legacy_metrics, gs, 
            gs_device, gs_ext, alpha, resolution, output_format)

    # Crop the images
    if pnmcrop != None:
        crop_files(pnmcrop, latex_file_re.sub("", latex_file))

    # Allow to skip .metrics creation for custom management
    # (see the dvipng method)
    if not skipMetrics:
        # Extract metrics info from the log file.
        metrics_file = latex_file_re.sub(".metrics", latex_file)
        write_metrics_info(legacy_metrics, metrics_file)

    return (0, legacy_metrics)


if __name__ == "__main__":
    sys.exit(legacy_conversion(sys.argv)[0])
