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

#include <QDockWidget>
#include <QString>
#include <QTimer>


class QTextDocument;


namespace lyx {

class TexRow;


namespace frontend {

class GuiViewSource;
class LaTeXHighlighter;


class ViewSourceWidget : public QWidget, public Ui::ViewSourceUi
{
	Q_OBJECT

public:
	ViewSourceWidget(QWidget * parent);
	/// returns true if the string has changed
	bool setText(QString const & qstr = QString());
	///
	void saveSession(QSettings & settings, QString const & session_key) const;
	///
	void restoreSession(QString const & session_key);
	///
	int updateDelay() const;

protected:
	///
	void resizeEvent (QResizeEvent * event);

public Q_SLOTS:
	///
	void updateView(BufferView const * bv);
	///
	void setViewFormat(int const index);
	//
	void updateDefaultFormat(BufferView const & bv);
	///
	void contentsChanged();
	///
	void goToCursor() const;
	/// Name of the current format. Empty if none.
	docstring currentFormatName(BufferView const * bv) const;

Q_SIGNALS:
	void needUpdate() const;

private:
	/// Get the source code of selected paragraphs, or the whole document.
	void getContent(BufferView const & view, Buffer::OutputWhat output,
			   docstring & str, std::string const & format, bool master);
	/// Grab double clicks on the viewport
	bool eventFilter(QObject * obj, QEvent * event) override;
	///
	QTextDocument * document_;
	/// LaTeX syntax highlighter
	LaTeXHighlighter * highlighter_;
	///
	std::string view_format_;
	/// TexRow information from the last source view. If TexRow is unavailable
	/// for the last format then texrow_ is null.
	unique_ptr<TexRow> texrow_;
};


class GuiViewSource : public DockView
{
	Q_OBJECT

public:
	GuiViewSource(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area = Qt::BottomDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);

	/// Controller inherited method.
	///@{
	bool initialiseParams(std::string const & source) override;
	void clearParams() override {}
	void dispatchParams() override {}
	bool isBufferDependent() const override { return true; }
	bool canApply() const override { return true; }
	bool canApplyToReadOnly() const override { return true; }
	void updateView() override;
	void enableView(bool enable) override;
	void saveSession(QSettings & settings) const override;
	void restoreSession() override;
	bool wantInitialFocus() const override { return false; }
	///@}

public Q_SLOTS:
	///
	void onBufferViewChanged() override;

private Q_SLOTS:
	/// The title displayed by the dialog reflects source type.
	void updateTitle();
	/// schedule an update after delay
	void scheduleUpdate();
	/// schedule an update now
	void scheduleUpdateNow();

	/// update content
	void realUpdateView();

private:
	/// The encapsulated widget.
	ViewSourceWidget * widget_;
	///
	QTimer * update_timer_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIVIEWSOURCE_H
