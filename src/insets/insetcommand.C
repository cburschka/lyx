/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcommand.h"
#include "debug.h"
#include "Painter.h"

using std::ostream;
using std::endl;


InsetCommandParams::InsetCommandParams()
{}


InsetCommandParams::InsetCommandParams( string const & n,
					string const & c,
					string const & o )
	: cmdname(n), contents(c), options(o)
{}


string InsetCommandParams::getAsString() const
{
	string b(cmdname);
	b += "|++|" + options + "|++|" + contents;
	return b;
}


void InsetCommandParams::setFromString( string const & b )
{
	string::size_type idx = b.find("|++|");
	if( idx == string::npos ) return;

	cmdname = b.substr(0, idx);
	string tmp = b.substr(idx+4);

	idx = tmp.find("|++|");
	if( idx == string::npos ) {
		options = tmp;
	} else {
		options  = tmp.substr(0, idx);
		contents = tmp.substr(idx+4);
	}
}


bool InsetCommandParams::operator==(InsetCommandParams const & o) const
{
	if( cmdname  != o.cmdname )  return false;
	if( contents != o.contents ) return false;
	if( options  != o.options )  return false;
	return true;
}


bool InsetCommandParams::operator!=(InsetCommandParams const & o) const
{
	if( cmdname  != o.cmdname )  return true;
	if( contents != o.contents ) return true;
	if( options  != o.options )  return true;
	return false;
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
		case OPTION: 	toptions += c; break;
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
	if (!tcmdname.empty())  setCmdName( tcmdname );
	if (!toptions.empty())  setOptions( toptions );
	if (!tcontents.empty()) setContents( tcontents ); 

	if (lyxerr.debugging(Debug::PARSER))
		lyxerr << "Command <" <<  cmd
		       << "> == <" << getCommand()
		       << "> == <" << getCmdName()
		       << '|' << getContents()
		       << '|' << getOptions() << '>' << endl;
}


// This function will not be necessary when lyx3
void InsetCommandParams::Read(LyXLex & lex)
{    
	string token;

	if (lex.EatLine()) {
		token = lex.GetString();
		scanCommand(token);
	} else
		lex.printError("InsetCommand: Parse error: `$$Token'");
	while (lex.IsOK()) {
		lex.nextToken();
		token = lex.GetString();
		if (token == "\\end_inset")
			break;
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
	}
}


void InsetCommandParams::Write(ostream & os) const
{
	os << "LatexCommand " << getCommand() << "\n";
}


string InsetCommandParams::getCommand() const
{	
	string s;
	if (!getCmdName().empty()) s += "\\"+getCmdName();
	if (!getOptions().empty()) s += "["+getOptions()+']';
	s += "{"+getContents()+'}';
	return s;
}


InsetCommand::InsetCommand( InsetCommandParams const & p )
	: p_( p.getCmdName(), p.getContents(), p.getOptions() )
{}


void InsetCommand::setParams(InsetCommandParams const & p )
{
	p_.setCmdName( p.getCmdName() );
	p_.setContents( p.getContents() );
	p_.setOptions( p.getOptions() );
}


int InsetCommand::Latex(Buffer const *, ostream & os,
			bool /*fragile*/, bool/*fs*/) const
{
	os << getCommand();
	return 0;
}


int InsetCommand::Ascii(Buffer const *, ostream &) const
{
	return 0;
}


int InsetCommand::Linuxdoc(Buffer const *, ostream &) const
{
	return 0;
}


int InsetCommand::DocBook(Buffer const *, ostream &) const
{
	return 0;
}
