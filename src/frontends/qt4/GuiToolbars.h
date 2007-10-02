// -*- C++ -*-
/**
 * \file GuiToolbars.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_TOOLBARS_H
#define GUI_TOOLBARS_H

#include "TextClass.h"
#include "ToolbarBackend.h"
#include "Session.h"

#include <map>

namespace lyx {
namespace frontend {

class GuiLayoutBox;
class GuiToolbar;
class GuiViewBase;

class GuiToolbars
{
public:
	///
	GuiToolbars(GuiViewBase & owner);

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
	bool updateLayoutList(TextClassPtr textclass);

	/// Drop down the layout list.
	void openLayoutList();
	/// Erase the layout list.
	void clearLayoutList();

	/// Show or hide the command buffer.
	void showCommandBuffer(bool show_it);


private:
	/// Add a new toolbar. if newline==true, start from a new line
	void add(ToolbarInfo const & tbinfo, bool newline);

	/// Show or hide a toolbar.
	void displayToolbar(ToolbarInfo const & tbinfo, bool show);

	/// Update the state of the icons
	void updateIcons();

	// load flags with saved values
	void initFlags(ToolbarInfo & tbinfo);

	/// The parent window.
	GuiViewBase & owner_;

	/** The layout box is actually owned by whichever toolbar
	 *  contains it. All the Toolbars class needs is a means of
	 *  accessing it.
	 *
	 *  We don't need to use boost::weak_ptr here because the toolbars
	 *  are also stored here. There are, therefore, no lifetime issues.
	 */
	GuiLayoutBox * layout_;

	/// Toolbar store providing access to individual toolbars by name.
	typedef std::map<std::string, GuiToolbar *> ToolbarsMap;
	ToolbarsMap toolbars_;

	/// The last textclass layout list in the layout choice selector
	TextClassPtr last_textclass_;
};


} // namespace frontend
} // namespace lyx

#endif // NOT GUI_TOOLBARS_H
