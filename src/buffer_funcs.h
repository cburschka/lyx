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

#include <string>

namespace lyx {

namespace support { class FileName; }

class Buffer;
class DocIterator;
class ParIterator;

/**
 *  Checks and loads a LyX file \param filename.
 *  \retval the newly created \c Buffer pointer if successful or 0.
 *  \retval 0 if the \c Buffer could not be created.
 */
Buffer * checkAndLoadLyXFile(support::FileName const & filename,
		bool acceptDirty = false);

/** Make a new file (buffer) with name \c filename based on a template
 *  named \c templatename
 */
Buffer * newFile(std::string const & filename, std::string const & templatename,
		 bool isNamed = false);

/** Make a new unnamed file (buffer) based on a template
 *  named \c templatename
 */
Buffer * newUnnamedFile(std::string const & templatename,
	support::FileName const & path);

/// Count the number of words in the text between these two iterators
int countWords(DocIterator const & from, DocIterator const & to);

/// Count the number of chars in the text between these two iterators
int countChars(DocIterator const & from, DocIterator const & to, bool with_blanks);

} // namespace lyx

#endif // BUFFER_FUNCS_H
