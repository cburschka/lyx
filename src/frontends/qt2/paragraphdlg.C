/****************************************************************************
** Form implementation generated from reading ui file 'paragraphdlg.ui'
**
** Created: Sat Feb 10 14:22:28 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "paragraphdlg.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a ParagraphDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ParagraphDlg::ParagraphDlg( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "ParagraphDlg" );
    resize( 559, 387 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, sizePolicy().hasHeightForWidth() ) );
    setCaption( tr( "Paragraph" ) );
    setSizeGripEnabled( TRUE );
    ParagraphDlgLayout = new QVBoxLayout( this ); 
    ParagraphDlgLayout->setSpacing( 6 );
    ParagraphDlgLayout->setMargin( 11 );

    TabsParagraph = new QTabWidget( this, "TabsParagraph" );

    tab = new QWidget( TabsParagraph, "tab" );
    tabLayout = new QVBoxLayout( tab ); 
    tabLayout->setSpacing( 6 );
    tabLayout->setMargin( 11 );

    Layout6 = new QHBoxLayout; 
    Layout6->setSpacing( 6 );
    Layout6->setMargin( 0 );

    alignmentL = new QLabel( tab, "alignmentL" );
    alignmentL->setText( tr( "Alignment:" ) );
    alignmentL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, alignmentL->sizePolicy().hasHeightForWidth() ) );
    Layout6->addWidget( alignmentL );

    alignment = new QComboBox( FALSE, tab, "alignment" );
    alignment->insertItem( tr( "Justified" ) );
    alignment->insertItem( tr( "Left" ) );
    alignment->insertItem( tr( "Right" ) );
    alignment->insertItem( tr( "Centered" ) );
    alignment->setInsertionPolicy( QComboBox::AtBottom );
    alignment->setAutoCompletion( FALSE );
    Layout6->addWidget( alignment );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout6->addItem( spacer );

    noIndent = new QCheckBox( tab, "noIndent" );
    noIndent->setText( tr( "No indentation" ) );
    Layout6->addWidget( noIndent );
    tabLayout->addLayout( Layout6 );

    spacingOptions = new QGroupBox( tab, "spacingOptions" );
    spacingOptions->setTitle( tr( "Spacing" ) );
    spacingOptions->setEnabled( TRUE );
    spacingOptions->setColumnLayout(0, Qt::Vertical );
    spacingOptions->layout()->setSpacing( 0 );
    spacingOptions->layout()->setMargin( 0 );
    spacingOptionsLayout = new QGridLayout( spacingOptions->layout() );
    spacingOptionsLayout->setAlignment( Qt::AlignTop );
    spacingOptionsLayout->setSpacing( 6 );
    spacingOptionsLayout->setMargin( 11 );

    spacingBelowValueUnit = new QComboBox( FALSE, spacingOptions, "spacingBelowValueUnit" );
    spacingBelowValueUnit->insertItem( tr( "Centimetres" ) );
    spacingBelowValueUnit->insertItem( tr( "Inches" ) );
    spacingBelowValueUnit->insertItem( tr( "Points" ) );
    spacingBelowValueUnit->insertItem( tr( "Millimetres" ) );
    spacingBelowValueUnit->insertItem( tr( "Picas" ) );
    spacingBelowValueUnit->insertItem( tr( "ex Units" ) );
    spacingBelowValueUnit->insertItem( tr( "em Units" ) );
    spacingBelowValueUnit->insertItem( tr( "Scaled Points" ) );
    spacingBelowValueUnit->insertItem( tr( "Big/PS Points" ) );
    spacingBelowValueUnit->insertItem( tr( "Didot Points" ) );
    spacingBelowValueUnit->insertItem( tr( "Cicero Points" ) );
    spacingBelowValueUnit->setEnabled( FALSE );
    spacingBelowValueUnit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, spacingBelowValueUnit->sizePolicy().hasHeightForWidth() ) );

    spacingOptionsLayout->addWidget( spacingBelowValueUnit, 4, 2 );

    spacingBelowPlusUnit = new QComboBox( FALSE, spacingOptions, "spacingBelowPlusUnit" );
    spacingBelowPlusUnit->insertItem( tr( "Centimetres" ) );
    spacingBelowPlusUnit->insertItem( tr( "Inches" ) );
    spacingBelowPlusUnit->insertItem( tr( "Points" ) );
    spacingBelowPlusUnit->insertItem( tr( "Millimetres" ) );
    spacingBelowPlusUnit->insertItem( tr( "Picas" ) );
    spacingBelowPlusUnit->insertItem( tr( "ex Units" ) );
    spacingBelowPlusUnit->insertItem( tr( "em Units" ) );
    spacingBelowPlusUnit->insertItem( tr( "Scaled Points" ) );
    spacingBelowPlusUnit->insertItem( tr( "Big/PS Points" ) );
    spacingBelowPlusUnit->insertItem( tr( "Didot Points" ) );
    spacingBelowPlusUnit->insertItem( tr( "Cicero Points" ) );
    spacingBelowPlusUnit->setEnabled( FALSE );
    spacingBelowPlusUnit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, spacingBelowPlusUnit->sizePolicy().hasHeightForWidth() ) );

    spacingOptionsLayout->addWidget( spacingBelowPlusUnit, 4, 3 );

    spacingBelowMinusUnit = new QComboBox( FALSE, spacingOptions, "spacingBelowMinusUnit" );
    spacingBelowMinusUnit->insertItem( tr( "Centimetres" ) );
    spacingBelowMinusUnit->insertItem( tr( "Inches" ) );
    spacingBelowMinusUnit->insertItem( tr( "Points" ) );
    spacingBelowMinusUnit->insertItem( tr( "Millimetres" ) );
    spacingBelowMinusUnit->insertItem( tr( "Picas" ) );
    spacingBelowMinusUnit->insertItem( tr( "ex Units" ) );
    spacingBelowMinusUnit->insertItem( tr( "em Units" ) );
    spacingBelowMinusUnit->insertItem( tr( "Scaled Points" ) );
    spacingBelowMinusUnit->insertItem( tr( "Big/PS Points" ) );
    spacingBelowMinusUnit->insertItem( tr( "Didot Points" ) );
    spacingBelowMinusUnit->insertItem( tr( "Cicero Points" ) );
    spacingBelowMinusUnit->setEnabled( FALSE );
    spacingBelowMinusUnit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, spacingBelowMinusUnit->sizePolicy().hasHeightForWidth() ) );

    spacingOptionsLayout->addWidget( spacingBelowMinusUnit, 4, 4 );

    spacingAbovePlusUnit = new QComboBox( FALSE, spacingOptions, "spacingAbovePlusUnit" );
    spacingAbovePlusUnit->insertItem( tr( "Centimetres" ) );
    spacingAbovePlusUnit->insertItem( tr( "Inches" ) );
    spacingAbovePlusUnit->insertItem( tr( "Points" ) );
    spacingAbovePlusUnit->insertItem( tr( "Millimetres" ) );
    spacingAbovePlusUnit->insertItem( tr( "Picas" ) );
    spacingAbovePlusUnit->insertItem( tr( "ex Units" ) );
    spacingAbovePlusUnit->insertItem( tr( "em Units" ) );
    spacingAbovePlusUnit->insertItem( tr( "Scaled Points" ) );
    spacingAbovePlusUnit->insertItem( tr( "Big/PS Points" ) );
    spacingAbovePlusUnit->insertItem( tr( "Didot Points" ) );
    spacingAbovePlusUnit->insertItem( tr( "Cicero Points" ) );
    spacingAbovePlusUnit->setEnabled( FALSE );
    spacingAbovePlusUnit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, spacingAbovePlusUnit->sizePolicy().hasHeightForWidth() ) );

    spacingOptionsLayout->addWidget( spacingAbovePlusUnit, 2, 3 );

    spacingAboveValueUnit = new QComboBox( FALSE, spacingOptions, "spacingAboveValueUnit" );
    spacingAboveValueUnit->insertItem( tr( "Centimetres" ) );
    spacingAboveValueUnit->insertItem( tr( "Inches" ) );
    spacingAboveValueUnit->insertItem( tr( "Points" ) );
    spacingAboveValueUnit->insertItem( tr( "Millimetres" ) );
    spacingAboveValueUnit->insertItem( tr( "Picas" ) );
    spacingAboveValueUnit->insertItem( tr( "ex Units" ) );
    spacingAboveValueUnit->insertItem( tr( "em Units" ) );
    spacingAboveValueUnit->insertItem( tr( "Scaled Points" ) );
    spacingAboveValueUnit->insertItem( tr( "Big/PS Points" ) );
    spacingAboveValueUnit->insertItem( tr( "Didot Points" ) );
    spacingAboveValueUnit->insertItem( tr( "Cicero Points" ) );
    spacingAboveValueUnit->setEnabled( FALSE );
    spacingAboveValueUnit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, spacingAboveValueUnit->sizePolicy().hasHeightForWidth() ) );

    spacingOptionsLayout->addWidget( spacingAboveValueUnit, 2, 2 );

    spacingBelowValue = new QLineEdit( spacingOptions, "spacingBelowValue" );
    spacingBelowValue->setEnabled( FALSE );

    spacingOptionsLayout->addWidget( spacingBelowValue, 3, 2 );

    spacingBelowPlus = new QLineEdit( spacingOptions, "spacingBelowPlus" );
    spacingBelowPlus->setEnabled( FALSE );

    spacingOptionsLayout->addWidget( spacingBelowPlus, 3, 3 );

    spacingAboveMinusUnit = new QComboBox( FALSE, spacingOptions, "spacingAboveMinusUnit" );
    spacingAboveMinusUnit->insertItem( tr( "Centimetres" ) );
    spacingAboveMinusUnit->insertItem( tr( "Inches" ) );
    spacingAboveMinusUnit->insertItem( tr( "Points" ) );
    spacingAboveMinusUnit->insertItem( tr( "Millimetres" ) );
    spacingAboveMinusUnit->insertItem( tr( "Picas" ) );
    spacingAboveMinusUnit->insertItem( tr( "ex Units" ) );
    spacingAboveMinusUnit->insertItem( tr( "em Units" ) );
    spacingAboveMinusUnit->insertItem( tr( "Scaled Points" ) );
    spacingAboveMinusUnit->insertItem( tr( "Big/PS Points" ) );
    spacingAboveMinusUnit->insertItem( tr( "Didot Points" ) );
    spacingAboveMinusUnit->insertItem( tr( "Cicero Points" ) );
    spacingAboveMinusUnit->setEnabled( FALSE );
    spacingAboveMinusUnit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, spacingAboveMinusUnit->sizePolicy().hasHeightForWidth() ) );

    spacingOptionsLayout->addWidget( spacingAboveMinusUnit, 2, 4 );

    spacingAboveValue = new QLineEdit( spacingOptions, "spacingAboveValue" );
    spacingAboveValue->setEnabled( FALSE );

    spacingOptionsLayout->addWidget( spacingAboveValue, 1, 2 );

    spacingAbovePlus = new QLineEdit( spacingOptions, "spacingAbovePlus" );
    spacingAbovePlus->setEnabled( FALSE );

    spacingOptionsLayout->addWidget( spacingAbovePlus, 1, 3 );

    spacingAboveMinus = new QLineEdit( spacingOptions, "spacingAboveMinus" );
    spacingAboveMinus->setEnabled( FALSE );

    spacingOptionsLayout->addWidget( spacingAboveMinus, 1, 4 );

    spacingBelowMinus = new QLineEdit( spacingOptions, "spacingBelowMinus" );
    spacingBelowMinus->setEnabled( FALSE );

    spacingOptionsLayout->addWidget( spacingBelowMinus, 3, 4 );

    Layout2_1 = new QHBoxLayout; 
    Layout2_1->setSpacing( 6 );
    Layout2_1->setMargin( 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout2_1->addItem( spacer_2 );

    spacingBelowUnitsL = new QLabel( spacingOptions, "spacingBelowUnitsL" );
    spacingBelowUnitsL->setText( tr( "Units:" ) );
    spacingBelowUnitsL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, spacingBelowUnitsL->sizePolicy().hasHeightForWidth() ) );
    spacingBelowUnitsL->setEnabled( FALSE );
    QToolTip::add(  spacingBelowUnitsL, tr( "Unit of Size, Stretch and Slink" ) );
    Layout2_1->addWidget( spacingBelowUnitsL );

    spacingOptionsLayout->addLayout( Layout2_1, 4, 1 );

    Layout2_1_2 = new QHBoxLayout; 
    Layout2_1_2->setSpacing( 6 );
    Layout2_1_2->setMargin( 0 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout2_1_2->addItem( spacer_3 );

    spacingAboveUnitsL = new QLabel( spacingOptions, "spacingAboveUnitsL" );
    spacingAboveUnitsL->setText( tr( "Units:" ) );
    spacingAboveUnitsL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, spacingAboveUnitsL->sizePolicy().hasHeightForWidth() ) );
    spacingAboveUnitsL->setEnabled( FALSE );
    QToolTip::add(  spacingAboveUnitsL, tr( "Unit of Size, Stretch and Slink" ) );
    Layout2_1_2->addWidget( spacingAboveUnitsL );

    spacingOptionsLayout->addLayout( Layout2_1_2, 2, 1 );

    spacingValueL = new QLabel( spacingOptions, "spacingValueL" );
    spacingValueL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, spacingValueL->sizePolicy().hasHeightForWidth() ) );
    spacingValueL->setText( tr( "Value:" ) );
    spacingValueL->setEnabled( FALSE );
    QToolTip::add(  spacingValueL, tr( "Amount of spacing" ) );

    spacingOptionsLayout->addWidget( spacingValueL, 0, 2 );

    spacingPlusL = new QLabel( spacingOptions, "spacingPlusL" );
    spacingPlusL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, spacingPlusL->sizePolicy().hasHeightForWidth() ) );
    spacingPlusL->setText( tr( "Stretch:" ) );
    spacingPlusL->setEnabled( FALSE );
    QToolTip::add(  spacingPlusL, tr( "Margin by with paragraph is allowed to increase" ) );

    spacingOptionsLayout->addWidget( spacingPlusL, 0, 3 );

    spacingMinusL = new QLabel( spacingOptions, "spacingMinusL" );
    spacingMinusL->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, spacingMinusL->sizePolicy().hasHeightForWidth() ) );
    spacingMinusL->setText( tr( "Shrink:" ) );
    spacingMinusL->setEnabled( FALSE );
    QToolTip::add(  spacingMinusL, tr( "Margin by with paragraph is allowed to increase" ) );

    spacingOptionsLayout->addWidget( spacingMinusL, 0, 4 );

    spacingAbove = new QComboBox( FALSE, spacingOptions, "spacingAbove" );
    spacingAbove->insertItem( tr( "None" ) );
    spacingAbove->insertItem( tr( "DefSkip" ) );
    spacingAbove->insertItem( tr( "SmallSkip" ) );
    spacingAbove->insertItem( tr( "MedSkip" ) );
    spacingAbove->insertItem( tr( "BigSkip" ) );
    spacingAbove->insertItem( tr( "VFill" ) );
    spacingAbove->insertItem( tr( "Custom" ) );
    spacingAbove->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, spacingAbove->sizePolicy().hasHeightForWidth() ) );

    spacingOptionsLayout->addWidget( spacingAbove, 1, 1 );

    spacingAboveL = new QLabel( spacingOptions, "spacingAboveL" );
    spacingAboveL->setText( tr( "Above:" ) );

    spacingOptionsLayout->addWidget( spacingAboveL, 1, 0 );

    spacingBelowL = new QLabel( spacingOptions, "spacingBelowL" );
    spacingBelowL->setText( tr( "Below:" ) );

    spacingOptionsLayout->addWidget( spacingBelowL, 3, 0 );

    spacingBelow = new QComboBox( FALSE, spacingOptions, "spacingBelow" );
    spacingBelow->insertItem( tr( "None" ) );
    spacingBelow->insertItem( tr( "DefSkip" ) );
    spacingBelow->insertItem( tr( "SmallSkip" ) );
    spacingBelow->insertItem( tr( "MedSkip" ) );
    spacingBelow->insertItem( tr( "BigSkip" ) );
    spacingBelow->insertItem( tr( "VFill" ) );
    spacingBelow->insertItem( tr( "Custom" ) );
    spacingBelow->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, spacingBelow->sizePolicy().hasHeightForWidth() ) );

    spacingOptionsLayout->addWidget( spacingBelow, 3, 1 );

    spacingAboveKeep = new QCheckBox( spacingOptions, "spacingAboveKeep" );
    spacingAboveKeep->setText( tr( "Keep space at the top of the page" ) );
    spacingAboveKeep->setEnabled( TRUE );
    QToolTip::add(  spacingAboveKeep, tr( "Keep space at top of the page" ) );

    spacingOptionsLayout->addMultiCellWidget( spacingAboveKeep, 5, 5, 1, 2 );

    spacingBelowKeep = new QCheckBox( spacingOptions, "spacingBelowKeep" );
    spacingBelowKeep->setText( tr( "Keep space at the bottom of the page" ) );
    spacingBelowKeep->setEnabled( TRUE );
    QToolTip::add(  spacingBelowKeep, tr( "Keep space at the bottom of the page" ) );

    spacingOptionsLayout->addMultiCellWidget( spacingBelowKeep, 5, 5, 3, 4 );
    tabLayout->addWidget( spacingOptions );

    listOptions = new QGroupBox( tab, "listOptions" );
    listOptions->setTitle( tr( "List environment" ) );
    listOptions->setEnabled( FALSE );
    listOptions->setColumnLayout(0, Qt::Vertical );
    listOptions->layout()->setSpacing( 0 );
    listOptions->layout()->setMargin( 0 );
    listOptionsLayout = new QHBoxLayout( listOptions->layout() );
    listOptionsLayout->setAlignment( Qt::AlignTop );
    listOptionsLayout->setSpacing( 6 );
    listOptionsLayout->setMargin( 11 );

    labelWidthL = new QLabel( listOptions, "labelWidthL" );
    labelWidthL->setText( tr( "Label width:" ) );
    labelWidthL->setEnabled( FALSE );
    QToolTip::add(  labelWidthL, tr( "Label width in list environment" ) );
    listOptionsLayout->addWidget( labelWidthL );

    labelWidth = new QLineEdit( listOptions, "labelWidth" );
    labelWidth->setEnabled( FALSE );
    listOptionsLayout->addWidget( labelWidth );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    listOptionsLayout->addItem( spacer_4 );
    tabLayout->addWidget( listOptions );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout->addItem( spacer_5 );
    TabsParagraph->insertTab( tab, tr( "&Alignment and Spacing" ) );

    tab_2 = new QWidget( TabsParagraph, "tab_2" );
    tabLayout_2 = new QGridLayout( tab_2 ); 
    tabLayout_2->setSpacing( 6 );
    tabLayout_2->setMargin( 11 );

    pagebreakOptions = new QGroupBox( tab_2, "pagebreakOptions" );
    pagebreakOptions->setTitle( tr( "Page break" ) );
    pagebreakOptions->setOrientation( QGroupBox::Vertical );
    pagebreakOptions->setColumnLayout(0, Qt::Vertical );
    pagebreakOptions->layout()->setSpacing( 0 );
    pagebreakOptions->layout()->setMargin( 0 );
    pagebreakOptionsLayout = new QVBoxLayout( pagebreakOptions->layout() );
    pagebreakOptionsLayout->setAlignment( Qt::AlignTop );
    pagebreakOptionsLayout->setSpacing( 6 );
    pagebreakOptionsLayout->setMargin( 11 );

    pagebreakAbove = new QCheckBox( pagebreakOptions, "pagebreakAbove" );
    pagebreakAbove->setText( tr( "above paragraph" ) );
    pagebreakOptionsLayout->addWidget( pagebreakAbove );

    pagebreakBelow = new QCheckBox( pagebreakOptions, "pagebreakBelow" );
    pagebreakBelow->setText( tr( "below paragraph" ) );
    pagebreakOptionsLayout->addWidget( pagebreakBelow );

    tabLayout_2->addWidget( pagebreakOptions, 0, 1 );
    QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    tabLayout_2->addItem( spacer_6, 0, 2 );
    QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout_2->addItem( spacer_7, 1, 2 );

    lineOptions = new QGroupBox( tab_2, "lineOptions" );
    lineOptions->setTitle( tr( "Line" ) );
    lineOptions->setColumnLayout(0, Qt::Vertical );
    lineOptions->layout()->setSpacing( 0 );
    lineOptions->layout()->setMargin( 0 );
    lineOptionsLayout = new QVBoxLayout( lineOptions->layout() );
    lineOptionsLayout->setAlignment( Qt::AlignTop );
    lineOptionsLayout->setSpacing( 6 );
    lineOptionsLayout->setMargin( 11 );

    lineAbove = new QCheckBox( lineOptions, "lineAbove" );
    lineAbove->setText( tr( "above paragraph" ) );
    lineOptionsLayout->addWidget( lineAbove );

    lineBelow = new QCheckBox( lineOptions, "lineBelow" );
    lineBelow->setText( tr( "below paragraph" ) );
    lineOptionsLayout->addWidget( lineBelow );

    tabLayout_2->addWidget( lineOptions, 0, 0 );
    TabsParagraph->insertTab( tab_2, tr( "&Lines and Page breaks" ) );

    tab_3 = new QWidget( TabsParagraph, "tab_3" );
    tabLayout_3 = new QGridLayout( tab_3 ); 
    tabLayout_3->setSpacing( 6 );
    tabLayout_3->setMargin( 11 );
    QSpacerItem* spacer_8 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    tabLayout_3->addItem( spacer_8, 1, 1 );

    Layout7 = new QGridLayout; 
    Layout7->setSpacing( 6 );
    Layout7->setMargin( 0 );

    extraUnitL = new QLabel( tab_3, "extraUnitL" );
    extraUnitL->setText( tr( "Unit:" ) );
    extraUnitL->setEnabled( FALSE );

    Layout7->addWidget( extraUnitL, 0, 2 );

    extraUnit = new QComboBox( FALSE, tab_3, "extraUnit" );
    extraUnit->insertItem( tr( "Centimetres" ) );
    extraUnit->insertItem( tr( "Inches" ) );
    extraUnit->insertItem( tr( "Points" ) );
    extraUnit->insertItem( tr( "Millimetres" ) );
    extraUnit->insertItem( tr( "Picas" ) );
    extraUnit->insertItem( tr( "ex Units" ) );
    extraUnit->insertItem( tr( "em Units" ) );
    extraUnit->insertItem( tr( "Scaled Points" ) );
    extraUnit->insertItem( tr( "Big/PS Points" ) );
    extraUnit->insertItem( tr( "Didot Points" ) );
    extraUnit->insertItem( tr( "Cicero Points" ) );
    extraUnit->setFocusPolicy( QComboBox::TabFocus );
    extraUnit->setEnabled( FALSE );

    Layout7->addWidget( extraUnit, 1, 2 );

    extraWidth = new QLineEdit( tab_3, "extraWidth" );
    extraWidth->setEnabled( FALSE );

    Layout7->addWidget( extraWidth, 1, 1 );

    extraType = new QComboBox( FALSE, tab_3, "extraType" );
    extraType->insertItem( tr( "None" ) );
    extraType->insertItem( tr( "Minipage" ) );
    extraType->insertItem( tr( "Wrap text around floats" ) );
    extraType->insertItem( tr( "Indent whole paragraph" ) );

    Layout7->addWidget( extraType, 1, 0 );

    extraWidthL = new QLabel( tab_3, "extraWidthL" );
    extraWidthL->setText( tr( "Width:" ) );
    extraWidthL->setEnabled( FALSE );

    Layout7->addWidget( extraWidthL, 0, 1 );

    tabLayout_3->addLayout( Layout7, 0, 0 );

    minipageOptions = new QGroupBox( tab_3, "minipageOptions" );
    minipageOptions->setTitle( tr( "Minipage options" ) );
    minipageOptions->setEnabled( FALSE );
    minipageOptions->setColumnLayout(0, Qt::Vertical );
    minipageOptions->layout()->setSpacing( 0 );
    minipageOptions->layout()->setMargin( 0 );
    minipageOptionsLayout = new QVBoxLayout( minipageOptions->layout() );
    minipageOptionsLayout->setAlignment( Qt::AlignTop );
    minipageOptionsLayout->setSpacing( 6 );
    minipageOptionsLayout->setMargin( 11 );

    minipageStart = new QCheckBox( minipageOptions, "minipageStart" );
    minipageStart->setText( tr( "Start new minipage" ) );
    minipageStart->setEnabled( FALSE );
    minipageOptionsLayout->addWidget( minipageStart );

    minipageHfill = new QCheckBox( minipageOptions, "minipageHfill" );
    minipageHfill->setText( tr( "HFill between minipage paragraphs" ) );
    minipageHfill->setEnabled( FALSE );
    minipageOptionsLayout->addWidget( minipageHfill );

    Layout15 = new QHBoxLayout; 
    Layout15->setSpacing( 6 );
    Layout15->setMargin( 0 );

    minipageValignL = new QLabel( minipageOptions, "minipageValignL" );
    minipageValignL->setText( tr( "Vertical Alignment:" ) );
    Layout15->addWidget( minipageValignL );

    minipageValign = new QComboBox( FALSE, minipageOptions, "minipageValign" );
    minipageValign->insertItem( tr( "Top" ) );
    minipageValign->insertItem( tr( "Middle" ) );
    minipageValign->insertItem( tr( "Bottom" ) );
    Layout15->addWidget( minipageValign );
    QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout15->addItem( spacer_9 );
    minipageOptionsLayout->addLayout( Layout15 );

    tabLayout_3->addWidget( minipageOptions, 1, 0 );
    QSpacerItem* spacer_10 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout_3->addItem( spacer_10, 2, 0 );
    TabsParagraph->insertTab( tab_3, tr( "&Extra options" ) );
    ParagraphDlgLayout->addWidget( TabsParagraph );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );

    defaultsPB = new QPushButton( this, "defaultsPB" );
    defaultsPB->setText( tr( "&Restore" ) );
    Layout1->addWidget( defaultsPB );
    QSpacerItem* spacer_11 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer_11 );

    applyPB = new QPushButton( this, "applyPB" );
    applyPB->setText( tr( "&Apply" ) );
    Layout1->addWidget( applyPB );

    okPB = new QPushButton( this, "okPB" );
    okPB->setText( tr( "&OK" ) );
    okPB->setFlat( FALSE );
    okPB->setDefault( FALSE );
    Layout1->addWidget( okPB );

    cancelPB = new QPushButton( this, "cancelPB" );
    cancelPB->setText( tr( "&Cancel" ) );
    cancelPB->setDefault( TRUE );
    Layout1->addWidget( cancelPB );
    ParagraphDlgLayout->addLayout( Layout1 );

    // signals and slots connections
    connect( defaultsPB, SIGNAL( clicked() ), this, SLOT( restore_adaptor() ) );
    connect( applyPB, SIGNAL( clicked() ), this, SLOT( apply_adaptor() ) );
    connect( cancelPB, SIGNAL( clicked() ), this, SLOT( cancel_adaptor() ) );
    connect( okPB, SIGNAL( clicked() ), this, SLOT( ok_adaptor() ) );
    connect( spacingAbove, SIGNAL( activated(int) ), this, SLOT( enable_spacingAbove(int) ) );
    connect( spacingBelow, SIGNAL( activated(int) ), this, SLOT( enable_spacingBelow(int) ) );
    connect( extraType, SIGNAL( activated(int) ), this, SLOT( enable_minipageOptions(int) ) );
    connect( extraType, SIGNAL( activated(int) ), this, SLOT( enable_extraOptions(int) ) );

    // tab order
    setTabOrder( TabsParagraph, alignment );
    setTabOrder( alignment, noIndent );
    setTabOrder( noIndent, spacingAbove );
    setTabOrder( spacingAbove, spacingAboveValue );
    setTabOrder( spacingAboveValue, spacingAbovePlus );
    setTabOrder( spacingAbovePlus, spacingAboveMinus );
    setTabOrder( spacingAboveMinus, spacingAboveKeep );
    setTabOrder( spacingAboveKeep, spacingAboveValueUnit );
    setTabOrder( spacingAboveValueUnit, spacingAbovePlusUnit );
    setTabOrder( spacingAbovePlusUnit, spacingAboveMinusUnit );
    setTabOrder( spacingAboveMinusUnit, spacingBelow );
    setTabOrder( spacingBelow, spacingBelowValue );
    setTabOrder( spacingBelowValue, spacingBelowPlus );
    setTabOrder( spacingBelowPlus, spacingBelowMinus );
    setTabOrder( spacingBelowMinus, spacingBelowKeep );
    setTabOrder( spacingBelowKeep, spacingBelowValueUnit );
    setTabOrder( spacingBelowValueUnit, spacingBelowPlusUnit );
    setTabOrder( spacingBelowPlusUnit, spacingBelowMinusUnit );
    setTabOrder( spacingBelowMinusUnit, labelWidth );
    setTabOrder( labelWidth, lineAbove );
    setTabOrder( lineAbove, lineBelow );
    setTabOrder( lineBelow, pagebreakAbove );
    setTabOrder( pagebreakAbove, pagebreakBelow );
    setTabOrder( pagebreakBelow, extraType );
    setTabOrder( extraType, extraWidth );
    setTabOrder( extraWidth, extraUnit );
    setTabOrder( extraUnit, minipageStart );
    setTabOrder( minipageStart, minipageHfill );
    setTabOrder( minipageHfill, minipageValign );
    setTabOrder( minipageValign, cancelPB );
    setTabOrder( cancelPB, okPB );
    setTabOrder( okPB, applyPB );
    setTabOrder( applyPB, defaultsPB );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ParagraphDlg::~ParagraphDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

void ParagraphDlg::apply_adaptor()
{
    qWarning( "ParagraphDlg::apply_adaptor(): Not implemented yet!" );
}

void ParagraphDlg::cancel_adaptor()
{
    qWarning( "ParagraphDlg::cancel_adaptor(): Not implemented yet!" );
}

void ParagraphDlg::enable_extraOptions(int)
{
    qWarning( "ParagraphDlg::enable_extraOptions(int): Not implemented yet!" );
}

void ParagraphDlg::enable_minipageOptions(int)
{
    qWarning( "ParagraphDlg::enable_minipageOptions(int): Not implemented yet!" );
}

void ParagraphDlg::enable_spacingAbove(int)
{
    qWarning( "ParagraphDlg::enable_spacingAbove(int): Not implemented yet!" );
}

void ParagraphDlg::enable_spacingBelow(int)
{
    qWarning( "ParagraphDlg::enable_spacingBelow(int): Not implemented yet!" );
}

void ParagraphDlg::ok_adaptor()
{
    qWarning( "ParagraphDlg::ok_adaptor(): Not implemented yet!" );
}

void ParagraphDlg::restore_adaptor()
{
    qWarning( "ParagraphDlg::restore_adaptor(): Not implemented yet!" );
}

