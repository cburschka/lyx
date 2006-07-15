#!/usr/bin/env python
# -*- coding: iso-8859-15 -*-

# file convertDefault.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# \author Herbert Voß
# \author Bo Peng

# Full author contact details are available in file CREDITS.

# The default converter if no other has been defined by the user from the
# Conversion->Converter tab of the Preferences dialog.

# The user can also redefine this default converter, placing their
# replacement in ~/.lyx/scripts

# converts an image from $1 to $2 format
import os, sys
if os.system(r'convert -depth 8 "%s" "%s"' % (sys.argv[1], sys.argv[2])) != 0:
    print >> sys.stderr, sys.argv[0], 'ERROR'
    print >> sys.stderr, 'Execution of "convert" failed.'
    sys.exit(1)
