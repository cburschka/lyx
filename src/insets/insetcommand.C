/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcommand.h"
#include "debug.h"
#include "frontends/Painter.h"
#include "lyxlex.h"

using std::ostream;
using std::endl;


InsetCommandParams::InsetCommandParams()
{}


InsetCommandParams::InsetCommandParams(string const & n,
					string const & c,
					string const & o)
	: cmdname(n), contents(c), options(o)
{}


string const InsetCommandParams::getAsString() const
{
	return cmdname + "|++|" + contents + "|++|" + options;
}


void InsetCommandParams::setFromString(string const & b)
{
	string::size_type idx = b.find("|++|");
	if (idx == string::npos) {
		cmdname = b;
		contents = "";
		options = "";
		return;
	}

	cmdname = b.substr(0, idx);
	string tmp = b.substr(idx+4);

	idx = tmp.find("|++|");
	if (idx == string::npos) {
		contents = tmp;
		options = "";
	} else {
		contents  = tmp.substr(0, idx);
		options = tmp.substr(idx+4);
	}
}


bool InsetCommandParams::operator==(InsetCommandParams const & o) const
{
	return cmdname == o.cmdname && contents == o.contents
		&& options == o.options;
}


bool InsetCommandParams::operator!=(InsetCommandParams const & o) const
{
	return !(*this == o);
}


void InsetCommandParams::scanCommand(string const & cmd)
{
	string tcmdname, toptions, tcontents;

	if (cmd.empty()) return;

	enum { WS, CMDNAME, OPTION, CONTENT } state = WS;

	// Used to handle things like \command[foo[bar]]{foo{bar}}
	int nestdepth = 0;

	for (string::size_type i = 0; i < cmd.length(); ++i) {
		char c = cmd[i];
		if ((state == CMDNAME && c == ' ') ||
		    (state == CMDNAME && c == '[') ||
		    (state == CMDNAME && c == '{')) {
			state = WS;
		}
		if ((state == OPTION  && c == ']') ||
		    (state == CONTENT && c == '}')) {
			if (nestdepth == 0) {
				state = WS;
			} else {
				--nestdepth;
			}
		}
		if ((state == OPTION  && c == '[') ||
		    (state == CONTENT && c == '{')) {
			++nestdepth;
		}
		switch (state) {
		case CMDNAME:	tcmdname += c; break;
		case OPTION:	toptions += c; break;
		case CONTENT:	tcontents += c; break;
		case WS:
			if (c == '\\') {
				state = CMDNAME;
			} else if (c == '[') {
				state = OPTION;
				nestdepth = 0; // Just to be sure
			} else if (c == '{') {
				state = CONTENT;
				nestdepth = 0; // Just to be sure
			}
			break;
		}
	}

	// Don't mess with this.
	if (!tcmdname.empty())  setCmdName(tcmdname);
	if (!toptions.empty())  setOptions(toptions);
	if (!tcontents.empty()) setContents(tcontents);

	if (lyxerr.debugging(Debug::PARSER))
		lyxerr << "Command <" <<  cmd
		       << "> == <" << getCommand()
		       << "> == <" << getCmdName()
		       << '|' << getContents()
		       << '|' << getOptions() << '>' << endl;
}


// This function will not be necessary when lyx3
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
		lex.nextToken();
		token = lex.getString();
		if (token == "\\end_inset")
			break;
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
	if (!getCmdName().empty()) s += "\\"+getCmdName();
	if (!getOptions().empty()) s += "["+getOptions()+']';
	s += "{"+getContents()+'}';
	return s;
}


InsetCommand::InsetCommand(InsetCommandParams const & p, bool)
	: p_(p.getCmdName(), p.getContents(), p.getOptions())
{}


void InsetCommand::setParams(InsetCommandParams const & p)
{
	p_.setCmdName(p.getCmdName());
	p_.setContents(p.getContents());
	p_.setOptions(p.getOptions());
}


int InsetCommand::latex(Buffer const *, ostream & os,
			bool /*fragile*/, bool/*fs*/) const
{
	os << getCommand();
	return 0;
}


int InsetCommand::ascii(Buffer const *, ostream &, int) const
{
	return 0;
}


int InsetCommand::linuxdoc(Buffer const *, ostream &) const
{
	return 0;
}


int InsetCommand::docbook(Buffer const *, ostream &, bool) const
{
	return 0;
}
