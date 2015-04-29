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
# * python 2.4 or later (subprocess module);
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
# lyxpreview2bitmap.py --bg=faf0e6 0lyxpreview.tex

# This script takes one obligatory argument:
#
#   <input file>:  The name of the .tex file to be converted.
#
# and these optional arguments:
#
#   --png, --ppm:  The desired output format. Either 'png' or 'ppm'.
#   --dpi=<res>:   A scale factor, used to ascertain the resolution of the
#                  generated image which is then passed to gs.
#   --fg=<color>:  The foreground color as a hexadecimal string, eg '000000'.
#   --bg=<color>:  The background color as a hexadecimal string, eg 'faf0e6'.
#   --latex=<exe>: The converter for latex files. Default is latex.
#   --bibtex=<exe>: The converter for bibtex files. Default is bibtex.
#   --lilypond:    Preprocess through lilypond-book. Default is false.
#   --lilypond-book=<exe>:
#                  The converter for lytex files. Default is lilypond-book.
#
#   -d, --debug    Show the output from external commands.
#   -h, --help     Show an help screen and exit.
#   -v, --verbose  Show progress messages.

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

import getopt, glob, os, re, shutil, string, sys

from legacy_lyxpreview2ppm import legacy_conversion_step1

from lyxpreview_tools import bibtex_commands, copyfileobj, error, \
     filter_pages, find_exe, find_exe_or_terminate, join_metrics_and_rename, \
     latex_commands, latex_file_re, make_texcolor, mkstemp, pdflatex_commands, \
     progress, run_command, run_latex, run_tex, warning, write_metrics_info


def usage(prog_name):
    msg = """
Usage: %s <options> <input file>

Options:
  --dpi=<res>:   Resolution per inch (default: 128)
  --png, --ppm:  Select the output format (default: png)
  --fg=<color>:  Foreground color (default: black, ie '000000')
  --bg=<color>:  Background color (default: white, ie 'ffffff')
  --latex=<exe>: Specify the executable for latex (default: latex)
  --bibtex=<exe>: Specify the executable for bibtex (default: bibtex)
  --lilypond:    Preprocess through lilypond-book (default: false)
  --lilypond-book=<exe>:
                 The executable for lilypond-book (default: lilypond-book)

  -d, --debug:   Show the output from external commands
  -h, --help:    Show this help screen and exit
  -v, --verbose: Show progress messages

The colors are hexadecimal strings, eg 'faf0e6'."""
    return msg % prog_name

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
        if ascent < 0:
            # This is an empty image, forbid its display
            frac = -1.0
        elif ascent >= 0 or descent >= 0:
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


def fix_latex_file(latex_file):
    documentclass_re = re.compile("(\\\\documentclass\[)(1[012]pt,?)(.+)")
    newcommandx_re = re.compile("^(\\\\newcommandx)(.+)")

    tmp = mkstemp()

    changed = 0
    for line in open(latex_file, 'r').readlines():
        match = documentclass_re.match(line)
        if match != None:
            changed = 1
            tmp.write("%s%s\n" % (match.group(1), match.group(3)))
            continue

        match = newcommandx_re.match(line)
        if match == None:
            tmp.write(line)
            continue

        changed = 1
        tmp.write("\\providecommandx%s\n" % match.group(2))

    if changed:
        copyfileobj(tmp, open(latex_file,"wb"), 1)

    return changed


def convert_to_ppm_format(pngtopnm, basename):
    png_file_re = re.compile("\.png$")

    for png_file in glob.glob("%s*.png" % basename):
        ppm_file = png_file_re.sub(".ppm", png_file)

        p2p_cmd = '%s "%s"' % (pngtopnm, png_file)
        p2p_status, p2p_stdout = run_command(p2p_cmd)
        if p2p_status:
            error("Unable to convert %s to ppm format" % png_file)

        ppm = open(ppm_file, 'w')
        ppm.write(p2p_stdout)
        os.remove(png_file)

# Returns a tuple of:
# ps_pages: list of page indexes of pages containing PS literals
# pdf_pages: list of page indexes of pages requiring running pdflatex
# page_count: total number of pages
# pages_parameter: parameter for dvipng to exclude pages with PostScript/PDF
def find_ps_pages(dvi_file):
    # latex failed
    # FIXME: try with pdflatex
    if not os.path.isfile(dvi_file):
        error("No DVI output.")

    # Check for PostScript specials in the dvi, badly supported by dvipng,
    # and inclusion of PDF/PNG/JPG files. 
    # This is required for correct rendering of PSTricks and TikZ
    dv2dt = find_exe_or_terminate(["dv2dt"])
    dv2dt_call = '%s "%s"' % (dv2dt, dvi_file)

    # The output from dv2dt goes to stdout
    dv2dt_status, dv2dt_output = run_command(dv2dt_call)
    psliteral_re = re.compile("^special[1-4] [0-9]+ '(\"|ps:)")
    pdffile_re = re.compile("^special[1-4] [0-9]+ 'PSfile=.*.(pdf|png|jpg)")

    # Parse the dtl file looking for PostScript specials and pdflatex files.
    # Pages using PostScript specials or pdflatex files are recorded in
    # ps_pages or pdf_pages, respectively, and then used to create a
    # different LaTeX file for processing in legacy mode.
    page_has_ps = False
    page_has_pdf = False
    page_index = 0
    ps_pages = []
    pdf_pages = []
    ps_or_pdf_pages = []

    for line in dv2dt_output.split("\n"):
        # New page
        if line.startswith("bop"):
            page_has_ps = False
            page_has_pdf = False
            page_index += 1

        # End of page
        if line.startswith("eop") and (page_has_ps or page_has_pdf):
            # We save in a list all the PostScript/PDF pages
            if page_has_ps:
                ps_pages.append(page_index)
            else:
                pdf_pages.append(page_index)
            ps_or_pdf_pages.append(page_index)

        if psliteral_re.match(line) != None:
            # Literal PostScript special detected!
            page_has_ps = True

        if pdffile_re.match(line) != None:
            # Inclusion of pdflatex image file detected!
            page_has_pdf = True

    # Create the -pp parameter for dvipng
    pages_parameter = ""
    if len(ps_or_pdf_pages) > 0 and len(ps_or_pdf_pages) < page_index:
        # Don't process Postscript/PDF pages with dvipng by selecting the
        # wanted pages through the -pp parameter. E.g., dvipng -pp 4-12,14,64
        pages_parameter = " -pp "
        skip = True
        last = -1

        # Use page ranges, as a list of pages could exceed command line
        # maximum length (especially under Win32)
        for index in xrange(1, page_index + 1):
            if (not index in ps_or_pdf_pages) and skip:
                # We were skipping pages but current page shouldn't be skipped.
                # Add this page to -pp, it could stay alone or become the
                # start of a range.
                pages_parameter += str(index)
                # Save the starting index to avoid things such as "11-11"
                last = index
                # We're not skipping anymore
                skip = False
            elif (index in ps_or_pdf_pages) and (not skip):
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
        if (not index in ps_or_pdf_pages) and (not skip) and (last != index):
                pages_parameter += "-" + str(index)

    return (ps_pages, pdf_pages, page_index, pages_parameter)

def main(argv):
    # Set defaults.
    dpi = 128
    fg_color = "000000"
    bg_color = "ffffff"
    bibtex = None
    latex = None
    lilypond = False
    lilypond_book = None
    output_format = "png"
    script_name = argv[0]

    # Parse and manipulate the command line arguments.
    try:
        (opts, args) = getopt.gnu_getopt(argv[1:], "dhv", ["bibtex=", "bg=",
            "debug", "dpi=", "fg=", "help", "latex=", "lilypond",
            "lilypond-book=", "png", "ppm", "verbose"])
    except getopt.GetoptError, err:
        error("%s\n%s" % (err, usage(script_name)))

    opts.reverse()
    for opt, val in opts:
        if opt in ("-h", "--help"):
            print usage(script_name)
            sys.exit(0)
        elif opt == "--bibtex":
            bibtex = [val]
        elif opt == "--bg":
            bg_color = val
        elif opt in ("-d", "--debug"):
            import lyxpreview_tools
            lyxpreview_tools.debug = True
        elif opt == "--dpi":
            try:
                dpi = string.atoi(val)
            except:
                error("Cannot convert %s to an integer value" % val)
        elif opt == "--fg":
            fg_color = val
        elif opt == "--latex":
            latex = [val]
        elif opt == "--lilypond":
            lilypond = True
        elif opt == "--lilypond-book":
            lilypond_book = [val]
        elif opt in ("--png", "--ppm"):
            output_format = opt[2:]
        elif opt in ("-v", "--verbose"):
            import lyxpreview_tools
            lyxpreview_tools.verbose = True

    # Determine input file
    if len(args) != 1:
        err = "A single input file is required, %s given" % (len(args) or "none")
        error("%s\n%s" % (err, usage(script_name)))

    input_path = args[0]
    dir, latex_file = os.path.split(input_path)

    # Echo the settings
    progress("Starting %s..." % script_name)
    progress("Output format: %s" % output_format)
    progress("Foreground color: %s" % fg_color)
    progress("Background color: %s" % bg_color)
    progress("Resolution (dpi): %s" % dpi)
    progress("File to process: %s" % input_path)

    # Check for the input file
    if not os.path.exists(input_path):
        error('File "%s" not found.' % input_path)
    if len(dir) != 0:
        os.chdir(dir)

    fg_color_dvipng = make_texcolor(fg_color, False)
    bg_color_dvipng = make_texcolor(bg_color, False)

    # External programs used by the script.
    latex = find_exe_or_terminate(latex or latex_commands)
    bibtex = find_exe(bibtex or bibtex_commands)
    if lilypond:
        lilypond_book = find_exe_or_terminate(lilypond_book or
            ["lilypond-book --safe"])

    # These flavors of latex are known to produce pdf output
    pdf_output = latex in pdflatex_commands

    progress("Latex command: %s" % latex)
    progress("Latex produces pdf output: %s" % pdf_output)
    progress("Bibtex command: %s" % bibtex)
    progress("Lilypond-book command: %s" % lilypond_book)
    progress("Preprocess through lilypond-book: %s" % lilypond)
    progress("Altering the latex file for font size and colors")

    # Omit font size specification in latex file and make sure multiple
    # defined macros are not an issue.
    fix_latex_file(latex_file)

    if lilypond:
        progress("Preprocess the latex file through %s" % lilypond_book)
        if pdf_output:
            lilypond_book += " --pdf"
        lilypond_book += " --latex-program=%s" % latex.split()[0]

        # Make a copy of the latex file
        lytex_file = latex_file_re.sub(".lytex", latex_file)
        shutil.copyfile(latex_file, lytex_file)

        # Preprocess the latex file through lilypond-book.
        lytex_status, lytex_stdout = run_tex(lilypond_book, lytex_file)

    if pdf_output:
        progress("Using the legacy conversion method (PDF support)")
        return legacy_conversion_step1(latex_file, dpi, output_format, fg_color,
            bg_color, latex, pdf_output)

    # This can go once dvipng becomes widespread.
    dvipng = find_exe(["dvipng"])
    if dvipng == None:
        progress("Using the legacy conversion method (dvipng not found)")
        return legacy_conversion_step1(latex_file, dpi, output_format, fg_color,
            bg_color, latex, pdf_output)

    dv2dt = find_exe(["dv2dt"])
    if dv2dt == None:
        progress("Using the legacy conversion method (dv2dt not found)")
        return legacy_conversion_step1(latex_file, dpi, output_format, fg_color,
            bg_color, latex, pdf_output)

    pngtopnm = ""
    if output_format == "ppm":
        pngtopnm = find_exe(["pngtopnm"])
        if pngtopnm == None:
            progress("Using the legacy conversion method (pngtopnm not found)")
            return legacy_conversion_step1(latex_file, dpi, output_format,
                fg_color, bg_color, latex, pdf_output)

    # Compile the latex file.
    latex_status, latex_stdout = run_latex(latex, latex_file, bibtex)
    if latex_status:
        warning("trying to recover from failed compilation")

    # The dvi output file name
    dvi_file = latex_file_re.sub(".dvi", latex_file)

    # If there's no DVI output, look for PDF and go to legacy or fail
    if not os.path.isfile(dvi_file):
        # No DVI, is there a PDF?
        pdf_file = latex_file_re.sub(".pdf", latex_file)
        if os.path.isfile(pdf_file):
            progress("%s produced a PDF output, fallback to legacy." \
                % (os.path.basename(latex)))
            progress("Using the legacy conversion method (PDF support)")
            return legacy_conversion_step1(latex_file, dpi, output_format,
                fg_color, bg_color, latex, True)
        else:
            error("No DVI or PDF output. %s failed." \
                % (os.path.basename(latex)))

    # Look for PS literals or inclusion of pdflatex files in DVI pages
    # ps_pages: list of indexes of pages containing PS literals
    # pdf_pages: list of indexes of pages requiring running pdflatex
    # page_count: total number of pages
    # pages_parameter: parameter for dvipng to exclude pages with PostScript
    (ps_pages, pdf_pages, page_count, pages_parameter) = find_ps_pages(dvi_file)

    # If all pages need PostScript or pdflatex, directly use the legacy method.
    if len(ps_pages) == page_count:
        progress("Using the legacy conversion method (PostScript support)")
        return legacy_conversion_step1(latex_file, dpi, output_format, fg_color,
            bg_color, latex, pdf_output)
    elif len(pdf_pages) == page_count:
        progress("Using the legacy conversion method (PDF support)")
        return legacy_conversion_step1(latex_file, dpi, output_format, fg_color,
            bg_color, "pdflatex", True)

    # Run the dvi file through dvipng.
    dvipng_call = '%s -Ttight -depth -height -D %d -fg "%s" -bg "%s" %s "%s"' \
        % (dvipng, dpi, fg_color_dvipng, bg_color_dvipng, pages_parameter, dvi_file)
    dvipng_status, dvipng_stdout = run_command(dvipng_call)

    if dvipng_status:
        warning("%s failed to generate images from %s... fallback to legacy method" \
              % (os.path.basename(dvipng), dvi_file))
        progress("Using the legacy conversion method (dvipng failed)")
        return legacy_conversion_step1(latex_file, dpi, output_format, fg_color,
            bg_color, latex, pdf_output)

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
        progress("Pages %s include postscript specials" % ps_pages)
        progress("Using the legacy conversion method (PostScript support)")
        legacy_status, legacy_metrics = legacy_conversion_step1(legacy_latex_file,
            dpi, output_format, fg_color, bg_color, latex, pdf_output, True)

        # Now we need to mix metrics data from dvipng and the legacy method
        original_bitmap = latex_file_re.sub("%d." + output_format, legacy_latex_file)
        destination_bitmap = latex_file_re.sub("%d." + output_format, latex_file)

        # Join metrics from dvipng and legacy, and rename legacy bitmaps
        join_metrics_and_rename(dvipng_metrics, legacy_metrics, ps_pages,
            original_bitmap, destination_bitmap)

    # If some pages require running pdflatex pass them to legacy method
    if len(pdf_pages) > 0:
        # Create a new LaTeX file just for the snippets needing
        # the legacy method
        legacy_latex_file = latex_file_re.sub("_legacy.tex", latex_file)
        filter_pages(latex_file, legacy_latex_file, pdf_pages)

        # Pass the new LaTeX file to the legacy method
        progress("Pages %s include pdflatex image files" % pdf_pages)
        progress("Using the legacy conversion method (PDF support)")
        legacy_status, legacy_metrics = legacy_conversion_step1(legacy_latex_file,
            dpi, output_format, fg_color, bg_color, "pdflatex", True, True)

        # Now we need to mix metrics data from dvipng and the legacy method
        original_bitmap = latex_file_re.sub("%d." + output_format, legacy_latex_file)
        destination_bitmap = latex_file_re.sub("%d." + output_format, latex_file)

        # Join metrics from dvipng and legacy, and rename legacy bitmaps
        join_metrics_and_rename(dvipng_metrics, legacy_metrics, pdf_pages,
            original_bitmap, destination_bitmap)

    # Convert images to ppm format if necessary.
    if output_format == "ppm":
        convert_to_ppm_format(pngtopnm, latex_file_re.sub("", latex_file))

    # Actually create the .metrics file
    write_metrics_info(dvipng_metrics, metrics_file)

    return (0, dvipng_metrics)

if __name__ == "__main__":
    sys.exit(main(sys.argv)[0])
