// -*- C++ -*-
/**
 * \file Toolbars.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * The Toolbars class is a container of toolbars.
 * It provides accessors to each Toolbar and to the LayoutBox.
 *
 * Each GUI frontend should provide toolbar and layout boxes by derivation
 * from the LayoutBox and Toolbar pure abstract classes.
 *
 * The Toolbars class has no knowledge at all of the details of each
 * frontend's implementation, which requires that each frontend should
 * provide a 'make_toolbar' function, signature below.
 */

#ifndef TOOLBARS_H
#define TOOLBARS_H

#include "ToolbarBackend.h"
#include <boost/shared_ptr.hpp>
#include <map>

class LyXView;


class LayoutBox {
public:
	virtual ~LayoutBox() {}
	/// Select the correct layout in the combox.
	virtual void set(std::string const & layout) = 0;
	/// Populate the layout combox.
	virtual void update() = 0;
	/// Erase the layout list.
	virtual void clear() = 0;
	/// Display the layout list.
	virtual void open() = 0;
	/// Set the activation status of the combox.
	virtual void setEnabled(bool) = 0;
};


class Toolbar {
public:
	virtual ~Toolbar() {}
	/// Add a button to the bar.
	virtual void add(FuncRequest const & func,
			 std::string const & tooltip) = 0;

	/** Hide the bar.
	 *  \param update_metrics is a hint to the layout engine that the
	 *  metrics should be updated.
	 */
	virtual void hide(bool update_metrics) = 0;
	/** Show the bar.
	 *  \param update_metrics is a hint to the layout engine that the
	 *  metrics should be updated.
	 */
	virtual void show(bool update_metrics) = 0;

	/// Refresh the contents of the bar.
	virtual void update() = 0;
	/// Accessor to the layout combox, if any.
	virtual LayoutBox * layout() const = 0;
};


class Toolbars {
public:
	///
	Toolbars(LyXView & owner);

	/// Initialize the toolbars using the backend database.
	void init();

	/// Show/hide the named toolbar.
	void display(std::string const & name, bool show);
	/// Update the state of the toolbars.
	void update(bool in_math, bool in_table);

	/// Select the right layout in the combox.
	void setLayout(std::string const & layout);

	/** Populate the layout combox - returns whether we did a full
	 *  update or not
	 */
	bool updateLayoutList(int textclass);

	/// Drop down the layout list.
	void openLayoutList();
	/// Erase the layout list.
	void clearLayoutList();

	///
	typedef boost::shared_ptr<Toolbar> ToolbarPtr;

private:
	/// Add a new toolbar.
	void add(ToolbarBackend::Toolbar const & tb);
	/// Show or hide a toolbar.
	void displayToolbar(ToolbarBackend::Toolbar const & tb, bool show);
	/// Update the state of the icons
	void update();

	/// The parent window.
	LyXView & owner_;

	/** The layout box is actually owned by whichever toolbar
	 *  contains it. All the Toolbars class needs is a means of
	 *  accessing it.
	 *
	 *  We don't need to use boost::weak_ptr here because the toolbars
	 *  are also stored here. There are, therefore, no lifetime issues.
	 */
	LayoutBox * layout_;

	/// Toolbar store providing access to individual toolbars by name.
	typedef std::map<std::string, ToolbarPtr> ToolbarsMap;
	ToolbarsMap toolbars_;

	/// The last textclass layout list in the layout choice selector
	int last_textclass_;
};


/** Each GUI frontend should provide its own version of this.
 */
Toolbars::ToolbarPtr make_toolbar(ToolbarBackend::Toolbar const &, LyXView &);

#endif // NOT TOOLBARS_H
