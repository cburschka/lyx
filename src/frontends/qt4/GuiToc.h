// -*- C++ -*-
/**
 * \file GuiToc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author Angus Leeming
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUITOC_H
#define GUITOC_H

#include "DockView.h"

#include "insets/InsetCommandParams.h"

#include "TocBackend.h"

#include <QObject>
#include <QStandardItemModel>
#include <QStringListModel>

#include <vector>

namespace lyx {
namespace frontend {

class TocModel;
class TocWidget;

class GuiToc : public DockView
{
	Q_OBJECT

public:
	///
	GuiToc(
		GuiViewBase & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area = Qt::LeftDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);

	///
	bool initialiseParams(std::string const & data);
	///
	void updateView();
	/// Test if outlining operation is possible
	bool canOutline(int type) const;

	QStandardItemModel * tocModel(int type);
	///
	QModelIndex currentIndex(int type) const;
	///
	void goTo(int type, QModelIndex const & index);
	///
	int getType();
	///
	int getTocDepth(int type);

Q_SIGNALS:
	/// Signal that the internal toc_models_ has been reset.
	void modelReset();

private:
	///
	TocWidget * widget_;
	///
	std::vector<TocModel *> toc_models_;

public:
	///
	TocList const & tocs() const;

	/// Return the list of types available
	std::vector<docstring> const & typeNames() const
	{ return type_names_; }

	///
	int selectedType() { return selected_type_; }

	/// Return the first TocItem before the cursor
	TocIterator currentTocItem(int type) const;

	/// Apply the selected outlining operation
	void outlineUp();
	///
	void outlineDown();
	///
	void outlineIn();
	///
	void outlineOut();
	///
	void updateBackend();

	std::vector<std::string> types_;
	std::vector<docstring> type_names_;
	int selected_type_;

	/// Return the guiname from a given cmdName of the TOC param
	docstring guiName(std::string const & type) const;

	/// clean-up on hide.
	void clearParams() { params_.clear(); }
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

private:
	///
	InsetCommandParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUITOC_H
