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

#include "TextClass.h"
#include "ToolbarBackend.h"
#include "Session.h"

namespace lyx {
namespace frontend {

class Toolbars {
public:
	///
	Toolbars();
	virtual ~Toolbars() {}

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
	virtual bool visible(std::string const & name) const = 0;

	/// save toolbar information
	virtual void saveToolbarInfo() = 0;

	/// Select the right layout in the combox.
	virtual void setLayout(docstring const & layout) = 0;

protected:
	/// Add a new toolbar. if newline==true, start from a new line
	virtual void add(ToolbarInfo const & tbinfo, bool newline) = 0;
	/// Show or hide a toolbar.
	virtual void displayToolbar(ToolbarInfo const & tbinfo, bool show) = 0;
	/// Update the state of the icons
	virtual void updateIcons() = 0;

	// load flags with saved values
	void initFlags(ToolbarInfo & tbinfo);
};

} // namespace frontend
} // namespace lyx

#endif // NOT TOOLBARS_H
