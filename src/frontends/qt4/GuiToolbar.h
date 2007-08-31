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

#include <QToolBar>

#include <boost/scoped_ptr.hpp>

#include <vector>

class QComboBox;

namespace lyx {

class FuncRequest;

namespace frontend {

class GuiCommandBuffer;
class GuiLayoutBox;
class GuiViewBase;
class Action;


class GuiLayoutBox : public QObject, public LayoutBox {
	Q_OBJECT
public:
	GuiLayoutBox(QToolBar *, GuiViewBase &);

	/// select the right layout in the combox.
	void set(docstring const & layout);
	/// Populate the layout combox.
	void update();
	/// Erase the layout list.
	void clear();
	/// Display the layout list.
	void open();
	///
	void setEnabled(bool);

private Q_SLOTS:
	void selected(const QString & str);

private:
	QComboBox * combo_;
	GuiViewBase & owner_;
};


class GuiToolbar : public QToolBar, public Toolbar {
	Q_OBJECT
public:
	GuiToolbar(ToolbarInfo const &, GuiViewBase &);

	//~GuiToolbar();

	void add(ToolbarItem const & item);
	void hide(bool);
	void show(bool);
	bool isVisible() const;
	void saveInfo(ToolbarSection::ToolbarInfo & info);
	void update();
	LayoutBox * layout() const { return layout_.get(); }
	///
	void focusCommandBuffer();

Q_SIGNALS:
	void updated();

private:
	GuiCommandBuffer * command_buffer_;

	std::vector<Action *> ActionVector;
	GuiViewBase & owner_;

	boost::scoped_ptr<GuiLayoutBox> layout_;
};

/// return a full path of an XPM for the given action
std::string const getIcon(FuncRequest const & f, bool unknown = true);

} // namespace frontend
} // namespace lyx

#endif // GUITOOLBAR_H
