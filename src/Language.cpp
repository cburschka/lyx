/**
 * \file Language.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Jürgen Spitzmüller
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
#include "support/filetools.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/Messages.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

Languages languages;
Language const * ignore_language = 0;
Language const * default_language = 0;
Language const * latex_language = 0;
Language const * reset_language = 0;


bool Language::isPolyglossiaExclusive() const
{
	return babel().empty() && !polyglossia().empty() && requires().empty();
}


docstring const Language::translateLayout(string const & m) const
{
	if (m.empty())
		return docstring();

	if (!isAscii(m)) {
		lyxerr << "Warning: not translating `" << m
		       << "' because it is not pure ASCII.\n";
		return from_utf8(m);
	}

	TranslationMap::const_iterator it = layoutTranslations_.find(m);
	if (it != layoutTranslations_.end())
		return it->second;

	docstring t = from_ascii(m);
	cleanTranslation(t);
	return t;
}


bool Language::readLanguage(Lexer & lex)
{
	enum LanguageTags {
		LA_AS_BABELOPTS = 1,
		LA_BABELNAME,
		LA_ENCODING,
		LA_END,
		LA_FONTENC,
		LA_GUINAME,
		LA_HAS_GUI_SUPPORT,
		LA_INTERNAL_ENC,
		LA_LANG_CODE,
		LA_LANG_VARIETY,
		LA_POLYGLOSSIANAME,
		LA_POLYGLOSSIAOPTS,
		LA_POSTBABELPREAMBLE,
		LA_QUOTESTYLE,
		LA_PREBABELPREAMBLE,
		LA_REQUIRES,
		LA_RTL
	};

	// Keep these sorted alphabetically!
	LexerKeyword languageTags[] = {
		{ "asbabeloptions",       LA_AS_BABELOPTS },
		{ "babelname",            LA_BABELNAME },
		{ "encoding",             LA_ENCODING },
		{ "end",                  LA_END },
		{ "fontencoding",         LA_FONTENC },
		{ "guiname",              LA_GUINAME },
		{ "hasguisupport",        LA_HAS_GUI_SUPPORT },
		{ "internalencoding",     LA_INTERNAL_ENC },
		{ "langcode",             LA_LANG_CODE },
		{ "langvariety",          LA_LANG_VARIETY },
		{ "polyglossianame",      LA_POLYGLOSSIANAME },
		{ "polyglossiaopts",      LA_POLYGLOSSIAOPTS },
		{ "postbabelpreamble",    LA_POSTBABELPREAMBLE },
		{ "prebabelpreamble",     LA_PREBABELPREAMBLE },
		{ "quotestyle",           LA_QUOTESTYLE },
		{ "requires",             LA_REQUIRES },
		{ "rtl",                  LA_RTL }
	};

	bool error = false;
	bool finished = false;
	lex.pushTable(languageTags);
	// parse style section
	while (!finished && lex.isOK() && !error) {
		int le = lex.lex();
		// See comment in LyXRC.cpp.
		switch (le) {
		case Lexer::LEX_FEOF:
			continue;

		case Lexer::LEX_UNDEF: // parse error
			lex.printError("Unknown language tag `$$Token'");
			error = true;
			continue;

		default:
			break;
		}
		switch (static_cast<LanguageTags>(le)) {
		case LA_END: // end of structure
			finished = true;
			break;
		case LA_AS_BABELOPTS:
			lex >> as_babel_options_;
			break;
		case LA_BABELNAME:
			lex >> babel_;
			break;
		case LA_POLYGLOSSIANAME:
			lex >> polyglossia_name_;
			break;
		case LA_POLYGLOSSIAOPTS:
			lex >> polyglossia_opts_;
			break;
		case LA_QUOTESTYLE:
			lex >> quote_style_;
			break;
		case LA_ENCODING:
			lex >> encodingStr_;
			break;
		case LA_FONTENC:
			lex >> fontenc_;
			break;
		case LA_GUINAME:
			lex >> display_;
			break;
		case LA_HAS_GUI_SUPPORT:
			lex >> has_gui_support_;
			break;
		case LA_INTERNAL_ENC:
			lex >> internal_enc_;
			break;
		case LA_LANG_CODE:
			lex >> code_;
			break;
		case LA_LANG_VARIETY:
			lex >> variety_;
			break;
		case LA_POSTBABELPREAMBLE:
			babel_postsettings_ =
				lex.getLongString("EndPostBabelPreamble");
			break;
		case LA_PREBABELPREAMBLE:
			babel_presettings_ =
				lex.getLongString("EndPreBabelPreamble");
			break;
		case LA_REQUIRES:
			lex >> requires_;
			break;
		case LA_RTL:
			lex >> rightToLeft_;
			break;
		}
	}
	lex.popTable();
	return finished && !error;
}


bool Language::read(Lexer & lex)
{
	as_babel_options_ = 0;
	encoding_ = 0;
	internal_enc_ = 0;
	rightToLeft_ = 0;

	if (!lex.next()) {
		lex.printError("No name given for language: `$$Token'.");
		return false;
	}

	lang_ = lex.getString();
	LYXERR(Debug::INFO, "Reading language " << lang_);
	if (!readLanguage(lex)) {
		LYXERR0("Error parsing language `" << lang_ << '\'');
		return false;
	}

	encoding_ = encodings.fromLyXName(encodingStr_);
	if (!encoding_ && !encodingStr_.empty()) {
		encoding_ = encodings.fromLyXName("iso8859-1");
		LYXERR0("Unknown encoding " << encodingStr_);
	}
	return true;
}


void Language::readLayoutTranslations(Language::TranslationMap const & trans, bool replace)
{
	TranslationMap::const_iterator const end = trans.end();
	for (TranslationMap::const_iterator it = trans.begin(); it != end; ++it) {
		if (replace
			|| layoutTranslations_.find(it->first) == layoutTranslations_.end())
			layoutTranslations_[it->first] = it->second;
	}
}


void Languages::read(FileName const & filename)
{
	Lexer lex;
	lex.setFile(filename);
	lex.setContext("Languages::read");
	while (lex.isOK()) {
		int le = lex.lex();
		switch (le) {
		case Lexer::LEX_FEOF:
			continue;

		default:
			break;
		}
		if (lex.getString() != "Language") {
			lex.printError("Unknown Language tag `$$Token'");
			continue;
		}
		Language l;
		l.read(lex);
		if (!lex)
			break;
		if (l.lang() == "latex") {
			// Check if latex language was not already defined.
			LASSERT(latex_language == 0, continue);
			static const Language latex_lang = l;
			latex_language = &latex_lang;
		} else if (l.lang() == "ignore") {
			// Check if ignore language was not already defined.
			LASSERT(ignore_language == 0, continue);
			static const Language ignore_lang = l;
			ignore_language = &ignore_lang;
		} else
			languagelist[l.lang()] = l;
	}

	default_language = getLanguage("english");
	if (!default_language) {
		LYXERR0("Default language \"english\" not found!");
		default_language = &(*languagelist.begin()).second;
		LYXERR0("Using \"" << default_language->lang() << "\" instead!");
	}

	// Read layout translations
	FileName const path = libFileSearch(string(), "layouttranslations");
	readLayoutTranslations(path);
}


namespace {

bool readTranslations(Lexer & lex, Language::TranslationMap & trans)
{
	while (lex.isOK()) {
		if (lex.checkFor("End"))
			break;
		if (!lex.next(true))
			return false;
		string const key = lex.getString();
		if (!lex.next(true))
			return false;
		docstring const val = lex.getDocString();
		trans[key] = val;
	}
	return true;
}


enum Match {
	NoMatch,
	ApproximateMatch,
	ExactMatch
};


Match match(string const & code, Language const & lang)
{
	// we need to mimic gettext: code can be a two-letter code, which
	// should match all variants, e.g. "de" should match "de_DE",
	// "de_AT" etc.
	// special case for chinese:
	// simplified  => code == "zh_CN", langcode == "zh_CN"
	// traditional => code == "zh_TW", langcode == "zh_CN"
	string const variety = lang.variety();
	string const langcode = variety.empty() ?
	                        lang.code() : lang.code() + '_' + variety;
	string const name = lang.lang();
	if ((code == langcode && name != "chinese-traditional")
		|| (code == "zh_TW"  && name == "chinese-traditional"))
		return ExactMatch;
	if ((code.size() == 2) && (langcode.size() > 2)
		&& (code + '_' == langcode.substr(0, 3)))
		return ApproximateMatch;
	return NoMatch;
}

}


void Languages::readLayoutTranslations(support::FileName const & filename)
{
	Lexer lex;
	lex.setFile(filename);
	lex.setContext("Languages::read");

	// 1) read all translations (exact and approximate matches) into trans
	typedef std::map<string, Language::TranslationMap> TransMap;
	TransMap trans;
	LanguageList::iterator const lbeg = languagelist.begin();
	LanguageList::iterator const lend = languagelist.end();
	while (lex.isOK()) {
		if (!lex.checkFor("Translation")) {
			if (lex.isOK())
				lex.printError("Unknown layout translation tag `$$Token'");
			break;
		}
		if (!lex.next(true))
			break;
		string const code = lex.getString();
		bool found = false;
		for (LanguageList::iterator lit = lbeg; lit != lend; ++lit) {
			if (match(code, lit->second) != NoMatch) {
				found = true;
				break;
			}
		}
		if (!found) {
			lex.printError("Unknown language `" + code + "'");
			break;
		}
		if (!readTranslations(lex, trans[code])) {
			lex.printError("Could not read layout translations for language `"
				+ code + "'");
			break;
		}
	}

	// 2) merge all translations into the languages
	// exact translations overwrite approximate ones
	TransMap::const_iterator const tbeg = trans.begin();
	TransMap::const_iterator const tend = trans.end();
	for (TransMap::const_iterator tit = tbeg; tit != tend; ++tit) {
		for (LanguageList::iterator lit = lbeg; lit != lend; ++lit) {
			Match const m = match(tit->first, lit->second);
			if (m == NoMatch)
				continue;
			lit->second.readLayoutTranslations(tit->second,
			                                   m == ExactMatch);
		}
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
