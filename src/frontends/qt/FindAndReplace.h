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

#include "DockView.h"
// This is needed so that ui_FindAndReplaceUi.h can find qt_()
#include "qt_helpers.h"
#include "ui_FindAndReplaceUi.h"

#include "lyxfind.h"

#include <string>

namespace lyx {
namespace frontend {

class FindAndReplaceWidget : public QTabWidget, public Ui::FindAndReplaceUi
{
	Q_OBJECT

public:
	FindAndReplaceWidget(GuiView & view);
	bool initialiseParams(std::string const & params);
	void updateGUI();

public Q_SLOTS:
	///
	void dockLocationChanged(Qt::DockWidgetArea area);

private:
	///
	GuiView & view_;
	///
	Buffer * old_buffer_;

	/// add a string to the combo if needed
	void remember(std::string const & find, QComboBox & combo);

	/// Perform the scope-related buffer switch while searching
	bool findAndReplaceScope(FindAndReplaceOptions & opt, bool replace_all = false);

	/// Collect options from the GUI elements, then perform the search
	bool findAndReplace(bool backwards, bool replace, bool replace_all = false);

	/// FIXME Probably to be merged with findAndReplace(bool, bool, bool)
	bool findAndReplace(bool casesensitive, bool matchword, bool backwards,
		bool expandmacros, bool ignoreformat, bool replace,
		bool keep_case, bool replace_all = false);

	bool eventFilter(QObject *obj, QEvent *event);

	void virtual showEvent(QShowEvent *ev);
	void virtual hideEvent(QHideEvent *ev);

	void hideDialog();

protected Q_SLOTS:
	void on_findNextPB_clicked();
	void on_replacePB_clicked();
	void on_replaceallPB_clicked();
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

	bool initialiseParams(std::string const &) override;
	void clearParams() override {}
	void dispatchParams() override {}
	bool isBufferDependent() const override { return false; }
	bool canApplyToReadOnly() const override { return true; }
	void selectAll();

	/// update
	void updateView() override;
	//virtual void update_contents() {}

protected:
	bool wantInitialFocus() const override { return true; }

private:
	/// The encapsulated widget.
	FindAndReplaceWidget * widget_;
};


} // namespace frontend
} // namespace lyx

#endif // QSEARCHADV_H
