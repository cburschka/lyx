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

#include "BufferParams.h"
#include "Encoding.h"
#include "LaTeXFonts.h"
#include "Lexer.h"
#include "LyXRC.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/qstring_helpers.h"
#include "support/Messages.h"

#include <QLocale>
#include <QString>

using namespace std;
using namespace lyx::support;

namespace lyx {

Languages languages;
Language const * ignore_language = nullptr;
Language const * default_language = nullptr;
Language const * latex_language = nullptr;
Language const * reset_language = nullptr;


bool Language::isPolyglossiaExclusive() const
{
	return babel().empty() && !polyglossia().empty() && required().empty();
}


bool Language::isBabelExclusive() const
{
	return !babel().empty() && polyglossia().empty() && required().empty();
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


string Language::fontenc(BufferParams const & params) const
{
	// Don't use LaTeX fonts, so just return the language's preferred
	// (although this is not used with nonTeXFonts anyway).
	if (params.useNonTeXFonts)
		return fontenc_.front() == "ASCII" ? "T1" : fontenc_.front();

	// Determine optimal font encoding
	// We check whether the used rm font supports an encoding our language supports
	LaTeXFont const & lf =
		theLaTeXFonts().getLaTeXFont(from_ascii(params.fontsRoman()));
	vector<string> const lfe = lf.fontencs();
	for (auto & fe : fontenc_) {
		// ASCII means: support all T* encodings plus OT1
		if (fe == "ASCII") {
			for (auto & afe : lfe) {
				if (afe == "OT1" || prefixIs(afe, "T"))
					// we found a suitable one; return that.
					return afe;
			}
		}
		// For other encodings, just check whether the font supports it
		if (lf.hasFontenc(fe))
			return fe;
	}
	// We did not find a suitable one; just take the first in the list,
	// the priorized one (which is "T1" for ASCII).
	return fontenc_.front() == "ASCII" ? "T1" : fontenc_.front();
}


string Language::dateFormat(size_t i) const
{
	if (i > dateformats_.size())
		return string();
	return dateformats_.at(i);
}


docstring Language::decimalSeparator() const
{
	if (lyxrc.default_decimal_sep == "locale") {
		QLocale loc = QLocale(toqstr(code()));
		return qstring_to_ucs4(QString(loc.decimalPoint()));
	}
	return from_utf8(lyxrc.default_decimal_sep);
}


bool Language::readLanguage(Lexer & lex)
{
	enum LanguageTags {
		LA_BABELNAME = 1,
		LA_DATEFORMATS,
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
		LA_XINDYNAME,
		LA_POSTBABELPREAMBLE,
		LA_PREBABELPREAMBLE,
		LA_PROVIDES,
		LA_REQUIRES,
		LA_QUOTESTYLE,
		LA_RTL,
		LA_WORDWRAP,
		LA_ACTIVECHARS
	};

	// Keep these sorted alphabetically!
	LexerKeyword languageTags[] = {
		{ "activechars",          LA_ACTIVECHARS },
		{ "babelname",            LA_BABELNAME },
		{ "dateformats",          LA_DATEFORMATS },
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
		{ "provides",             LA_PROVIDES },
		{ "quotestyle",           LA_QUOTESTYLE },
		{ "requires",             LA_REQUIRES },
		{ "rtl",                  LA_RTL },
		{ "wordwrap",             LA_WORDWRAP },
		{ "xindyname",            LA_XINDYNAME }
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
		case LA_BABELNAME:
			lex >> babel_;
			break;
		case LA_POLYGLOSSIANAME:
			lex >> polyglossia_name_;
			break;
		case LA_POLYGLOSSIAOPTS:
			lex >> polyglossia_opts_;
			break;
		case LA_XINDYNAME:
			lex >> xindy_;
			break;
		case LA_QUOTESTYLE:
			lex >> quote_style_;
			break;
		case LA_ACTIVECHARS:
			lex >> active_chars_;
			break;
		case LA_ENCODING:
			lex >> encodingStr_;
			break;
		case LA_FONTENC: {
			lex.eatLine();
			vector<string> const fe =
				getVectorFromString(lex.getString(true), "|");
			fontenc_.insert(fontenc_.end(), fe.begin(), fe.end());
			break;
		}
		case LA_DATEFORMATS: {
			lex.eatLine();
			vector<string> const df =
				getVectorFromString(trim(lex.getString(true), "\""), "|");
			dateformats_.insert(dateformats_.end(), df.begin(), df.end());
			break;
		}
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
				lex.getLongString(from_ascii("EndPostBabelPreamble"));
			break;
		case LA_PREBABELPREAMBLE:
			babel_presettings_ =
				lex.getLongString(from_ascii("EndPreBabelPreamble"));
			break;
		case LA_REQUIRES:
			lex >> required_;
			break;
		case LA_PROVIDES:
			lex >> provides_;
			break;
		case LA_RTL:
			lex >> rightToLeft_;
			break;
		case LA_WORDWRAP:
			lex >> word_wrap_;
			break;
		}
	}
	lex.popTable();
	return finished && !error;
}


bool Language::read(Lexer & lex)
{
	encoding_ = nullptr;
	internal_enc_ = false;
	rightToLeft_ = false;

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
	if (fontenc_.empty())
		fontenc_.push_back("ASCII");
	if (dateformats_.empty()) {
		dateformats_.push_back("MMMM dd, yyyy");
		dateformats_.push_back("MMM dd, yyyy");
		dateformats_.push_back("M/d/yyyy");
	}
	return true;
}


void Language::readLayoutTranslations(Language::TranslationMap const & trans, bool replace)
{
	for (auto const & t : trans) {
		if (replace
		    || layoutTranslations_.find(t.first) == layoutTranslations_.end())
			layoutTranslations_[t.first] = t.second;
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
			LASSERT(latex_language == nullptr, continue);
			static const Language latex_lang = l;
			latex_language = &latex_lang;
		} else if (l.lang() == "ignore") {
			// Check if ignore language was not already defined.
			LASSERT(ignore_language == nullptr, continue);
			static const Language ignore_lang = l;
			ignore_language = &ignore_lang;
		} else
			languagelist_[l.lang()] = l;
	}

	default_language = getLanguage("english");
	if (!default_language) {
		LYXERR0("Default language \"english\" not found!");
		default_language = &(*languagelist_.begin()).second;
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
	VeryApproximateMatch,
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
	if (code.substr(0,2) == langcode.substr(0,2))
		return VeryApproximateMatch;
	return NoMatch;
}

} // namespace



Language const * Languages::getFromCode(string const & code) const
{
	// 1/ exact match with any known language
	for (auto const & l : languagelist_) {
		if (match(code, l.second) == ExactMatch)
			return &l.second;
	}

	// 2/ approximate with any known language
	for (auto const & l : languagelist_) {
		if (match(code, l.second) == ApproximateMatch)
			return &l.second;
	}
	return nullptr;
}


Language const * Languages::getFromCode(string const & code,
			set<Language const *> const & tryfirst) const
{
	// 1/ exact match with tryfirst list
	for (auto const * lptr : tryfirst) {
		if (match(code, *lptr) == ExactMatch)
			return lptr;
	}

	// 2/ approximate match with tryfirst list
	for (auto const * lptr : tryfirst) {
		Match const m = match(code, *lptr);
		if (m == ApproximateMatch || m == VeryApproximateMatch)
			return lptr;
	}

	// 3/ stricter match in all languages
	return getFromCode(code);

	LYXERR0("Unknown language `" << code << "'");
	return nullptr;
}


void Languages::readLayoutTranslations(support::FileName const & filename)
{
	Lexer lex;
	lex.setFile(filename);
	lex.setContext("Languages::read");

	// 1) read all translations (exact and approximate matches) into trans
	std::map<string, Language::TranslationMap> trans;
	while (lex.isOK()) {
		if (!lex.checkFor("Translation")) {
			if (lex.isOK())
				lex.printError("Unknown layout translation tag `$$Token'");
			break;
		}
		if (!lex.next(true))
			break;
		string const code = lex.getString();
		bool found = getFromCode(code);
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
	for (auto & tr : trans) {
		for (auto & lang : languagelist_) {
			Match const m = match(tr.first, lang.second);
			if (m == NoMatch)
				continue;
			lang.second.readLayoutTranslations(tr.second, m == ExactMatch);
		}
	}

}


Language const * Languages::getLanguage(string const & language) const
{
	if (language == "reset")
		return reset_language;
	if (language == "ignore")
		return ignore_language;
	const_iterator it = languagelist_.find(language);
	return it == languagelist_.end() ? reset_language : &it->second;
}


} // namespace lyx
