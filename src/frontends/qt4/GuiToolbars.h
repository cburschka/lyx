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

#include "frontends/Toolbars.h"

#include <map>

namespace lyx {
namespace frontend {

class GuiLayoutBox;
class GuiToolbar;
class GuiViewBase;

class GuiToolbars : public Toolbars
{
public:
	///
	GuiToolbars(GuiViewBase & owner);

	bool visible(std::string const & name) const;
	void saveToolbarInfo();
	void setLayout(docstring const & layout);

	/** Populate the layout combox - returns whether we did a full
	 *  update or not
	 */
	bool updateLayoutList(TextClassPtr textclass);

	/// Drop down the layout list.
	void openLayoutList();
	/// Erase the layout list.
	void clearLayoutList();

protected:
	void add(ToolbarInfo const & tbinfo, bool newline);
	void displayToolbar(ToolbarInfo const & tbinfo, bool show);
	void updateIcons();

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
