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

#ifndef GUINOTE_H
#define GUINOTE_H

#include "GuiDialogView.h"
#include "ControlNote.h"
#include "ui_NoteUi.h"

#include <QDialog>


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


/** This class provides a QT implementation of the Note Dialog.
 */
class GuiNote : public GuiView<GuiNoteDialog>
{
public:
	friend class GuiNoteDialog;

	/// Constructor
	GuiNote(GuiDialog &);
	/// parent controller
	ControlNote & controller()
	{ return static_cast<ControlNote &>(this->getController()); }
	/// parent controller
	ControlNote const & controller() const
	{ return static_cast<ControlNote const &>(this->getController()); }
private:
	///  Apply changes
	virtual void applyView();
	/// Build the dialog
	virtual void build_dialog();
	/// Update dialog before showing it
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUINOTE_H
