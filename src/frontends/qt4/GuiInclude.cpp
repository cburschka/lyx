/**
 * \file GuiInclude.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/os.h"
#include "support/lstrings.h"

#include "GuiInclude.h"

#include "Qt2BC.h"
#include "qt_helpers.h"
#include "LyXRC.h"

#include "insets/InsetListingsParams.h"

#include <QPushButton>
#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>

using std::string;
using std::vector;

using lyx::support::os::internal_path;
using lyx::support::prefixIs;
using lyx::support::getStringFromVector;
using lyx::support::getVectorFromString;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiIncludeDialog
//
/////////////////////////////////////////////////////////////////////

GuiIncludeDialog::GuiIncludeDialog(GuiInclude * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));

	connect(visiblespaceCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(filenameED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(editPB, SIGNAL(clicked()), this, SLOT(editClicked()));
	connect(browsePB, SIGNAL(clicked()), this, SLOT(browseClicked()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
	connect(previewCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(captionLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(labelLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(listingsED, SIGNAL(textChanged()), this, SLOT(change_adaptor()));
	connect(listingsED, SIGNAL(textChanged()), this, SLOT(set_listings_msg()));
	connect(bypassCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(bypassCB, SIGNAL(clicked()), this, SLOT(set_listings_msg()));

	setFocusProxy(filenameED);
}


void GuiIncludeDialog::show()
{
	QDialog::show();
}


void GuiIncludeDialog::change_adaptor()
{
	form_->changed();
}


docstring GuiIncludeDialog::validate_listings_params()
{
	// use a cache here to avoid repeated validation
	// of the same parameters
	static string param_cache = string();
	static docstring msg_cache = docstring();
	
	if (typeCO->currentIndex() != 3 || bypassCB->isChecked())
		return docstring();

	string params = fromqstr(listingsED->toPlainText());
	if (params != param_cache) {
		param_cache = params;
		msg_cache = InsetListingsParams(params).validate();
	}
	return msg_cache;
}


void GuiIncludeDialog::set_listings_msg()
{
	static bool isOK = true;
	docstring msg = validate_listings_params();
	if (msg.empty()) {
		if (isOK)
			return;
		isOK = true;
		listingsTB->setPlainText(
			qt_("Input listing parameters on the right. Enter ? for a list of parameters."));
	} else {
		isOK = false;
		listingsTB->setPlainText(toqstr(msg));
	}
}


void GuiIncludeDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiIncludeDialog::typeChanged(int v)
{
	switch (v) {
		//case Include
		case 0:
			visiblespaceCB->setEnabled(false);
			visiblespaceCB->setChecked(false);
			previewCB->setEnabled(false);
			previewCB->setChecked(false);
			listingsGB->setEnabled(false);
			break;
		//case Input
		case 1:
			visiblespaceCB->setEnabled(false);
			visiblespaceCB->setChecked(false);
			previewCB->setEnabled(true);
			listingsGB->setEnabled(false);
			break;
		//case listings
		case 3:
			visiblespaceCB->setEnabled(false);
			visiblespaceCB->setChecked(false);
			previewCB->setEnabled(false);
			previewCB->setChecked(false);
			listingsGB->setEnabled(true);
			break;
		//case Verbatim
		default:
			visiblespaceCB->setEnabled(true);
			previewCB->setEnabled(false);
			previewCB->setChecked(false);
			listingsGB->setEnabled(false);
			break;
	}
	//see this thread 
	//  http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg118471.html
	//for the reason this is here.
	okPB->setDefault(true);
}


void GuiIncludeDialog::editClicked()
{
	form_->edit();
}


void GuiIncludeDialog::browseClicked()
{
	form_->browse();
}


/////////////////////////////////////////////////////////////////////
//
// GuiInclude
//
/////////////////////////////////////////////////////////////////////


GuiInclude::GuiInclude(Dialog & parent)
	: GuiView<GuiIncludeDialog>(parent, _("Child Document"))
{}


void GuiInclude::build_dialog()
{
	dialog_.reset(new GuiIncludeDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->filenameED);
	bcview().addReadOnly(dialog_->browsePB);
	bcview().addReadOnly(dialog_->visiblespaceCB);
	bcview().addReadOnly(dialog_->typeCO);
	bcview().addReadOnly(dialog_->listingsED);

	addCheckedLineEdit(bcview(), dialog_->filenameED, dialog_->filenameLA);
}


void GuiInclude::update_contents()
{
	InsetCommandParams const & params = controller().params();

	dialog_->filenameED->setText(toqstr(params["filename"]));

	dialog_->visiblespaceCB->setChecked(false);
	dialog_->visiblespaceCB->setEnabled(false);
	dialog_->previewCB->setChecked(false);
	dialog_->previewCB->setEnabled(false);
	dialog_->listingsGB->setEnabled(false);
	dialog_->captionLE->clear();
	dialog_->labelLE->clear();
	dialog_->listingsED->clear();
	dialog_->listingsTB->setPlainText(
		qt_("Input listing parameters on the right. Enter ? for a list of parameters."));

	string cmdname = controller().params().getCmdName();
	if (cmdname != "include" &&
	    cmdname != "verbatiminput" &&
	    cmdname != "verbatiminput*" &&
		cmdname != "lstinputlisting")
		cmdname = "input";

	if (cmdname == "include") {
		dialog_->typeCO->setCurrentIndex(0);

	} else if (cmdname == "input") {
		dialog_->typeCO->setCurrentIndex(1);
		dialog_->previewCB->setEnabled(true);
		dialog_->previewCB->setChecked(params.preview());

	} else if (cmdname == "verbatiminput*") {
		dialog_->typeCO->setCurrentIndex(2);
		dialog_->visiblespaceCB->setEnabled(true);
		dialog_->visiblespaceCB->setChecked(true);

	} else if (cmdname == "verbatiminput") {
		dialog_->typeCO->setCurrentIndex(2);
		dialog_->visiblespaceCB->setEnabled(true);

	} else if (cmdname == "lstinputlisting") {
		dialog_->typeCO->setCurrentIndex(3);
		dialog_->listingsGB->setEnabled(true);
		dialog_->listingsED->setEnabled(true);
		InsetListingsParams par(params.getOptions());
		// extract caption and label and put them into their respective editboxes
		vector<string> pars = getVectorFromString(par.separatedParams(), "\n");
		for (vector<string>::iterator it = pars.begin();
			it != pars.end(); ++it) {
			if (prefixIs(*it, "caption=")) {
				string cap = it->substr(8);
				if (cap[0] == '{' && cap[cap.size()-1] == '}') {
					dialog_->captionLE->setText(toqstr(cap.substr(1, cap.size()-2)));
					*it = "";
				} 
			} else if (prefixIs(*it, "label=")) {
				string lbl = it->substr(6);
				if (lbl[0] == '{' && lbl[lbl.size()-1] == '}') {
					dialog_->labelLE->setText(toqstr(lbl.substr(1, lbl.size()-2)));
					*it = "";
				}
			}
		}
		// the rest is put to the extra edit box.
		string extra = getStringFromVector(pars);
		dialog_->listingsED->setPlainText(toqstr(InsetListingsParams(extra).separatedParams()));
	}
}


void GuiInclude::apply()
{
	InsetCommandParams params = controller().params();

	params["filename"] = from_utf8(internal_path(fromqstr(dialog_->filenameED->text())));
	params.preview(dialog_->previewCB->isChecked());

	int const item = dialog_->typeCO->currentIndex();
	if (item == 0) {
		params.setCmdName("include");
	} else if (item == 1) {
		params.setCmdName("input");
	} else if (item == 3) {
		params.setCmdName("lstinputlisting");
		// the parameter string should have passed validation
		InsetListingsParams par(fromqstr(dialog_->listingsED->toPlainText()));
		string caption = fromqstr(dialog_->captionLE->text());
		string label = fromqstr(dialog_->labelLE->text());
		if (!caption.empty())
			par.addParam("caption", "{" + caption + "}");
		if (!label.empty())
			par.addParam("label", "{" + label + "}");
		params.setOptions(par.params());
	} else {
		if (dialog_->visiblespaceCB->isChecked())
			params.setCmdName("verbatiminput*");
		else
			params.setCmdName("verbatiminput");
	}
	controller().setParams(params);
}


void GuiInclude::browse()
{
	ControlInclude::Type type;

	int const item = dialog_->typeCO->currentIndex();
	if (item == 0)
		type = ControlInclude::INCLUDE;
	else if (item == 1)
		type = ControlInclude::INPUT;
	else if (item == 2)
		type = ControlInclude::VERBATIM;
	else
		type = ControlInclude::LISTINGS;

	docstring const & name =
		controller().browse(qstring_to_ucs4(dialog_->filenameED->text()), type);
	if (!name.empty())
		dialog_->filenameED->setText(toqstr(name));
}


void GuiInclude::edit()
{
	if (isValid()) {
		string const file = fromqstr(dialog_->filenameED->text());
		slotOK();
		controller().edit(file);
	}
}


bool GuiInclude::isValid()
{
	return !dialog_->filenameED->text().isEmpty() &&
		dialog_->validate_listings_params().empty();
}

} // namespace frontend
} // namespace lyx

#include "GuiInclude_moc.cpp"
