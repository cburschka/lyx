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
#include "FancyLineEdit.h"

#include <QMenu>
#include <QDockWidget>

#include "ui_SearchUi.h"

namespace lyx {
namespace frontend {

class GuiSearch;

class GuiSearchWidget : public QWidget, public Ui::SearchUi
{
	Q_OBJECT

public:
	GuiSearchWidget(QWidget * parent, GuiView & view);
	///
	void saveSession(QSettings & settings, QString const & session_key) const;
	///
	void restoreSession(QString const & session_key);
	///
	void setBufferView(BufferView const * bv) { bv_ = bv; }
	///
	bool initialiseParams(std::string const &);
	///
	bool isMinimized() { return minimized_; }

private Q_SLOTS:
	void findChanged();
	void findBufferChanged();
	void findClicked();
	void findPrevClicked();
	void replaceClicked();
	void replacePrevClicked();
	void replaceallClicked();
	void minimizeClicked(bool const toggle = true);
	void caseSenseActTriggered();
	void wholeWordsActTriggered();
	void searchSelActTriggered();
	void immediateActTriggered();
	void wrapActTriggered();
Q_SIGNALS:
	void needTitleBarUpdate() const;
	void needSizeUpdate() const;

private:
	///
	void keyPressEvent(QKeyEvent * e) override;
	///
	void showEvent(QShowEvent * e) override;
	///
	void hideEvent(QHideEvent * e) override;
	///
	void doFind(bool const backwards = false,
		    bool const instant = false);
	///
	void doReplace(bool const backwards = false);
	/// Searches occurrence of string
	void find(docstring const & search,
		  bool casesensitive, bool matchword,
		  bool forward, bool instant, bool wrap, bool onlysel);
	/// Replaces occurrence of string
	void replace(docstring const & search, docstring const & replace,
		     bool casesensitive, bool matchword,
		     bool forward, bool all, bool wrap, bool onlysel);
	///
	void handleIndicators();
	///
	BufferView const * bv_ = nullptr;
	///
	GuiView & view_;
	///
	bool minimized_ = false;
	/// contains the search box
	FancyLineEdit * findLE_;
	/// The options menu
	QMenu * menu_;
	/// And its actions
	QAction * act_casesense_ = new QAction(this);
	///
	QAction * act_wholewords_ = new QAction(this);
	///
	QAction * act_selection_ = new QAction(this);
	///
	QAction * act_immediate_ = new QAction(this);
	///
	QAction * act_wrap_ = new QAction(this);
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
	bool initialiseParams(std::string const & sdata) override
		{ return widget_->initialiseParams(sdata); }
	void clearParams() override {}
	void dispatchParams() override {}
	bool isBufferDependent() const override { return true; }
	void updateView() override;
	void saveSession(QSettings & settings) const override;
	void restoreSession() override;
	///@}

protected:
	bool wantInitialFocus() const override { return true; }
	void mouseMoveEvent(QMouseEvent * event) override;
	void mousePressEvent(QMouseEvent * event) override;
	void mouseDoubleClickEvent(QMouseEvent *event) override;

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
	///
	QPoint dragPosition;
};

} // namespace frontend
} // namespace lyx

#endif // GUISEARCH_H
