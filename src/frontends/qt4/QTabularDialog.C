/**
 * \file QTabularDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QTabularDialog.h"
#include "QTabular.h"
#include "validators.h"
#include "qt_helpers.h"

#include "controllers/ButtonController.h"
#include "controllers/ControlTabular.h"

#include <QCloseEvent>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>

using std::string;

namespace lyx {
namespace frontend {


QTabularDialog::QTabularDialog(QTabular * form)
	: form_(form)
{
	setupUi(this);

	widthED->setValidator(unsignedLengthValidator(widthED));
	topspaceED->setValidator(new LengthValidator(topspaceED));
	bottomspaceED->setValidator(new LengthValidator(bottomspaceED));
	interlinespaceED->setValidator(new LengthValidator(interlinespaceED));

	connect(topspaceED, SIGNAL(returnPressed()),
	        this, SLOT(topspace_changed()));
	connect(topspaceUnit, SIGNAL(selectionChanged(LyXLength::UNIT)),
	        this, SLOT(topspace_changed()));
	connect(topspaceCO, SIGNAL(activated(int)), this, SLOT(topspace_changed()));
	connect(bottomspaceED, SIGNAL(returnPressed()),
	        this, SLOT(bottomspace_changed()));
	connect(bottomspaceUnit, SIGNAL(selectionChanged(LyXLength::UNIT)),
	        this, SLOT(bottomspace_changed()));
	connect(bottomspaceCO, SIGNAL(activated(int)), this, SLOT(bottomspace_changed()));
	connect(interlinespaceED, SIGNAL(returnPressed()),
	        this, SLOT(interlinespace_changed()));
	connect(interlinespaceUnit, SIGNAL(selectionChanged(LyXLength::UNIT)),
	        this, SLOT(interlinespace_changed()));
	connect(interlinespaceCO, SIGNAL(activated(int)), this, SLOT(interlinespace_changed()));
	connect(booktabsRB, SIGNAL(clicked()), this, SLOT(on_booktabsRB_toggled()));
	connect(borderSetPB, SIGNAL(clicked()), this, SLOT(borderSet_clicked()));
	connect(borderUnsetPB, SIGNAL(clicked()), this, SLOT(borderUnset_clicked()));
	connect(longTabularCB, SIGNAL(toggled(bool)), longtableGB, SLOT(setEnabled(bool)));
	connect(longTabularCB, SIGNAL(toggled(bool)), newpageCB, SLOT(setEnabled(bool)));
	connect(hAlignCB, SIGNAL(activated(int)), this, SLOT(hAlign_changed(int)));
	connect(vAlignCB, SIGNAL(activated(int)), this, SLOT(vAlign_changed(int)));
	connect(multicolumnCB, SIGNAL(clicked()), this, SLOT(multicolumn_clicked()));
	connect(newpageCB, SIGNAL(clicked()), this, SLOT(ltNewpage_clicked()));
	connect(headerStatusCB, SIGNAL(clicked()), this, SLOT(ltHeaderStatus_clicked()));
	connect(headerBorderAboveCB, SIGNAL(clicked()), this, SLOT(ltHeaderBorderAbove_clicked()));
	connect(headerBorderBelowCB, SIGNAL(clicked()), this, SLOT(ltHeaderBorderBelow_clicked()));
	connect(firstheaderStatusCB, SIGNAL(clicked()), this, SLOT(ltFirstHeaderStatus_clicked()));
	connect(firstheaderBorderAboveCB, SIGNAL(clicked()), this, SLOT(ltFirstHeaderBorderAbove_clicked()));
	connect(firstheaderBorderBelowCB, SIGNAL(clicked()), this, SLOT(ltFirstHeaderBorderBelow_clicked()));
	connect(firstheaderNoContentsCB, SIGNAL(clicked()), this, SLOT(ltFirstHeaderEmpty_clicked()));
	connect(footerStatusCB, SIGNAL(clicked()), this, SLOT(ltFooterStatus_clicked()));
	connect(footerBorderAboveCB, SIGNAL(clicked()), this, SLOT(ltFooterBorderAbove_clicked()));
	connect(footerBorderBelowCB, SIGNAL(clicked()), this, SLOT(ltFooterBorderBelow_clicked()));
	connect(lastfooterStatusCB, SIGNAL(clicked()), this, SLOT(ltLastFooterStatus_clicked()));
	connect(lastfooterBorderAboveCB, SIGNAL(clicked()), this, SLOT(ltLastFooterBorderAbove_clicked()));
	connect(lastfooterBorderBelowCB, SIGNAL(clicked()), this, SLOT(ltLastFooterBorderBelow_clicked()));
	connect(lastfooterNoContentsCB, SIGNAL(clicked()), this, SLOT(ltLastFooterEmpty_clicked()));
	connect(specialAlignmentED, SIGNAL(returnPressed()), this, SLOT(specialAlignment_changed()));
	connect(widthED, SIGNAL(returnPressed()), this, SLOT(width_changed()));
	connect(widthUnit, SIGNAL(selectionChanged(LyXLength::UNIT)), this, SLOT(width_changed()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(close_clicked()));
	connect(borders, SIGNAL(topSet(bool)), this, SLOT(topBorder_changed()));
	connect(borders, SIGNAL(bottomSet(bool)), this, SLOT(bottomBorder_changed()));
	connect(borders, SIGNAL(rightSet(bool)), this, SLOT(rightBorder_changed()));
	connect(borders, SIGNAL(leftSet(bool)), this, SLOT(leftBorder_changed()));
	connect(rotateTabularCB, SIGNAL(clicked()), this, SLOT(rotateTabular()));
	connect(rotateCellCB, SIGNAL(clicked()), this, SLOT(rotateCell()));
	connect(longTabularCB, SIGNAL(clicked()), this, SLOT(longTabular()));
}


void QTabularDialog::change_adaptor()
{
	form_->changed();
}


void QTabularDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QTabularDialog::on_booktabsRB_toggled()
{
	form_->changed();
	form_->controller().booktabs(booktabsRB->isChecked());
	form_->update_borders();
}


void QTabularDialog::topspace_changed()
{
	switch(topspaceCO->currentItem()) {
		case 0: {
			form_->controller().set(LyXTabular::SET_TOP_SPACE, "");
				topspaceED->setEnabled(false);
				topspaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			form_->controller().set(LyXTabular::SET_TOP_SPACE, "default");
			topspaceED->setEnabled(false);
			topspaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!topspaceED->text().isEmpty())
				form_->controller().set(LyXTabular::SET_TOP_SPACE,
					widgetsToLength(topspaceED, topspaceUnit));
			if (!form_->bc().bp().isReadOnly()) {
				topspaceED->setEnabled(true);
				topspaceUnit->setEnabled(true);
			}
			break;
		}
	}
	form_->changed();
}


void QTabularDialog::bottomspace_changed()
{
	switch(bottomspaceCO->currentItem()) {
		case 0: {
			form_->controller().set(LyXTabular::SET_BOTTOM_SPACE, "");
				bottomspaceED->setEnabled(false);
				bottomspaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			form_->controller().set(LyXTabular::SET_BOTTOM_SPACE, "default");
			bottomspaceED->setEnabled(false);
			bottomspaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!bottomspaceED->text().isEmpty())
				form_->controller().set(LyXTabular::SET_BOTTOM_SPACE,
					widgetsToLength(bottomspaceED, bottomspaceUnit));
			if (!form_->bc().bp().isReadOnly()) {
				bottomspaceED->setEnabled(true);
				bottomspaceUnit->setEnabled(true);
			}
			break;
		}
	}
	form_->changed();
}


void QTabularDialog::interlinespace_changed()
{
	switch(interlinespaceCO->currentItem()) {
		case 0: {
			form_->controller().set(LyXTabular::SET_INTERLINE_SPACE, "");
				interlinespaceED->setEnabled(false);
				interlinespaceUnit->setEnabled(false);
			break;
		}
		case 1: {
			form_->controller().set(LyXTabular::SET_INTERLINE_SPACE, "default");
			interlinespaceED->setEnabled(false);
			interlinespaceUnit->setEnabled(false);
			break;
		}
		case 2: {
			if (!interlinespaceED->text().isEmpty())
				form_->controller().set(LyXTabular::SET_INTERLINE_SPACE,
					widgetsToLength(interlinespaceED, interlinespaceUnit));
			if (!form_->bc().bp().isReadOnly()) {
				interlinespaceED->setEnabled(true);
				interlinespaceUnit->setEnabled(true);
			}
			break;
		}
	}
	form_->changed();
}


void QTabularDialog::close_clicked()
{
	form_->closeGUI();
	form_->slotClose();
}


void QTabularDialog::borderSet_clicked()
{
	form_->controller().set(LyXTabular::SET_ALL_LINES);
	form_->update_borders();
	form_->changed();
}


void QTabularDialog::borderUnset_clicked()
{
	form_->controller().set(LyXTabular::UNSET_ALL_LINES);
	form_->update_borders();
	form_->changed();
}


void QTabularDialog::leftBorder_changed()
{
	form_->controller().toggleLeftLine();
	form_->changed();
}


void QTabularDialog::rightBorder_changed()
{
	form_->controller().toggleRightLine();
	form_->changed();
}


void QTabularDialog::topBorder_changed()
{
	form_->controller().toggleTopLine();
	form_->changed();
}


void QTabularDialog::bottomBorder_changed()
{
	form_->controller().toggleBottomLine();
	form_->changed();
}


void QTabularDialog::specialAlignment_changed()
{
	string special = fromqstr(specialAlignmentED->text());
	form_->controller().setSpecial(special);
	form_->changed();
}


void QTabularDialog::width_changed()
{
	form_->changed();
	string const width = widgetsToLength(widthED, widthUnit);
	form_->controller().setWidth(width);
}


void QTabularDialog::multicolumn_clicked()
{
	form_->controller().toggleMultiColumn();
	form_->changed();
}


void QTabularDialog::rotateTabular()
{
	form_->controller().rotateTabular(rotateTabularCB->isChecked());
	form_->changed();
}


void QTabularDialog::rotateCell()
{
	form_->controller().rotateCell(rotateCellCB->isChecked());
	form_->changed();
}


void QTabularDialog::hAlign_changed(int align)
{
	ControlTabular::HALIGN h = ControlTabular::LEFT;

	switch (align) {
		case 0: h = ControlTabular::LEFT; break;
		case 1: h = ControlTabular::CENTER; break;
		case 2: h = ControlTabular::RIGHT; break;
		case 3: h = ControlTabular::BLOCK; break;
	}

	form_->controller().halign(h);
}


void QTabularDialog::vAlign_changed(int align)
{
	ControlTabular::VALIGN v = ControlTabular::TOP;

	switch (align) {
		case 0: v = ControlTabular::TOP; break;
		case 1: v = ControlTabular::MIDDLE; break;
		case 2: v = ControlTabular::BOTTOM; break;
	}

	form_->controller().valign(v);
}


void QTabularDialog::longTabular()
{
	form_->controller().longTabular(longTabularCB->isChecked());
	form_->changed();
}


void QTabularDialog::ltNewpage_clicked()
{
	form_->controller().set(LyXTabular::SET_LTNEWPAGE);
	form_->changed();
}


void QTabularDialog::ltHeaderStatus_clicked()
{
	bool enable(headerStatusCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTHEAD, "");
	else
		form_->controller().set(LyXTabular::UNSET_LTHEAD, "");
	headerBorderAboveCB->setEnabled(enable);
	headerBorderBelowCB->setEnabled(enable);
	firstheaderNoContentsCB->setEnabled(enable);
	form_->changed();
}


void QTabularDialog::ltHeaderBorderAbove_clicked()
{
	if (headerBorderAboveCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTHEAD, "dl_above");
	else
		form_->controller().set(LyXTabular::UNSET_LTHEAD, "dl_above");
	form_->changed();
}


void QTabularDialog::ltHeaderBorderBelow_clicked()
{
	if (headerBorderBelowCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTHEAD, "dl_below");
	else
		form_->controller().set(LyXTabular::UNSET_LTHEAD, "dl_below");
	form_->changed();
}


void QTabularDialog::ltFirstHeaderBorderAbove_clicked()
{
	if (firstheaderBorderAboveCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTFIRSTHEAD, "dl_above");
	else
		form_->controller().set(LyXTabular::UNSET_LTFIRSTHEAD, "dl_above");
	form_->changed();
}


void QTabularDialog::ltFirstHeaderBorderBelow_clicked()
{
	if (firstheaderBorderBelowCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTFIRSTHEAD, "dl_below");
	else
		form_->controller().set(LyXTabular::UNSET_LTFIRSTHEAD, "dl_below");
	form_->changed();
}


void QTabularDialog::ltFirstHeaderStatus_clicked()
{
	bool enable(firstheaderStatusCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTFIRSTHEAD, "");
	else
		form_->controller().set(LyXTabular::UNSET_LTFIRSTHEAD, "");
	firstheaderBorderAboveCB->setEnabled(enable);
	firstheaderBorderBelowCB->setEnabled(enable);
	form_->changed();
}


void QTabularDialog::ltFirstHeaderEmpty_clicked()
{
	bool enable(firstheaderNoContentsCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTFIRSTHEAD, "empty");
	else
		form_->controller().set(LyXTabular::UNSET_LTFIRSTHEAD, "empty");
	firstheaderStatusCB->setEnabled(!enable);
	firstheaderBorderAboveCB->setEnabled(!enable);
	firstheaderBorderBelowCB->setEnabled(!enable);
	form_->changed();
}


void QTabularDialog::ltFooterStatus_clicked()
{
	bool enable(footerStatusCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTFOOT, "");
	else
		form_->controller().set(LyXTabular::UNSET_LTFOOT, "");
	footerBorderAboveCB->setEnabled(enable);
	footerBorderBelowCB->setEnabled(enable);
	lastfooterNoContentsCB->setEnabled(enable);
	form_->changed();
}


void QTabularDialog::ltFooterBorderAbove_clicked()
{
	if (footerBorderAboveCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTFOOT, "dl_above");
	else
		form_->controller().set(LyXTabular::UNSET_LTFOOT, "dl_above");
	form_->changed();
}


void QTabularDialog::ltFooterBorderBelow_clicked()
{
	if (footerBorderBelowCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTFOOT, "dl_below");
	else
		form_->controller().set(LyXTabular::UNSET_LTFOOT, "dl_below");
	form_->changed();
}


void QTabularDialog::ltLastFooterStatus_clicked()
{
	bool enable(lastfooterStatusCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTLASTFOOT, "");
	else
		form_->controller().set(LyXTabular::UNSET_LTLASTFOOT, "");
	lastfooterBorderAboveCB->setEnabled(enable);
	lastfooterBorderBelowCB->setEnabled(enable);
	form_->changed();
}


void QTabularDialog::ltLastFooterBorderAbove_clicked()
{
	if (lastfooterBorderAboveCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTLASTFOOT, "dl_above");
	else
		form_->controller().set(LyXTabular::UNSET_LTLASTFOOT, "dl_above");
	form_->changed();
}


void QTabularDialog::ltLastFooterBorderBelow_clicked()
{
	if (lastfooterBorderBelowCB->isChecked())
		form_->controller().set(LyXTabular::SET_LTLASTFOOT, "dl_below");
	else
		form_->controller().set(LyXTabular::UNSET_LTLASTFOOT, "dl_below");
	form_->changed();
}


void QTabularDialog::ltLastFooterEmpty_clicked()
{
	bool enable(lastfooterNoContentsCB->isChecked());
	if (enable)
		form_->controller().set(LyXTabular::SET_LTLASTFOOT, "empty");
	else
		form_->controller().set(LyXTabular::UNSET_LTLASTFOOT, "empty");
	lastfooterStatusCB->setEnabled(!enable);
	lastfooterBorderAboveCB->setEnabled(!enable);
	lastfooterBorderBelowCB->setEnabled(!enable);
	form_->changed();
}

} // namespace frontend
} // namespace lyx

#include "QTabularDialog_moc.cpp"
