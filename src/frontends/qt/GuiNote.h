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
#include "insets/InsetNote.h"
#include "ui_NoteUi.h"

namespace lyx {
namespace frontend {

class GuiNote : public GuiDialog, public Ui::NoteUi
{
	Q_OBJECT
public:
	GuiNote(GuiView & lv);
private Q_SLOTS:
	void change_adaptor();
private:
	/// Apply changes
	void applyView() override;
	/// Update dialog before showing it
	void updateContents() override;
	///
	bool initialiseParams(std::string const & data) override;
	///
	void clearParams() override;
	///
	void dispatchParams() override;
	///
	bool isBufferDependent() const override { return true; }
private:
	///
	InsetNoteParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUINOTE_H
