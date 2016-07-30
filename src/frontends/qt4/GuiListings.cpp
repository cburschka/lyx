/**
 * \file GuiListings.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiListings.h"

#include "qt_helpers.h"

#include "FuncRequest.h"

#include "insets/InsetListings.h"
#include "insets/InsetListingsParams.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QLineEdit>
#include <QPushButton>
#include <QValidator>
#include <QRegExpValidator>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


/////////////////////////////////////////////////////////////////////
//
// GuiListings
//
/////////////////////////////////////////////////////////////////////


char const * languages_supported[] =
{ "no language", "ABAP", "ACSL", "Ada", "ALGOL", "Assembler", "Awk", "bash", "Basic", "C",
  "C++", "Caml", "Clean", "Cobol", "Comal 80", "command.com", "Comsol", "csh", "Delphi",
  "Eiffel", "Elan", "erlang", "Euphoria", "Fortran", "Gnuplot", "Haskell", "HTML", "IDL", "inform",
  "Java", "JVMIS", "ksh", "Lingo", "Lisp", "Logo", "make", "Mathematica", "Matlab", "Mercury",
  "MetaPost", "Miranda", "ML", "Modula-2", "MuPAD", "NASTRAN", "Oberon-2", "OCL", "Octave",
  "Oz", "Pascal", "Perl", "PHP", "PL/I", "Plasm", "PostScript", "POV", "Prolog", "Promela",
  "PSTricks", "Python", "R", "Reduce", "Rexx", "RSL", "Ruby", "S", "SAS", "Scilab", "sh",
  "SHELXL", "Simula", "tcl", "SPARQL", "SQL", "tcl", "TeX", "VBScript", "Verilog", "VHDL",
  "VRML", "XML", "XSLT", "" };


char const * languages_gui[] =
{ N_("No language"), "ABAP", "ACSL", "Ada", "ALGOL", "Assembler", "Awk", "bash", "Basic",
  "C", "C++", "Caml", "Clean", "Cobol", "Comal 80", "command.com", "Comsol", "csh", "Delphi",
  "Eiffel", "Elan", "Erlang", "Euphoria", "Fortran", "Gnuplot", "Haskell", "HTML", "IDL", "inform",
  "Java", "JVMIS", "ksh", "Lingo", "Lisp", "Logo", "make", "Mathematica", "Matlab", "Mercury",
  "MetaPost", "Miranda", "ML", "Modula-2", "MuPAD", "NASTRAN", "Oberon-2", "OCL", "Octave",
  "Oz", "Pascal", "Perl", "PHP", "PL/I", "Plasm", "PostScript", "POV", "Prolog", "Promela",
  "PSTricks", "Python", "R", "Reduce", "Rexx", "RSL", "Ruby", "S", "SAS", "Scilab", "sh",
  "SHELXL", "Simula", "tcl", "SPARQL", "SQL", "tcl", "TeX", "VBScript", "Verilog", "VHDL",
  "VRML", "XML", "XSLT", "" };


struct dialect_info {
	/// the dialect
	char const * dialect;
	/// the associated language
	char const * language;
	/// representation of the dialect in the gui
	char const * gui;
	/// is this the default dialect?
	bool is_default;
};


dialect_info const dialects[] = {
	{ "R/2 4.3", "ABAP", "R/2 4.3", false },
	{ "R/2 5.0", "ABAP", "R/2 5.0", false },
	{ "R/3 3.1", "ABAP", "R/3 3.1", false },
	{ "R/3 4.6C", "ABAP", "R/3 4.6C", false },
	{ "R/3 6.10", "ABAP", "R/3 6.10", true },
	{ "2005", "Ada", "2005", true },
	{ "83", "Ada", "83", false },
	{ "95", "Ada", "95", false },
	{ "60", "Algol", "60", false },
	{ "68", "Algol", "68", true },
	{ "Motorola68k", "Assembler", "Motorola 68xxx", false },
	{ "x86masm", "Assembler", "x86 (MASM)", false },
	{ "gnu", "Awk", "gnu", true },
	{ "POSIX", "Awk", "POSIX", false },
	{ "Visual", "Basic", "Visual", false },
	{ "ANSI", "C", "ANSI", true },
	{ "Handel", "C", "Handel", false },
	{ "Objective", "C", "Objective", false },
	{ "Sharp", "C", "Sharp", false },
	{ "ANSI", "C++", "ANSI", false },
	{ "GNU", "C++", "GNU", false },
	{ "ISO", "C++", "ISO", true },
	{ "Visual", "C++", "Visual", false },
	{ "light", "Caml", "light", true },
	{ "Objective", "Caml", "Objective", false },
	{ "1974", "Cobol", "1974", false },
	{ "1985", "Cobol", "1985", true },
	{ "ibm", "Cobol", "IBM", false },
	{ "WinXP", "command.com", "Windows XP", true },
	{ "77", "Fortran", "77", false },
	{ "90", "Fortran", "90", false },
	{ "95", "Fortran", "95", true },
	{ "CORBA", "IDL", "CORBA", false },
	{ "AspectJ", "Java", "Aspect J", false },
	{ "Auto", "Lisp", "Auto", false },
	{ "gnu", "make", "gnu", false },
	{ "1.0", "Mathematica", "1.0", false },
	{ "3.0", "Mathematica", "3.0", false },
	{ "5.2", "Mathematica", "5.2", true },
	{ "decorative", "OCL", "decorative", false },
	{ "OMG", "OCL", "OMG", true },
	{ "Borland6", "Pascal", "Borland 6", false },
	{ "Standard", "Pascal", "Standard", true },
	{ "XSC", "Pascal", "XSC", false },
	{ "PLUS", "S", "PLUS", false },
	{ "67", "Simula", "67", true },
	{ "CII", "Simula", "CII", false },
	{ "DEC", "Simula", "DEC", false },
	{ "IBM", "Simula", "IBM", false },
	{ "tk", "tcl", "tk", false },
	{ "AlLaTeX", "TeX", "AlLaTeX", false },
	{ "common", "TeX", "common", false },
	{ "LaTeX", "TeX", "LaTeX", false },
	{ "plain", "TeX", "plain", true },
	{ "primitive", "TeX", "primitive", false },
	{ "AMS", "VHDL", "AMS", false },
	{ "97", "VRML", "97", true }
};


size_t const nr_dialects = sizeof(dialects) / sizeof(dialect_info);


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



GuiListings::GuiListings(GuiView & lv)
	: GuiDialog(lv, "listings", qt_("Program Listing Settings"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(languageCO, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));
	connect(dialectCO, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));
	connect(inlineCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(floatCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(placementLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(numberSideCO, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));
	connect(numberStepLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(numberFontSizeCO, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));
	connect(firstlineLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(lastlineLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(fontsizeCO, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));
	connect(fontstyleCO, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));
	connect(breaklinesCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(spaceCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(spaceInStringCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(tabsizeSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(extendedcharsCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	connect(listingsED,  SIGNAL(textChanged()),
		this, SLOT(change_adaptor()));
	connect(listingsED,  SIGNAL(textChanged()),
		this, SLOT(setListingsMsg()));
	connect(bypassCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(bypassCB, SIGNAL(clicked()),
		this, SLOT(setListingsMsg()));

	for (int n = 0; languages_supported[n][0]; ++n)
		languageCO->addItem(qt_(languages_gui[n]));

	for (int n = 0; font_styles[n][0]; ++n)
		fontstyleCO->addItem(qt_(font_styles_gui[n]));

	for (int n = 0; font_sizes[n][0]; ++n) {
		QString font = qt_(font_sizes_gui[n]);
		fontsizeCO->addItem(font);
		numberFontSizeCO->addItem(font);
	}

	// set validators
	numberStepLE->setValidator(new QIntValidator(0, 1000000, this));
	firstlineLE->setValidator(new QIntValidator(0, 1000000, this));
	lastlineLE->setValidator(new QIntValidator(0, 1000000, this));
	placementLE->setValidator(new QRegExpValidator(QRegExp("[\\*tbph]*"), this));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	listingsTB->setPlainText(
		qt_("Input listing parameters on the right. Enter ? for a list of parameters."));

	updateContents();

}


void GuiListings::change_adaptor()
{
	changed();
}


string GuiListings::construct_params()
{
	string language = languages_supported[qMax(0, languageCO->currentIndex())];
	string dialect;
	string const dialect_gui = fromqstr(dialectCO->currentText());
	if (dialectCO->currentIndex() > 0) {
		for (size_t i = 0; i != nr_dialects; ++i) {
			if (dialect_gui == dialects[i].gui
			&& dialects[i].language == language
			&& !dialects[i].is_default) {
				dialect = dialects[i].dialect;
				break;
			}
		}
	}

	bool float_ = floatCB->isChecked();
	string placement;
	if (placementLE->isEnabled())
		placement = fromqstr(placementLE->text());

	string numberSide;
	switch (qMax(0, numberSideCO->currentIndex())) {
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
	string numberfontsize = font_sizes[qMax(0, numberFontSizeCO->currentIndex())];
	string firstline = fromqstr(firstlineLE->text());
	string lastline = fromqstr(lastlineLE->text());

	string fontsize = font_sizes[qMax(0, fontsizeCO->currentIndex())];
	string fontstyle = font_styles[qMax(0, fontstyleCO->currentIndex())];
	string basicstyle;
	if (fontsize != "default")
		basicstyle = "\\" + fontsize;
	if (fontstyle != "default")
		basicstyle += "\\" + fontstyle;
	bool breakline = breaklinesCB->isChecked();
	bool space = spaceCB->isChecked();
	int tabsize = tabsizeSB->value();
	bool spaceInString = spaceInStringCB->isChecked();
	bool extendedchars = extendedcharsCB->isChecked();
	string extra = fromqstr(listingsED->toPlainText());

	// compose a string
	InsetListingsParams par;
	if (language != "no language" && !contains(extra, "language=")) {
		if (dialect.empty())
			par.addParam("language", language);
		else
			par.addParam("language", "{[" + dialect + "]" + language + "}");
	}
	// this dialog uses float=placement instead of float,floatplacement=placement
	// because float accepts *tbph and floatplacement accepts bph.
	// our placement textedit is actually for the float parameter
	if (float_)
		par.addParam("float", placement);
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
	if (!spaceInString)
		par.addParam("showstringspaces", "false");
	if (tabsize != 8)
		par.addParam("tabsize", convert<string>(tabsize));
	if (extendedchars)
		par.addParam("extendedchars", "true");
	par.addParams(extra);
	return par.params();
}


docstring GuiListings::validate_listings_params()
{
	if (bypassCB->isChecked())
		return docstring();
	return InsetListingsParams(construct_params()).validate();
}


void GuiListings::setListingsMsg()
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


void GuiListings::on_floatCB_stateChanged(int state)
{
	if (state == Qt::Checked) {
		inlineCB->setChecked(false);
		placementLE->setEnabled(true);
	} else
		placementLE->setEnabled(false);
}


void GuiListings::on_inlineCB_stateChanged(int state)
{
	if (state == Qt::Checked) {
		floatCB->setChecked(false);
		placementLE->setEnabled(false);
	}
}


void GuiListings::on_numberSideCO_currentIndexChanged(int index)
{
	numberStepLE->setEnabled(index > 0);
	numberFontSizeCO->setEnabled(index > 0);
}


void GuiListings::on_languageCO_currentIndexChanged(int index)
{
	dialectCO->clear();
	// 0 is "no dialect"
	int default_dialect = 0;
	dialectCO->addItem(qt_("No dialect"));
	string const language = languages_supported[index];

	for (size_t i = 0; i != nr_dialects; ++i) {
		if (language == dialects[i].language) {
			dialectCO->addItem(qt_(dialects[i].gui));
			if (dialects[i].is_default)
				default_dialect =
					dialectCO->findText(qt_(dialects[i].gui));
		}
	}
	dialectCO->setCurrentIndex(default_dialect);
	dialectCO->setEnabled(dialectCO->count() > 1);
}


void GuiListings::applyView()
{
	params_.setInline(inlineCB->isChecked());
	params_.setParams(construct_params());
}


static string plainParam(string const & par)
{
	// remove enclosing braces
	if (prefixIs(par, "{") && suffixIs(par, "}"))
		return par.substr(1, par.size() - 2);
	return par;
}


void GuiListings::updateContents()
{
	// set default values
	listingsTB->setPlainText(
		qt_("Input listing parameters on the right. Enter ? for a list of parameters."));
	languageCO->setCurrentIndex(findToken(languages_supported, "no language"));
	dialectCO->setCurrentIndex(0);
	floatCB->setChecked(false);
	placementLE->clear();
	numberSideCO->setCurrentIndex(0);
	numberStepLE->clear();
	numberFontSizeCO->setCurrentIndex(findToken(font_sizes, "default"));
	firstlineLE->clear();
	lastlineLE->clear();
	fontstyleCO->setCurrentIndex(findToken(font_styles, "default"));
	fontsizeCO->setCurrentIndex(findToken(font_sizes, "default"));
	breaklinesCB->setChecked(false);
	spaceCB->setChecked(false);
	spaceInStringCB->setChecked(true);
	tabsizeSB->setValue(8);
	extendedcharsCB->setChecked(false);

	// set values from param string
	inlineCB->setChecked(params_.isInline());
	if (params_.isInline()) {
		floatCB->setChecked(false);
		placementLE->setEnabled(false);
	}
	// break other parameters and set values
	vector<string> pars = getVectorFromString(params_.separatedParams(), "\n");
	// process each of them
	for (vector<string>::iterator it = pars.begin();
	    it != pars.end(); ++it) {
		if (prefixIs(*it, "language=")) {
			string arg = plainParam(it->substr(9));
			// has dialect?
			string language;
			string dialect;
			bool in_gui = false;
			if (prefixIs(arg, "[") && contains(arg, "]")) {
				size_t end_dialect = arg.find("]");
				dialect = arg.substr(1, end_dialect - 1);
				language = arg.substr(end_dialect + 1);
			} else {
				language = arg;
			}
			int n = findToken(languages_supported, language);
			if (n >= 0) {
				languageCO->setCurrentIndex(n);
				in_gui = true;
			}
			// on_languageCO_currentIndexChanged should have set dialects
			if (!dialect.empty()) {
				string dialect_gui;
				for (size_t i = 0; i != nr_dialects; ++i) {
					if (dialect == dialects[i].dialect
					    && dialects[i].language == language) {
						dialect_gui = dialects[i].gui;
						break;
					}
				}
				n = dialectCO->findText(qt_(dialect_gui));
				if (n >= 0)
					dialectCO->setCurrentIndex(n);
				else
					in_gui = false;
			}
			if (in_gui)
				*it = "";
			languageCO->setEnabled(in_gui);
			dialectCO->setEnabled(
				in_gui && dialectCO->count() > 1);
		} else if (prefixIs(*it, "float")) {
			floatCB->setChecked(true);
			inlineCB->setChecked(false);
			placementLE->setEnabled(true);
			if (prefixIs(*it, "float="))
				placementLE->setText(
					toqstr(plainParam(it->substr(6))));
			*it = "";
		} else if (prefixIs(*it, "numbers=")) {
			string s = plainParam(it->substr(8));
			int n = 0;
			if (s == "left")
				n = 1;
			else if (s == "right")
				n = 2;
			numberSideCO->setCurrentIndex(n);
			*it = "";
		} else if (prefixIs(*it, "stepnumber=")) {
			numberStepLE->setText(
				toqstr(plainParam(it->substr(11))));
			*it = "";
		} else if (prefixIs(*it, "numberstyle=")) {
			string par = plainParam(it->substr(12));
			int n = findToken(font_sizes, par.substr(1));
			if (n >= 0)
				numberFontSizeCO->setCurrentIndex(n);
			*it = "";
		} else if (prefixIs(*it, "firstline=")) {
			firstlineLE->setText(
				toqstr(plainParam(it->substr(10))));
			*it = "";
		} else if (prefixIs(*it, "lastline=")) {
			lastlineLE->setText(
				toqstr(plainParam(it->substr(9))));
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
			if (plainParam(it->substr(11)) == style + size
			    || plainParam(it->substr(11)) == size + style) {
				if (!style.empty()) {
					int n = findToken(font_styles, style.substr(1));
					if (n >= 0)
						fontstyleCO->setCurrentIndex(n);
				}
				if (!size.empty()) {
					int n = findToken(font_sizes, size.substr(1));
					if (n >= 0)
						fontsizeCO->setCurrentIndex(n);
				}
				*it = "";
			}
		} else if (prefixIs(*it, "breaklines=")) {
			breaklinesCB->setChecked(contains(*it, "true"));
			*it = "";
		} else if (prefixIs(*it, "showspaces=")) {
			spaceCB->setChecked(contains(*it, "true"));
			*it = "";
		} else if (prefixIs(*it, "showstringspaces=")) {
			spaceInStringCB->setChecked(contains(*it, "true"));
			*it = "";
		} else if (prefixIs(*it, "tabsize=")) {
			tabsizeSB->setValue(convert<int>(plainParam(it->substr(8))));
			*it = "";
		} else if (prefixIs(*it, "extendedchars=")) {
			extendedcharsCB->setChecked(contains(*it, "true"));
			*it = "";
		}
	}

	numberStepLE->setEnabled(numberSideCO->currentIndex() > 0);
	numberFontSizeCO->setEnabled(numberSideCO->currentIndex() > 0);
	// parameters that can be handled by widgets are cleared
	// the rest is put to the extra edit box.
	string extra = getStringFromVector(pars);
	listingsED->setPlainText(toqstr(InsetListingsParams(extra).separatedParams()));
}


bool GuiListings::isValid()
{
	return validate_listings_params().empty();
}


bool GuiListings::initialiseParams(string const & data)
{
	InsetListings::string2params(data, params_);
	return true;
}


void GuiListings::clearParams()
{
	params_.clear();
}


void GuiListings::dispatchParams()
{
	string const lfun = InsetListings::params2string(params_);
	dispatch(FuncRequest(getLfun(), lfun));
}


void GuiListings::setParams(InsetListingsParams const & params)
{
	params_ = params;
}


Dialog * createGuiListings(GuiView & lv) { return new GuiListings(lv); }


} // namespace frontend
} // namespace lyx


#include "moc_GuiListings.cpp"
