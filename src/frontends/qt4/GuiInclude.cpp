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

#include "GuiInclude.h"
#include "ControlInclude.h"

#include "support/os.h"
#include "support/lstrings.h"

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

GuiIncludeDialog::GuiIncludeDialog(LyXView & lv)
	: GuiDialog(lv, "include")
{
	setupUi(this);
	setViewTitle(_("Child Document"));
	setController(new ControlInclude(*this));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

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

	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(filenameED);
	bc().addReadOnly(browsePB);
	bc().addReadOnly(visiblespaceCB);
	bc().addReadOnly(typeCO);
	bc().addReadOnly(listingsED);

	bc().addCheckedLineEdit(filenameED, filenameLA);
}


ControlInclude & GuiIncludeDialog::controller()
{
	return static_cast<ControlInclude &>(GuiDialog::controller());
}


void GuiIncludeDialog::change_adaptor()
{
	changed();
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
	slotClose();
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
	edit();
}


void GuiIncludeDialog::browseClicked()
{
	browse();
}


void GuiIncludeDialog::updateContents()
{
	InsetCommandParams const & params = controller().params();

	filenameED->setText(toqstr(params["filename"]));

	visiblespaceCB->setChecked(false);
	visiblespaceCB->setEnabled(false);
	previewCB->setChecked(false);
	previewCB->setEnabled(false);
	listingsGB->setEnabled(false);
	captionLE->clear();
	labelLE->clear();
	listingsED->clear();
	listingsTB->setPlainText(
		qt_("Input listing parameters on the right. Enter ? for a list of parameters."));

	string cmdname = controller().params().getCmdName();
	if (cmdname != "include" &&
	    cmdname != "verbatiminput" &&
	    cmdname != "verbatiminput*" &&
		cmdname != "lstinputlisting")
		cmdname = "input";

	if (cmdname == "include") {
		typeCO->setCurrentIndex(0);

	} else if (cmdname == "input") {
		typeCO->setCurrentIndex(1);
		previewCB->setEnabled(true);
		previewCB->setChecked(params.preview());

	} else if (cmdname == "verbatiminput*") {
		typeCO->setCurrentIndex(2);
		visiblespaceCB->setEnabled(true);
		visiblespaceCB->setChecked(true);

	} else if (cmdname == "verbatiminput") {
		typeCO->setCurrentIndex(2);
		visiblespaceCB->setEnabled(true);

	} else if (cmdname == "lstinputlisting") {
		typeCO->setCurrentIndex(3);
		listingsGB->setEnabled(true);
		listingsED->setEnabled(true);
		InsetListingsParams par(params.getOptions());
		// extract caption and label and put them into their respective editboxes
		vector<string> pars = getVectorFromString(par.separatedParams(), "\n");
		for (vector<string>::iterator it = pars.begin();
			it != pars.end(); ++it) {
			if (prefixIs(*it, "caption=")) {
				string cap = it->substr(8);
				if (cap[0] == '{' && cap[cap.size()-1] == '}') {
					captionLE->setText(toqstr(cap.substr(1, cap.size()-2)));
					*it = "";
				} 
			} else if (prefixIs(*it, "label=")) {
				string lbl = it->substr(6);
				if (lbl[0] == '{' && lbl[lbl.size()-1] == '}') {
					labelLE->setText(toqstr(lbl.substr(1, lbl.size()-2)));
					*it = "";
				}
			}
		}
		// the rest is put to the extra edit box.
		string extra = getStringFromVector(pars);
		listingsED->setPlainText(toqstr(InsetListingsParams(extra).separatedParams()));
	}
}


void GuiIncludeDialog::applyView()
{
	InsetCommandParams params = controller().params();

	params["filename"] = from_utf8(internal_path(fromqstr(filenameED->text())));
	params.preview(previewCB->isChecked());

	int const item = typeCO->currentIndex();
	if (item == 0) {
		params.setCmdName("include");
	} else if (item == 1) {
		params.setCmdName("input");
	} else if (item == 3) {
		params.setCmdName("lstinputlisting");
		// the parameter string should have passed validation
		InsetListingsParams par(fromqstr(listingsED->toPlainText()));
		string caption = fromqstr(captionLE->text());
		string label = fromqstr(labelLE->text());
		if (!caption.empty())
			par.addParam("caption", "{" + caption + "}");
		if (!label.empty())
			par.addParam("label", "{" + label + "}");
		params.setOptions(par.params());
	} else {
		if (visiblespaceCB->isChecked())
			params.setCmdName("verbatiminput*");
		else
			params.setCmdName("verbatiminput");
	}
	controller().setParams(params);
}


void GuiIncludeDialog::browse()
{
	ControlInclude::Type type;

	int const item = typeCO->currentIndex();
	if (item == 0)
		type = ControlInclude::INCLUDE;
	else if (item == 1)
		type = ControlInclude::INPUT;
	else if (item == 2)
		type = ControlInclude::VERBATIM;
	else
		type = ControlInclude::LISTINGS;

	docstring const & name =
		controller().browse(qstring_to_ucs4(filenameED->text()), type);
	if (!name.empty())
		filenameED->setText(toqstr(name));
}


void GuiIncludeDialog::edit()
{
	if (isValid()) {
		string const file = fromqstr(filenameED->text());
		slotOK();
		controller().edit(file);
	}
}


bool GuiIncludeDialog::isValid()
{
	return !filenameED->text().isEmpty() &&
		validate_listings_params().empty();
}

} // namespace frontend
} // namespace lyx

#include "GuiInclude_moc.cpp"
