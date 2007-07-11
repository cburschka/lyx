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
#include "support/docstring.h"

#include <string>


namespace lyx {

namespace support { class FileName; }

class Buffer;
class DocIterator;
class ErrorList;
class TeXErrors;
class ParIterator;

/**
 *  Loads a LyX file \c filename into \c Buffer
 *  and \return success status.
 */
bool loadLyXFile(Buffer *, support::FileName const & filename);

/**
 *  Checks and loads a LyX file \param filename.
 *  \retval the newly created \c Buffer pointer if successful or 0.
 *  \retval 0 if the \c Buffer could not be created.
 */
Buffer * checkAndLoadLyXFile(support::FileName const & filename);

/* Make a new file (buffer) with name \c filename based on a template
 * named \c templatename
 */
Buffer * newFile(std::string const & filename, std::string const & templatename,
		 bool isNamed = false);

///return the format of the buffer on a string
std::string const bufferFormat(Buffer const & buffer);

/// Fill in the ErrorList with the TeXErrors
void bufferErrors(Buffer const &, TeXErrors const &, ErrorList &);

/// Count the number of words in the text between these two iterators
int countWords(DocIterator const & from, DocIterator const & to);

/// updates all counters
void updateLabels(Buffer const &, bool childonly = false);

///
void checkBufferStructure(Buffer &, ParIterator const &);

} // namespace lyx

#endif // BUFFER_FUNCS_H
