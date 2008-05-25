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

#include "support/docstring.h"

#include <map>

namespace lyx {

class DocumentClass;

namespace frontend {

class GuiToolbar;
class GuiView;
class ToolbarInfo;

class GuiToolbars
{
public:
	///
	GuiToolbars(GuiView & owner);

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
	void update(bool in_math, bool in_table, bool review, 
		    bool in_mathmacrotemplate);

	/// Is the Toolbar currently visible?
	bool visible(std::string const & name) const;

	/// save toolbar information
	void saveToolbarInfo();

	/// Show or hide the command buffer.
	void showCommandBuffer(bool show_it);

	/// toggle visibility of toolbars and save its flags for return
	void toggleFullScreen(bool start_full_screen);

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
	GuiView & owner_;

	/// Toolbar store providing access to individual toolbars by name.
	typedef std::map<std::string, GuiToolbar *> ToolbarsMap;
	ToolbarsMap toolbars_;

	/// The last textclass layout list in the layout choice selector
	DocumentClass * last_textclass_;
};


} // namespace frontend
} // namespace lyx

#endif // GUI_TOOLBARS_H
