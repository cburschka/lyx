/****************************************************************************
** Form implementation generated from reading ui file 'chardlg.ui'
**
** Created: Thu Mar 1 12:56:20 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "chardlg.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CharDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CharDlg::CharDlg( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "CharDlg" );
    resize( 341, 249 ); 
    setCaption( tr( "Character" ) );
    setSizeGripEnabled( TRUE );
    CharDlgLayout = new QGridLayout( this ); 
    CharDlgLayout->setSpacing( 6 );
    CharDlgLayout->setMargin( 11 );

    sizeGB = new QGroupBox( this, "sizeGB" );
    sizeGB->setTitle( tr( "Never toggled" ) );
    sizeGB->setColumnLayout(0, Qt::Vertical );
    sizeGB->layout()->setSpacing( 0 );
    sizeGB->layout()->setMargin( 0 );
    sizeGBLayout = new QHBoxLayout( sizeGB->layout() );
    sizeGBLayout->setAlignment( Qt::AlignTop );
    sizeGBLayout->setSpacing( 6 );
    sizeGBLayout->setMargin( 11 );

    sizeL = new QLabel( sizeGB, "sizeL" );
    sizeL->setText( tr( "Size:" ) );
    sizeGBLayout->addWidget( sizeL );

    size = new QComboBox( FALSE, sizeGB, "size" );
    size->insertItem( tr( "No Change" ) );
    size->insertItem( tr( "Reset" ) );
    size->insertItem( tr( "Tiny" ) );
    size->insertItem( tr( "Smallest" ) );
    size->insertItem( tr( "Smaller" ) );
    size->insertItem( tr( "Small" ) );
    size->insertItem( tr( "Normal" ) );
    size->insertItem( tr( "Large" ) );
    size->insertItem( tr( "Larger" ) );
    size->insertItem( tr( "Largest" ) );
    size->insertItem( tr( "Huge" ) );
    size->insertItem( tr( "Huger" ) );
    size->insertItem( tr( "Increase" ) );
    size->insertItem( tr( "Decrease" ) );
    sizeGBLayout->addWidget( size );

    CharDlgLayout->addWidget( sizeGB, 0, 1 );

    miscGB = new QGroupBox( this, "miscGB" );
    miscGB->setTitle( tr( "Always toggled" ) );
    miscGB->setColumnLayout(0, Qt::Vertical );
    miscGB->layout()->setSpacing( 0 );
    miscGB->layout()->setMargin( 0 );
    miscGBLayout = new QHBoxLayout( miscGB->layout() );
    miscGBLayout->setAlignment( Qt::AlignTop );
    miscGBLayout->setSpacing( 6 );
    miscGBLayout->setMargin( 11 );

    always_toggledL = new QLabel( miscGB, "always_toggledL" );
    always_toggledL->setText( tr( "Misc:" ) );
    miscGBLayout->addWidget( always_toggledL );

    misc = new QComboBox( FALSE, miscGB, "misc" );
    misc->insertItem( tr( "No Change" ) );
    misc->insertItem( tr( "Reset" ) );
    misc->insertItem( tr( "Emph" ) );
    misc->insertItem( tr( "Underbar" ) );
    misc->insertItem( tr( "Noun" ) );
    misc->insertItem( tr( "LaTeX Mode" ) );
    miscGBLayout->addWidget( misc );

    CharDlgLayout->addWidget( miscGB, 1, 1 );

    charGB = new QGroupBox( this, "charGB" );
    charGB->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, charGB->sizePolicy().hasHeightForWidth() ) );
    charGB->setTitle( tr( "Character" ) );
    charGB->setColumnLayout(0, Qt::Vertical );
    charGB->layout()->setSpacing( 0 );
    charGB->layout()->setMargin( 0 );
    charGBLayout = new QGridLayout( charGB->layout() );
    charGBLayout->setAlignment( Qt::AlignTop );
    charGBLayout->setSpacing( 6 );
    charGBLayout->setMargin( 11 );

    familyL = new QLabel( charGB, "familyL" );
    familyL->setText( tr( "Family:" ) );

    charGBLayout->addWidget( familyL, 0, 0 );

    family = new QComboBox( FALSE, charGB, "family" );
    family->insertItem( tr( "No Change" ) );
    family->insertItem( tr( "Reset" ) );
    family->insertItem( tr( "Roman" ) );
    family->insertItem( tr( "Sans Serif" ) );
    family->insertItem( tr( "Typewriter" ) );

    charGBLayout->addWidget( family, 0, 1 );

    seriesL = new QLabel( charGB, "seriesL" );
    seriesL->setText( tr( "Series:" ) );

    charGBLayout->addWidget( seriesL, 1, 0 );

    langL = new QLabel( charGB, "langL" );
    langL->setText( tr( "Language:" ) );

    charGBLayout->addWidget( langL, 4, 0 );

    shape = new QComboBox( FALSE, charGB, "shape" );
    shape->insertItem( tr( "No Change" ) );
    shape->insertItem( tr( "Reset" ) );
    shape->insertItem( tr( "Upright" ) );
    shape->insertItem( tr( "Italic" ) );
    shape->insertItem( tr( "Slanted" ) );
    shape->insertItem( tr( "Small Caps" ) );

    charGBLayout->addWidget( shape, 2, 1 );

    color = new QComboBox( FALSE, charGB, "color" );
    color->insertItem( tr( "No Change" ) );
    color->insertItem( tr( "Reset" ) );
    color->insertItem( tr( "No Color" ) );
    color->insertItem( tr( "Black" ) );
    color->insertItem( tr( "White" ) );
    color->insertItem( tr( "Red" ) );
    color->insertItem( tr( "Green" ) );
    color->insertItem( tr( "Blue" ) );
    color->insertItem( tr( "Cyan" ) );
    color->insertItem( tr( "Yellow" ) );
    color->insertItem( tr( "Magenta" ) );

    charGBLayout->addWidget( color, 3, 1 );

    shapeL = new QLabel( charGB, "shapeL" );
    shapeL->setText( tr( "Shape:" ) );

    charGBLayout->addWidget( shapeL, 2, 0 );

    series = new QComboBox( FALSE, charGB, "series" );
    series->insertItem( tr( "No Change" ) );
    series->insertItem( tr( "Reset" ) );
    series->insertItem( tr( "Medium" ) );
    series->insertItem( tr( "Bold" ) );

    charGBLayout->addWidget( series, 1, 1 );

    colorL = new QLabel( charGB, "colorL" );
    colorL->setEnabled( TRUE );
    colorL->setText( tr( "Color:" ) );

    charGBLayout->addWidget( colorL, 3, 0 );

    lang = new QComboBox( FALSE, charGB, "lang" );
    lang->insertItem( tr( "No Change" ) );
    lang->insertItem( tr( "Reset" ) );

    charGBLayout->addWidget( lang, 4, 1 );

    toggleall = new QCheckBox( charGB, "toggleall" );
    toggleall->setText( tr( "Toggle all" ) );

    charGBLayout->addWidget( toggleall, 5, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    charGBLayout->addItem( spacer, 6, 1 );

    CharDlgLayout->addMultiCellWidget( charGB, 0, 2, 0, 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CharDlgLayout->addItem( spacer_2, 2, 1 );

    Layout9 = new QHBoxLayout; 
    Layout9->setSpacing( 6 );
    Layout9->setMargin( 0 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout9->addItem( spacer_3 );

    applyPB = new QPushButton( this, "applyPB" );
    applyPB->setText( tr( "&Apply" ) );
    Layout9->addWidget( applyPB );

    okPB = new QPushButton( this, "okPB" );
    okPB->setText( tr( "&OK" ) );
    Layout9->addWidget( okPB );

    cancelPB = new QPushButton( this, "cancelPB" );
    cancelPB->setText( tr( "&Cancel" ) );
    cancelPB->setDefault( TRUE );
    Layout9->addWidget( cancelPB );

    CharDlgLayout->addMultiCellLayout( Layout9, 3, 3, 0, 1 );

    // signals and slots connections
    connect( cancelPB, SIGNAL( clicked() ), this, SLOT( cancel_adaptor() ) );
    connect( okPB, SIGNAL( clicked() ), this, SLOT( close_adaptor() ) );
    connect( applyPB, SIGNAL( clicked() ), this, SLOT( apply_adaptor() ) );

    // tab order
    setTabOrder( family, series );
    setTabOrder( series, shape );
    setTabOrder( shape, color );
    setTabOrder( color, lang );
    setTabOrder( lang, toggleall );
    setTabOrder( toggleall, size );
    setTabOrder( size, misc );
    setTabOrder( misc, applyPB );
    setTabOrder( applyPB, okPB );
    setTabOrder( okPB, cancelPB );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CharDlg::~CharDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

void CharDlg::apply_adaptor()
{
    qWarning( "CharDlg::apply_adaptor(): Not implemented yet!" );
}

void CharDlg::cancel_adaptor()
{
    qWarning( "CharDlg::cancel_adaptor(): Not implemented yet!" );
}

void CharDlg::close_adaptor()
{
    qWarning( "CharDlg::close_adaptor(): Not implemented yet!" );
}

