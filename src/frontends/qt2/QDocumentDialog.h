/**
 * \file QDocumentDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef QDOCUMENTDIALOG_H
#define QDOCUMENTDIALOG_H
 
#include "ui/QDocumentDialogBase.h"

class QDocument;

class QDocumentDialog : public QDocumentDialogBase
{ Q_OBJECT

public:
	QDocumentDialog(QDocument * form, QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags fl = 0);
	~QDocumentDialog();

public slots:
#if 0 
	void slotApply();
	void slotBulletLaTeX(const QString&);
	void slotAMSMath(bool);
	void slotBulletDepth1();
	void slotBulletDepth2();
	void slotBulletDepth3();
	void slotBulletDepth4();
	void slotBulletDing1();
	void slotBulletDing2();
	void slotBulletDing3();
	void slotBulletDing4();
	void slotBulletMaths();
	void slotBulletSize(int);
	void slotBulletStandard();
	void slotBulletSymbol(int);
	void slotClass(int);
	void slotClose();
	void slotColumns(int);
	void slotDefaultSkip(const QString&);
	void slotDefaultSkip(int);
	void slotEncoding(int);
	void slotExtraOptions(const QString&);
	void slotFloatPlacement(const QString&);
	void slotFont(int);
	void slotFontSize(int);
	void slotFootskip(const QString&);
	void slotHeadheight(const QString&);
	void slotHeadsep(const QString&);
	void slotHeight(const QString&);
	void slotLanguage(int);
	void slotMarginBottom(const QString&);
	void slotMarginLeft(const QString&);
	void slotMarginRight(const QString&);
	void slotMarginTop(const QString&);
	void slotOK();
	void slotOrientation(int);
	void slotPSDriver(int);
	void slotPageStyle(int);
	void slotPapersize(int);
	void slotQuoteStyle(int);
	void slotQuoteType(int);
	void slotRestore();
	void slotSectionNumberDepth(int);
	void slotSeparation(int);
	void slotSides(int);
	void slotSpacing(const QString&);
	void slotSpacing(int);
	void slotSpecialPaperPackage(int);
	void slotTOCDepth(int);
	void slotUseGeometryPackage(bool);
	void slotWidth(const QString&);
#endif 

protected:
	void closeEvent(QCloseEvent *);

private:
	QDocument * form_;
	QToolButton* bulletbuttons[36];
};

#endif // QDOCUMENTDIALOG_H
