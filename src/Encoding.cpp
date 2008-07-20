/**
 * \file Encoding.cpp
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

#include "Encoding.h"

#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "LyXRC.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/lstrings.h"
#include "support/unicode.h"

#include <boost/cstdint.hpp>

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

Encodings encodings;

namespace {

char_type arabic_table[172][4] = {
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

	{0, 0, 0, 0}, // 0x0653
	{0, 0, 0, 0}, // 0x0654
	{0, 0, 0, 0}, // 0x0655
	{0, 0, 0, 0}, // 0x0656
	{0, 0, 0, 0}, // 0x0657
	{0, 0, 0, 0}, // 0x0658
	{0, 0, 0, 0}, // 0x0659
	{0, 0, 0, 0}, // 0x065a
	{0, 0, 0, 0}, // 0x065b
	{0, 0, 0, 0}, // 0x065c
	{0, 0, 0, 0}, // 0x065d
	{0, 0, 0, 0}, // 0x065e
	{0, 0, 0, 0}, // 0x065f
	{0, 0, 0, 0}, // 0x0660
	{0, 0, 0, 0}, // 0x0661
	{0, 0, 0, 0}, // 0x0662
	{0, 0, 0, 0}, // 0x0663
	{0, 0, 0, 0}, // 0x0664
	{0, 0, 0, 0}, // 0x0665
	{0, 0, 0, 0}, // 0x0666
	{0, 0, 0, 0}, // 0x0667
	{0, 0, 0, 0}, // 0x0668
	{0, 0, 0, 0}, // 0x0669
	{0, 0, 0, 0}, // 0x066a
	{0, 0, 0, 0}, // 0x066b
	{0, 0, 0, 0}, // 0x066c
	{0, 0, 0, 0}, // 0x066d
	{0, 0, 0, 0}, // 0x066e
	{0, 0, 0, 0}, // 0x066f
	{0, 0, 0, 0}, // 0x0670
	{0, 0, 0, 0}, // 0x0671
	{0, 0, 0, 0}, // 0x0672
	{0, 0, 0, 0}, // 0x0673
	{0, 0, 0, 0}, // 0x0674
	{0, 0, 0, 0}, // 0x0675
	{0, 0, 0, 0}, // 0x0676
	{0, 0, 0, 0}, // 0x0677
	{0, 0, 0, 0}, // 0x0678
	{0, 0, 0, 0}, // 0x0679
	{0, 0, 0, 0}, // 0x067a
	{0, 0, 0, 0}, // 0x067b
	{0, 0, 0, 0}, // 0x067c
	{0, 0, 0, 0}, // 0x067d
	{0xfb56, 0xfb57, 0xfb58, 0xfb59}, // 0x067e = peh 
	{0, 0, 0, 0}, // 0x067f
	{0, 0, 0, 0}, // 0x0680
	{0, 0, 0, 0}, // 0x0681
	{0, 0, 0, 0}, // 0x0682
	{0, 0, 0, 0}, // 0x0683
	{0, 0, 0, 0}, // 0x0684
	{0, 0, 0, 0}, // 0x0685
	{0xfb7a, 0xfb7b, 0xfb7c, 0xfb7d}, // 0x0686 = tcheh 
	{0, 0, 0, 0}, // 0x0687
	{0, 0, 0, 0}, // 0x0688
	{0, 0, 0, 0}, // 0x0689
	{0, 0, 0, 0}, // 0x068a
	{0, 0, 0, 0}, // 0x068b
	{0, 0, 0, 0}, // 0x068c
	{0, 0, 0, 0}, // 0x068d
	{0, 0, 0, 0}, // 0x068e
	{0, 0, 0, 0}, // 0x068f
	{0, 0, 0, 0}, // 0x0690
	{0, 0, 0, 0}, // 0x0691
	{0, 0, 0, 0}, // 0x0692
	{0, 0, 0, 0}, // 0x0693
	{0, 0, 0, 0}, // 0x0694
	{0, 0, 0, 0}, // 0x0695
	{0, 0, 0, 0}, // 0x0696
	{0, 0, 0, 0}, // 0x0697
	{0xfb8a, 0xfb8b, 0xfb8a, 0xfb8b}, // 0x0698 = jeh
	{0, 0, 0, 0}, // 0x0699
	{0, 0, 0, 0}, // 0x069a
	{0, 0, 0, 0}, // 0x069b
	{0, 0, 0, 0}, // 0x069c
	{0, 0, 0, 0}, // 0x069d
	{0, 0, 0, 0}, // 0x069e
	{0, 0, 0, 0}, // 0x069f
	{0, 0, 0, 0}, // 0x06a0
	{0, 0, 0, 0}, // 0x06a1
	{0, 0, 0, 0}, // 0x06a2
	{0, 0, 0, 0}, // 0x06a3
	{0, 0, 0, 0}, // 0x06a4
	{0, 0, 0, 0}, // 0x06a5
	{0, 0, 0, 0}, // 0x06a6
	{0, 0, 0, 0}, // 0x06a7
	{0, 0, 0, 0}, // 0x06a8
	{0xfb8e, 0xfb8f, 0xfb90, 0xfb91}, // 0x06a9 = farsi kaf 
	{0, 0, 0, 0}, // 0x06aa
	{0, 0, 0, 0}, // 0x06ab
	{0, 0, 0, 0}, // 0x06ac
	{0, 0, 0, 0}, // 0x06ad
	{0, 0, 0, 0}, // 0x06ae
	{0xfb92, 0xfb93, 0xfb94, 0xfb95}, // 0x06af = gaf 
	{0, 0, 0, 0}, // 0x06b0
	{0, 0, 0, 0}, // 0x06b1
	{0, 0, 0, 0}, // 0x06b2
	{0, 0, 0, 0}, // 0x06b3
	{0, 0, 0, 0}, // 0x06b4
	{0, 0, 0, 0}, // 0x06b5
	{0, 0, 0, 0}, // 0x06b6
	{0, 0, 0, 0}, // 0x06b7
	{0, 0, 0, 0}, // 0x06b8
	{0, 0, 0, 0}, // 0x06b9
	{0, 0, 0, 0}, // 0x06ba
	{0, 0, 0, 0}, // 0x06bb
	{0, 0, 0, 0}, // 0x06bc
	{0, 0, 0, 0}, // 0x06bd
	{0, 0, 0, 0}, // 0x06be
	{0, 0, 0, 0}, // 0x06bf
	{0, 0, 0, 0}, // 0x06c0
	{0, 0, 0, 0}, // 0x06c1
	{0, 0, 0, 0}, // 0x06c2
	{0, 0, 0, 0}, // 0x06c3
	{0, 0, 0, 0}, // 0x06c4
	{0, 0, 0, 0}, // 0x06c5
	{0, 0, 0, 0}, // 0x06c6
	{0, 0, 0, 0}, // 0x06c7
	{0, 0, 0, 0}, // 0x06c8
	{0, 0, 0, 0}, // 0x06c9
	{0, 0, 0, 0}, // 0x06ca
	{0, 0, 0, 0}, // 0x06cb
	{0xfbfc, 0xfbfd, 0xfbfe, 0xfbff} // 0x06cc = farsi yeh	
};


char_type const arabic_start = 0x0621;
char_type const arabic_end = 0x06cc;


/// Information about a single UCS4 character
struct CharInfo {
	/// LaTeX command (text mode) for this character
	docstring textcommand;
	/// LaTeX command (math mode) for this character
	docstring mathcommand;
	/// Needed LaTeX preamble (or feature) for text mode
	string textpreamble;
	/// Needed LaTeX preamble (or feature) for math mode
	string mathpreamble;
	/// Is this a combining character?
	bool combining;
	/// Is \c textpreamble a feature known by LaTeXFeatures, or a raw LaTeX
	/// command?
	bool textfeature;
	/// Is \c mathpreamble a feature known by LaTeXFeatures, or a raw LaTeX
	/// command?
	bool mathfeature;
	/// Always force the LaTeX command, even if the encoding contains
	/// this character?
	bool force;
};


typedef map<char_type, CharInfo> CharInfoMap;
CharInfoMap unicodesymbols;

typedef std::set<char_type> CharSet;
CharSet forced;


/// The highest code point in UCS4 encoding (1<<20 + 1<<16)
char_type const max_ucs4 = 0x110000;

} // namespace anon


EncodingException::EncodingException(char_type c)
	: failed_char(c), par_id(0), pos(0)
{
}


const char * EncodingException::what() const throw()
{
	return "Could not find LaTeX command for a character";
}


Encoding::Encoding(string const & n, string const & l, string const & g,
		   string const & i, bool f, Encoding::Package p)
	: name_(n), latexName_(l), guiName_(g), iconvName_(i), fixedwidth_(f), package_(p)
{
	if (n == "ascii") {
		// ASCII can encode 128 code points and nothing else
		start_encodable_ = 128;
		complete_ = true;
	} else if (i == "UTF-8") {
		// UTF8 can encode all UCS4 code points
		start_encodable_ = max_ucs4;
		complete_ = true;
	} else {
		complete_ = false;
	}
}


void Encoding::init() const
{
	if (complete_)
		return;

	start_encodable_ = 0;
	// temporarily switch off lyxerr, since we will generate iconv errors
	lyxerr.disable();
	if (fixedwidth_) {
		// We do not need to check all UCS4 code points, it is enough
		// if we check all 256 code points of this encoding.
		for (unsigned short j = 0; j < 256; ++j) {
			char const c = char(j);
			vector<char_type> const ucs4 = eightbit_to_ucs4(&c, 1, iconvName_);
			if (ucs4.size() != 1)
				continue;
			char_type const uc = ucs4[0];
			CharInfoMap::const_iterator const it = unicodesymbols.find(uc);
			if (it == unicodesymbols.end() || !it->second.force)
				encodable_.insert(uc);
		}
	} else {
		// We do not know how many code points this encoding has, and
		// they do not have a direct representation as a single byte,
		// therefore we need to check all UCS4 code points.
		// This is expensive!
		for (char_type c = 0; c < max_ucs4; ++c) {
			vector<char> const eightbit = ucs4_to_eightbit(&c, 1, iconvName_);
			if (!eightbit.empty()) {
				CharInfoMap::const_iterator const it = unicodesymbols.find(c);
				if (it == unicodesymbols.end() || !it->second.force)
					encodable_.insert(c);
			}
		}
	}
	lyxerr.enable();
	CharSet::iterator it = encodable_.find(start_encodable_);
	while (it != encodable_.end()) {
		encodable_.erase(it);
		++start_encodable_;
		it = encodable_.find(start_encodable_);
	}
	complete_ = true;
}


docstring Encoding::latexChar(char_type c, bool for_mathed) const
{
	// assure the used encoding is properly initialized
	init();

	if (iconvName_ == "UTF-8" && package_ == none)
		return docstring(1, c);
	if (c < start_encodable_ && !encodings.isForced(c))
		return docstring(1, c);
	if (encodable_.find(c) != encodable_.end())
		return docstring(1, c);
	if (for_mathed)
		return docstring();

	// c cannot (or should not) be encoded in this encoding
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	if (it == unicodesymbols.end())
		throw EncodingException(c);
	// at least one of mathcommand and textcommand is nonempty
	if (it->second.textcommand.empty())
		return "\\ensuremath{" + it->second.mathcommand + '}';
	return it->second.textcommand;
}


vector<char_type> Encoding::symbolsList() const
{
	// assure the used encoding is properly initialized
	init();

	// first all encodable characters
	vector<char_type> symbols(encodable_.begin(), encodable_.end());
	// add those below start_encodable_
	for (char_type c = 0; c < start_encodable_; ++c)
		symbols.push_back(c);
	// now the ones from the unicodesymbols file
	CharInfoMap::const_iterator const end = unicodesymbols.end();
	CharInfoMap::const_iterator it = unicodesymbols.begin();
	for (; it != end; ++it)
		symbols.push_back(it->first);
	return symbols;
}


bool Encodings::latexMathChar(char_type c, Encoding const * encoding,
				docstring & command)
{
	if (encoding) {
		command = encoding->latexChar(c, true);
		if (!command.empty())
			return false;
	}
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	if (it == unicodesymbols.end())
		throw EncodingException(c);
	if (it->second.mathcommand.empty()) {
		if (it->second.textcommand.empty())
			throw EncodingException(c);
		command = it->second.textcommand;
		return false;
	}
	command = it->second.mathcommand;
	return true;
}


char_type Encodings::fromLaTeXCommand(docstring const & cmd, bool & combining)
{
	CharInfoMap::const_iterator const end = unicodesymbols.end();
	CharInfoMap::const_iterator it = unicodesymbols.begin();
	for (; it != end; ++it) {
		docstring const math = it->second.mathcommand;
		docstring const text = it->second.textcommand;
		if (math == cmd || text == cmd) {
			combining = it->second.combining;
			return it->first;
		}
	}
	return 0;
}


docstring Encodings::fromLaTeXCommand(docstring const & cmd, docstring & rem)
{
	docstring symbols;
	size_t i = 0;
	size_t const cmdend = cmd.size();
	CharInfoMap::const_iterator const uniend = unicodesymbols.end();
	for (size_t j = 0; j < cmdend; ++j) {
		// Also get the char after a backslash
		if (j + 1 < cmdend && cmd[j] == '\\')
			++j;
		// If a macro argument follows, get it, too
		if (j + 1 < cmdend && cmd[j + 1] == '{') {
			size_t k = j + 1;
			int count = 1;
			while (k < cmdend && count && k != docstring::npos) {
				k = cmd.find_first_of(from_ascii("{}"), k + 1);
				if (cmd[k] == '{')
					++count;
				else
					--count;
			}
			if (k != docstring::npos)
				j = k;
		}
		// Start with this substring and try augmenting it when it is
		// the prefix of some command in the unicodesymbols file
		docstring const subcmd = cmd.substr(i, j - i + 1);

		CharInfoMap::const_iterator it = unicodesymbols.begin();
		size_t unicmd_size = 0;
		char_type c = 0;
		for (; it != uniend; ++it) {
			docstring const math = it->second.mathcommand;
			docstring const text = it->second.textcommand;
			size_t cur_size = max(math.size(), text.size());
			// The current math or text unicode command cannot
			// match, or we already matched a longer one
			if (cur_size < subcmd.size() || cur_size <= unicmd_size)
				continue;

			docstring tmp = subcmd;
			size_t k = j;
			while (prefixIs(math, tmp) || prefixIs(text, tmp)) {
				++k;
				if (k >= cmdend || cur_size <= tmp.size())
					break;
				tmp += cmd[k];
			}
			// No match
			if (k == j)
				continue;

			// The last added char caused a mismatch, because
			// we didn't exhaust the chars in cmd and didn't
			// exceed the maximum size of the current unicmd
			if (k < cmdend && cur_size > tmp.size())
				tmp.resize(tmp.size() - 1);

			// If this is an exact match, we found a (longer)
			// matching command in the unicodesymbols file
			if (math == tmp || text == tmp) {
				c = it->first;
				j = k - 1;
				i = j + 1;
				unicmd_size = cur_size;
			}
		}
		if (unicmd_size)
			symbols += c;
		else if (j + 1 == cmdend)
			// No luck. Return what remains
			rem = cmd.substr(i);
	}
	return symbols;
}


void Encodings::validate(char_type c, LaTeXFeatures & features, bool for_mathed)
{
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	if (it != unicodesymbols.end()) {
		// at least one of mathcommand and textcommand is nonempty
		bool const use_math = (for_mathed && !it->second.mathcommand.empty()) ||
		                      (!for_mathed && it->second.textcommand.empty());
		if (use_math) {
			if (!it->second.mathpreamble.empty()) {
				if (it->second.mathfeature)
					features.require(it->second.mathpreamble);
				else
					features.addPreambleSnippet(it->second.mathpreamble);
			}
		} else {
			if (!it->second.textpreamble.empty()) {
				if (it->second.textfeature)
					features.require(it->second.textpreamble);
				else
					features.addPreambleSnippet(it->second.textpreamble);
			}
			if (for_mathed) {
				features.require("relsize");
				features.require("lyxmathsym");
			}
		}
	}
}


bool Encodings::isHebrewComposeChar(char_type c)
{
	return c <= 0x05c2 && c >= 0x05b0 && c != 0x05be && c != 0x05c0;
}


// Special Arabic letters are ones that do not get connected from left
// they are hamza, alef_madda, alef_hamza, waw_hamza, alef_hamza_under,
// alef, tah_marbota, dal, thal, rah, zai, wow, alef_maksoura

bool Encodings::isArabicSpecialChar(char_type c)
{
	return (c >= 0x0621 && c <= 0x0625) || (c >= 0x0630 && c <= 0x0632)
		|| c == 0x0627 || c == 0x0629 || c == 0x062f || c == 0x0648
		|| c == 0x0649 || c == 0x0698;
}


bool Encodings::isArabicComposeChar(char_type c)
{
	return c >= 0x064b && c <= 0x0652;
}


bool Encodings::isArabicChar(char_type c)
{
	return c >= arabic_start && c <= arabic_end
		&& arabic_table[c-arabic_start][0];
}


char_type Encodings::transformChar(char_type c, Encodings::LetterForm form)
{
	return isArabicChar(c) ? arabic_table[c-arabic_start][form] : c;
}


bool Encodings::isCombiningChar(char_type c)
{
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	if (it != unicodesymbols.end())
		return it->second.combining;
	return false;
}


bool Encodings::isKnownScriptChar(char_type const c, string & preamble)
{
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);

	if (it == unicodesymbols.end())
		return false;

	if (it->second.textpreamble != "textgreek" && it->second.textpreamble != "textcyr")
		return false;

	if (preamble.empty()) {
		preamble = it->second.textpreamble;
		return true;
	}
	return it->second.textpreamble == preamble;
}


bool Encodings::isForced(char_type c)
{
	return (!forced.empty() && forced.find(c) != forced.end());
}


Encoding const * Encodings::fromLyXName(string const & name) const
{
	EncodingList::const_iterator const it = encodinglist.find(name);
	return it != encodinglist.end() ? &it->second : 0;
}


Encoding const * Encodings::fromLaTeXName(string const & name) const
{
	// We don't use find_if because it makes copies of the pairs in
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
	Lexer symbolslex;
	symbolslex.setFile(symbolsfile);
	bool getNextToken = true;
	while (symbolslex.isOK()) {
		char_type symbol;
		CharInfo info;
		string flags;

		if (getNextToken) {
			if (!symbolslex.next(true))
				break;
		} else
			getNextToken = true;

		istringstream is(symbolslex.getString());
		// reading symbol directly does not work if
		// char_type == wchar_t.
		boost::uint32_t tmp;
		if(!(is >> hex >> tmp))
			break;
		symbol = tmp;

		if (!symbolslex.next(true))
			break;
		info.textcommand = symbolslex.getDocString();
		if (!symbolslex.next(true))
			break;
		info.textpreamble = symbolslex.getString();
		if (!symbolslex.next(true))
			break;
		flags = symbolslex.getString();

		info.combining = false;
		info.textfeature = false;
		info.force = false;
		while (!flags.empty()) {
			string flag;
			flags = split(flags, flag, ',');
			if (flag == "combining")
				info.combining = true;
			else if (flag == "force") {
				info.force = true;
				forced.insert(symbol);
			} else
				lyxerr << "Ignoring unknown flag `" << flag
				       << "' for symbol `0x"
				       << hex << symbol << dec
				       << "'." << endl;
		}
		// mathcommand and mathpreamble have been added for 1.6.0.
		// make them optional so that old files still work.
		int const lineno = symbolslex.lineNumber();
		bool breakout = false;
		if (symbolslex.next(true)) {
			if (symbolslex.lineNumber() != lineno) {
				// line in old format without mathcommand and mathpreamble
				getNextToken = false;
			} else {
				info.mathcommand = symbolslex.getDocString();
				if (symbolslex.next(true)) {
					if (symbolslex.lineNumber() != lineno) {
						// line in new format with mathcommand only
						getNextToken = false;
					} else {
						// line in new format with mathcommand and mathpreamble
						info.mathpreamble = symbolslex.getString();
					}
				} else
					breakout = true;
			}
		} else {
			breakout = true;
		}

		if (!info.textpreamble.empty())
			info.textfeature = info.textpreamble[0] != '\\';
		if (!info.mathpreamble.empty())
			info.mathfeature = info.mathpreamble[0] != '\\';

		LYXERR(Debug::INFO, "Read unicode symbol " << symbol << " '"
			<< to_utf8(info.textcommand) << "' '" << info.textpreamble
			<< "' " << info.combining << ' ' << info.textfeature
			<< " '" << to_utf8(info.mathcommand) << "' '"
			<< info.mathpreamble << "' " << info.mathfeature);

		// we assume that at least one command is nonempty when using unicodesymbols
		if (!info.textcommand.empty() || !info.mathcommand.empty())
			unicodesymbols[symbol] = info;

		if (breakout)
			break;
	}

	// Now read the encodings
	enum {
		et_encoding = 1,
		et_end,
	};

	LexerKeyword encodingtags[] = {
		{ "encoding", et_encoding },
		{ "end", et_end }
	};

	Lexer lex(encodingtags);
	lex.setFile(encfile);
	lex.setContext("Encodings::read");
	while (lex.isOK()) {
		switch (lex.lex()) {
		case et_encoding:
		{
			lex.next();
			string const name = lex.getString();
			lex.next();
			string const latexname = lex.getString();
			lex.next();
			string const guiname = lex.getString();
			lex.next();
			string const iconvname = lex.getString();
			lex.next();
			string const width = lex.getString();
			bool fixedwidth = false;
			if (width == "fixed")
				fixedwidth = true;
			else if (width == "variable")
				fixedwidth = false;
			else
				lex.printError("Unknown width");

			lex.next();
			string const p = lex.getString();
			Encoding::Package package = Encoding::none;
			if (p == "none")
				package = Encoding::none;
			else if (p == "inputenc")
				package = Encoding::inputenc;
			else if (p == "CJK")
				package = Encoding::CJK;
			else
				lex.printError("Unknown package");

			LYXERR(Debug::INFO, "Reading encoding " << name);
			encodinglist[name] = Encoding(name, latexname,
				guiname, iconvname, fixedwidth, package);

			if (lex.lex() != et_end)
				lex.printError("Missing end");
			break;
		}
		case et_end:
			lex.printError("Misplaced end");
			break;
		case Lexer::LEX_FEOF:
			break;
		default:
			lex.printError("Unknown tag");
			break;
		}
	}
}


} // namespace lyx
