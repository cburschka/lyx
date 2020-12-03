/**
 * \file GuiDelimiter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiDelimiter.h"

#include "GuiApplication.h"
#include "GuiFontLoader.h"
#include "GuiView.h"
#include "qt_helpers.h"

#include "FontEnums.h"
#include "FontInfo.h"
#include "FuncRequest.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QPixmap>
#include <QCheckBox>
#include <QListWidgetItem>
#include <QScrollBar>

#include <map>
#include <string>

using namespace std;

namespace lyx {
namespace frontend {

namespace {

static char const *  latex_delimiters[] = {
	"(", ")", "{", "}", "[", "]",
	"lceil", "rceil", "lfloor", "rfloor", "langle", "rangle",
	"llbracket", "rrbracket",
	"uparrow", "updownarrow", "Uparrow", "Updownarrow", "downarrow", "Downarrow",
	"|", "Vert", "/", "backslash", ""
};


static int const nr_latex_delimiters =
	sizeof(latex_delimiters) / sizeof(char const *);

static string const bigleft[]  = {"", "bigl", "Bigl", "biggl", "Biggl"};

static string const bigright[] = {"", "bigr", "Bigr", "biggr", "Biggr"};

static char const * const biggui[] = {
	N_("big[[delimiter size]]"),
	N_("Big[[delimiter size]]"),
	N_("bigg[[delimiter size]]"),
	N_("Bigg[[delimiter size]]"),
	""
};


// FIXME: It might be better to fix the big delim LFUN to not require
// additional '\' prefix.
static docstring fix_name(string const & str, bool big)
{
	if (str.empty())
		return from_ascii(".");
	if (!big || str == "(" || str == ")" || str == "[" || str == "]"
	    || str == "|" || str == "/")
		return from_ascii(str);

	return "\\" + from_ascii(str);
}

struct MathSymbol {
	MathSymbol(char_type uc = '?', unsigned char fc = 0,
		FontFamily ff = SYMBOL_FAMILY)
		: unicode(uc), fontcode(fc), fontfamily(ff)
	{}
	char_type unicode;
	unsigned char fontcode;
	FontFamily fontfamily;
};

/// TeX-name / Math-symbol map.
static map<std::string, MathSymbol> math_symbols_;
/// Math-symbol / TeX-name map.
/// This one is for fast search, it contains the same data as
/// \c math_symbols_.
static map<char_type, string> tex_names_;

void initMathSymbols()
{
	// FIXME: Ideally, those unicode codepoints would be defined
	// in "lib/symbols". Unfortunately, some of those are already
	// defined with non-unicode ids for use within mathed.
	// FIXME 2: We should fill-in this map with the parsed "symbols"
	// file done in MathFactory.cpp.
	math_symbols_["("] = MathSymbol('(', 40, CMR_FAMILY);
	math_symbols_[")"] = MathSymbol(')', 41, CMR_FAMILY);
	math_symbols_["{"] = MathSymbol('{', 102, CMSY_FAMILY);
	math_symbols_["}"] = MathSymbol('}', 103, CMSY_FAMILY);
	math_symbols_["["] = MathSymbol('[', 91, CMR_FAMILY);
	math_symbols_["]"] = MathSymbol(']', 93, CMR_FAMILY);
	math_symbols_["|"] = MathSymbol('|', 106, CMSY_FAMILY);
	math_symbols_["/"] = MathSymbol('/', 47, CMR_FAMILY);
	math_symbols_["backslash"] = MathSymbol('\\', 110, CMSY_FAMILY);
	math_symbols_["lceil"] = MathSymbol(0x2308, 100, CMSY_FAMILY);
	math_symbols_["rceil"] = MathSymbol(0x2309, 101, CMSY_FAMILY);
	math_symbols_["lfloor"] = MathSymbol(0x230A, 98, CMSY_FAMILY);
	math_symbols_["rfloor"] = MathSymbol(0x230B, 99, CMSY_FAMILY);
	math_symbols_["langle"] = MathSymbol(0x2329, 104, CMSY_FAMILY);
	math_symbols_["rangle"] = MathSymbol(0x232A, 105, CMSY_FAMILY);
	math_symbols_["llbracket"] = MathSymbol(0x27e6, 74, STMARY_FAMILY);
	math_symbols_["rrbracket"] = MathSymbol(0x27e7, 75, STMARY_FAMILY);
	math_symbols_["uparrow"] = MathSymbol(0x2191, 34, CMSY_FAMILY);
	math_symbols_["Uparrow"] = MathSymbol(0x21D1, 42, CMSY_FAMILY);
	math_symbols_["updownarrow"] = MathSymbol(0x2195, 108, CMSY_FAMILY);
	math_symbols_["Updownarrow"] = MathSymbol(0x21D5, 109, CMSY_FAMILY);
	math_symbols_["downarrow"] = MathSymbol(0x2193, 35, CMSY_FAMILY);
	math_symbols_["Downarrow"] = MathSymbol(0x21D3, 43, CMSY_FAMILY);
	math_symbols_["downdownarrows"] = MathSymbol(0x21CA, 184, MSA_FAMILY);
	math_symbols_["downharpoonleft"] = MathSymbol(0x21C3, 188, MSA_FAMILY);
	math_symbols_["downharpoonright"] = MathSymbol(0x21C2, 186, MSA_FAMILY);
	math_symbols_["vert"] = MathSymbol(0x007C, 106, CMSY_FAMILY);
	math_symbols_["Vert"] = MathSymbol(0x2016, 107, CMSY_FAMILY);

	map<string, MathSymbol>::const_iterator it = math_symbols_.begin();
	map<string, MathSymbol>::const_iterator end = math_symbols_.end();
	for (; it != end; ++it)
		tex_names_[it->second.unicode] = it->first;
}

/// \return the math unicode symbol associated to a TeX name.
MathSymbol const & mathSymbol(string const & tex_name)
{
	map<string, MathSymbol>::const_iterator it =
		math_symbols_.find(tex_name);

	static MathSymbol const unknown_symbol;
	if (it == math_symbols_.end())
		return unknown_symbol;

	return it->second;
}

/// \return the TeX name associated to a math unicode symbol.
string const & texName(char_type math_symbol)
{
	map<char_type, string>::const_iterator it =
		tex_names_.find(math_symbol);

	static string const empty_string;
	if (it == tex_names_.end())
		return empty_string;

	return it->second;
}


void setDelimiterName(QListWidgetItem * lwi, string const & name)
{
	lwi->setData(Qt::UserRole, toqstr(name));
}


string getDelimiterName(QListWidgetItem const * lwi)
{
	return fromqstr(lwi->data(Qt::UserRole).toString());
}


} // namespace


GuiDelimiter::GuiDelimiter(GuiView & lv)
	: GuiDialog(lv, "mathdelimiter", qt_("Math Delimiter"))
{
	setupUi(this);

	connect(buttonBox, SIGNAL(rejected()), this, SLOT(accept()));

	setFocusProxy(leftLW);

	leftLW->setViewMode(QListView::IconMode);
	rightLW->setViewMode(QListView::IconMode);

	leftLW->setDragDropMode(QAbstractItemView::NoDragDrop);
	rightLW->setDragDropMode(QAbstractItemView::NoDragDrop);

	initMathSymbols();

	FontInfo lyxfont;
	lyxfont.setFamily(CMR_FAMILY);
	QFontMetrics fm(frontend::getFont(lyxfont));
	QSize item_size(fm.maxWidth(), fm.height() + 8);

	leftLW->setMinimumWidth(5 * item_size.width());
	rightLW->setMinimumWidth(5 * item_size.width());

	typedef map<char_type, QListWidgetItem *> ListItems;
	ListItems list_items;
	// The last element is the empty one.
	int const end = nr_latex_delimiters - 1;
	for (int i = 0; i < end; ++i) {
		string const delim = latex_delimiters[i];
		MathSymbol const & ms = mathSymbol(delim);
		QString symbol(ms.fontcode?
			QChar(ms.fontcode) : toqstr(docstring(1, ms.unicode)));
		QListWidgetItem * lwi = new QListWidgetItem(symbol);
		lyxfont.setFamily(ms.fontfamily);
		QFont const & font = frontend::getFont(lyxfont);
		lwi->setFont(font);
		setDelimiterName(lwi, delim);
		lwi->setToolTip(toqstr(delim));
		lwi->setSizeHint(item_size);
		switch (ms.fontfamily) {
		case CMSY_FAMILY:
		case STMARY_FAMILY:
			// Hack to work around the broken metrics of these fonts
			// FIXME: Better fix the fonts or use fonts that are not broken
			lwi->setTextAlignment(Qt::AlignTop | Qt::AlignHCenter);
			break;
		default:
			lwi->setTextAlignment(Qt::AlignCenter);
		}
		list_items[ms.unicode] = lwi;
		leftLW->addItem(lwi);
	}

	for (int i = 0; i != leftLW->count(); ++i) {
		MathSymbol const & ms = mathSymbol(getDelimiterName(leftLW->item(i)));
		rightLW->addItem(list_items[doMatch(ms.unicode)]->clone());
	}

	// The last element is the empty one.
	QListWidgetItem * lwi = new QListWidgetItem(qt_("(None)"));
	QListWidgetItem * rwi = new QListWidgetItem(qt_("(None)"));
	leftLW->addItem(lwi);
	rightLW->addItem(rwi);

	sizeCO->addItem(qt_("Variable"));

	for (int i = 0; *biggui[i]; ++i)
		sizeCO->addItem(qt_(biggui[i]));

	on_leftLW_currentRowChanged(0);
	// synchronise the scroll bars
	on_matchCB_stateChanged(matchCB->checkState());
	bc().setPolicy(ButtonPolicy::IgnorantPolicy);
}


char_type GuiDelimiter::doMatch(char_type const symbol)
{
	string const & str = texName(symbol);
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
	else if (str == "llbracket") match = "rrbracket";
	else if (str == "rrbracket") match = "llbracket";
	else if (str == "backslash") match = "/";
	else if (str == "/") match = "backslash";
	else return symbol;

	return mathSymbol(match).unicode;
}


void GuiDelimiter::updateTeXCode(int size)
{
	bool const bigsize = size != 0;

	docstring left_str = fix_name(getDelimiterName(leftLW->currentItem()),
	                              bigsize);
	docstring right_str = fix_name(getDelimiterName(rightLW->currentItem()),
	                               bigsize);

	if (!bigsize)
		tex_code_ = left_str + ' ' + right_str;
	else {
		tex_code_ = from_ascii(bigleft[size]) + ' '
			+ left_str + ' '
			+ from_ascii(bigright[size]) + ' '
			+ right_str;
	}

	// Generate TeX-code for GUI display.
	// FIXME: Instead of reconstructing the TeX code it would be nice to
	// FIXME: retrieve the LateX code directly from mathed.
	// In all cases, we want the '\' prefix if needed, so we pass 'true'
	// to fix_name.
	left_str = fix_name(getDelimiterName(leftLW->currentItem()),
	                    true);
	right_str = fix_name(getDelimiterName(rightLW->currentItem()),
	                     true);
	docstring code_str;
	if (!bigsize)
		code_str = "\\left" + left_str + " \\right" + right_str;
	else {
		// There should be nothing in the TeX-code when the delimiter is "None".
		if (left_str != ".")
			code_str = "\\" + from_ascii(bigleft[size]) + left_str + ' ';
		if (right_str != ".")
			code_str += "\\" + from_ascii(bigright[size]) + right_str;
	}

	texCodeL->setText(qt_("TeX Code: ") + toqstr(code_str));

	// Enable the Swap button with non-matched pairs
	bool const allow_swap =
		(doMatch(mathSymbol(getDelimiterName(leftLW->currentItem())).unicode)
		 != mathSymbol(getDelimiterName(rightLW->currentItem())).unicode);
	swapPB->setEnabled(allow_swap);
}


void  GuiDelimiter::on_buttonBox_clicked(QAbstractButton * button)
{
	switch (buttonBox->standardButton(button)) {
	case QDialogButtonBox::Apply:
		insert();
		break;
	case QDialogButtonBox::Ok:
		insert();
	// fall through
	case QDialogButtonBox::Cancel:
		accept();
		break;
	default:
		break;
	}
}


void GuiDelimiter::insert()
{
	if (sizeCO->currentIndex() == 0)
		dispatch(FuncRequest(LFUN_MATH_DELIM, tex_code_));
	else {
		docstring command = '"' + tex_code_ + '"';
		command = support::subst(command, from_ascii(" "), from_ascii("\" \""));
		dispatch(FuncRequest(LFUN_MATH_BIGDELIM, command));
	}
	buttonBox->button(QDialogButtonBox::Cancel)->setText(qt_("Close"));
}


void GuiDelimiter::on_sizeCO_activated(int index)
{
	updateTeXCode(index);
}


void GuiDelimiter::on_leftLW_itemActivated(QListWidgetItem *)
{
	// do not auto-apply if !matchCB->isChecked()
	if (!matchCB->isChecked())
		return;
	insert();
	accept();
}


void GuiDelimiter::on_rightLW_itemActivated(QListWidgetItem *)
{
	// do not auto-apply if !matchCB->isChecked()
	if (!matchCB->isChecked())
		return;
	insert();
	accept();
}


void GuiDelimiter::on_leftLW_currentRowChanged(int item)
{
	if (matchCB->isChecked())
		rightLW->setCurrentRow(item);

	updateTeXCode(sizeCO->currentIndex());
}


void GuiDelimiter::on_rightLW_currentRowChanged(int item)
{
	if (matchCB->isChecked())
		leftLW->setCurrentRow(item);

	updateTeXCode(sizeCO->currentIndex());
}


void GuiDelimiter::on_matchCB_stateChanged(int state)
{
	// Synchronise the vertical scroll bars when checked
	QScrollBar * ls = leftLW->verticalScrollBar();
	QScrollBar * rs = rightLW->verticalScrollBar();

	if (state == Qt::Checked) {
		on_leftLW_currentRowChanged(leftLW->currentRow());

		connect(ls, SIGNAL(valueChanged(int)), rs, SLOT(setValue(int)),
		        Qt::UniqueConnection);
		connect(rs, SIGNAL(valueChanged(int)), ls, SLOT(setValue(int)),
		        Qt::UniqueConnection);
		rs->setValue(ls->value());
	} else {
		ls->disconnect(rs);
		rs->disconnect(ls);
	}

	updateTeXCode(sizeCO->currentIndex());
}

void GuiDelimiter::on_swapPB_clicked()
{
	// Get current math symbol for each side.
	MathSymbol const & lms =
		mathSymbol(getDelimiterName(leftLW->currentItem()));
	MathSymbol const & rms =
		mathSymbol(getDelimiterName(rightLW->currentItem()));

	// Swap and match.
	char_type const lc = doMatch(rms.unicode);
	char_type const rc = doMatch(lms.unicode);

	// Convert back to QString to locate them in the widget.
	MathSymbol const & nlms = mathSymbol(texName(lc));
	MathSymbol const & nrms = mathSymbol(texName(rc));
	QString lqs(nlms.fontcode ?
		QChar(nlms.fontcode) : toqstr(docstring(1, nlms.unicode)));
	QString rqs(nrms.fontcode ?
		QChar(nrms.fontcode) : toqstr(docstring(1, nrms.unicode)));

	// Handle unencoded "symbol" of "(None)".
	if (lqs == "?")
		lqs = qt_("(None)");
	if(rqs == "?")
		rqs = qt_("(None)");

	// Locate matching QListWidgetItem.
	QList<QListWidgetItem *> lwi = leftLW->findItems(lqs, Qt::MatchExactly);
	QList<QListWidgetItem *> rwi = rightLW->findItems(rqs, Qt::MatchExactly);

	// Select.
	leftLW->setCurrentItem(lwi.first());
	rightLW->setCurrentItem(rwi.first());

	updateTeXCode(sizeCO->currentIndex());
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiDelimiter.cpp"
