/**
 * \file QGraphicsDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voﬂ
 * \author Abdelrazak Younes
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QGraphicsDialog.h"
#include "QGraphics.h"

#include "LengthCombo.h"
#include "Validator.h"
#include "qt_helpers.h"

#include "debug.h"

#include "controllers/ControlGraphics.h"

#include "insets/InsetGraphicsParams.h"

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
	//main buttons
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));

	//graphics pane
	connect(filename, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(WidthCB, SIGNAL( clicked()),
		this, SLOT(change_adaptor()));
	connect(HeightCB, SIGNAL( clicked()),
		this, SLOT(change_adaptor()));
	connect(Width, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(Height, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(heightUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(widthUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(aspectratio, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(angle, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(origin, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(scaleCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(Scale, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(rotateOrderCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	filename->setValidator(new PathValidator(true, filename));
	setFocusProxy(filename);

	QDoubleValidator * scaleValidator = new DoubleAutoValidator(Scale);
	scaleValidator->setBottom(0);
	scaleValidator->setDecimals(256); //I guess that will do
	Scale->setValidator(scaleValidator);
	Height->setValidator(unsignedLengthAutoValidator(Height));
	Width->setValidator(unsignedLengthAutoValidator(Width));
	angle->setValidator(new QDoubleValidator(-360, 360, 2, angle));

	//clipping pane
	connect(clip, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(lbY, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_bb()));
	connect(lbYunit, SIGNAL(activated(int)),
		this, SLOT(change_bb()));
	connect(rtY, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_bb()));
	connect(rtYunit, SIGNAL(activated(int)),
		this, SLOT(change_bb()));
	connect(lbX, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_bb()));
	connect(lbXunit, SIGNAL(activated(int)),
		this, SLOT(change_bb()));
	connect(rtX, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_bb()));
	connect(rtXunit, SIGNAL(activated(int)),
		this, SLOT(change_bb()));
	connect(getPB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	lbX->setValidator(new QDoubleValidator(lbX));
	lbY->setValidator(new QDoubleValidator(lbY));
	rtX->setValidator(new QDoubleValidator(rtX));
	rtY->setValidator(new QDoubleValidator(rtY));

	//extra options pane
	connect(latexoptions, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(draftCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(unzipCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	// FIXME: we should connect to clicked() when we move to Qt 4.2	because
	// the toggled(bool) signal is also trigged when we update the widgets
	// (rgh-4/07) this isn't as much or a problem as it was, because we're now
	// using blockSignals() to keep from triggering that signal when we call
	// setChecked(). Note, too, that clicked() would get called whenever it
	// is clicked, even right clicked (I think), not just whenever it is
	// toggled.
	connect(subfigure, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(subcaption, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(displayGB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(showCB, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));
	connect(displayscale, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	displayscale->setValidator(new QIntValidator(displayscale));
}


void QGraphicsDialog::show()
{
	QDialog::show();
}


void QGraphicsDialog::change_adaptor()
{
	form_->changed();
}


void QGraphicsDialog::change_bb()
{
	form_->controller().bbChanged = true;
	LYXERR(Debug::GRAPHICS)
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
	if(!str.empty()){
		filename->setText(toqstr(str));
		form_->changed();
	}
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


void QGraphicsDialog::setAutoText()
{
	if (scaleCB->isChecked()) return;
	if (!Scale->isEnabled() && Scale->text() != "100")
		Scale->setText(QString("auto"));

	setAutoTextCB(WidthCB, Width, widthUnit);
	setAutoTextCB(HeightCB, Height, heightUnit);
}


void QGraphicsDialog::on_scaleCB_toggled(bool setScale)
{
	Scale->setEnabled(setScale);
	if (setScale) {
		Scale->setText("100");
		Scale->setFocus(Qt::OtherFocusReason);
	}

	WidthCB->setDisabled(setScale);
	WidthCB->blockSignals(true);
	WidthCB->setChecked(false);
	WidthCB->blockSignals(false);
	Width->setEnabled(false);
	widthUnit->setEnabled(false);

	HeightCB->setDisabled(setScale);
	HeightCB->blockSignals(true);
	HeightCB->setChecked(false);
	HeightCB->blockSignals(false);
	Height->setEnabled(false);
	heightUnit->setEnabled(false);

	aspectratio->setDisabled(true);
	aspectratio->setChecked(true);

	rotateOrderCB->setEnabled((WidthCB->isChecked() ||
				 HeightCB->isChecked() ||
				 scaleCB->isChecked()) &&
				 (angle->text() != "0"));

	setAutoText();
}

void QGraphicsDialog::on_WidthCB_toggled(bool setWidth)
{
	Width->setEnabled(setWidth);
	widthUnit->setEnabled(setWidth);
	if (setWidth)
		Width->setFocus(Qt::OtherFocusReason);

	bool const setHeight = HeightCB->isChecked();
	aspectratio->setEnabled(setWidth && setHeight);
	aspectratio->blockSignals(true);
	aspectratio->setChecked(!(setWidth && setHeight));
	aspectratio->blockSignals(false);

	scaleCB->setEnabled(!setWidth && !setHeight);
	//already will be unchecked, so don't need to do that
	Scale->setEnabled((!setWidth && !setHeight) //=scaleCB->isEnabled()
			&& scaleCB->isChecked()); //should be false, but let's check
	rotateOrderCB->setEnabled((setWidth || setHeight ||
				 scaleCB->isChecked()) &&
				 (angle->text() != "0"));

	setAutoText();
}

void QGraphicsDialog::on_HeightCB_toggled(bool setHeight)
{
	Height->setEnabled(setHeight);
	heightUnit->setEnabled(setHeight);
	if (setHeight)
		Height->setFocus(Qt::OtherFocusReason);

	bool const setWidth = WidthCB->isChecked();
	aspectratio->setEnabled(setWidth && setHeight);
	aspectratio->blockSignals(true);
	aspectratio->setChecked(!(setWidth && setHeight));
	aspectratio->blockSignals(false);

	scaleCB->setEnabled(!setWidth && !setHeight);
	//already unchecked
	Scale->setEnabled((!setWidth && !setHeight) //=scaleCB->isEnabled()
		&& scaleCB->isChecked()); //should be false
	rotateOrderCB->setEnabled((setWidth || setHeight ||
				 scaleCB->isChecked()) &&
				 (angle->text() != "0"));

	setAutoText();
}


void QGraphicsDialog::on_angle_textChanged(const QString & filename)
{
	rotateOrderCB->setEnabled((WidthCB->isChecked() ||
				 HeightCB->isChecked() ||
				 scaleCB->isChecked()) &&
				 (filename != "0"));
}


} // namespace frontend
} // namespace lyx

#include "QGraphicsDialog_moc.cpp"
