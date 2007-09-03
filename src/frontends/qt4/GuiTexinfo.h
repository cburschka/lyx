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

#ifndef QTEXINFO_H
#define QTEXINFO_H

#include "GuiDialogView.h"
#include "ControlTexinfo.h"
#include "ui_TexinfoUi.h"

#include <QDialog>

#include <map>
#include <vector>

namespace lyx {
namespace frontend {

class GuiTexinfo;

class GuiTexinfoDialog : public QDialog, public Ui::TexinfoUi
{
	Q_OBJECT
public:
	GuiTexinfoDialog(GuiTexinfo * form);
public Q_SLOTS:
	virtual void update();
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void rescanClicked();
	virtual void viewClicked();
	virtual void enableViewPB();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiTexinfo * form_;
};


///
class GuiTexinfo : public GuiView<GuiTexinfoDialog>
{
public:
	///
	friend class GuiTexinfoDialog;
	///
	GuiTexinfo(GuiDialog &);
	/// parent controller
	ControlTexinfo & controller()
	{ return static_cast<ControlTexinfo &>(this->getController()); }
	/// parent controller
	ControlTexinfo const & controller() const
	{ return static_cast<ControlTexinfo const &>(this->getController()); }
private:
	/// Apply changes
	virtual void apply() {}
	/// update (do we need this?)
	virtual void update_contents() {}
	/// build the dialog
	virtual void build_dialog();
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

#endif // QTEXINFO_H
