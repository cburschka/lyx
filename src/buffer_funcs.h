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

#ifndef BUFFER_FUNCS_H
#define BUFFER_FUNCS_H

#include "LString.h"

class Buffer;
class TeXErrors;
class ErrorList;

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

///return the format of the buffer on a string
string const BufferFormat(Buffer const & buffer);

void parseErrors(Buffer const &, TeXErrors const &);

void parseErrors(Buffer const &, ErrorList const &);

#endif // BUFFER_FUNCS_H
