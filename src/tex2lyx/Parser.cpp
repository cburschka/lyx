/**
 * \file Parser.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Encoding.h"
#include "Parser.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <iostream>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

/*!
 * Translate a line ending to '\n'.
 * \p c must have catcode catNewline, and it must be the last character read
 * from \p is.
 */
char_type getNewline(iparserdocstream & is, char_type c)
{
	// we have to handle 3 different line endings:
	// - UNIX (\n)
	// - MAC  (\r)
	// - DOS  (\r\n)
	if (c == '\r') {
		// MAC or DOS
		char_type wc;
		if (is.get(wc) && wc != '\n') {
			// MAC
			is.putback(wc);
		}
		return '\n';
	}
	// UNIX
	return c;
}

}

//
// Token
//

ostream & operator<<(ostream & os, Token const & t)
{
	if (t.cat() == catComment)
		os << '%' << t.cs() << '\n';
	else if (t.cat() == catSpace)
		os << t.cs();
	else if (t.cat() == catEscape)
		os << '\\' << t.cs() << ' ';
	else if (t.cat() == catLetter)
		os << t.cs();
	else if (t.cat() == catNewline)
		os << "[" << t.cs().size() << "\\n," << t.cat() << "]\n";
	else
		os << '[' << t.cs() << ',' << t.cat() << ']';
	return os;
}


string Token::asInput() const
{
	if (cat_ == catComment)
		return '%' + cs_ + '\n';
	if (cat_ == catEscape)
		return '\\' + cs_;
	return cs_;
}


bool Token::isAlnumASCII() const
{
	return cat_ == catLetter ||
	       (cat_ == catOther && cs_.length() == 1 && isDigitASCII(cs_[0]));
}


#ifdef FILEDEBUG
void debugToken(std::ostream & os, Token const & t, unsigned int flags)
{
	char sep = ' ';
	os << "t: " << t << " flags: " << flags;
	if (flags & FLAG_BRACE_LAST) { os << sep << "BRACE_LAST"; sep = '|'; }
	if (flags & FLAG_RIGHT     ) { os << sep << "RIGHT"     ; sep = '|'; }
	if (flags & FLAG_END       ) { os << sep << "END"       ; sep = '|'; }
	if (flags & FLAG_BRACK_LAST) { os << sep << "BRACK_LAST"; sep = '|'; }
	if (flags & FLAG_TEXTMODE  ) { os << sep << "TEXTMODE"  ; sep = '|'; }
	if (flags & FLAG_ITEM      ) { os << sep << "ITEM"      ; sep = '|'; }
	if (flags & FLAG_LEAVE     ) { os << sep << "LEAVE"     ; sep = '|'; }
	if (flags & FLAG_SIMPLE    ) { os << sep << "SIMPLE"    ; sep = '|'; }
	if (flags & FLAG_EQUATION  ) { os << sep << "EQUATION"  ; sep = '|'; }
	if (flags & FLAG_SIMPLE2   ) { os << sep << "SIMPLE2"   ; sep = '|'; }
	if (flags & FLAG_OPTION    ) { os << sep << "OPTION"    ; sep = '|'; }
	if (flags & FLAG_BRACED    ) { os << sep << "BRACED"    ; sep = '|'; }
	if (flags & FLAG_CELL      ) { os << sep << "CELL"      ; sep = '|'; }
	if (flags & FLAG_TABBING   ) { os << sep << "TABBING"   ; sep = '|'; }
	os << "\n";
}
#endif


//
// Wrapper
//

void iparserdocstream::setEncoding(std::string const & e)
{
	is_ << lyx::setEncoding(e);
}


void iparserdocstream::putback(char_type c)
{
	s_ = c + s_;
}


void iparserdocstream::putback(docstring s)
{
	s_ = s + s_;
}


iparserdocstream & iparserdocstream::get(char_type &c)
{
	if (s_.empty())
		is_.get(c);
	else {
		//cerr << "unparsed: " << to_utf8(s_) <<endl;
		c = s_[0];
		s_.erase(0,1);
	}
	return *this;
}


//
// Parser
//


Parser::Parser(idocstream & is, std::string const & fixedenc)
	: lineno_(0), pos_(0), iss_(0), is_(is),
	  encoding_iconv_(fixedenc.empty() ? "UTF-8" : fixedenc),
	  theCatcodesType_(NORMAL_CATCODES), curr_cat_(UNDECIDED_CATCODES),
	  fixed_enc_(!fixedenc.empty())
{
	if (fixed_enc_)
		is_.setEncoding(fixedenc);
	catInit();
}


Parser::Parser(string const & s)
	: lineno_(0), pos_(0),
	  iss_(new idocstringstream(from_utf8(s))), is_(*iss_),
	  encoding_iconv_("UTF-8"),
	  theCatcodesType_(NORMAL_CATCODES), curr_cat_(UNDECIDED_CATCODES),
	  // An idocstringstream can not change the encoding
	  fixed_enc_(true)
{
	catInit();
}


Parser::~Parser()
{
	delete iss_;
}


void Parser::deparse()
{
	string s;
	for(size_type i = pos_ ; i < tokens_.size() ; ++i) {
		s += tokens_[i].asInput();
	}
	is_.putback(from_utf8(s));
	tokens_.erase(tokens_.begin() + pos_, tokens_.end());
	// make sure that next token is read
	tokenize_one();
}


bool Parser::setEncoding(std::string const & e, int const & p)
{
	// We may (and need to) use unsafe encodings here: Since the text is
	// converted to unicode while reading from is_, we never see text in
	// the original encoding of the parser, but operate on utf8 strings
	// instead. Therefore, we cannot misparse high bytes as {, } or \\.
	Encoding const * const enc = encodings.fromLaTeXName(e, p, true);
	if (!enc) {
		cerr << "Unknown encoding " << e << ". Ignoring." << std::endl;
		return false;
	}
	return setEncoding(enc->iconvName());
}


void Parser::catInit()
{
	if (curr_cat_ == theCatcodesType_)
		return;
	curr_cat_ = theCatcodesType_;

	fill(theCatcode_, theCatcode_ + 256, catOther);
	fill(theCatcode_ + 'a', theCatcode_ + 'z' + 1, catLetter);
	fill(theCatcode_ + 'A', theCatcode_ + 'Z' + 1, catLetter);
	// This is wrong!
	theCatcode_[int('@')]  = catLetter;

	if (theCatcodesType_ == NORMAL_CATCODES) {
		theCatcode_[int('\\')] = catEscape;
		theCatcode_[int('{')]  = catBegin;
		theCatcode_[int('}')]  = catEnd;
		theCatcode_[int('$')]  = catMath;
		theCatcode_[int('&')]  = catAlign;
		theCatcode_[int('\n')] = catNewline;
		theCatcode_[int('#')]  = catParameter;
		theCatcode_[int('^')]  = catSuper;
		theCatcode_[int('_')]  = catSub;
		theCatcode_[0x7f]      = catIgnore;
		theCatcode_[int(' ')]  = catSpace;
		theCatcode_[int('\t')] = catSpace;
		theCatcode_[int('\r')] = catNewline;
		theCatcode_[int('~')]  = catActive;
		theCatcode_[int('%')]  = catComment;
	}
}

CatCode Parser::catcode(char_type c) const
{
	if (c < 256)
		return theCatcode_[(unsigned char)c];
	return catOther;
}


void Parser::setCatcode(char c, CatCode cat)
{
	theCatcode_[(unsigned char)c] = cat;
	deparse();
}


void Parser::setCatcodes(cat_type t)
{
	theCatcodesType_ = t;
	deparse();
}


bool Parser::setEncoding(std::string const & e)
{
	//cerr << "setting encoding to " << e << std::endl;
	encoding_iconv_ = e;
	// If the encoding is fixed, we must not change the stream encoding
	// (because the whole input uses that encoding, e.g. if it comes from
	// the clipboard). We still need to track the original encoding in
	// encoding_iconv_, so that the generated output is correct.
	if (!fixed_enc_)
		is_.setEncoding(e);
	return true;
}


void Parser::push_back(Token const & t)
{
	tokens_.push_back(t);
}


// We return a copy here because the tokens_ vector may get reallocated
Token const Parser::prev_token() const
{
	static const Token dummy;
	return pos_ > 1 ? tokens_[pos_ - 2] : dummy;
}


// We return a copy here because the tokens_ vector may get reallocated
Token const Parser::curr_token() const
{
	static const Token dummy;
	return pos_ > 0 ? tokens_[pos_ - 1] : dummy;
}


// We return a copy here because the tokens_ vector may get reallocated
Token const Parser::next_token()
{
	static const Token dummy;
	if (!good())
		return dummy;
	if (pos_ >= tokens_.size())
		tokenize_one();
	return pos_ < tokens_.size() ? tokens_[pos_] : dummy;
}


// We return a copy here because the tokens_ vector may get reallocated
Token const Parser::next_next_token()
{
	static const Token dummy;
	if (!good())
		return dummy;
	// If tokenize_one() has not been called after the last get_token() we
	// need to tokenize two more tokens.
	if (pos_ >= tokens_.size())
		tokenize_one();
	if (pos_ + 1 >= tokens_.size())
		tokenize_one();
	return pos_ + 1 < tokens_.size() ? tokens_[pos_ + 1] : dummy;
}


// We return a copy here because the tokens_ vector may get reallocated
Token const Parser::get_token()
{
	static const Token dummy;
	if (!good())
		return dummy;
	if (pos_ >= tokens_.size()) {
		tokenize_one();
		if (pos_ >= tokens_.size())
			return dummy;
	}
	// cerr << "looking at token " << tokens_[pos_] 
	//      << " pos: " << pos_ << '\n';
	return tokens_[pos_++];
}


bool Parser::isParagraph()
{
	// A new paragraph in TeX ist started
	// - either by a newline, following any amount of whitespace
	//   characters (including zero), and another newline
	// - or the token \par
	if (curr_token().cat() == catNewline &&
	    (curr_token().cs().size() > 1 ||
	     (next_token().cat() == catSpace &&
	      next_next_token().cat() == catNewline)))
		return true;
	if (curr_token().cat() == catEscape && curr_token().cs() == "par")
		return true;
	return false;
}


bool Parser::skip_spaces(bool skip_comments)
{
	// We just silently return if we have no more tokens.
	// skip_spaces() should be callable at any time,
	// the caller must check p::good() anyway.
	bool skipped = false;
	while (good()) {
		get_token();
		if (isParagraph()) {
			putback();
			break;
		}
		if (curr_token().cat() == catSpace ||
		    curr_token().cat() == catNewline) {
			skipped = true;
			continue;
		}
		if ((curr_token().cat() == catComment && curr_token().cs().empty()))
			continue;
		if (skip_comments && curr_token().cat() == catComment) {
			// If positions_ is not empty we are doing some kind
			// of look ahead
			if (!positions_.empty())
				cerr << "  Ignoring comment: "
				     << curr_token().asInput();
		} else {
			putback();
			break;
		}
	}
	return skipped;
}


void Parser::unskip_spaces(bool skip_comments)
{
	while (pos_ > 0) {
		if ( curr_token().cat() == catSpace ||
		    (curr_token().cat() == catNewline && curr_token().cs().size() == 1))
			putback();
		else if (skip_comments && curr_token().cat() == catComment) {
			// TODO: Get rid of this
			// If positions_ is not empty we are doing some kind
			// of look ahead
			if (!positions_.empty())
				cerr << "Unignoring comment: "
				     << curr_token().asInput();
			putback();
		}
		else
			break;
	}
}


void Parser::putback()
{
	--pos_;
}


void Parser::pushPosition()
{
	positions_.push_back(pos_);
}


void Parser::popPosition()
{
	pos_ = positions_.back();
	positions_.pop_back();
	deparse();
}


void Parser::dropPosition()
{
	positions_.pop_back();
}


bool Parser::good()
{
	if (pos_ < tokens_.size())
		return true;
	if (!is_.good())
		return false;
	return is_.peek() != idocstream::traits_type::eof();
}


bool Parser::hasOpt()
{
	// An optional argument can occur in any of the following forms:
	// - \foo[bar]
	// - \foo [bar]
	// - \foo
	//   [bar]
	// - \foo %comment
	//   [bar]

	// remember current position
	unsigned int oldpos = pos_;
	// skip spaces and comments
	while (good()) {
		get_token();
		if (isParagraph()) {
			putback();
			break;
		}
		if (curr_token().cat() == catSpace ||
		    curr_token().cat() == catNewline ||
		    curr_token().cat() == catComment)
			continue;
		putback();
		break;
	}
	bool const retval = (next_token().asInput() == "[");
	pos_ = oldpos;
	return retval;
}


Parser::Arg Parser::getFullArg(char left, char right, bool allow_escaping)
{
	skip_spaces(true);

	// This is needed if a partial file ends with a command without arguments,
	// e. g. \medskip
	if (! good())
		return make_pair(false, string());

	string result;
	Token t = get_token();

	if (t.cat() == catComment || t.cat() == catEscape ||
	    t.character() != left) {
		putback();
		return make_pair(false, string());
	} else {
		while (good()) {
			t = get_token();
			// Ignore comments
			if (t.cat() == catComment) {
				if (!t.cs().empty())
					cerr << "Ignoring comment: " << t.asInput();
				continue;
			}
			if (allow_escaping) {
				if (t.cat() != catEscape && t.character() == right)
					break;
			} else {
				if (t.character() == right) {
					if (t.cat() == catEscape)
						result += '\\';
					break;
				}
			}
			result += t.asInput();
		}
	}
	return make_pair(true, result);
}


string Parser::getArg(char left, char right, bool allow_escaping)
{
	return getFullArg(left, right, allow_escaping).second;
}


string Parser::getFullOpt(bool keepws)
{
	Arg arg = getFullArg('[', ']');
	if (arg.first)
		return '[' + arg.second + ']';
	if (keepws)
		unskip_spaces(true);
	return string();
}


string Parser::getOpt(bool keepws)
{
	string const res = getArg('[', ']');
	if (res.empty()) {
		if (keepws)
			unskip_spaces(true);
		return string();
	}
	return '[' + res + ']';
}


string Parser::getFullParentheseArg()
{
	Arg arg = getFullArg('(', ')');
	if (arg.first)
		return '(' + arg.second + ')';
	return string();
}


string const Parser::ertEnvironment(string const & name)
{
	if (!good())
		return string();

	ostringstream os;
	for (Token t = get_token(); good(); t = get_token()) {
		if (t.cat() == catBegin) {
			putback();
			os << '{' << verbatim_item() << '}';
		} else if (t.asInput() == "\\begin") {
			string const env = getArg('{', '}');
			os << "\\begin{" << env << '}'
			   << ertEnvironment(env)
			   << "\\end{" << env << '}';
		} else if (t.asInput() == "\\end") {
			string const end = getArg('{', '}');
			if (end != name)
				cerr << "\\end{" << end
				     << "} does not match \\begin{" << name
				     << "}." << endl;
			return os.str();
		} else
			os << t.asInput();
	}
	cerr << "unexpected end of input" << endl;
	return os.str();
}


string const Parser::plainEnvironment(string const & name)
{
	if (!good())
		return string();

	ostringstream os;
	for (Token t = get_token(); good(); t = get_token()) {
		if (t.asInput() == "\\end") {
			string const end = getArg('{', '}');
			if (end == name)
				return os.str();
			else
				os << "\\end{" << end << '}';
		} else
			os << t.asInput();
	}
	cerr << "unexpected end of input" << endl;
	return os.str();
}


string const Parser::plainCommand(char left, char right, string const & name)
{
	if (!good())
		return string();
	// check if first token is really the start character
	Token tok = get_token();
	if (tok.character() != left) {
		cerr << "first character does not match start character of command \\" << name << endl;
		return string();
	}
	ostringstream os;
	for (Token t = get_token(); good(); t = get_token()) {
		if (t.character() == right) {
			return os.str();
		} else
			os << t.asInput();
	}
	cerr << "unexpected end of input" << endl;
	return os.str();
}


Parser::Arg Parser::verbatimStuff(string const & end_string, bool const allow_linebreak)
{
	if (!good())
		return Arg(false, string());

	pushPosition();
	ostringstream oss;
	size_t match_index = 0;
	setCatcodes(VERBATIM_CATCODES);
	for (Token t = get_token(); good(); t = get_token()) {
		// FIXME t.asInput() might be longer than we need ?
		if (t.asInput() == end_string.substr(match_index,
						     t.asInput().length())) {
			match_index += t.asInput().length();
			if (match_index >= end_string.length())
				break;
		} else {
			if (!allow_linebreak && t.asInput() == "\n") {
				cerr << "unexpected end of input" << endl;
				popPosition();
				setCatcodes(NORMAL_CATCODES);
				return Arg(false, string());
			}
			if (match_index) {
				oss << end_string.substr(0, match_index) 
				    << t.asInput();
				match_index = 0;
			} else
				oss << t.asInput();
		}
	}

	if (!good()) {
		cerr << "unexpected end of input" << endl;
		popPosition();
		setCatcodes(NORMAL_CATCODES);
		return Arg(false, string());
	}
	setCatcodes(NORMAL_CATCODES);
	dropPosition();
	return Arg(true, oss.str());
}


string const Parser::verbatimEnvironment(string const & name)
{
	//FIXME: do something if endstring is not found
	string s = verbatimStuff("\\end{" + name + "}").second;
	// ignore one newline at beginning or end of string
	if (prefixIs(s, "\n"))
		s.erase(0,1);
	if (suffixIs(s, "\n"))
		s.erase(s.length() - 1,1);
	return s;
}


string Parser::verbatimOption()
{
	string res;
	if (next_token().character() == '[') {
		Token t = get_token();
		for (t = get_token(); t.character() != ']' && good(); t = get_token()) {
			if (t.cat() == catBegin) {
				putback();
				res += '{' + verbatim_item() + '}';
			} else
				res += t.asInput();
		}
	}
	return res;
}


string Parser::verbatim_item()
{
	if (!good())
		error("stream bad");
	skip_spaces();
	if (next_token().cat() == catBegin) {
		Token t = get_token(); // skip brace
		string res;
		for (Token t = get_token(); t.cat() != catEnd && good(); t = get_token()) {
			if (t.cat() == catBegin) {
				putback();
				res += '{' + verbatim_item() + '}';
			}
			else
				res += t.asInput();
		}
		return res;
	}
	return get_token().asInput();
}


void Parser::tokenize_one()
{
	catInit();
	char_type c;
	if (!is_.get(c))
		return;

	switch (catcode(c)) {
	case catSpace: {
		docstring s(1, c);
		while (is_.get(c) && catcode(c) == catSpace)
			s += c;
		if (catcode(c) != catSpace)
			is_.putback(c);
		push_back(Token(s, catSpace));
		break;
	}

	case catNewline: {
		++lineno_;
		docstring s(1, getNewline(is_, c));
		while (is_.get(c) && catcode(c) == catNewline) {
			++lineno_;
			s += getNewline(is_, c);
		}
		if (catcode(c) != catNewline)
			is_.putback(c);
		push_back(Token(s, catNewline));
		break;
	}

	case catComment: {
		// We don't treat "%\n" combinations here specially because
		// we want to preserve them in the preamble
		docstring s;
		while (is_.get(c) && catcode(c) != catNewline)
			s += c;
		// handle possible DOS line ending
		if (catcode(c) == catNewline)
			c = getNewline(is_, c);
		// Note: The '%' at the beginning and the '\n' at the end
		// of the comment are not stored.
		++lineno_;
		push_back(Token(s, catComment));
		break;
	}

	case catEscape: {
		is_.get(c);
		if (!is_) {
			error("unexpected end of input");
		} else {
			docstring s(1, c);
			if (catcode(c) == catLetter) {
				// collect letters
				while (is_.get(c) && catcode(c) == catLetter)
					s += c;
				if (catcode(c) != catLetter)
					is_.putback(c);
			}
			push_back(Token(s, catEscape));
		}
		break;
	}

	case catIgnore: {
		cerr << "ignoring a char: " << c << "\n";
		break;
	}

	default:
		push_back(Token(docstring(1, c), catcode(c)));
	}
	//cerr << tokens_.back();
}


void Parser::dump() const
{
	cerr << "\nTokens: ";
	for (unsigned i = 0; i < tokens_.size(); ++i) {
		if (i == pos_)
			cerr << " <#> ";
		cerr << tokens_[i];
	}
	cerr << " pos: " << pos_ << "\n";
}


void Parser::error(string const & msg)
{
	cerr << "Line ~" << lineno_ << ":  parse error: " << msg << endl;
	dump();
	//exit(1);
}


void Parser::reset()
{
	pos_ = 0;
}


} // namespace lyx
