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

QString const bigleft[]  = {"bigl", "Bigl", "biggl", "Biggl", ""};


QString const bigright[] = {"bigr", "Bigr", "biggr", "Biggr", ""};


char const * const biggui[]   = {N_("big[[delimiter size]]"), N_("Big[[delimiter size]]"),
	N_("bigg[[delimiter size]]"), N_("Bigg[[delimiter size]]"), ""};


QString fix_name(QString const & str, bool big)
{
	if (str.isEmpty())
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


void QDelimiterDialog::updateTeXCode(int size)
{
	QString left_str;
	QString right_str;
	QString bigl;
	QString bigr;
	QString code_str;
	bool bigsize = size != 0;

	left_str = fix_name(leftLW->currentItem()->toolTip(), bigsize);
	right_str = fix_name(rightLW->currentItem()->toolTip(), bigsize);

	if (!bigsize)
		tex_code_ = left_str + ' ' + right_str;
	else {
		tex_code_ = bigleft[size - 1] + '  '
			+ left_str + ' ' 
			+ bigright[size - 1] + ' '
			+ right_str;
		}

	// generate TeX-code
	left_str = fix_name(leftLW->currentItem()->toolTip(), true);
	right_str = fix_name(rightLW->currentItem()->toolTip(), true);
	if (bigsize == true) {
		bigl = "\\" + bigleft[size];
		bigr = "\\" + bigright[size];
	}
	if (!bigsize)
		code_str = "TeX-Code: \\left" + left_str + ' ' + "\\right" + right_str;
	else {
		// There is nothing in the TeX-code when the delimiter is "None"
		if (left_str == ".") {
			left_str = "";
			bigl = "";
		}
		if (right_str == ".") {
			right_str = "";
			bigr = "";
		}
		code_str = "TeX-Code: " + bigl
			+ left_str + '  ' 
			+ bigr
			+ right_str;
	}

	texCodeL->setText(code_str);
}

void QDelimiterDialog::on_insertPB_clicked()
{
	if (sizeCO->currentIndex() == 0)
		form_->controller().dispatchDelim(fromqstr(tex_code_));
	else {
		QString command = '"' + tex_code_ + '"';
		command.replace(' ', "\" \"");
		form_->controller().dispatchBigDelim(fromqstr(command));
 	}
 }

 
void QDelimiterDialog::on_sizeCO_activated(int index)
{
	updateTeXCode(index);
}


void QDelimiterDialog::on_leftLW_itemActivated(QListWidgetItem *)
{
	on_insertPB_clicked();
	accept();
}


void QDelimiterDialog::on_rightLW_itemActivated(QListWidgetItem *)
{
	on_insertPB_clicked();
	accept();
}


void QDelimiterDialog::on_leftLW_currentRowChanged(int item)
{
	if (matchCB->isChecked())
		rightLW->setCurrentRow(item);

	updateTeXCode(sizeCO->currentIndex());
}


void QDelimiterDialog::on_rightLW_currentRowChanged(int item)
{
	if (matchCB->isChecked())
		leftLW->setCurrentRow(item);

	updateTeXCode(sizeCO->currentIndex());
}


void QDelimiterDialog::on_matchCB_stateChanged(int state)
{
	if (state == Qt::Checked)
		on_leftLW_currentRowChanged(leftLW->currentRow());
}


} // namespace frontend
} // namespace lyx

#include "QDelimiterDialog_moc.cpp"
