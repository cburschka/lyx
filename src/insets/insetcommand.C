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


InsetCommand::InsetCommand()
{
}


InsetCommand::InsetCommand(string const & cmd, string const & arg, 
			   string const & opt)
	: cmdname(cmd), options(opt), contents(arg)
{
}


int InsetCommand::ascent(Painter & pain, LyXFont const &) const
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	
	int width, ascent, descent;
        string s = getScreenLabel();
	
        if (Editable()) {
		pain.buttonText(0, 0, s, font,
				false, width, ascent, descent);
	} else {
		pain.rectText(0, 0, s, font,
			      LColor::commandbg, LColor::commandframe,
			      false, width, ascent, descent);
	}
	return ascent;
}


int InsetCommand::descent(Painter & pain, LyXFont const &) const
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	
	int width, ascent, descent;
        string s = getScreenLabel();
	
        if (Editable()) {
		pain.buttonText(0, 0, s, font,
				false, width, ascent, descent);
	} else {
		pain.rectText(0, 0, s, font,
			      LColor::commandbg, LColor::commandframe,
			      false, width, ascent, descent);
	}
	return descent;
}


int InsetCommand::width(Painter & pain, LyXFont const &) const
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	
	int width, ascent, descent;
        string s = getScreenLabel();
	
        if (Editable()) {
		pain.buttonText(0, 0, s, font,
				false, width, ascent, descent);
	} else {
		pain.rectText(0, 0, s, font,
			      LColor::commandbg, LColor::commandframe,
			      false, width, ascent, descent);
	}
	return width + 4;
}


void InsetCommand::draw(Painter & pain, LyXFont const &,
			int baseline, float & x) const
{
	// Draw it as a box with the LaTeX text
	LyXFont font(LyXFont::ALL_SANE);
	font.setColor(LColor::command).decSize();

	int width;
	string s = getScreenLabel();

	if (Editable()) {
		pain.buttonText(int(x)+2, baseline, s, font, true, width);
	} else {
		pain.rectText(int(x)+2, baseline, s, font,
			      LColor::commandbg, LColor::commandframe,
			      true, width);
	}

	x += width + 4;
}


// In lyxf3 this will be just LaTeX
void InsetCommand::Write(ostream & os) const
{
	os << "LatexCommand " << getCommand() << "\n";
}


void InsetCommand::scanCommand(string const & cmd)
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
	if (!tcmdname.empty()) cmdname = tcmdname;
	if (!toptions.empty()) options = toptions;
	if (!tcontents.empty()) setContents(tcontents); 
                	// setContents is overloaded in InsetInclude

	if (lyxerr.debugging(Debug::PARSER))
		lyxerr << "Command <" <<  cmd
		       << "> == <" << getCommand()
		       << "> == <" << getCmdName()
		       << '|' << getContents()
		       << '|' << getOptions() << '>' << endl;
}


// This function will not be necessary when lyx3
void InsetCommand::Read(LyXLex & lex)
{    
	if (lex.EatLine()) {
		string t = lex.GetString();
		scanCommand(t);
	} else
		lex.printError("InsetCommand: Parse error: `$$Token'");
}


int InsetCommand::Latex(ostream & os, bool /*fragile*/, bool/*fs*/) const
{
	os << getCommand();
	return 0;
}


int InsetCommand::Ascii(ostream &) const
{
	return 0;
}


int InsetCommand::Linuxdoc(ostream &) const
{
	return 0;
}


int InsetCommand::DocBook(ostream &) const
{
	return 0;
}


Inset * InsetCommand::Clone() const
{
	return new InsetCommand(cmdname, contents, options);
}


string InsetCommand::getCommand() const
{	
	string s;
	if (!cmdname.empty()) s += "\\"+cmdname;
	if (!options.empty()) s += "["+options+']';
	s += "{"+contents+'}';
	return s;
}
