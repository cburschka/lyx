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
#include <QValidator>
#include <QRegExpValidator>


using std::string;
using std::vector;
using lyx::support::getVectorFromString;
using lyx::support::getStringFromVector;
using lyx::support::prefixIs;
using lyx::support::contains;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QListingsDialog
//
/////////////////////////////////////////////////////////////////////


string const allowed_languages = 
	"no language\nBAP\nACSL\nAda\nALGOL\nC\nC++\nCaml\nClean\nCobol\n"
	"Comal 80\ncsh\nDelphi\nEiffel\nElan\nEuphoria\nFortran\nHaskell\n"
	"HTML\nIDL\nJava\nLisp\nLogo\nmake\nMathematica\nMatlab\nMercury\n"
	"Miranda\nML\nModula-2\nOberon-2\nOCL\nPascal\nPerl\nPHP\nPL/I\nPOV\n"
	"Python\nProlog\nR\nS\nSAS\nSHELXL\nSimula\ntcl\nSQL\nTeX\nVBScript\n"
	"VHDL\nXML";
string const allowed_fontsizes = "default\ntiny\nscriptsize\nfootnotesize\nsmall\n"
	"normalsize\nlarge\nLarge";
string const allowed_fontstyles = "default\nrmfamily\nttfamily\nsffamily";

QListingsDialog::QListingsDialog(QListings * form)
	: form_(form)
{
	setupUi(this);
	
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	
	connect(languageCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(inlineCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(floatCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(placementLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(numberLeftCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(numberRightCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(numberStepLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(numberFontSizeCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(firstlineLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(lastlineLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(fontsizeCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(fontstyleCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(breaklinesCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(spaceCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(extendedcharsCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(captionLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(labelLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	
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


string QListingsDialog::construct_params()
{
	string language = fromqstr(languageCO->currentText());
	
	bool float_ = floatCB->checkState() == Qt::Checked;
	string placement = fromqstr(placementLE->text());
	
	bool left = numberLeftCB->checkState() == Qt::Checked;
	bool right = numberRightCB->checkState() == Qt::Checked;
	string step = fromqstr(numberStepLE->text());
	string numberfontsize = fromqstr(numberFontSizeCO->currentText());
	string firstline = fromqstr(firstlineLE->text());
	string lastline = fromqstr(lastlineLE->text());
	
	string fontsize = fromqstr(fontsizeCO->currentText());
	string fontstyle = fromqstr(fontstyleCO->currentText());
	string basicstyle;
	if (fontsize != "default")
		basicstyle = "\\" + fontsize;
	if (fontstyle != "default")
		basicstyle += "\\" + fontstyle;
	bool breakline = breaklinesCB->checkState() == Qt::Checked;
	bool space = spaceCB->checkState() == Qt::Checked;
	bool extendedchar = extendedcharsCB->checkState() == Qt::Checked;
	
	string caption = fromqstr(captionLE->text());
	string label = fromqstr(labelLE->text());
	
	string extra = fromqstr(listingsED->toPlainText());

	// compose a string
	InsetListingsParams par;
	if (language != "no language")
		par.addParam("language", language);
	if (float_)
		par.addParam("float", "");
	if (!placement.empty())
		par.addParam("floatplacement", placement);
	if (left)
		par.addParam("numbers", "left");
	else if (right)
		par.addParam("numbers", "right");
	if (numberfontsize != "default")
		par.addParam("numberstyle", "\\" + numberfontsize);
	if (!firstline.empty())
		par.addParam("firstline", firstline);
	if (!lastline.empty())
		par.addParam("lastline", lastline);
	if (basicstyle != "")
		par.addParam("basicstyle", basicstyle);
	if (breakline)
		par.addParam("breaklines", "true");
	if (space)
		par.addParam("showspaces", "true");
	if (extendedchar)
		par.addParam("extendedchars", "true");
	if (!caption.empty())
		par.addParam("caption", "{" + caption + "}");
	if (!label.empty())
		par.addParam("label", "{" + label + "}");
	par.addParams(extra);
	return par.params();
}


void QListingsDialog::validate_listings_params()
{
	static bool isOK = true;
	try {
		InsetListingsParams par(construct_params());
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

typedef QController<ControlListings, QView<QListingsDialog> > listings_wrap_base_class;

QListings::QListings(Dialog & parent)
	: listings_wrap_base_class(parent, _("Program Listings Settings"))
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
	params.setParams(dialog_->construct_params());
	controller().setParams(params);
}


void QListings::update_contents()
{
	// first prepare all choices
	vector<string> const languages = 
		getVectorFromString(allowed_languages, "\n");
	vector<string> const fontstyles = 
		getVectorFromString(allowed_fontstyles, "\n");
	vector<string> const fontsizes = 
		getVectorFromString(allowed_fontsizes, "\n");

	dialog_->languageCO->clear();
	for (vector<string>::const_iterator it = languages.begin();
	    it != languages.end(); ++it) {
		dialog_->languageCO->addItem(toqstr(*it));
	}
	dialog_->fontstyleCO->clear();
	dialog_->fontstyleCO->setEditable(false);
	for (vector<string>::const_iterator it = fontstyles.begin();
	    it != fontstyles.end(); ++it) {
		dialog_->fontstyleCO->addItem(toqstr(*it));
	}
	dialog_->fontsizeCO->clear();
	dialog_->fontsizeCO->setEditable(false);
	dialog_->numberFontSizeCO->clear();
	dialog_->numberFontSizeCO->setEditable(false);
	for (vector<string>::const_iterator it = fontsizes.begin();
	    it != fontsizes.end(); ++it) {
		dialog_->fontsizeCO->addItem(toqstr(*it));
		dialog_->numberFontSizeCO->addItem(toqstr(*it));
	}

	// set validators
	dialog_->numberStepLE->setValidator(new QIntValidator(0, 1000000, this));
	dialog_->firstlineLE->setValidator(new QIntValidator(0, 1000000, this));
	dialog_->lastlineLE->setValidator(new QIntValidator(0, 1000000, this));
	dialog_->placementLE->setValidator(new QRegExpValidator(QRegExp("[tbph]*"), this));

	//
	dialog_->listingsTB->setPlainText("Input listings parameters below. Enter ? for a list of parameters.");

	// set values from param string
	InsetListingsParams & params = controller().params();
	if (params.isInline())
		dialog_->inlineCB->setChecked(true);
	else
		dialog_->inlineCB->setChecked(false);
	// break other parameters and set values
	vector<string> pars = getVectorFromString(params.separatedParams(), "\n");
	// process each of them
	for (vector<string>::iterator it = pars.begin();
	    it != pars.end(); ++it) {
		if (prefixIs(*it, "language=")) {
			for (vector<string>::const_iterator st = languages.begin();
			    st != languages.end(); ++st) {
				if (*it == "language=" + *st) {
					dialog_->languageCO->setCurrentIndex(
						dialog_->languageCO->findText(toqstr(*st)));
					*it = "";
				}			
			}
		} else if (prefixIs(*it, "float")) {
			if (prefixIs(*it, "float="))
				dialog_->placementLE->setText(toqstr(it->substr(6)));
			else
				dialog_->floatCB->setChecked(true);
			*it = "";
		} else if (prefixIs(*it, "floatplacement=")) {
			dialog_->placementLE->setText(toqstr(it->substr(15)));
			*it = "";
		} else if (prefixIs(*it, "numbers=")) {
			if (contains(*it, "left"))
				dialog_->numberLeftCB->setChecked(true);
			else if (contains(*it, "right"))
				dialog_->numberRightCB->setChecked(true);
			*it = "";
		} else if (prefixIs(*it, "stepnumber=")) {
			dialog_->numberStepLE->setText(toqstr(it->substr(11)));
			*it = "";
		} else if (prefixIs(*it, "numberstyle=")) {
			for (vector<string>::const_iterator st = fontsizes.begin();
			    st != fontsizes.end(); ++st) {
				if (*it == "numberstyle=\\" + *st) {
					dialog_->numberFontSizeCO->setCurrentIndex(
						dialog_->numberFontSizeCO->findText(toqstr(*st)));
					*it = "";
				}			
			}
		} else if (prefixIs(*it, "firstline=")) {
			dialog_->firstlineLE->setText(toqstr(it->substr(10)));
			*it = "";
		} else if (prefixIs(*it, "lastline=")) {
			dialog_->lastlineLE->setText(toqstr(it->substr(9)));
			*it = "";
		} else if (prefixIs(*it, "basicstyle=")) {
			string style;
			string size;
			for (vector<string>::const_iterator st = fontstyles.begin();
			    st != fontstyles.end(); ++st)
				if (contains(*it, "\\" + *st)) {
					style = "\\" + *st;
					break;
				}
			for (vector<string>::const_iterator st = fontsizes.begin();
			    st != fontsizes.end(); ++st)
				if (contains(*it, "\\" + *st)) {
					size = "\\" + *st;
					break;
				}
			if (it->substr(11) == style + size || it->substr(11) == size + style) {
				if (!style.empty())
					dialog_->fontstyleCO->setCurrentIndex(
						dialog_->fontstyleCO->findText(toqstr(style.substr(1))));
				if (!size.empty())
					dialog_->fontsizeCO->setCurrentIndex(
						dialog_->fontsizeCO->findText(toqstr(size.substr(1))));
				*it = "";
			}
		} else if (prefixIs(*it, "breaklines=")) {
			dialog_->breaklinesCB->setChecked(contains(*it, "true"));
			*it = "";
		} else if (prefixIs(*it, "showspaces=")) {
			dialog_->spaceCB->setChecked(contains(*it, "true"));
			*it = "";
		} else if (prefixIs(*it, "extendedchars=")) {
			dialog_->extendedcharsCB->setChecked(contains(*it, "true"));
			*it = "";
		} else if (prefixIs(*it, "caption=")) {
			string cap = it->substr(8);
			if ((cap[0] == '{' && cap[cap.size()-1] == '}') ||
				(cap[0] == '"' && cap[cap.size()-1] == '"') )
				dialog_->captionLE->setText(toqstr(cap.substr(1, cap.size()-2)));
			else
				dialog_->captionLE->setText(toqstr(cap));		
			*it = "";
		} else if (prefixIs(*it, "label=")) {
			string lbl = it->substr(6);
			if ((lbl[0] == '{' && lbl[lbl.size()-1] == '}') ||
				(lbl[0] == '"' && lbl[lbl.size()-1] == '"') )
				dialog_->labelLE->setText(toqstr(lbl.substr(1, lbl.size()-2)));
			else
				dialog_->labelLE->setText(toqstr(lbl));			
			*it = "";
		}
	}
	// parameters that can be handled by widgets are cleared
	// the rest is put to the extra edit box.
	string extra = getStringFromVector(pars);
	dialog_->listingsED->setPlainText(toqstr(InsetListingsParams(extra).separatedParams()));
}


} // namespace frontend
} // namespace lyx


#include "QListings_moc.cpp"
