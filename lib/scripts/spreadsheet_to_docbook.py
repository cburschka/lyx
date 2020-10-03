#!/usr/bin/python3

# file spreadsheet_to_docbook.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Thibaut Cuvelier & Kornel Benko

# Full author contact details are available in file CREDITS.

"""reformat output of ssconvert of a single spreadsheet to match the needs
of docbook5 table format .

Expects to read from file specified by sys.argv[1]
and output to to file specified by sys.argv[2]
"""

import re
import sys
import subprocess


def process_file(contents):
    # Scrap the header and the footer.
    contents = contents.split("<body>")[1]
    contents = contents.split("</body>")[0]

    # Gnumeric may generate more than one table, just take the first one.
    contents = contents.split("</table>")[0] + "\n</table>"

    # Convert the rest of the table to DocBook.
    contents = contents.replace("<p></p>", "")
    contents = contents.replace("<i>", "<emphasis>")
    contents = contents.replace("</i>", "</emphasis>")
    contents = contents.replace("<b>", "<emphasis role='bold'>")
    contents = contents.replace("</b>", "</emphasis>")
    contents = contents.replace("<u>", "<emphasis role='underline'>")
    contents = contents.replace("</u>", "</emphasis>")

    contents = re.sub(r"<font color=\"(.*)\">", "<phrase role='color \\1'>", contents)
    assert '<font' not in contents  # If this happens, implement something to catch these cases.
    contents = contents.replace("</font>", "</phrase>")  # Generates invalid XML if there are still font tags left...

    # If the table has a caption, then the right tag is <table>. Otherwise, it's <informaltable>.
    if '<caption>' not in contents:
        contents = contents.replace("<table", "<informaltable")
        contents = contents.replace("</table>", "</informaltable>")

    # Return the processed string.
    contents = contents.replace("\n\n", "\n")
    return contents


if __name__ == "__main__":
    if len(sys.argv) == 1:
        # Read from stdin, output to stdout.
        contents = sys.stdin.read()
        f = sys.stdout
    else:
        # Read from output of ssconvert
        assert len(sys.argv) == 3  # Script name, file to process, output file.
        proc = subprocess.Popen(["ssconvert", "--export-type=Gnumeric_html:xhtml", sys.argv[1], "fd://1"], stdout=subprocess.PIPE)
        f = open(sys.argv[2], 'w')
        sys.stdout = f  # Redirect stdout to the output file.
        contents = proc.stdout.read()

    # Process and output to stdout.
    print(process_file(contents))
    f.close()
    exit(0)
