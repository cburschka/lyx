/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcommand.h"
#include "lyxdraw.h"
#include "debug.h"

InsetCommand::InsetCommand()
{
}


InsetCommand::InsetCommand(string const & cmd, string const & arg, 
			   string const & opt)
	: command(cmd), options(opt), contents(arg)
{
}


int InsetCommand::Ascent(LyXFont const & font) const
{
	LyXFont f = font;
	f.decSize();
	return f.maxAscent() + 3;
}


int InsetCommand::Descent(LyXFont const & font) const
{
	LyXFont f = font;
	f.decSize();
	return f.maxDescent() + 3;
}


int InsetCommand::Width(LyXFont const & font) const
{
	LyXFont f = font;
	f.decSize();
	string s = getScreenLabel();
	return 10 + f.stringWidth(s);
}


void InsetCommand::Draw(LyXFont font, LyXScreen & scr,
		      int baseline, float & x)
{
	// Draw it as a box with the LaTeX text
	x += 3;

	scr.fillRectangle(gc_lighted,
			  int(x), baseline - Ascent(font) + 1,
			  Width(font) - 6,
			  Ascent(font) + Descent(font)-2);
        // Tell whether this slows down the drawing  (ale)
	// lets draw editable and non-editable insets differently
        if (Editable()) {
		int y = baseline - Ascent(font)+1, w = Width(font)-6,
			h = (Ascent(font)+Descent(font)-2);
		scr.drawFrame(FL_UP_FRAME, int(x), y, w, h, FL_BLACK, -1);
	} else {
		scr.drawRectangle(gc_note_frame,
				  int(x), baseline - Ascent(font)+1,
				  Width(font)-6,
				  Ascent(font)+Descent(font)-2); 
	}
        string s = getScreenLabel();
       	LyXFont f = font;
	f.decSize();
	f.setColor(LyXFont::NONE);
	f.setLatex(LyXFont::OFF);
	scr.drawString(f, s, baseline, int(x+2));

	x +=  Width(font) - 3;
}


// In lyxf3 this will be just LaTeX
void InsetCommand::Write(FILE * file)
{
	fprintf(file, "LatexCommand %s\n", getCommand().c_str());
}


void InsetCommand::scanCommand(string const & cmd)
{
	string tcommand, toptions, tcontents;

	if (cmd.empty()) return;

	enum { WS, Command, Option, Content } state = WS;
	
	// Used to handle things like \command[foo[bar]]{foo{bar}}
	int nestdepth = 0;

	for (string::size_type i = 0; i < cmd.length(); ++i) {
		char c = cmd[i];
		if ((state == Command && c == ' ') ||
		    (state == Command && c == '[') ||
		    (state == Command && c == '{')) {
			state = WS;
		}
		if ((state == Option  && c == ']') ||
		    (state == Content && c == '}')) {
			if (nestdepth == 0) {
				state = WS;
			} else {
				nestdepth--;
			}
		}
		if ((state == Option  && c == '[') ||
		    (state == Content && c == '{')) {
		    	nestdepth++;
		}
		switch (state) {
		case Command:	tcommand += c; break;
		case Option: 	toptions += c; break;
		case Content:	tcontents += c; break;
		case WS:
			if (c == '\\') {
				state = Command;
			} else if (c == '[') {
				state = Option;
				nestdepth = 0; // Just to be sure
			} else if (c == '{') {
				state = Content;
				nestdepth = 0; // Just to be sure
			}
			break;
		}
	}

	// Don't mess with this.
	if (!tcommand.empty()) command = tcommand;
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


int InsetCommand::Latex(FILE * file, signed char /*fragile*/)
{
	fprintf(file, "%s", getCommand().c_str());
	return 0;
}


int InsetCommand::Latex(string & file, signed char /*fragile*/)
{
	file += getCommand();
	return 0;
}


int InsetCommand::Linuxdoc(string &/*file*/)
{
	return 0;
}


int InsetCommand::DocBook(string &/*file*/)
{
	return 0;
}


Inset * InsetCommand::Clone() const
{
	return new InsetCommand(command, contents, options);
}


string InsetCommand::getCommand() const
{	
	string s;
	if (!command.empty()) s += "\\"+command;
	if (!options.empty()) s += "["+options+']';
	s += "{"+contents+'}';
	return s;
}
