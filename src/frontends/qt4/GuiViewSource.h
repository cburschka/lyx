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

#include "Buffer.h"
#include "DockView.h"
#include "TexRow.h"

#include <QDockWidget>
#include <QString>
#include <QTimer>

class QTextDocument;

namespace lyx {
namespace frontend {

class GuiViewSource;
class LaTeXHighlighter;

class ViewSourceWidget : public QWidget, public Ui::ViewSourceUi
{
	Q_OBJECT

public:
	ViewSourceWidget();
	///
	void setBufferView(BufferView const * bv);
	/// returns true if the string has changed
	bool setText(QString const & qstr = QString());
	///
	void saveSession(QString const & session_key) const;
	///
	void restoreSession(QString const & session_key);

protected:
	///
	void resizeEvent (QResizeEvent * event);

public Q_SLOTS:
	/// schedule an update after delay
	void updateView();
	/// schedule an update now
	void updateViewNow();
	///
	void setViewFormat(int const index);
	//
	void updateDefaultFormat();
	///
	void contentsChanged();
	///
	void gotoCursor();
	/// Name of the current format. Empty if none.
	docstring currentFormatName() const;

Q_SIGNALS:
	void formatChanged() const;

private Q_SLOTS:
	/// update content
	void realUpdateView();

private:
	/// Get the source code of selected paragraphs, or the whole document.
	void getContent(BufferView const * view, Buffer::OutputWhat output,
			   docstring & str, std::string const & format, bool master);
	///
	BufferView const * bv_;
	///
	QTextDocument * document_;
	/// LaTeX syntax highlighter
	LaTeXHighlighter * highlighter_;
	///
	std::string view_format_;
	///
	QTimer * update_timer_;
	/// TexRow information from the last source view. If TexRow is unavailable
	/// for the last format then texrow_ is null.
	std::auto_ptr<TexRow> texrow_;
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
	void saveSession() const;
	void restoreSession();
	bool wantInitialFocus() const { return false; }
	///@}

private Q_SLOTS:
	/// The title displayed by the dialog reflects source type.
	void updateTitle();

private:
	/// The encapsulated widget.
	ViewSourceWidget * widget_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIVIEWSOURCE_H
