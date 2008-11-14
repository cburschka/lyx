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

#include <map>
#include <string>


namespace lyx {

namespace support { class FileName; }

class Encoding;
class Lexer;

///
class Language {
public:
	///
	Language() : rightToLeft_(false) {}
	///
	std::string const & lang() const { return lang_; }
	///
	std::string const & babel() const { return babel_; }
	///
	std::string const & display() const { return display_; }
	///
	bool rightToLeft() const { return rightToLeft_; }
	///
	Encoding const * encoding() const { return encoding_; }
	///
	std::string const & encodingStr() const { return encodingStr_; }
	///
	std::string const & code() const { return code_; }
	///
	std::string const & latex_options() const { return latex_options_; }
	///
	bool read(Lexer & lex);
private:
	///
	std::string lang_;
	///
	std::string babel_;
	///
	std::string display_;
	///
	bool rightToLeft_;
	///
	std::string encodingStr_;
	///
	Encoding const * encoding_;
	///
	std::string code_;
	///
	std::string latex_options_;
};

class Languages
{
public:
	///
	typedef std::map<std::string, Language> LanguageList;
	///
	typedef LanguageList::const_iterator const_iterator;
	///
	typedef LanguageList::size_type size_type;
	///
	void read(support::FileName const & filename);
	///
	Language const * getLanguage(std::string const & language) const;
	///
	size_type size() const { return languagelist.size(); }
	///
	const_iterator begin() const { return languagelist.begin(); }
	///
	const_iterator end() const { return languagelist.end(); }
	///

private:
	///
	LanguageList languagelist;
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
