// -*- C++ -*-
/**
 * \file GuiNote.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QNOTE_H
#define QNOTE_H

#include "GuiDialogView.h"
#include "ui_NoteUi.h"

#include <QDialog>
class QCloseEvent;

namespace lyx {
namespace frontend {

class GuiNote;

class GuiNoteDialog : public QDialog, public Ui::NoteUi {
	Q_OBJECT
public:
	GuiNoteDialog(GuiNote * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiNote * form_;
};


class ControlNote;

/** This class provides a QT implementation of the Note Dialog.
 */
class GuiNote : public QController<ControlNote, GuiView<GuiNoteDialog> >
{
public:
	friend class GuiNoteDialog;

	/// Constructor
	GuiNote(Dialog &);
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
