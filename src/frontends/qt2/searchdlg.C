/****************************************************************************
** Form implementation generated from reading ui file 'searchdlg.ui'
**
** Created: Mon Mar 26 21:50:02 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "searchdlg.h"

#include <config.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a SearchDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SearchDlg::SearchDlg( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "SearchDlg" );
    resize( 388, 168 ); 
    setCaption( tr( "Form1" ) );
    setSizeGripEnabled( TRUE );
    SearchDlgLayout = new QGridLayout( this ); 
    SearchDlgLayout->setSpacing( 6 );
    SearchDlgLayout->setMargin( 11 );

    findStrLabel = new QLabel( this, "findStrLabel" );
    findStrLabel->setText( tr( "Find:" ) );

    SearchDlgLayout->addWidget( findStrLabel, 0, 0 );

    find = new QComboBox( FALSE, this, "find" );
    find->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, find->sizePolicy().hasHeightForWidth() ) );
    find->setFocusPolicy( QComboBox::StrongFocus );
    find->setEditable( TRUE );
    find->setDuplicatesEnabled( TRUE );

    SearchDlgLayout->addMultiCellWidget( find, 0, 0, 1, 2 );

    replaceLabel = new QLabel( this, "replaceLabel" );
    replaceLabel->setText( tr( "Replace with:" ) );

    SearchDlgLayout->addWidget( replaceLabel, 1, 0 );

    replace = new QComboBox( FALSE, this, "replace" );
    replace->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, replace->sizePolicy().hasHeightForWidth() ) );
    replace->setEditable( TRUE );

    SearchDlgLayout->addMultiCellWidget( replace, 1, 1, 1, 2 );

    caseSensitive = new QCheckBox( this, "caseSensitive" );
    caseSensitive->setText( tr( "&Case sensitive" ) );

    SearchDlgLayout->addMultiCellWidget( caseSensitive, 2, 2, 0, 1 );

    matchWord = new QCheckBox( this, "matchWord" );
    matchWord->setText( tr( "Match whole words onl&y" ) );

    SearchDlgLayout->addMultiCellWidget( matchWord, 3, 4, 0, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    SearchDlgLayout->addItem( spacer, 2, 2 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    SearchDlgLayout->addMultiCell( spacer_2, 6, 6, 0, 1 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    SearchDlgLayout->addItem( spacer_3, 6, 3 );

    findPB = new QPushButton( this, "findPB" );
    findPB->setText( tr( "Find &Next" ) );

    SearchDlgLayout->addWidget( findPB, 0, 3 );

    replacePB = new QPushButton( this, "replacePB" );
    replacePB->setText( tr( "&Replace" ) );

    SearchDlgLayout->addWidget( replacePB, 1, 3 );

    replaceAllPB = new QPushButton( this, "replaceAllPB" );
    replaceAllPB->setText( tr( "Replace &All " ) );

    SearchDlgLayout->addMultiCellWidget( replaceAllPB, 2, 3, 3, 3 );

    searchBack = new QCheckBox( this, "searchBack" );
    searchBack->setText( tr( "Search &backwards" ) );

    SearchDlgLayout->addMultiCellWidget( searchBack, 5, 5, 0, 1 );

    cancelPB = new QPushButton( this, "cancelPB" );
    cancelPB->setText( tr( "&Cancel" ) );

    SearchDlgLayout->addMultiCellWidget( cancelPB, 4, 5, 3, 3 );

    // signals and slots connections
    connect( findPB, SIGNAL( clicked() ), this, SLOT( Find() ) );
    connect( replacePB, SIGNAL( clicked() ), this, SLOT( Replace() ) );
    connect( replaceAllPB, SIGNAL( clicked() ), this, SLOT( ReplaceAll() ) );
    connect( cancelPB, SIGNAL( clicked() ), this, SLOT( cancel_adaptor() ) );

    // tab order
    setTabOrder( find, replace );
    setTabOrder( replace, caseSensitive );
    setTabOrder( caseSensitive, matchWord );
    setTabOrder( matchWord, searchBack );
    setTabOrder( searchBack, findPB );
    setTabOrder( findPB, replacePB );
    setTabOrder( replacePB, replaceAllPB );
    setTabOrder( replaceAllPB, cancelPB );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SearchDlg::~SearchDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

void SearchDlg::Find()
{
    qWarning( "SearchDlg::Find(): Not implemented yet!" );
}

void SearchDlg::Replace()
{
    qWarning( "SearchDlg::Replace(): Not implemented yet!" );
}

void SearchDlg::ReplaceAll()
{
    qWarning( "SearchDlg::ReplaceAll(): Not implemented yet!" );
}

void SearchDlg::cancel_adaptor()
{
    qWarning( "SearchDlg::cancel_adaptor(): Not implemented yet!" );
}

