// -*- C++ -*-
/**
 * \file XFormsToolbar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XFORMSTOOLBAR_H
#define XFROMSTOOLBAR_H

#include "LayoutEngine.h"
#include "XFormsView.h"

#include "frontends/Toolbars.h"

#include <boost/scoped_ptr.hpp>
#include <vector>


class XFormsToolbar;
class XFormsView;
class Tooltips;


class XLayoutBox: public LayoutBox {
public:
	XLayoutBox(LyXView & owner, XFormsToolbar & toolbar);

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
	///
	void selected();

private:

	FL_OBJECT * combox_;
	LyXView & owner_;
};



/** The LyX xforms toolbar class
 */
class XFormsToolbar : public Toolbar {
	friend class XLayoutBox;

public:
	XFormsToolbar(ToolbarBackend::Toolbar const & tbb, LyXView & o);
	~XFormsToolbar();

	void add(FuncRequest const & func, std::string const & tooltip);
	void hide(bool);
	void show(bool);
 	void update();
	LayoutBox * layout() const { return layout_.get(); }

	/// an item on the toolbar
	struct toolbarItem
	{
		toolbarItem();

		~toolbarItem();

		toolbarItem & operator=(toolbarItem const & ti);

		void generateInactivePixmaps();

		/// deallocate icon
		void kill_icon();

		/// lyx action
		FuncRequest func;
		/// icon for this item
		FL_OBJECT * icon;
		///
		Pixmap unused_pixmap;
		Pixmap active_pixmap;
		Pixmap inactive_pixmap;
		Pixmap mask;
	};

	///
	XFormsView::Position position_;
	///
	lyx::frontend::Box * toolbar_;
	///
	lyx::frontend::BoxList * toolbar_buttons_;
	///
	lyx::frontend::WidgetMap widgets_;

	typedef std::vector<FuncRequest> Funcs;

	Funcs funcs;

	typedef std::vector<toolbarItem> ToolbarList;

	/// The list containing all the buttons
	ToolbarList toollist_;
	/// owning view
	XFormsView & owner_;
	/// tooltips manager
	boost::scoped_ptr<Tooltips> tooltip_;
	/// layout combo
	boost::scoped_ptr<XLayoutBox> layout_;
};

#endif
