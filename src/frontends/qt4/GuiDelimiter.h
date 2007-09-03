// -*- C++ -*-
/**
 * \file GuiDelimiterDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIDELIMITERDIALOG_H
#define GUIDELIMITERDIALOG_H

#include "GuiDialogView.h"
#include "ControlMath.h"
#include "ui_DelimiterUi.h"

class QListWidgetItem;

namespace lyx {
namespace frontend {

class ControlMath;
class GuiDelimiter;

class GuiDelimiterDialog : public QDialog, public Ui::DelimiterUi
{
	Q_OBJECT
public:
	GuiDelimiterDialog(GuiDelimiter * form, QWidget * parent);
public Q_SLOTS:
	void on_leftLW_itemActivated(QListWidgetItem *);
	void on_rightLW_itemActivated(QListWidgetItem *);
	void on_leftLW_currentRowChanged(int);
	void on_rightLW_currentRowChanged(int);
	void on_matchCB_stateChanged(int);
	void on_insertPB_clicked();
	void on_sizeCO_activated(int);
private:
	///
	char_type doMatch(char_type const symbol) const;
	///
	void updateTeXCode(int size);
	/// owning form
	GuiDelimiter * form_;
	/// TeX code that will be inserted.
	QString tex_code_;
};


class GuiDelimiter : public GuiView<GuiDelimiterDialog>
{
public:
	friend class GuiDelimiterDialog;
	///
	GuiDelimiter(GuiDialog &);
	/// parent controller
	ControlMath & controller()
	{ return static_cast<ControlMath &>(this->getController()); }
	/// parent controller
	ControlMath const & controller() const
	{ return static_cast<ControlMath const &>(this->getController()); }
private:
	virtual void apply() {}
	virtual void update_contents() {}
	/// Build the dialog.
	virtual void build_dialog();
};


} // namespace frontend
} // namespace lyx

#endif // GUIDELIMITERDIALOG_H
