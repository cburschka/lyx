
#include "texparser.h"
#include "Lsstream.h"

using std::cerr;
using std::endl;
using std::fill;
using std::ios;
using std::istream;
using std::istringstream;
using std::ostream;
using std::string;
using std::vector;


namespace {

CatCode theCatcode[256];

void skipSpaceTokens(istream & is, char c)
{
	// skip trailing spaces
	while (catcode(c) == catSpace || catcode(c) == catNewline)
		if (!is.get(c))
			break;
	//cerr << "putting back: " << c << "\n";
	is.putback(c);
}


void catInit()
{
	fill(theCatcode, theCatcode + 256, catOther);
	fill(theCatcode + 'a', theCatcode + 'z' + 1, catLetter);
	fill(theCatcode + 'A', theCatcode + 'Z' + 1, catLetter);

	theCatcode['\\'] = catEscape;
	theCatcode['{']  = catBegin;
	theCatcode['}']  = catEnd;
	theCatcode['$']  = catMath;
	theCatcode['&']  = catAlign;
	theCatcode['\n'] = catNewline;
	theCatcode['#']  = catParameter;
	theCatcode['^']  = catSuper;
	theCatcode['_']  = catSub;
	theCatcode[''] = catIgnore;
	theCatcode[' ']  = catSpace;
	theCatcode['\t'] = catSpace;
	theCatcode['\r'] = catNewline;
	theCatcode['~']  = catActive;
	theCatcode['%']  = catComment;
}

}


// 
// catcodes
//

mode_type asMode(mode_type oldmode, string const & str)
{
	if (str == "mathmode")
		return MATH_MODE;
	if (str == "textmode" || str == "forcetext")
		return TEXT_MODE;
	return oldmode;
}


CatCode catcode(unsigned char c)
{
	return theCatcode[c];
}



//
// Token
//

ostream & operator<<(ostream & os, Token const & t)
{
	if (t.cs().size())
		os << '\\' << t.cs();
	else
		os << '[' << t.character() << ',' << t.cat() << ']';
	return os;
}



//
// Parser
//


Parser::Parser(istream & is)
	: lineno_(0), pos_(0)
{
	tokenize(is);
}


void Parser::push_back(Token const & t)
{
	tokens_.push_back(t);
}


void Parser::pop_back()
{
	tokens_.pop_back();
}


Token const & Parser::prevToken() const
{
	static const Token dummy;
	return pos_ > 0 ? tokens_[pos_ - 1] : dummy;
}


Token const & Parser::nextToken() const
{
	static const Token dummy;
	return good() ? tokens_[pos_] : dummy;
}


Token const & Parser::getToken()
{
	static const Token dummy;
	//cerr << "looking at token " << tokens_[pos_] << " pos: " << pos_ << '\n';
	return good() ? tokens_[pos_++] : dummy;
}


void Parser::skipSpaces()
{
	while (nextToken().cat() == catSpace || nextToken().cat() == catNewline)
		getToken();
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


string Parser::getArg(char left, char right)
{
	skipSpaces();

	string result;
	char c = getChar();

	if (c != left)
		putback();
	else
		while ((c = getChar()) != right && good())
			result += c;

	return result;
}


void Parser::tokenize(istream & is)
{
	static bool init_done = false;

	if (!init_done) {
		catInit();
		init_done = true;
	}

	char c;
	while (is.get(c)) {
		//cerr << "reading c: " << c << "\n";

		switch (catcode(c)) {
			case catNewline: {
				++lineno_;
				is.get(c);
				if (catcode(c) == catNewline)
					push_back(Token("par"));
				else {
					push_back(Token('\n', catNewline));
					is.putback(c);
				}
				break;
			}

/*
			case catComment: {
				while (is.get(c) && catcode(c) != catNewline)
					;
				++lineno_;
				break;
			}
*/

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
						skipSpaceTokens(is, c);
					}
					push_back(Token(s));
				}
				break;
			}

			case catSuper:
			case catSub: {
				push_back(Token(c, catcode(c)));
				is.get(c);
				skipSpaceTokens(is, c);
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
	if (nextToken().character() == '[') {
		Token t = getToken();
		for (Token t = getToken(); t.character() != ']' && good(); t = getToken()) {
			if (t.cat() == catBegin) {
				putback();
				res += '{' + verbatimItem() + '}';
			} else
				res += t.asString();
		}
	}
	return res;
}


string Parser::verbatimItem()
{
	string res;
	if (nextToken().cat() == catBegin) {
		Token t = getToken();
		for (Token t = getToken(); t.cat() != catEnd && good(); t = getToken()) {
			if (t.cat() == catBegin) {
				putback();
				res += '{' + verbatimItem() + '}';
			}
			else
				res += t.asString();
		}
	}
	return res;
}
