/**
 * \file text_funcs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 *
 * This file contains some utility functions for actually mutating
 * the text contents of a document 
 */

#ifndef TEXT_FUNCS_H
#define TEXT_FUNCS_H

#include <config.h>

class LyXText;
class LyXCursor;

void transposeChars(LyXText & text, LyXCursor const & cursor);

#endif // TEXT_FUNCS_H
