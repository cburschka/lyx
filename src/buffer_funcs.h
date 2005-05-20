// -*- C++ -*-
/* \file buffer_funcs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFER_FUNCS_H
#define BUFFER_FUNCS_H

#include "lyxlayout_ptr_fwd.h"

#include <string>


class Buffer;
class DocIterator;
class ErrorList;
class TeXErrors;

/**
 *  Loads a LyX file \c filename into \c Buffer
 *  and \return success status.
 */
bool loadLyXFile(Buffer *, std::string const & filename);

/* Make a new file (buffer) with name \c filename based on a template
 * named \c templatename
 */
Buffer * newFile(std::string const & filename, std::string const & templatename,
		 bool isNamed = false);

///return the format of the buffer on a string
std::string const BufferFormat(Buffer const & buffer);
///
void bufferErrors(Buffer const &, TeXErrors const &);
///
void bufferErrors(Buffer const &, ErrorList const &);

/// Count the number of words in the text between these two iterators
int countWords(DocIterator const & from, DocIterator const & to);

/// Expand the counters for the labelstring of \c layout
std::string expandLabel(Buffer const & buf, LyXLayout_ptr const & layout, 
			bool appendix);

/// updates all counters
void updateCounters(Buffer const &);


#endif // BUFFER_FUNCS_H
