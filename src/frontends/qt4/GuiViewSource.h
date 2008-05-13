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

#include "DockView.h"
#include "GuiView.h"
#include "qt_helpers.h"

#include "support/debug.h"

#include <QDockWidget>
#include <QString>
#include <QTextCharFormat>

class QTextDocument;

namespace lyx {
namespace frontend {

class GuiViewSource;
class LaTeXHighlighter;

class ViewSourceWidget : public QWidget, public Ui::ViewSourceUi
{
	Q_OBJECT

public:
	ViewSourceWidget(GuiViewSource &);

public Q_SLOTS:
	// update content
	void updateView();

private:
	///
	GuiViewSource & controller_;	
	///
	QTextDocument * document_;
	/// LaTeX syntax highlighter
	LaTeXHighlighter * highlighter_;
};


class GuiViewSource : public DockView
{
	Q_OBJECT

public:
	GuiViewSource(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area = Qt::BottomDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);

	~GuiViewSource();

	/// Controller inherited method.
	///@{
	bool initialiseParams(std::string const & source);
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return true; }
	bool canApply() const { return true; }
	bool canApplyToReadOnly() const { return true; }
	void updateView();
	void enableView(bool enable);
	///@}

	/// The title displayed by the dialog reflects source type.
	QString title() const;

	/** get the source code of selected paragraphs, or the whole document
		\param fullSource get full source code
	 */
	QString getContent(bool fullSource);
	// cursor position in the source code
	struct Row { int begin; int end; };
	Row getRows() const;

private:
	/// The encapsulated widget.
	ViewSourceWidget * widget_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIVIEWSOURCE_H
