/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 the LyX Team.
 *
 * ====================================================== */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "insets/insetgraphics.h"
#include "Painter.h"
#include "form_graphics.h"
#include "lyx_gui_misc.h"
#include "filedlg.h"
#include "support/FileInfo.h"
#include "support/filetools.h"

using std::ostream;
using std::endl;

extern string system_lyxdir;
extern string user_lyxdir;
extern string system_tempdir;

string browseFile();

extern "C" void GraphicxCB(FL_OBJECT * obj, long arg) 
{
	lyxerr << "GraphicxCB: obj = " << obj << " arg = " << arg << endl;
	switch (arg) {
	case 0: // The graphics file
		lyxerr << "Set the graphics file in InsetGraphics" << endl;
		break;
	case 1: // The file browser
		browseFile();
		break;
	case 2: // The Apply button
		lyxerr << "Scan the form and set the "
			"InsetGraphics accordingly." << endl;
		break;
	case 3: // The OK button
		GraphicxCB(obj, 2); // do the apply
		GraphicxCB(obj, 4); // do the cancel
		break;
	case 4: // The Cancel button
		lyxerr << "Just hide the form and do nothing else!" << endl;
		break;
	case 99:
		lyxerr << "Not implemented yet..." << endl;
		break;
	default:
		lyxerr << "Unknown callback value!" << endl;
		break;
	}
}


string browseFile()
{
	// This function is probably not good enough yet, and does need some
	// arguemnts to tell what dir to start looking in.
	
	static string current_figure_path = ".";

	LyXFileDlg fileDlg;

	// Does user clipart directory exist?
	string bufclip = AddName (user_lyxdir, "clipart");	
	FileInfo fileInfo(bufclip);
	if (!(fileInfo.isOK() && fileInfo.isDir()))
		// No - bail out to system clipart directory
		bufclip = AddName (system_lyxdir, "clipart");	


	fileDlg.SetButton(0, _("Clipart"), bufclip); 

	bool error = false;
	string buf;
	do {
		string p = fileDlg.Select(_("Graphics"),
				   current_figure_path,
				   "*ps", string());

		if (p.empty()) return p;

		current_figure_path = OnlyPath(p);

		if (p.find_first_of("#~$% ") != string::npos) {
			WriteAlert(_("Filename can't contain any "
				     "of these characters:"),
				   // xgettext:no-c-format
				   _("space, '#', '~', '$' or '%'.")); 
			error = true;
		}
	} while (error);

	return buf;
}


InsetGraphics::InsetGraphics()
	: form(0)
{}


int InsetGraphics::ascent(Painter &, LyXFont const &) const 
{
	
	return 100;
}


int InsetGraphics::descent(Painter &, LyXFont const &) const 
{
	// this is not true if viewport is used and clip is not.
	return 1;
}


int InsetGraphics::width(Painter &, LyXFont const &) const 
{
	if (bb.isSet()) {
		return bb.urx - bb.llx;
	}
	return 100;
}


void InsetGraphics::draw(Painter & pain, LyXFont const & font,
			 int baseline, float & x) const
{
	// This will draw the graphics. As for now we only draw a
	// placeholder rectangele.
	pain.rectangle(x, baseline - ascent(pain, font),
		       width(pain, font),
		       ascent(pain, font) + descent(pain, font));
}


void InsetGraphics::Edit(BufferView *, int, int, unsigned int)
{
	lyxerr.debug() << "InsetGraphics::Edit" << endl;

	if (!form) {
		form = create_form_Graphics();
		fl_set_form_atclose(form->Graphics, CancelCloseBoxCB, 0);
		fl_set_object_return(form->Angle, FL_RETURN_ALWAYS);
		fl_set_object_return(form->Width, FL_RETURN_ALWAYS);
		fl_set_object_return(form->Height, FL_RETURN_ALWAYS);
	}

	if (form->Graphics->visible) {
		fl_raise_form(form->Graphics);
	} else {
		fl_show_form(form->Graphics, FL_PLACE_MOUSE | FL_PLACE_SIZE,
			     FL_FULLBORDER, _("Graphics"));
	}
}


Inset::EDITABLE InsetGraphics::Editable() const
{
	return IS_EDITABLE;
}


void InsetGraphics::Write(ostream & os) const
{
	// The question on the file format is still open.
	// Suggestions?
	// perhaps a format that is xml-parsable
	//<graphics name="test.eps"/>
	os << "GRAPHICS\n";
}


void InsetGraphics::Read(LyXLex & /*lex*/) 
{
	// For now we only use a static file...
	graphicsfile = "testfile.xpm";
	//graphicscache.addFile(graphicsfile);
	//bb = graphicscache.getBB(graphicsfile);
	//pixmap = graphicscache.getPixmap(graphicsfile);
}


int InsetGraphics::Latex(ostream & os,
			 bool /*fragile*/, bool/*fs*/) const
{
	// MISSING: We have to decide how to do the order of the options
	// that is depentant of order, like witdth, height, andlge. Should
	// we rotate before scale? Should we let the user decide?
	// bool rot_before_scale; ?
	// Nothing to do if we don't have a graphics file
	if (graphicsfile.empty()) return 0;

	// We never used the starred form, we use the "clip" option instead.
	string command("\\insetgraphics");
	
#ifdef HAVE_SSTREAM
	std::ostringstream options;
#else
	ostrstream options;
#endif
	if (bb.isSet() && use_bb) {
		options << "bb="
			<< bb.llx << " " << bb.lly << " "
			<< bb.urx << " " << bb.ury << ",";
	}
	if (hiresbb) {
		options << "hiresbb,";
	}
	if (viewport.isSet()) {
		options << "viewport="
			<< viewport.llx << " " << viewport.lly << " "
			<< viewport.urx << " " << viewport.ury << ",";
	}
	if (trim.isSet()) {
		options << "trim="
			<< trim.llx << " " << trim.lly << " "
			<< trim.urx << " " << trim.ury << ",";
	}
	if (natheight.value() == 0) {
		options << "natheight=" << natheight.asString() << ",";
	}
	if (natwidth.value() == 0) {
		options << "natwidth=" << natwidth.asString() << ",";
	}
	if (angle != 0.0) {
		options << "angle=" << angle << ",";
	}
	if (origin != DEFAULT) {
		switch(origin) {
		case DEFAULT: break;
		case LEFTTOP:
			options << "origin=lt,";
			break;
		case LEFTCENTER:
			options << "origin=lc,";
			break;
		case LEFTBASELINE:
			options << "origin=lB,";
			break;
		case LEFTBOTTOM:
			options << "origin=lb,";
			break;
		case CENTERTOP:
			options << "origin=ct,";
			break;
		case CENTER:
			options << "origin=c,";
			break;
		case CENTERBASELINE:
			options << "origin=cB,";
			break;
		case CENTERBOTTOM:
			options << "origin=cb,";
			break;
		case RIGHTTOP:
			options << "origin=rt,";
			break;
		case RIGHTCENTER:
			options << "origin=rc,";
			break;
		case RIGHTBASELINE:
			options << "origin=rB,";
			break;
		case RIGHTBOTTOM:
			options << "origin=rb,";
			break;
		}
	}
	if (g_width.value() != 0) {
		options << "width=" << g_width.asString() << ",";
	}
	if (g_height.value() != 0) {
		options << "height=" << g_height.asString() << ",";
	}
	if (totalheight.value() != 0) {
		options << "totalheight=" << totalheight.asString() << ",";
	}
	if (keepaspectratio) {
		options << "keepaspectratio,";
	}
	if (scale != 0.0) {
		options << "scale=" << scale << ",";
	}
	if (clip) {
		options << "clip,";
	}
	if (draft) {
		options << "draft,";
	}
	if (!type.empty()) {
		options << "type=" << type << ",";
	}
	if (!ext.empty()) {
		options << "ext=" << type << ",";
	}
	if (!read.empty()) {
		options << "read=" << type << ",";
	}
	if (!command.empty()) {
		options << "command=" << type << ",";
	}
#ifdef HAVE_SSTREAM
	string opts(options.str().c_str());
#else
	options << '\0';
	char * tmp = options.str();
	string opts(tmp);
	delete [] tmp;
#endif
	opts = strip(opts, ',');
	if (!opts.empty()) {
		command += "[";
		command += opts;
		command += "]";
	}
	command += "{";
	command += graphicsfile;
	command += "}";

	os << command << '\n';
	
	return 1;
}


int InsetGraphics::Ascii(ostream &) const
{
	return 0;
}


int InsetGraphics::Linuxdoc(ostream &) const
{
	return 0;
}


int InsetGraphics::DocBook(ostream &) const
{
	return 0;
}


void InsetGraphics::Validate(LaTeXFeatures & /*features*/) const
{
	//features.graphicx = true;
}


Inset * InsetGraphics::Clone() const
{
	return new InsetGraphics;
}


