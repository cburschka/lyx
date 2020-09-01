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
#include "BufferList.h"
#include "BufferParams.h"
#include "FuncRequest.h"
#include "LyXRC.h"

#include "qt_helpers.h"

#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/FileName.h"
#include "support/filetools.h"

#include "frontends/alert.h"

#include "insets/InsetListingsParams.h"
#include "insets/InsetInclude.h"

#include <QCheckBox>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>

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

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));

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
	connect(literalCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(listingsED, SIGNAL(textChanged()), this, SLOT(change_adaptor()));
	connect(listingsED, SIGNAL(textChanged()), this, SLOT(setListingsMsg()));
	connect(bypassCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(bypassCB, SIGNAL(clicked()), this, SLOT(setListingsMsg()));

	setFocusProxy(filenameED);

	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);
	bc().setOK(buttonBox->button(QDialogButtonBox::Ok));
	bc().setCancel(buttonBox->button(QDialogButtonBox::Cancel));
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
	InsetListingsParams lstparams(params);
	lstparams.setMinted(buffer().params().use_minted);
	return lstparams.validate();
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
}


void GuiInclude::paramsToDialog(InsetCommandParams const & icp)
{
	filenameED->setText(toqstr(icp["filename"]));

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

	string cmdname = icp.getCmdName();
	if (cmdname != "include" &&
	    cmdname != "verbatiminput" &&
	    cmdname != "verbatiminput*" &&
	    cmdname != "lstinputlisting" &&
	    cmdname != "inputminted")
		cmdname = "input";

	if (cmdname == "include") {
		typeCO->setCurrentIndex(0);

	} else if (cmdname == "input") {
		typeCO->setCurrentIndex(1);
		previewCB->setEnabled(true);
		previewCB->setChecked(icp.preview());

	} else if (cmdname == "verbatiminput*") {
		typeCO->setCurrentIndex(2);
		visiblespaceCB->setEnabled(true);
		visiblespaceCB->setChecked(true);

	} else if (cmdname == "verbatiminput") {
		typeCO->setCurrentIndex(2);
		visiblespaceCB->setEnabled(true);

	} else if (cmdname == "lstinputlisting" || cmdname == "inputminted") {
		typeCO->setCurrentIndex(3);
		listingsGB->setEnabled(true);
		listingsED->setEnabled(true);
		InsetListingsParams par(to_utf8(icp["lstparams"]));
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
	literalCB->setChecked(icp["literal"] == "true");

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
		if (buffer().params().use_minted)
			params_.setCmdName("inputminted");
		else
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
	params_["literal"] = literalCB->isChecked()
			? from_ascii("true") : from_ascii("false");

	// Do we need to create a LyX file?
	if (item == 0 || item == 1) {
		QString fname = filenameED->text();
		string const mypath = buffer().absFileName();
		string const bpath = buffer().filePath();
		QString absfname = makeAbsPath(fname, toqstr(bpath));
		if (!QFile::exists(absfname)) {
			dispatch(FuncRequest(LFUN_BUFFER_NEW, fromqstr(absfname)));
			dispatch(FuncRequest(LFUN_BUFFER_WRITE));
			dispatch(FuncRequest(LFUN_BUFFER_SWITCH, mypath));
		}
	}
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
	QString const fname = filenameED->text();
	string const bpath = buffer().filePath();
	string const absfname = support::makeAbsPath(fromqstr(fname), bpath).absFileName();
	// The button is enabled only if the document is already open.
	// If something goes wrong and it is not, we'll get an error
	// message from the dispatch. So no need for one here.
	dispatch(FuncRequest(LFUN_BUFFER_SWITCH, absfname));
}


bool GuiInclude::isValid()
{
	QString fname = filenameED->text();
	if (fname.isEmpty() || !validate_listings_params().empty()) {
		editPB->setEnabled(false);
		return false;
	}

	QPushButton * okbutton = buttonBox->button(QDialogButtonBox::Ok);
	int const item = typeCO->currentIndex();
	// Are we inputting or including a LyX file?
	if (item != 0 && item != 1) {
		okbutton->setText(qt_("OK"));
		return true;
	}
	// Do we have a LyX filename?
	if (!isLyXFileName(fromqstr(fname))) {
		okbutton->setText(qt_("OK"));
		return false;
	}
	string const bpath = buffer().filePath();
	// Path might be relative to current Buffer, so make absolute
	FileName const absfname = support::makeAbsPath(fromqstr(fname), bpath);
	// Set OK button text according to whether file already exists
	okbutton->setText(absfname.exists() ? qt_("OK") : qt_("Create"));
	// enable edit button iff file is open in some Buffer
	editPB->setEnabled(theBufferList().getBuffer(absfname));
	return true;
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
		qt_("D&ocuments"), toqstr(lyxrc.document_path));
}


bool GuiInclude::initialiseParams(std::string const & sdata)
{
	InsetCommand::string2params(sdata, params_);
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
