// -*- C++ -*-
/* \file buffer_funcs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS
 */

#include "LString.h"

class Buffer;

/**
 *  Loads a LyX file \c filename into \c Buffer 
 *  and \return success status.
 */
bool loadLyXFile(Buffer *, string const & filename);

/* Make a new file (buffer) with name \c filename based on a template
 * named \c templatename
 */
Buffer * newFile(string const & filename, string const & templatename, 
		 bool isNamed = false);
