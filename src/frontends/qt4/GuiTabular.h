// -*- C++ -*-
/**
 * \file GuiTabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author Jürgen Spitzmüller
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTABULAR_H
#define QTABULAR_H

#include "GuiDialogView.h"

#include "ui_TabularUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class GuiTabular;

class GuiTabularDialog : public QDialog, public Ui::TabularUi {
	Q_OBJECT
public:
	GuiTabularDialog(GuiTabular * form);

protected Q_SLOTS:
	virtual void change_adaptor();

	virtual void topspace_changed();
	virtual void bottomspace_changed();
	virtual void interlinespace_changed();
	virtual void booktabsChanged(bool);
	virtual void close_clicked();
	virtual void borderSet_clicked();
	virtual void borderUnset_clicked();
	virtual void leftBorder_changed();
	virtual void rightBorder_changed();
	virtual void topBorder_changed();
	virtual void bottomBorder_changed();
	virtual void multicolumn_clicked();
	virtual void rotateTabular();
	virtual void rotateCell();
	virtual void hAlign_changed(int align);
	virtual void vAlign_changed(int align);
	virtual void specialAlignment_changed();
	virtual void width_changed();
	virtual void longTabular();
	virtual void ltNewpage_clicked();
	virtual void ltHeaderStatus_clicked();
	virtual void ltHeaderBorderAbove_clicked();
	virtual void ltHeaderBorderBelow_clicked();
	virtual void ltFirstHeaderStatus_clicked();
	virtual void ltFirstHeaderBorderAbove_clicked();
	virtual void ltFirstHeaderBorderBelow_clicked();
	virtual void ltFirstHeaderEmpty_clicked();
	virtual void ltFooterStatus_clicked();
	virtual void ltFooterBorderAbove_clicked();
	virtual void ltFooterBorderBelow_clicked();
	virtual void ltLastFooterStatus_clicked();
	virtual void ltLastFooterBorderAbove_clicked();
	virtual void ltLastFooterBorderBelow_clicked();
	virtual void ltLastFooterEmpty_clicked();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	GuiTabular * form_;
};


class ControlTabular;

class GuiTabular :
	public QController<ControlTabular, GuiView<GuiTabularDialog> >
{
public:
	friend class GuiTabularDialog;

	GuiTabular(Dialog &);

protected:
	virtual bool isValid();

private:
	/// We can't use this ...
	virtual void apply() {}
	/// update borders
	virtual void update_borders();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
	/// save some values before closing the gui
	virtual void closeGUI();
};

} // namespace frontend
} // namespace lyx

#endif // QTABULAR_H
