// -*- C++ -*-
/**
 * \file GToolbar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GTOOLBAR_H
#define GTOOLBAR_H

#include "frontends/Toolbars.h"

#include <boost/scoped_ptr.hpp>

#include <gtkmm.h>

namespace lyx {
namespace frontend {

class GView;

class GLayoutBox: public LayoutBox, public sigc::trackable {
public:
	GLayoutBox(LyXView &, Gtk::Toolbar &, FuncRequest const &);

	/// select the right layout in the combox.
	void set(std::string const & layout);
	/// Populate the layout combox.
	void update();
	/// Erase the layout list.
	void clear();
	/// Display the layout list.
	void open();
	///
	void setEnabled(bool);
private:
	///
	void selected();

	Gtk::Combo combo_;
	LyXView & owner_;
	bool internal_;
};


class GToolbar : public Toolbar, public sigc::trackable {
public:
	GToolbar(ToolbarBackend::Toolbar const &, LyXView &);
	void add(FuncRequest const & func, std::string const & tooltip);
	void hide(bool);
	void show(bool);
	void update();
	LayoutBox * layout() const { return layout_.get(); }
private:
	void clicked(FuncRequest);

	GView & owner_;
	Gtk::Toolbar toolbar_;
	Gtk::Tooltips tooltips_;
	boost::scoped_ptr<GLayoutBox> layout_;
};

} // namespace frontend
} // namespace lyx

#endif // NOT GTOOLBAR_H
