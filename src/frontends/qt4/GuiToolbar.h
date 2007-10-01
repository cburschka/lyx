// -*- C++ -*-
/**
 * \file GuiToolbar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUITOOLBAR_H
#define GUITOOLBAR_H

#include "frontends/Toolbars.h"
#include "Session.h"

#include <QList>
#include <QToolBar>

class QComboBox;

namespace lyx {

class FuncRequest;

namespace frontend {

class GuiCommandBuffer;
class GuiLayoutBox;
class GuiViewBase;
class Action;


class GuiLayoutBox : public QComboBox
{
	Q_OBJECT
public:
	GuiLayoutBox(GuiViewBase &);

	/// select the right layout in the combobox.
	void set(docstring const & layout);
	/// Populate the layout combox.
	void updateContents();

private Q_SLOTS:
	void selected(const QString & str);

private:
	GuiViewBase & owner_;
};


class GuiToolbar : public QToolBar
{
	Q_OBJECT
public:
	GuiToolbar(ToolbarInfo const &, GuiViewBase &);

	/// Add a button to the bar.
	void add(ToolbarItem const & item);
	/** update toolbar information
	* ToolbarInfo will then be saved by session
	*/
	void saveInfo(ToolbarSection::ToolbarInfo & info);
	/// Refresh the contents of the bar.
	void updateContents();
	GuiLayoutBox * layout() const { return layout_; }
	/// Set the focus on the command buffer, if any.
	void focusCommandBuffer();

Q_SIGNALS:
	void updated();

private:
	Action * addItem(ToolbarItem const & item);

	QList<Action *> actions_;
	GuiViewBase & owner_;

	GuiLayoutBox * layout_;
	GuiCommandBuffer * command_buffer_;
};

/// return a full path of an XPM for the given action
std::string const getIcon(FuncRequest const & f, bool unknown = true);

} // namespace frontend
} // namespace lyx

#endif // GUITOOLBAR_H
