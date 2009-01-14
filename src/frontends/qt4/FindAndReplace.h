// -*- C++ -*-
/**
 * \file FindAndReplace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Tommaso Cucinotta
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSEARCHADV_H
#define QSEARCHADV_H

#include "GuiWorkArea.h"

#include "DockView.h"
#include "ui_FindAndReplaceUi.h"

#include "BufferView.h"
#include "Buffer.h"
#include "LyX.h"
#include "LyXFunc.h"
#include "Text.h"

#include <QDialog>

#include <string>

namespace lyx {
namespace frontend {

class FindAndReplaceWidget : public QWidget, public Ui::FindAndReplaceUi
{
	Q_OBJECT

public:
	FindAndReplaceWidget(GuiView & view);
	bool initialiseParams(std::string const & params);

private:
	///
	GuiView & view_;

	// add a string to the combo if needed
	void remember(std::string const & find, QComboBox & combo);
	void findAndReplace(
		bool casesensitive, bool matchword, bool backwards,
		bool expandmacros, bool ignoreformat, bool replace
	);
	void find(docstring const & str, int len, bool casesens,
		  bool words, bool backwards, bool expandmacros);
	void findAndReplace(bool backwards, bool replace);

	bool eventFilter(QObject *obj, QEvent *event);

	void virtual showEvent(QShowEvent *ev);
	void virtual hideEvent(QHideEvent *ev);

protected Q_SLOTS:
	void on_findNextPB_clicked();
	void on_findPrevPB_clicked();
	void on_replaceNextPB_clicked();
	void on_replacePrevPB_clicked();
	void on_replaceallPB_clicked();
	void on_closePB_clicked();
	void on_regexpInsertCombo_currentIndexChanged(int index);
};


class FindAndReplace : public DockView
{
	Q_OBJECT
public:
	FindAndReplace(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area = Qt::RightDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);

	~FindAndReplace();

	bool initialiseParams(std::string const &);
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return true; }
	void selectAll();

	/// update
	void updateView() {}
	//virtual void update_contents() {}

protected:
	virtual bool wantInitialFocus() const { return true; }

private:
	/// The encapsulated widget.
	FindAndReplaceWidget * widget_;
};


} // namespace frontend
} // namespace lyx

#endif // QSEARCHADV_H
