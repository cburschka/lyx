/****************************************************************************
** Form implementation generated from reading ui file 'printdlg.ui'
**
** Created: Sun Feb 4 23:02:20 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "printdlg.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a PrintDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PrintDlg::PrintDlg( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "PrintDlg" );
    resize( 363, 342 ); 
    setCaption( tr( "Form1" ) );
    PrintDlgLayout = new QVBoxLayout( this ); 
    PrintDlgLayout->setSpacing( 6 );
    PrintDlgLayout->setMargin( 11 );

    ButtonGroup1 = new QButtonGroup( this, "ButtonGroup1" );
    ButtonGroup1->setTitle( tr( "Print Destination" ) );
    ButtonGroup1->setColumnLayout(0, Qt::Vertical );
    ButtonGroup1->layout()->setSpacing( 0 );
    ButtonGroup1->layout()->setMargin( 0 );
    ButtonGroup1Layout = new QGridLayout( ButtonGroup1->layout() );
    ButtonGroup1Layout->setAlignment( Qt::AlignTop );
    ButtonGroup1Layout->setSpacing( 3 );
    ButtonGroup1Layout->setMargin( 11 );

    toPrinter = new QRadioButton( ButtonGroup1, "toPrinter" );
    toPrinter->setText( tr( "&Printer" ) );
    toPrinter->setChecked( TRUE );

    ButtonGroup1Layout->addWidget( toPrinter, 0, 0 );

    toFile = new QRadioButton( ButtonGroup1, "toFile" );
    toFile->setText( tr( "&File" ) );

    ButtonGroup1Layout->addWidget( toFile, 1, 0 );

    printerName = new QLineEdit( ButtonGroup1, "printerName" );

    ButtonGroup1Layout->addWidget( printerName, 0, 1 );

    fileName = new QLineEdit( ButtonGroup1, "fileName" );
    fileName->setEnabled( FALSE );

    ButtonGroup1Layout->addWidget( fileName, 1, 1 );

    browsePB = new QPushButton( ButtonGroup1, "browsePB" );
    browsePB->setText( tr( "&Browse..." ) );
    browsePB->setEnabled( FALSE );

    ButtonGroup1Layout->addWidget( browsePB, 1, 2 );
    PrintDlgLayout->addWidget( ButtonGroup1 );

    ButtonGroup3 = new QButtonGroup( this, "ButtonGroup3" );
    ButtonGroup3->setTitle( tr( "Pages" ) );
    ButtonGroup3->setColumnLayout(0, Qt::Vertical );
    ButtonGroup3->layout()->setSpacing( 0 );
    ButtonGroup3->layout()->setMargin( 0 );
    ButtonGroup3Layout = new QGridLayout( ButtonGroup3->layout() );
    ButtonGroup3Layout->setAlignment( Qt::AlignTop );
    ButtonGroup3Layout->setSpacing( 3 );
    ButtonGroup3Layout->setMargin( 11 );

    allPages = new QRadioButton( ButtonGroup3, "allPages" );
    allPages->setText( tr( "All" ) );
    allPages->setChecked( TRUE );

    ButtonGroup3Layout->addWidget( allPages, 0, 0 );

    oddPages = new QRadioButton( ButtonGroup3, "oddPages" );
    oddPages->setText( tr( "Odd" ) );

    ButtonGroup3Layout->addWidget( oddPages, 1, 0 );

    evenPages = new QRadioButton( ButtonGroup3, "evenPages" );
    evenPages->setText( tr( "Even" ) );

    ButtonGroup3Layout->addWidget( evenPages, 2, 0 );

    fromPageL = new QLabel( ButtonGroup3, "fromPageL" );
    fromPageL->setText( tr( "Starting range:" ) );
    fromPageL->setEnabled( FALSE );

    ButtonGroup3Layout->addWidget( fromPageL, 3, 1 );

    toPageL = new QLabel( ButtonGroup3, "toPageL" );
    toPageL->setText( tr( "Last page:" ) );
    toPageL->setEnabled( FALSE );

    ButtonGroup3Layout->addWidget( toPageL, 3, 2 );

    toPage = new QLineEdit( ButtonGroup3, "toPage" );
    toPage->setEnabled( FALSE );

    ButtonGroup3Layout->addWidget( toPage, 4, 2 );

    reverse = new QCheckBox( ButtonGroup3, "reverse" );
    reverse->setText( tr( "&Reverse order" ) );

    ButtonGroup3Layout->addWidget( reverse, 0, 2 );

    fromPage = new QLineEdit( ButtonGroup3, "fromPage" );
    fromPage->setEnabled( FALSE );

    ButtonGroup3Layout->addWidget( fromPage, 4, 1 );

    customPages = new QRadioButton( ButtonGroup3, "customPages" );
    customPages->setText( tr( "Pages:" ) );

    ButtonGroup3Layout->addWidget( customPages, 4, 0 );
    PrintDlgLayout->addWidget( ButtonGroup3 );

    GroupBox3 = new QGroupBox( this, "GroupBox3" );
    GroupBox3->setTitle( tr( "Copies" ) );
    GroupBox3->setColumnLayout(0, Qt::Vertical );
    GroupBox3->layout()->setSpacing( 0 );
    GroupBox3->layout()->setMargin( 0 );
    GroupBox3Layout = new QHBoxLayout( GroupBox3->layout() );
    GroupBox3Layout->setAlignment( Qt::AlignTop );
    GroupBox3Layout->setSpacing( 6 );
    GroupBox3Layout->setMargin( 11 );

    copies = new QSpinBox( GroupBox3, "copies" );
    copies->setMinValue( 1 );
    copies->setValue( 1 );
    GroupBox3Layout->addWidget( copies );

    collate = new QCheckBox( GroupBox3, "collate" );
    collate->setText( tr( "Co&llate" ) );
    collate->setEnabled( FALSE );
    GroupBox3Layout->addWidget( collate );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox3Layout->addItem( spacer );
    PrintDlgLayout->addWidget( GroupBox3 );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer_2 );

    printPB = new QPushButton( this, "printPB" );
    printPB->setText( tr( "&Print" ) );
    Layout1->addWidget( printPB );

    cancelPB = new QPushButton( this, "cancelPB" );
    cancelPB->setText( tr( "&Cancel" ) );
    cancelPB->setDefault( TRUE );
    Layout1->addWidget( cancelPB );
    PrintDlgLayout->addLayout( Layout1 );

    // signals and slots connections
    connect( toFile, SIGNAL( toggled(bool) ), browsePB, SLOT( setEnabled(bool) ) );
    connect( toFile, SIGNAL( toggled(bool) ), fileName, SLOT( setEnabled(bool) ) );
    connect( toFile, SIGNAL( toggled(bool) ), printerName, SLOT( setDisabled(bool) ) );
    connect( printPB, SIGNAL( clicked() ), this, SLOT( print() ) );
    connect( cancelPB, SIGNAL( clicked() ), this, SLOT( cancel_adaptor() ) );
    connect( copies, SIGNAL( valueChanged(int) ), this, SLOT( set_collate(int) ) );
    connect( browsePB, SIGNAL( clicked() ), this, SLOT( browse_file() ) );
    connect( customPages, SIGNAL( toggled(bool) ), fromPage, SLOT( setEnabled(bool) ) );
    connect( customPages, SIGNAL( toggled(bool) ), fromPageL, SLOT( setEnabled(bool) ) );
    connect( customPages, SIGNAL( toggled(bool) ), toPage, SLOT( setEnabled(bool) ) );
    connect( customPages, SIGNAL( toggled(bool) ), toPageL, SLOT( setEnabled(bool) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
PrintDlg::~PrintDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

void PrintDlg::cancel_adaptor()
{
    qWarning( "PrintDlg::cancel_adaptor(): Not implemented yet!" );
}

void PrintDlg::enable_pagerange(int)
{
    qWarning( "PrintDlg::enable_pagerange(int): Not implemented yet!" );
}

void PrintDlg::browse_file()
{
    qWarning( "PrintDlg::browse_file(): Not implemented yet!" );
}

void PrintDlg::print()
{
    qWarning( "PrintDlg::print(): Not implemented yet!" );
}

void PrintDlg::set_collate(int)
{
    qWarning( "PrintDlg::set_collate(int): Not implemented yet!" );
}

