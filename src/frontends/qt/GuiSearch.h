// -*- C++ -*-
/**
 * \file GuiSearch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISEARCH_H
#define GUISEARCH_H

#include "GuiDialog.h"
#include "DockView.h"

#include <QDockWidget>

#include "ui_SearchUi.h"

namespace lyx {
namespace frontend {

class GuiSearch;

class GuiSearchWidget : public QWidget, public Ui::SearchUi
{
	Q_OBJECT

public:
	GuiSearchWidget(QWidget * parent);
	///
	void saveSession(QSettings & settings, QString const & session_key) const;
	///
	void restoreSession(QString const & session_key);
	///
	void setBufferView(BufferView const * bv) { bv_ = bv; }
	///
	bool isMinimized() { return minimized_; }

private Q_SLOTS:
	void findChanged();
	void findClicked(bool const backwards = false);
	void findPrevClicked();
	void replaceClicked(bool const backwards = false);
	void replacePrevClicked();
	void replaceallClicked();
	void minimizeClicked(bool const toggle = true);
Q_SIGNALS:
	void needTitleBarUpdate() const;
	void needSizeUpdate() const;

private:
	///
	void keyPressEvent(QKeyEvent * e) override;
	///
	void showEvent(QShowEvent * e) override;
	/// Searches occurrence of string
	void find(docstring const & search,
		  bool casesensitive, bool matchword, bool forward);
	/// Replaces occurrence of string
	void replace(docstring const & search, docstring const & replace,
		     bool casesensitive, bool matchword,
		     bool forward, bool all);
	///
	BufferView const * bv_;
	///
	bool minimized_ = false;
};


class GuiSearch : public DockView
{
	Q_OBJECT

public:
	GuiSearch(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area = Qt::BottomDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = {});

	/// Controller inherited method.
	///@{
	bool initialiseParams(std::string const &) override { return true; }
	void clearParams() override {}
	void dispatchParams() override {}
	bool isBufferDependent() const override { return true; }
	void updateView() override;
	void saveSession(QSettings & settings) const override;
	void restoreSession() override;
	bool wantInitialFocus() const override { return true; }
	///@}

public Q_SLOTS:
	///
	void onBufferViewChanged() override;

private Q_SLOTS:
	/// update title display
	void updateTitle();
	/// update dock size
	void updateSize();

private:
	/// The encapsulated widget.
	GuiSearchWidget * widget_;
};

} // namespace frontend
} // namespace lyx

#endif // GUISEARCH_H
