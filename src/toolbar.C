/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 *           This file is Copyright 1996-1998
 *           Lars Gullik Bjønnes
 *
 * ====================================================== */

//  Added pseudo-action handling, asierra 180296

#include <config.h>

#ifdef __GNUG__
#pragma implementation "toolbar.h"
#endif

#include "lyx_main.h"
#include "lyx_gui_misc.h"
#include "lyx.h"
#include "toolbar.h"
#include "lyxfunc.h"
#include "lyxlex.h"
#include "debug.h"
#include "combox.h"
#include "lyx_cb.h"
#include "LyXView.h"
#include "support/lstrings.h"

#ifdef TWO_COLOR_ICONS
#include "cut_bw.xpm"
#include "emph_bw.xpm"
#include "fig_bw.xpm"
#include "foot_bw.xpm"
#include "math_bw.xpm"
#include "depth_bw.xpm"
#include "margin_bw.xpm"
#include "melt_bw.xpm"
#include "copy_bw.xpm"
#include "noun_bw.xpm"
#include "paste_bw.xpm"
#include "free_bw.xpm"
#include "tab_bw.xpm"
#include "tex_bw.xpm"
#include "open_bw.xpm"
#include "close_bw.xpm"
#include "save_bw.xpm"
#include "print1_bw.xpm"
#include "quit_bw.xpm"
#include "typeset_ps_bw.xpm"
#include "unknown_bw.xpm"
#else 
#include "cut.xpm"
#include "emph.xpm"
#include "fig.xpm"
#include "foot.xpm"
#include "math.xpm"
#include "depth.xpm"
#include "margin.xpm"
#include "melt.xpm"
#include "copy.xpm"
#include "noun.xpm"
#include "paste.xpm"
#include "free.xpm"
#include "tab.xpm"
#include "tex.xpm"
#include "open.xpm"
#include "close.xpm"
#include "save.xpm"
#include "print1.xpm"
#include "quit.xpm"
#include "typeset_ps.xpm"
#include "unknown.xpm"
#endif

// These pixmaps are the same regardless of color:
#include "bold_bw.xpm"
#include "make_ascii_bw.xpm"
#include "make_latex_bw.xpm"
#include "run_latex_bw.xpm"
#include "sans_bw.xpm"
#include "view_dvi_bw.xpm"
#include "view_ps_bw.xpm"
#include "layout_code.xpm"
#include "layout_latex.xpm"
#include "layout_scrap.xpm"
#include "layout_sec.xpm"
#include "layout_std.xpm"
#include "build.xpm"

// this one is not "C" because combox callbacks are really C++ %-|
extern void LayoutsCB(int, void*);
extern char** get_pixmap_from_symbol(char const *arg, int, int);
extern LyXAction lyxaction;


enum _tooltags {
	TO_ADD = 1,
	TO_ENDTOOLBAR,
        TO_SEPARATOR,
        TO_LAYOUTS,
        TO_NEWLINE,
	TO_LAST
};


struct keyword_item toolTags[TO_LAST-1] = {
	{ "\\add", TO_ADD },
	{ "\\end_toolbar", TO_ENDTOOLBAR },
        { "\\layouts", TO_LAYOUTS },
        { "\\newline", TO_NEWLINE },
        { "\\separator", TO_SEPARATOR }
};


Toolbar::Toolbar(Toolbar const &rct, LyXView *o, int x, int y)
	: owner(o), sxpos(x), sypos(y)
{
	combox = 0;
	bubble_timer = 0;
	reset();

	// extracts the toolbar struct form rct.
	toolbarItem *tmplist = rct.toollist;
	while (tmplist != 0) {
		add(tmplist->action);
		lyxerr[Debug::TOOLBAR] << "tool action: "
				       << tmplist->action << endl;
		tmplist= tmplist->next;
	}
}


// timer-cb for bubble-help (Matthias)
void Toolbar::BubbleTimerCB(FL_OBJECT *, long data)
{
	FL_OBJECT* ob = (FL_OBJECT*) data;
	char* help = (char*) ob->u_vdata;
	fl_show_oneliner(help, ob->form->x + ob->x,
			 ob->form->y + ob->y + ob->h);
}

extern "C" void C_Toolbar_BubbleTimerCB(FL_OBJECT *ob, long data)
{
	Toolbar::BubbleTimerCB(ob, data);
}

// post_handler for bubble-help (Matthias)
int Toolbar::BubblePost(FL_OBJECT *ob, int event,
	     FL_Coord /*mx*/, FL_Coord /*my*/, int /*key*/, void */*xev*/)
{
	string help = (char *)ob->u_vdata;
	Toolbar *t = (Toolbar*)ob->u_ldata;
	
	if(event == FL_ENTER && !help.empty()){
		fl_set_object_callback(t->bubble_timer,
				       C_Toolbar_BubbleTimerCB, (long) ob);
		fl_set_timer(t->bubble_timer, 1);
	}
	else if(event != FL_MOTION){
		fl_set_timer(t->bubble_timer, 0);
		fl_hide_oneliner();
	}
	return 0;
}

extern "C" int C_Toolbar_BubblePost(FL_OBJECT *ob, int event,
				   FL_Coord /*mx*/, FL_Coord /*my*/, 
				   int key, void *xev)
{
	return Toolbar::BubblePost(ob, event, 0, 0, key, xev);
}

void Toolbar::activate()
{
	toolbarItem *item, *tmp= 0;
	item = toollist;
	while(item){
		tmp = item->next;
		if (item->icon) {
			fl_activate_object(item->icon);
		}
		item = tmp;
	}
}


void Toolbar::deactivate()
{
	toolbarItem *item, *tmp= 0;
	item = toollist;
	while(item){
		tmp = item->next;
		if (item->icon) {
			fl_deactivate_object(item->icon);
		}
		item = tmp;
	}
}


void Toolbar::ToolbarCB(FL_OBJECT *ob, long ac)
{
	Toolbar *t = (Toolbar*)ob->u_ldata;
	
	string res = t->owner->getLyXFunc()->Dispatch(int(ac));
	if(!res.empty())
		lyxerr[Debug::TOOLBAR] << res << endl;
}

extern "C" void C_Toolbar_ToolbarCB(FL_OBJECT *ob, long data)
{
	Toolbar::ToolbarCB(ob, data);
}

int Toolbar::get_toolbar_func(string const & func)
{
	int action = lyxaction.LookupFunc(func.c_str());
	if (action == -1) {
               if (func == "separator"){
                       action = TOOL_SEPARATOR;
               } else if (func == "layouts"){
                        action = TOOL_LAYOUTS;
                } else action = 0;
	}
	return action;
}


void Toolbar::init()
{
	add(TOOL_LAYOUTS);
	add(LFUN_MENUOPEN);
	//add(LFUN_CLOSEBUFFER);
	add(LFUN_MENUWRITE);
	add(LFUN_MENUPRINT);
	add(TOOL_SEPARATOR);

	add(LFUN_CUT);
	add(LFUN_COPY);
	add(LFUN_PASTE);
	add(TOOL_SEPARATOR);
	
	add(LFUN_EMPH);
	add(LFUN_NOUN);
	add(LFUN_FREE);
	add(TOOL_SEPARATOR);
	
	add(LFUN_FOOTMELT);
	add(LFUN_MARGINMELT);
	add(LFUN_DEPTH);
	add(TOOL_SEPARATOR);

	add(LFUN_TEX);
        add(LFUN_MATH_MODE);
	add(TOOL_SEPARATOR);

	add(LFUN_FIGURE);
	add(LFUN_TABLE);
	//add(LFUN_MELT);
}


void Toolbar::set(bool doingmain)
{
	// we shouldn't set if we have not cleaned
	if (!cleaned) return;
	
	toolbarItem *item;
	FL_OBJECT *obj;
	item = toollist;
	
	if (!doingmain) {
		fl_freeze_form(owner->getForm());
		fl_addto_form(owner->getForm());
	}

#if FL_REVISION <86
	// Ensure borderwidth is 2 to get visual feedback
	int bw = fl_get_border_width();
	fl_set_border_width(-2);
#endif

	// add the time if it don't exist
	if (bubble_timer == 0)
		bubble_timer = fl_add_timer(FL_HIDDEN_TIMER,
					    xpos, ypos, 0, 0, "Timer");
	
	while(item != 0) {
		switch(item->action){
		  case TOOL_SEPARATOR:
			  xpos += sepspace;
			  item = item->next;
			  break;
		  case TOOL_LAYOUTS:
			  xpos += standardspacing;
			  if (!combox)
				  combox = new Combox(FL_COMBOX_DROPLIST);
			  combox->add(xpos, ypos, 135, height, 300);
			  combox->setcallback(LayoutsCB);
			  combox->resize(FL_RESIZE_ALL);
			  combox->gravity(NorthWestGravity, NorthWestGravity);
			  item = item->next;
			  xpos += 135;
			  break;
		  default:
			  xpos += standardspacing;
			  item->icon = obj = 
				  fl_add_pixmapbutton(FL_NORMAL_BUTTON,
						      xpos, ypos,
						      buttonwidth,
						      height, "");
			  fl_set_object_boxtype(obj, FL_UP_BOX);
			  fl_set_object_color(obj, FL_MCOL, FL_BLUE);
			  fl_set_object_resize(obj, FL_RESIZE_ALL);
			  fl_set_object_gravity(obj,
						NorthWestGravity,
						NorthWestGravity);
			  fl_set_object_callback(obj, C_Toolbar_ToolbarCB,
						 (long)item->action);
#if FL_REVISION >85
			  // Remove the blue feedback rectangle
			  fl_set_pixmapbutton_focus_outline(obj, 0);
#endif

			  // set the bubble-help (Matthias)
			  obj->u_vdata = (void *) item->help.c_str();
			  // we need to know what toolbar this item
			  // belongs too. (Lgb)
			  obj->u_ldata = (long) this;
			  
			  fl_set_object_posthandler(obj, C_Toolbar_BubblePost);

			  fl_set_pixmapbutton_data(obj, item->pixmap);
			  item = item->next;
			  // we must remember to update the positions
			  xpos += buttonwidth;
			  // ypos is constant
			  /* Here will come a check to see if the new
			   * pos is within the bounds of the main frame,
			   * and perhaps wrap the toolbar if not.
			   */
			  break;
		}
	}
#if FL_REVISION <86
	// Reset borderwidth to its default value.
	fl_set_border_width(bw);
#endif
	if (!doingmain) {
		fl_end_form();
		fl_unfreeze_form(owner->getForm());
		// Should be safe to do this here.
		owner->updateLayoutChoice();
	}
	
	cleaned = false;
}


char **Toolbar::getPixmap(kb_action action, string const & arg)
{
	char **pixmap = unknown_xpm; //0
	switch(action){
	case LFUN_MENUOPEN:    pixmap = open_xpm; break;
	case LFUN_CLOSEBUFFER: pixmap = close_xpm; break;
	case LFUN_MENUPRINT:   pixmap = print1_xpm; break;
	case LFUN_MENUWRITE:   pixmap = save_xpm; break;
	case LFUN_EMPH:	 pixmap = emph_xpm; break;
	case LFUN_NOUN:        pixmap = noun_xpm; break;
	case LFUN_FREE:        pixmap = free_xpm; break;
	case LFUN_FOOTMELT:    pixmap = foot_xpm; break;
	case LFUN_DEPTH:       pixmap = depth_xpm; break;
	case LFUN_COPY:        pixmap = copy_xpm; break;
	case LFUN_CUT:         pixmap = cut_xpm; break;
	case LFUN_PASTE:       pixmap = paste_xpm; break;
	case LFUN_TEX:         pixmap = tex_xpm; break;
	case LFUN_MATH_MODE:   pixmap = math_xpm; break;
	case LFUN_MARGINMELT:  pixmap = margin_xpm; break;
	case LFUN_FIGURE:      pixmap = fig_xpm; break;
	case LFUN_TABLE:       pixmap = tab_xpm; break;
	case LFUN_MELT:        pixmap = melt_xpm; break;
	case LFUN_QUIT:        pixmap = quit_xpm; break;
	case LFUN_RUNDVIPS:    pixmap = update_ps_xpm; break;
	case LFUN_EXPORT:
	{
		if (arg == "ascii")
			pixmap = make_ascii_xpm;
		else if (arg == "latex")
			pixmap = make_latex_xpm;
	}
	break; 
	case LFUN_LAYOUT:
	{
	        if (arg == "Section")
		        pixmap = layout_sec_xpm;
		else if (arg == "LaTeX")
		        pixmap = layout_latex_xpm;
		else if (arg == "LyX-Code")
		        pixmap = layout_code_xpm;
		else if (arg == "Scrap")
		        pixmap = layout_scrap_xpm;
		else
		        pixmap = layout_std_xpm;
	}
	break;

	case LFUN_BOLD : pixmap = bold_xpm; break; 
	case LFUN_SANS: pixmap = sans_xpm; break; 
	case LFUN_RUNLATEX: pixmap = run_latex_xpm; break; 
        case LFUN_BUILDPROG: pixmap = build_xpm; break; 
	case LFUN_PREVIEWPS: pixmap = view_ps_xpm; break; 
	case LFUN_PREVIEW: pixmap = view_dvi_xpm; break; 
	case LFUN_INSERT_MATH:
	{
		if (!arg.empty())
			pixmap = get_pixmap_from_symbol(arg.c_str(),
							buttonwidth,
							height);
	}
	break;
	default:
		//pixmap = unknown_xpm;
		break;
	}
	return pixmap;
}


void Toolbar::add(int action, bool doclean)
{
	if (doclean && !cleaned) clean();

	// this is what we do if we want to add to an existing
	// toolbar.
	if (!doclean && owner) {
		// first «hide» the toolbar buttons. This is not a real hide
		// actually it deletes and frees the button altogether.
		lyxerr << "Toolbar::add: «hide» the toolbar buttons." << endl;
		toolbarItem *item, *tmp= 0;
		item = toollist;

		lightReset();
		
		fl_freeze_form(owner->getForm());
		while(item){
			tmp = item->next;
			if (item->icon) {
				fl_delete_object(item->icon);
				fl_free_object(item->icon);
			}
			item = tmp;
		}
		if (combox) {
			delete combox;
			combox = 0;
		}
		fl_unfreeze_form(owner->getForm());
		cleaned = true; // this is not completely true, but OK anyway
	}
	
	// there exist some special actions not part of
	// kb_action: SEPARATOR, LAYOUTS
	char **pixmap = 0;
	string help;

	toolbarItem *newItem,*tmp;

	if (lyxaction.isPseudoAction(action)) {
		char const *arg;
		kb_action act = (kb_action)lyxaction.retrieveActionArg(action, &arg);
		pixmap = getPixmap(act, arg);
		help = lyxaction.helpText(act);
		help += " ";
		help += arg;
		lyxerr.debug() << "Pseudo action " << action << endl;
	} else {
		pixmap = getPixmap((kb_action)action);
		help = lyxaction.helpText((kb_action)action);
	}
	
	// adds an item to the list
	if (pixmap != 0
	    || action == TOOL_SEPARATOR
	    || action == TOOL_LAYOUTS)
	{
		newItem = new toolbarItem;
		newItem->action = action;
		newItem->pixmap = pixmap;
		newItem->help = help;
		// the new item is placed at the end of the list
		tmp = toollist;
		if (tmp != 0){
			while(tmp->next != 0)
				tmp = tmp->next;
			// here is tmp->next == 0
			tmp->next = newItem;
		} else
			toollist = newItem;
	}
	//if (action == TOOL_LAYOUTS) {
	//	combox = new Combox(FL_COMBOX_DROPLIST);
	//}
}


void Toolbar::add(string const & func, bool doclean)
{
	int tf = lyxaction.LookupFunc(func.c_str());

	if (tf == -1){
		lyxerr << "Toolbar::add: no LyX command called`"
		       << func << "'exists!" << endl; 
	} else {
		add(tf, doclean);
	}
}


void Toolbar::clean()
{
	toolbarItem *item, *tmp= 0;
	item = toollist;

	reset();

	//now delete all the objects..
	if (owner)
		fl_freeze_form(owner->getForm());
	while (item) {
		tmp = item->next;
		delete item;
		item = tmp;
	}
	lyxerr[Debug::TOOLBAR] << "Combox: " << combox << endl;
	if (combox) {
		delete combox;
		combox = 0;
	}
	if (owner)
		fl_unfreeze_form(owner->getForm());
	lyxerr[Debug::TOOLBAR] << "toolbar cleaned" << endl;
	cleaned = true;
}


void Toolbar::push(int nth)
{
	lyxerr[Debug::TOOLBAR] << "Toolbar::push: trying to trigger no `"
			       << nth << '\'' << endl;
	
	if (nth == 0) return;

	int count= 0;
	toolbarItem *tmp = toollist;
	while (tmp) {
		count++;
		if (count == nth) {
			fl_trigger_object(tmp->icon);
			return;
		}
		tmp = tmp->next;
	}
	// item nth not found...
	LyXBell();
}


void Toolbar::read(LyXLex &lex)
{
	//consistency check
	if (lex.GetString() != "\\begin_toolbar")
		lyxerr << "Toolbar::read: ERROR wrong token:`"
		       << lex.GetString() << '\'' << endl;

	clean();
	string func;
	bool quit = false;
	
	lex.pushTable(toolTags, TO_LAST - 1);

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable();
	
	while (lex.IsOK() && !quit) {
		
		lyxerr[Debug::TOOLBAR] << "Toolbar::read: current lex text: `"
				       << lex.GetString() << '\'' << endl;

		switch(lex.lex()) {
		  case TO_ADD:
			  if (lex.EatLine()) {
				  func = lex.GetString();
				  lyxerr[Debug::TOOLBAR]
					  << "Toolbar::read TO_ADD func: `"
					  << func << "'" << endl;
				  add(func);
			  }
			  break;
		   
		  case TO_SEPARATOR:
			  add(TOOL_SEPARATOR);
			  break;
		   
		  case TO_LAYOUTS:
			  add(TOOL_LAYOUTS);
			  break;
		   
		  case TO_NEWLINE:
			  add(TOOL_NEWLINE);
			  break;
			
		  case TO_ENDTOOLBAR:
			  // should not set automatically
			  //set();
			  quit = true;
			  break;
		  default:
			  lex.printError("Toolbar::read: "
					  "Unknown toolbar tag: `$$Token'");
			  break;
		}
	}
	lex.popTable();
}
