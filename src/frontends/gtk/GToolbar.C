/**
 * \file GToolbar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
#include <gtkmm.h>

#include "GToolbar.h"
#include "GView.h"
#include "LyXAction.h"
#include "lyxfunc.h"
#include "FuncStatus.h"
#include "buffer.h"
#include "funcrequest.h"
#include "gettext.h"
#include "Tooltips.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "debug.h"


namespace
{


char const * gToolData = "tool_data";


inline void comboClear(Gtk::Combo & combo)
{
	std::vector<Glib::ustring> strings;
	strings.push_back("");
	combo.set_popdown_strings(strings);
}


inline bool comboIsEmpty(Gtk::Combo & combo)
{
	std::vector<Glib::ustring> strings = combo.get_popdown_strings();
	return (strings.empty() || (strings.size() == 1 && strings[0] == ""));
}


}


GToolbar::GToolbar(LyXView * lyxView, int /*x*/, int /*y*/)
	: view_(lyxView), internal_(false)
{
	combo_.set_value_in_list();
	combo_.get_entry()->set_editable(false);
	combo_.unset_flags(Gtk::CAN_FOCUS | Gtk::CAN_DEFAULT);
	combo_.get_entry()->unset_flags(Gtk::CAN_FOCUS | Gtk::CAN_DEFAULT);
	comboClear(combo_);
	combo_.get_entry()->signal_changed().connect(
		SigC::slot(*this,
			   &GToolbar::onLayoutSelected));
	GView * gview = static_cast<GView*>(lyxView);
	vbox_.show();
	Gtk::VBox & vbox = gview->getVBox();
	vbox.children().push_back(Gtk::Box_Helpers::Element(vbox_,
							    Gtk::PACK_SHRINK));
}


GToolbar::~GToolbar()
{
}


void GToolbar::add(ToolbarBackend::Toolbar const & tb)
{
	Gtk::Toolbar * toolbar = manage(new Gtk::Toolbar());
	ToolbarBackend::item_iterator it = tb.items.begin();
	ToolbarBackend::item_iterator end = tb.items.end();
	for (; it != end; ++it)
		add(toolbar, it->first, it->second);
	toolbar->set_toolbar_style(Gtk::TOOLBAR_ICONS);
	toolbar->show();
	vbox_.children().push_back(
		Gtk::Box_Helpers::Element(*toolbar,
					  Gtk::PACK_SHRINK));
	toolbars_.push_back(toolbar);
}


void GToolbar::add(Gtk::Toolbar * toolbar, 
			 int action,
			 string const & tooltip)
{
	switch (action) {
	case ToolbarBackend::SEPARATOR:
		toolbar->tools().push_back(Gtk::Toolbar_Helpers::Space());
		break;
	case ToolbarBackend::MINIBUFFER:
		// Not supported yet.
		break;
	case ToolbarBackend::LAYOUTS:
	{
		combo_.show();
		toolbar->tools().push_back(
			Gtk::Toolbar_Helpers::Element(combo_));
		toolbar->tools().back().get_widget()->set_data(
			gToolData,
			reinterpret_cast<void*>(LFUN_LAYOUT));
		break;
	}
	default:
	{
		Glib::ustring xpmName = 
			Glib::locale_to_utf8(toolbarbackend.getIcon(action));
		Glib::ustring tip = Glib::locale_to_utf8(tooltip);
		if (xpmName.size() == 0) {
			toolbar->tools().push_back(
				Gtk::Toolbar_Helpers::ButtonElem(
					"",
					SigC::bind(SigC::slot(*this, &GToolbar::onButtonClicked),
						   action),
					tip));
		} else {
			Gtk::Image * image = 
				Gtk::manage(new Gtk::Image(xpmName));
			image->show();
			toolbar->tools().push_back(
				Gtk::Toolbar_Helpers::ButtonElem(
					"",
					*image,
					SigC::bind(SigC::slot(*this, &GToolbar::onButtonClicked),
						   action),
					tip));
		}
		toolbar->tools().back().get_content()->set_data(
			gToolData,
			reinterpret_cast<void*>(action));
		break;
	}
	}
}


void GToolbar::onButtonClicked(int action)
{
	view_->getLyXFunc().dispatch(action, true);
}


void GToolbar::onLayoutSelected()
{
	if (internal_)
		return;
	string layoutGuiName = combo_.get_entry()->get_text();
	// we get two signal, one of it is empty and useless
	if (layoutGuiName.empty())
		return;
	LyXTextClass const & tc =
		view_->buffer()->params.getLyXTextClass();

	LyXTextClass::const_iterator end = tc.end();
	for (LyXTextClass::const_iterator cit = tc.begin();
	     cit != end; ++cit) {
		if ((*cit)->name() == layoutGuiName) {
			view_->getLyXFunc().dispatch(
				FuncRequest(LFUN_LAYOUT, (*cit)->name()),
				true);
			return;
		}
	}
	lyxerr << "ERROR (GToolbar::layoutSelected): layout not found! name : "
	       << layoutGuiName
	       << std::endl;
}


void GToolbar::displayToolbar(ToolbarBackend::Toolbar const & /*tb*/, bool /*show*/)
{
}


void GToolbar::update()
{
	std::vector<Gtk::Toolbar*>::iterator itToolbar;
	for (itToolbar = toolbars_.begin();
	     itToolbar != toolbars_.end(); ++itToolbar) {
		Gtk::Toolbar * toolbar = *itToolbar;
		Gtk::Toolbar_Helpers::ToolList::iterator it;
		for (it = toolbar->tools().begin();
		     it != toolbar->tools().end(); ++it) {
			Gtk::Widget * widget;
			switch (it->get_type()) {
			case Gtk::TOOLBAR_CHILD_WIDGET:
				widget = it->get_widget();
				break;
			case Gtk::TOOLBAR_CHILD_SPACE:
				continue;
			default:
				widget = it->get_content();
			}
			int action = reinterpret_cast<int>(
				widget->get_data(gToolData));
			FuncStatus const status = view_->
				getLyXFunc().getStatus(action);
			bool sensitive = !status.disabled();
			widget->set_sensitive(sensitive);
			if (it->get_type() != Gtk::TOOLBAR_CHILD_BUTTON)
				return;
			if (status.onoff(true))
				static_cast<Gtk::Button*>(widget)->
					set_relief(Gtk::RELIEF_NORMAL);
			if (status.onoff(false))
				static_cast<Gtk::Button*>(widget)->
					set_relief(Gtk::RELIEF_NONE);
		}
	}
}


void GToolbar::setLayout(string const & layout)
{
	LyXTextClass const & tc =
		view_->buffer()->params.getLyXTextClass();
	internal_ = true;
	combo_.get_entry()->set_text(tc[layout]->name());
	internal_ = false;
}


void GToolbar::updateLayoutList()
{
	LyXTextClass const & tc =
		view_->buffer()->params.getLyXTextClass();
	LyXTextClass::const_iterator end = tc.end();
	std::vector<Glib::ustring> strings;
	for (LyXTextClass::const_iterator cit = tc.begin();
	     cit != end; ++cit)
		if ((*cit)->obsoleted_by().empty())
			strings.push_back(
				Glib::locale_to_utf8((*cit)->name()));
	internal_ = true;
	combo_.set_popdown_strings(strings);
	internal_ = false;
}


void GToolbar::openLayoutList()
{
	combo_.get_list()->activate();
}


void GToolbar::clearLayoutList()
{
	internal_ = true;
	comboClear(combo_);
	internal_ = false;
}
