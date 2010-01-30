// -*- C++ -*-
/**
 * \file GuiInfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_INFO_H
#define GUI_INFO_H

#include "InsetDialog.h"
#include "ui_InfoUi.h"

namespace lyx {

class Inset;

namespace frontend {

class GuiInfo : public InsetDialog, public Ui::InfoUi
{
	Q_OBJECT

public:
	GuiInfo(GuiView & lv);

	/// \name Dialog inherited methods
	//@{
	void enableView(bool enable);
	//@}

private Q_SLOTS:
	void on_newPB_clicked();
	void on_typeCO_currentIndexChanged(int);
	void on_nameLE_textChanged(QString const &);

private:
	/// \name InsetDialog inherited methods
	//@{
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUI_INFO_H
