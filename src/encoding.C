/**
 * \file encoding.C
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

#include "encoding.h"

#include "debug.h"
#include "LaTeXFeatures.h"
#include "lyxlex.h"
#include "lyxrc.h"

#include "support/filename.h"
#include "support/lstrings.h"
#include "support/unicode.h"

#include <sstream>


namespace lyx {

using support::FileName;

#ifndef CXX_GLOBAL_CSTD
using std::strtol;
#endif

using std::endl;
using std::string;


Encodings encodings;

namespace {

char_type arabic_table[50][4] = {
	{0xfe80, 0xfe80, 0xfe80, 0xfe80}, // 0x0621 = hamza
	{0xfe81, 0xfe82, 0xfe81, 0xfe82}, // 0x0622 = ligature madda on alef
	{0xfe83, 0xfe84, 0xfe83, 0xfe84}, // 0x0623 = ligature hamza on alef
	{0xfe85, 0xfe86, 0xfe85, 0xfe86}, // 0x0624 = ligature hamza on waw
	{0xfe87, 0xfe88, 0xfe87, 0xfe88}, // 0x0625 = ligature hamza under alef
	{0xfe89, 0xfe8a, 0xfe8b, 0xfe8c}, // 0x0626 = ligature hamza on ya
	{0xfe8d, 0xfe8e, 0xfe8d, 0xfe8e}, // 0x0627 = alef
	{0xfe8f, 0xfe90, 0xfe91, 0xfe92}, // 0x0628 = baa
	{0xfe93, 0xfe94, 0xfe93, 0xfe94}, // 0x0629 = taa marbuta
	{0xfe95, 0xfe96, 0xfe97, 0xfe98}, // 0x062a = taa
	{0xfe99, 0xfe9a, 0xfe9b, 0xfe9c}, // 0x062b = thaa
	{0xfe9d, 0xfe9e, 0xfe9f, 0xfea0}, // 0x062c = jeem
	{0xfea1, 0xfea2, 0xfea3, 0xfea4}, // 0x062d = haa
	{0xfea5, 0xfea6, 0xfea7, 0xfea8}, // 0x062e = khaa
	{0xfea9, 0xfeaa, 0xfea9, 0xfeaa}, // 0x062f = dal

	{0xfeab, 0xfeac, 0xfeab, 0xfeac}, // 0x0630 = thal
	{0xfead, 0xfeae, 0xfead, 0xfeae}, // 0x0631 = ra
	{0xfeaf, 0xfeb0, 0xfeaf, 0xfeb0}, // 0x0632 = zain
	{0xfeb1, 0xfeb2, 0xfeb3, 0xfeb4}, // 0x0633 = seen
	{0xfeb5, 0xfeb6, 0xfeb7, 0xfeb8}, // 0x0634 = sheen
	{0xfeb9, 0xfeba, 0xfebb, 0xfebc}, // 0x0635 = sad
	{0xfebd, 0xfebe, 0xfebf, 0xfec0}, // 0x0636 = dad
	{0xfec1, 0xfec2, 0xfec3, 0xfec4}, // 0x0637 = tah
	{0xfec5, 0xfec6, 0xfec7, 0xfec8}, // 0x0638 = zah
	{0xfec9, 0xfeca, 0xfecb, 0xfecc}, // 0x0639 = ain
	{0xfecd, 0xfece, 0xfecf, 0xfed0}, // 0x063a = ghain
	{0, 0, 0, 0}, // 0x063b
	{0, 0, 0, 0}, // 0x063c
	{0, 0, 0, 0}, // 0x063d
	{0, 0, 0, 0}, // 0x063e
	{0, 0, 0, 0}, // 0x063f

	{0, 0, 0, 0}, // 0x0640
	{0xfed1, 0xfed2, 0xfed3, 0xfed4}, // 0x0641 = fa
	{0xfed5, 0xfed6, 0xfed7, 0xfed8}, // 0x0642 = qaf
	{0xfed9, 0xfeda, 0xfedb, 0xfedc}, // 0x0643 = kaf
	{0xfedd, 0xfede, 0xfedf, 0xfee0}, // 0x0644 = lam
	{0xfee1, 0xfee2, 0xfee3, 0xfee4}, // 0x0645 = meem
	{0xfee5, 0xfee6, 0xfee7, 0xfee8}, // 0x0646 = noon
	{0xfee9, 0xfeea, 0xfeeb, 0xfeec}, // 0x0647 = ha
	{0xfeed, 0xfeee, 0xfeed, 0xfeee}, // 0x0648 = waw
	{0xfeef, 0xfef0, 0xfeef, 0xfef0}, // 0x0649 = alef maksura
	{0xfef1, 0xfef2, 0xfef3, 0xfef4}, // 0x064a = ya
	{0x065b, 0x065b, 0x065b, 0x065b}, // 0x064b = fathatan
	{0x065c, 0x065c, 0x065c, 0x065c}, // 0x064c = dammatan
	{0x064d, 0x064d, 0x064d, 0x064d}, // 0x064d = kasratan
	{0x064e, 0x064e, 0x064e, 0x064e}, // 0x064e = fatha
	{0x064f, 0x064f, 0x064f, 0x064f}, // 0x064f = damma

	{0x0650, 0x0650, 0x0650, 0x0650}, // 0x0650 = kasra
	{0x0651, 0x0651, 0x0651, 0x0651}, // 0x0651 = shadda
	{0x0652, 0x0652, 0x0652, 0x0652}, // 0x0652 = sukun
};


char_type const arabic_start = 0x0621;
char_type const arabic_end = 0x0652;


/// Information about a single UCS4 character
struct CharInfo {
	/// LaTeX command for this character
	docstring command;
	/// Needed LaTeX preamble (or feature)
	string preamble;
	/// Is this a combining character?
	bool combining;
	/// Is \c preamble a feature known by LaTeXFeatures, or a raw LaTeX
	/// command?
	bool feature;
	/// Always force the LaTeX command, even if the encoding contains
	/// this character?
	bool force;
};


typedef std::map<char_type, CharInfo> CharInfoMap;
CharInfoMap unicodesymbols;

} // namespace anon


Encoding::Encoding(string const & n, string const & l, string const & i)
	: Name_(n), LatexName_(l), iconvName_(i)
{
	if (n == "ascii")
		// ASCII can encode 128 code points and nothing else
		start_encodable_ = 128;
	else if (i == "UTF-8")
		// UTF8 can encode all 1<<20 + 1<<16 UCS4 code points
		start_encodable_ = 0x110000;
	else {
		start_encodable_ = 0;
		// temporarily switch off lyxerr, since we will generate iconv errors
		lyxerr.disable();
		for (unsigned short j = 0; j < 256; ++j) {
			char const c = j;
			std::vector<char_type> const ucs4 = eightbit_to_ucs4(&c, 1, i);
			if (ucs4.size() == 1) {
				char_type const c = ucs4[0];
				CharInfoMap::const_iterator const it = unicodesymbols.find(c);
				if (it == unicodesymbols.end() || !it->second.force)
					encodable_.insert(c);
			}
		}
		lyxerr.enable();
		CharSet::iterator it = encodable_.find(start_encodable_);
		while (it != encodable_.end()) {
			encodable_.erase(it);
			++start_encodable_;
			it = encodable_.find(start_encodable_);
		}
	}
}


docstring const Encoding::latexChar(char_type c) const
{
	if (c < start_encodable_)
		return docstring(1, c);
	if (encodable_.find(c) == encodable_.end()) {
		// c cannot be encoded in this encoding
		CharInfoMap::const_iterator const it = unicodesymbols.find(c);
		if (it == unicodesymbols.end())
			lyxerr << "Could not find LaTeX command for character 0x"
			       << std::hex << c << ".\nLaTeX export will fail."
			       << endl;
		else
			return it->second.command;
	}
	return docstring(1, c);
}


void Encoding::validate(char_type c, LaTeXFeatures & features) const
{
	// Add the preamble stuff even if c can be encoded in this encoding,
	// since the inputenc package only maps the code point c to a command,
	// it does not make this command available.
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	if (it != unicodesymbols.end() && !it->second.preamble.empty()) {
		if (it->second.feature)
			features.require(it->second.preamble);
		else
			features.addPreambleSnippet(it->second.preamble);
	}
}


bool Encodings::isComposeChar_hebrew(char_type c)
{
	return c <= 0xd2 && c >= 0xc0 &&
		c != 0xce && c != 0xd0;
}


// Special Arabic letters are ones that do not get connected from left
// they are hamza, alef_madda, alef_hamza, waw_hamza, alef_hamza_under,
// alef, tah_marbota, dal, thal, rah, zai, wow, alef_maksoura

bool Encodings::is_arabic_special(char_type c)
{
	return (c >= 0x0621 && c <= 0x0625) ||
	        c == 0x0627 || c == 0x0629  ||
	        c == 0x062f || c == 0x0648  ||
	       (c >= 0x0630 && c <= 0x0632) ||
	        c == 0x0649;
}


bool Encodings::isComposeChar_arabic(char_type c)
{
	return c >= 0x064b && c <= 0x0652;
}


bool Encodings::is_arabic(char_type c)
{
	return c >= arabic_start && c <= arabic_end &&
	       arabic_table[c-arabic_start][0];
}


char_type Encodings::transformChar(char_type c,
				      Encodings::Letter_Form form)
{
	if (!is_arabic(c))
		return c;

	return arabic_table[c-arabic_start][form];
}


bool Encodings::isCombiningChar(char_type c)
{
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	if (it != unicodesymbols.end())
		return it->second.combining;
	return false;
}


Encoding const * Encodings::getFromLyXName(string const & name) const
{
	EncodingList::const_iterator it = encodinglist.find(name);
	if (it != encodinglist.end())
		return &it->second;
	else
		return 0;
}


Encoding const * Encodings::getFromLaTeXName(string const & name) const
{
	// We don't use std::find_if because it makes copies of the pairs in
	// the map.
	// This linear search is OK since we don't have many encodings.
	// Users could even optimize it by putting the encodings they use
	// most at the top of lib/encodings.
	EncodingList::const_iterator const end = encodinglist.end();
	for (EncodingList::const_iterator it = encodinglist.begin(); it != end; ++it)
		if (it->second.latexName() == name)
			return &it->second;
	return 0;
}


Encodings::Encodings()
{
}


void Encodings::read(FileName const & encfile, FileName const & symbolsfile)
{
	// We must read the symbolsfile first, because the Encoding
	// constructor depends on it.
	LyXLex symbolslex(0, 0);
	symbolslex.setFile(symbolsfile);
	while (symbolslex.isOK()) {
		char_type symbol;
		CharInfo info;
		string flags;

		if (symbolslex.next(true)) {
			std::istringstream is(symbolslex.getString());
			// reading symbol directly does not work if
			// char_type == std::wchar_t.
			boost::uint32_t tmp;
			if(!(is >> std::hex >> tmp))
				break;
			symbol = tmp;
		} else
			break;
		if (symbolslex.next(true))
			info.command = symbolslex.getDocString();
		else
			break;
		if (symbolslex.next(true))
			info.preamble = symbolslex.getString();
		else
			break;
		if (symbolslex.next(true))
			flags = symbolslex.getString();
		else
			break;

		info.combining = false;
		info.feature = false;
		info.force = false;
		while (!flags.empty()) {
			string flag;
			flags = support::split(flags, flag, ',');
			if (flag == "combining")
				info.combining = true;
			else if (flag == "force")
				info.force = true;
			else
				lyxerr << "Ignoring unknown flag `" << flag
				       << "' for symbol `0x" << std::hex
				       << symbol << "'." << endl;
		}

		if (!info.preamble.empty())
			info.feature = info.preamble[0] != '\\';

		lyxerr[Debug::INFO]
			<< "Read unicode symbol " << symbol << " '"
			<< to_utf8(info.command) << "' '" << info.preamble
			<< "' " << info.combining << ' ' << info.feature
			<< endl;
		unicodesymbols[symbol] = info;
	}

	// Now read the encodings
	enum Encodingtags {
		et_encoding = 1,
		et_end,
		et_last
	};

	struct keyword_item encodingtags[et_last - 1] = {
		{ "encoding", et_encoding },
		{ "end", et_end }
	};

	LyXLex lex(encodingtags, et_last - 1);
	lex.setFile(encfile);
	while (lex.isOK()) {
		switch (lex.lex()) {
		case et_encoding:
		{
			lex.next();
			string const name = lex.getString();
			lex.next();
			string const latexname = lex.getString();
			lex.next();
			string const iconvname = lex.getString();
			lyxerr[Debug::INFO] << "Reading encoding " << name << endl;
			encodinglist[name] = Encoding(name, latexname, iconvname);
			if (lex.lex() != et_end)
				lex.printError("Encodings::read: "
					       "missing end");
			break;
		}
		case et_end:
			lex.printError("Encodings::read: Misplaced end");
			break;
		case LyXLex::LEX_FEOF:
			break;
		default:
			lex.printError("Encodings::read: "
				       "Unknown tag: `$$Token'");
			break;
		}
	}
}


} // namespace lyx
