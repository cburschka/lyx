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
#include "buffer.h"
#include "funcrequest.h"
#include "MathsSymbols.h"
#include "gettext.h"
#include "Tooltips.h"
#include FORMS_H_LOCATION
#include "combox.h"
#include "ToolbarDefaults.h"
#include "LyXAction.h"

#include "support/LAssert.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include <boost/tuple/tuple.hpp>
 
using std::endl;

// some constants
const int standardspacing = 2; // the usual space between items
const int sepspace = 6; // extra space
const int buttonwidth = 30; // the standard button width
const int height = 30; // the height of all items in the toolbar

Toolbar::Pimpl::toolbarItem::toolbarItem()
	: action(LFUN_NOACTION), icon(0)
{
}


Toolbar::Pimpl::toolbarItem::~toolbarItem()
{
	// Lars said here that ~XFormsView() dealt with the icons.
	// This is not true. But enabling this causes crashes,
	// because somehow we kill the same icon twice :(
	// FIXME
	//kill_icon();
}


void Toolbar::Pimpl::toolbarItem::kill_icon()
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
	if (this == &ti)
		return *this;

	// If we already have an icon, release it.
	// But we don't copy the icon from ti
	kill_icon();
 
	action = ti.action;

	return *this;
}



Toolbar::Pimpl::Pimpl(LyXView * o, int x, int y)
	: owner_(static_cast<XFormsView *>(o)), xpos(x), ypos(y)
{
	combox_ = 0;
	tooltip_ = new Tooltips();
}


Toolbar::Pimpl::~Pimpl()
{
	fl_freeze_form(owner_->getForm());

	// G++ vector does not have clear defined
	//toollist.clear();
	toollist_.erase(toollist_.begin(), toollist_.end());

	delete combox_;

	fl_unfreeze_form(owner_->getForm());
	delete tooltip_;
}


void Toolbar::Pimpl::update()
{
	ToolbarList::const_iterator p = toollist_.begin();
	ToolbarList::const_iterator end = toollist_.end();
	for (; p != end; ++p) {
		if (p->action == ToolbarDefaults::LAYOUTS && combox_) {
			if (owner_->getLyXFunc().getStatus(LFUN_LAYOUT).disabled())
				combox_->deactivate();
			else
				combox_->activate();
			continue;
		}

		if (!p->icon)
			continue;
 
		FuncStatus const status = owner_->getLyXFunc().getStatus(p->action);
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
	}
}


// this one is not "C" because combox callbacks are really C++ %-|
void Toolbar::Pimpl::layoutSelectedCB(int, void * arg, Combox *)
{
	reinterpret_cast<Toolbar::Pimpl *>(arg)->layoutSelected();
}


void Toolbar::Pimpl::layoutSelected()
{
	string const & layoutguiname = combox_->getline();
	LyXTextClass const & tc =
		owner_->buffer()->params.getLyXTextClass();

	LyXTextClass::const_iterator end = tc.end();
	for (LyXTextClass::const_iterator cit = tc.begin();
	     cit != end; ++cit) {
		if (_((*cit)->name()) == layoutguiname) {
			owner_->getLyXFunc().dispatch(FuncRequest(LFUN_LAYOUT, (*cit)->name()));
			return;
		}
	}
	lyxerr << "ERROR (Toolbar::Pimpl::layoutSelected): layout not found!"
	       << endl;
}


void Toolbar::Pimpl::setLayout(string const & layout)
{
	if (combox_) {
		LyXTextClass const & tc =
			owner_->buffer()->params.getLyXTextClass();
		combox_->select(_(tc[layout]->name()));
	}
}


void Toolbar::Pimpl::updateLayoutList(bool force)
{
	// Update the layout display
	if (!combox_) return;

	// If textclass is different, we need to update the list
	if (combox_->empty() || force) {
		combox_->clear();
		LyXTextClass const & tc =
			owner_->buffer()->params.getLyXTextClass();
		LyXTextClass::const_iterator end = tc.end();
		for (LyXTextClass::const_iterator cit = tc.begin();
		     cit != end; ++cit) {
			// ignore obsolete entries
			if ((*cit)->obsoleted_by().empty())
				combox_->addline(_((*cit)->name()));
		}
	}
	// we need to do this.
	combox_->redraw();
}


void Toolbar::Pimpl::clearLayoutList()
{
	if (combox_) {
		combox_->clear();
		combox_->redraw();
	}
}


void Toolbar::Pimpl::openLayoutList()
{
	if (combox_)
		combox_->show();
}


namespace {

void ToolbarCB(FL_OBJECT * ob, long ac)
{
	XFormsView * owner = static_cast<XFormsView *>(ob->u_vdata);

	owner->getLyXFunc().dispatch(int(ac), true);
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
	string xpm_name;
	FuncRequest ev = lyxaction.retrieveActionArg(action);
 
	string const name = lyxaction.getActionName(ev.action);
	if (!ev.argument.empty())
		xpm_name = subst(name + ' ' + ev.argument, ' ','_');
	else
		xpm_name = name;

	string fullname = LibFileSearch("images", xpm_name, "xpm");

	if (!fullname.empty()) {
		lyxerr[Debug::GUI] << "Full icon name is `"
				   << fullname << "'" << endl;
		fl_set_pixmapbutton_file(obj, fullname.c_str());
		return;
	}

	if (ev.action == LFUN_INSERT_MATH && !ev.argument.empty()) {
		char const ** pixmap = get_pixmap_from_symbol(ev.argument.c_str(),
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


void Toolbar::Pimpl::add(int action)
{
	FL_OBJECT * obj;

	toolbarItem item;
	item.action = action;
 
	switch (action) {
	case ToolbarDefaults::SEPARATOR:
		xpos += sepspace;
		break;
	case ToolbarDefaults::NEWLINE:
		// Not supported yet.
		break;
	case ToolbarDefaults::LAYOUTS:
		xpos += standardspacing;
		if (!combox_)
			combox_ = new Combox(FL_COMBOX_DROPLIST);
		combox_->add(xpos, ypos, 135, height, 400);
		combox_->setcallback(layoutSelectedCB, this);
		combox_->resize(FL_RESIZE_ALL);
		combox_->gravity(NorthWestGravity, NorthWestGravity);
		xpos += 135;
		break;
	default:
		xpos += standardspacing;
		item.icon = obj =
			fl_add_pixmapbutton(FL_NORMAL_BUTTON,
					    xpos, ypos,
					    buttonwidth,
					    height, "");
		fl_set_object_resize(obj, FL_RESIZE_ALL);
		fl_set_object_gravity(obj,
				      NorthWestGravity,
				      NorthWestGravity);
		fl_set_object_callback(obj, C_Toolbar_ToolbarCB,
				       static_cast<long>(action));
		// Remove the blue feedback rectangle
		fl_set_pixmapbutton_focus_outline(obj, 0);

		// initialise the tooltip
		string const tip = _(lyxaction.helpText(obj->argument));
		tooltip_->init(obj, tip);

		// The view that this object belongs to.
		obj->u_vdata = owner_;

		setPixmap(obj, action, buttonwidth, height);
		// we must remember to update the positions
		xpos += buttonwidth;
		// ypos is constant
		/* Here will come a check to see if the new
		 * pos is within the bounds of the main frame,
		 * and perhaps wrap the toolbar if not.
		 */
		break;
	}
 
	toollist_.push_back(item);
}
