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

#include "debug.h"
#include "Encoding.h"
#include "Lexer.h"
#include "LyXRC.h"

#include "support/FileName.h"

using std::endl;
using std::string;


namespace lyx {


Languages languages;
Language const * english_language;
Language const * default_language;
Language ignore_lang("ignore", "ignore", "Ignore", false, "", 0, "ignore", "");
Language const * ignore_language = &ignore_lang;
Language latex_lang("latex", "latex", "Latex", false, "", 0, "latex", "");
Language const * latex_language = &latex_lang;


void Languages::read(support::FileName const & filename)
{
	// We need to set the encoding of latex_lang
	latex_lang = Language("latex", "latex", "Latex", false, "iso8859-1",
			      encodings.getFromLyXName("iso8859-1"),
			      "latex", "");

	Lexer lex(0, 0);
	lex.setFile(filename);
	while (lex.isOK()) {
		string lang;
		string babel;
		string display;
		string encoding_str;
		string code;
		string latex_options;
		bool rtl = false;

		if (lex.next())
			lang = lex.getString();
		else
			break;
		LYXERR(Debug::INFO) << "Reading language " << lang << endl;

		if (lex.next())
			babel = lex.getString();
		if (lex.next())
			display = lex.getString();
		if (lex.next())
			rtl = lex.getBool();
		if (lex.next())
			encoding_str = lex.getString();
		if (lex.next())
			code = lex.getString();
		if (lex.next())
			latex_options = lex.getString();

		Encoding const * encoding = encodings.getFromLyXName(encoding_str);
		if (!encoding) {
			encoding = encodings.getFromLyXName("iso8859-1");
			lyxerr << "Unknown encoding " << encoding_str << endl;
		}

		languagelist[lang] = Language(lang, babel, display, rtl,
					      encoding_str, encoding, code, latex_options);
	}

	default_language = getLanguage(lyxrc.default_language);
	if (!default_language) {
		lyxerr << "Default language \"" << lyxrc.default_language
		       << "\" not found!" << endl;
		default_language = getLanguage("english");
		if (!default_language)
			default_language = &(*languagelist.begin()).second;
		lyxerr << "Using \"" << default_language->lang()
		       << "\" instead!" << endl;
	}
	english_language = getLanguage("english");
	if (!english_language)
		english_language = default_language;
}


Language const * Languages::getLanguage(string const & language) const
{
	const_iterator it = languagelist.find(language);
	return it == languagelist.end() ? 0 : &it->second;
}


} // namespace lyx
