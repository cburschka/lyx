/**
 * \file QDelimiterDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QDelimiterDialog.h"

#include "iconpalette.h"
#include "QMath.h"
#include "qt_helpers.h"

#include "controllers/ControlMath.h"

#include "gettext.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <sstream>


using std::string;

namespace lyx {
namespace frontend {

namespace {

char const * delim[] = {
	"(", ")", "{", "}", "[", "]",
	"lceil", "rceil", "lfloor", "rfloor", "langle", "rangle",
	"uparrow", "Uparrow", "downarrow", "Downarrow",
	"|", "Vert", "slash", "backslash", ""
};


char const * const bigleft[]  = {"bigl", "Bigl", "biggl", "Biggl", ""};
char const * const bigright[] = {"bigr", "Bigr", "biggr", "Biggr", ""};
char const * const biggui[]   = {N_("big size"), N_("Big size"),
	N_("bigg size"), N_("Bigg size"), ""};


string do_match(const string & str)
{
	if (str == "(") return ")";
	if (str == ")") return "(";
	if (str == "[") return "]";
	if (str == "]") return "[";
	if (str == "{") return "}";
	if (str == "}") return "{";
	if (str == "l") return "r";
	if (str == "rceil") return "lceil";
	if (str == "lceil") return "rceil";
	if (str == "rfloor") return "lfloor";
	if (str == "lfloor") return "rfloor";
	if (str == "rangle") return "langle";
	if (str == "langle") return "rangle";
	if (str == "backslash") return "slash";
	if (str == "slash") return "backslash";
	return str;
}


string fix_name(const string & str, bool big)
{
	if (str == "slash")
		return "/";
	if (str == "backslash")
		return big ? "\\\\" : "\\";
	if (str == "empty")
		return ".";
	if (!big || str == "(" || str == ")" || str == "[" || str == "]"
	    || str == "|" || str == "/")
		return str;

	return "\\" + str;
}

} // namespace anon


QDelimiterDialog::QDelimiterDialog(QMathDelimiter * form)
	: QDelimiterDialogBase(0, 0, false, 0),
	form_(form)
{
	setCaption(qt_("LyX: Delimiters"));

	for (int i = 0; *delim[i]; ++i) {
		string xpm(find_xpm(delim[i]));
		leftIP->add(QPixmap(toqstr(xpm)), delim[i], delim[i]);
		rightIP->add(QPixmap(toqstr(xpm)), delim[i], delim[i]);
	}

	string empty_xpm(find_xpm("empty"));

	leftIP->add(QPixmap(toqstr(empty_xpm)), "empty", "empty");
	rightIP->add(QPixmap(toqstr(empty_xpm)), "empty", "empty");
	delimSize->insertItem(qt_("Variable size"));
	for (int i = 0; *biggui[i]; ++i)
		delimSize->insertItem(qt_(biggui[i]));
	size_ = 0;
	// Leave these std:: qualifications alone !
	connect(leftIP, SIGNAL(button_clicked(const std::string &)),
		this, SLOT(ldelim_clicked(const std::string &)));
	connect(rightIP, SIGNAL(button_clicked(const std::string &)),
		this, SLOT(rdelim_clicked(const std::string &)));
	connect(delimSize, SIGNAL(activated(int)),
		this, SLOT(size_selected(int)) );
	ldelim_clicked("(");
	rdelim_clicked(")");
}


void QDelimiterDialog::insertClicked()
{
	if (size_ == 0) {
		form_->controller().dispatchDelim(
			fix_name(left_, false) + ' ' +
			fix_name(right_, false));
	} else {
		std::ostringstream os;
		os << '"' << bigleft[size_ - 1] << "\" \""
		   << fix_name(left_, true) << "\" \""
		   << bigright[size_ - 1] << "\" \""
		   << fix_name(right_, true) << '"';
		form_->controller().dispatchBigDelim(os.str());
	}
}


void QDelimiterDialog::set_label(QLabel * label, const string & str)
{
	label->setUpdatesEnabled(false);
	label->setPixmap(QPixmap(toqstr(find_xpm(str))));
	label->setUpdatesEnabled(true);
	label->update();
}


void QDelimiterDialog::ldelim_clicked(const string & str)
{
	left_ = str;

	set_label(leftPI, left_);
	if (matchCB->isChecked()) {
		right_ = do_match(left_);
		set_label(rightPI, right_);
	}
}


void QDelimiterDialog::rdelim_clicked(const string & str)
{
	right_ = str;

	set_label(rightPI, right_);
	if (matchCB->isChecked()) {
		left_ = do_match(right_);
		set_label(leftPI, left_);
	}
}


void QDelimiterDialog::size_selected(int index)
{
	size_ = index;
}

} // namespace frontend
} // namespace lyx
