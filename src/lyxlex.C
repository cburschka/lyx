/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1996-2001 The LyX Team.
 *
 *   Generalized simple lexical analizer.
 *   It can be used for simple syntax parsers, like lyxrc,
 *   texclass and others to come.   [asierra30/03/96]
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "lyxlex.h"
#endif

#include "lyxlex.h"
#include "lyxlex_pimpl.h"
#include "debug.h"
#include "support/filetools.h"
#include "support/lstrings.h"

using std::ostream;
using std::istream;
using std::endl;


LyXLex::LyXLex(keyword_item * tab, int num)
	: pimpl_(new Pimpl(tab, num))
{}


LyXLex::~LyXLex()
{
	delete pimpl_;
}


bool LyXLex::isOK() const
{
	return pimpl_->is.good();
}


void LyXLex::setLineNo(int l)
{
	pimpl_->lineno = l;
}


int LyXLex::getLineNo() const
{
	return pimpl_->lineno;
}


string const LyXLex::text() const
{
	return &pimpl_->buff[0];
}


istream & LyXLex::getStream()
{
	return pimpl_->is;
}


void LyXLex::pushTable(keyword_item * tab, int num)
{
	pimpl_->pushTable(tab, num);
}


void LyXLex::popTable()
{
	pimpl_->popTable();
}


void LyXLex::printTable(ostream & os)
{
	pimpl_->printTable(os);
}


void LyXLex::printError(string const & message) const
{
	pimpl_->printError(message);
}


bool LyXLex::setFile(string const & filename)
{
	return pimpl_->setFile(filename);
}


void LyXLex::setStream(istream & i)
{
	pimpl_->setStream(i);
}


void LyXLex::setCommentChar(char c)
{
	pimpl_->setCommentChar(c);
}

int LyXLex::lex()
{
	return pimpl_->lex();
}


int LyXLex::getInteger() const
{
	if (isStrInt(pimpl_->getString())) {
		return strToInt(pimpl_->getString());
	} else {
		pimpl_->printError("Bad integer `$$Token'");
		return -1;
	}
}


float LyXLex::getFloat() const
{
	// replace comma with dot in case the file was written with
	// the wrong locale (should be rare, but is easy enough to
	// avoid).
	string str = subst(pimpl_->getString(), ",", ".");
	if (isStrDbl(str))
		return strToDbl(str);
	else {
		pimpl_->printError("Bad float `$$Token'");
		return -1;
	}
}


string const LyXLex::getString() const
{
	return pimpl_->getString();
}


// I would prefer to give a tag number instead of an explicit token
// here, but it is not possible because Buffer::readLyXformat2 uses
// explicit tokens (JMarc)
string const LyXLex::getLongString(string const & endtoken)
{
	string str, prefix;
	bool firstline = true;

	while (isOK()) {
		if (!eatLine())
			// blank line in the file being read
			continue;

		string const token = trim(getString(), " \t");

		lyxerr[Debug::PARSER] << "LongString: `"
				      << getString() << '\'' << endl;

		// We do a case independent comparison, like search_kw
		// does.
		if (compare_ascii_no_case(token, endtoken) != 0) {
			string tmpstr = getString();
			if (firstline) {
				unsigned int i = 0;
				while (i < tmpstr.length()
				      && tmpstr[i] == ' ') {
					++i;
					prefix += ' ';
				}
				firstline = false;
				lyxerr[Debug::PARSER] << "Prefix = `" << prefix
						      << '\'' << endl;
			}

			if (!prefix.empty()
			    && prefixIs(tmpstr, prefix)) {
				tmpstr.erase(0, prefix.length() - 1);
			}
			str += ltrim(tmpstr, "\t") + '\n';
		}
		else // token == endtoken
			break;
	}
	if (!isOK()) {
		printError("Long string not ended by `" + endtoken + '\'');
	}

	return str;
}


bool LyXLex::getBool() const
{
	if (compare(pimpl_->buff, "true") == 0) {
		return true;
	} else if (compare(pimpl_->buff, "false") != 0) {
		pimpl_->printError("Bad boolean `$$Token'. Use \"false\" or \"true\"");
	}
	return false;
}


bool LyXLex::eatLine()
{
	return pimpl_->eatLine();
}


bool LyXLex::next(bool esc)
{
	return pimpl_->next(esc);
}


bool LyXLex::nextToken()
{
	return pimpl_->nextToken();
}


void LyXLex::pushToken(string const & pt)
{
	pimpl_->pushToken(pt);
}


int LyXLex::findToken(char const * str[])
{
	if (!next()) {
		pimpl_->printError("file ended while scanning string token");
		return -1;
	}

	int i = 0;

	if (compare(pimpl_->buff, "default")) {
		while (str[i][0] && compare(str[i], pimpl_->buff)) {
			++i;
		}
		if (!str[i][0]) {
			pimpl_->printError("Unknown argument `$$Token'");
			i = -1;
		}
	}

	return i;
}
