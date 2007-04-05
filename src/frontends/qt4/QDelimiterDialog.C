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
#include <QListWidgetItem>

#include <sstream>

using std::string;

namespace lyx {
namespace frontend {

namespace {

string const delim[] = {
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
	setFocusProxy(leftLW);

	// The last element is the empty one.
	for (size_t i = 0; !delim[i].empty(); ++i) {
		QString const left_d = toqstr(delim[i]);
		QString const right_d = do_match(left_d);
		if (left_d.size() == 1) {
			leftLW->addItem(left_d);
			rightLW->addItem(right_d);
		} else {
			QPixmap left_pm = QPixmap(toqstr(find_xpm(fromqstr(left_d))));
			leftLW->addItem(new QListWidgetItem(QIcon(left_pm), left_d));
			QPixmap right_pm = QPixmap(toqstr(find_xpm(fromqstr(right_d))));
			rightLW->addItem(new QListWidgetItem(QIcon(right_pm), right_d));
		}
	}

	leftLW->addItem(qt_("(None)"));
	rightLW->addItem(qt_("(None)"));

	sizeCO->addItem(qt_("Variable"));

	for (int i = 0; *biggui[i]; ++i)
		sizeCO->addItem(qt_(biggui[i]));

	on_leftLW_currentRowChanged(0);
}


void QDelimiterDialog::insertClicked()
{
	QString const left_ = (leftLW->currentRow() < leftLW->count() - 1)?
		leftLW->currentItem()->text(): QString();
	QString const right_ = (rightLW->currentRow() < rightLW->count() - 1)?
		rightLW->currentItem()->text(): QString();
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


void QDelimiterDialog::on_leftLW_itemActivated(QListWidgetItem *)
{
	insertClicked();
	accept();
}


void QDelimiterDialog::on_rightLW_itemActivated(QListWidgetItem *)
{
	insertClicked();
	accept();
}


void QDelimiterDialog::on_leftLW_currentRowChanged(int item)
{
	if (matchCB->isChecked())
		rightLW->setCurrentRow(item);
}


void QDelimiterDialog::on_rightLW_currentRowChanged(int item)
{
	if (matchCB->isChecked())
		leftLW->setCurrentRow(item);
}


void QDelimiterDialog::on_matchCB_stateChanged(int state)
{
	if (state == Qt::Checked)
		on_leftLW_currentRowChanged(leftLW->currentRow());
}


} // namespace frontend
} // namespace lyx

#include "QDelimiterDialog_moc.cpp"
