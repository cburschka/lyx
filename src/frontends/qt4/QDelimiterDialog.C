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

#include "GuiApplication.h"
#include "QMath.h"

#include "qt_helpers.h"
#include "controllers/ControlMath.h"

#include "gettext.h"

#include <QPixmap>
#include <QCheckBox>
#include <QListWidgetItem>

#include <sstream>

// Set to zero if unicode symbols are preferred.
#define USE_PIXMAP 1

using std::string;

namespace lyx {
namespace frontend {

namespace {

char const * const bigleft[]  = {"bigl", "Bigl", "biggl", "Biggl", ""};


char const * const bigright[] = {"bigr", "Bigr", "biggr", "Biggr", ""};


char const * const biggui[]   = {N_("big[[delimiter size]]"), N_("Big[[delimiter size]]"),
	N_("bigg[[delimiter size]]"), N_("Bigg[[delimiter size]]"), ""};


string fix_name(string const & str, bool big)
{
	if (str.empty())
		return ".";
	if (!big || str == "(" || str == ")" || str == "[" || str == "]"
	    || str == "|" || str == "/")
		return str;

	return "\\" + str;
}

} // namespace anon


char_type QDelimiterDialog::doMatch(char_type const symbol) const
{
	string const & str = form_->controller().texName(symbol);
	string match;
	if (str == "(") match = ")";
	else if (str == ")") match = "(";
	else if (str == "[") match = "]";
	else if (str == "]") match = "[";
	else if (str == "{") match = "}";
	else if (str == "}") match = "{";
	else if (str == "l") match = "r";
	else if (str == "rceil") match = "lceil";
	else if (str == "lceil") match = "rceil";
	else if (str == "rfloor") match = "lfloor";
	else if (str == "lfloor") match = "rfloor";
	else if (str == "rangle") match = "langle";
	else if (str == "langle") match = "rangle";
	else if (str == "backslash") match = "/";
	else if (str == "/") match = "backslash";
	else return symbol;

	return form_->controller().mathSymbol(match).unicode;
}


QDelimiterDialog::QDelimiterDialog(QMathDelimiter * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), this, SLOT(accept()));
	connect(insertPB, SIGNAL(clicked()), this, SLOT(insertClicked()));

	setWindowTitle(qt_("LyX: Delimiters"));
	setFocusProxy(leftLW);
	
	typedef std::map<char_type, QListWidgetItem *> ListItems;
	ListItems list_items;
	// The last element is the empty one.
	int const end = nr_latex_delimiters - 1;
	for (int i = 0; i < end; ++i) {
		string const delim = latex_delimiters[i];
		MathSymbol const & ms =	form_->controller().mathSymbol(delim);
		QString symbol(ms.fontcode?
			QChar(ms.fontcode) : toqstr(docstring(1, ms.unicode)));
		QListWidgetItem * lwi = new QListWidgetItem(symbol);
		lwi->setToolTip(toqstr(delim));
		LyXFont lyxfont;
		lyxfont.setFamily(ms.fontfamily);
		QFont const & symbol_font = guiApp->guiFontLoader().get(lyxfont);
		lwi->setFont(symbol_font);
		list_items[ms.unicode] = lwi;
	}

	ListItems::const_iterator it = list_items.begin();
	ListItems::const_iterator it_end = list_items.end();
	for (; it != it_end; ++it) {
		leftLW->addItem(it->second);
		rightLW->addItem(list_items[doMatch(it->first)]->clone());
	}
	// The last element is the empty one.
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
		left_str = fromqstr(leftLW->currentItem()->toolTip());
	if (rightLW->currentRow() < rightLW->count() - 1)
		right_str = fromqstr(rightLW->currentItem()->toolTip());

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
	else
		texCodeL->setText("TeX code: \\" + leftLW->currentItem()->toolTip());
}


void QDelimiterDialog::on_rightLW_currentRowChanged(int item)
{
	if (matchCB->isChecked())
		leftLW->setCurrentRow(item);

	// Display the associated TeX name.
	if (rightLW->currentRow() == leftLW->count() - 1)
		texCodeL->clear();
	else
		texCodeL->setText("TeX code: \\" + rightLW->currentItem()->toolTip());
}


void QDelimiterDialog::on_matchCB_stateChanged(int state)
{
	if (state == Qt::Checked)
		on_leftLW_currentRowChanged(leftLW->currentRow());
}


} // namespace frontend
} // namespace lyx

#include "QDelimiterDialog_moc.cpp"
