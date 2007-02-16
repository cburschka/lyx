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

char_type arabic_table2[63][4] = {
	{0x41, 0x41, 0x41, 0x41}, // 0xc1 = hamza
	{0x42, 0xa1, 0x42, 0xa1}, // 0xc2 = ligature madda on alef
	{0x43, 0xa2, 0x43, 0xa2}, // 0xc3 = ligature hamza on alef
	{0x44, 0xa3, 0x44, 0xa3}, // 0xc4 = ligature hamza on waw
	{0x45, 0xa4, 0x45, 0xa4}, // 0xc5 = ligature hamza under alef
	{0x46, 0xf9, 0xf8, 0xa0}, // 0xc6 = ligature hamza on ya
	{0x47, 0xa5, 0x47, 0xa5}, // 0xc7 = alef
	{0x48, 0xae, 0xac, 0xad}, // 0xc8 = baa
	{0x49, 0xb1, 0x49, 0xb1}, // 0xc9 = taa marbuta
	{0x4a, 0xb4, 0xb2, 0xb3}, // 0xca = taa
	{0x4b, 0xb7, 0xb5, 0xb6}, // 0xcb = thaa
	{0x4c, 0xba, 0xb8, 0xb9}, // 0xcc = jeem
	{0x4d, 0xbd, 0xbb, 0xbc}, // 0xcd = haa
	{0x4e, 0xc0, 0xbe, 0xbf}, // 0xce = khaa
	{0x4f, 0xa6, 0x4f, 0xa6}, // 0xcf = dal

	{0x50, 0xa7, 0x50, 0xa7}, // 0xd0 = thal
	{0x51, 0xa8, 0x51, 0xa8}, // 0xd1 = ra
	{0x52, 0xa9, 0x52, 0xa9}, // 0xd2 = zain
	{0x53, 0xc3, 0xc1, 0xc2}, // 0xd3 = seen
	{0x54, 0xc6, 0xc4, 0xc5}, // 0xd4 = sheen
	{0x55, 0xc9, 0xc7, 0xc8}, // 0xd5 = sad
	{0x56, 0xcc, 0xca, 0xcb}, // 0xd6 = dad
	{0x57, 0xcf, 0xcd, 0xce}, // 0xd7 = tah
	{0x58, 0xd2, 0xd0, 0xd1}, // 0xd8 = zah
	{0x59, 0xd5, 0xd3, 0xd4}, // 0xd9 = ain
	{0x5a, 0xd8, 0xd6, 0xd7}, // 0xda = ghain
	{0,0,0,0}, // 0xdb
	{0,0,0,0}, // 0xdc
	{0,0,0,0}, // 0xdd
	{0,0,0,0}, // 0xde
	{0,0,0,0}, // 0xdf

	{0,0,0,0}, // 0xe0
	{0x61, 0xdb, 0xd9, 0xda}, // 0xe1 = fa
	{0x62, 0xde, 0xdc, 0xdd}, // 0xe2 = qaf
	{0x63, 0xe1, 0xdf, 0xe0}, // 0xe3 = kaf
	{0x64, 0xe4, 0xe2, 0xe3}, // 0xe4 = lam
	{0x65, 0xe7, 0xe5, 0xe6}, // 0xe5 = meem
	{0x66, 0xea, 0xe8, 0xe9}, // 0xe6 = noon
	{0x67, 0xed, 0xeb, 0xec}, // 0xe7 = ha
	{0x68, 0xaa, 0x68, 0xaa}, // 0xe8 = waw
	{0x69, 0xab, 0x69, 0xab}, // 0xe9 = alef maksura
	{0x6a, 0xf0, 0xee, 0xef}, // 0xea = ya
	{0x6b, 0x6b, 0x6b, 0x6b}, // 0xeb = fathatan
	{0x6c, 0x6c, 0x6c, 0x6c}, // 0xec = dammatan
	{0x6d, 0x6d, 0x6d, 0x6d}, // 0xed = kasratan
	{0x6e, 0x6e, 0x6e, 0x6e}, // 0xee = fatha
	{0x6f, 0x6f, 0x6f, 0x6f}, // 0xef = damma

	{0x70, 0x70, 0x70, 0x70}, // 0xf0 = kasra
	{0x71, 0x71, 0x71, 0x71}, // 0xf1 = shadda
	{0x72, 0x72, 0x72, 0x72}, // 0xf2 = sukun
	{0,0,0,0}, // 0xf3
	{0,0,0,0}, // 0xf4
	{0,0,0,0}, // 0xf5
	{0,0,0,0}, // 0xf6
	{0,0,0,0}, // 0xf7
	{0,0,0,0}, // 0xf8
	{0,0,0,0}, // 0xf9
	{0,0,0,0}, // 0xfa
	{0,0,0,0}, // 0xfb
	{0,0,0,0}, // 0xfc
	{0,0,0,0}, // 0xfd
	{0,0,0,0}, // 0xfe
	{0,0,0,0}, // 0xff
};


char_type arabic_table[63][2] = {
	{0xc1, 0xc1}, // 0xc1 = hamza
	{0xc2, 0xc2}, // 0xc2 = ligature madda on alef
	{0xc3, 0xc3}, // 0xc3 = ligature hamza on alef
	{0xc4, 0xc4}, // 0xc4 = ligature hamza on waw
	{0xc5, 0xc5}, // 0xc5 = ligature hamza under alef
	{0xc6, 0xc0}, // 0xc6 = ligature hamza on ya
	{0xc7, 0xc7}, // 0xc7 = alef
	{0xc8, 0xeb}, // 0xc8 = baa
	{0xc9, 0xc9}, // 0xc9 = taa marbuta
	{0xca, 0xec}, // 0xca = taa
	{0xcb, 0xed}, // 0xcb = thaa
	{0xcc, 0xee}, // 0xcc = jeem
	{0xcd, 0xef}, // 0xcd = haa
	{0xce, 0xf0}, // 0xce = khaa
	{0xcf, 0xcf}, // 0xcf = dal

	{0xd0, 0xd0}, // 0xd0 = thal
	{0xd1, 0xd1}, // 0xd1 = ra
	{0xd2, 0xd2}, // 0xd2 = zain
	{0xd3, 0xf1}, // 0xd3 = seen
	{0xd4, 0xf2}, // 0xd4 = sheen
	{0xd5, 0xf3}, // 0xd5 = sad
	{0xd6, 0xf4}, // 0xd6 = dad
	{0xd7, 0xd7}, // 0xd7 = tah
	{0xd8, 0xd8}, // 0xd8 = zah
	{0xd9, 0xf5}, // 0xd9 = ain
	{0xda, 0xf6}, // 0xda = ghain
	{0,0}, // 0xdb
	{0,0}, // 0xdc
	{0,0}, // 0xdd
	{0,0}, // 0xde
	{0,0}, // 0xdf

	{0,0},	// 0xe0
	{0xe1, 0xf7},	// 0xe1 = fa
	{0xe2, 0xf8},	// 0xe2 = qaf
	{0xe3, 0xf9},	// 0xe3 = kaf
	{0xe4, 0xfa},	// 0xe4 = lam
	{0xe5, 0xfb},	// 0xe5 = meem
	{0xe6, 0xfc},	// 0xe6 = noon
	{0xe7, 0xfd},	// 0xe7 = ha
	{0xe8, 0xe8},	// 0xe8 = waw
	{0xe9, 0xe9},	// 0xe9 = alef maksura
	{0xea, 0xfe},	// 0xea = ya
	{0xa8, 0xa8},	// 0xeb = fathatan
	{0xa9, 0xa9},	// 0xec = dammatan
	{0xaa, 0xaa},	// 0xed = kasratan
	{0xab, 0xab},	// 0xee = fatha
	{0xac, 0xac},	// 0xef = damma

	{0xad, 0xad},	// 0xf0 = kasra
	{0xae, 0xae},	// 0xf1 = shadda
	{0xaf, 0xaf},	// 0xf2 = sukun
	{0,0}, // 0xf3
	{0,0}, // 0xf4
	{0,0}, // 0xf5
	{0,0}, // 0xf6
	{0,0}, // 0xf7
	{0,0}, // 0xf8
	{0,0}, // 0xf9
	{0,0}, // 0xfa
	{0,0}, // 0xfb
	{0,0}, // 0xfc
	{0,0}, // 0xfd
	{0,0}, // 0xfe
	{0,0} // 0xff
};


char_type const arabic_start = 0xc1;


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
	if (n == "utf8" || n == "utf8x")
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
	if (c < start_encodable_)
		return;

	if (encodable_.find(c) != encodable_.end())
		return;

	// c cannot be encoded in this encoding
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
	return	(c >= 0xc1 && c <= 0xc5) ||
		 c == 0xc7 || c  == 0xc9  ||
		 c == 0xcf || c  == 0xe8  ||
		(c >= 0xd0 && c <= 0xd2) ||
		 c == 0xe9;
}

bool Encodings::isComposeChar_arabic(char_type c)
{
	return c >= 0xeb && c <= 0xf2;
}


bool Encodings::is_arabic(char_type c)
{
	return c >= arabic_start && arabic_table[c-arabic_start][0];
}


char_type Encodings::transformChar(char_type c,
				      Encodings::Letter_Form form)
{
	if (!is_arabic(c))
		return c;

	if (lyxrc.font_norm_type == LyXRC::ISO_10646_1)
		return arabic_table2[c-arabic_start][form];
	else
		return arabic_table[c-arabic_start][form >> 1];
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
