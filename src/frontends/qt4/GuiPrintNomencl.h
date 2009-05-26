// -*- C++ -*-
/**
 * \file GuiPrintNomencl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPRINTNOMENCL_H
#define GUIPRINTNOMENCL_H

#include "GuiDialog.h"
#include "ui_PrintNomenclUi.h"

#include "insets/InsetCommandParams.h"


namespace lyx {
namespace frontend {

class GuiPrintNomencl : public GuiDialog, public Ui::PrintNomenclUi
{
	Q_OBJECT

public:
	GuiPrintNomencl(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();
	void on_setWidthCO_activated(int);

private:
	/// Apply changes
	void applyView();
	/// Update dialog before showing it
	void updateContents();
	///
	bool initialiseParams(std::string const & data);
	///
	void paramsToDialog(InsetCommandParams const & icp);
	///
	void clearParams() { params_.clear(); }
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
	///
	bool isValid() const;

	///
	InsetCommandParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIPRINTNOMENCL_H
