#include "FormDocumentDialog.h"
#include "Dialogs.h"
#include "FormDocument.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qtoolbutton.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qspinbox.h>

/* 
 *  Constructs a FormDocumentDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FormDocumentDialog::FormDocumentDialog( FormDocument* _form, QWidget* parent,  const char* name, bool modal, WFlags fl )
  : FormDocumentDialogBase( parent, name, modal, fl ),
    form( _form )
{
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
}

/*  
 *  Destroys the object and frees any allocated resources
 */
FormDocumentDialog::~FormDocumentDialog()
{
  // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot
 */
void FormDocumentDialog::slotApply()
{
  form->apply();
  // PENDING(kalle) Check whether we need this
  //  form->bc_->apply();
}
/* 
 * public slot
 */
void FormDocumentDialog::slotAMSMath(bool)
{
  form->checkDocumentInput( amsMathCB );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletDepth1()
{
  form->bulletDepth( 0 );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletDepth2()
{
  form->bulletDepth( 1 );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletDepth3()
{
  form->bulletDepth( 2 );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletDepth4()
{
  form->bulletDepth( 3 );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletDing1()
{
  qDebug( "FormDocumentDialog::slotBulletDing1()" );
  form->checkDocumentInput( bulletDing1PB );
  form->setBulletPics();
  bulletStandardPB->setOn( false );
  bulletMathsPB->setOn( false );
  bulletDing2PB->setOn( false );
  bulletDing3PB->setOn( false );
  bulletDing4PB->setOn( false );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletDing2()
{
  form->checkDocumentInput( bulletDing2PB );
  form->setBulletPics();
  bulletStandardPB->setOn( false );
  bulletMathsPB->setOn( false );
  bulletDing1PB->setOn( false );
  bulletDing3PB->setOn( false );
  bulletDing4PB->setOn( false );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletDing3()
{
  form->checkDocumentInput( bulletDing3PB );
  form->setBulletPics();
  bulletStandardPB->setOn( false );
  bulletMathsPB->setOn( false );
  bulletDing2PB->setOn( false );
  bulletDing1PB->setOn( false );
  bulletDing4PB->setOn( false );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletDing4()
{
  form->checkDocumentInput( bulletDing4PB );
  form->setBulletPics();
  bulletStandardPB->setOn( false );
  bulletMathsPB->setOn( false );
  bulletDing2PB->setOn( false );
  bulletDing3PB->setOn( false );
  bulletDing1PB->setOn( false );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletMaths()
{
  form->checkDocumentInput( bulletMathsPB );
  form->setBulletPics();
  bulletStandardPB->setOn( false );
  bulletDing1PB->setOn( false );
  bulletDing2PB->setOn( false );
  bulletDing3PB->setOn( false );
  bulletDing4PB->setOn( false );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletSize(int)
{
  form->choiceBulletSize();
  form->checkDocumentInput( bulletSizeCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletStandard()
{
  form->checkDocumentInput( bulletStandardPB );
  form->setBulletPics();
  bulletDing1PB->setOn( false );
  bulletMathsPB->setOn( false );
  bulletDing2PB->setOn( false );
  bulletDing3PB->setOn( false );
  bulletDing4PB->setOn( false );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletSymbol(int n)
{
  qDebug( "FormDocumentDialog::slotBulletSymbol( %d )", n );
  for( int i = 0; i < 36; i++ )
    bulletbuttons[i]->setOn( false );
  bulletbuttons[n]->setOn( true );
  form->bulletBMTable( n );
  form->checkDocumentInput( bulletTypeBG );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotClass(int)
{
  form->checkChoiceClass( 0 );
  form->checkDocumentInput( classesCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotClose()
{
  form->cancel();
  form->hide();
  // PENDING(kalle) do something with this
  //  form->bc_->cancel();
}
/* 
 * public slot
 */
void FormDocumentDialog::slotColumns(int)
{
  form->checkDocumentInput( columnsBG );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotDefaultSkip(const QString&)
{
  form->checkDocumentInput( defaultSkipED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotDefaultSkip(int)
{
  form->checkChoiceClass( defaultSkipCO );
  form->checkDocumentInput( defaultSkipCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotEncoding(int)
{
  form->checkChoiceClass( encodingCO );
  form->checkDocumentInput( encodingCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotExtraOptions(const QString&)
{
  form->checkDocumentInput( extraOptionsED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotFloatPlacement(const QString&)
{
  form->checkDocumentInput( floatPlacementED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotFont(int)
{
  form->checkChoiceClass( fontsCO );
  form->checkDocumentInput( fontsCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotFontSize(int)
{
  form->checkChoiceClass( fontSizeCO );
  form->checkDocumentInput( fontSizeCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotFootskip(const QString&)
{
  form->checkDocumentInput( footskipED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotHeadheight(const QString&)
{
  form->checkDocumentInput( headheightED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotHeadsep(const QString&)
{
  form->checkDocumentInput( headsepED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotHeight(const QString&)
{
  form->checkDocumentInput( customHeightED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotLanguage(int)
{
  form->checkChoiceClass( languageCO );
  form->checkDocumentInput( languageCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotBulletLaTeX(const QString&)
{
  form->inputBulletLaTeX();
  form->checkDocumentInput( latexED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotMarginBottom(const QString&)
{
  form->checkDocumentInput( marginBottomED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotMarginLeft(const QString&)
{
  form->checkDocumentInput( marginLeftED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotMarginRight(const QString&)
{
  form->checkDocumentInput( marginRightED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotMarginTop(const QString&)
{
  form->checkDocumentInput( marginTopED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotOK()
{
  form->apply();
  form->hide();
  // PENDING(kalle) Do something about this.
  //  form->bc_->ok();
}
/* 
 * public slot
 */
void FormDocumentDialog::slotOrientation(int)
{
  form->checkDocumentInput( orientationBG );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotPSDriver(int)
{
  form->checkChoiceClass( psDriverCO );
  form->checkDocumentInput( psDriverCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotPageStyle(int)
{
  form->checkChoiceClass( pagestyleCO );
  form->checkDocumentInput( pagestyleCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotPapersize(int)
{
  form->checkChoiceClass( pagestyleCO );
  form->checkDocumentInput( papersizeCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotQuoteStyle(int)
{
  form->checkChoiceClass( quoteStyleTypeCO );
  form->checkDocumentInput( quoteStyleTypeCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotQuoteType(int)
{
  // Intentionally left blank
}
/* 
 * public slot
 */
void FormDocumentDialog::slotRestore()
{
  form->update();
  // PENDING(kalle) Do something about this.
  //  form->bc_->updateAll();
}
/* 
 * public slot
 */
void FormDocumentDialog::slotSectionNumberDepth(int)
{
  form->checkDocumentInput( sectionNumberDepthSB );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotSeparation(int)
{
  form->checkDocumentInput( separationBG );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotSides(int)
{
  form->checkDocumentInput( sidesBG );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotSpacing(const QString&)
{
  form->checkDocumentInput( spacingED );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotSpacing(int)
{
  form->checkDocumentInput( spacingCO );
  form->checkDocumentInput( spacingCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotSpecialPaperPackage(int)
{
  form->checkChoiceClass( specialCO );
  form->checkDocumentInput( specialCO );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotTOCDepth(int)
{
  form->checkDocumentInput( tocDepthSB );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotUseGeometryPackage(bool)
{
  form->checkDocumentInput( CheckBox1 );
}
/* 
 * public slot
 */
void FormDocumentDialog::slotWidth(const QString&)
{
  form->checkDocumentInput( customWidthED );
}


void FormDocumentDialog::closeEvent( QCloseEvent* e )
{
  form->hide();
  //PENDING(kalle) Do something about this.
  //  form->bc_->hide();
  e->accept();
}
