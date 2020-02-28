// -*- C++ -*-
/**
 * \file Language.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "BufferParams.h"

#include "support/docstring.h"
#include "support/trivstring.h"

#include <map>
#include <set>
#include <vector>


namespace lyx {

namespace support { class FileName; }

class Encoding;
class Lexer;

///
class Language {
public:
	///
	Language() : rightToLeft_(false), encoding_(0), internal_enc_(false),
				 has_gui_support_(false), word_wrap_(true) {}
	/// LyX language name
	std::string const lang() const { return lang_; }
	/// Babel language name
	std::string const babel() const { return babel_; }
	/// polyglossia language name
	std::string const polyglossia() const { return polyglossia_name_; }
	/// polyglossia language options
	std::string const polyglossiaOpts() const { return polyglossia_opts_; }
	/// polyglossia language options
	std::string const xindy() const { return xindy_; }
	/// Is this language only supported by polyglossia?
	bool isPolyglossiaExclusive() const;
	/// Is this language only supported by babel?
	bool isBabelExclusive() const;
	/// quotation marks style
	std::string const quoteStyle() const { return quote_style_; }
	/// active characters
	std::string const activeChars() const { return active_chars_; }
	/// requirement (package, function)
	std::string const required() const { return requires_; }
	/// provides feature
	std::string const provides() const { return provides_; }
	/// translatable GUI name
	std::string const display() const { return display_; }
	/// is this a RTL language?
	bool rightToLeft() const { return rightToLeft_; }
	/// shall text be wrapped at word boundary ?
	bool wordWrap() const { return word_wrap_; }
	/**
	 * Translate a string from the layout files that appears in the output.
	 * It takes the translations from lib/layouttranslations instead of
	 * the .mo files. This should be used for every translation that
	 * appears in the exported document, since the output must not depend
	 * on installed locales. Non-ASCII keys are not translated. */
	docstring const translateLayout(std::string const & msg) const;
	/// default encoding
	Encoding const * encoding() const { return encoding_; }
	///
	std::string const encodingStr() const { return encodingStr_; }
	/// language code
	std::string const code() const { return code_; }
	/// set code (needed for rc.spellchecker_alt_lang)
	void setCode(std::string const & c) { code_ = c; }
	/// language variety (needed by aspell checker)
	std::string const variety() const { return variety_; }
	/// set variety (needed for rc.spellchecker_alt_lang)
	void setVariety(std::string const & v) { variety_ = v; }
	/// preamble settings after babel was called
	docstring babel_postsettings() const { return babel_postsettings_; }
	/// preamble settings before babel is called
	docstring babel_presettings() const { return babel_presettings_; }
	/// This language internally sets a font encoding
	bool internalFontEncoding() const { return internal_enc_; }
	/// The most suitable font encoding(s) for the selected document font
	std::string fontenc(BufferParams const &) const;
	/// Return the localized date formats (long, medium, short format)
	std::string dateFormat(size_t i) const;
	/// Return the localized decimal separator
	docstring decimalSeparator() const;
	/// This language corresponds to a translation of the GUI
	bool hasGuiSupport() const { return has_gui_support_; }
	///
	bool read(Lexer & lex);
	///
	bool readLanguage(Lexer & lex);
	///
	typedef std::map<trivstring, trivdocstring> TranslationMap;
	///
	void readLayoutTranslations(TranslationMap const & trans, bool replace);
	// for the use in std::map
	friend bool operator<(Language const & p, Language const & q);
private:
	///
	trivstring lang_;
	///
	trivstring babel_;
	///
	trivstring polyglossia_name_;
	///
	trivstring polyglossia_opts_;
	///
	trivstring xindy_;
	///
	trivstring quote_style_;
	///
	trivstring active_chars_;
	///
	trivstring requires_;
	///
	trivstring provides_;
	///
	trivstring display_;
	///
	bool rightToLeft_;
	///
	trivstring encodingStr_;
	///
	Encoding const * encoding_;
	///
	trivstring code_;
	///
	trivstring variety_;
	///
	trivdocstring babel_postsettings_;
	///
	trivdocstring babel_presettings_;
	///
	std::vector<std::string> fontenc_;
	///
	std::vector<std::string> dateformats_;
	///
	bool internal_enc_;
	///
	bool has_gui_support_;
	///
	bool word_wrap_;
	///
	TranslationMap layoutTranslations_;
};


inline bool operator<(Language const & p, Language const & q)
{
	return q.lang() > p.lang();
}


class Languages
{
public:
	///
	typedef std::map<trivstring, Language> LanguageList;
	///
	typedef LanguageList::const_iterator const_iterator;
	///
	typedef LanguageList::size_type size_type;
	///
	void read(support::FileName const & filename);
	///
	Language const * getFromCode(std::string const & code) const;
	///
	Language const * getFromCode(std::string const & code,
			std::set<Language const *> const & tryfirst) const;
	///
	void readLayoutTranslations(support::FileName const & filename);
	///
	Language const * getLanguage(std::string const & language) const;
	///
	size_type size() const { return languagelist_.size(); }
	///
	const_iterator begin() const { return languagelist_.begin(); }
	///
	const_iterator end() const { return languagelist_.end(); }

private:
	///
	LanguageList languagelist_;
};

/// Global singleton instance.
extern Languages languages;
/// Default language defined in LyXRC
extern Language const * default_language;
/// Used to indicate that the language should be left unchanged when
/// applying a font change.
extern Language const * ignore_language;
/// Default language defined in LyXRC
extern Language const * latex_language;
/// Used to indicate that the language should be reset to the Buffer
// language when applying a font change.
extern Language const * reset_language;


} // namespace lyx

#endif
