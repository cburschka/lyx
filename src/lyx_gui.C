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
#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include <fcntl.h>
#include "lyx_gui.h"
#include FORMS_H_LOCATION
#include "support/filetools.h"
#include "combox.h"
#include "lyx.h"
#include "form1.h"
#include "layout_forms.h"
#include "print_form.h"
#include "tex-strings.h"
#include "lyx_main.h"
#include "log_form.h"
#include "debug.h"
#include "version.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxserver.h"
#include "lyxrc.h"
#include "gettext.h"
#include "lyx_gui_misc.h"
#include "lyxlookup.h"
#include "bufferlist.h"
#include "language.h"
#include "ColorHandler.h"
#include "frontends/GUIRunTime.h"

using std::endl;

FD_form_title * fd_form_title;
FD_form_paragraph * fd_form_paragraph;
FD_form_paragraph_extra * fd_form_paragraph_extra;
FD_form_character * fd_form_character;
FD_form_document * fd_form_document;
FD_form_paper * fd_form_paper;
FD_form_table_options * fd_form_table_options;
FD_form_table_extra * fd_form_table_extra;
FD_form_quotes * fd_form_quotes;
FD_form_preamble * fd_form_preamble;
FD_form_table * fd_form_table;
FD_form_sendto * fd_form_sendto;
FD_form_figure * fd_form_figure;
FD_LaTeXLog * fd_latex_log; // from log_form.h
Combox * combo_language;
Combox * combo_language2;

extern LyXServer * lyxserver;
extern bool finished;	// flag, that we are quitting the program
extern BufferList bufferlist;
extern GUIRunTime guiruntime;

FL_CMD_OPT cmdopt[] =
{
	{"-geometry", "*.geometry", XrmoptionSepArg, "690x510"}
};

static int width  = 690;
static int height = 510;
static int xpos   = -1;
static int ypos   = -1;
static char geometry[40];
bool	   cursor_follows_scrollbar;


FL_resource res[] =
{
	{"geometry", "geometryClass", FL_STRING, geometry, "", 40}
};


extern "C"
int LyX_XErrHandler(Display * display, XErrorEvent * xeev)
{
//#warning Please see if you can trigger this!
	// emergency save
	if (!bufferlist.empty())
		bufferlist.emergencyWriteAll();

	// Get the reason for the crash.
	char etxt[513];
	XGetErrorText(display, xeev->error_code, etxt, 512);
	lyxerr << etxt << endl;
	// By doing an abort we get a nice backtrace. (hopefully)
	lyx::abort();
	return 0; // Solaris CC wants us to return something
}


LyXGUI::LyXGUI(LyX * owner, int * argc, char * argv[], bool GUI)
	: _owner(owner), lyxViews(0)
{
	gui = GUI;
	if (!gui)
		return;

	// 
	setDefaults();
	
	static const int num_res = sizeof(res)/sizeof(FL_resource);
	fl_initialize(argc, argv, "LyX", cmdopt, num_res);
	fl_get_app_resources(res, num_res);

	static const int geometryBitmask =
		XParseGeometry( geometry,
				&xpos,
				&ypos,
				reinterpret_cast<unsigned int *>(&width),
				reinterpret_cast<unsigned int *>(&height));

	Display * display = fl_get_display();
	if (!display) {
		lyxerr << "LyX: unable to access X display, exiting" << endl;
		exit(1);
	}
	fcntl(ConnectionNumber(display), F_SETFD, FD_CLOEXEC);
	// X Error handler install goes here
	XSetErrorHandler(LyX_XErrHandler);

	// A width less than 590 pops up an awkward main window
	// The minimal values of width/height (590/400) are defined in
	// src/lyx.C  
        if (width < 590) width = 590;
	if (height < 400) height = 400;
	
	// If width is not set by geometry, check it against monitor width
	if ( !(geometryBitmask & 4) ) {
		Screen * scr = DefaultScreenOfDisplay(fl_get_display());
		if (WidthOfScreen(scr) - 8 < width)
			width = WidthOfScreen(scr) - 8;
	}

	// If height is not set by geometry, check it against monitor height
	if ( !(geometryBitmask & 8) ) {
		Screen * scr = DefaultScreenOfDisplay(fl_get_display());
		if (HeightOfScreen(scr) - 24 < height)
			height = HeightOfScreen(scr) - 24;
	}

	// Recalculate xpos if it's negative
	if (geometryBitmask & 16)
		xpos += WidthOfScreen(DefaultScreenOfDisplay(fl_get_display())) - width;

	// Recalculate ypos if it's negative
	if (geometryBitmask & 32)
		ypos += HeightOfScreen(DefaultScreenOfDisplay(fl_get_display())) - height;

	// Initialize the LyXColorHandler
	lyxColorHandler = new LyXColorHandler;
}


// A destructor is always necessary  (asierra-970604)
LyXGUI::~LyXGUI()
{
        // Lyxserver was created in this class so should be destroyed
        // here.  asierra-970604
	delete lyxserver;
	lyxserver = 0;
	delete lyxViews;

	CloseLyXLookup();
}


void LyXGUI::setDefaults()
{
	GUIRunTime::setDefaults();
}


// This is called after we have parsed lyxrc
void LyXGUI::init()
{
	if (!gui)
		return;

	create_forms();

	if (lyxrc.font_norm_menu.empty())
		lyxrc.font_norm_menu = lyxrc.font_norm;
	// Set the font name for popups and menus
        string menufontname = lyxrc.menu_font_name 
		               + "-*-*-*-?-*-*-*-*-"  
		               + lyxrc.font_norm_menu;
		// "?" means "scale that font"
        string popupfontname = lyxrc.popup_font_name 
		               + "-*-*-*-?-*-*-*-*-"  
		               + lyxrc.font_norm_menu;

	int bold = fl_set_font_name(FL_BOLD_STYLE, menufontname.c_str());
	int normal = fl_set_font_name(FL_NORMAL_STYLE, popupfontname.c_str());
        if (bold < 0)
                lyxerr << "Could not set menu font to "
		       << menufontname << endl;

        if (normal < 0)
                lyxerr << "Could not set popup font to "
		       << popupfontname << endl;

	if (bold < 0 && normal < 0) {
		lyxerr << "Using 'helvetica' font for menus" << endl;
		bold = fl_set_font_name(FL_BOLD_STYLE,
					"-*-helvetica-bold-r-*-*-*-?-*-*-*-*-iso8859-1");
		normal = fl_set_font_name(FL_NORMAL_STYLE,
					  "-*-helvetica-medium-r-*-*-*-?-*-*-*-*-iso8859-1");
		if (bold < 0 && normal < 0) {
			lyxerr << "Could not find helvetica font. Using 'fixed'." << endl;
			normal = fl_set_font_name(FL_NORMAL_STYLE, "fixed");
		}
	}

 	// put here (after fl_initialize) to avoid segfault. Cannot be done
	// in setDefaults() (Matthias 140496)
	// Moved from ::LyXGUI to ::init to allow popup font customization 
	// (petr 120997).
 	fl_setpup_fontstyle(FL_NORMAL_STYLE);
 	fl_setpup_fontsize(FL_NORMAL_SIZE);
 	fl_setpup_color(FL_MCOL, FL_BLACK);
	fl_set_goodies_font(FL_NORMAL_STYLE, FL_NORMAL_SIZE);

        // all lyxrc settings has to be done here as lyxrc has not yet
        // been read when the GUI is created (Jug)

	// the sendto form
        if (!lyxrc.custom_export_command.empty())
                fl_set_input(fd_form_sendto->input_cmd,
                             lyxrc.custom_export_command.c_str());
	if (lyxrc.custom_export_format == "lyx")
		fl_set_button(fd_form_sendto->radio_ftype_lyx, 1);
	else if (lyxrc.custom_export_format == "tex")
		fl_set_button(fd_form_sendto->radio_ftype_latex, 1);
	else if (lyxrc.custom_export_format == "dvi")
		fl_set_button(fd_form_sendto->radio_ftype_dvi, 1);
	else if (lyxrc.custom_export_format == "ps")
		fl_set_button(fd_form_sendto->radio_ftype_ps, 1);
        else if (lyxrc.custom_export_format == "ascii")
                fl_set_button(fd_form_sendto->radio_ftype_ascii, 1);

	// Update parameters.
	lyxViews->redraw();

	// Initialize the views.
	lyxViews->init();

	// in 0.12 the initialisation of the LyXServer must be done here
	// 0.13 it should be moved again...
	lyxserver = new LyXServer(lyxViews->getLyXFunc(), lyxrc.lyxpipes);
}


void LyXGUI::create_forms()
{
	lyxerr[Debug::INIT] << "Initializing LyXView..." << endl;
	lyxViews = new LyXView(width, height);
	lyxerr[Debug::INIT] << "Initializing LyXView...done" << endl;

	// From here down should be done by somebody else. (Lgb)

	//
	// Create forms
	//

	// the title form
	string banner_file = LibFileSearch("images", "banner", "xpm");
	if (lyxrc.show_banner && !banner_file.empty()) {
		fd_form_title = create_form_form_title();
		fl_set_form_dblbuffer(fd_form_title->form_title, 1); // use dbl buffer
		fl_set_form_atclose(fd_form_title->form_title, CancelCloseBoxCB, 0);
		fl_addto_form(fd_form_title->form_title);
		FL_OBJECT *obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 0, 0, 425, 290, "");
		fl_set_pixmapbutton_file(obj, banner_file.c_str());
		
		fl_set_pixmapbutton_focus_outline(obj, 3);
		fl_set_button_shortcut(obj, "^M ^[", 1);
		fl_set_object_boxtype(obj, FL_NO_BOX);
		fl_set_object_callback(obj, TimerCB, 0);
		
		obj = fl_add_text(FL_NORMAL_TEXT, 248, 265, 170, 16, LYX_VERSION);
		fl_set_object_lsize(obj, FL_NORMAL_SIZE);
		fl_mapcolor(FL_FREE_COL2, 0x05, 0x2e, 0x4c);
		fl_mapcolor(FL_FREE_COL3, 0xe1, 0xd2, 0x9b);
		fl_set_object_color(obj, FL_FREE_COL2, FL_FREE_COL2);
		fl_set_object_lcol(obj, FL_FREE_COL3);
		fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
		fl_set_object_lstyle(obj, FL_BOLD_STYLE);
		fl_end_form();
	}
	
	// the paragraph form
	fd_form_paragraph = create_form_form_paragraph();
	fl_set_form_atclose(fd_form_paragraph->form_paragraph,
			    CancelCloseBoxCB, 0);
	fl_addto_choice(fd_form_paragraph->choice_space_above,
			_(" None | Defskip | Smallskip "
			"| Medskip | Bigskip | VFill | Length "));
	fl_addto_choice(fd_form_paragraph->choice_space_below,
			_(" None | Defskip | Smallskip "
			"| Medskip | Bigskip | VFill | Length ")); 
	fl_set_input_return(fd_form_paragraph->input_space_above,
			    FL_RETURN_ALWAYS);
	fl_set_input_return(fd_form_paragraph->input_space_below,
			    FL_RETURN_ALWAYS);

        // the paragraph extra form
	fd_form_paragraph_extra = create_form_form_paragraph_extra();
	fl_set_form_atclose(fd_form_paragraph_extra->form_paragraph_extra,
			    CancelCloseBoxCB, 0);
	fl_set_input_return(fd_form_paragraph_extra->input_pextra_width,
			    FL_RETURN_ALWAYS);
	fl_set_input_return(fd_form_paragraph_extra->input_pextra_widthp,
			    FL_RETURN_ALWAYS);
	lyxerr[Debug::INIT] << "Initializing form_paragraph...done" << endl;

	// the character form
	fd_form_character = create_form_form_character();
	fl_set_form_atclose(fd_form_character->form_character,
			    CancelCloseBoxCB, 0);
	fl_addto_choice(fd_form_character->choice_family, 
			_(" No change %l| Roman | Sans Serif | Typewriter %l| Reset "));
	fl_addto_choice(fd_form_character->choice_series, 
			_(" No change %l| Medium | Bold %l| Reset "));
	fl_addto_choice(fd_form_character->choice_shape,
			_(" No change %l| Upright | Italic | Slanted | Small Caps "
			"%l| Reset "));
	fl_addto_choice(fd_form_character->choice_size, 
			_(" No change %l| Tiny | Smallest | Smaller | Small "
			"| Normal | Large | Larger | Largest | Huge | Huger "
			"%l| Increase | Decrease | Reset "));
	fl_addto_choice(fd_form_character->choice_bar, 
			_(" No change %l| Emph | Underbar | Noun | LaTeX mode %l| Reset "));
	fl_addto_choice(fd_form_character->choice_color, 
			_(" No change %l| No color | Black | White | Red | Green "
			"| Blue | Cyan | Magenta | Yellow %l| Reset "));
	fl_set_form_minsize(fd_form_character->form_character,
			    fd_form_character->form_character->w,
			    fd_form_character->form_character->h);
	lyxerr[Debug::INIT] << "Initializing form_character::combox..." << endl;
	fl_addto_form(fd_form_character->form_character);
	combo_language2 = new Combox(FL_COMBOX_DROPLIST);
	FL_OBJECT * ob = fd_form_character->choice_language;
	combo_language2->add(ob->x, ob->y, ob->w, ob->h, 250);
	combo_language2->shortcut("#L", 1);
	fl_end_form();
	lyxerr[Debug::INIT] << "Initializing form_character...done" << endl;

	// build up the combox entries
	combo_language2->addto(_("No change"));
	combo_language2->addto(_("Reset"));
	for(Languages::const_iterator cit = languages.begin();
	    cit != languages.end(); ++cit) {
	    if ((*cit).second.lang() != "default")
		combo_language2->addto((*cit).second.lang().c_str());
	}
	combo_language2->select_text(_("No change"));

#ifdef USE_OLD_DOCUMENT_LAYOUT
	// the document form
	fd_form_document = create_form_form_document();
	fl_set_form_atclose(fd_form_document->form_document,
			    CancelCloseBoxCB, 0);
	fl_addto_choice(fd_form_document->choice_spacing,
			_(" Single | OneHalf | Double | Other "));

	fl_set_counter_bounds(fd_form_document->slider_secnumdepth,-2, 5);
	fl_set_counter_bounds(fd_form_document->slider_tocdepth,-1, 5);
	fl_set_counter_step(fd_form_document->slider_secnumdepth, 1, 1);
	fl_set_counter_step(fd_form_document->slider_tocdepth, 1, 1);
	fl_set_counter_precision(fd_form_document->slider_secnumdepth, 0);
	fl_set_counter_precision(fd_form_document->slider_tocdepth, 0);
	fl_addto_form(fd_form_document->form_document);
	combo_language = new Combox(FL_COMBOX_DROPLIST);
	ob = fd_form_document->choice_language;
	combo_language->add(ob->x, ob->y, ob->w, ob->h, 250);
	combo_language->shortcut("#G", 1);
	fl_end_form();

	// "default" is not part of the languages array any more.
	combo_language->addto("default");
	for(Languages::const_iterator cit = languages.begin();
	    cit != languages.end(); ++cit) {
		combo_language->addto((*cit).second.lang().c_str());
	}

	// not really necessary, but we can do it anyway.
	fl_addto_choice(fd_form_document->choice_fontsize, "default|10|11|12");
	int n;
        for (n = 0; tex_fonts[n][0]; ++n) {
	    fl_addto_choice(fd_form_document->choice_fonts, tex_fonts[n]);
	}

	fl_addto_choice(fd_form_document->choice_inputenc,
			"default|auto|latin1|latin2|latin5"
			"|koi8-r|koi8-u|cp866|cp1251|iso88595");

        for (n = 0; tex_graphics[n][0]; ++n) {
	    fl_addto_choice(fd_form_document->choice_postscript_driver,
					tex_graphics[n]);
	}
	// not really necessary, but we can do it anyway.
	fl_addto_choice(fd_form_document->choice_pagestyle,
			"default|empty|plain|headings|fancy");
	fl_addto_choice(fd_form_document->choice_default_skip,
			_(" Smallskip | Medskip | Bigskip | Length "));
	fl_set_input_return(fd_form_document->input_default_skip,
			    FL_RETURN_ALWAYS);
	fl_set_form_minsize(fd_form_document->form_document,
			    fd_form_document->form_document->w,
			    fd_form_document->form_document->h);
	lyxerr[Debug::INIT] << "Initializing form_document...done" << endl;

        // the paper form
	fd_form_paper = create_form_form_paper();
	fl_set_form_atclose(fd_form_paper->form_paper,
			    CancelCloseBoxCB, 0);
	fl_addto_choice(fd_form_paper->choice_papersize2,
			_(" Default | Custom | USletter | USlegal "
                        "| USexecutive | A3 | A4 | A5 | B3 | B4 | B5 "));
	fl_addto_choice(fd_form_paper->choice_paperpackage,
			_(" None "
			"| A4 small Margins (only portrait) "
			"| A4 very small Margins (only portrait) "
			"| A4 very wide margins (only portrait) "));
	fl_set_input_return(fd_form_paper->input_custom_width,
			    FL_RETURN_ALWAYS);
	fl_set_input_return(fd_form_paper->input_custom_height,
			    FL_RETURN_ALWAYS);
	fl_set_input_return(fd_form_paper->input_top_margin,
			    FL_RETURN_ALWAYS);
	fl_set_input_return(fd_form_paper->input_bottom_margin,
			    FL_RETURN_ALWAYS);
	fl_set_input_return(fd_form_paper->input_left_margin,
			    FL_RETURN_ALWAYS);
	fl_set_input_return(fd_form_paper->input_right_margin,
			    FL_RETURN_ALWAYS);
	fl_set_input_return(fd_form_paper->input_head_height,
			    FL_RETURN_ALWAYS);
	fl_set_input_return(fd_form_paper->input_head_sep,
			    FL_RETURN_ALWAYS);
	fl_set_input_return(fd_form_paper->input_foot_skip,
			    FL_RETURN_ALWAYS);
	lyxerr[Debug::INIT] << "Initializing form_paper...done" << endl;
#endif

#ifdef USE_OLD_DOCUMENT_LAYOUT
	// the quotes form
	fd_form_quotes = create_form_form_quotes();
	fl_set_form_atclose(fd_form_quotes->form_quotes,
			    CancelCloseBoxCB, 0);
	// Is it wrong of me to use « » instead of << >> ? (Lgb)
	// Maybe if people use a font other than latin1... (JMarc)
	fl_addto_choice(fd_form_quotes->choice_quotes_language,
			_(" ``text'' | ''text'' | ,,text`` | ,,text'' | «text» | »text« "));
#endif

	// the preamble form
	fd_form_preamble = create_form_form_preamble();
	fl_set_form_atclose(fd_form_preamble->form_preamble,
			    CancelCloseBoxCB, 0);

	// the sendto form
	fd_form_sendto = create_form_form_sendto();
	fl_set_form_atclose(fd_form_sendto->form_sendto, CancelCloseBoxCB, 0);

	// the figure form
	fd_form_figure = create_form_form_figure();
	fl_set_form_atclose(fd_form_figure->form_figure,
			    CancelCloseBoxCB, 0);
	fl_set_button(fd_form_figure->radio_postscript, 1);

	// the latex log form
	fd_latex_log = create_form_LaTeXLog();
	fl_set_form_atclose(fd_latex_log->LaTeXLog,
			    CancelCloseBoxCB, 0);

	// Show the main & title form
	int main_placement = FL_PLACE_CENTER | FL_FREE_SIZE;
	int title_placement = FL_PLACE_CENTER;
	// Did we get a valid position?
	if (xpos >= 0 && ypos >= 0) {
		lyxViews->setPosition(xpos, ypos);
		if (lyxrc.show_banner) {
			// show the title form in the middle of the main form
			fl_set_form_position(fd_form_title->form_title,
					     abs(xpos + (width/2) - (370 / 2)),
					     abs(ypos + (height/2) - (290 / 2)));
			title_placement = FL_PLACE_GEOMETRY;
			// The use of abs() above is a trick to ensure
			// valid positions
		}
			main_placement = FL_PLACE_POSITION;
	}
	lyxViews->show(main_placement, FL_FULLBORDER, "LyX");
	if (lyxrc.show_banner) {
		fl_show_form(fd_form_title->form_title, 
			     title_placement, FL_NOBORDER, 
			     _("LyX Banner"));
		fl_redraw_form(fd_form_title->form_title);
		fl_raise_form(fd_form_title->form_title);

		// Show the title form at most 7 secs (lowered from 10 secs)
		fl_set_timer(fd_form_title->timer_title, 7);
	}
}


void LyXGUI::runTime()
{
	if (!gui) return;

	guiruntime.runTime();
}


void LyXGUI::regBuf(Buffer * b)
{
	lyxViews->view()->buffer(b);
}


LyXView * LyXGUI::getLyXView() const
{
	return lyxViews;
}
