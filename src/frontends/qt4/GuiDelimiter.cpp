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

#include "support/gettext.h"
#include "support/docstring.h"

#include <QPixmap>
#include <QCheckBox>
#include <QListWidgetItem>

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

static QString const bigleft[]  = {"", "bigl", "Bigl", "biggl", "Biggl"};

static QString const bigright[] = {"", "bigr", "Bigr", "biggr", "Biggr"};

static char const * const biggui[] = {
	N_("big[[delimiter size]]"),
	N_("Big[[delimiter size]]"),
	N_("bigg[[delimiter size]]"),
	N_("Bigg[[delimiter size]]"),
	""
};


// FIXME: It might be better to fix the big delim LFUN to not require
// additional '\' prefix.
static QString fix_name(QString const & str, bool big)
{
	if (str.isEmpty())
		return ".";
	if (!big || str == "(" || str == ")" || str == "[" || str == "]"
	    || str == "|" || str == "/")
		return str;

	return "\\" + str;
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
	math_symbols_["("] = MathSymbol('(');
	math_symbols_[")"] = MathSymbol(')');
	math_symbols_["{"] = MathSymbol('{');
	math_symbols_["}"] = MathSymbol('}');
	math_symbols_["["] = MathSymbol('[');
	math_symbols_["]"] = MathSymbol(']');
	math_symbols_["|"] = MathSymbol('|');
	math_symbols_["/"] = MathSymbol('/', 54, CMSY_FAMILY);
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
MathSymbol const & mathSymbol(string tex_name)
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

} // anon namespace


GuiDelimiter::GuiDelimiter(GuiView & lv)
	: GuiDialog(lv, "mathdelimiter", qt_("Math Delimiter"))
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), this, SLOT(accept()));

	setFocusProxy(leftLW);

	leftLW->setViewMode(QListView::IconMode);
	rightLW->setViewMode(QListView::IconMode);

	leftLW->setDragDropMode(QAbstractItemView::NoDragDrop);
	rightLW->setDragDropMode(QAbstractItemView::NoDragDrop);

	initMathSymbols();

	typedef map<char_type, QListWidgetItem *> ListItems;
	ListItems list_items;
	// The last element is the empty one.
	int const end = nr_latex_delimiters - 1;
	for (int i = 0; i < end; ++i) {
		string const delim = latex_delimiters[i];
		MathSymbol const & ms =	mathSymbol(delim);
// Due to a bug in Qt 4 on Windows, we need to use our math symbol font
// on Windows, which results in sub-optimal glyph display (see #5760).
// FIXME: Re-check after Windows has settled to Qt 5.
#if defined(_WIN32)
		QString symbol(ms.fontcode?
			QChar(ms.fontcode) : toqstr(docstring(1, ms.unicode)));
		QListWidgetItem * lwi = new QListWidgetItem(symbol);
		FontInfo lyxfont;
		lyxfont.setFamily(ms.fontfamily);
		QFont font = frontend::getFont(lyxfont);
#else
		QString symbol(QChar(ms.unicode));
		QListWidgetItem * lwi = new QListWidgetItem(symbol);
		QFont font = lwi->font();
		font.setPointSize(2 * font.pointSize());
#endif
		lwi->setFont(font);
		lwi->setToolTip(toqstr(delim));
		list_items[ms.unicode] = lwi;
		leftLW->addItem(lwi);
	}

	for (int i = 0; i != leftLW->count(); ++i) {
		MathSymbol const & ms =	mathSymbol(
			fromqstr(leftLW->item(i)->toolTip()));
		rightLW->addItem(list_items[doMatch(ms.unicode)]->clone());
	}

	// The last element is the empty one.
	QListWidgetItem * lwi = new QListWidgetItem(qt_("(None)"));
	QListWidgetItem * rwi = new QListWidgetItem(qt_("(None)"));
// See above comment.
// FIXME: Re-check after Windows has settled to Qt 5.
#if !defined(_WIN32)
	QFont font = lwi->font();
	font.setPointSize(2 * font.pointSize());
	lwi->setFont(font);
	rwi->setFont(font);
#endif
	leftLW->addItem(lwi);
	rightLW->addItem(rwi);

	sizeCO->addItem(qt_("Variable"));

	for (int i = 0; *biggui[i]; ++i)
		sizeCO->addItem(qt_(biggui[i]));

	on_leftLW_currentRowChanged(0);
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

	QString left_str = fix_name(leftLW->currentItem()->toolTip(), bigsize);
	QString right_str = fix_name(rightLW->currentItem()->toolTip(), bigsize);

	if (!bigsize)
		tex_code_ = left_str + ' ' + right_str;
	else {
		tex_code_ = bigleft[size] + ' '
			+ left_str + ' '
			+ bigright[size] + ' '
			+ right_str;
	}

	// Generate TeX-code for GUI display.
	// FIXME: Instead of reconstructing the TeX code it would be nice to
	// FIXME: retrieve the LateX code directly from mathed.
	// In all cases, we want the '\' prefix if needed, so we pass 'true'
	// to fix_name.
	left_str = fix_name(leftLW->currentItem()->toolTip(), true);
	right_str = fix_name(rightLW->currentItem()->toolTip(), true);
	QString code_str;
	if (!bigsize)
		code_str = "\\left" + left_str + " \\right" + right_str;
	else {
		// There should be nothing in the TeX-code when the delimiter is "None".
		if (left_str != ".")
			code_str = "\\" + bigleft[size] + left_str + ' ';
		if (right_str != ".")
			code_str += "\\" + bigright[size] + right_str;
	}

	texCodeL->setText(qt_("TeX Code: ") + code_str);
}


void GuiDelimiter::on_insertPB_clicked()
{
	if (sizeCO->currentIndex() == 0)
		dispatch(FuncRequest(LFUN_MATH_DELIM, fromqstr(tex_code_)));
	else {
		QString command = '"' + tex_code_ + '"';
		command.replace(' ', "\" \"");
		dispatch(FuncRequest(LFUN_MATH_BIGDELIM, fromqstr(command)));
	}
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
	on_insertPB_clicked();
	accept();
}


void GuiDelimiter::on_rightLW_itemActivated(QListWidgetItem *)
{
	// do not auto-apply if !matchCB->isChecked()
	if (!matchCB->isChecked())
		return;
	on_insertPB_clicked();
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
	if (state == Qt::Checked)
		on_leftLW_currentRowChanged(leftLW->currentRow());

	updateTeXCode(sizeCO->currentIndex());
}


Dialog * createGuiDelimiter(GuiView & lv) { return new GuiDelimiter(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiDelimiter.cpp"
