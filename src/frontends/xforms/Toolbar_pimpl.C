/**
 * \file Toolbar_pimpl.C
 * Copyright 1995 Matthias Ettrich
 * Copyright 1995-2001 The LyX Team.
 * Copyright 1996-1998 Lars Gullik Bjønnes
 * See the file COPYING.
 *
 * \author Lars Gullik Bjønnes, larsbj@lyx.org
 */

//  Added pseudo-action handling, asierra 180296

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Toolbar_pimpl.h"
#include "debug.h"
#include "XFormsView.h"
#include "lyxfunc.h"
#include "FuncStatus.h"
#include "BufferView.h"
#include "buffer.h"
#include "LyXAction.h"
#include "MathsSymbols.h"
#include "gettext.h"
#include "Tooltips.h"
#include FORMS_H_LOCATION
#include "combox.h"
#include "ToolbarDefaults.h"

#include "support/LAssert.h"
#include "support/filetools.h"
#include "support/lstrings.h"

using std::endl;

extern LyXAction lyxaction;

// some constants
const int standardspacing = 2; // the usual space between items
const int sepspace = 6; // extra space
const int buttonwidth = 30; // the standard button width
const int height = 30; // the height of all items in the toolbar

Toolbar::Pimpl::toolbarItem::toolbarItem()
{
	action = LFUN_NOACTION;
	icon = 0;
}


Toolbar::Pimpl::toolbarItem::~toolbarItem()
{
	// It seems that now this is taken care of
	// in the XFormsView destructor. (Lgb)
	// clean();
}


void Toolbar::Pimpl::toolbarItem::clean()
{
	if (icon) {
		fl_delete_object(icon);
		fl_free_object(icon);
		icon = 0;
	}
}


Toolbar::Pimpl::toolbarItem &
Toolbar::Pimpl::toolbarItem::operator=(toolbarItem const & ti)
{
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



Toolbar::Pimpl::Pimpl(LyXView * o, Dialogs & d, int x, int y)
	: owner(static_cast<XFormsView *>(o)), sxpos(x), sypos(y)
{
	combox = 0;
	tooltip_ = new Tooltips(d);
}


Toolbar::Pimpl::~Pimpl()
{
	clean();
	delete tooltip_;
}


void Toolbar::Pimpl::update()
{
	ToolbarList::const_iterator p = toollist.begin();
	ToolbarList::const_iterator end = toollist.end();
	for (; p != end; ++p) {
		if (p->icon) {
			FuncStatus status = owner->getLyXFunc()->getStatus(p->action);
			if (status.onoff(true)) {
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
			if (status.disabled()) {
				// Is there a way here to specify a
				// mask in order to show that the
				// button is disabled? (JMarc)
				fl_deactivate_object(p->icon);
			}
			else
				fl_activate_object(p->icon);
		} else if (p->action == ToolbarDefaults::LAYOUTS && combox) {
			if (owner->getLyXFunc()->getStatus(LFUN_LAYOUT).disabled())
				combox->deactivate();
			else
				combox->activate();
		}
	}
}


// this one is not "C" because combox callbacks are really C++ %-|
void Toolbar::Pimpl::layoutSelectedCB(int, void * arg, Combox *)
{
	Toolbar::Pimpl * tb = reinterpret_cast<Toolbar::Pimpl *>(arg);

	tb->layoutSelected();
}


void Toolbar::Pimpl::layoutSelected()
{
	string const & layoutguiname = combox->getline();
	LyXTextClass const & tc =
		owner->buffer()->params.getLyXTextClass();

	LyXTextClass::const_iterator end = tc.end();
	for (LyXTextClass::const_iterator cit = tc.begin();
	     cit != end; ++cit) {
		if (_((*cit)->name()) == layoutguiname) {
			owner->getLyXFunc()->dispatch(LFUN_LAYOUT, (*cit)->name());
			return;
		}
	}
	lyxerr << "ERROR (Toolbar::Pimpl::layoutSelected): layout not found!"
	       << endl;
}


void Toolbar::Pimpl::setLayout(string const & layout)
{
	if (combox) {
		LyXTextClass const & tc =
			owner->buffer()->params.getLyXTextClass();
		combox->select(_(tc[layout]->name()));
	}
}


void Toolbar::Pimpl::updateLayoutList(bool force)
{
	// Update the layout display
	if (!combox) return;

	// If textclass is different, we need to update the list
	if (combox->empty() || force) {
		combox->clear();
		LyXTextClass const & tc =
			owner->buffer()->params.getLyXTextClass();
		LyXTextClass::const_iterator end = tc.end();
		for (LyXTextClass::const_iterator cit = tc.begin();
		     cit != end; ++cit) {
			// ignore obsolete entries
			if ((*cit)->obsoleted_by().empty())
				combox->addline(_((*cit)->name()));
		}
	}
	// we need to do this.
	combox->redraw();
}


void Toolbar::Pimpl::clearLayoutList()
{
	if (combox) {
		combox->clear();
		combox->redraw();
	}
}


void Toolbar::Pimpl::openLayoutList()
{
	if (combox)
		combox->show();
}


namespace {

void ToolbarCB(FL_OBJECT * ob, long ac)
{
	XFormsView * owner = static_cast<XFormsView *>(ob->u_vdata);

	owner->getLyXFunc()->dispatch(int(ac), true);
}


extern "C" {

	static
	void C_Toolbar_ToolbarCB(FL_OBJECT * ob, long data)
	{
		ToolbarCB(ob, data);
	}

}


void setPixmap(FL_OBJECT * obj, int action, int buttonwidth, int height)
{
	string arg;
	string xpm_name;

	const kb_action act = lyxaction.retrieveActionArg(action, arg);
	string const name = lyxaction.getActionName(act);
	if (!arg.empty())
		xpm_name = subst(name + ' ' + arg, ' ','_');
	else
		xpm_name = name;

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

} // namespace anon


void Toolbar::Pimpl::set(bool doingmain)
{
	// we shouldn't set if we have not cleaned
	if (!cleaned) return;

	FL_OBJECT * obj;

	if (!doingmain) {
		fl_freeze_form(owner->getForm());
		fl_addto_form(owner->getForm());
	}

	ToolbarList::iterator item = toollist.begin();
	ToolbarList::iterator end = toollist.end();
	for (; item != end; ++item) {
		switch (item->action) {
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
			combox->setcallback(layoutSelectedCB, this);
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

			// initialise the tooltip
			string const tip = _(lyxaction.helpText(obj->argument));
			tooltip_->init(obj, tip);

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
