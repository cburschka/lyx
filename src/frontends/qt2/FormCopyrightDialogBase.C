/****************************************************************************
** Form implementation generated from reading ui file 'FormCopyrightDialogBase.ui'
**
** Created: Wed Feb 7 18:46:40 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "FormCopyrightDialogBase.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a FormCopyrightDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FormCopyrightDialogBase::FormCopyrightDialogBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "FormCopyrightDialogBase" );
    resize( 472, 454 ); 
    setCaption( tr( "Form1" ) );
    FormCopyrightDialogBaseLayout = new QVBoxLayout( this ); 
    FormCopyrightDialogBaseLayout->setSpacing( 6 );
    FormCopyrightDialogBaseLayout->setMargin( 11 );

    TextLabel5 = new QLabel( this, "TextLabel5" );
    TextLabel5->setText( tr( "LyX is Copyright (C) 1995 by Matthias Ettrich,\n1995-2001 LyX Team" ) );
    TextLabel5->setFrameShape( QLabel::Box );
    TextLabel5->setFrameShadow( QLabel::Sunken );
    TextLabel5->setMargin( 6 );
    TextLabel5->setAlignment( int( QLabel::AlignCenter ) );
    FormCopyrightDialogBaseLayout->addWidget( TextLabel5 );

    TextLabel5_2 = new QLabel( this, "TextLabel5_2" );
    TextLabel5_2->setText( tr( "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version." ) );
    TextLabel5_2->setFrameShape( QLabel::Box );
    TextLabel5_2->setFrameShadow( QLabel::Sunken );
    TextLabel5_2->setMargin( 6 );
    TextLabel5_2->setAlignment( int( QLabel::WordBreak | QLabel::AlignCenter ) );
    TextLabel5_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, TextLabel5_2->sizePolicy().hasHeightForWidth() ) );
    FormCopyrightDialogBaseLayout->addWidget( TextLabel5_2 );

    TextLabel5_2_2 = new QLabel( this, "TextLabel5_2_2" );
    TextLabel5_2_2->setText( tr( "LyX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA." ) );
    TextLabel5_2_2->setFrameShape( QLabel::Box );
    TextLabel5_2_2->setFrameShadow( QLabel::Sunken );
    TextLabel5_2_2->setMargin( 6 );
    TextLabel5_2_2->setAlignment( int( QLabel::WordBreak | QLabel::AlignCenter ) );
    TextLabel5_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, TextLabel5_2_2->sizePolicy().hasHeightForWidth() ) );
    TextLabel5_2_2->setMinimumSize( QSize( 0, 0 ) );
    FormCopyrightDialogBaseLayout->addWidget( TextLabel5_2_2 );

    okPB = new QPushButton( this, "okPB" );
    okPB->setText( tr( "&OK" ) );
    okPB->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, okPB->sizePolicy().hasHeightForWidth() ) );
    okPB->setDefault( TRUE );
    FormCopyrightDialogBaseLayout->addWidget( okPB );

    // signals and slots connections
    connect( okPB, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
FormCopyrightDialogBase::~FormCopyrightDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

