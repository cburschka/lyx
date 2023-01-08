# Stricter version of the export tests: validate the XHTML code produced by
# LyX' lyxhtml output as HTML5. It also validates the CSS and MathML parts.
# Validation errors usually are mistakes in the generator.
#
# Call:
#     python simplehtml_validity.py PATH_TO_HTML5_SOURCE
#
# Written with Python 3.8.8.
# Requirements:
# - Python package: html5validator: at least v0.4.2
# - Java runtime engine (JRE): at least v8 (depending on html5validator)
# Run:
#     pip install html5validator>=0.4.2

import collections
import glob
import sys
import tempfile
import os

import html5validator


if len(sys.argv) != 2:
    print('Expecting one argument, the path to the LyX-create xhtml file')
    sys.exit(-1)
if not os.path.exists(sys.argv[1]):
    print('The given path does not point to an existing file')
    sys.exit(-1)


xhtml_file_name = sys.argv[1]
xhtml_list = [xhtml_file_name]

validator = html5validator.Validator(format='text')
error_count = validator.validate(xhtml_list)

n_invalid = 0
n_valid = 0
if error_count == 0:
    n_valid += 1
    print(f'> Found no validation error!')
    sys.exit(0)
else:
    n_invalid += 1
    print(f'> Found {error_count} validation error{"" if error_count == 1 else "s"}!')
    sys.exit(-2)

if n_invalid == 0:
    print("That's excellent! Give yourself a pat on the back!")
    sys.exit(0)
else:
    sys.exit(-3)
