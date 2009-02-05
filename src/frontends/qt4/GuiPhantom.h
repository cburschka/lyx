// -*- C++ -*-
/**
 * \file GuiPhantom.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPHANTOM_H
#define GUIPHANTOM_H

#include "GuiDialog.h"
#include "insets/InsetPhantom.h"
#include "ui_PhantomUi.h"

namespace lyx {
namespace frontend {

class GuiPhantom : public GuiDialog, public Ui::PhantomUi
{
	Q_OBJECT
public:
	GuiPhantom(GuiView & lv);
private Q_SLOTS:
	void change_adaptor();
private:
	/// Apply changes
	void applyView();
	/// Update dialog before showing it
	void updateContents();
	///
	bool initialiseParams(std::string const & data);
	///
	void clearParams();
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
private:
	///
	InsetPhantomParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIPHANTOM_H
