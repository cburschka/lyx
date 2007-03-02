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

#include "lengthcombo.h"
#include "validators.h"
#include "qt_helpers.h"

#include "debug.h"

#include "controllers/ControlGraphics.h"

#include "insets/insetgraphicsParams.h"

#include <QCloseEvent>
#include <QPushButton>
#include <QLineEdit>
#include <QValidator>


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
	connect(filename, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_adaptor() ) );
	connect(subcaption, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_adaptor() ) );

	// FIXME: we should connect to clicked() when we move to Qt 4.2	because
	// the toggled(bool) signal is also trigged when we update the widgets
	connect(subfigure, SIGNAL( toggled(bool) ),
		this, SLOT( change_adaptor() ) );
	connect(displayGB, SIGNAL( toggled(bool) ),
		this, SLOT( change_adaptor() ) );

	connect(latexoptions, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_adaptor() ) );
	connect(clip, SIGNAL( stateChanged(int) ),
		this, SLOT( change_adaptor() ) );
	connect(showCB, SIGNAL( currentIndexChanged(int) ),
		this, SLOT( change_adaptor() ) );
	connect(displayscale, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_adaptor() ) );
	connect(Width, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_adaptor() ) );
	connect(aspectratio, SIGNAL( stateChanged(int) ),
		this, SLOT( change_adaptor() ) );
	connect(draftCB, SIGNAL( stateChanged(int) ),
		this, SLOT( change_adaptor() ) );
	connect(unzipCB, SIGNAL( stateChanged(int) ),
		this, SLOT( change_adaptor() ) );
	connect(Height, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_adaptor() ) );
	connect(heightUnit, SIGNAL( selectionChanged(lyx::LyXLength::UNIT) ),
		this, SLOT( change_adaptor() ) );
	connect(widthUnit, SIGNAL( selectionChanged(lyx::LyXLength::UNIT) ),
		this, SLOT( change_adaptor() ) );
	connect(angle, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_adaptor() ) );
	connect(origin, SIGNAL( activated(int) ),
		this, SLOT( change_adaptor() ) );
	connect(getPB, SIGNAL( clicked() ),
		this, SLOT( change_adaptor() ) );
	connect(scaleCB, SIGNAL(clicked() ),
		this, SLOT(change_adaptor()) );
	connect(Scale, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_adaptor() ) );

	connect(lbY, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_bb() ) );
	connect(lbYunit, SIGNAL( activated(int) ),
		this, SLOT( change_bb() ) );
	connect(rtY, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_bb() ) );
	connect(rtYunit, SIGNAL( activated(int) ),
		this, SLOT( change_bb() ) );

	connect(lbX, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_bb() ) );
	connect(lbXunit, SIGNAL( activated(int) ),
		this, SLOT( change_bb() ) );
	connect(rtX, SIGNAL( textChanged(const QString&) ),
		this, SLOT( change_bb() ) );
	connect(rtXunit, SIGNAL( activated(int) ),
		this, SLOT( change_bb() ) );

	angle->setValidator(new QDoubleValidator(-360, 360, 2, angle));

	lbX->setValidator(new QDoubleValidator(lbX));
	lbY->setValidator(new QDoubleValidator(lbY));
	rtX->setValidator(new QDoubleValidator(rtX));
	rtY->setValidator(new QDoubleValidator(rtY));

	displayscale->setValidator(new QIntValidator(displayscale));
	Height->setValidator(unsignedLengthValidator(Height));
	Width->setValidator(unsignedLengthValidator(Width));

	filename->setValidator(new PathValidator(true, filename));
}


void QGraphicsDialog::show()
{
	QDialog::show();
	if (form_->controller().params().filename.empty())
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


void QGraphicsDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QGraphicsDialog::on_browsePB_clicked()
{
	docstring const str =
		form_->controller().browse(qstring_to_ucs4(filename->text()));
	filename->setText(toqstr(str));
	form_->changed();
}


void QGraphicsDialog::on_getPB_clicked()
{
	form_->getBB();
}


void QGraphicsDialog::on_editPB_clicked()
{
	form_->controller().editGraphics();
}


void QGraphicsDialog::on_filename_textChanged(const QString & filename)
{
	editPB->setDisabled(filename.isEmpty());
}


void QGraphicsDialog::on_scaleCB_toggled(bool setscale)
{
	Scale->setEnabled(setscale);
	widthL->setDisabled(setscale);
	Width->setDisabled(setscale);
	widthUnit->setDisabled(setscale);
	aspectratio->setDisabled(setscale);
	bool noheight = setscale || aspectratio->checkState()==Qt::Checked;
	heightL->setDisabled(noheight);
	Height->setDisabled(noheight);
	heightUnit->setDisabled(noheight);
}

} // namespace frontend
} // namespace lyx

#include "QGraphicsDialog_moc.cpp"
