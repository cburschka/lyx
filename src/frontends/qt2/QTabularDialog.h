// -*- C++ -*-
/**
 * \file QTabularDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Juergen Spitzmueller
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTABULARDIALOG_H
#define QTABULARDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QTabularDialogBase.h"

class QTabular;

class QTabularDialog : public QTabularDialogBase {
	Q_OBJECT
public:
	QTabularDialog(QTabular * form);

protected slots:
	virtual void change_adaptor();
	
	virtual void close_clicked();
	virtual void columnAppend_clicked();
	virtual void rowAppend_clicked();
	virtual void columnDelete_clicked(); 
	virtual void rowDelete_clicked();
	virtual void borderSet_clicked();
	virtual void borderUnset_clicked();
	virtual void leftBorder_changed();
	virtual void rightBorder_changed();
	virtual void topBorder_changed();
	virtual void bottomBorder_changed();
	virtual void multicolumn_clicked();
	virtual void rotateTabular_checked(int state);
	virtual void rotateCell_checked(int state);
	virtual void hAlign_changed(int align);
	virtual void vAlign_changed(int align);
	virtual void specialAlignment_changed();
	virtual void width_changed();
	virtual void longTabular_changed(int state);
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
	QTabular * form_;
};

#endif // QTABULARDIALOG_H
