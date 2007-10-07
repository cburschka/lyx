// -*- C++ -*-
/**
 * \file GuiViewSource.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIVIEWSOURCE_H
#define GUIVIEWSOURCE_H

#include "ui_ViewSourceUi.h"

#include "Dialog.h"
#include "GuiView.h"
#include "qt_helpers.h"
#include "debug.h"

#include <QDockWidget>
#include <QWidget>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

namespace lyx {
namespace frontend {

class ControlViewSource;
class LaTeXHighlighter;

class GuiViewSourceDialog : public QWidget, public Ui::ViewSourceUi
{
	Q_OBJECT

public:
	GuiViewSourceDialog(ControlViewSource &);

public Q_SLOTS:
	// update content
	void updateView();
	///
	QTextDocument * document() { return document_; }
	///
	void update(bool full_source);

private:
	///
	ControlViewSource & controller_;	
	///
	QTextDocument * document_;
	/// LaTeX syntax highlighter
	LaTeXHighlighter * highlighter_;
};

/**
 * A controller for a read-only text browser.
 */
class ControlViewSource : public Controller {
public:
	///
	ControlViewSource(Dialog &);
	/** \param source source code to be displayed
	 */
	bool initialiseParams(std::string const & source);
	///
	void clearParams() {}
	///
	void dispatchParams() {}
	///
	bool isBufferDependent() const { return true; }
	///
	bool canApply() const { return true; }
	///
	bool canApplyToReadOnly() const { return true; }

	/// The title displayed by the dialog reflects source type.
	docstring const title() const;

	/** get the source code of selected paragraphs, or the whole document
		\param fullSource get full source code
	 */
	docstring const updateContent(bool fullSource);
	/** get the cursor position in the source code
	 */
	std::pair<int, int> getRows() const;
};


class GuiViewSource : public QDockWidget, public Dialog
{
public:
	GuiViewSource(GuiViewBase & parent)
		: QDockWidget(&parent, Qt::WindowFlags(0)), name_("view-source")
	{
		ControlViewSource * c = new ControlViewSource(*this);
		controller_ = c;
		controller_->setLyXView(parent);
		widget_ = new GuiViewSourceDialog(*c);
		setWidget(widget_);
		setWindowTitle(widget_->windowTitle());
		parent.addDockWidget(Qt::BottomDockWidgetArea, this);
	}
	~GuiViewSource() { delete widget_; delete controller_; }

	/// Dialog inherited methods
	//@{
	void applyView() {}
	void hideView()	{ QDockWidget::hide(); }
	void showData(std::string const & data)
	{
		controller_->initialiseParams(data);
		showView();
	}
	void showView()
	{
		widget_->updateView();  // make sure its up-to-date
		QDockWidget::show();
	}
	bool isVisibleView() const { return QDockWidget::isVisible(); }
	void checkStatus() { updateView(); }
	void redraw() { redrawView(); }
	void redrawView() {}
	void updateData(std::string const & data)
	{
		controller_->initialiseParams(data);
		updateView();
	}
	void updateView()
	{
		widget_->updateView();
		QDockWidget::update();
	}
	bool isClosing() const { return false; }
	void partialUpdateView(int /*id*/) {}
	Controller & controller() { return *controller_; }
	std::string name() const { return name_; }
	//@}
private:
	/// The encapsulated widget.
	GuiViewSourceDialog * widget_;
	Controller * controller_;
	std::string name_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIVIEWSOURCE_H
