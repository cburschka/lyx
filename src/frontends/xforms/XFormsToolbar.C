/**
 * \file XFormsToolbar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

//  Added pseudo-action handling, asierra 180296

#include <config.h>

#include "XFormsToolbar.h"

#include "Tooltips.h"
#include "xforms_helpers.h"
#include "XFormsView.h"

#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "lyxfunc.h"

#include "lyx_forms.h"
#include "combox.h"

using std::distance;
using std::endl;
using std::string;


// some constants
const int standardspacing = 2; // the usual space between items
const int sepspace = 6; // extra space
const int buttonwidth = 30; // the standard button width
const int height = 30; // the height of all items in the toolbar

XFormsToolbar::toolbarItem::toolbarItem()
	: icon(0)
{}


XFormsToolbar::toolbarItem::~toolbarItem()
{
	// Lars said here that ~XFormsView() dealt with the icons.
	// This is not true. But enabling this causes crashes,
	// because somehow we kill the same icon twice :(
	// FIXME
	//kill_icon();
}


/// Display toolbar, not implemented. But moved out of line so that
/// linking will work properly.
void XFormsToolbar::displayToolbar(ToolbarBackend::Toolbar const & /*tb*/,
				    bool /*show*/)
{}


void XFormsToolbar::toolbarItem::kill_icon()
{
	if (icon) {
		fl_delete_object(icon);
		fl_free_object(icon);
		icon = 0;
	}
}


XFormsToolbar::toolbarItem &
XFormsToolbar::toolbarItem::operator=(toolbarItem const & ti)
{
	if (this == &ti)
		return *this;

	// If we already have an icon, release it.
	// But we don't copy the icon from ti
	kill_icon();

	func = ti.func;

	return *this;
}



XFormsToolbar::XFormsToolbar(LyXView * o, int x, int y)
	: owner_(static_cast<XFormsView *>(o)), combox_(0), xpos(x), ypos(y)
{
	tooltip_ = new Tooltips;
}


XFormsToolbar::~XFormsToolbar()
{
	fl_freeze_form(owner_->getForm());

	// G++ vector does not have clear defined
	//toollist.clear();
	toollist_.erase(toollist_.begin(), toollist_.end());

	fl_unfreeze_form(owner_->getForm());
	delete tooltip_;
}


void XFormsToolbar::update()
{
	ToolbarList::const_iterator p = toollist_.begin();
	ToolbarList::const_iterator end = toollist_.end();
	for (; p != end; ++p) {
		if (p->func.action == int(ToolbarBackend::LAYOUTS) && combox_) {
			LyXFunc const & lf = owner_->getLyXFunc();
			bool const disable =
				lf.getStatus(FuncRequest(LFUN_LAYOUT)).disabled();
			setEnabled(combox_, !disable);
			continue;
		}

		if (!p->icon)
			continue;

		FuncStatus const status = owner_->getLyXFunc().getStatus(p->func);
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


namespace {

void C_layoutSelectedCB(FL_OBJECT * ob, long)
{
	if (!ob || !ob->u_vdata)
		return;
	XFormsToolbar * ptr = static_cast<XFormsToolbar *>(ob->u_vdata);
	ptr->layoutSelected();
}

} // namespace anon


void XFormsToolbar::layoutSelected()
{
	if (!combox_)
		return;

	string const & layoutguiname = getString(combox_);
	LyXTextClass const & tc =
		owner_->buffer()->params().getLyXTextClass();

	LyXTextClass::const_iterator end = tc.end();
	for (LyXTextClass::const_iterator cit = tc.begin();
	     cit != end; ++cit) {
		if (_((*cit)->name()) == layoutguiname) {
			owner_->getLyXFunc().dispatch(FuncRequest(LFUN_LAYOUT, (*cit)->name()), true);
			return;
		}
	}
	lyxerr << "ERROR (XFormsToolbar::layoutSelected): layout not found!"
	       << endl;
}


void XFormsToolbar::setLayout(string const & layout)
{
	if (!combox_)
		return;

	LyXTextClass const & tc = owner_->buffer()->params().getLyXTextClass();
	string const layoutname = _(tc[layout]->name());

	int const nnames = fl_get_combox_maxitems(combox_);
	for (int i = 1; i <= nnames; ++i) {
		string const name = fl_get_combox_line(combox_, i);
		if (name == layoutname) {
			fl_set_combox(combox_, i);
			break;
		}
	}
}


void XFormsToolbar::updateLayoutList()
{
	if (!combox_)
		return;

	fl_clear_combox(combox_);
	LyXTextClass const & tc = owner_->buffer()->params().getLyXTextClass();
	LyXTextClass::const_iterator end = tc.end();
	for (LyXTextClass::const_iterator cit = tc.begin();
	     cit != end; ++cit) {
		// ignore obsolete entries
		if ((*cit)->obsoleted_by().empty()) {
			string const & name = _((*cit)->name());
			fl_addto_combox(combox_, name.c_str());
		}
	}

	// we need to do this.
	fl_redraw_object(combox_);
}


void XFormsToolbar::clearLayoutList()
{
	if (!combox_)
		return;

	Toolbar::clearLayoutList();
	fl_clear_combox(combox_);
	fl_redraw_object(combox_);
}


void XFormsToolbar::openLayoutList()
{
	if (!combox_)
		return;

	fl_show_combox_browser(combox_);
}


namespace {

void ToolbarCB(FL_OBJECT * ob, long ac)
{
	if (!ob || !ob->u_vdata)
		return;

	XFormsToolbar * ptr = static_cast<XFormsToolbar *>(ob->u_vdata);
	XFormsView * owner = ptr->owner_;
	owner->getLyXFunc().dispatch(ptr->funcs[ac], true);
}


extern "C" {

void C_Toolbar_ToolbarCB(FL_OBJECT * ob, long data)
{
	ToolbarCB(ob, data);
}

}

} // namespace anon


void XFormsToolbar::add(ToolbarBackend::Toolbar const & tb)
{
	// we can only handle one toolbar
	if (!toollist_.empty())
		return;

	funcs.clear();

	ToolbarBackend::item_iterator it = tb.items.begin();
	ToolbarBackend::item_iterator end = tb.items.end();
	for (; it != end; ++it)
		add(it->first, it->second);
}


void XFormsToolbar::add(FuncRequest const & func, string const & tooltip)
{
	toolbarItem item;
	item.func = func;

	switch (func.action) {
	case ToolbarBackend::SEPARATOR:
		xpos += sepspace;
		break;
	case ToolbarBackend::MINIBUFFER:
		// Not implemented
		break;
	case ToolbarBackend::LAYOUTS:
		xpos += standardspacing;
		if (combox_)
			break;

		combox_ = fl_add_combox(FL_DROPLIST_COMBOX,
					xpos, ypos, 135, height, "");
		fl_set_combox_browser_height(combox_, 400);
		fl_set_object_boxtype(combox_, FL_DOWN_BOX);
		fl_set_object_color(combox_, FL_MCOL, FL_MCOL);
		fl_set_object_gravity(combox_, FL_NorthWest, FL_NorthWest);
		fl_set_object_resize(combox_, FL_RESIZE_ALL);

		combox_->u_vdata = this;
		fl_set_object_callback(combox_, C_layoutSelectedCB, 0);
		xpos += 135;
		break;
	default: {
		FL_OBJECT * obj;

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

		Funcs::iterator fit = funcs.insert(funcs.end(), func);
		int const index = distance(funcs.begin(), fit);
		fl_set_object_callback(obj, C_Toolbar_ToolbarCB, index);
		// Remove the blue feedback rectangle
		fl_set_pixmapbutton_focus_outline(obj, 0);

		tooltip_->init(obj, tooltip);

		// The view that this object belongs to.
		obj->u_vdata = this;

		string const xpm = toolbarbackend.getIcon(func);
		fl_set_pixmapbutton_file(obj, xpm.c_str());

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

	toollist_.push_back(item);
}
