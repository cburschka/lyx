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

#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "lyxfunc.h"

#include "lyx_forms.h"
#include "combox.h"

#include <boost/bind.hpp>

using lyx::frontend::Box;
using lyx::frontend::BoxList;

using std::distance;
using std::endl;
using std::string;


// some constants
const int standardspacing = 2; // the usual space between items
const int sepspace = 6; // extra space
const int buttonwidth = 30; // the standard button width
const int height = 30; // the height of all items in the toolbar

namespace {

XFormsView::Position getPosition(ToolbarBackend::Flags const & flags)
{
	if (flags & ToolbarBackend::TOP)
		return XFormsView::Top;
	if (flags & ToolbarBackend::BOTTOM)
		return XFormsView::Bottom;
	if (flags & ToolbarBackend::LEFT)
		return XFormsView::Left;
	if (flags & ToolbarBackend::RIGHT)
		return XFormsView::Right;
	return XFormsView::Top;
}


LyXTextClass const & getTextClass(LyXView const & lv)
{
	return lv.buffer()->params().getLyXTextClass();
}

} // namespace anon


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



Toolbars::ToolbarPtr make_toolbar(ToolbarBackend::Toolbar const & tbb,
				  LyXView & owner)
{
	return Toolbars::ToolbarPtr(new XFormsToolbar(tbb, owner));
}


XFormsToolbar::XFormsToolbar(ToolbarBackend::Toolbar const & tbb,
			     LyXView & o)
	: toolbar_(0),
	  toolbar_buttons_(0),
	  owner_(static_cast<XFormsView &>(o)),
	  tooltip_(new Tooltips)
{
	position_ = getPosition(tbb.flags);
	BoxList & boxlist = owner_.getBox(position_).children();
	toolbar_ = &boxlist.push_back(Box(0,0));

	// If the toolbar is horizontal, then it contains three
	// vertically-aligned Boxes,the center one of which is to
	// contain the buttons, aligned horizontally.
	// The other two provide some visual padding.

	// If it is vertical, then this is swapped around.

	Box::Orientation const toolbar_orientation =
		(position_ == XFormsView::Left ||
		 position_ == XFormsView::Right)
		? Box::Vertical : Box::Horizontal;

	Box::Orientation const padding_orientation =
		(toolbar_orientation == Box::Vertical)
		? Box::Horizontal : Box::Vertical;

	toolbar_->set(padding_orientation);

	// A bit of a hack, but prevents 'M-x' causing the addition of
	// visible borders.
	int const padding =
		(tbb.name == "minibuffer") ?
		0 : 2 + abs(fl_get_border_width());

	toolbar_->children().push_back(Box(padding, padding));

	Box & toolbar_center = toolbar_->children().push_back(Box(0,0));
	toolbar_center.set(toolbar_orientation);
	toolbar_buttons_ = &toolbar_center.children();

	toolbar_->children().push_back(Box(padding, padding));

	using lyx::frontend::WidgetMap;
	owner_.metricsUpdated.connect(boost::bind(&WidgetMap::updateMetrics,
						  &widgets_));

	// Populate the toolbar.
	ToolbarBackend::item_iterator it = tbb.items.begin();
	ToolbarBackend::item_iterator end = tbb.items.end();
	for (; it != end; ++it)
		add(it->first, it->second);

}


XFormsToolbar::~XFormsToolbar()
{
	fl_freeze_form(owner_.getForm());

	// G++ vector does not have clear defined
	//toollist.clear();
	toollist_.erase(toollist_.begin(), toollist_.end());

	fl_unfreeze_form(owner_.getForm());
}


namespace {

extern "C" {

void C_ToolbarCB(FL_OBJECT * ob, long ac)
{
	if (!ob || !ob->u_vdata)
		return;

	XFormsToolbar * ptr = static_cast<XFormsToolbar *>(ob->u_vdata);
	XFormsView & owner = ptr->owner_;
	owner.getLyXFunc().dispatch(ptr->funcs[ac], true);
}

} // extern "C"

} // namespace anon


void XFormsToolbar::hide(bool update_metrics)
{
 	toolbar_->set(Box::Invisible);
	if (update_metrics)
		owner_.updateMetrics();
}


void XFormsToolbar::show(bool update_metrics)
{
 	toolbar_->set(Box::Visible);
	toolbar_->show();
	if (update_metrics)
		owner_.updateMetrics();
}


void XFormsToolbar::add(FuncRequest const & func, string const & tooltip)
{
	toolbarItem item;
	item.func = func;

	switch (func.action) {
	case ToolbarBackend::SEPARATOR:
		toolbar_buttons_->push_back(Box(sepspace, sepspace));
		break;

	case ToolbarBackend::MINIBUFFER:
		// Not implemented.
		// XForms uses the same widget to display the buffer messages
		// and to input commands.
		break;

	case ToolbarBackend::LAYOUTS:
		layout_.reset(new XLayoutBox(owner_, *this));
		break;

	default: {
		FL_OBJECT * obj;

		toolbar_buttons_->push_back(Box(standardspacing,
						standardspacing));

		item.icon = obj =
			fl_add_pixmapbutton(FL_NORMAL_BUTTON,
					    0, 0, 0, 0, "");

		widgets_.add(obj, *toolbar_buttons_, buttonwidth, height);

		fl_set_object_resize(obj, FL_RESIZE_ALL);

		int gravity = 0;
		if (position_ == XFormsView::Top ||
		    position_ == XFormsView::Left)
			gravity = NorthWestGravity;
		else if (position_ == XFormsView::Right)
			gravity = NorthEastGravity;
		else if (position_ == XFormsView::Bottom)
			gravity = SouthWestGravity;
		
		fl_set_object_gravity(obj, gravity, gravity);

		Funcs::iterator fit = funcs.insert(funcs.end(), func);
		int const index = distance(funcs.begin(), fit);
		fl_set_object_callback(obj, C_ToolbarCB, index);
		// Remove the blue feedback rectangle
		fl_set_pixmapbutton_focus_outline(obj, 0);

		tooltip_->init(obj, tooltip);

		// The view that this object belongs to.
		obj->u_vdata = this;

		string const xpm = toolbarbackend.getIcon(func);
		fl_set_pixmapbutton_file(obj, xpm.c_str());
		break;
	}
	}

	toollist_.push_back(item);
}


void XFormsToolbar::update()
{
	ToolbarList::const_iterator p = toollist_.begin();
	ToolbarList::const_iterator end = toollist_.end();
	for (; p != end; ++p) {
		if (!p->icon)
			continue;

		FuncStatus const status =
			owner_.getLyXFunc().getStatus(p->func);

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
		if (status.enabled()) {
			fl_activate_object(p->icon);
		} else {
			// Is there a way here to specify a
			// mask in order to show that the
			// button is disabled? (JMarc)
			fl_deactivate_object(p->icon);
		}
	}

	bool const enable = owner_.getLyXFunc().
		getStatus(FuncRequest(LFUN_LAYOUT)).enabled();

	if (layout_.get())
		layout_->setEnabled(enable);
}


namespace {

extern "C"
void C_LayoutBoxSelectedCB(FL_OBJECT * ob, long)
{
	if (!ob || !ob->u_vdata)
		return;
	XLayoutBox * ptr = static_cast<XLayoutBox *>(ob->u_vdata);
	ptr->selected();
}

} // namespace anon


XLayoutBox::XLayoutBox(LyXView & owner, XFormsToolbar & toolbar)
	: owner_(owner)
{
	toolbar.toolbar_buttons_->push_back(Box(standardspacing, 0));

	combox_ = fl_add_combox(FL_DROPLIST_COMBOX,
				0, 0, 135, height, "");

	toolbar.widgets_.add(combox_, *toolbar.toolbar_buttons_, 135, height);

	fl_set_combox_browser_height(combox_, 400);
	fl_set_object_boxtype(combox_, FL_DOWN_BOX);
	fl_set_object_color(combox_, FL_MCOL, FL_MCOL);
	fl_set_object_gravity(combox_, FL_NorthWest, FL_NorthWest);
	fl_set_object_resize(combox_, FL_RESIZE_ALL);

	combox_->u_vdata = this;
	fl_set_object_callback(combox_, C_LayoutBoxSelectedCB, 0);
}


void XLayoutBox::set(string const & layout)
{
	LyXTextClass const & tc = getTextClass(owner_);

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


void XLayoutBox::update()
{
	LyXTextClass const & tc = getTextClass(owner_);

	fl_clear_combox(combox_);

	LyXTextClass::const_iterator it = tc.begin();
	LyXTextClass::const_iterator const end = tc.end();
	for (; it != end; ++it) {
		// ignore obsolete entries
		if ((*it)->obsoleted_by().empty()) {
			string const & name = _((*it)->name());
			fl_addto_combox(combox_, name.c_str());
		}
	}

	// we need to do this.
	fl_redraw_object(combox_);
}


void XLayoutBox::clear()
{
	fl_clear_combox(combox_);
	fl_redraw_object(combox_);
}


void XLayoutBox::open()
{
	fl_show_combox_browser(combox_);
}


void XLayoutBox::setEnabled(bool enable)
{
	::setEnabled(combox_, enable);
}


void XLayoutBox::selected()
{
	string const layoutguiname = getString(combox_);

	LyXTextClass const & tc = getTextClass(owner_);

	LyXTextClass::const_iterator it  = tc.begin();
	LyXTextClass::const_iterator const end = tc.end();
	for (; it != end; ++it) {
		string const & name = (*it)->name();
		if (_(name) == layoutguiname) {
			owner_.getLyXFunc()
				.dispatch(FuncRequest(LFUN_LAYOUT, name),
					  true);
			return;
		}
	}
	lyxerr << "ERROR (XLayoutBox::selected): layout not found!" << endl;
}
