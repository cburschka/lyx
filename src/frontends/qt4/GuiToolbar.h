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

#include <QAbstractProxyModel>
#include <QComboBox>
#include <QList>
#include <QToolBar>

class QSortFilterProxyModel;
class QStandardItemModel;

namespace lyx {

class DocumentClass;
class Inset;

namespace frontend {

class Action;
class GuiCommandBuffer;
class GuiLayoutFilterModel;
class GuiToolbar;
class GuiView;
class LayoutItemDelegate;
class ToolbarInfo;
class ToolbarItem;

class GuiLayoutBox : public QComboBox
{
	Q_OBJECT
public:
	GuiLayoutBox(GuiToolbar * bar, GuiView &);

	/// select the right layout in the combobox.
	void set(docstring const & layout);
	/// Populate the layout combobox.
	void updateContents(bool reset);
	/// Add Item to Layout box according to sorting settings from preferences
	void addItemSort(docstring const & item, docstring const & category,
		bool sorted, bool sortedByCat, bool unknown);

	///
	void showPopup();
	
	///
	bool eventFilter(QObject * o, QEvent * e);
	///
	QString const & filter() { return filter_; }

private Q_SLOTS:
	///
	void selected(int index);
	///
	void setIconSize(QSize size);

private:
	friend class LayoutItemDelegate;

	///
	void resetFilter();
	///
	void setFilter(QString const & s);
	///
	QString charFilterRegExp(QString const & filter);
	///
	void countCategories();
	
	///
	GuiView & owner_;
	///
	GuiToolbar * bar_;
	///
	DocumentClass const * text_class_;
	///
	Inset const * inset_;
	
	/// the layout model: 1st column translated, 2nd column raw layout name
	QStandardItemModel * model_;
	/// the proxy model filtering \c model_
	GuiLayoutFilterModel * filterModel_;
	/// the (model-) index of the last successful selection
	int lastSel_;
	/// the character filter
	QString filter_;
	///
	LayoutItemDelegate * layoutItemDelegate_;
	///
	unsigned visibleCategories_;
	///
	bool inShowPopup_;
};


class GuiToolbar : public QToolBar
{
	Q_OBJECT
public:
	///
	GuiToolbar(ToolbarInfo const &, GuiView &);

	///
	void setVisibility(int visibility);

	/// Add a button to the bar.
	void add(ToolbarItem const & item);

	/// Session key.
	/**
	 * This key must be used for any session setting.
	 **/
	QString sessionKey() const;
	/// Save session settings.
	void saveSession() const;
	/// Restore session settings.
	void restoreSession();

	/// Refresh the contents of the bar.
	void update(bool in_math, bool in_table, bool review,
		bool in_mathmacrotemplate);

	///
	void toggle();

	///
	GuiCommandBuffer * commandBuffer() { return command_buffer_; }

	///
	Action * addItem(ToolbarItem const & item);

Q_SIGNALS:
	///
	void updated();

private:
	// load flags with saved values
	void initFlags();
	///
	void fill();
	///
	void showEvent(QShowEvent *);

	///
	QString name_;
	///
	QList<Action *> actions_;
	/// initial visibility flags
	int visibility_;
	///
	bool allowauto_;
	///
	GuiView & owner_;
	///
	GuiLayoutBox * layout_;
	///
	GuiCommandBuffer * command_buffer_;
	///
	ToolbarInfo const & tbinfo_;
	///
	bool filled_;
};

} // namespace frontend
} // namespace lyx

#endif // GUITOOLBAR_H
