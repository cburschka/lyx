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

#include "Session.h"

#include <QList>
#include <QToolBar>
#include <QComboBox>

class QStandardItemModel;

namespace lyx {

class Inset;
class DocumentClass;
class ToolbarItem;

namespace frontend {

class GuiCommandBuffer;
class GuiFilterProxyModel;
class GuiView;
class Action;


class GuiLayoutBox : public QComboBox
{
	Q_OBJECT
public:
	GuiLayoutBox(GuiView &);

	/// select the right layout in the combobox.
	void set(docstring const & layout);
	/// Populate the layout combobox.
	void updateContents(bool reset);
	/// Add Item to Layout box according to sorting settings from preferences
	void addItemSort(docstring const & item, bool sorted);

	///
	bool eventFilter(QObject *o, QEvent *e);
	///
	QString const & filter() { return filter_; }

private Q_SLOTS:
	///
	void selected(int index);

private:
	///
	void resetFilter();
	///
	void setFilter(QString const & s);

	///
	GuiView & owner_;
	///
	DocumentClass const * text_class_;
	///
	Inset const * inset_;
	
	/// the layout model: 1st column translated, 2nd column raw layout name
	QStandardItemModel * model_;
	/// the proxy model filtering \c model_
	GuiFilterProxyModel * filterModel_;
	/// the (model-) index of the last successful selection
	int lastSel_;
	/// the character filter
	QString filter_;
};


class GuiToolbar : public QToolBar
{
	Q_OBJECT
public:
	GuiToolbar(ToolbarInfo const &, GuiView &);

	/// Add a button to the bar.
	void add(ToolbarItem const & item);
	/** update toolbar information
	* ToolbarInfo will then be saved by session
	*/
	void saveInfo(ToolbarSection::ToolbarInfo & info);
	/// Refresh the contents of the bar.
	void updateContents();
	///
	GuiCommandBuffer * commandBuffer() { return command_buffer_; }

	Action * addItem(ToolbarItem const & item);

Q_SIGNALS:
	void updated();

private:

	QList<Action *> actions_;
	GuiView & owner_;

	GuiLayoutBox * layout_;
	GuiCommandBuffer * command_buffer_;
};

} // namespace frontend
} // namespace lyx

#endif // GUITOOLBAR_H
