/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "language.h"
#include "gettext.h"
#include "encoding.h"
#include "lyxrc.h"
#include "lyxlex.h"
#include "debug.h"

using std::endl;

Languages languages;
Language const * english_language;
Language const * default_language;
Language ignore_lang("ignore", "ignore", "Ignore", false, 0, "ignore", "");
Language const * ignore_language = &ignore_lang;
Language latex_lang("latex", "latex", "Latex", false, 0, "latex", "");
Language const * latex_language = &latex_lang;
#ifdef INHERIT_LANG
Language inherit_lang("inherit", "inherit", "Inherit", false, 0, "inherit", "");
Language const * inherit_language = &inherit_lang;
#endif

void Languages::setDefaults()
{
	languagelist["english"] = Language("english", "english", N_("English"),
					   false, 
					   encodings.getEncoding("iso8859-1"),
					   "en", "");
	english_language = default_language = &languagelist["english"];
}

void Languages::read(string const & filename)
{
	LyXLex lex(0, 0);
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
		lyxerr[Debug::INIT] << "Reading language " << lang << endl;

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

		Encoding const * encoding = encodings.getEncoding(encoding_str);
		if (!encoding) {
			encoding = encodings.getEncoding("iso8859-1");
			lyxerr << "Unknown encoding " << encoding_str << endl; 
		}

		languagelist[lang] = Language(lang, babel, display, rtl, 
					      encoding, code, latex_options);
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
	if (it != languagelist.end())
		return &it->second;
	else
		return 0;
}
