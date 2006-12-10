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
#include "QMath.h"

#include "qt_helpers.h"
#include "controllers/ControlMath.h"

#include "gettext.h"

#include <QPixmap>
#include <QCheckBox>


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


char const * const biggui[]   = {N_("big[[delimiter size]]"), N_("Big[[delimiter size]]"),
	N_("bigg[[delimiter size]]"), N_("Bigg[[delimiter size]]"), ""};


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
		return "\\";
	if (str == "empty")
		return ".";
	if (!big || str == "(" || str == ")" || str == "[" || str == "]")
		return str;

	return "\\" + str;
}

} // namespace anon


QDelimiterDialog::QDelimiterDialog(QMathDelimiter * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), this, SLOT(accept()));
	connect(insertPB, SIGNAL(clicked()), this, SLOT(insertClicked()));

	setWindowTitle(qt_("LyX: Delimiters"));

	for (int i = 0; *delim[i]; ++i) {
		QPixmap pm = QPixmap(toqstr(find_xpm(delim[i])));
		leftCO->addItem(QIcon(pm), "");
		rightCO->addItem(QIcon(pm), "");
	}

	string empty_xpm(find_xpm("empty"));
	leftCO->addItem(QIcon(QPixmap(toqstr(empty_xpm))), qt_("(None)"));
	rightCO->addItem(QIcon(QPixmap(toqstr(empty_xpm))), qt_("(None)"));

	sizeCO->addItem(qt_("Variable"));

	for (int i = 0; *biggui[i]; ++i)
		sizeCO->addItem(qt_(biggui[i]));

	on_leftCO_activated(0);
}


void QDelimiterDialog::insertClicked()
{
	string const left_ = delim[leftCO->currentIndex()];
	string const right_ = delim[rightCO->currentIndex()];
	int const size_ = sizeCO->currentIndex();

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


void QDelimiterDialog::on_leftCO_activated(int item)
{
	if (matchCB->isChecked()) {
		string const match = do_match(delim[item]);
		int k = 0;
		while (delim[k] && delim[k] != match)
			++k;
		rightCO->setCurrentIndex(k);
	}
}


void QDelimiterDialog::on_rightCO_activated(int item)
{
	if (matchCB->isChecked()) {
		string const match = do_match(delim[item]);
		int k = 0;
		while (delim[k] && delim[k] != match)
			++k;
		leftCO->setCurrentIndex(k);
	}
}


void QDelimiterDialog::on_matchCB_stateChanged(int state)
{
	if (state == Qt::Checked)
		on_leftCO_activated(leftCO->currentIndex());
}


} // namespace frontend
} // namespace lyx

#include "QDelimiterDialog_moc.cpp"
