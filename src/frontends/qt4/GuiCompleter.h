// -*- C++ -*-
/**
 * \file GuiCompleter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICOMPLETER_H
#define GUICOMPLETER_H

#include "frontends/WorkArea.h"

#include "DocIterator.h"
#include "FuncRequest.h"
#include "qt_helpers.h"
#include "support/docstring.h"

#include <QAbstractItemModel>
#include <QCompleter>
#include <QStringListModel>
#include <QTimer>

namespace lyx {

class Buffer;

namespace frontend {

class GuiWorkArea;
class RtlItemDelegate;

class GuiCompleter : private QCompleter
{
	Q_OBJECT

public:
	///
	GuiCompleter(GuiWorkArea * gui, QObject * parent = 0);
	///
	virtual ~GuiCompleter();

	///
	bool popupVisible() const;
	///
	bool inlineVisible() const;
	///
	bool popupPossible(Cursor const & cur) const;
	///
	bool inlinePossible(Cursor const & cur) const;
	/// Activate the current completion, i.e. finalize it.
	void activate();
	/// Do a completion as far as it is unique, but at least one character.
	void tab();
	
	/// Update the visibility of the popup and the inline completion.
	/// This method might set the update flags of the cursor to request
	/// a redraw.
	void updateVisibility(Cursor & cur, bool start, bool keep, bool cursorInView = true);
	/// Update the visibility of the popup and the inline completion.
	/// This method handles the redraw if needed.
	void updateVisibility(bool start, bool keep);
	///
	QString currentCompletion() const;
	///
	docstring longestUniqueCompletion() const;
	
public Q_SLOTS:
	/// Show the popup.
	void showPopup();
	/// Show the inline completion.
	void showInline();

private Q_SLOTS:
	///
	void popupActivated(const QString & completion);
	///
	void popupHighlighted(const QString & completion);
	
private:
	///
	void setCurrentCompletion(QString const & s);
	///
	void showPopup(Cursor & cur);
	///
	void showInline(Cursor & cur);
	///
	void updatePopup(Cursor & cur);
	///
	void updateInline(Cursor & cur, QString const & completion);
	///
	void updatePrefix(Cursor & cur);
	///
	void updateModel(Cursor & cur, bool popupUpdate, bool inlineUpdate);
	///
	bool eventFilter(QObject * watched, QEvent * event);

	///
	GuiWorkArea * gui_;
	///
	DocIterator old_cursor_;
	///
	QTimer popup_timer_;
	///
	QTimer inline_timer_;
	///
	QString last_selection_;
	/// lock to stop updates of the inline completion
	int updateLock_;
	/// the BufferView::inlineCursorPos might be reset by destructive
	/// operations like backspace. Hence, we have to keep this flag
	/// in addition to know whether the completion is to be kept visible.
	bool inlineVisible_;
	///
	RtlItemDelegate * rtlItemDelegate_;
}; // GuiCompleter

} // namespace frontend
} // namespace lyx

#endif // GUICOMPLETER_H
