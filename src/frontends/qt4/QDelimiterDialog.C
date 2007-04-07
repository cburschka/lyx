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

char const * const bigleft[]  = {"bigl", "Bigl", "biggl", "Biggl", ""};


char const * const bigright[] = {"bigr", "Bigr", "biggr", "Biggr", ""};


char const * const biggui[]   = {N_("big[[delimiter size]]"), N_("Big[[delimiter size]]"),
	N_("bigg[[delimiter size]]"), N_("Bigg[[delimiter size]]"), ""};


string do_match(string const & str)
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
	if (str == "\\") return "/";
	if (str == "/") return "\\";
	return str;
}


string fix_name(string const & str, bool big)
{
	if (str.empty())
		return ".";
	if (!big || str == "(" || str == ")" || str == "[" || str == "]" || str == "|")
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
	setFocusProxy(leftLW);
	
	// The last element is the empty one.
	for (int i = 0; i < nr_latex_delimiters - 1; ++i) {
		docstring const left_d(1,
			form_->controller().mathSymbol(latex_delimiters[i]));
		docstring const right_d(1,
			form_->controller().mathSymbol(do_match(latex_delimiters[i])));
		leftLW->addItem(toqstr(left_d));
		rightLW->addItem(toqstr(right_d));
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
	string left_str;
	string right_str;
	if (leftLW->currentRow() < leftLW->count() - 1)
		left_str = form_->controller().texName(qstring_to_ucs4(leftLW->currentItem()->text())[0]);
	if (rightLW->currentRow() < rightLW->count() - 1)
		right_str = form_->controller().texName(qstring_to_ucs4(rightLW->currentItem()->text())[0]);

	int const size_ = sizeCO->currentIndex();
	if (size_ == 0) {
		form_->controller().dispatchDelim(
			fix_name(left_str, false) + ' ' +
			fix_name(right_str, false));
	} else {
		std::ostringstream os;
		os << '"' << bigleft[size_ - 1] << "\" \""
		   << fix_name(left_str, true) << "\" \""
		   << bigright[size_ - 1] << "\" \""
		   << fix_name(right_str, true) << '"';
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

	// Display the associated TeX name.
	if (leftLW->currentRow() == leftLW->count() - 1)
		texCodeL->clear();
	else {
		QString const str = toqstr(form_->controller().texName(
			qstring_to_ucs4(leftLW->currentItem()->text())[0]));
		texCodeL->setText("TeX code: \\" + str);
	}
}


void QDelimiterDialog::on_rightLW_currentRowChanged(int item)
{
	if (matchCB->isChecked())
		leftLW->setCurrentRow(item);

	// Display the associated TeX name.
	if (rightLW->currentRow() == leftLW->count() - 1)
		texCodeL->clear();
	else {
		QString const str = toqstr(form_->controller().texName(
			qstring_to_ucs4(rightLW->currentItem()->text())[0]));
		texCodeL->setText("TeX code: \\" + str);
	}
}


void QDelimiterDialog::on_matchCB_stateChanged(int state)
{
	if (state == Qt::Checked)
		on_leftLW_currentRowChanged(leftLW->currentRow());
}


} // namespace frontend
} // namespace lyx

#include "QDelimiterDialog_moc.cpp"
