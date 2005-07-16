// -*- C++ -*-
/**
 * \file language.h
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


class Encoding;

///
class Language {
public:
	///
	Language() : rightToLeft_(false) {}
	///
	Language(std::string const & l, std::string const & b, std::string const & d,
		 bool rtl, std::string const & es, Encoding const * e, std::string const & c,
		 std::string const & o)
		: lang_(l), babel_(b), display_(d), rightToLeft_(rtl),
		  encodingStr_(es), encoding_(e), code_(c), latex_options_(o)
		{}
	///
	std::string const & lang() const { return lang_; }
	///
	std::string const & babel() const { return babel_; }
	///
	std::string const & display() const { return display_; }
	///
	bool RightToLeft() const { return rightToLeft_; }
	///
	Encoding const * encoding() const { return encoding_; }
	///
	std::string const & encodingStr() const { return encodingStr_; }
	///
	std::string const & code() const { return code_; }
	///
	std::string const & latex_options() const { return latex_options_; }
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
	void read(std::string const & filename);
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

extern Languages languages;
extern Language const * default_language;
extern Language const * english_language;
extern Language const * ignore_language;
extern Language const * latex_language;

#endif
