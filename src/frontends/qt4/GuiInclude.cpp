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

#include "frontend_helpers.h"

#include "Buffer.h"
#include "Format.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "LyXRC.h"

#include "qt_helpers.h"
#include "LyXRC.h"

#include "support/os.h"
#include "support/lstrings.h"
#include "support/FileFilterList.h"
#include "support/filetools.h"

#include "insets/InsetListingsParams.h"
#include "insets/InsetInclude.h"

#include <QPushButton>
#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>

#include <utility>

using std::string;
using std::vector;
using std::pair;
using std::string;


namespace lyx {
namespace frontend {

using support::FileFilterList;
using support::FileName;
using support::makeAbsPath;
using support::onlyPath;
using support::os::internal_path;
using support::prefixIs;
using support::getStringFromVector;
using support::getVectorFromString;


/// Flags what action is taken by Kernel::dispatch()
static std::string const lfun_name_ = "include";


GuiInclude::GuiInclude(LyXView & lv)
	: GuiDialog(lv, "include"), params_(INCLUDE_CODE)
{
	setupUi(this);
	setViewTitle(_("Child Document"));

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


void GuiInclude::change_adaptor()
{
	changed();
}


docstring GuiInclude::validate_listings_params()
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


void GuiInclude::set_listings_msg()
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


void GuiInclude::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
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


void GuiInclude::updateContents()
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
		InsetListingsParams par(params_.getOptions());
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
		params_.setOptions(par.params());
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

	docstring const & name = browse(qstring_to_ucs4(filenameED->text()), type);
	if (!name.empty())
		filenameED->setText(toqstr(name));
}


void GuiInclude::edit()
{
	if (isValid()) {
		string const file = fromqstr(filenameED->text());
		slotOK();
		edit(file);
	}
}


bool GuiInclude::isValid()
{
	return !filenameED->text().isEmpty() && validate_listings_params().empty();
}


bool GuiInclude::initialiseParams(string const & data)
{
	InsetCommandMailer::string2params(lfun_name_, data, params_);
	return true;
}


void GuiInclude::clearParams()
{
	params_.clear();
}


void GuiInclude::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetCommandMailer::params2string(lfun_name_, params_)));
}


docstring GuiInclude::browse(docstring const & in_name, Type in_type) const
{
	docstring const title = _("Select document to include");

	// input TeX, verbatim, or LyX file ?
	FileFilterList filters;
	switch (in_type) {
	case INCLUDE:
	case INPUT:
		filters = FileFilterList(_("LaTeX/LyX Documents (*.tex *.lyx)"));
		break;
	case VERBATIM:
	case LISTINGS:
		break;
	}

	pair<docstring, docstring> dir1(_("Documents|#o#O"),
		from_utf8(lyxrc.document_path));

	docstring const docpath = from_utf8(onlyPath(buffer().absFileName()));

	return browseRelFile(in_name, docpath, title,
			     filters, false, dir1);
}


void GuiInclude::edit(string const & file)
{
	string const ext = support::getExtension(file);
	if (ext == "lyx")
		dispatch(FuncRequest(LFUN_BUFFER_CHILD_OPEN, file));
	else
		// tex file or other text file in verbatim mode
		formats.edit(buffer(), 
			makeAbsPath(file, onlyPath(buffer().absFileName())),
			"text");
}


Dialog * createGuiInclude(LyXView & lv) { return new GuiInclude(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiInclude_moc.cpp"
