/**
 * \file QDocumentDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QDocumentDialog.h"
#include "Dialogs.h"
#include "QDocument.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qtoolbutton.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qspinbox.h>

QDocumentDialog::QDocumentDialog(QDocument * form, QWidget * parent, const char * name, bool modal, WFlags fl)
	: QDocumentDialogBase(parent, name, modal, fl),
		form_(form)
{
#if 0
	// Copy the pointers to the bullet buttons into an array so that
	// they can all be manipulated together.
	bulletbuttons[0] = bullet00PB;
	bulletbuttons[1] = bullet01PB;
	bulletbuttons[2] = bullet02PB;
	bulletbuttons[3] = bullet03PB;
	bulletbuttons[4] = bullet04PB;
	bulletbuttons[5] = bullet05PB;
	bulletbuttons[6] = bullet10PB;
	bulletbuttons[7] = bullet11PB;
	bulletbuttons[8] = bullet12PB;
	bulletbuttons[9] = bullet13PB;
	bulletbuttons[10] = bullet14PB;
	bulletbuttons[11] = bullet15PB;
	bulletbuttons[12] = bullet20PB;
	bulletbuttons[13] = bullet21PB;
	bulletbuttons[14] = bullet22PB;
	bulletbuttons[15] = bullet23PB;
	bulletbuttons[16] = bullet24PB;
	bulletbuttons[17] = bullet25PB;
	bulletbuttons[18] = bullet30PB;
	bulletbuttons[19] = bullet31PB;
	bulletbuttons[20] = bullet32PB;
	bulletbuttons[21] = bullet33PB;
	bulletbuttons[22] = bullet34PB;
	bulletbuttons[23] = bullet35PB;
	bulletbuttons[24] = bullet40PB;
	bulletbuttons[25] = bullet41PB;
	bulletbuttons[26] = bullet42PB;
	bulletbuttons[27] = bullet43PB;
	bulletbuttons[28] = bullet44PB;
	bulletbuttons[29] = bullet45PB;
	bulletbuttons[30] = bullet50PB;
	bulletbuttons[31] = bullet51PB;
	bulletbuttons[32] = bullet52PB;
	bulletbuttons[33] = bullet53PB;
	bulletbuttons[34] = bullet54PB;
	bulletbuttons[35] = bullet55PB;
#endif
}


QDocumentDialog::~QDocumentDialog()
{
}


#if 0
void QDocumentDialog::slotApply()
{
	form->apply();
	// PENDING(kalle) Check whether we need this
	//	form->bc_->apply();
}


void QDocumentDialog::slotAMSMath(bool)
{
	form->checkDocumentInput( amsMathCB );
}


void QDocumentDialog::slotBulletDepth1()
{
	form->bulletDepth( 0 );
}


void QDocumentDialog::slotBulletDepth2()
{
	form->bulletDepth( 1 );
}


void QDocumentDialog::slotBulletDepth3()
{
	form->bulletDepth( 2 );
}


void QDocumentDialog::slotBulletDepth4()
{
	form->bulletDepth( 3 );
}


void QDocumentDialog::slotBulletDing1()
{
	qDebug( "QDocumentDialog::slotBulletDing1()" );
	form->checkDocumentInput( bulletDing1PB );
	form->setBulletPics();
	bulletStandardPB->setOn( false );
	bulletMathsPB->setOn( false );
	bulletDing2PB->setOn( false );
	bulletDing3PB->setOn( false );
	bulletDing4PB->setOn( false );
}


void QDocumentDialog::slotBulletDing2()
{
	form->checkDocumentInput( bulletDing2PB );
	form->setBulletPics();
	bulletStandardPB->setOn( false );
	bulletMathsPB->setOn( false );
	bulletDing1PB->setOn( false );
	bulletDing3PB->setOn( false );
	bulletDing4PB->setOn( false );
}


void QDocumentDialog::slotBulletDing3()
{
	form->checkDocumentInput( bulletDing3PB );
	form->setBulletPics();
	bulletStandardPB->setOn( false );
	bulletMathsPB->setOn( false );
	bulletDing2PB->setOn( false );
	bulletDing1PB->setOn( false );
	bulletDing4PB->setOn( false );
}


void QDocumentDialog::slotBulletDing4()
{
	form->checkDocumentInput( bulletDing4PB );
	form->setBulletPics();
	bulletStandardPB->setOn( false );
	bulletMathsPB->setOn( false );
	bulletDing2PB->setOn( false );
	bulletDing3PB->setOn( false );
	bulletDing1PB->setOn( false );
}


void QDocumentDialog::slotBulletMaths()
{
	form->checkDocumentInput( bulletMathsPB );
	form->setBulletPics();
	bulletStandardPB->setOn( false );
	bulletDing1PB->setOn( false );
	bulletDing2PB->setOn( false );
	bulletDing3PB->setOn( false );
	bulletDing4PB->setOn( false );
}


void QDocumentDialog::slotBulletSize(int)
{
	form->choiceBulletSize();
	form->checkDocumentInput( bulletSizeCO );
}


void QDocumentDialog::slotBulletStandard()
{
	form->checkDocumentInput( bulletStandardPB );
	form->setBulletPics();
	bulletDing1PB->setOn( false );
	bulletMathsPB->setOn( false );
	bulletDing2PB->setOn( false );
	bulletDing3PB->setOn( false );
	bulletDing4PB->setOn( false );
}


void QDocumentDialog::slotBulletSymbol(int n)
{
	qDebug( "QDocumentDialog::slotBulletSymbol( %d )", n );
	for( int i = 0; i < 36; i++ )
		bulletbuttons[i]->setOn( false );
	bulletbuttons[n]->setOn( true );
	form->bulletBMTable( n );
	form->checkDocumentInput( bulletTypeBG );
}


void QDocumentDialog::slotClass(int)
{
	form->checkChoiceClass( 0 );
	form->checkDocumentInput( classesCO );
}


void QDocumentDialog::slotClose()
{
	form->cancel();
	form->hide();
	// PENDING(kalle) do something with this
	//	form->bc_->cancel();
}


void QDocumentDialog::slotColumns(int)
{
	form->checkDocumentInput( columnsBG );
}


void QDocumentDialog::slotDefaultSkip(const QString&)
{
	form->checkDocumentInput( defaultSkipED );
}


void QDocumentDialog::slotDefaultSkip(int)
{
	form->checkChoiceClass( defaultSkipCO );
	form->checkDocumentInput( defaultSkipCO );
}


void QDocumentDialog::slotEncoding(int)
{
	form->checkChoiceClass( encodingCO );
	form->checkDocumentInput( encodingCO );
}


void QDocumentDialog::slotExtraOptions(const QString&)
{
	form->checkDocumentInput( extraOptionsED );
}


void QDocumentDialog::slotFloatPlacement(const QString&)
{
	form->checkDocumentInput( floatPlacementED );
}


void QDocumentDialog::slotFont(int)
{
	form->checkChoiceClass( fontsCO );
	form->checkDocumentInput( fontsCO );
}


void QDocumentDialog::slotFontSize(int)
{
	form->checkChoiceClass( fontSizeCO );
	form->checkDocumentInput( fontSizeCO );
}


void QDocumentDialog::slotFootskip(const QString&)
{
	form->checkDocumentInput( footskipED );
}


void QDocumentDialog::slotHeadheight(const QString&)
{
	form->checkDocumentInput( headheightED );
}


void QDocumentDialog::slotHeadsep(const QString&)
{
	form->checkDocumentInput( headsepED );
}


void QDocumentDialog::slotHeight(const QString&)
{
	form->checkDocumentInput( customHeightED );
}


void QDocumentDialog::slotLanguage(int)
{
	form->checkChoiceClass( languageCO );
	form->checkDocumentInput( languageCO );
}


void QDocumentDialog::slotBulletLaTeX(const QString&)
{
	form->inputBulletLaTeX();
	form->checkDocumentInput( latexED );
}


void QDocumentDialog::slotMarginBottom(const QString&)
{
	form->checkDocumentInput( marginBottomED );
}


void QDocumentDialog::slotMarginLeft(const QString&)
{
	form->checkDocumentInput( marginLeftED );
}


void QDocumentDialog::slotMarginRight(const QString&)
{
	form->checkDocumentInput( marginRightED );
}


void QDocumentDialog::slotMarginTop(const QString&)
{
	form->checkDocumentInput( marginTopED );
}


void QDocumentDialog::slotOK()
{
	form->apply();
	form->hide();
	// PENDING(kalle) Do something about this.
	//	form->bc_->ok();
}


void QDocumentDialog::slotOrientation(int)
{
	form->checkDocumentInput( orientationBG );
}


void QDocumentDialog::slotPSDriver(int)
{
	form->checkChoiceClass( psDriverCO );
	form->checkDocumentInput( psDriverCO );
}


void QDocumentDialog::slotPageStyle(int)
{
	form->checkChoiceClass( pagestyleCO );
	form->checkDocumentInput( pagestyleCO );
}


void QDocumentDialog::slotPapersize(int)
{
	form->checkChoiceClass( pagestyleCO );
	form->checkDocumentInput( papersizeCO );
}


void QDocumentDialog::slotQuoteStyle(int)
{
	form->checkChoiceClass( quoteStyleTypeCO );
	form->checkDocumentInput( quoteStyleTypeCO );
}


void QDocumentDialog::slotQuoteType(int)
{
	// Intentionally left blank
}


void QDocumentDialog::slotRestore()
{
	form->update();
	// PENDING(kalle) Do something about this.
	//	form->bc_->updateAll();
}


void QDocumentDialog::slotSectionNumberDepth(int)
{
	form->checkDocumentInput( sectionNumberDepthSB );
}


void QDocumentDialog::slotSeparation(int)
{
	form->checkDocumentInput( separationBG );
}


void QDocumentDialog::slotSides(int)
{
	form->checkDocumentInput( sidesBG );
}


void QDocumentDialog::slotSpacing(const QString&)
{
	form->checkDocumentInput( spacingED );
}


void QDocumentDialog::slotSpacing(int)
{
	form->checkDocumentInput( spacingCO );
	form->checkDocumentInput( spacingCO );
}


void QDocumentDialog::slotSpecialPaperPackage(int)
{
	form->checkChoiceClass( specialCO );
	form->checkDocumentInput( specialCO );
}


void QDocumentDialog::slotTOCDepth(int)
{
	form->checkDocumentInput( tocDepthSB );
}


void QDocumentDialog::slotUseGeometryPackage(bool)
{
	form->checkDocumentInput( CheckBox1 );
}


void QDocumentDialog::slotWidth(const QString&)
{
	form->checkDocumentInput( customWidthED );
}
#endif


void QDocumentDialog::closeEvent( QCloseEvent* e )
{
	//form_->hide();
	//PENDING(kalle) Do something about this.
	//	form->bc_->hide();
	e->accept();
}
