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

import sys
import os

import html5validator


if len(sys.argv) != 2:
    print('Expecting one argument, the path to the LyX-created XHTML file')
    sys.exit(-1)
if not os.path.exists(sys.argv[1]):
    print('The given path does not point to an existing file')
    sys.exit(-1)


xhtml_file_name = sys.argv[1]
xhtml_list = [xhtml_file_name]

validator = html5validator.Validator(format='text')
error_count = validator.validate(xhtml_list)

if error_count == 0:
    sys.exit(0)
else:
    print('> Found a validation error!')
    sys.exit(-2)
