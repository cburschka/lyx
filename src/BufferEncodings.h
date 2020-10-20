// -*- C++ -*-
/**
 * \file BufferEncodings.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Stephan Witt
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFER_ENCODINGS_H
#define BUFFER_ENCODINGS_H

#include "Encoding.h"

#include "support/strfwd.h"

namespace lyx {

class Buffer;
class LaTeXFeatures;

class BufferEncodings : public Encodings {
public:
	/**
	 * Initialize mathcmd, textcmd, and mathsym sets.
	 */
	static void initUnicodeMath(Buffer const & buffer, bool for_master = true);
	/**
	 * If \p c cannot be encoded in the given \p encoding, convert
	 * it to something that LaTeX can understand in mathmode.
	 * \p needsTermination indicates whether the command needs to be
	 * terminated by {} or a space.
	 * \return whether \p command is a mathmode command
	 */
	static void validate(char_type c, LaTeXFeatures & features, bool for_mathed = false);
};

} // namespace lyx

#endif // BUFFER_ENCODINGS_H
