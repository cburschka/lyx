// -*- C++ -*-
/**
 * \file QNote.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QNOTE_H
#define QNOTE_H

#include "QDialogView.h"
#include "ui_NoteUi.h"

#include <QDialog>
class QCloseEvent;

namespace lyx {
namespace frontend {

class QNote;

class QNoteDialog : public QDialog, public Ui::QNoteUi {
	Q_OBJECT
public:
	QNoteDialog(QNote * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QNote * form_;
};


class ControlNote;

/** This class provides a QT implementation of the Note Dialog.
 */
class QNote : public QController<ControlNote, QView<QNoteDialog> >
{
public:
	friend class QNoteDialog;

	/// Constructor
	QNote(Dialog &);
private:
	///  Apply changes
	virtual void apply();
	/// Build the dialog
	virtual void build_dialog();
	/// Update dialog before showing it
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // QNOTE_H
