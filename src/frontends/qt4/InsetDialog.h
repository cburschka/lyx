// -*- C++ -*-
/**
 * \file InsetDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_DIALOG_H
#define INSET_DIALOG_H

#include "DialogView.h"

#include "qt_i18n.h"

namespace lyx {

class Inset;

namespace frontend {


class InsetDialog : public DialogView
{
	Q_OBJECT

public:
	InsetDialog(GuiView & lv, InsetCode code, FuncCode creation_code,
		char const * name, char const * display_name);

	/// \name DialogView inherited methods
	//@{
	void applyView();
	void updateView();
	void dispatchParams() {}
	bool isBufferDependent() const { return true; }
	bool canApply() const { return true; }
	//@}

protected Q_SLOTS:
	void on_newPB_clicked();
	void on_closePB_clicked();

protected:
	///
	virtual void paramsToDialog(Inset const *) = 0;
	///
	virtual docstring dialogToParams() const = 0;

private:
	///
	InsetCode inset_code_;
	///
	FuncCode creation_code_;
};

} // namespace frontend
} // namespace lyx

#endif // INSET_DIALOG_H
