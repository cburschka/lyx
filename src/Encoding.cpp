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
#include "support/mutex.h"
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
	docstring const & textcommand, docstring const & mathcommand,
	std::string const & textpreamble, std::string const & mathpreamble,
	std::string const & tipashortcut, unsigned int flags)
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
	// Since the the constructor is the only method which sets complete_
	// to false the test for complete_ is thread-safe without mutex.
	if (complete_)
		return;

	static Mutex mutex;
	Mutex::Locker lock(&mutex);

	// We need to test again for complete_, since another thread could
	// have set it to true while we were waiting for the lock and we must
	// not modify an encoding which is already complete.
	if (complete_)
		return;

	// We do not make any member mutable  so that it can be easily verified
	// that all const methods are thread-safe: init() is the only const
	// method which changes complete_, encodable_ and start_encodable_, and
	// it uses a mutex to ensure thread-safety.
	CharSet & encodable = const_cast<Encoding *>(this)->encodable_;
	char_type & start_encodable = const_cast<Encoding *>(this)->start_encodable_;

	start_encodable = 0;
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
				encodable.insert(uc);
			else if (!it->second.force()) {
				if (forced_->empty() || forced_->find(uc) == forced_->end())
					encodable.insert(uc);
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
					encodable.insert(c);
				else if (!it->second.force()) {
					if (forced_->empty() || forced_->find(c) == forced_->end())
						encodable.insert(c);
				}
			}
		}
	}
	lyxerr.enable();
	CharSet::iterator it = encodable_.find(start_encodable_);
	while (it != encodable_.end()) {
		encodable.erase(it);
		++start_encodable;
		it = encodable_.find(start_encodable_);
	}
	const_cast<Encoding *>(this)->complete_ = true;
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


pair<docstring, docstring> Encoding::latexString(docstring const & input, bool dryrun) const
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


CharInfo const & Encodings::unicodeCharInfo(char_type c)
{
	static CharInfo empty;
	CharInfoMap::const_iterator const it = unicodesymbols.find(c);
	return it != unicodesymbols.end() ? it->second : empty;
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
	if (it == encodinglist.end())
		return 0;
	if (!allowUnsafe && it->second.unsafe())
		return 0;
	return &it->second;
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
