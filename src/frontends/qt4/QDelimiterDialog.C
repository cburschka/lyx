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


QString do_match(QString const & str)
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


string fix_name(QString const & str, bool big)
{
	if (str == "slash")
		return "/";
	if (str == "backslash")
		return "\\";
	if (str.isEmpty())
		return ".";
	if (!big || str == "(" || str == ")" || str == "[" || str == "]")
		return fromqstr(str);

	return "\\" + fromqstr(str);
}

} // namespace anon


QDelimiterDialog::QDelimiterDialog(QMathDelimiter * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), this, SLOT(accept()));
	connect(insertPB, SIGNAL(clicked()), this, SLOT(insertClicked()));

	setWindowTitle(qt_("LyX: Delimiters"));

	for (size_t i = 0; i != 21; ++i)
		delimiters_.append(toqstr(delim[i]));

	// The last element is the empty one.
	size_t end = delimiters_.size() - 1;
	for (size_t i = 0; i != end; ++i) {
		if (delimiters_[i].size() == 1) {
			leftCO->addItem(delimiters_[i]);
			rightCO->addItem(delimiters_[i]);
		} else {
			QPixmap pm = QPixmap(toqstr(find_xpm(fromqstr(delimiters_[i]))));
			leftCO->addItem(QIcon(pm), delimiters_[i]);
			rightCO->addItem(QIcon(pm), delimiters_[i]);
		}
	}

	leftCO->addItem(qt_("(None)"));
	rightCO->addItem(qt_("(None)"));

	sizeCO->addItem(qt_("Variable"));

	for (int i = 0; *biggui[i]; ++i)
		sizeCO->addItem(qt_(biggui[i]));

	on_leftCO_activated(0);
}


void QDelimiterDialog::insertClicked()
{
	QString const left_ = delimiters_[leftCO->currentIndex()];
	QString const right_ = delimiters_[rightCO->currentIndex()];
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
		QString const match = do_match(delimiters_[item]);
		rightCO->setCurrentIndex(delimiters_.indexOf(match));
	}
}


void QDelimiterDialog::on_rightCO_activated(int item)
{
	if (matchCB->isChecked()) {
		QString const match = do_match(delimiters_[item]);
		leftCO->setCurrentIndex(delimiters_.indexOf(match));
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
