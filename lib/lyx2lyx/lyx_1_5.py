# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2006 José Matos <jamatos@lyx.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

from parser_tools import find_token_exact, find_tokens, get_value

##
#  Notes: Framed/Shaded
#

def revert_framed(file):
    i = 0
    while 1:
        i = find_tokens(file.body, ["\\begin_inset Note Framed", "\\begin_inset Note Shaded"], i)

        if i == -1:
            return
        file.body[i] = "\\begin_inset Note"
        i = i + 1


##
#  Fonts
#

roman_fonts      = {'default' : 'default', 'ae'       : 'ae',
                    'times'   : 'times',   'palatino' : 'palatino',
                    'helvet'  : 'default', 'avant'    : 'default',
                    'newcent' : 'newcent', 'bookman'  : 'bookman',
                    'pslatex' : 'times'}
sans_fonts       = {'default' : 'default', 'ae'       : 'default',
                    'times'   : 'default', 'palatino' : 'default',
                    'helvet'  : 'helvet',  'avant'    : 'avant',
                    'newcent' : 'default', 'bookman'  : 'default',
                    'pslatex' : 'helvet'}
typewriter_fonts = {'default' : 'default', 'ae'       : 'default',
                    'times'   : 'default', 'palatino' : 'default',
                    'helvet'  : 'default', 'avant'    : 'default',
                    'newcent' : 'default', 'bookman'  : 'default',
                    'pslatex' : 'courier'}

def convert_font_settings(file):
    i = 0
    i = find_token_exact(file.header, "\\fontscheme", i)
    if i == -1:
        file.warning("Malformed LyX file: Missing `\\fontscheme'.")
        return
    font_scheme = get_value(file.header, "\\fontscheme", i, i + 1)
    if font_scheme == '':
        file.warning("Malformed LyX file: Empty `\\fontscheme'.")
        font_scheme = 'default'
    if not font_scheme in roman_fonts.keys():
        file.warning("Malformed LyX file: Unknown `\\fontscheme' `%s'." % font_scheme)
        font_scheme = 'default'
    file.header[i:i+1] = ['\\font_roman %s' % roman_fonts[font_scheme],
                          '\\font_sans %s' % sans_fonts[font_scheme],
                          '\\font_typewriter %s' % typewriter_fonts[font_scheme],
                          '\\font_default_family default',
                          '\\font_sc false',
                          '\\font_osf false',
                          '\\font_sf_scale 100',
                          '\\font_tt_scale 100']


def revert_font_settings(file):
    i = 0
    insert_line = -1
    fonts = {'roman' : 'default', 'sans' : 'default', 'typewriter' : 'default'}
    for family in 'roman', 'sans', 'typewriter':
        name = '\\font_%s' % family
        i = find_token_exact(file.header, name, i)
        if i == -1:
            file.warning("Malformed LyX file: Missing `%s'." % name)
            i = 0
        else:
            if (insert_line < 0):
                insert_line = i
            fonts[family] = get_value(file.header, name, i, i + 1)
            del file.header[i]
    i = find_token_exact(file.header, '\\font_default_family', i)
    if i == -1:
        file.warning("Malformed LyX file: Missing `\\font_default_family'.")
        font_default_family = 'default'
    else:
        font_default_family = get_value(file.header, "\\font_default_family", i, i + 1)
        del file.header[i]
    i = find_token_exact(file.header, '\\font_sc', i)
    if i == -1:
        file.warning("Malformed LyX file: Missing `\\font_sc'.")
        font_sc = 'false'
    else:
        font_sc = get_value(file.header, '\\font_sc', i, i + 1)
        del file.header[i]
    if font_sc != 'false':
        file.warning("Conversion of '\\font_sc' not yet implemented.")
    i = find_token_exact(file.header, '\\font_osf', i)
    if i == -1:
        file.warning("Malformed LyX file: Missing `\\font_osf'.")
        font_osf = 'false'
    else:
        font_osf = get_value(file.header, '\\font_osf', i, i + 1)
        del file.header[i]
    i = find_token_exact(file.header, '\\font_sf_scale', i)
    if i == -1:
        file.warning("Malformed LyX file: Missing `\\font_sf_scale'.")
        font_sf_scale = '100'
    else:
        font_sf_scale = get_value(file.header, '\\font_sf_scale', i, i + 1)
        del file.header[i]
    if font_sf_scale != '100':
        file.warning("Conversion of '\\font_sf_scale' not yet implemented.")
    i = find_token_exact(file.header, '\\font_tt_scale', i)
    if i == -1:
        file.warning("Malformed LyX file: Missing `\\font_tt_scale'.")
        font_tt_scale = '100'
    else:
        font_tt_scale = get_value(file.header, '\\font_tt_scale', i, i + 1)
        del file.header[i]
    if font_tt_scale != '100':
        file.warning("Conversion of '\\font_tt_scale' not yet implemented.")
    for font_scheme in roman_fonts.keys():
        if (roman_fonts[font_scheme] == fonts['roman'] and
            sans_fonts[font_scheme] == fonts['sans'] and
            typewriter_fonts[font_scheme] == fonts['typewriter']):
            file.header.insert(insert_line, '\\fontscheme %s' % font_scheme)
            if font_default_family != 'default':
                file.preamble.append('\\renewcommand{\\familydefault}{\\%s}' % font_default_family)
            if font_osf == 'true':
                file.warning("Ignoring `\\font_osf = true'")
            return
    font_scheme = 'default'
    file.header.insert(insert_line, '\\fontscheme %s' % font_scheme)
    if fonts['roman'] == 'cmr':
        file.preamble.append('\\renewcommand{\\rmdefault}{cmr}')
        if font_osf == 'true':
            file.preamble.append('\\usepackage{eco}')
            font_osf = 'false'
    for font in 'lmodern', 'charter', 'utopia', 'beraserif', 'ccfonts', 'chancery':
        if fonts['roman'] == font:
            file.preamble.append('\\usepackage{%s}' % font)
    for font in 'cmss', 'lmss', 'cmbr':
        if fonts['sans'] == font:
            file.preamble.append('\\renewcommand{\\sfdefault}{%s}' % font)
    for font in 'berasans':
        if fonts['sans'] == font:
            file.preamble.append('\\usepackage{%s}' % font)
    for font in 'cmtt', 'lmtt', 'cmtl':
        if fonts['typewriter'] == font:
            file.preamble.append('\\renewcommand{\\ttdefault}{%s}' % font)
    for font in 'courier', 'beramono', 'luximono':
        if fonts['typewriter'] == font:
            file.preamble.append('\\usepackage{%s}' % font)
    if font_default_family != 'default':
        file.preamble.append('\\renewcommand{\\familydefault}{\\%s}' % font_default_family)
    if font_osf == 'true':
        file.warning("Ignoring `\\font_osf = true'")


##
# Conversion hub
#

convert = [[246, []],
           [247, [convert_font_settings]]]

revert  = [[246, [revert_font_settings]],
           [245, [revert_framed]]]

if __name__ == "__main__":
    pass

