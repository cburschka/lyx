/**
 * \file GuiInclude.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiInclude.h"

#include "Buffer.h"
#include "FuncRequest.h"
#include "LyXRC.h"

#include "qt_helpers.h"
#include "LyXRC.h"

#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/FileName.h"
#include "support/filetools.h"

#include "insets/InsetListingsParams.h"
#include "insets/InsetInclude.h"

#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>

#include <utility>

using namespace std;
using namespace lyx::support;
using namespace lyx::support::os;

namespace lyx {
namespace frontend {


GuiInclude::GuiInclude(GuiView & lv)
	: GuiDialog(lv, "include", qt_("Child Document")),
	  params_(insetCode("include"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(visiblespaceCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(filenameED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(editPB, SIGNAL(clicked()), this, SLOT(edit()));
	connect(browsePB, SIGNAL(clicked()), this, SLOT(browse()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
	connect(previewCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(captionLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(labelLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(listingsED, SIGNAL(textChanged()), this, SLOT(change_adaptor()));
	connect(listingsED, SIGNAL(textChanged()), this, SLOT(setListingsMsg()));
	connect(bypassCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(bypassCB, SIGNAL(clicked()), this, SLOT(setListingsMsg()));

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


void GuiInclude::change_adaptor()
{
	changed();
}


docstring GuiInclude::validate_listings_params()
{
	if (typeCO->currentIndex() != 3 || bypassCB->isChecked())
		return docstring();
	string params = fromqstr(listingsED->toPlainText());
	return InsetListingsParams(params).validate();
}


void GuiInclude::setListingsMsg()
{
	// FIXME THREAD
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


void GuiInclude::typeChanged(int v)
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


void GuiInclude::paramsToDialog(InsetCommandParams const & params_)
{
	filenameED->setText(toqstr(params_["filename"]));

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

	string cmdname = params_.getCmdName();
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
		previewCB->setChecked(params_.preview());

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
		InsetListingsParams par(to_utf8(params_["lstparams"]));
		// extract caption and label and put them into their respective editboxes
		vector<string> pars = getVectorFromString(par.separatedParams(), "\n");
		for (vector<string>::iterator it = pars.begin();
			it != pars.end(); ++it) {
			if (prefixIs(*it, "caption=")) {
				string cap = it->substr(8);
				if (cap[0] == '{' && cap[cap.size() - 1] == '}') {
					captionLE->setText(toqstr(cap.substr(1, cap.size() - 2)));
					*it = "";
				} 
			} else if (prefixIs(*it, "label=")) {
				string lbl = it->substr(6);
				if (lbl[0] == '{' && lbl[lbl.size()-1] == '}') {
					labelLE->setText(toqstr(lbl.substr(1, lbl.size() - 2)));
					*it = "";
				}
			}
		}
		// the rest is put to the extra edit box.
		string extra = getStringFromVector(pars);
		listingsED->setPlainText(toqstr(InsetListingsParams(extra).separatedParams()));
	}

	// Make sure that the bc is in the INITIAL state
	if (bc().policy().buttonStatus(ButtonPolicy::OKAY))
		bc().restore();
}


void GuiInclude::applyView()
{
	params_["filename"] = from_utf8(internal_path(fromqstr(filenameED->text())));
	params_.preview(previewCB->isChecked());

	int const item = typeCO->currentIndex();
	if (item == 0) {
		params_.setCmdName("include");
	} else if (item == 1) {
		params_.setCmdName("input");
	} else if (item == 3) {
		params_.setCmdName("lstinputlisting");
		// the parameter string should have passed validation
		InsetListingsParams par(fromqstr(listingsED->toPlainText()));
		string caption = fromqstr(captionLE->text());
		string label = fromqstr(labelLE->text());
		if (!caption.empty())
			par.addParam("caption", "{" + caption + "}");
		if (!label.empty())
			par.addParam("label", "{" + label + "}");
		string const listparams = par.params();
		params_["lstparams"] = from_utf8(listparams);
	} else {
		if (visiblespaceCB->isChecked())
			params_.setCmdName("verbatiminput*");
		else
			params_.setCmdName("verbatiminput");
	}
}


void GuiInclude::browse()
{
	Type type;

	int const item = typeCO->currentIndex();
	if (item == 0)
		type = INCLUDE;
	else if (item == 1)
		type = INPUT;
	else if (item == 2)
		type = VERBATIM;
	else
		type = LISTINGS;

	QString name = browse(filenameED->text(), type);
	if (!name.isEmpty())
		filenameED->setText(name);
}


void GuiInclude::edit()
{
	if (!isValid())
		return;
	if (bc().policy().buttonStatus(ButtonPolicy::OKAY)) {
		slotOK();
		applyView();
	} else
		hideView();
	dispatch(FuncRequest(LFUN_INSET_EDIT));
}


bool GuiInclude::isValid()
{
	return !filenameED->text().isEmpty() && validate_listings_params().empty();
}


QString GuiInclude::browse(QString const & in_name, Type in_type) const
{
	QString const title = qt_("Select document to include");

	// input TeX, verbatim, or LyX file ?
	QStringList filters;
	switch (in_type) {
	case INCLUDE:
	case INPUT:
		filters = fileFilters(qt_("LaTeX/LyX Documents (*.tex *.lyx)"));
		break;
	case VERBATIM:
	case LISTINGS:
		filters = fileFilters(QString());
		break;
	}

	QString const docpath = toqstr(support::onlyPath(buffer().absFileName()));

	return browseRelToParent(in_name, docpath, title, filters, false,
		qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
}


bool GuiInclude::initialiseParams(std::string const & data)
{
	InsetCommand::string2params(data, params_);
	paramsToDialog(params_);
	return true;
}


void GuiInclude::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string(params_);
	dispatch(FuncRequest(getLfun(), lfun));
}


Dialog * createGuiInclude(GuiView & lv) { return new GuiInclude(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiInclude.cpp"
