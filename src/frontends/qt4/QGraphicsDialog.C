/**
 * \file QGraphicsDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voﬂ
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QGraphicsDialog.h"
#include "QGraphics.h"
//Added by qt3to4:
#include <QCloseEvent>

#include "lengthcombo.h"
#include "validators.h"
#include "qt_helpers.h"

#include "debug.h"

#include "controllers/ControlGraphics.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qvalidator.h>


using std::string;

namespace lyx {
namespace frontend {


QGraphicsDialog::QGraphicsDialog(QGraphics * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	connect(editPB, SIGNAL(clicked()),
		this, SLOT(edit_clicked()));

    connect( subfigure, SIGNAL( toggled(bool) ), subcaption, SLOT( setEnabled(bool) ) );
    connect( browsePB, SIGNAL( clicked() ), this, SLOT( browse_clicked() ) );
    connect( filename, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( subcaption, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( subfigure, SIGNAL( stateChanged(int) ), this, SLOT( change_adaptor() ) );
    connect( latexoptions, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( clip, SIGNAL( stateChanged(int) ), this, SLOT( change_adaptor() ) );
    connect( lbX, SIGNAL( textChanged(const QString&) ), this, SLOT( change_bb() ) );
    connect( showCB, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( displayscale, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( Width, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( widthUnit, SIGNAL( activated(int) ), this, SLOT( change_WUnit() ) );
    connect( aspectratio, SIGNAL( stateChanged(int) ), this, SLOT( change_adaptor() ) );
    connect( displayCB, SIGNAL( stateChanged(int) ), this, SLOT( change_adaptor() ) );
    connect( draftCB, SIGNAL( stateChanged(int) ), this, SLOT( change_adaptor() ) );
    connect( unzipCB, SIGNAL( stateChanged(int) ), this, SLOT( change_adaptor() ) );
    connect( displayCB, SIGNAL( toggled(bool) ), showCB, SLOT( setEnabled(bool) ) );
    connect( displayCB, SIGNAL( toggled(bool) ), displayscale, SLOT( setEnabled(bool) ) );
    connect( Height, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( heightUnit, SIGNAL( selectionChanged(LyXLength::UNIT) ), this, SLOT( change_adaptor() ) );
    connect( angle, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( origin, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( getPB, SIGNAL( clicked() ), this, SLOT( getBB_clicked() ) );
    connect( getPB, SIGNAL( clicked() ), this, SLOT( change_adaptor() ) );
    connect( lbY, SIGNAL( textChanged(const QString&) ), this, SLOT( change_bb() ) );
    connect( rtX, SIGNAL( textChanged(const QString&) ), this, SLOT( change_bb() ) );
    connect( rtY, SIGNAL( textChanged(const QString&) ), this, SLOT( change_bb() ) );
    connect( lbXunit, SIGNAL( activated(int) ), this, SLOT( change_bb() ) );
    connect( lbYunit, SIGNAL( activated(int) ), this, SLOT( change_bb() ) );
    connect( rtXunit, SIGNAL( activated(int) ), this, SLOT( change_bb() ) );
    connect( rtYunit, SIGNAL( activated(int) ), this, SLOT( change_bb() ) );

	angle->setValidator(new QDoubleValidator(-360, 360, 2, angle));

	lbX->setValidator(new QIntValidator(lbX));
	lbY->setValidator(new QIntValidator(lbY));
	rtX->setValidator(new QIntValidator(rtX));
	rtY->setValidator(new QIntValidator(rtY));

	displayscale->setValidator(new QIntValidator(displayscale));
	Height->setValidator(unsignedLengthValidator(Height));
	Width->setValidator(unsignedLengthValidator(Width));

	filename->setValidator(new PathValidator(true, filename));
}


void QGraphicsDialog::show()
{
	QDialog::show();
	filename->setFocus();
}


void QGraphicsDialog::change_adaptor()
{
	form_->changed();
}


void QGraphicsDialog::change_bb()
{
	form_->controller().bbChanged = true;
	lyxerr[Debug::GRAPHICS]
		<< "[controller().bb_Changed set to true]\n";
	form_->changed();
}


void QGraphicsDialog::change_WUnit()
{
	bool useHeight = (widthUnit->currentItem() > 0);
	Height->setEnabled(useHeight);
	heightUnit->setEnabled(useHeight);
	form_->changed();
}


void QGraphicsDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QGraphicsDialog::browse_clicked()
{
	string const str =
		form_->controller().browse(fromqstr(filename->text()));
	filename->setText(toqstr(str));
	form_->changed();
}


void QGraphicsDialog::getBB_clicked()
{
	form_->getBB();
}


void QGraphicsDialog::edit_clicked()
{
	form_->controller().editGraphics();
}

} // namespace frontend
} // namespace lyx
