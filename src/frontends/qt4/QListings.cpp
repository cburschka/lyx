/**
 * \file QListings.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QListings.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "controllers/ControlListings.h"
#include "insets/InsetListingsParams.h"
#include "debug.h"

#include "support/convert.h"
#include "support/lstrings.h"

#include <QLineEdit>
#include <QCloseEvent>
#include <QPushButton>


using std::string;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QListingsDialog
//
/////////////////////////////////////////////////////////////////////


QListingsDialog::QListingsDialog(QListings * form)
	: form_(form)
{
	setupUi(this);
	
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	connect(inlineCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(listingsED,  SIGNAL(textChanged()), this, SLOT(change_adaptor()));
	connect(listingsED,  SIGNAL(textChanged()), this, SLOT(validate_listings_params()));
}


void QListingsDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QListingsDialog::change_adaptor()
{
	form_->changed();
}


void QListingsDialog::validate_listings_params()
{
	static bool isOK = true;
	try {
		InsetListingsParams par(fromqstr(listingsED->toPlainText()));
		if (!isOK) {
			isOK = true;
			// listingsTB->setTextColor("black");
			listingsTB->setPlainText("Input listings parameters below. Enter ? for a list of parameters.");
			okPB->setEnabled(true);
		}
	} catch (invalidParam & e) {
		isOK = false;
		// listingsTB->setTextColor("red");
		listingsTB->setPlainText(e.what());
		okPB->setEnabled(false);
	}
}

/////////////////////////////////////////////////////////////////////
//
// QListings
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlListings, QView<QListingsDialog> > wrap_base_class;

QListings::QListings(Dialog & parent)
	: wrap_base_class(parent, _("Program Listings Settings"))
{
}


void QListings::build_dialog()
{
	dialog_.reset(new QListingsDialog(this));
	
	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	dialog_->listingsTB->setPlainText("Input listings parameters below. Enter ? for a list of parameters.");

	update_contents();
}


/// not used right now.
void QListings::apply()
{
	InsetListingsParams & params = controller().params();
	params.setInline(dialog_->inlineCB->isChecked());
	params.setParams(fromqstr(dialog_->listingsED->toPlainText()));
	controller().setParams(params);
}


void QListings::update_contents()
{
	InsetListingsParams & params = controller().params();
	dialog_->listingsED->setText(toqstr(params.separatedParams()));
	
	if (params.isInline())
		dialog_->inlineCB->setChecked(true);
	else
		dialog_->inlineCB->setChecked(false);
}


} // namespace frontend
} // namespace lyx


#include "QListings_moc.cpp"
