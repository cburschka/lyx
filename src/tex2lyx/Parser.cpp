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

#include "Parser.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace lyx {

namespace {

CatCode theCatcode[256];

void catInit()
{
	fill(theCatcode, theCatcode + 256, catOther);
	fill(theCatcode + 'a', theCatcode + 'z' + 1, catLetter);
	fill(theCatcode + 'A', theCatcode + 'Z' + 1, catLetter);

	theCatcode[int('\\')] = catEscape;
	theCatcode[int('{')]  = catBegin;
	theCatcode[int('}')]  = catEnd;
	theCatcode[int('$')]  = catMath;
	theCatcode[int('&')]  = catAlign;
	theCatcode[int('\n')] = catNewline;
	theCatcode[int('#')]  = catParameter;
	theCatcode[int('^')]  = catSuper;
	theCatcode[int('_')]  = catSub;
	theCatcode[0x7f]      = catIgnore;
	theCatcode[int(' ')]  = catSpace;
	theCatcode[int('\t')] = catSpace;
	theCatcode[int('\r')] = catNewline;
	theCatcode[int('~')]  = catActive;
	theCatcode[int('%')]  = catComment;

	// This is wrong!
	theCatcode[int('@')]  = catLetter;
}


/*!
 * Translate a line ending to '\n'.
 * \p c must have catcode catNewline, and it must be the last character read
 * from \p is.
 */
char getNewline(istream & is, char c)
{
	// we have to handle 3 different line endings:
	// - UNIX (\n)
	// - MAC  (\r)
	// - DOS  (\r\n)
	if (c == '\r') {
		// MAC or DOS
		if (is.get(c) && c != '\n') {
			// MAC
			is.putback(c);
		}
		return '\n';
	}
	// UNIX
	return c;
}

}


//
// catcodes
//

CatCode catcode(unsigned char c)
{
	return theCatcode[c];
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
		os << t.character();
	else if (t.cat() == catNewline)
		os << "[" << t.cs().size() << "\\n," << t.cat() << "]\n";
	else
		os << '[' << t.character() << ',' << t.cat() << ']';
	return os;
}


string Token::asString() const
{
	return cs_.size() ? cs_ : string(1, char_);
}


string Token::asInput() const
{
	if (cat_ == catComment)
		return '%' + cs_ + '\n';
	if (cat_ == catSpace || cat_ == catNewline)
		return cs_;
	return char_ ? string(1, char_) : '\\' + cs_;
}


//
// Parser
//


Parser::Parser(istream & is)
	: lineno_(0), pos_(0)
{
	tokenize(is);
}


Parser::Parser(string const & s)
	: lineno_(0), pos_(0)
{
	istringstream is(s);
	tokenize(is);
}


void Parser::push_back(Token const & t)
{
	tokens_.push_back(t);
}


Token const & Parser::prev_token() const
{
	static const Token dummy;
	return pos_ > 1 ? tokens_[pos_ - 2] : dummy;
}


Token const & Parser::curr_token() const
{
	static const Token dummy;
	return pos_ > 0 ? tokens_[pos_ - 1] : dummy;
}


Token const & Parser::next_token() const
{
	static const Token dummy;
	return good() ? tokens_[pos_] : dummy;
}


Token const & Parser::get_token()
{
	static const Token dummy;
	//cerr << "looking at token " << tokens_[pos_] << " pos: " << pos_ << '\n';
	return good() ? tokens_[pos_++] : dummy;
}


bool Parser::isParagraph() const
{
	// A new paragraph in TeX ist started
	// - either by a newline, following any amount of whitespace
	//   characters (including zero), and another newline
	// - or the token \par
	if (curr_token().cat() == catNewline &&
	    (curr_token().cs().size() > 1 ||
	     (next_token().cat() == catSpace &&
	      pos_ < tokens_.size() - 1 &&
	      tokens_[pos_ + 1].cat() == catNewline)))
		return true;
	if (curr_token().cat() == catEscape && curr_token().cs() == "par")
		return true;
	return false;
}


void Parser::skip_spaces(bool skip_comments)
{
	// We just silently return if we have no more tokens.
	// skip_spaces() should be callable at any time,
	// the caller must check p::good() anyway.
	while (good()) {
		get_token();
		if (isParagraph()) {
			putback();
			break;
		}
		if ( curr_token().cat() == catSpace ||
		     curr_token().cat() == catNewline ||
		    (curr_token().cat() == catComment && curr_token().cs().empty()))
			continue;
		if (skip_comments && curr_token().cat() == catComment)
			cerr << "  Ignoring comment: " << curr_token().asInput();
		else {
			putback();
			break;
		}
	}
}


void Parser::unskip_spaces(bool skip_comments)
{
	while (pos_ > 0) {
		if ( curr_token().cat() == catSpace ||
		    (curr_token().cat() == catNewline && curr_token().cs().size() == 1))
			putback();
		else if (skip_comments && curr_token().cat() == catComment) {
			// TODO: Get rid of this
			cerr << "Unignoring comment: " << curr_token().asInput();
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


bool Parser::good() const
{
	return pos_ < tokens_.size();
}


char Parser::getChar()
{
	if (!good())
		error("The input stream is not well...");
	return tokens_[pos_++].character();
}


Parser::Arg Parser::getFullArg(char left, char right)
{
	skip_spaces(true);

	// This is needed if a partial file ends with a command without arguments,
	// e. g. \medskip
	if (! good())
		return make_pair(false, string());

	string result;
	char c = getChar();

	if (c != left) {
		putback();
		return make_pair(false, string());
	} else
		while ((c = getChar()) != right && good()) {
			// Ignore comments
			if (curr_token().cat() == catComment) {
				if (!curr_token().cs().empty())
					cerr << "Ignoring comment: " << curr_token().asInput();
			}
			else
				result += curr_token().asInput();
		}

	return make_pair(true, result);
}


string Parser::getArg(char left, char right)
{
	return getFullArg(left, right).second;
}


string Parser::getFullOpt()
{
	Arg arg = getFullArg('[', ']');
	if (arg.first)
		return '[' + arg.second + ']';
	return string();
}


string Parser::getOpt()
{
	string const res = getArg('[', ']');
	return res.empty() ? string() : '[' + res + ']';
}


string Parser::getFullParentheseArg()
{
	Arg arg = getFullArg('(', ')');
	if (arg.first)
		return '(' + arg.second + ')';
	return string();
}


string const Parser::verbatimEnvironment(string const & name)
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
			   << verbatimEnvironment(env)
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


void Parser::tokenize_one(istream & is)
{
	char c;
	if (!is.get(c)) 
		return;
	//cerr << "reading c: " << c << "\n";

	switch (catcode(c)) {
	case catSpace: {
		string s(1, c);
		while (is.get(c) && catcode(c) == catSpace)
			s += c;
		if (catcode(c) != catSpace)
			is.putback(c);
		push_back(Token(s, catSpace));
		break;
	}
		
	case catNewline: {
		++lineno_;
		string s(1, getNewline(is, c));
		while (is.get(c) && catcode(c) == catNewline) {
			++lineno_;
			s += getNewline(is, c);
		}
		if (catcode(c) != catNewline)
			is.putback(c);
		push_back(Token(s, catNewline));
		break;
	}
		
	case catComment: {
		// We don't treat "%\n" combinations here specially because
		// we want to preserve them in the preamble
		string s;
		while (is.get(c) && catcode(c) != catNewline)
			s += c;
		// handle possible DOS line ending
		if (catcode(c) == catNewline)
			c = getNewline(is, c);
		// Note: The '%' at the beginning and the '\n' at the end
		// of the comment are not stored.
		++lineno_;
		push_back(Token(s, catComment));
		break;
	}
		
	case catEscape: {
		is.get(c);
		if (!is) {
			error("unexpected end of input");
		} else {
			string s(1, c);
			if (catcode(c) == catLetter) {
				// collect letters
				while (is.get(c) && catcode(c) == catLetter)
					s += c;
				if (catcode(c) != catLetter)
					is.putback(c);
			}
			push_back(Token(s, catEscape));
		}
		break;
	}
		
	case catIgnore: {
		cerr << "ignoring a char: " << int(c) << "\n";
		break;
	}
		
	default:
		push_back(Token(c, catcode(c)));
	}
}


void Parser::tokenize(istream & is)
{
	static bool init_done = false;

	if (!init_done) {
		catInit();
		init_done = true;
	}

	while (is) 
		tokenize_one(is);
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


string Parser::verbatimOption()
{
	string res;
	if (next_token().character() == '[') {
		Token t = get_token();
		for (Token t = get_token(); t.character() != ']' && good(); t = get_token()) {
			if (t.cat() == catBegin) {
				putback();
				res += '{' + verbatim_item() + '}';
			} else
				res += t.asString();
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


void Parser::reset()
{
	pos_ = 0;
}


void Parser::setCatCode(char c, CatCode cat)
{
	theCatcode[(unsigned char)c] = cat;
}


CatCode Parser::getCatCode(char c) const
{
	return theCatcode[(unsigned char)c];
}


} // namespace lyx
