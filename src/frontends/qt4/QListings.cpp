/**
 * \file QListings.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 * \author Jürgen Spitzmüller
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
using lyx::support::findToken;
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


char const * languages[] =
{ "no language", "BAP", "ACSL", "Ada", "ALGOL", "C", "C++", "Caml", "Clean", "Cobol",
  "Comal 80", "csh", "Delphi", "Eiffel", "Elan", "Euphoria", "Fortran", "Haskell",
  "HTML", "IDL", "Java", "Lisp", "Logo", "make", "Mathematica", "Matlab", "Mercury",
  "Miranda", "ML", "Modula-2", "Oberon-2", "OCL", "Pascal", "Perl", "PHP", "PL/I", "POV",
  "Python", "Prolog", "R", "S", "SAS", "SHELXL", "Simula", "tcl", "SQL", "TeX", "VBScript",
  "VHDL", "XML", "" };

char const * languages_gui[] =
{ N_("No language"), "BAP", "ACSL", "Ada", "ALGOL", "C", "C++", "Caml", "Clean", "Cobol",
  "Comal 80", "csh", "Delphi", "Eiffel", "Elan", "Euphoria", "Fortran", "Haskell",
  "HTML", "IDL", "Java", "Lisp", "Logo", "make", "Mathematica", "Matlab", "Mercury",
  "Miranda", "ML", "Modula-2", "Oberon-2", "OCL", "Pascal", "Perl", "PHP", "PL/I", "POV",
  "Python", "Prolog", "R", "S", "SAS", "SHELXL", "Simula", "tcl", "SQL", "TeX", "VBScript",
  "VHDL", "XML", "" };

char const * font_sizes[] =
{ "default", "tiny", "scriptsize", "footnotesize", "small", "normalsize", "large",
  "Large", "" };

char const * font_sizes_gui[] =
{ N_("Default"), N_("Tiny"), N_("Smallest"), N_("Smaller"), N_("Small"), N_("Normal"),
  N_("Large"), N_("Larger"), "" };

char const * font_styles[] =
{ "default", "rmfamily", "ttfamily", "sffamily", "" };

char const * font_styles_gui[] =
{ N_("Default"), N_("Roman"), N_("Typewriter"), N_("Sans Serif"), "" };



QListingsDialog::QListingsDialog(QListings * form)
	: form_(form)
{
	setupUi(this);
	
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	
	connect(languageCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(inlineCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(floatCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(placementLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(numberSideCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(numberStepLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(numberFontSizeCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(firstlineLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(lastlineLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(fontsizeCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(fontstyleCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(breaklinesCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(spaceCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(extendedcharsCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	
	connect(listingsED,  SIGNAL(textChanged()), this, SLOT(change_adaptor()));
	connect(listingsED,  SIGNAL(textChanged()), this, SLOT(validate_listings_params()));

	for (int n = 0; languages[n][0]; ++n)
		languageCO->addItem(languages_gui[n]);

	for (int n = 0; font_styles[n][0]; ++n)
		fontstyleCO->addItem(font_styles_gui[n]);

	for (int n = 0; font_sizes[n][0]; ++n) {
		QString font = toqstr(font_sizes_gui[n]);
		fontsizeCO->addItem(font);
		numberFontSizeCO->addItem(font);
	}

	// set validators
	numberStepLE->setValidator(new QIntValidator(0, 1000000, this));
	firstlineLE->setValidator(new QIntValidator(0, 1000000, this));
	lastlineLE->setValidator(new QIntValidator(0, 1000000, this));
	placementLE->setValidator(new QRegExpValidator(QRegExp("[tbph]*"), this));
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
	string language = languages[languageCO->currentIndex()];
	
	bool float_ = floatCB->isChecked();
	string placement;
	if (placementLE->isEnabled())
		placement = fromqstr(placementLE->text());

	string numberSide;
	switch (numberSideCO->currentIndex()) {
	case 0:
		numberSide = "none";
		break;
	case 1:
		numberSide = "left";
		break;
	case 2:
		numberSide = "right";
		break;
	default:
		numberSide = "none";
		break;
	}
	string stepnumber = fromqstr(numberStepLE->text());
	string numberfontsize = font_sizes[numberFontSizeCO->currentIndex()];
	string firstline = fromqstr(firstlineLE->text());
	string lastline = fromqstr(lastlineLE->text());
	
	string fontsize = font_sizes[fontsizeCO->currentIndex()];
	string fontstyle = font_styles[fontstyleCO->currentIndex()];
	string basicstyle;
	if (fontsize != "default")
		basicstyle = "\\" + fontsize;
	if (fontstyle != "default")
		basicstyle += "\\" + fontstyle;
	bool breakline = breaklinesCB->isChecked();
	bool space = spaceCB->isChecked();
	bool extendedchars = extendedcharsCB->isChecked();
	string extra = fromqstr(listingsED->toPlainText());

	// compose a string
	InsetListingsParams par;
	if (language != "no language")
		par.addParam("language", language);
	if (float_)
		par.addParam("float", "");
	if (!placement.empty())
		par.addParam("floatplacement", placement);
	if (numberSide != "none")
		par.addParam("numbers", numberSide);
	if (numberfontsize != "default" && numberSide != "none")
		par.addParam("numberstyle", "\\" + numberfontsize);
	if (!stepnumber.empty() && numberSide != "none")
		par.addParam("stepnumber", stepnumber);
	if (!firstline.empty())
		par.addParam("firstline", firstline);
	if (!lastline.empty())
		par.addParam("lastline", lastline);
	if (!basicstyle.empty())
		par.addParam("basicstyle", basicstyle);
	if (breakline)
		par.addParam("breaklines", "true");
	if (space)
		par.addParam("showspaces", "true");
	if (extendedchars)
		par.addParam("extendedchars", "true");
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
			listingsTB->setPlainText(
				qt_("Input listings parameters on the right. Enter ? for a list of parameters."));
			okPB->setEnabled(true);
			applyPB->setEnabled(true);
		}
	} catch (invalidParam & e) {
		isOK = false;
		listingsTB->setPlainText(e.what());
		okPB->setEnabled(false);
		applyPB->setEnabled(false);
	}
}


void QListingsDialog::on_floatCB_stateChanged(int state)
{
	if (state == Qt::Checked) {
		inlineCB->setChecked(false);
		placementLE->setEnabled(true);
	} else
		placementLE->setEnabled(false);
}


void QListingsDialog::on_inlineCB_stateChanged(int state)
{
	if (state == Qt::Checked) {
		floatCB->setChecked(false);
		placementLE->setEnabled(false);
	}
}


void QListingsDialog::on_numberSideCO_currentIndexChanged(int index)
{
	numberStepLE->setEnabled(index > 0);
	numberFontSizeCO->setEnabled(index > 0);
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
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	dialog_->listingsTB->setPlainText(
		qt_("Input listings parameters on the right. Enter ? for a list of parameters."));

	update_contents();
}


void QListings::apply()
{
	InsetListingsParams & params = controller().params();
	params.setInline(dialog_->inlineCB->isChecked());
	params.setParams(dialog_->construct_params());
	controller().setParams(params);
}


void QListings::update_contents()
{
	// set default values 
	dialog_->listingsTB->setPlainText(
		qt_("Input listings parameters on the right. Enter ? for a list of parameters."));
	dialog_->languageCO->setCurrentIndex(findToken(languages, "no language"));
	dialog_->floatCB->setChecked(false);
	dialog_->placementLE->clear();
	dialog_->numberSideCO->setCurrentIndex(0);
	dialog_->numberStepLE->clear();
	dialog_->numberFontSizeCO->setCurrentIndex(findToken(font_sizes, "default"));
	dialog_->firstlineLE->clear();
	dialog_->lastlineLE->clear();
	dialog_->fontstyleCO->setCurrentIndex(findToken(font_styles, "default"));
	dialog_->fontsizeCO->setCurrentIndex(findToken(font_sizes, "default"));
	dialog_->breaklinesCB->setChecked(false);
	dialog_->spaceCB->setChecked(false);
	dialog_->extendedcharsCB->setChecked(false);

	// set values from param string
	InsetListingsParams & params = controller().params();
	dialog_->inlineCB->setChecked(params.isInline());
	if (params.isInline()) {
		dialog_->floatCB->setChecked(false);
		dialog_->placementLE->setEnabled(false);
	}
	// break other parameters and set values
	vector<string> pars = getVectorFromString(params.separatedParams(), "\n");
	// process each of them
	for (vector<string>::iterator it = pars.begin();
	    it != pars.end(); ++it) {
		if (prefixIs(*it, "language=")) {
			int n = findToken(languages, it->substr(9));
			dialog_->languageCO->setCurrentIndex(n);
			*it = "";
		} else if (prefixIs(*it, "floatplacement=")) {
			dialog_->floatCB->setChecked(true);
			dialog_->placementLE->setEnabled(true);
			dialog_->placementLE->setText(toqstr(it->substr(15)));
			dialog_->inlineCB->setChecked(false);
			*it = "";
		} else if (prefixIs(*it, "float")) {
			dialog_->floatCB->setChecked(true);
			dialog_->inlineCB->setChecked(false);
			dialog_->placementLE->setEnabled(true);
			if (prefixIs(*it, "float="))
				dialog_->placementLE->setText(toqstr(it->substr(6)));
			*it = "";
		} else if (prefixIs(*it, "numbers=")) {
			string s = it->substr(8);
			int n = 0;
			if (s == "left")
				n = 1;
			else if (s == "right")
				n = 2;
			dialog_->numberSideCO->setCurrentIndex(n);
			*it = "";
		} else if (prefixIs(*it, "stepnumber=")) {
			dialog_->numberStepLE->setText(toqstr(it->substr(11)));
			*it = "";
		} else if (prefixIs(*it, "numberstyle=")) {
			int n = findToken(font_sizes, it->substr(13));
			dialog_->numberFontSizeCO->setCurrentIndex(n);
			*it = "";
		} else if (prefixIs(*it, "firstline=")) {
			dialog_->firstlineLE->setText(toqstr(it->substr(10)));
			*it = "";
		} else if (prefixIs(*it, "lastline=")) {
			dialog_->lastlineLE->setText(toqstr(it->substr(9)));
			*it = "";
		} else if (prefixIs(*it, "basicstyle=")) {
			string style;
			string size;
			for (int n = 0; font_styles[n][0]; ++n) {
				string const s = font_styles[n];
				if (contains(*it, "\\" + s)) {
					style = "\\" + s;
					break;
				}
			}
			for (int n = 0; font_sizes[n][0]; ++n) {
				string const s = font_sizes[n];
				if (contains(*it, "\\" + s)) {
					size = "\\" + s;
					break;
				}
			}
			if (it->substr(11) == style + size || it->substr(11) == size + style) {
				if (!style.empty())
					dialog_->fontstyleCO->setCurrentIndex(
						findToken(font_styles, style.substr(1)));
				if (!size.empty())
					dialog_->fontsizeCO->setCurrentIndex(
						findToken(font_sizes, size.substr(1)));
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
		}
	}

	dialog_->numberStepLE->setEnabled(dialog_->numberSideCO->currentIndex() > 0);
	dialog_->numberFontSizeCO->setEnabled(dialog_->numberSideCO->currentIndex() > 0);
	// parameters that can be handled by widgets are cleared
	// the rest is put to the extra edit box.
	string extra = getStringFromVector(pars);
	dialog_->listingsED->setPlainText(toqstr(InsetListingsParams(extra).separatedParams()));
}


} // namespace frontend
} // namespace lyx


#include "QListings_moc.cpp"
