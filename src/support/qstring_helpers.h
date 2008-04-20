// -*- C++ -*-
/**
 * \file qstring_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSTRING_HELPERS_H
#define QSTRING_HELPERS_H

#include "support/strfwd.h"

class QString;

namespace lyx {

/**
 * toqstr - convert a UTF8 encoded char * into a QString
 *
 * This should not be used, since all possibly non-ASCII stuff should be
 * stored in a docstring.
 */
QString toqstr(char const * str);


/**
 * toqstr - convert a UTF8 encoded std::string into a QString
 *
 * This should not be used, since all possibly non-ASCII stuff should be
 * stored in a docstring.
 */
QString toqstr(std::string const & str);


/// Is \p c a valid utf16 char?
inline bool is_utf16(unsigned int c)
{
	// 0xd800 ... 0xdfff is the range of surrogate pairs.
	return c < 0xd800 || (c > 0xdfff && c < 0x10000);
}


/**
 * toqstr - convert a UCS4 encoded docstring into a QString
 *
 * This is the preferred method of converting anything that possibly
 * contains non-ASCII stuff to QString.
 */
QString toqstr(docstring const & ucs4);

/**
 * toqstr - convert a UCS4 encoded character into a QString
 *
 * This is the preferred method of converting anything that possibly
 * contains non-ASCII stuff to QString.
 */
QString toqstr(char_type ucs4);

/**
 * qstring_to_ucs4 - convert a QString into a UCS4 encoded docstring
 *
 * This is the preferred method of converting anything that possibly
 * contains non-ASCII stuff to docstring.
 */
docstring qstring_to_ucs4(QString const & qstr);

/**
 * fromqstr - convert a QString into a UTF8 encoded std::string
 *
 * This should not be used except for output to lyxerr, since all possibly
 * non-ASCII stuff should be stored in a docstring.
 */
std::string fromqstr(QString const & str);

} // namespace lyx

#endif // QSTRING_HELPERS_H
