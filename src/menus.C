/*
 *  This file is part of
 * ================================================== 
 *
 *       LyX, The Document Processor
 *
 *       Copyright 1995 Matthias Ettrich
 *       Copyright 1995-2000 The LyX Team.
 *
 * ================================================== 
 */

/* This file contains all the menu and submenu declarations.
   The call backs are in lyx_cb.C */

/*
 * REMEMBER:
 * XFORMS can handle 10 (more with the new xforms, but not unlimited)
 * popups at the same time
 * so when you are finished looking at a pup free it (fl_freepup)
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <algorithm>

#include "menus.h"
#include "version.h"
#include "lyxfont.h"
#include "lyx_main.h"
#include "lyxfunc.h"
#include "spellchecker.h"
#include "support/filetools.h"
#include "LyXView.h"
#include "lastfiles.h"
#include "bufferlist.h"
#include "lyx_gui_misc.h"
#include "minibuffer.h"
#include "intl.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "gettext.h"
#include "layout.h"
#include "lyx_cb.h"
#include "bufferview_funcs.h"
#include "insets/insetref.h"
#include "insets/insettabular.h"
#include "tabular.h"

#include "frontends/Dialogs.h"

using std::vector;
using std::endl;
using std::max;
using std::min;
using std::sort;

extern FD_form_screen * fd_form_screen;
extern BufferList bufferlist;

// I would really prefere to see most or all of these 'extern's disappear.
// Their commands should be called through LyXFunc (IMO). (Lgb)

extern void MenuLayoutSave();
extern void ShowCredits();
extern void show_symbols_form(LyXFunc *);

// A bunch of wrappers

extern "C" void C_Menus_ShowFileMenu(FL_OBJECT * ob, long data)
{
	Menus::ShowFileMenu(ob, data);
}

extern "C" void C_Menus_ShowFileMenu2(FL_OBJECT * ob, long data)
{
	Menus::ShowFileMenu2(ob, data);
}

extern "C" void C_Menus_ShowEditMenu(FL_OBJECT * ob, long data)
{
	Menus::ShowEditMenu(ob, data);
}

extern "C" void C_Menus_ShowTocMenu(FL_OBJECT * ob, long data)
{
	Menus::ShowTocMenu(ob, data);
}

extern "C" void C_Menus_ShowRefsMenu(FL_OBJECT * ob, long data)
{
	Menus::ShowRefsMenu(ob, data);
}

extern "C" void C_Menus_ShowLayoutMenu(FL_OBJECT * ob, long data)
{
	Menus::ShowLayoutMenu(ob, data);
}

extern "C" void C_Menus_ShowInsertMenu(FL_OBJECT * ob, long data)
{
	Menus::ShowInsertMenu(ob, data);
}

extern "C" void C_Menus_ShowMathMenu(FL_OBJECT * ob, long data)
{
	Menus::ShowMathMenu(ob, data);
}

extern "C" void C_Menus_ShowOptionsMenu(FL_OBJECT * ob, long data)
{
	Menus::ShowOptionsMenu(ob, data);
}

extern "C" void C_Menus_ShowBufferMenu(FL_OBJECT * ob, long data)
{
	Menus::ShowBufferMenu(ob, data);
}

extern "C" void C_Menus_ShowHelpMenu(FL_OBJECT * ob, long data)
{
	Menus::ShowHelpMenu(ob, data);
}


Menus::Menus(LyXView * view, int air)
	: _view(view)
{	
	create_menus(air);
	// deactivate the menu accelerators
	fl_set_object_shortcut(menu_file, "", 1);
	fl_set_object_shortcut(menu_file2, "", 1);
	fl_set_object_shortcut(menu_edit, "", 1);
	fl_set_object_shortcut(menu_toc, "", 1);
	fl_set_object_shortcut(menu_refs, "", 1);
	fl_set_object_shortcut(menu_layout, "", 1);
	fl_set_object_shortcut(menu_math, "", 1);
	fl_set_object_shortcut(menu_insert, "", 1);
	fl_set_object_shortcut(menu_options, "", 1);
	fl_set_object_shortcut(menu_options2, "", 1);
	fl_set_object_shortcut(menu_buffer, "", 1);
	fl_set_object_shortcut(menu_help, "", 1);
	fl_set_object_shortcut(menu_help2, "", 1);
	hideMenus();
}


inline
BufferView * Menus::currentView() 
{
	return _view->view(); 
}


void Menus::showMenus()
{
	fl_hide_object(menu_grp2);
	fl_show_object(menu_grp1);
}


void Menus::hideMenus()
{
	fl_hide_object(menu_grp1);
	fl_show_object(menu_grp2);
}


void Menus::openByName(string const & menuName)
	/* Opens the visible menu of given name, or simply does nothing
	   when the name is not known. NOTE THE EXTREMELY STUPID
	   IMPLEMENTATION! :-) There are probably hundred ways to do
	   this better, for instance, by scanning the menu objects and
	   testing for the given name. I leave this as an exercise for an
	   experienced GG (GUI Guy/Girl). RVDK_PATCH_5. */
{
	if (menu_file->visible) {
		if (menuName == _("File"))
			ShowFileMenu(menu_file, 0);
		else if (menuName == _("Edit"))
			ShowEditMenu(menu_edit, 0);
		else if (menuName == _("TOC"))
			ShowTocMenu(menu_toc, 0);
		else if (menuName == _("Refs"))
			ShowRefsMenu(menu_refs, 0);
		else if (menuName == _("Layout"))
			ShowLayoutMenu(menu_layout, 0);
		else if (menuName == _("Insert"))
			ShowInsertMenu(menu_insert, 0);
		else if (menuName == _("Math"))
			ShowMathMenu(menu_math, 0);
		else if (menuName == _("Options"))
			ShowOptionsMenu(menu_options, 0);
		else if (menuName == _("Documents"))
			ShowBufferMenu(menu_buffer, 0);
		else if (menuName == _("Help"))
			ShowHelpMenu(menu_help, 0);
		else lyxerr << "The menu '" << menuName
			    << "' is not available." << endl;
	} else {
		if (menuName == _("File"))
			ShowFileMenu2(menu_file2, 0);
		else if (menuName == _("Options"))
			ShowOptionsMenu(menu_options2, 0);
		else if (menuName == _("Help"))
			ShowHelpMenu(menu_help2, 0);
		else lyxerr << "The menu '" << menuName
			    << "' is not available." << endl;
	}
}


void Menus::create_menus(int air)
{
	FL_FORM * form = _view->getForm(); 

	// Here I'd really like to see code like:
	// addMenuBar();
	FL_OBJECT * obj;

	const int MENU_LABEL_SIZE = FL_NORMAL_SIZE;
	const int mheight = 30;
	const int mbheight= 22;
	// where to place the menubar?
	const int yloc = (mheight - mbheight)/2; //air + bw;
	const int mbadd = 20; // menu button add (to width)
	int moffset = 0;

	// menubar frame
	obj = fl_add_frame(FL_UP_FRAME, 0, 0, form->w, mheight, "");
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, NorthEastGravity);

	menu_grp1 = fl_bgn_group();
	
	// File menu button
	menu_file = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      air+moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("File"),
						  strlen(_("File"))) + mbadd,
			      mbheight, _("File"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#F")), 1);
	fl_set_object_callback(obj, C_Menus_ShowFileMenu, 0);
	obj->u_vdata = this;
	
	// Edit menu button
	menu_edit = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("Edit"),
						  strlen(_("Edit"))) + mbadd,
			      mbheight, _("Edit"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#E")), 1);
	fl_set_object_callback(obj, C_Menus_ShowEditMenu, 0);
	obj->u_vdata = this;
	
	/// TOC menu button
	menu_toc = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("TOC"),
						  strlen(_("TOC"))) + mbadd,
			      mbheight, _("TOC"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#T")), 1);
	fl_set_object_callback(obj, C_Menus_ShowTocMenu, 0);
	obj->u_vdata = this;

	/// Refs menu button
	menu_refs = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("Refs"),
						  strlen(_("Refs"))) + mbadd,
			      mbheight, _("Refs"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#R")), 1);
	fl_set_object_callback(obj, C_Menus_ShowRefsMenu, 0);
	obj->u_vdata = this;

	// Layout menu button
	menu_layout = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("Layout"),
						  strlen(_("Layout"))) + mbadd,
			      mbheight, _("Layout"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#L")), 1);
	fl_set_object_callback(obj, C_Menus_ShowLayoutMenu, 0);
	obj->u_vdata = this;
	
	// Insert menu button button
	menu_insert = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("Insert"),
						  strlen(_("Insert"))) + mbadd,
			      mbheight, _("Insert"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#I")), 1);
	fl_set_object_callback(obj, C_Menus_ShowInsertMenu, 0);
	obj->u_vdata = this;
	
	// Math menu button
	menu_math = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("Math"),
						  strlen(_("Math"))) + mbadd,
			      mbheight, _("Math"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#M")), 1);
	fl_set_object_callback(obj, C_Menus_ShowMathMenu, 0);
	obj->u_vdata = this;
	
	// Options menu button
	menu_options = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("Options"),
						  strlen(_("Options"))) + mbadd,
			      mbheight, _("Options"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#O")), 1);
	fl_set_object_callback(obj, C_Menus_ShowOptionsMenu, 0);
	obj->u_vdata = this;

	// Documents menu button
	menu_buffer = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("Documents"),
						  strlen(_("Documents"))) + mbadd,
			      mbheight, _("Documents"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#D")), 1);
	fl_set_object_callback(obj, C_Menus_ShowBufferMenu, 0);
	obj->u_vdata = this;
	
	// Help menu button
	menu_help = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("Help"),
						  strlen(_("Help"))) + mbadd,
			      mbheight, _("Help"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#H")), 1);
	fl_set_object_callback(obj, C_Menus_ShowHelpMenu, 0);
	obj->u_vdata = this;
	
	fl_end_group();

	// Set the menu buttons atrributes.
	// Due to a bug in xforms we cant do this only an the group.
	obj = menu_grp1->next;
	do {
		fl_set_object_boxtype(obj, FL_FLAT_BOX);
		fl_set_object_color(obj, FL_MCOL, FL_MCOL);
		fl_set_object_lsize(obj, MENU_LABEL_SIZE);
		fl_set_object_lstyle(obj, FL_BOLD_STYLE);
		fl_set_object_resize(obj, FL_RESIZE_ALL);
		fl_set_object_gravity(obj, NorthWestGravity, NorthWestGravity);
		obj= obj->next;
	} while (obj != 0 && obj->objclass != FL_END_GROUP);

	// group 2
	moffset = 0;
	menu_grp2 = fl_bgn_group();
	
	// File menu button
	menu_file2 = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      air+moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("File"),
						  strlen(_("File"))) + mbadd,
			      mbheight, _("File"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#F")), 1);
	fl_set_object_callback(obj, C_Menus_ShowFileMenu2, 0);
	obj->u_vdata = this;
	
	// Options menu button
	menu_options2 = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("Options"),
						  strlen(_("Options"))) +mbadd,
			      mbheight, _("Options"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#O")), 1);
	fl_set_object_callback(obj, C_Menus_ShowOptionsMenu, 0);
	obj->u_vdata = this;

	// Help menu button
	menu_help2 = obj = 
		fl_add_button(FL_TOUCH_BUTTON,
			      moffset, yloc,
			      fl_get_string_width(FL_BOLD_STYLE,
						  MENU_LABEL_SIZE,
						  _("Help"),
						  strlen(_("Help"))) + mbadd,
			      mbheight, _("Help"));
	moffset += obj->w + air;
	fl_set_object_shortcut(obj, scex(_("MB|#H")), 1);
	fl_set_object_callback(obj, C_Menus_ShowHelpMenu, 0);
	obj->u_vdata = this;
	
	fl_end_group();

	// Set the menu buttons atrributes.
	// Due to a bug in xforms we cant do this only an the group.
	obj = menu_grp2->next;
	do {
		fl_set_object_boxtype(obj, FL_FLAT_BOX);
		fl_set_object_color(obj, FL_MCOL, FL_MCOL);
		fl_set_object_lsize(obj, MENU_LABEL_SIZE);
		fl_set_object_lstyle(obj, FL_BOLD_STYLE);
		fl_set_object_resize(obj, FL_RESIZE_ALL);
		fl_set_object_gravity(obj, NorthWestGravity, NorthWestGravity);
		obj= obj->next;
	} while (obj != 0 && obj->objclass != FL_END_GROUP);
}


void Menus::ScreenOptions()
{
	static int ow = -1, oh;

	// this is not very nice....
	fl_set_input(fd_form_screen->input_roman, 
		     lyxrc.roman_font_name.c_str());
	fl_set_input(fd_form_screen->input_sans, 
		     lyxrc.sans_font_name.c_str());
	fl_set_input(fd_form_screen->input_typewriter,
		     lyxrc.typewriter_font_name.c_str());
	fl_set_input(fd_form_screen->input_font_norm, 
		     lyxrc.font_norm.c_str());
	char tmpstring[10];
	sprintf(tmpstring, "%d", lyxrc.zoom);
	fl_set_input(fd_form_screen->intinput_size, tmpstring);
	if (fd_form_screen->form_screen->visible) {
		fl_raise_form(fd_form_screen->form_screen);
	} else {
		fl_show_form(fd_form_screen->form_screen,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
			     _("Screen Options"));
		if (ow < 0) {
			ow = fd_form_screen->form_screen->w;
			oh = fd_form_screen->form_screen->h;
		}
		fl_set_form_minsize(fd_form_screen->form_screen, ow, oh);
	}
}


//
// Here comes all the menu callbacks.
//

void Menus::ShowFileMenu(FL_OBJECT * ob, long)
{
	Menus * men = static_cast<Menus*>(ob->u_vdata);

	// Regarding the pseudo-menu-button:
	// ok, ok this is a hack. It would be better to use the menus of the
	// xforms 0.8 library. but then all popups have to be defined all the
	// time, code rewriting would be necessary and contex-depending menus
	// (i.e. the linux-doc-sgml stuff) are a bit more complicated. But of
	// course it would be more proper (and little faster). So if anybody
	// likes to do the cleanup, just do it. Matthias

	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);

	Buffer * tmpbuffer = men->_view->buffer();
	LyXFunc * tmpfunc = men->_view->getLyXFunc();

	bool LinuxDoc = tmpbuffer->isLinuxDoc();
	bool DocBook  = tmpbuffer->isDocBook();
	bool Literate = tmpbuffer->isLiterate();

	// Import sub-menu

	int SubFileImport = fl_defpup(FL_ObjWin(ob),
				      _("Import%t"
					"|LaTeX...%x30"
					"|Ascii Text as Lines...%x31"
					"|Ascii Text as Paragraphs%x32"
					"|Noweb%x33"
					"|LinuxDoc%x34"));

	fl_setpup_shortcut(SubFileImport, 30, scex(_("FIM|Ll#l#L")));
	fl_setpup_shortcut(SubFileImport, 31, scex(_("FIM|Aa#a#A")));
	fl_setpup_shortcut(SubFileImport, 32, scex(_("FIM|Pp#p#P")));
	fl_setpup_shortcut(SubFileImport, 33, scex(_("FIM|Nn#n#N")));
	fl_setpup_shortcut(SubFileImport, 34, scex(_("FIM|Dd#d#D")));

	// Export sub-menu

	// remember to make this handle linuxdoc too.
	// and now docbook also.
	int SubFileExport = 0;
	if (!LinuxDoc && !DocBook)
		SubFileExport= fl_defpup(FL_ObjWin(ob),
					 _("Export%t"
					   "|as LaTeX...%x40"
					   "|as DVI...%x41"
					   "|as PostScript...%x42"
					   "|as Ascii Text...%x43"
					   "|as HTML...%x44"
					   "|Custom...%x45"));
        else if(LinuxDoc)
		SubFileExport= fl_defpup(FL_ObjWin(ob),
					 _("Export%t"
					   "|as LinuxDoc...%x40"
					   "|as DVI...%x41"
					   "|as PostScript...%x42"
					   "|as Ascii Text...%x43"
					   "|as HTML...%x44"));
        else if(DocBook)
		SubFileExport= fl_defpup(FL_ObjWin(ob),
					 _("Export%t"
					   "|as DocBook...%x40"
					   "|as DVI...%x41"
					   "|as PostScript...%x42"
					   "|as Ascii Text...%x43"
					   "|as HTML...%x44"));

	fl_setpup_shortcut(SubFileExport, 40, scex(_("FEX|Ll#l#L")));
	fl_setpup_shortcut(SubFileExport, 41, scex(_("FEX|Dd#d#D")));
	fl_setpup_shortcut(SubFileExport, 42, scex(_("FEX|Pp#p#P")));
	fl_setpup_shortcut(SubFileExport, 43, scex(_("FEX|Tt#t#T")));
	fl_setpup_shortcut(SubFileExport, 44, scex(_("FEX|Hh#h#H")));

	if (!LinuxDoc && !DocBook) {
		fl_setpup_shortcut(SubFileExport, 45, scex(_("FEX|mM#m#M")));
	}
	
	int FileMenu = fl_defpup(FL_ObjWin(ob),
				 _("New..."
				   "|New from template..."
				   "|Open...%l"
				   "|Close"
				   "|Save"
				   "|Save As..."
				   "|Revert to saved%l"
				   "|View dvi"
				   "|View PostScript"
				   "|Update dvi"
                                   "|Update PostScript"
                                   "|Build program%l"
				   "|Print..."
				   "|Fax..."));

	fl_setpup_shortcut(FileMenu, 1, scex(_("FM|Nn#n#N")));
	fl_setpup_shortcut(FileMenu, 2, scex(_("FM|tT#t#T")));
	fl_setpup_shortcut(FileMenu, 3, scex(_("FM|Oo#o#O")));
	fl_setpup_shortcut(FileMenu, 4, scex(_("FM|Cc#c#C")));
	fl_setpup_shortcut(FileMenu, 5, scex(_("FM|Ss#s#S")));
	fl_setpup_shortcut(FileMenu, 6, scex(_("FM|Aa#a#A")));
	fl_setpup_shortcut(FileMenu, 7, scex(_("FM|Rr#r#R")));
	fl_setpup_shortcut(FileMenu, 8, scex(_("FM|dD#d#D")));
	fl_setpup_shortcut(FileMenu, 9, scex(_("FM|wW#w#W")));
	fl_setpup_shortcut(FileMenu, 10, scex(_("FM|vV#v#V")));
	fl_setpup_shortcut(FileMenu, 11, scex(_("FM|Uu#u#U")));
        fl_setpup_shortcut(FileMenu, 12, scex(_("FM|Bb#b#B")));
        fl_setpup_shortcut(FileMenu, 13, scex(_("FM|Pp#p#P")));
        fl_setpup_shortcut(FileMenu, 14, scex(_("FM|Ff#f#F")));

	// These commands are disabled when the corresponding programs
	// are not installed. I simply grey them out, since I do not
	// want to change their number (JMarc)
	bool hasLaTeX = lyxrc.latex_command != "none";

	if (!hasLaTeX || lyxrc.view_dvi_command == "none") 
		fl_setpup_mode(FileMenu, 8, FL_PUP_GREY);
	
	if (!hasLaTeX || lyxrc.view_ps_command == "none") 
		fl_setpup_mode(FileMenu, 9, FL_PUP_GREY);
	
	if (!hasLaTeX) {
		fl_setpup_mode(FileMenu, 10, FL_PUP_GREY);
		fl_setpup_mode(FileMenu, 11, FL_PUP_GREY);
	} 

        if (lyxrc.literate_command == "none" || ! Literate) 
		fl_setpup_mode(FileMenu, 12, FL_PUP_GREY);

	if (!hasLaTeX || lyxrc.print_command == "none") 
		fl_setpup_mode(FileMenu, 13, FL_PUP_GREY);

	if (!hasLaTeX || lyxrc.fax_command == "none") 
		fl_setpup_mode(FileMenu, 14, FL_PUP_GREY);

	bool hasReLyX = lyxrc.relyx_command != "none";
	if (!hasReLyX) {
		// Disable import LaTeX and Noweb
		fl_setpup_mode(SubFileImport, 30, FL_PUP_GREY);
		fl_setpup_mode(SubFileImport, 33, FL_PUP_GREY);
	}

	if ( lyxrc.linuxdoc_to_lyx_command == "none")
		fl_setpup_mode(SubFileImport, 34, FL_PUP_GREY);

	if (!hasLaTeX) {
		// Disable export dvi and export postscript
		fl_setpup_mode(SubFileExport, 41, FL_PUP_GREY);
		fl_setpup_mode(SubFileExport, 42, FL_PUP_GREY);
	}

	if ((!LinuxDoc && !DocBook && lyxrc.html_command == "none") ||
	    ( LinuxDoc && lyxrc.linuxdoc_to_html_command == "none") ||
	    ( DocBook  && lyxrc.docbook_to_html_command  == "none")) {
		// Disable export HTML
		fl_setpup_mode(SubFileExport, 44, FL_PUP_GREY);
	}

	// xgettext:no-c-format
	fl_addtopup(FileMenu, _("|Import%m"), SubFileImport);
	// xgettext:no-c-format
	fl_addtopup(FileMenu, _("|Export%m%l"), SubFileExport);
	// xgettext:no-c-format
	fl_addtopup(FileMenu, _("|Exit%l"));
	fl_setpup_shortcut(FileMenu, 15, scex(_("FM|Ii#i#I")));
	fl_setpup_shortcut(FileMenu, 16, scex(_("FM|Ee#e#E")));
	fl_setpup_shortcut(FileMenu, 17, scex(_("FM|xX#x#X")));

	// make the lastfiles menu
	int ii = 1;
	for (LastFiles::const_iterator cit = lastfiles->begin();
	     cit != lastfiles->end() && ii < 10; ++cit, ++ii) {
		string tmp = tostr(ii);
		string tmp2 = tmp + "#" + tmp;
		tmp += ". " + MakeDisplayPath((*cit), 30);
		fl_addtopup(FileMenu, tmp.c_str());
		fl_setpup_shortcut(FileMenu, 18 - 1 + ii, tmp2.c_str());
	}

	// place popup
	fl_setpup_position(
		men->_view->getForm()->x + ob->x,
		men->_view->getForm()->y + ob->y + ob->h + 10);   
	int choice = fl_dopup(FileMenu);
	XFlush(fl_display);

	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);

	switch (choice) {
	case -1: case 0: // we won't do anything
		break;
	case  1: tmpfunc->Dispatch(LFUN_MENUNEW); break;
	case  2: tmpfunc->Dispatch(LFUN_MENUNEWTMPLT); break;
	case  3: tmpfunc->Dispatch(LFUN_MENUOPEN); break;
	case  4: tmpfunc->Dispatch(LFUN_CLOSEBUFFER); break;
	case  5: tmpfunc->Dispatch(LFUN_MENUWRITE); break;
	case  6: tmpfunc->Dispatch(LFUN_MENUWRITEAS); break;
	case  7: tmpfunc->Dispatch(LFUN_MENURELOAD); break;
	case  8: tmpfunc->Dispatch(LFUN_PREVIEW); break;
	case  9: tmpfunc->Dispatch(LFUN_PREVIEWPS); break;
	case 10: tmpfunc->Dispatch(LFUN_RUNLATEX); break;
	case 11: tmpfunc->Dispatch(LFUN_RUNDVIPS); break;
	case 12: tmpfunc->Dispatch(LFUN_BUILDPROG); break;
	case 13: tmpfunc->Dispatch(LFUN_MENUPRINT); break;
	case 14: tmpfunc->Dispatch(LFUN_FAX); break;
	case 15: // import menu
	case 30: tmpfunc->Dispatch(LFUN_IMPORT, "latex");
		break;
	case 31: tmpfunc->Dispatch(LFUN_IMPORT, "ascii");
		break;
	case 32: tmpfunc->Dispatch(LFUN_IMPORT, "asciiparagraph");
		break;
	case 33: tmpfunc->Dispatch(LFUN_IMPORT, "noweb");
		break;
	case 34: tmpfunc->Dispatch(LFUN_IMPORT, "linuxdoc");
		break;
	case 16: // export menu
	case 40:
		if (!LinuxDoc && !DocBook)
			tmpfunc->Dispatch(LFUN_EXPORT, "latex");
		else if(LinuxDoc)
			tmpfunc->Dispatch(LFUN_EXPORT, "linuxdoc");
		else
			tmpfunc->Dispatch(LFUN_EXPORT, "docbook");
		break;
	case 41: tmpfunc->Dispatch(LFUN_EXPORT, "dvi");
		break;
	case 42: tmpfunc->Dispatch(LFUN_EXPORT, "postscript");
		break;
	case 43: tmpfunc->Dispatch(LFUN_EXPORT, "ascii");
		break;
	case 44:
		if (!LinuxDoc && !DocBook)
			tmpfunc->Dispatch(LFUN_EXPORT, "html");
		else if(LinuxDoc)
			tmpfunc->Dispatch(LFUN_EXPORT, "html-linuxdoc");
		else
			tmpfunc->Dispatch(LFUN_EXPORT, "html-docbook");
		break;
	case 45: tmpfunc->Dispatch(LFUN_EXPORT, "custom"); 
		break;
	case 17: tmpfunc->Dispatch(LFUN_QUIT); break;
		// Lastfiles:
	case 18: // The first item with lastfiles.
	default:
		men->currentView()
			->buffer(bufferlist
				 .loadLyXFile((*lastfiles)[choice - 18]));
		break;
	}
	fl_freepup(SubFileImport);
	fl_freepup(SubFileExport);
	fl_freepup(FileMenu);

	AllowInput(men->currentView());
}


void Menus::ShowFileMenu2(FL_OBJECT * ob, long)
{
	Menus * men = static_cast<Menus*>(ob->u_vdata);

	// Regarding the pseudo-menu-button:
	// ok, ok this is a hack. It would be better to use the menus of the
	// xforms 0.8 library. but then all popups have to be defined all the
	// time, code rewriting would be necessary and contex-depending menus
	// (i.e. the linux-doc-sgml stuff) are a bit more complicated. But of
	// course it would be more proper (and little faster). So if anybody
	// likes to do the cleanup, just do it. Matthias

	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);

	LyXFunc * tmpfunc = men->_view->getLyXFunc();

	// Import sub-menu
	
	int SubFileImport = fl_defpup(FL_ObjWin(ob),
				      _("Import%t"
					"|LaTeX...%x15"
					"|Ascii Text as Lines...%x16"
					"|Ascii Text as Paragraphs...%x17"
					"|Noweb...%x18"
					"|LinuxDoc...%x19"));
	
	fl_setpup_shortcut(SubFileImport, 15, scex(_("FIM|Ll#l#L")));
	fl_setpup_shortcut(SubFileImport, 16, scex(_("FIM|Aa#a#A")));
	fl_setpup_shortcut(SubFileImport, 17, scex(_("FIM|Pp#p#P")));
	fl_setpup_shortcut(SubFileImport, 18, scex(_("FIM|Nn#n#N")));
	fl_setpup_shortcut(SubFileImport, 19, scex(_("FIM|Dd#d#D")));

	bool hasReLyX = lyxrc.relyx_command != "none";
	if (!hasReLyX) {
		// Disable import LaTeX and Noweb
		fl_setpup_mode(SubFileImport, 15, FL_PUP_GREY);
		fl_setpup_mode(SubFileImport, 18, FL_PUP_GREY);
	}

	if ( lyxrc.linuxdoc_to_lyx_command == "none")
		fl_setpup_mode(SubFileImport, 19, FL_PUP_GREY);

	// This can be done cleaner later.
	int FileMenu = fl_defpup(FL_ObjWin(ob), 
				 _("New..."
				   "|New from template..."
				   "|Open...%l"
				   "|Import%m%l"
				   "|Exit%l"), SubFileImport);
	
	fl_setpup_shortcut(FileMenu, 1, scex(_("FM|Nn#n#N")));
	fl_setpup_shortcut(FileMenu, 2, scex(_("FM|tT#t#T")));
	fl_setpup_shortcut(FileMenu, 3, scex(_("FM|Oo#o#O")));
	fl_setpup_shortcut(FileMenu, 4, scex(_("FM|Ii#i#I")));
	fl_setpup_shortcut(FileMenu, 5, scex(_("FM|xX#x#X")));
	
	// make the lastfiles menu
	int ii = 1;
	for (LastFiles::const_iterator cit = lastfiles->begin();
	     cit != lastfiles->end() && ii < 10; ++cit, ++ii) {
		string tmp = tostr(ii);
		string tmp2 = tmp + "#" + tmp;
		tmp += ". " + MakeDisplayPath((*cit), 30);
		fl_addtopup(FileMenu, tmp.c_str());
		fl_setpup_shortcut(FileMenu, 18 - 1 + ii, tmp2.c_str());
	}

	// place popup
	fl_setpup_position(
		men->_view->getForm()->x + ob->x,
		men->_view->getForm()->y + ob->y + ob->h + 10);   
	int choice = fl_dopup(FileMenu);
	XFlush(fl_display);

	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);

	switch (choice) {
	case -1: case 0: // we won't do anything
		break;
	case 1:
		tmpfunc->Dispatch(LFUN_MENUNEW);
		break;
	case 2:
		tmpfunc->Dispatch(LFUN_MENUNEWTMPLT);
		break;
	case 3:
		tmpfunc->Dispatch(LFUN_MENUOPEN);
		break;
	case 4: // import menu
	case 15: tmpfunc->Dispatch(LFUN_IMPORT, "latex");
		break;
	case 16: tmpfunc->Dispatch(LFUN_IMPORT, "ascii");
		break;
	case 17: tmpfunc->Dispatch(LFUN_IMPORT, "asciiparagraph");
		break;
	case 18: tmpfunc->Dispatch(LFUN_IMPORT, "noweb");
		break;
	case 19: tmpfunc->Dispatch(LFUN_IMPORT, "linuxdoc");
		break;
	case 5:
		tmpfunc->Dispatch(LFUN_QUIT);
		break;
		// Lastfiles:
	case 6: // The first item with lastfiles.
	default:
		men->currentView()
			->buffer(bufferlist
				 .loadLyXFile((*lastfiles)[choice - 6]));
		break;
	}

	fl_freepup(SubFileImport);
	fl_freepup(FileMenu);
	AllowInput(men->currentView());
	return;
}


void Menus::ShowEditMenu(FL_OBJECT * ob, long)
{
	Menus * men = static_cast<Menus*>(ob->u_vdata);
	
	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);

	Buffer * tmpbuffer = men->_view->buffer();
	LyXFunc * tmpfunc = men->_view->getLyXFunc();

	// Floats & Insets submenu
	int SubEditFloats= fl_defpup(FL_ObjWin(ob),
				     _("Floats & Insets%t"
				       "|Open/Close%x21"
				       "|Melt%x22"
				       "|Open All Footnotes/Margin Notes%x23"
				       "|Close All Footnotes/Margin Notes%x24"
				       "|Open All Figures/Tables%x25"
				       "|Close All Figures/Tables%x26%l"
				       "|Remove all Error Boxes%x27"));
	
	fl_setpup_shortcut(SubEditFloats, 21, scex(_("EMF|Oo#o#O")));
	fl_setpup_shortcut(SubEditFloats, 22, scex(_("EMF|Mm#m#M")));
	fl_setpup_shortcut(SubEditFloats, 23, scex(_("EMF|Aa#a#A")));
	fl_setpup_shortcut(SubEditFloats, 24, scex(_("EMF|Cc#c#C")));
	fl_setpup_shortcut(SubEditFloats, 25, scex(_("EMF|Ff#f#F")));
	fl_setpup_shortcut(SubEditFloats, 26, scex(_("EMF|Tt#t#T")));
	fl_setpup_shortcut(SubEditFloats, 27, scex(_("EMF|Rr#r#R")));

	// Table submenu
	int SubEditTable = fl_newpup(FL_ObjWin(ob));
	if (men->currentView()->available() && 
	    men->currentView()->text->cursor.par()->table &&
	    !tmpbuffer->isReadonly()){
		LyXTable * table = men->currentView()->text->cursor.par()->table;

		fl_addtopup(SubEditTable, _("Table%t"));

		if (table->IsMultiColumn(men->currentView()->text->
					 NumberOfCell(men->currentView()->
						      text->cursor.par(), 
						      men->currentView()->
						      text->cursor.pos())))
			fl_addtopup(SubEditTable, _("|Multicolumn%B%x44%l"));
		else
			fl_addtopup(SubEditTable, _("|Multicolumn%b%x44%l"));
		fl_setpup_shortcut(SubEditTable, 44, scex(_("EMT|Mm#m#M")));
     
		if (table->TopLine(men->currentView()->text->
				   NumberOfCell(men->currentView()->
						text->cursor.par(), 
						men->currentView()->text->
						cursor.pos())))
			fl_addtopup(SubEditTable, _("|Line Top%B%x36"));
		else
			fl_addtopup(SubEditTable, _("|Line Top%b%x36"));
		fl_setpup_shortcut(SubEditTable, 36, scex(_("EMT|Tt#t#T")));
     
		if (table->BottomLine(men->currentView()->text->
				      NumberOfCell(men->currentView()->
						   text->cursor.par(), 
						   men->currentView()->
						   text->cursor.pos())))
			fl_addtopup(SubEditTable, _("|Line Bottom%B%x37"));
		else
			fl_addtopup(SubEditTable, _("|Line Bottom%b%x37"));
		fl_setpup_shortcut(SubEditTable, 37, scex(_("EMT|Bb#b#B")));

		if (table->LeftLine(men->currentView()->text->
				    NumberOfCell(men->currentView()->
						 text->cursor.par(), 
						 men->currentView()->
						 text->cursor.pos())))
			fl_addtopup(SubEditTable, _("|Line Left%B%x38"));
		else
			fl_addtopup(SubEditTable, _("|Line Left%b%x38"));
		fl_setpup_shortcut(SubEditTable, 38, scex(_("EMT|Ll#l#L")));

		if (table->RightLine(men->currentView()->text->
				     NumberOfCell(men->currentView()->
						  text->cursor.par(), 
						  men->currentView()->
						  text->cursor.pos())))
			fl_addtopup(SubEditTable, _("|Line Right%B%x39%l"));
		else
			fl_addtopup(SubEditTable, _("|Line Right%b%x39%l"));
		fl_setpup_shortcut(SubEditTable, 39, scex(_("EMT|Rr#r#R")));

		int align =
			table->GetAlignment(men->currentView()->text->
					    NumberOfCell(men->currentView()->
							 text->cursor.par(), 
							 men->currentView()->
							 text->cursor.pos()));
		if (align == LYX_ALIGN_LEFT)
			fl_addtopup(SubEditTable, _("|Align Left%R%x40"));
		else
			fl_addtopup(SubEditTable, _("|Align Left%r%x40"));
		fl_setpup_shortcut(SubEditTable, 40, scex(_("EMT|eE#e#E")));

		if (align == LYX_ALIGN_RIGHT)
			fl_addtopup(SubEditTable, _("|Align Right%R%x41"));
		else
			fl_addtopup(SubEditTable, _("|Align Right%r%x41"));
		fl_setpup_shortcut(SubEditTable, 41, scex(_("EMT|iI#i#I")));

		if (align == LYX_ALIGN_CENTER)
			fl_addtopup(SubEditTable, _("|Align Center%R%x42%l"));
		else
			fl_addtopup(SubEditTable, _("|Align Center%r%x42%l"));
		fl_setpup_shortcut(SubEditTable, 42, scex(_("EMT|Cc#c#C")));

		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Append Row%x32"));
		fl_setpup_shortcut(SubEditTable, 32, scex(_("EMT|oO#o#O")));
		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Append Column%x33%l"));
		fl_setpup_shortcut(SubEditTable, 33, scex(_("EMT|uU#u#U")));
		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Delete Row%x34"));
		fl_setpup_shortcut(SubEditTable, 34, scex(_("EMT|wW#w#W")));
		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Delete Column%x35%l"));
		fl_setpup_shortcut(SubEditTable, 35, scex(_("EMT|nN#n#N")));
		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Delete Table%x43"));
		fl_setpup_shortcut(SubEditTable, 43, scex(_("EMT|Dd#d#D")));
	} else if (men->currentView()->the_locking_inset &&
		   (men->currentView()->the_locking_inset->LyxCode() ==
		    Inset::TABULAR_CODE) &&
		   !tmpbuffer->isReadonly()) {
		InsetTabular * inset = static_cast<InsetTabular *>(men->currentView()->the_locking_inset);
		LyXTabular * table = inset->tabular;

		fl_addtopup(SubEditTable, _("Table%t"));

		if (table->IsMultiColumn(inset->GetActCell()))
			fl_addtopup(SubEditTable, _("|Multicolumn%B%x44%l"));
		else
			fl_addtopup(SubEditTable, _("|Multicolumn%b%x44%l"));
		fl_setpup_shortcut(SubEditTable, 44, scex(_("EMT|Mm#m#M")));

		if (table->TopLine(inset->GetActCell()))
			fl_addtopup(SubEditTable, _("|Line Top%B%x36"));
		else
			fl_addtopup(SubEditTable, _("|Line Top%b%x36"));
		fl_setpup_shortcut(SubEditTable, 36, scex(_("EMT|Tt#t#T")));
     
		if (table->BottomLine(inset->GetActCell()))
			fl_addtopup(SubEditTable, _("|Line Bottom%B%x37"));
		else
			fl_addtopup(SubEditTable, _("|Line Bottom%b%x37"));
		fl_setpup_shortcut(SubEditTable, 37, scex(_("EMT|Bb#b#B")));

		if (table->LeftLine(inset->GetActCell()))
			fl_addtopup(SubEditTable, _("|Line Left%B%x38"));
		else
			fl_addtopup(SubEditTable, _("|Line Left%b%x38"));
		fl_setpup_shortcut(SubEditTable, 38, scex(_("EMT|Ll#l#L")));

		if (table->RightLine(inset->GetActCell()))
			fl_addtopup(SubEditTable, _("|Line Right%B%x39%l"));
		else
			fl_addtopup(SubEditTable, _("|Line Right%b%x39%l"));
		fl_setpup_shortcut(SubEditTable, 39, scex(_("EMT|Rr#r#R")));

		int align = table->GetAlignment(inset->GetActCell());
		if (align == LYX_ALIGN_LEFT)
			fl_addtopup(SubEditTable, _("|Align Left%R%x40"));
		else
			fl_addtopup(SubEditTable, _("|Align Left%r%x40"));
		fl_setpup_shortcut(SubEditTable, 40, scex(_("EMT|eE#e#E")));

		if (align == LYX_ALIGN_RIGHT)
			fl_addtopup(SubEditTable, _("|Align Right%R%x41"));
		else
			fl_addtopup(SubEditTable, _("|Align Right%r%x41"));
		fl_setpup_shortcut(SubEditTable, 41, scex(_("EMT|iI#i#I")));

		if (align == LYX_ALIGN_CENTER)
			fl_addtopup(SubEditTable, _("|Align Center%R%x42%l"));
		else
			fl_addtopup(SubEditTable, _("|Align Center%r%x42%l"));
		fl_setpup_shortcut(SubEditTable, 42, scex(_("EMT|Cc#c#C")));

		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Append Row%x32"));
		fl_setpup_shortcut(SubEditTable, 32, scex(_("EMT|oO#o#O")));
		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Append Column%x33%l"));
		fl_setpup_shortcut(SubEditTable, 33, scex(_("EMT|uU#u#U")));
		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Delete Row%x34"));
		fl_setpup_shortcut(SubEditTable, 34, scex(_("EMT|wW#w#W")));
		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Delete Column%x35%l"));
		fl_setpup_shortcut(SubEditTable, 35, scex(_("EMT|nN#n#N")));
		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Delete Table%x43"));
		fl_setpup_shortcut(SubEditTable, 43, scex(_("EMT|Dd#d#D")));
	} else {
		fl_addtopup(SubEditTable, _("Table%t"));
		// xgettext:no-c-format
		fl_addtopup(SubEditTable, _("|Insert table%x31"));
		fl_setpup_shortcut(SubEditTable, 31, scex(_("EMT|Ii#i#I")));
	}

	int SubVersionControl = 	fl_newpup(FL_ObjWin(ob));
	fl_addtopup(SubVersionControl, _("Version Control%t"));
	if (tmpbuffer->lyxvc.inUse()) {
		// xgettext:no-c-format
		fl_addtopup(SubVersionControl, _("|Register%d%x51"));
		if (tmpbuffer->isReadonly()) {
			// signifies that the file is not checked out
			// xgettext:no-c-format
			fl_addtopup(SubVersionControl, _("|Check In Changes%d%x52"));
			// xgettext:no-c-format
			fl_addtopup(SubVersionControl, _("|Check Out for Edit%x53"));
		} else {
			// signifies that the file is checked out
			// xgettext:no-c-format
			fl_addtopup(SubVersionControl, _("|Check In Changes%x52"));
			// xgettext:no-c-format
			fl_addtopup(SubVersionControl, _("|Check Out for Edit%d%x53"));
		}
		// xgettext:no-c-format
		fl_addtopup(SubVersionControl, _("|Revert to last version%x54"));
		// xgettext:no-c-format
		fl_addtopup(SubVersionControl, _("|Undo last check in%x55"));
		// xgettext:no-c-format
		fl_addtopup(SubVersionControl, _("|Show History%x56"));
	} else {
		// xgettext:no-c-format
		fl_addtopup(SubVersionControl, _("|Register%x51"));
	}
	// the shortcuts are not good.
	fl_setpup_shortcut(SubVersionControl, 51, scex(_("EMV|Rr#r#R")));
	fl_setpup_shortcut(SubVersionControl, 52, scex(_("EMV|Ii#i#I")));
	fl_setpup_shortcut(SubVersionControl, 53, scex(_("EMV|Oo#o#O")));
	fl_setpup_shortcut(SubVersionControl, 54, scex(_("EMV|lL#l#l")));
	fl_setpup_shortcut(SubVersionControl, 55, scex(_("EMV|Uu#u#U")));
	fl_setpup_shortcut(SubVersionControl, 56, scex(_("EMV|Hh#h#H")));

	int EditMenu= fl_defpup(FL_ObjWin(ob),
				_("Undo"
				  "|Redo %l"
				  "|Cut"
				  "|Copy"
				  "|Paste%l"
				  "|Find & Replace..."
				  "|Go to Error"
				  "|Go to Note"
				  "|Floats & Insets%m"
				  "|Table%m"
				  "|Spellchecker...."
				  "|Check TeX"
				  "|Table of Contents..."
				  "|List of Figures..."
				  "|List of Tables..."
				  "|List of Algorithms...%l"
				  "|Version Control%m%l"
				  "|View LaTeX log file%l"
				  "|Paste Primary Selection as Lines"
				  "|Paste Primary Selection as Paragraphs"),
				SubEditFloats, SubEditTable, SubVersionControl);

	fl_setpup_shortcut(EditMenu, 1, scex(_("EM|Uu#u#U")));
	fl_setpup_shortcut(EditMenu, 2, scex(_("EM|Rr#r#R")));
	fl_setpup_shortcut(EditMenu, 3, scex(_("EM|Cc#c#C")));
	fl_setpup_shortcut(EditMenu, 4, scex(_("EM|oO#o#O")));
	fl_setpup_shortcut(EditMenu, 5, scex(_("EM|Pp#p#P")));
	fl_setpup_shortcut(EditMenu, 6, scex(_("EM|Ff#f#F")));
	fl_setpup_shortcut(EditMenu, 7, scex(_("EM|Ee#e#E")));
	fl_setpup_shortcut(EditMenu, 8, scex(_("EM|Nn#n#N")));
	fl_setpup_shortcut(EditMenu, 9, scex(_("EM|Ii#i#I")));
	//fl_setpup_shortcut(EditMenu, 10, scex(_("EM|Tt#t#T")));
	fl_setpup_shortcut(EditMenu, 11, scex(_("EM|Ss#s#S")));
	fl_setpup_shortcut(EditMenu, 12, scex(_("EM|hH#h#H")));
	fl_setpup_shortcut(EditMenu, 13, scex(_("EM|Tt#t#T")));
	//fl_setpup_shortcut(EditMenu, 14, scex(_("EM|Ff#f#F")));
	//fl_setpup_shortcut(EditMenu, 15, scex(_("EM|bB#b#B")));
	//fl_setpup_shortcut(EditMenu, 16, scex(_("EM|mM#m#M")));
	fl_setpup_shortcut(EditMenu, 17, scex(_("EM|Vv#v#V")));
	fl_setpup_shortcut(EditMenu, 18, scex(_("EM|wW#w#W")));
	fl_setpup_shortcut(EditMenu, 19, scex(_("EM|Ll#l#L")));
	fl_setpup_shortcut(EditMenu, 20, scex(_("EM|gG#g#G")));
      
	// disable unavailable entries.
	if(tmpbuffer->undostack.empty())
		fl_setpup_mode(EditMenu, 1, FL_PUP_GREY);
	if(tmpbuffer->redostack.empty())
		fl_setpup_mode(EditMenu, 2, FL_PUP_GREY);
	if(lyxrc.isp_command == "none") 
		fl_setpup_mode(EditMenu, 11, FL_PUP_GREY);
	if(lyxrc.chktex_command == "none") 
		fl_setpup_mode(EditMenu, 12, FL_PUP_GREY);

	if (tmpbuffer->isReadonly()) {
                fl_setpup_mode(EditMenu, 1, FL_PUP_GREY); 
                fl_setpup_mode(EditMenu, 2, FL_PUP_GREY); 
                fl_setpup_mode(EditMenu, 3, FL_PUP_GREY); 
                fl_setpup_mode(EditMenu, 5, FL_PUP_GREY); 
                fl_setpup_mode(EditMenu, 19, FL_PUP_GREY); 
                fl_setpup_mode(EditMenu, 20, FL_PUP_GREY);
	}
	
	fl_setpup_position(men->_view->getForm()->x + ob->x,
			   men->_view->getForm()->y + ob->y +
			   ob->h + 10);   
	int choice = fl_dopup(EditMenu);
	XFlush(fl_display);
   
	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);

	switch (choice) {
	case  1: tmpfunc->Dispatch(LFUN_UNDO); break;
	case  2: tmpfunc->Dispatch(LFUN_REDO); break;
	case  3: tmpfunc->Dispatch(LFUN_CUT); break;
	case  4: tmpfunc->Dispatch(LFUN_COPY); break;
	case  5: tmpfunc->Dispatch(LFUN_PASTE); break;
	case  6: tmpfunc->Dispatch(LFUN_MENUSEARCH); break;
	case  7: tmpfunc->Dispatch(LFUN_GOTOERROR); break;
	case  8: tmpfunc->Dispatch(LFUN_GOTONOTE); break;
	case  9: // floats & insets
		break;
	case 10:// table 
		break;
	case 11: tmpfunc->Dispatch(LFUN_SPELLCHECK); break;
	case 12: tmpfunc->Dispatch(LFUN_RUNCHKTEX); break;
	case 13: tmpfunc->Dispatch(LFUN_TOCVIEW); break;
	case 14: tmpfunc->Dispatch(LFUN_LOFVIEW); break;
	case 15: tmpfunc->Dispatch(LFUN_LOTVIEW); break;
	case 16: tmpfunc->Dispatch(LFUN_LOAVIEW); break;
	case 17: // version control
		break;
	case 18: tmpfunc->Dispatch(LFUN_LATEX_LOG); break;
	case 19: tmpfunc->Dispatch(LFUN_PASTESELECTION, "line"); break;
	case 20: tmpfunc->Dispatch(LFUN_PASTESELECTION, "paragraph"); break;

		// floats & insets sub-menu
	case 21: men->currentView()->toggleFloat(); break;
	case 22: tmpfunc->Dispatch(LFUN_MELT); break;
	case 23: men->currentView()->allFloats(1, 0); break;
	case 24: men->currentView()->allFloats(0, 0); break;
	case 25: men->currentView()->allFloats(1, 1); break;
	case 26: men->currentView()->allFloats(0, 1); break;
	case 27: tmpfunc->Dispatch(LFUN_REMOVEERRORS); break;

	case 31: tmpfunc->Dispatch(LFUN_TABLE); break;
		// this is really temporary. We need new function in keybind.C
		// These should set the minibuffer, too.
	case 32: case 33: case 34:
	case 35: case 36: case 37:
	case 38: case 39: case 40: 
	case 41: case 42: case 43:
	case 44: 
     		if (men->currentView()->available()) {
			men->currentView()->hideCursor();
			if (!men->currentView()->text->selection){
				men->currentView()->beforeChange(); 
				men->currentView()->update(BufferView::SELECT|BufferView::FITCUR);
			}
			if (men->currentView()->the_locking_inset &&
			    (men->currentView()->the_locking_inset->LyxCode()
			     == Inset::TABULAR_CODE)) {
				InsetTabular * inset =
				    static_cast<InsetTabular *>
				       (men->currentView()->the_locking_inset);
				inset->TabularFeatures(men->currentView(), choice - 32);
			} else {
				men->currentView()->text->
					TableFeatures(men->currentView(), choice - 32);
				men->currentView()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			}
		}
		break;
		// version control sub-menu
	case 51: // register
		tmpfunc->Dispatch(LFUN_VC_REGISTER);
		break;
	case 52: // check in
		tmpfunc->Dispatch(LFUN_VC_CHECKIN);
		break;
	case 53: // check out
		tmpfunc->Dispatch(LFUN_VC_CHECKOUT);
		break;
	case 54: // revert to last
		tmpfunc->Dispatch(LFUN_VC_REVERT);
		break;
	case 55: // undo last
		tmpfunc->Dispatch(LFUN_VC_UNDO);
		break;
	case 56: // show history
		tmpfunc->Dispatch(LFUN_VC_HISTORY);
		break;
	}

	fl_freepup(EditMenu);
	fl_freepup(SubEditFloats);
	fl_freepup(SubEditTable);
	fl_freepup(SubVersionControl);
}

vector<int>::size_type const max_number_of_menus = 32;
unsigned int const max_number_of_items = 25;

string Limit_string_length(string const & str) {
	string::size_type const max_item_length = 45;

	if (str.size() > max_item_length)
		return str.substr(0, max_item_length-3) + "...";
	else
		return str;
}

void Add_to_toc_menu(vector<Buffer::TocItem> const & toclist, 
		     unsigned int from, unsigned int to, int depth,
		     int menu, vector<int> & menus, FL_OBJECT * ob)
{
	if (to - from <= max_number_of_items) {
		for (unsigned int i = from; i < to; ++i) {
			string entry(4 * max(0, toclist[i].depth - depth),' ');
			entry += toclist[i].str;
			entry = Limit_string_length(entry);
			entry += "%x" + tostr(i + 1);
			fl_addtopup(menu, entry.c_str());
		}
	} else {
		unsigned int pos = from;
		unsigned int count = 0;
		while (pos < to) {
			++count;
			if (count > max_number_of_items) {
				fl_addtopup(menu, ". . .%d");
				break;
			}
			unsigned int new_pos = pos+1;
			while (new_pos < to &&
			       toclist[new_pos].depth > depth)
				++new_pos;

			string entry(4 * max(0, toclist[pos].depth - depth), ' ');
			entry += toclist[pos].str;
			entry = Limit_string_length(entry);

			if (new_pos == pos + 1) {
				entry += "%x";
				entry += tostr(pos + 1);
				fl_addtopup(menu, entry.c_str());
			} else if (menus.size() < max_number_of_menus) {
				int menu2 = fl_newpup(FL_ObjWin(ob));
				menus.push_back(menu2);
				Add_to_toc_menu(toclist, pos, new_pos,
						depth + 1, menu2, menus,ob);
				entry += "%m";
				fl_addtopup(menu, entry.c_str(), menu2);
			} else {
				entry += "%d";
				fl_addtopup(menu, entry.c_str());
			}
			pos = new_pos;
		}
	}
}

int const BIG_NUM = 1048576;

void Menus::ShowTocMenu(FL_OBJECT * ob, long)
{
	Menus * men = static_cast<Menus*>(ob->u_vdata);
	vector<int> menus;

	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);
   
	int TocMenu = fl_newpup(FL_ObjWin(ob));
	menus.push_back(TocMenu);
	vector<vector<Buffer::TocItem> > toclist =
		men->currentView()->buffer()->getTocList();

	//xgettext:no-c-format
	static char const * MenuNames[3] = { N_("List of Figures%m"),
	//xgettext:no-c-format
					     N_("List of Tables%m"),
	//xgettext:no-c-format
					     N_("List of Algorithms%m") };

	int max_nonempty = 0;
	for (int j = 1; j <= 3; ++j)
		if (!toclist[j].empty())
			max_nonempty = j;

	for (int j = 1; j <= 3; ++j)
		if (!toclist[j].empty()) {
			int menu2 = fl_newpup(FL_ObjWin(ob));
			menus.push_back(menu2);
			for (unsigned int i = 0; i < toclist[j].size(); ++i) {
				if (i > max_number_of_items) {
					fl_addtopup(menu2, ". . .%d");
					break;
				}
				string entry = Limit_string_length(toclist[j][i].str);
				entry += "%x" + tostr(i+1+j*BIG_NUM);
				fl_addtopup(menu2, entry.c_str());
			}
			if (j == max_nonempty) {
				string tmp = _(MenuNames[j-1]);
				tmp += "%l";
				fl_addtopup(TocMenu, tmp.c_str(), menu2);
			} else
				fl_addtopup(TocMenu, _(MenuNames[j-1]), menu2);
		}

	Add_to_toc_menu(toclist[0], 0, toclist[0].size(), 0,
			TocMenu, menus, ob);

	fl_setpup_position(
		men->_view->getForm()->x + ob->x,
		men->_view->getForm()->y + ob->y + ob->h + 10);   
	int choice = fl_dopup(TocMenu);
	XFlush(fl_display);

	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);
	if (choice == 0)
		men->_view->getLyXFunc()->Dispatch(LFUN_TOCVIEW);
	else if (choice > 0) {
		int type = choice / BIG_NUM;
		int num = (choice % BIG_NUM) - 1;
		BufferView *bv = men->currentView();
		bv->beforeChange();
		bv->text->SetCursor(bv, toclist[type][num].par, 0);
		bv->text->sel_cursor = bv->text->cursor;
		bv->update(BufferView::SELECT|BufferView::FITCUR);
	}
	for (unsigned int i = 0; i < menus.size(); ++i)
		fl_freepup(menus[i]);
}

void Add_to_refs_menu(vector<string> const & label_list, int offset,
		     int menu, vector<int> & menus, FL_OBJECT * ob)
{
	typedef vector<string>::size_type size_type;
	size_type const max_number_of_items = 25;
	size_type const max_number_of_items2 = 20;
	string::size_type const max_item_length = 40;
	string::size_type const max_item_length2 = 20;

	if (label_list.size() <= max_number_of_items)
		for (size_type i = 0; i < label_list.size(); ++i) {
			string entry = label_list[i];
			if (entry.size() > max_item_length)
				entry = entry.substr(0, max_item_length-1) + "$";
			entry += "%x" + tostr(i+offset);
			fl_addtopup(menu, entry.c_str());
		}
	else {
		size_type count = 0;
		for (size_type i = 0; i < label_list.size();
		     i += max_number_of_items2) {
			++count;
			if (count > max_number_of_items) {
				fl_addtopup(menu, ". . .%d");
				break;
			}
			size_type j = min(label_list.size(),
					  i+max_number_of_items2);

			string entry;
			if (label_list[i].size() > max_item_length2)
				entry += label_list[i].substr(0, max_item_length2-1) + "$";
			else
				entry += label_list[i];
			entry += "..";
			if (label_list[j-1].size() > max_item_length2)
				entry += label_list[j-1].substr(0, max_item_length2-1) + "$";
			else
				entry += label_list[j-1];

			if (menus.size() < max_number_of_menus) {
				int menu2 = fl_newpup(FL_ObjWin(ob));
				menus.push_back(menu2);
				for (size_type k = i;  k < j; ++k) {
					string entry2 = label_list[k];
					if (entry2.size() > max_item_length)
						entry2 = entry2.substr(0, max_item_length-1) + "$";
					entry2 += "%x" + tostr(k+offset);
					fl_addtopup(menu2, entry2.c_str());
				}
				entry += "%m";
				fl_addtopup(menu, entry.c_str(), menu2);
			} else {
				entry += "%d";
				fl_addtopup(menu, entry.c_str());
			}
		}
	}
}

void Menus::ShowRefsMenu(FL_OBJECT * ob, long)
{
	vector<int> menus;

	Menus * men = static_cast<Menus*>(ob->u_vdata);

	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);
   
	int RefsMenu = fl_newpup(FL_ObjWin(ob));
	menus.push_back(RefsMenu);
	Buffer * buffer = men->currentView()->buffer();
	vector<string> label_list = buffer->getLabelList();
	sort(label_list.begin(), label_list.end());

	//xgettext:no-c-format
	static char const * MenuNames[6] = { N_("Insert Reference%m"),
	//xgettext:no-c-format
					     N_("Insert Page Number%m"),
	//xgettext:no-c-format
					     N_("Insert vref%m"),
	//xgettext:no-c-format
					     N_("Insert vpageref%m"),
	//xgettext:no-c-format
					     N_("Insert Pretty Ref%m"),
	//xgettext:no-c-format
					     N_("Goto Reference%m") };

	for (int j = 0; j < 6; ++j) {
		if (menus.size() < max_number_of_menus) {
			int menu2 = fl_newpup(FL_ObjWin(ob));
			menus.push_back(menu2);
			Add_to_refs_menu(label_list, (j+1)*BIG_NUM, menu2, menus, ob);
			fl_addtopup(RefsMenu, _(MenuNames[j]), menu2);
		} else {
			string tmp = _(MenuNames[j]);
			tmp += "%d";
			fl_addtopup(RefsMenu, tmp.c_str());	
		}
	}
	fl_addtopup(RefsMenu, _("Go Back"));

	bool empty = label_list.empty();
	bool sgml = buffer->isSGML();
	bool readonly = buffer->isReadonly();

	if (empty)
		fl_setpup_mode(RefsMenu, 6, FL_PUP_GREY);
	if (empty || readonly) {
		fl_setpup_mode(RefsMenu, 1, FL_PUP_GREY);
		fl_setpup_mode(RefsMenu, 2, FL_PUP_GREY);
	}
	if (empty || readonly || sgml) {
		fl_setpup_mode(RefsMenu, 3, FL_PUP_GREY);
		fl_setpup_mode(RefsMenu, 4, FL_PUP_GREY);
		fl_setpup_mode(RefsMenu, 5, FL_PUP_GREY);
	}
	if (men->currentView()->NoSavedPositions())
		fl_setpup_mode(RefsMenu, 7, FL_PUP_GREY);	

	fl_setpup_position(
		men->_view->getForm()->x + ob->x,
		men->_view->getForm()->y + ob->y + ob->h + 10);   
	int choice = fl_dopup(RefsMenu);
	XFlush(fl_display);

	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);

	if (choice == 7)
		men->_view->getLyXFunc()->Dispatch(LFUN_REFBACK);
	else if (choice >= BIG_NUM) {
		int type = (choice / BIG_NUM) - 1;
		int num = choice % BIG_NUM;
		if (type >= 5)
			men->_view->getLyXFunc()->Dispatch(LFUN_REFGOTO,
							   label_list[num].c_str());
		else {
			static string const commands[5]
				= { "\\ref", "\\pageref", "\\vref", "\\vpageref",
				    "\\prettyref"};
			string t = commands[type] + "{" + label_list[num] + "}";
			men->currentView()->insertInset(new InsetRef(t, buffer));
		}
	}

	for (unsigned int i = 0; i < menus.size(); ++i)
		fl_freepup(menus[i]);
}


void Menus::ShowLayoutMenu(FL_OBJECT * ob, long)
{
	Menus * men = static_cast<Menus*>(ob->u_vdata);

	if (!men->currentView()->available())
		return;
	
	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);
   
	Buffer * tmpbuffer = men->_view->buffer();
	LyXFunc * tmpfunc = men->_view->getLyXFunc();

	int LayoutMenu = fl_newpup(FL_ObjWin(ob));
	fl_addtopup(LayoutMenu, _("Character..."
				  "|Paragraph..."
				  "|Document..."
				  "|Paper..."
				  "|Table..."
				  "|Quotes...%l"
				  "|Emphasize Style%b"
				  "|Noun Style%b"
				  "|Bold Style%b"
				  "|TeX Style%b%l"
				  "|Change Environment Depth"
				  "|LaTeX Preamble..."
				  "|Start of Appendix%l"
				  "|Save layout as default"));
	fl_setpup_shortcut(LayoutMenu, 1, scex(_("LM|Cc#c#C")));
	fl_setpup_shortcut(LayoutMenu, 2, scex(_("LM|Pp#p#P")));
	fl_setpup_shortcut(LayoutMenu, 3, scex(_("LM|Dd#d#D")));
 	fl_setpup_shortcut(LayoutMenu, 4, scex(_("LM|aA#a#A")));
 	fl_setpup_shortcut(LayoutMenu, 5, scex(_("LM|eE#e#E")));
	fl_setpup_shortcut(LayoutMenu, 6, scex(_("LM|Qq#q#Q")));
	fl_setpup_shortcut(LayoutMenu, 7, scex(_("LM|mM#m#M")));
	fl_setpup_shortcut(LayoutMenu, 8, scex(_("LM|Nn#n#N")));
	fl_setpup_shortcut(LayoutMenu, 9, scex(_("LM|Bb#b#B")));
	fl_setpup_shortcut(LayoutMenu, 10, scex(_("LM|Tt#t#T")));
	fl_setpup_shortcut(LayoutMenu, 11, scex(_("LM|vV#v#V")));
	fl_setpup_shortcut(LayoutMenu, 12, scex(_("LM|Ll#l#L")));
	fl_setpup_shortcut(LayoutMenu, 13, scex(_("LM|xX#x#X")));
	fl_setpup_shortcut(LayoutMenu, 14, scex(_("LM|Ss#s#S")));

	// Set values of checkboxes according to font
	LyXFont font = men->currentView()->text->real_current_font;
	if (font.emph() == LyXFont::ON)
		fl_setpup_mode(LayoutMenu, 7, FL_PUP_CHECK);
	if (font.noun() == LyXFont::ON)
		fl_setpup_mode(LayoutMenu, 8, FL_PUP_CHECK);
	if (font.series() == LyXFont::BOLD_SERIES)
		fl_setpup_mode(LayoutMenu, 9, FL_PUP_CHECK);
	if (font.latex() == LyXFont::ON)
		fl_setpup_mode(LayoutMenu, 10, FL_PUP_CHECK);
	   
	// Grey out unavailable entries
	if (!men->currentView()->text->cursor.par()->table)
		fl_setpup_mode(LayoutMenu, 5, FL_PUP_GREY);

	if (tmpbuffer->isReadonly()) {
	        fl_setpup_mode(LayoutMenu, 1, FL_PUP_GREY);
		fl_setpup_mode(LayoutMenu, 6, FL_PUP_GREY);
		fl_setpup_mode(LayoutMenu, 11, FL_PUP_GREY);
	}

	fl_setpup_position(
		men->_view->getForm()->x + ob->x,
		men->_view->getForm()->y + ob->y + ob->h + 10);
	int choice = fl_dopup(LayoutMenu);
	XFlush(fl_display);

	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);

	switch (choice) {
	case 1:  tmpfunc->Dispatch(LFUN_LAYOUT_CHARACTER); break;
	case 2:  tmpfunc->Dispatch(LFUN_LAYOUT_PARAGRAPH); break;
	case 3:	 tmpfunc->Dispatch(LFUN_LAYOUT_DOCUMENT);  break;
	case 4:	 tmpfunc->Dispatch(LFUN_LAYOUT_PAPER); break;
	case 5:  tmpfunc->Dispatch(LFUN_LAYOUT_TABLE, "true"); break;
	case 6:  tmpfunc->Dispatch(LFUN_LAYOUT_QUOTES); break;
	case 7:  tmpfunc->Dispatch(LFUN_EMPH); break;
	case 8:  tmpfunc->Dispatch(LFUN_NOUN); break;
	case 9:  tmpfunc->Dispatch(LFUN_BOLD); break;
	case 10: tmpfunc->Dispatch(LFUN_TEX); break;
	case 11: tmpfunc->Dispatch(LFUN_DEPTH_PLUS); break;
	case 12: tmpfunc->Dispatch(LFUN_LAYOUT_PREAMBLE); break;
	case 13: tmpfunc->Dispatch(LFUN_APPENDIX); break;
	case 14: tmpfunc->Dispatch(LFUN_LAYOUT_SAVE_DEFAULT); break;
	}
	fl_freepup(LayoutMenu); 
}


void Menus::ShowInsertMenu(FL_OBJECT * ob, long)
{
	Menus * men = static_cast<Menus*>(ob->u_vdata);
	
	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);
 
	Buffer * tmpbuffer = men->_view->buffer();
	LyXFunc * tmpfunc = men->_view->getLyXFunc();

	int SubInsertAscii = fl_defpup(FL_ObjWin(ob),
				       _("Import ASCII file%t"
					 "|As Lines%x41"
					 "|As Paragraphs%x42"));
	
	fl_setpup_shortcut(SubInsertAscii, 41, scex(_("IMA|Ll#l#L")));
	fl_setpup_shortcut(SubInsertAscii, 42, scex(_("IMA|Pp#p#P")));

	int SubInsertTableList= fl_defpup(FL_ObjWin(ob),
					  _("Lists & TOC%t"
					    "|Table of Contents%x21"
					    "|List of Figures%x22"
					    "|List of Tables%x23"
					    "|List of Algorithms%x24"
					    "|Index List%x25"
					    "|BibTeX Reference%x26"));
	
	fl_setpup_shortcut(SubInsertTableList, 21, scex(_("IMT|Cc#c#C")));
	fl_setpup_shortcut(SubInsertTableList, 22, scex(_("IMT|Ff#f#F")));
	fl_setpup_shortcut(SubInsertTableList, 23, scex(_("IMT|Tt#t#T")));
	fl_setpup_shortcut(SubInsertTableList, 24, scex(_("IMT|Aa#a#A")));
	fl_setpup_shortcut(SubInsertTableList, 25, scex(_("IMT|Ii#i#I")));
	fl_setpup_shortcut(SubInsertTableList, 26, scex(_("IMT|Bb#b#B")));

	int SubInsertFloatList = fl_defpup(FL_ObjWin(ob),
					   _("Floats%t"
					     "|Figure Float%x71"
					     "|Table Float%x72"
					     "|Wide Figure Float%x73"
					     "|Wide Table Float%l%x74"
					     "|Algorithm Float%x75"));
	
	fl_setpup_shortcut(SubInsertFloatList, 71, scex(_("IMF|gG#g#G")));
	fl_setpup_shortcut(SubInsertFloatList, 72, scex(_("IMF|Tt#t#T")));
	fl_setpup_shortcut(SubInsertFloatList, 73, scex(_("IMF|Ww#w#W")));
	fl_setpup_shortcut(SubInsertFloatList, 74, scex(_("IMF|iI#i#I")));
	fl_setpup_shortcut(SubInsertFloatList, 75, scex(_("IMF|Aa#a#A")));
 	
	int SubInsertSpecial = fl_defpup(FL_ObjWin(ob),
					 _("Special Character%t"
					   "|HFill%x31"
					   "|Hyphenation Point%x32"
					   "|Protected Blank%x33"
					   "|Linebreak%x34"
					   "|Ellipsis (...)%x35"
					   "|End of sentence period%x36"
					   "|Ordinary Quote (\")%x37"
					   "|Menu Separator %x38"));

	fl_setpup_shortcut(SubInsertSpecial, 31, scex(_("IMS|Hh#h#H")));
	fl_setpup_shortcut(SubInsertSpecial, 32, scex(_("IMS|Pp#p#P")));
	fl_setpup_shortcut(SubInsertSpecial, 33, scex(_("IMS|Bb#b#B")));
	fl_setpup_shortcut(SubInsertSpecial, 34, scex(_("IMS|Ll#l#L")));
	fl_setpup_shortcut(SubInsertSpecial, 35, scex(_("IMS|iI#i#I")));
	fl_setpup_shortcut(SubInsertSpecial, 36, scex(_("IMS|Ee#e#E")));
	fl_setpup_shortcut(SubInsertSpecial, 37, scex(_("IMS|Qq#q#Q")));
	fl_setpup_shortcut(SubInsertSpecial, 38, scex(_("IMS|Mm#m#M")));
	
	int InsertMenu = fl_defpup(FL_ObjWin(ob),
				   _("Figure..."
				     "|Table...%l"
				     "|Include File..." 
				     "|Import ASCII File%m"
				     "|Insert LyX File..."
				     "|Insert external material...%l"
				     "|Footnote"
				     "|Margin Note"
				     "|Floats%m%l"      
				     "|Lists & TOC%m%l"
				     "|Special Character%m%l"
				     "|Note..."
				     "|Label..."
				     "|Cross-Reference..."
				     "|Citation Reference..."
     				     "|Index entry..."
     				     "|Index entry of last word"),
				   SubInsertAscii,
				   SubInsertFloatList, 	
				   SubInsertTableList,
				   SubInsertSpecial);

	fl_setpup_shortcut(InsertMenu, 1, scex(_("IM|gG#g#G")));
	fl_setpup_shortcut(InsertMenu, 2, scex(_("IM|bB#b#B")));
	fl_setpup_shortcut(InsertMenu, 3, scex(_("IM|cC#c#C")));
	fl_setpup_shortcut(InsertMenu, 4, scex(_("IM|Aa#a#A")));
	fl_setpup_shortcut(InsertMenu, 5, scex(_("IM|Xx#x#X")));
	fl_setpup_shortcut(InsertMenu, 6, scex(_("IM|Ee#e#E")));
	fl_setpup_shortcut(InsertMenu, 7, scex(_("IM|Ff#f#F")));
	fl_setpup_shortcut(InsertMenu, 8, scex(_("IM|Mm#m#M")));
	fl_setpup_shortcut(InsertMenu, 9, scex(_("IM|oO#o#O")));
	fl_setpup_shortcut(InsertMenu, 10, scex(_("IM|Tt#t#T")));
	fl_setpup_shortcut(InsertMenu, 11, scex(_("IM|Ss#s#S")));
	fl_setpup_shortcut(InsertMenu, 12, scex(_("IM|Nn#n#N")));
	fl_setpup_shortcut(InsertMenu, 13, scex(_("IM|Ll#l#L")));
	fl_setpup_shortcut(InsertMenu, 14, scex(_("IM|rR#r#R")));     
	fl_setpup_shortcut(InsertMenu, 15, scex(_("IM|iI#i#I")));
	fl_setpup_shortcut(InsertMenu, 16, scex(_("IM|dD#d#D")));
	fl_setpup_shortcut(InsertMenu, 17, scex(_("IM|wW#w#W")));

	fl_addtopup(InsertMenu, _("|URL..."));
	fl_setpup_shortcut(InsertMenu, 18, scex(_("IM|Uu#u#U")));

	if (tmpbuffer->isReadonly()) {
                for (int ii = 1; ii <= 17; ++ii)
	                fl_setpup_mode(InsertMenu, ii, FL_PUP_GREY);
		fl_setpup_mode(InsertMenu, 18, FL_PUP_GREY);
	}

	fl_setpup_position(
		men->_view->getForm()->x + ob->x,
		men->_view->getForm()->y + ob->y + ob->h + 10);
   
	int choice = fl_dopup(InsertMenu);
	XFlush(fl_display);

	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);

	if (men->currentView()->available()){
		men->currentView()->hideCursor();
		switch (choice) {
		case 1: tmpfunc->Dispatch(LFUN_FIGURE); break;
		case 2: tmpfunc->Dispatch(LFUN_TABLE); break;
		case 3: tmpfunc->Dispatch(LFUN_CHILDINSERT); break;
		case 4: // Insert ASCII file submenu
			break;
		case 5: tmpfunc->Dispatch(LFUN_FILE_INSERT); break;
		case 41: tmpfunc->Dispatch(LFUN_FILE_INSERT_ASCII, "line"); break;
		case 42: tmpfunc->Dispatch(LFUN_FILE_INSERT_ASCII, "paragraph"); break;
		case 43: 
			break;

		case 6: tmpfunc->Dispatch(LFUN_INSET_EXTERNAL); break;

		case 7: tmpfunc->Dispatch(LFUN_FOOTMELT); break
								  ;
		case 8: tmpfunc->Dispatch(LFUN_MARGINMELT); break;
  
		case 9: // Float sub-menu
                case 71:
			tmpfunc->Dispatch(LFUN_INSERTFOOTNOTE, "figure");
			break;
		case 72:
			tmpfunc->Dispatch(LFUN_INSERTFOOTNOTE, "table");
			break;
		case 73:
			tmpfunc->Dispatch(LFUN_INSERTFOOTNOTE, "wide-fig");
			break;
		case 74:
			tmpfunc->Dispatch(LFUN_INSERTFOOTNOTE, "wide-tab");
			break;
		case 75:
			tmpfunc->Dispatch(LFUN_INSERTFOOTNOTE, "algorithm");
			break;

		case 10: // Table/List submenu
			break;
		case 21: tmpfunc->Dispatch(LFUN_TOC_INSERT); break;
		case 22: tmpfunc->Dispatch(LFUN_LOF_INSERT); break;
		case 23: tmpfunc->Dispatch(LFUN_LOT_INSERT); break;
		case 24: tmpfunc->Dispatch(LFUN_LOA_INSERT); break;
		case 25: tmpfunc->Dispatch(LFUN_INDEX_PRINT); break;
		case 26: tmpfunc->Dispatch(LFUN_INSERT_BIBTEX); break;

		case 11: // Special Character submenu
			break;
		case 31: tmpfunc->Dispatch(LFUN_HFILL); break;
		case 32: tmpfunc->Dispatch(LFUN_HYPHENATION); break;
		case 33: tmpfunc->Dispatch(LFUN_PROTECTEDSPACE); break; 
		case 34: tmpfunc->Dispatch(LFUN_BREAKLINE); break; 
		case 35: tmpfunc->Dispatch(LFUN_LDOTS); break;
		case 36: tmpfunc->Dispatch(LFUN_END_OF_SENTENCE); break;
		case 37: tmpfunc->Dispatch(LFUN_QUOTE); break;
		case 38: tmpfunc->Dispatch(LFUN_MENU_SEPARATOR); break;

		case 12: tmpfunc->Dispatch(LFUN_INSERT_NOTE); break;
		case 13: tmpfunc->Dispatch(LFUN_INSERT_LABEL); break;
		case 14: tmpfunc->Dispatch(LFUN_INSERT_REF); break;
		case 15: tmpfunc->Dispatch(LFUN_CREATE_CITATION); break;
		case 16: tmpfunc->Dispatch(LFUN_INDEX_INSERT); break;
		case 17: tmpfunc->Dispatch(LFUN_INDEX_INSERT_LAST); break;
		case 18: tmpfunc->Dispatch(LFUN_URL); break;
		}
	}
	fl_freepup(InsertMenu);
	fl_freepup(SubInsertAscii);
	fl_freepup(SubInsertTableList);
	fl_freepup(SubInsertFloatList);
	fl_freepup(SubInsertSpecial);
}


void Menus::ShowMathMenu(FL_OBJECT * ob, long)
{
	extern void math_insert_symbol(char const * s);

	Menus * men = static_cast<Menus*>(ob->u_vdata);

	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);

	Buffer * tmpbuffer = men->_view->buffer();
	LyXFunc * tmpfunc = men->_view->getLyXFunc();

	int MathMenu = fl_defpup(FL_ObjWin(ob), 
				 _("Fraction"
				   "|Square root"
				   "|Exponent"
				   "|Index"
				   "|Sum"
				   "|Integral%l"
				   "|Math mode"
				   "|Display%l"
				   "|Math Panel..."));

	fl_setpup_shortcut(MathMenu, 1, scex(_("MM|Ff#f#F")));
	fl_setpup_shortcut(MathMenu, 2, scex(_("MM|Ss#s#S")));
	fl_setpup_shortcut(MathMenu, 3, scex(_("MM|Ee#e#E")));
	fl_setpup_shortcut(MathMenu, 4, scex(_("MM|xX#x#X")));
	fl_setpup_shortcut(MathMenu, 5, scex(_("MM|uU#u#U")));
	fl_setpup_shortcut(MathMenu, 6, scex(_("MM|Ii#i#I")));
	fl_setpup_shortcut(MathMenu, 7, scex(_("MM|Mm#m#M")));
	fl_setpup_shortcut(MathMenu, 8, scex(_("MM|Dd#d#D")));
	fl_setpup_shortcut(MathMenu, 9, scex(_("MM|Pp#p#P")));

	if (tmpbuffer->isReadonly()) 
                for (int ii = 1; ii <= 9; ++ii)
	                fl_setpup_mode(MathMenu, ii, FL_PUP_GREY);


	fl_setpup_position(
		men->_view->getForm()->x + ob->x,
		men->_view->getForm()->y + ob->y + ob->h + 10);   
	int choice = fl_dopup(MathMenu);  
	XFlush(fl_display);

	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);

	if (men->currentView()->available())  {
		switch (choice) {
		case 1: /* frac */
			tmpfunc->Dispatch(LFUN_INSERT_MATH, "frac");
			break;
		case 2: /* sqrt */
			tmpfunc->Dispatch(LFUN_INSERT_MATH, "sqrt");
			break;
		case 3: /* Exponent */ 
			tmpfunc->Dispatch(LFUN_INSERT_MATH, "^");
			break;
		case 4: /* Index */
			tmpfunc->Dispatch(LFUN_INSERT_MATH, "_");
			break;
		case 5: /* sum */ 
			tmpfunc->Dispatch(LFUN_INSERT_MATH, "sum");
			break;
		case 6: /* int */
			tmpfunc->Dispatch(LFUN_INSERT_MATH, "int");
			break;
		case 7:
			tmpfunc->Dispatch(LFUN_MATH_MODE);
			break;
		case 8:
			tmpfunc->Dispatch(LFUN_MATH_DISPLAY);
			break;
		case 9: /* Panel */
			show_symbols_form(tmpfunc);
			break;
		}
		men->currentView()->update(BufferView::SELECT|BufferView::FITCUR);
	} 
	fl_freepup(MathMenu);
}


void Menus::ShowOptionsMenu(FL_OBJECT * ob, long)
{
	Menus * men = static_cast<Menus*>(ob->u_vdata);

	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);

	//Buffer *tmpbuffer = men->_view->currentBuffer();
	LyXFunc * tmpfunc = men->_view->getLyXFunc();

	int OptionsMenu = fl_defpup(FL_ObjWin(ob),
				    _("Screen Fonts..."
				      "|Spellchecker Options..."
				      "|Keyboard...%l"
				      "|Reconfigure"
				      "|Preferences"));

	fl_setpup_shortcut(OptionsMenu, 1, scex(_("OM|Ff#f#F")));
	fl_setpup_shortcut(OptionsMenu, 2, scex(_("OM|Ss#s#S")));
	fl_setpup_shortcut(OptionsMenu, 3, scex(_("OM|Kk#k#K")));
	fl_setpup_shortcut(OptionsMenu, 4, scex(_("OM|Rr#r#R")));
	fl_setpup_shortcut(OptionsMenu, 5, scex(_("OM|Pp#p#P")));

	if(lyxrc.isp_command == "none") 
		fl_setpup_mode(OptionsMenu, 2, FL_PUP_GREY);

	fl_setpup_position(
		men->_view->getForm()->x + ob->x,
		men->_view->getForm()->y + ob->y + ob->h + 10);   
	int choice = fl_dopup(OptionsMenu);
	XFlush(fl_display);

	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);
	switch (choice){
	case 1: men->ScreenOptions(); break;
	case 2: SpellCheckerOptions(); break;      
	case 3: men->_view->getIntl()->MenuKeymap(); break;
	case 4: tmpfunc->Dispatch(LFUN_RECONFIGURE); break;
	case 5: men->_view->getDialogs()->showPreferences(); break;
	default: break;
	}   
	fl_freepup(OptionsMenu);
}


void Menus::ShowBufferMenu(FL_OBJECT * ob, long)
{
	Menus * men = static_cast<Menus*>(ob->u_vdata);
	
	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);
   
	int BufferMenu = fl_newpup(FL_ObjWin(ob));
	vector<string> namevec = bufferlist.getFileNames();
	if (namevec.size() != 0) {
		for (vector<string>::const_iterator cit = namevec.begin();
		     cit != namevec.end(); ++cit) {
			string relbuf = MakeDisplayPath((*cit), 30);
			fl_addtopup(BufferMenu, relbuf.c_str());
		}
	} else {
		fl_addtopup(BufferMenu, _("No Documents Open!%t"));
	}
   
	fl_setpup_position(
		men->_view->getForm()->x + ob->x,
		men->_view->getForm()->y + ob->y + ob->h + 10);   
	int choice = fl_dopup(BufferMenu);
	XFlush(fl_display);

	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);
	if (choice > 0) men->handleBufferMenu(choice - 1);
   
	fl_freepup(BufferMenu);
}


static
char const * doc_files [] = {"Intro", "Tutorial", 
			     "UserGuide", "Extended",
			     "Customization", "Reference",
			     "FAQ", "TOC",  
			     "BUGS", "LyXConfig"}; 

void Menus::ShowHelpMenu(FL_OBJECT * ob, long)
{
	Menus * men = static_cast<Menus*>(ob->u_vdata);
	LyXFunc * tmpfunc = men->_view->getLyXFunc();

	// set the pseudo menu-button
	fl_set_object_boxtype(ob, FL_UP_BOX);
	fl_set_button(ob, 0);
	fl_redraw_object(ob);
   
	int HelpMenu = fl_defpup(FL_ObjWin(ob),
				 _("Introduction"
				   "|Tutorial"
				   "|User's Guide"
				   "|Extended Features"
				   "|Customization"
				   "|Reference Manual"
				   "|FAQ"
				   "|Table of Contents"
				   "|Known Bugs"
				   "|LaTeX Configuration%l"
				   "|Copyright and Warranty..."
				   "|Credits..."
				   "|Version..."));
   
	fl_setpup_shortcut(HelpMenu,  1, scex(_("HM|Ii#I#i")));
	fl_setpup_shortcut(HelpMenu,  2, scex(_("HM|Tt#T#t")));
	fl_setpup_shortcut(HelpMenu,  3, scex(_("HM|Uu#U#u")));
	fl_setpup_shortcut(HelpMenu,  4, scex(_("HM|xX#x#X")));
	fl_setpup_shortcut(HelpMenu,  5, scex(_("HM|Cc#C#c")));
	fl_setpup_shortcut(HelpMenu,  6, scex(_("HM|Rr#R#r")));
	fl_setpup_shortcut(HelpMenu,  7, scex(_("HM|Ff#F#f")));
	fl_setpup_shortcut(HelpMenu,  8, scex(_("HM|aA#a#A")));
	fl_setpup_shortcut(HelpMenu,  9, scex(_("HM|Kk#K#k")));
	fl_setpup_shortcut(HelpMenu, 10, scex(_("HM|Ll#L#l")));
	fl_setpup_shortcut(HelpMenu, 11, scex(_("HM|oO#o#O")));
	fl_setpup_shortcut(HelpMenu, 12, scex(_("HM|eE#e#E")));
	fl_setpup_shortcut(HelpMenu, 13, scex(_("HM|Vv#v#V")));

	fl_setpup_position(
		men->_view->getForm()->x + ob->x,
		men->_view->getForm()->y + ob->y + ob->h + 10);   
	int choice = fl_dopup(HelpMenu);
	XFlush(fl_display);

	// set the pseudo menu-button back
	fl_set_object_boxtype(ob, FL_FLAT_BOX);
	fl_redraw_object(ob);

	switch (choice) {
	case 1: case 2: case 3: case 4: case 5: 
	case 6: case 7: case 8: case 9: case 10:
		ProhibitInput(men->currentView());
		men->MenuDocu(doc_files[choice - 1]);
		AllowInput(men->currentView());
		break;
	case 11: tmpfunc->Dispatch(LFUN_HELP_COPYRIGHT); break;
	case 12: ShowCredits(); break;
	case 13:
		ProhibitInput(men->currentView());
		fl_show_message((string(_("LyX Version ")) + LYX_VERSION 
				 + _(" of ") + LYX_RELEASE).c_str(),
				(_("Library directory: ")
				 + MakeDisplayPath(system_lyxdir)).c_str(),
				(_("User directory: ") 
				 + MakeDisplayPath(user_lyxdir)).c_str());
		AllowInput(men->currentView());
		break;
	}
	fl_freepup(HelpMenu);
}


void Menus::MenuDocu(string const & docname) 
{
	string fname = i18nLibFileSearch("doc", docname, "lyx");
	if (fname.empty()) {
		WriteAlert(_("Error!"),
			   _("Could not find requested Documentation file"),
			   fname);
		return;
	}
	_view->getMiniBuffer()->Set(_("Opening help file"),
				    MakeDisplayPath(fname), "...");
	currentView()->buffer(bufferlist.loadLyXFile(fname, false));

	if (docname == "Reference")
		_view->getLyXFunc()->Dispatch(LFUN_TOCVIEW);
}


void Menus::handleBufferMenu(int choice)
{
	currentView()->buffer(bufferlist.getBuffer(choice));
}
