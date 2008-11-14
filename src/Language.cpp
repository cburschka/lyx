/**
 * \file Language.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Language.h"

#include "Encoding.h"
#include "Lexer.h"
#include "LyXRC.h"

#include "support/debug.h"
#include "support/FileName.h"

using namespace std;
using namespace lyx::support;


namespace lyx {

Languages languages;
Language ignore_lang;
Language latex_lang;
Language const * default_language;
Language const * ignore_language = &ignore_lang;
Language const * latex_language = &latex_lang;
Language const * reset_language = 0;


bool Language::read(Lexer & lex)
{
	encoding_ = 0;
	lex >> lang_;
	lex >> babel_;
	lex >> display_;
	lex >> rightToLeft_;
	lex >> encodingStr_;
	lex >> code_;
	lex >> latex_options_;
	if (!lex)
		return false;

	encoding_ = encodings.fromLyXName(encodingStr_);
	if (!encoding_ && !encodingStr_.empty()) {
		encoding_ = encodings.fromLyXName("iso8859-1");
		LYXERR0("Unknown encoding " << encodingStr_);
	}
	return true;
}


void Languages::read(FileName const & filename)
{
	Lexer lex;
	lex.setFile(filename);
	lex.setContext("Languages::read");
	while (1) {
		Language l;
		l.read(lex);
		if (!lex)
			break;
		LYXERR(Debug::INFO, "Reading language " << l.lang());
		if (l.lang() == "latex")
			latex_lang = l;
		else if (l.lang() == "ignore")
			ignore_lang = l;
		else
			languagelist[l.lang()] = l;
	}

	default_language = getLanguage(lyxrc.default_language);
	if (!default_language) {
		LYXERR0("Default language \"" << lyxrc.default_language
		       << "\" not found!");
		default_language = getLanguage("english");
		if (!default_language)
			default_language = &(*languagelist.begin()).second;
		LYXERR0("Using \"" << default_language->lang() << "\" instead!");
	}
}


Language const * Languages::getLanguage(string const & language) const
{
	if (language == "reset")
		return reset_language;
	if (language == "ignore")
		return ignore_language;
	const_iterator it = languagelist.find(language);
	return it == languagelist.end() ? reset_language : &it->second;
}


} // namespace lyx
