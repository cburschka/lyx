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
#include "Session.h"

#include <boost/shared_ptr.hpp>

#include <map>

namespace lyx {
namespace frontend {

class LyXView;

class LayoutBox {
public:
	virtual ~LayoutBox() {}
	/// Select the correct layout in the combox.
	virtual void set(docstring const & layout) = 0;
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
	virtual void add(ToolbarItem const & item) = 0;

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
	/** update toolbar information
	* ToolbarInfo will then be saved by session
	*/
	virtual void saveInfo(ToolbarSection::ToolbarInfo & tbinfo) = 0;

	/// whether toolbar is visible
	virtual bool isVisible() const = 0;
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

	/// get toolbar info
	ToolbarInfo * getToolbarInfo(std::string const & name);

	/** toggle the state of toolbars (on/off/auto). Skip "auto"
	 * when allowauto is false.
	 */
	void toggleToolbarState(std::string const & name, bool allowauto);

	/// Update the state of the toolbars.
	void update(bool in_math, bool in_table, bool review);

	/// Is the Toolbar currently visible?
	bool visible(std::string const & name) const;

	/// save toolbar information
	void saveToolbarInfo();

	/// Select the right layout in the combox.
	void setLayout(docstring const & layout);

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
	/// Add a new toolbar. if newline==true, start from a new line
	void add(ToolbarInfo const & tbinfo, bool newline);
	/// Show or hide a toolbar.
	void displayToolbar(ToolbarInfo const & tbinfo, bool show);
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

	// load flags with saved values
	void initFlags(ToolbarInfo & tbinfo);
};

/// Set the layout in the kernel when an entry has been selected
void layoutSelected(LyXView & lv, docstring const & name);


} // namespace frontend
} // namespace lyx

#endif // NOT TOOLBARS_H
