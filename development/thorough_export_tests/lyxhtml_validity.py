# Stricter version of the export tests: validate the XHTML code produced by
# LyX' lyxhtml output as HTML5. It also validates the CSS and MathML parts.
# Validation errors usually are mistakes in the generator.
#
# Call:
#     python lyxhtml_validity.py PATH_TO_LYX/lyx
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
    print('Expecting one argument, the path to the LyX binary to test')
    sys.exit(-1)
if not os.path.exists(sys.argv[1]):
    print('The given path does not point to an existing file')
    sys.exit(-1)
if not os.access(sys.argv[1], os.X_OK):
    print('The given path does not point to an executable file')
    sys.exit(-1)


PATH_SCRIPT = os.path.dirname(os.path.realpath(__file__))
PATH_EXPORT_TESTS = os.path.realpath(PATH_SCRIPT + '/../../autotests/export/')
PATH_LYX = sys.argv[1]


TestFile = collections.namedtuple(
    "TestFile", ["lyx_path", "lyx_file_name", "xhtml_path", "xhtml_file_name"]
)

validator = html5validator.Validator(format='text')


with tempfile.TemporaryDirectory() as out_dir_name:
    all_lyx_files = glob.glob(PATH_EXPORT_TESTS + '/**/*.lyx', recursive=True)
    all_files = [
        TestFile(
            lyx_path=path,
            lyx_file_name=os.path.basename(path),
            xhtml_path = os.path.join(out_dir_name, os.path.basename(path)),
            xhtml_file_name=os.path.basename(path).replace('.lyx', '.html')
        ) for path in all_lyx_files
    ]
    
    # Generate XHTML files.
    print(
        f'Exporting {len(all_lyx_files)} LyX files to LyXHTML format in the ' +
        f'directory {out_dir_name}'
    )
    
    for file in all_files:
        print(f'* Generating {file.lyx_file_name}...')
        os.system(f'{PATH_LYX} --export-to xhtml "{file.xhtml_path}" "{file.lyx_path}"')
        print(f'> Done generating {file.lyx_path} to {file.xhtml_path}')
        
        # print()
        # print(open(file.xhtml_path, 'r').read())

    print(f'Exported successfully all {len(all_lyx_files)} files!')

    # Validate the XHTML files.
    print(f'Validating {len(all_lyx_files)} XHTML files...')
    
    n_valid = 0
    n_invalid = 0
    for file in all_files:
        print(f'* Validating {file.xhtml_file_name}...')
        error_count = validator.validate([file.xhtml_file_name])
        # Caution: this call outputs all validation errors to stdout!
        # This line is equivalent to running vnu on the corresponding file.
        # https://github.com/validator/validator
        
        if error_count == 0:
            n_valid += 1
            print(f'> Found no validation error!')
        else:
            n_invalid += 1
            print(f'> Found {error_count} validation error{"" if error_count == 1 else "s"}!')

    print(f'Validated all {len(all_lyx_files)} files! Among them:')
    print(f'> {n_valid} were valid ({100.0 * n_valid / len(all_files)}%)')
    print(f'> {n_invalid} were invalid ({100.0 * n_invalid / len(all_files)}%)')
    
    if n_invalid == 0:
        print("That's excellent! Give yourself a pat on the back!")
    elif 100.0 * n_invalid / len(all_files) <= 5.0:
        print("That's a pretty good job!")
