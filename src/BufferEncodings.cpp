/**
 * \file BufferEncodings.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Dekel Tsur
 * \author Stephan Witt
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferEncodings.h"

#include "Buffer.h"
#include "InsetIterator.h"
#include "LaTeXFeatures.h"

#include "support/lstrings.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

void BufferEncodings::initUnicodeMath(Buffer const & buffer, bool for_master)
{
	if (for_master) {
		mathcmd.clear();
		textcmd.clear();
		mathsym.clear();
	}

	// Check this buffer
	Inset & inset = buffer.inset();
	InsetIterator it = inset_iterator_begin(inset);
	InsetIterator const end = inset_iterator_end(inset);
	for (; it != end; ++it)
		it->initUnicodeMath();

	if (!for_master)
		return;

	// Check children
	for (Buffer * buf : buffer.getDescendants())
		initUnicodeMath(*buf, false);
}


void BufferEncodings::validate(char_type c, LaTeXFeatures & features, bool for_mathed)
{
	CharInfo const & ci = Encodings::unicodeCharInfo(c);
	if (ci.isUnicodeSymbol()) {
		// In mathed, c could be used both in textmode and mathmode
		docstring const textcommand = ci.textcommand();
		bool const math_mode = for_mathed && isMathCmd(c);
		bool const use_math = math_mode ||
		                      (!for_mathed && textcommand.empty());
		bool const use_text = (for_mathed && isTextCmd(c)) ||
		                      (!for_mathed && !textcommand.empty());
		bool const plain_utf8 = (features.runparams().encoding->name() == "utf8-plain");
		bool const unicode_math = (features.isRequired("unicode-math")
			&& features.isAvailable("unicode-math"));
		// with utf8-plain, we only load packages when in mathed (see #7766)
		// and if we do not use unicode-math
		if ((math_mode && !unicode_math)
		     || (use_math && !plain_utf8)) {
			string const mathpreamble = ci.mathpreamble();
			if (!mathpreamble.empty()) {
				if (ci.mathfeature()) {
					string feats = mathpreamble;
					while (!feats.empty()) {
						string feat;
						feats = split(feats, feat, ',');
						features.require(feat);
					}
				} else
					features.addPreambleSnippet(from_utf8(mathpreamble));
			}
		}
		// with utf8-plain, we do not load packages (see #7766)
		if (use_text && !plain_utf8) {
			string const textpreamble = ci.textpreamble();
			if (!textpreamble.empty()) {
				if (ci.textfeature()) {
					string feats = textpreamble;
					while (!feats.empty()) {
						string feat;
						feats = split(feats, feat, ',');
						// context-dependent features are handled
						// in Paragraph::Private::validate()
						if (!contains(feat, '='))
							features.require(feat);
					}
				} else
					features.addPreambleSnippet(from_utf8(textpreamble));
			}
		}
	}
	if (for_mathed && isMathSym(c)) {
		features.require("amstext");
		features.require("lyxmathsym");
	}
}

} // namespace lyx
