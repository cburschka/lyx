/**
 * \file insetcommandparams.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetcommandparams.h"

#include "debug.h"
#include "lyxlex.h"


using std::string;
using std::endl;
using std::ostream;


InsetCommandParams::InsetCommandParams()
{}


InsetCommandParams::InsetCommandParams(string const & n,
					string const & c,
					string const & o,
					string const & s)
	: cmdname(n), contents(c), options(o), sec_options(s),
	preview_(false)
{}


void InsetCommandParams::scanCommand(string const & cmd)
{
	string tcmdname, toptions, tsecoptions, tcontents;

	if (cmd.empty()) return;

	enum { WS, CMDNAME, OPTION, SECOPTION, CONTENT } state = WS;

	// Used to handle things like \command[foo[bar]]{foo{bar}}
	int nestdepth = 0;

	for (string::size_type i = 0; i < cmd.length(); ++i) {
		char const c = cmd[i];
		if ((state == CMDNAME && c == ' ') ||
		    (state == CMDNAME && c == '[') ||
		    (state == CMDNAME && c == '{')) {
			state = WS;
		}
		if ((state == OPTION  && c == ']') ||
		    (state == SECOPTION  && c == ']') ||
		    (state == CONTENT && c == '}')) {
			if (nestdepth == 0) {
				state = WS;
			} else {
				--nestdepth;
			}
		}
		if ((state == OPTION  && c == '[') ||
		    (state == SECOPTION  && c == '[') ||
		    (state == CONTENT && c == '{')) {
			++nestdepth;
		}
		switch (state) {
		case CMDNAME:	tcmdname += c; break;
		case OPTION:	toptions += c; break;
		case SECOPTION:	tsecoptions += c; break;
		case CONTENT:	tcontents += c; break;
		case WS: {
			char const b = i? cmd[i-1]: 0;
			if (c == '\\') {
				state = CMDNAME;
			} else if (c == '[' && b != ']') {
				state = OPTION;
				nestdepth = 0; // Just to be sure
			} else if (c == '[' && b == ']') {
				state = SECOPTION;
				nestdepth = 0; // Just to be sure
			} else if (c == '{') {
				state = CONTENT;
				nestdepth = 0; // Just to be sure
			}
			break;
		}
		}
	}

	// Don't mess with this.
	if (!tcmdname.empty())  setCmdName(tcmdname);
	if (!toptions.empty())  setOptions(toptions);
	if (!tsecoptions.empty())  setSecOptions(tsecoptions);
	if (!tcontents.empty()) setContents(tcontents);

	if (lyxerr.debugging(Debug::PARSER))
		lyxerr << "Command <" <<  cmd
		       << "> == <" << getCommand()
		       << "> == <" << getCmdName()
		       << '|' << getContents()
		       << '|' << getOptions()
		       << '|' << getSecOptions() << '>' << endl;
}


void InsetCommandParams::read(LyXLex & lex)
{
	string token;

	if (lex.eatLine()) {
		token = lex.getString();
		scanCommand(token);
	} else {
		lex.printError("InsetCommand: Parse error: `$$Token'");
	}

	while (lex.isOK()) {
		lex.next();
		token = lex.getString();
		if (token == "\\end_inset")
			break;
		if (token == "preview") {
			lex.next();
			preview_ = lex.getBool();
		}
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
	}
}


void InsetCommandParams::write(ostream & os) const
{
	os << "LatexCommand " << getCommand() << "\n";
}


string const InsetCommandParams::getCommand() const
{
	string s;
	if (!getCmdName().empty()) s += '\\' + getCmdName();
	if (!getOptions().empty()) s += '[' + getOptions() + ']';
	if (!getSecOptions().empty()) {
		// for cases like \command[][sec_option]{arg}
		if (getOptions().empty()) s += "[]";
	s += '[' + getSecOptions() + ']';
	}
	s += '{' + getContents() + '}';
	return s;
}


bool operator==(InsetCommandParams const & o1,
		InsetCommandParams const & o2)
{
	return o1.getCmdName() == o2.getCmdName()
		&& o1.getContents() == o2.getContents()
		&& o1.getOptions() == o2.getOptions()
		&& o1.getSecOptions() == o2.getSecOptions()
		&& o1.preview() == o2.preview();
}


bool operator!=(InsetCommandParams const & o1,
		InsetCommandParams const & o2)
{
	return !(o1 == o2);
}
