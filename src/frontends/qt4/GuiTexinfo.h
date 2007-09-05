// -*- C++ -*-
/**
 * \file GuiTexinfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUITEXINFO_H
#define GUITEXINFO_H

#include "GuiDialog.h"
#include "ControlTexinfo.h"
#include "ui_TexinfoUi.h"

#include <map>
#include <vector>

namespace lyx {
namespace frontend {

class GuiTexinfoDialog : public GuiDialog, public Ui::TexinfoUi
{
	Q_OBJECT

public:
	GuiTexinfoDialog(LyXView & lv);

public Q_SLOTS:
	void updateView();

private Q_SLOTS:
	void change_adaptor();
	void rescanClicked();
	void viewClicked();
	void enableViewPB();

private:
	///
	void closeEvent(QCloseEvent * e);
	///
	ControlTexinfo & controller() const;
	///
	void updateStyles(ControlTexinfo::texFileSuffix);
	///
	void updateStyles();
	///
	bool warningPosted;
	///
	ControlTexinfo::texFileSuffix activeStyle;
	///
	typedef std::vector<std::string> ContentsType;
	std::map<ControlTexinfo::texFileSuffix, ContentsType> texdata_;
};

} // namespace frontend
} // namespace lyx

#endif // GUITEXINFO_H
