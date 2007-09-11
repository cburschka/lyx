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

#include "GuiDialog.h"
#include "ControlNote.h"
#include "ui_NoteUi.h"

namespace lyx {
namespace frontend {

class GuiNoteDialog : public GuiDialog, public Ui::NoteUi
{
	Q_OBJECT
public:
	GuiNoteDialog(LyXView & lv);
private Q_SLOTS:
	void change_adaptor();
private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlNote & controller();
	/// Apply changes
	void applyView();
	/// Update dialog before showing it
	void updateContents();
};

} // namespace frontend
} // namespace lyx

#endif // GUINOTE_H
