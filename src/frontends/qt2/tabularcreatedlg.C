/****************************************************************************
** Form implementation generated from reading ui file 'tabularcreatedlg.ui'
**
** Created: Tue Feb 6 01:41:28 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "tabularcreatedlg.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include "emptytable.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a InsertTabularDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
InsertTabularDlg::InsertTabularDlg( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "InsertTabularDlg" );
    resize( 245, 235 ); 
    setSizeGripEnabled( TRUE );
    setCaption( tr( "Form1" ) );
    InsertTabularDlgLayout = new QVBoxLayout( this ); 
    InsertTabularDlgLayout->setSpacing( 6 );
    InsertTabularDlgLayout->setMargin( 11 );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );

    rowsL = new QLabel( this, "rowsL" );
    rowsL->setText( tr( "Rows:" ) );
    Layout1->addWidget( rowsL );

    rows = new QSpinBox( this, "rows" );
    rows->setMinValue( 1 );
    rows->setMaxValue( 511 );
    Layout1->addWidget( rows );

    columnsL = new QLabel( this, "columnsL" );
    columnsL->setText( tr( "Columns:" ) );
    Layout1->addWidget( columnsL );

    columns = new QSpinBox( this, "columns" );
    columns->setMinValue( 1 );
    columns->setMaxValue( 511 );
    Layout1->addWidget( columns );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );
    InsertTabularDlgLayout->addLayout( Layout1 );

    table = new EmptyTable( this, "table" );
    InsertTabularDlgLayout->addWidget( table );

    Layout2 = new QHBoxLayout; 
    Layout2->setSpacing( 6 );
    Layout2->setMargin( 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout2->addItem( spacer_2 );

    insertPB = new QPushButton( this, "insertPB" );
    insertPB->setText( tr( "&Insert" ) );
    Layout2->addWidget( insertPB );

    cancelPB = new QPushButton( this, "cancelPB" );
    cancelPB->setText( tr( "&Cancel" ) );
    Layout2->addWidget( cancelPB );
    InsertTabularDlgLayout->addLayout( Layout2 );

    // signals and slots connections
    connect( table, SIGNAL( rowsChanged(int) ), rows, SLOT( setValue(int) ) );
    connect( table, SIGNAL( colsChanged(int) ), columns, SLOT( setValue(int) ) );
    connect( rows, SIGNAL( valueChanged(int) ), table, SLOT( setNumberRows(int) ) );
    connect( columns, SIGNAL( valueChanged(int) ), table, SLOT( setNumberColumns(int) ) );
    connect( insertPB, SIGNAL( clicked() ), this, SLOT( insert_tabular() ) );
    connect( cancelPB, SIGNAL( clicked() ), this, SLOT( cancel_adaptor() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
InsertTabularDlg::~InsertTabularDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

void InsertTabularDlg::insert_tabular()
{
    qWarning( "InsertTabularDlg::insert_tabular(): Not implemented yet!" );
}

void InsertTabularDlg::cancel_adaptor()
{
    qWarning( "InsertTabularDlg::cancel_adaptor(): Not implemented yet!" );
}

