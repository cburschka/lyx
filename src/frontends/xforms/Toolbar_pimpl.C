/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file is Copyright 1996-1998
 *           Lars Gullik Bjønnes
 *
 * ====================================================== */

//  Added pseudo-action handling, asierra 180296

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Toolbar_pimpl.h"
#include "lyxfunc.h"
#include "debug.h"
#include "LyXView.h"
#include "BufferView.h"
#include "buffer.h"
#include "LyXAction.h"
#include "support/filetools.h"
#include "gettext.h"

using std::endl;

// this one is not "C" because combox callbacks are really C++ %-|
extern void LayoutsCB(int, void *, Combox *);
extern char const ** get_pixmap_from_symbol(char const * arg, int, int);
extern LyXAction lyxaction;

// some constants
const int standardspacing = 2; // the usual space between items
const int sepspace = 6; // extra space
const int buttonwidth = 30; // the standard button width
const int height = 30; // the height of all items in the toolbar

Toolbar::Pimpl::toolbarItem::toolbarItem() {
	action = LFUN_NOACTION;
	icon = 0;
}


Toolbar::Pimpl::toolbarItem::~toolbarItem() {
	clean();
}


void Toolbar::Pimpl::toolbarItem::clean() {
	if (icon) {
		fl_delete_object(icon);
		fl_free_object(icon);
		icon = 0;
	}
}


Toolbar::Pimpl::toolbarItem & 
Toolbar::Pimpl::toolbarItem::operator=(toolbarItem const & ti) {
	// Are we assigning the object onto itself?
	if (this == &ti)
		return *this;

	// If we already have an icon, release it.
	clean();
    
	// do we have to check icon too?
	action = ti.action;
	icon = 0; // locally we need to get the icon anew
	
	return *this;
}



Toolbar::Pimpl::Pimpl(LyXView * o, int x, int y)
	: owner(o), sxpos(x), sypos(y)
{
	combox = 0;
#if FL_REVISION < 89
	bubble_timer = 0;
#endif
}


#if FL_REVISION < 89
// timer-cb for bubble-help (Matthias)
static 
void BubbleTimerCB(FL_OBJECT *, long data)
{
	FL_OBJECT * ob = reinterpret_cast<FL_OBJECT*>(data);
	// The trick we use to get the help text is to read the
	// argument of the callback that has been registered for
	// ToolBarCB.  (JMarc)
	string help = _(lyxaction.helpText(ob->argument));
	fl_show_oneliner(help.c_str(), ob->form->x + ob->x,
			 ob->form->y + ob->y + ob->h);
}


extern "C" void C_Toolbar_BubbleTimerCB(FL_OBJECT * ob, long data)
{
	BubbleTimerCB(ob, data);
}


// post_handler for bubble-help (Matthias)
static
int BubblePost(FL_OBJECT *ob, int event,
			FL_Coord /*mx*/, FL_Coord /*my*/,
			int /*key*/, void */*xev*/)
{
	FL_OBJECT * bubble_timer = reinterpret_cast<FL_OBJECT *>(ob->u_cdata);
	
	// We do not test for empty help here, since this can never happen
	if (event == FL_ENTER){
		fl_set_object_callback(bubble_timer,
				       C_Toolbar_BubbleTimerCB,
				       reinterpret_cast<long>(ob));
		fl_set_timer(bubble_timer, 1);
	}
	else if (event != FL_MOTION){
		fl_set_timer(bubble_timer, 0);
		fl_hide_oneliner();
	}
	return 0;
}


extern "C" int C_Toolbar_BubblePost(FL_OBJECT * ob, int event,
				    FL_Coord /*mx*/, FL_Coord /*my*/, 
				    int key, void * xev)
{
	return BubblePost(ob, event, 0, 0, key, xev);
}
#endif


void Toolbar::Pimpl::activate()
{
	ToolbarList::const_iterator p = toollist.begin();
	ToolbarList::const_iterator end = toollist.end();
	for (; p != end; ++p) {
		if (p->icon) {
			fl_activate_object(p->icon);
		}
	}
}


void Toolbar::Pimpl::deactivate()
{
	ToolbarList::const_iterator p = toollist.begin();
	ToolbarList::const_iterator end = toollist.end();
	for (; p != end; ++p) {
		if (p->icon) {
			fl_deactivate_object(p->icon);
		}
	}
}

void Toolbar::Pimpl::update()
{
	ToolbarList::const_iterator p = toollist.begin();
	ToolbarList::const_iterator end = toollist.end();
	for (; p != end; ++p) {
		if (p->icon) {
			int status = owner->getLyXFunc()->getStatus(p->action);
			if (status & LyXFunc::ToggleOn) {
				// I'd like to use a different color
				// here, but then the problem is to
				// know how to use transparency with
				// Xpm library. It seems pretty
				// complicated to me (JMarc)
				fl_set_object_color(p->icon, FL_LEFT_BCOL, FL_BLUE);
				fl_set_object_boxtype(p->icon, FL_DOWN_BOX);
			} else {
				fl_set_object_color(p->icon, FL_MCOL, FL_BLUE);
				fl_set_object_boxtype(p->icon, FL_UP_BOX);
			}

			if (status & LyXFunc::Disabled) {
				// Is there a way here to specify a
				// mask in order to show that the
				// button is disabled? (JMarc)
				fl_deactivate_object(p->icon);
			}
			else
				fl_activate_object(p->icon);
		}
	}
}


void Toolbar::Pimpl::setLayout(int layout) {
	if (combox)
		combox->select(layout+1);
}


void Toolbar::Pimpl::updateLayoutList(bool force)
{
	// Update the layout display
	if (!combox) return;

	// If textclass is different, we need to update the list
	if (combox->empty() || force) {
		combox->clear();
		LyXTextClass const & tc =
			textclasslist.TextClass(owner->buffer()->
						params.textclass);
		LyXTextClass::const_iterator end = tc.end();
		for (LyXTextClass::const_iterator cit = tc.begin();
		     cit != end; ++cit) {
			if ((*cit).obsoleted_by().empty())
				combox->addline(_((*cit).name()));
			else
				combox->addline("@N" + _((*cit).name()));
		}
	}
	// we need to do this.
	combox->Redraw();
}


void Toolbar::Pimpl::clearLayoutList()
{
	if (combox) {
		combox->clear();
		combox->Redraw();
	}
}


void Toolbar::Pimpl::openLayoutList()
{
	if (combox)
		combox->Show();
}


static
void ToolbarCB(FL_OBJECT * ob, long ac)
{
	LyXView * owner = static_cast<LyXView *>(ob->u_vdata);
	
	string res = owner->getLyXFunc()->Dispatch(int(ac));
	if (!res.empty())
		lyxerr[Debug::GUI] << "ToolbarCB: Function returned: " 
				   << res << endl;
}


extern "C" void C_Toolbar_ToolbarCB(FL_OBJECT * ob, long data)
{
	ToolbarCB(ob, data);
}


#if 0
// What are we supposed to do with that??
int Toolbar::get_toolbar_func(string const & func)
{
	int action = lyxaction.LookupFunc(func);
	if (action == -1) {
		if (func == "separator"){
			action = TOOL_SEPARATOR;
		} else if (func == "layouts"){
                        action = TOOL_LAYOUTS;
                } else action = 0;
	}
	return action;
}
#endif


static
void setPixmap(FL_OBJECT * obj, int action, int buttonwidth, int height) {
	string name, arg, xpm_name;
	kb_action act;

	if (lyxaction.isPseudoAction(action)) {
		act = lyxaction.retrieveActionArg(action, arg);
		name = lyxaction.getActionName(act);
		xpm_name = subst(name + ' ' + arg, ' ','_');
	} else {
		act = (kb_action)action;
		name = lyxaction.getActionName(action);
		xpm_name = name;
	}

	string fullname = LibFileSearch("images", xpm_name, "xpm");

	if (!fullname.empty()) {
		lyxerr[Debug::GUI] << "Full icon name is `" 
				       << fullname << "'" << endl;
		fl_set_pixmapbutton_file(obj, fullname.c_str());
		return;
	}

	if (act == LFUN_INSERT_MATH && !arg.empty()) {
		char const ** pixmap = get_pixmap_from_symbol(arg.c_str(),
							buttonwidth,
							height);
		if (pixmap) {
			lyxerr[Debug::GUI] << "Using mathed-provided icon"
					   << endl;
			fl_set_pixmapbutton_data(obj,
						 const_cast<char **>(pixmap));
			return;
		}
	}
	
	lyxerr << "Unable to find icon `" << xpm_name << "'" << endl;
	fullname = LibFileSearch("images", "unknown", "xpm");
	if (!fullname.empty()) {
		lyxerr[Debug::GUI] << "Using default `unknown' icon" 
				       << endl;
		fl_set_pixmapbutton_file(obj, fullname.c_str());
	}
}

void Toolbar::Pimpl::set(bool doingmain)
{
	// we shouldn't set if we have not cleaned
	if (!cleaned) return;
	
	FL_OBJECT * obj;
	
	if (!doingmain) {
		fl_freeze_form(owner->getForm());
		fl_addto_form(owner->getForm());
	}

#if FL_REVISION < 89
	// add the time if it don't exist
	if (bubble_timer == 0)
		bubble_timer = fl_add_timer(FL_HIDDEN_TIMER,
					    xpos, ypos, 0, 0, "Timer");
#endif
	
	ToolbarList::iterator item = toollist.begin();
	ToolbarList::iterator end = toollist.end();
	for (; item != end; ++item) {
		switch (item->action){
		case ToolbarDefaults::SEPARATOR:
			xpos += sepspace;
			break;
		case ToolbarDefaults::NEWLINE:
			// Not supported yet.
			break;
		case ToolbarDefaults::LAYOUTS:
			xpos += standardspacing;
			if (!combox)
				combox = new Combox(FL_COMBOX_DROPLIST);
			combox->add(xpos, ypos, 135, height, 400);
			combox->setcallback(LayoutsCB);
			combox->resize(FL_RESIZE_ALL);
			combox->gravity(NorthWestGravity, NorthWestGravity);
			xpos += 135;
			break;
		default:
			xpos += standardspacing;
			item->icon = obj = 
				fl_add_pixmapbutton(FL_NORMAL_BUTTON,
						    xpos, ypos,
						    buttonwidth,
						    height, "");
			fl_set_object_resize(obj, FL_RESIZE_ALL);
			fl_set_object_gravity(obj,
					      NorthWestGravity,
					      NorthWestGravity);
			fl_set_object_callback(obj, C_Toolbar_ToolbarCB,
					       static_cast<long>(item->action));
			// Remove the blue feedback rectangle
			fl_set_pixmapbutton_focus_outline(obj, 0);

			// Set the tooltip
#if FL_REVISION >= 89
			string help = _(lyxaction.helpText(item->action));
			fl_set_object_helper(obj, help.c_str());	
#else
			fl_set_object_posthandler(obj, C_Toolbar_BubblePost);
			obj->u_cdata = reinterpret_cast<char *>(bubble_timer);
#endif

			// The view that this object belongs to.
			obj->u_vdata = owner;

			setPixmap(obj, item->action, buttonwidth, height);
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

	if (!doingmain) {
		fl_end_form();
		fl_unfreeze_form(owner->getForm());
		// Should be safe to do this here.
		owner->updateLayoutChoice();
	}

	// set the state of the icons
	//update();

	cleaned = false;
}


void Toolbar::Pimpl::add(int action, bool doclean)
{
	if (doclean && !cleaned) clean();

	// this is what we do if we want to add to an existing
	// toolbar.
	if (!doclean && owner) {
		// first "hide" the toolbar buttons. This is not a real hide
		// actually it deletes and frees the button altogether.
		lyxerr << "Toolbar::add: \"hide\" the toolbar buttons." 
		       << endl;

		lightReset();
		
		fl_freeze_form(owner->getForm());

		ToolbarList::iterator p = toollist.begin();
		ToolbarList::iterator end = toollist.end();
		for (; p != end; ++p) {
			p->clean();
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

	toolbarItem newItem;
	newItem.action = action;
	toollist.push_back(newItem);
}


void Toolbar::Pimpl::clean()
{
	//reset(); // I do not understand what this reset() is, anyway

	//now delete all the objects..
	if (owner)
		fl_freeze_form(owner->getForm());

	// G++ vector does not have clear defined
	//toollist.clear();
	toollist.erase(toollist.begin(), toollist.end());

	lyxerr[Debug::GUI] << "Combox: " << combox << endl;
	if (combox) {
		delete combox;
		combox = 0;
	}

	if (owner)
		fl_unfreeze_form(owner->getForm());
	lyxerr[Debug::GUI] << "toolbar cleaned" << endl;
	cleaned = true;
}


void Toolbar::Pimpl::push(int nth)
{
	lyxerr[Debug::GUI] << "Toolbar::push: trying to trigger no `"
			       << nth << '\'' << endl;
	
	if (nth <= 0 || nth >= int(toollist.size())) {
		// item nth not found...
		return;
	}

	fl_trigger_object(toollist[nth - 1].icon);
}


void Toolbar::Pimpl::reset() 
{
	//toollist = 0; // what is this supposed to do?
	cleaned = false;
	lightReset();
}

void Toolbar::Pimpl::lightReset() {
	xpos = sxpos - standardspacing;
	ypos = sypos;
}
