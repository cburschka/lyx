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

#include "Lexer.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"
#include "support/unicode.h"

#include <boost/cstdint.hpp>

#include <sstream>
#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {

int const Encoding::any = -1;

Encodings encodings;

Encodings::MathCommandSet Encodings::mathcmd;
Encodings::TextCommandSet Encodings::textcmd;
Encodings::MathSymbolSet  Encodings::mathsym;

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


typedef map<char_type, CharInfo> CharInfoMap;
CharInfoMap unicodesymbols;

typedef set<char_type> CharSet;
typedef map<string, CharSet> CharSetMap;
CharSet forced;
CharSetMap forcedselected;

typedef set<char_type> MathAlphaSet;
MathAlphaSet mathalpha;


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


CharInfo::CharInfo(
	docstring const textcommand, docstring const mathcommand,
	std::string const textpreamble, std::string const mathpreamble,
	std::string const tipashortcut, unsigned int flags)
	: textcommand_(textcommand), mathcommand_(mathcommand),
	  textpreamble_(textpreamble), mathpreamble_(mathpreamble),
	  tipashortcut_(tipashortcut), flags_(flags)
{
}

Encoding::Encoding(string const & n, string const & l, string const & g,
		   string const & i, bool f, bool u, Encoding::Package p)
	: name_(n), latexName_(l), guiName_(g), iconvName_(i), fixedwidth_(f),
	  unsafe_(u), forced_(&forcedselected[n]), package_(p)
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
		start_encodable_ = 0;
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
			if (it == unicodesymbols.end())
				encodable_.insert(uc);
			else if (!it->second.force()) {
				if (forced_->empty() || forced_->find(uc) == forced_->end())
					encodable_.insert(uc);
			}
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
				if (it == unicodesymbols.end())
					encodable_.insert(c);
				else if (!it->second.force()) {
					if (forced_->empty() || forced_->find(c) == forced_->end())
						encodable_.insert(c);
				}
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


bool Encoding::isForced(char_type c) const
{
	if (!forced.empty() && forced.find(c) != forced.end())
		return true;
	return !forced_->empty() && forced_->find(c) != forced_->end();
}


bool Encoding::encodable(char_type c) const
{
	// assure the used encoding is properly initialized
	init();

	if (iconvName_ == "UTF-8" && package_ == none)
		return true;
	if (c < start_encodable_ && !isForced(c))
		return true;
	if (encodable_.find(c) != encodable_.end())
		return true;
	return false;
}


pair<docstring, bool> Encoding::latexChar(char_type c) const
{
	if (encodable(c))
		return make_pair(docstring(1, c), false);

	// c cannot (or should not) be encoded in this encoding
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	if (it == unicodesymbols.end())
		throw EncodingException(c);
	// at least one of mathcommand and textcommand is nonempty
	if (it->second.textcommand().empty())
		return make_pair(
			"\\ensuremath{" + it->second.mathcommand() + '}', false);
	return make_pair(it->second.textcommand(), !it->second.textnotermination());
}


pair<docstring, docstring> Encoding::latexString(docstring const input, bool dryrun) const
{
	docstring result;
	docstring uncodable;
	bool terminate = false;
	for (size_t n = 0; n < input.size(); ++n) {
		try {
			char_type const c = input[n];
			pair<docstring, bool> latex_char = latexChar(c);
			docstring const latex = latex_char.first;
			if (terminate && !prefixIs(latex, '\\')
			    && !prefixIs(latex, '{')
			    && !prefixIs(latex, '}')) {
					// Prevent eating of a following
					// space or command corruption by
					// following characters
					if (latex == " ")
						result += "{}";
					else
						result += " ";
				}
			result += latex;
			terminate = latex_char.second;
		} catch (EncodingException & /* e */) {
			LYXERR0("Uncodable character in latexString!");
			if (dryrun) {
				result += "<" + _("LyX Warning: ")
					   + _("uncodable character") + " '";
				result += docstring(1, input[n]);
				result += "'>";
			} else
				uncodable += input[n];
		}
	}
	return make_pair(result, uncodable);
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


bool Encodings::latexMathChar(char_type c, bool mathmode,
			Encoding const * encoding, docstring & command,
			bool & needsTermination)
{
	command = empty_docstring();
	if (encoding)
		if (encoding->encodable(c))
			command = docstring(1, c);
	needsTermination = false;

	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	if (it == unicodesymbols.end()) {
		if (!encoding || command.empty())
			throw EncodingException(c);
		if (mathmode)
			addMathSym(c);
		return false;
	}
	// at least one of mathcommand and textcommand is nonempty
	bool use_math = (mathmode && !it->second.mathcommand().empty()) ||
			(!mathmode && it->second.textcommand().empty());
	if (use_math) {
		command = it->second.mathcommand();
		needsTermination = !it->second.mathnotermination();
		addMathCmd(c);
	} else {
		if (!encoding || command.empty()) {
			command = it->second.textcommand();
			needsTermination = !it->second.textnotermination();
			addTextCmd(c);
		}
		if (mathmode)
			addMathSym(c);
	}
	return use_math;
}


char_type Encodings::fromLaTeXCommand(docstring const & cmd, int cmdtype,
		bool & combining, bool & needsTermination, set<string> * req)
{
	CharInfoMap::const_iterator const end = unicodesymbols.end();
	CharInfoMap::const_iterator it = unicodesymbols.begin();
	for (combining = false; it != end; ++it) {
		docstring const math = it->second.mathcommand();
		docstring const text = it->second.textcommand();
		if ((cmdtype & MATH_CMD) && math == cmd) {
			combining = it->second.combining();
			needsTermination = !it->second.mathnotermination();
			if (req && it->second.mathfeature() &&
			    !it->second.mathpreamble().empty())
				req->insert(it->second.mathpreamble());
			return it->first;
		}
		if ((cmdtype & TEXT_CMD) && text == cmd) {
			combining = it->second.combining();
			needsTermination = !it->second.textnotermination();
			if (req && it->second.textfeature() &&
			    !it->second.textpreamble().empty())
				req->insert(it->second.textpreamble());
			return it->first;
		}
	}
	needsTermination = false;
	return 0;
}


docstring Encodings::fromLaTeXCommand(docstring const & cmd, int cmdtype,
		bool & needsTermination, docstring & rem, set<string> * req)
{
	needsTermination = false;
	rem = empty_docstring();
	bool const mathmode = cmdtype & MATH_CMD;
	bool const textmode = cmdtype & TEXT_CMD;
	docstring symbols;
	size_t const cmdend = cmd.size();
	size_t prefix = 0;
	CharInfoMap::const_iterator const uniend = unicodesymbols.end();
	for (size_t i = 0, j = 0; j < cmdend; ++j) {
		// Also get the char after a backslash
		if (j + 1 < cmdend && cmd[j] == '\\') {
			++j;
			prefix = 1;
			// Detect things like \=*{e} as well
			if (j + 3 < cmdend && cmd[j+1] == '*' &&
			    cmd[j+2] == '{') {
				++j;
				prefix = 2;
			}
		}
		// position of the last character before a possible macro
		// argument
		size_t m = j;
		// If a macro argument follows, get it, too
		// Do it here only for single character commands. Other
		// combining commands need this too, but they are handled in
		// the loop below for performance reasons.
		if (j + 1 < cmdend && cmd[j + 1] == '{') {
			size_t k = j + 1;
			int count = 1;
			while (k < cmdend && count) {
				k = cmd.find_first_of(from_ascii("{}"), k + 1);
				// braces may not be balanced
				if (k == docstring::npos)
					break;
				if (cmd[k] == '{')
					++count;
				else
					--count;
			}
			if (k != docstring::npos)
				j = k;
		} else if (m + 1 < cmdend && isAlphaASCII(cmd[m])) {
			while (m + 2 < cmdend && isAlphaASCII(cmd[m+1]))
				m++;
		}
		// Start with this substring and try augmenting it when it is
		// the prefix of some command in the unicodesymbols file
		docstring subcmd = cmd.substr(i, j - i + 1);

		CharInfoMap::const_iterator it = unicodesymbols.begin();
		// First part of subcmd which might be a combining character
		docstring combcmd = (m == j) ? docstring() : cmd.substr(i, m - i + 1);
		// The combining character of combcmd if it exists
		CharInfoMap::const_iterator combining = uniend;
		size_t unicmd_size = 0;
		char_type c = 0;
		for (; it != uniend; ++it) {
			docstring const math = mathmode ? it->second.mathcommand()
							: docstring();
			docstring const text = textmode ? it->second.textcommand()
							: docstring();
			if (!combcmd.empty() && it->second.combining() &&
			    (math == combcmd || text == combcmd))
				combining = it;
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
			// matching entry in the unicodesymbols file.
			if (math != tmp && text != tmp)
				continue;
			// If we found a combining command, we need to append
			// the macro argument if this has not been done above.
			if (tmp == combcmd && combining != uniend &&
			    k < cmdend && cmd[k] == '{') {
				size_t l = k;
				int count = 1;
				while (l < cmdend && count) {
					l = cmd.find_first_of(from_ascii("{}"), l + 1);
					// braces may not be balanced
					if (l == docstring::npos)
						break;
					if (cmd[l] == '{')
						++count;
					else
						--count;
				}
				if (l != docstring::npos) {
					j = l;
					subcmd = cmd.substr(i, j - i + 1);
				}
			}
			// If the entry doesn't start with '\', we take note
			// of the match and continue (this is not a ultimate
			// acceptance, as some other entry may match a longer
			// portion of the cmd string). However, if the entry
			// does start with '\', we accept the match only if
			// this is a valid macro, i.e., either it is a single
			// (nonletter) char macro, or nothing else follows,
			// or what follows is a nonletter char, or the last
			// character is a }.
			else if (tmp[0] != '\\'
				   || (tmp.size() == prefix + 1 &&
				       !isAlphaASCII(tmp[1]) &&
				       (prefix == 1 || !isAlphaASCII(tmp[2])))
				   || k == cmdend 
				   || !isAlphaASCII(cmd[k])
				   || tmp[tmp.size() - 1] == '}'
				 ) {
				c = it->first;
				j = k - 1;
				i = j + 1;
				unicmd_size = cur_size;
				if (math == tmp)
					needsTermination = !it->second.mathnotermination();
				else
					needsTermination = !it->second.textnotermination();
				if (req) {
					if (math == tmp && it->second.mathfeature() &&
					    !it->second.mathpreamble().empty())
						req->insert(it->second.mathpreamble());
					if (text == tmp && it->second.textfeature() &&
					    !it->second.textpreamble().empty())
						req->insert(it->second.textpreamble());
				}
			}
		}
		if (unicmd_size)
			symbols += c;
		else if (combining != uniend &&
		         prefixIs(subcmd, combcmd + '{')) {
			// We know that subcmd starts with combcmd and
			// contains an argument in braces.
			docstring const arg = subcmd.substr(
				combcmd.length() + 1,
				subcmd.length() - combcmd.length() - 2);
			// If arg is a single character we can construct a
			// combining sequence.
			char_type a;
			bool argcomb = false;
			if (arg.size() == 1 && isAlnumASCII(arg[0]))
				a = arg[0];
			else {
				// Use the version of fromLaTeXCommand() that
				// parses only one command, since we cannot
				// use more than one character.
				bool dummy = false;
				set<string> r;
				a = fromLaTeXCommand(arg, cmdtype, argcomb,
				                     dummy, &r);
				if (a && req && !argcomb)
					req->insert(r.begin(), r.end());
			}
			if (a && !argcomb) {
				// In unicode the combining character comes
				// after its base
				symbols += a;
				symbols += combining->first;
				i = j + 1;
				unicmd_size = 2;
			}
		}
		if (j + 1 == cmdend && !unicmd_size) {
			// No luck. Return what remains
			rem = cmd.substr(i);
			if (needsTermination && !rem.empty()) {
				if (rem.substr(0, 2) == "{}") {
					rem = rem.substr(2);
					needsTermination = false;
				} else if (rem[0] == ' ') {
					needsTermination = false;
					// LaTeX would swallow all spaces
					rem = ltrim(rem);
				}
			}
		}
	}
	return symbols;
}


bool Encodings::isHebrewChar(char_type c)
{
	return c >= 0x0590 && c <= 0x05ff;
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


CharInfo const & Encodings::unicodeCharInfo(char_type c)
{
	static CharInfo empty;
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	return it != unicodesymbols.end() ? it->second : empty;
}


char_type Encodings::transformChar(char_type c, Encodings::LetterForm form)
{
	return isArabicChar(c) ? arabic_table[c-arabic_start][form] : c;
}


bool Encodings::isCombiningChar(char_type c)
{
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	if (it != unicodesymbols.end())
		return it->second.combining();
	return false;
}


string const Encodings::TIPAShortcut(char_type c)
{
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	if (it != unicodesymbols.end())
		return it->second.tipashortcut();
	return string();
}


bool Encodings::isKnownScriptChar(char_type const c, string & preamble)
{
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);

	if (it == unicodesymbols.end())
		return false;

	if (it->second.textpreamble() != "textgreek" && it->second.textpreamble() != "textcyr")
		return false;

	if (preamble.empty()) {
		preamble = it->second.textpreamble();
		return true;
	}
	return it->second.textpreamble() == preamble;
}


bool Encodings::isMathAlpha(char_type c)
{
	return mathalpha.count(c);
}


Encoding const *
Encodings::fromLyXName(string const & name, bool allowUnsafe) const
{
	EncodingList::const_iterator const it = encodinglist.find(name);
	if (!allowUnsafe && it->second.unsafe())
		return 0;
	return it != encodinglist.end() ? &it->second : 0;
}


Encoding const *
Encodings::fromLaTeXName(string const & n, int const & p, bool allowUnsafe) const
{
	string name = n;
	// FIXME: if we have to test for too many of these synonyms,
	// we should instead extend the format of lib/encodings
	if (n == "ansinew")
		name = "cp1252";

	// We don't use find_if because it makes copies of the pairs in
	// the map.
	// This linear search is OK since we don't have many encodings.
	// Users could even optimize it by putting the encodings they use
	// most at the top of lib/encodings.
	EncodingList::const_iterator const end = encodinglist.end();
	for (EncodingList::const_iterator it = encodinglist.begin(); it != end; ++it)
		if ((it->second.latexName() == name) && (it->second.package() & p)
				&& (!it->second.unsafe() || allowUnsafe))
			return &it->second;
	return 0;
}


Encoding const *
Encodings::fromIconvName(string const & n, int const & p, bool allowUnsafe) const
{
	EncodingList::const_iterator const end = encodinglist.end();
	for (EncodingList::const_iterator it = encodinglist.begin(); it != end; ++it)
		if ((it->second.iconvName() == n) && (it->second.package() & p)
				&& (!it->second.unsafe() || allowUnsafe))
			return &it->second;
	return 0;
}


Encodings::Encodings()
{}


void Encodings::read(FileName const & encfile, FileName const & symbolsfile)
{
	// We must read the symbolsfile first, because the Encoding
	// constructor depends on it.
	CharSetMap forcednotselected;
	Lexer symbolslex;
	symbolslex.setFile(symbolsfile);
	bool getNextToken = true;
	while (symbolslex.isOK()) {
		char_type symbol;

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
		docstring textcommand = symbolslex.getDocString();
		if (!symbolslex.next(true))
			break;
		string textpreamble = symbolslex.getString();
		if (!symbolslex.next(true))
			break;
		string sflags = symbolslex.getString();
		
		string tipashortcut;
		int flags = 0;

		if (suffixIs(textcommand, '}'))
			flags |= CharInfoTextNoTermination;
		while (!sflags.empty()) {
			string flag;
			sflags = split(sflags, flag, ',');
			if (flag == "combining") {
				flags |= CharInfoCombining;
			} else if (flag == "force") {
				flags |= CharInfoForce;
				forced.insert(symbol);
			} else if (prefixIs(flag, "force=")) {
				vector<string> encodings =
					getVectorFromString(flag.substr(6), ";");
				for (size_t i = 0; i < encodings.size(); ++i)
					forcedselected[encodings[i]].insert(symbol);
				flags |= CharInfoForceSelected;
			} else if (prefixIs(flag, "force!=")) {
				vector<string> encodings =
					getVectorFromString(flag.substr(7), ";");
				for (size_t i = 0; i < encodings.size(); ++i)
					forcednotselected[encodings[i]].insert(symbol);
				flags |= CharInfoForceSelected;
			} else if (flag == "mathalpha") {
				mathalpha.insert(symbol);
			} else if (flag == "notermination=text") {
				flags |= CharInfoTextNoTermination;
			} else if (flag == "notermination=math") {
				flags |= CharInfoMathNoTermination;
			} else if (flag == "notermination=both") {
				flags |= CharInfoTextNoTermination;
				flags |= CharInfoMathNoTermination;
			} else if (flag == "notermination=none") {
				flags &= ~CharInfoTextNoTermination;
				flags &= ~CharInfoMathNoTermination;
			} else if (contains(flag, "tipashortcut=")) {
				tipashortcut = split(flag, '=');
			} else {
				lyxerr << "Ignoring unknown flag `" << flag
				       << "' for symbol `0x"
				       << hex << symbol << dec
				       << "'." << endl;
			}
		}
		// mathcommand and mathpreamble have been added for 1.6.0.
		// make them optional so that old files still work.
		int const lineno = symbolslex.lineNumber();
		bool breakout = false;
		docstring mathcommand;
		string mathpreamble;
		if (symbolslex.next(true)) {
			if (symbolslex.lineNumber() != lineno) {
				// line in old format without mathcommand and mathpreamble
				getNextToken = false;
			} else {
				mathcommand = symbolslex.getDocString();
				if (suffixIs(mathcommand, '}'))
					flags |= CharInfoMathNoTermination;
				if (symbolslex.next(true)) {
					if (symbolslex.lineNumber() != lineno) {
						// line in new format with mathcommand only
						getNextToken = false;
					} else {
						// line in new format with mathcommand and mathpreamble
						mathpreamble = symbolslex.getString();
					}
				} else
					breakout = true;
			}
		} else {
			breakout = true;
		}

		// backward compatibility
		if (mathpreamble == "esintoramsmath")
			mathpreamble = "esint|amsmath";

		if (!textpreamble.empty())
			if (textpreamble[0] != '\\')
				flags |= CharInfoTextFeature;
		if (!mathpreamble.empty())
			if (mathpreamble[0] != '\\')
				flags |= CharInfoMathFeature;

		CharInfo info = CharInfo(
			textcommand, mathcommand,
			textpreamble, mathpreamble,
			tipashortcut, flags);
		LYXERR(Debug::INFO, "Read unicode symbol " << symbol << " '"
			   << to_utf8(info.textcommand()) << "' '" << info.textpreamble()
			   << " '" << info.textfeature() << ' ' << info.textnotermination()
			   << ' ' << to_utf8(info.mathcommand()) << "' '" << info.mathpreamble()
			   << "' " << info.mathfeature() << ' ' << info.mathnotermination()
			   << ' ' << info.combining() << ' ' << info.force()
			   << ' ' << info.forceselected());

		// we assume that at least one command is nonempty when using unicodesymbols
		if (info.isUnicodeSymbol()) {
			unicodesymbols[symbol] = info;
		}

		if (breakout)
			break;
	}

	// Now read the encodings
	enum {
		et_encoding = 1,
		et_end
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
			bool unsafe = false;
			if (width == "fixed")
				fixedwidth = true;
			else if (width == "variable")
				fixedwidth = false;
			else if (width == "variableunsafe") {
				fixedwidth = false;
				unsafe = true;
			}
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
			else if (p == "japanese")
				package = Encoding::japanese;
			else
				lex.printError("Unknown package");

			LYXERR(Debug::INFO, "Reading encoding " << name);
			encodinglist[name] = Encoding(name, latexname,
				guiname, iconvname, fixedwidth, unsafe,
				package);

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

	// Move all information from forcednotselected to forcedselected
	for (CharSetMap::const_iterator it1 = forcednotselected.begin(); it1 != forcednotselected.end(); ++it1) {
		for (CharSetMap::iterator it2 = forcedselected.begin(); it2 != forcedselected.end(); ++it2) {
			if (it2->first != it1->first)
				it2->second.insert(it1->second.begin(), it1->second.end());
		}
	}

}


} // namespace lyx
