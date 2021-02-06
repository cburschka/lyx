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

#include <QBitmap>
#include <QPainter>
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
	MathSymbol(char_type uc = '?', string icon = string())
		: unicode(uc), icon(icon)
	{}
	char_type unicode;
	string icon;
};


QPixmap getSelectedPixmap(QPixmap pixmap)
{
	QPalette palette = QPalette();
	QColor text_color = (guiApp->isInDarkMode())
			? palette.color(QPalette::Active, QPalette::WindowText)
			: Qt::black;
	QColor highlight_color = palette.color(QPalette::Active, QPalette::HighlightedText);

	// create a layer with black text turned to QPalette::HighlightedText
	QPixmap hl_overlay(pixmap.size());
	hl_overlay.fill(highlight_color);
	hl_overlay.setMask(pixmap.createMaskFromColor(text_color, Qt::MaskOutColor));

	// put layers on top of existing pixmap
	QPainter painter(&pixmap);
	painter.drawPixmap(pixmap.rect(), hl_overlay);

	return pixmap;
}

/// TeX-name / Math-symbol map.
static map<std::string, MathSymbol> math_symbols_;
/// Math-symbol / TeX-name map.
/// This one is for fast search, it contains the same data as
/// \c math_symbols_.
static map<char_type, string> tex_names_;

typedef map<char_type, QListWidgetItem *> ListItems;
ListItems left_list_items_;
ListItems right_list_items_;

void initMathSymbols()
{
	// FIXME: Ideally, those unicode codepoints would be defined
	// in "lib/symbols". Unfortunately, some of those are already
	// defined with non-unicode ids for use within mathed.
	// FIXME 2: We should fill-in this map with the parsed "symbols"
	// file done in MathFactory.cpp.
	math_symbols_["("] = MathSymbol('(', "lparen");
	math_symbols_[")"] = MathSymbol(')', "rparen");
	math_symbols_["{"] = MathSymbol('{', "lbrace");
	math_symbols_["}"] = MathSymbol('}', "rbrace");
	math_symbols_["["] = MathSymbol('[', "lbracket");
	math_symbols_["]"] = MathSymbol(']', "rbracket");
	math_symbols_["|"] = MathSymbol('|', "mid");
	math_symbols_["/"] = MathSymbol('/', "slash");
	math_symbols_["backslash"] = MathSymbol('\\', "backslash");
	math_symbols_["lceil"] = MathSymbol(0x2308, "lceil");
	math_symbols_["rceil"] = MathSymbol(0x2309, "rceil");
	math_symbols_["lfloor"] = MathSymbol(0x230A, "lfloor");
	math_symbols_["rfloor"] = MathSymbol(0x230B, "rfloor");
	math_symbols_["langle"] = MathSymbol(0x2329, "langle");
	math_symbols_["rangle"] = MathSymbol(0x232A, "rangle");
	math_symbols_["llbracket"] = MathSymbol(0x27e6, "llbracket");
	math_symbols_["rrbracket"] = MathSymbol(0x27e7, "rrbracket");
	math_symbols_["uparrow"] = MathSymbol(0x2191, "uparrow");
	math_symbols_["Uparrow"] = MathSymbol(0x21D1, "uparrow2");
	math_symbols_["updownarrow"] = MathSymbol(0x2195, "updownarrow");
	math_symbols_["Updownarrow"] = MathSymbol(0x21D5, "updownarrow2");
	math_symbols_["downarrow"] = MathSymbol(0x2193, "downarrow");
	math_symbols_["Downarrow"] = MathSymbol(0x21D3, "downarrow2");
	math_symbols_["downdownarrows"] = MathSymbol(0x21CA, "downdownarrows");
	math_symbols_["downharpoonleft"] = MathSymbol(0x21C3, "downharpoonleft");
	math_symbols_["downharpoonright"] = MathSymbol(0x21C2, "downharpoonright");
	math_symbols_["vert"] = MathSymbol(0x007C, "vert");
	math_symbols_["Vert"] = MathSymbol(0x2016, "vert2");

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

	left_list_items_.clear();
	right_list_items_.clear();

	initMathSymbols();

	QSize icon_size(32, 32);

	leftLW->setMinimumWidth(5 * icon_size.width());
	rightLW->setMinimumWidth(5 * icon_size.width());
	leftLW->setIconSize(icon_size);
	rightLW->setIconSize(icon_size);

	// The last element is the empty one.
	int const end = nr_latex_delimiters - 1;
	for (int i = 0; i < end; ++i) {
		string const delim = latex_delimiters[i];
		MathSymbol const & ms = mathSymbol(delim);
		// get pixmap with bullets
		QPixmap pixmap = getPixmap("images/math/", toqstr(ms.icon), "svgz,png");
		QIcon icon(pixmap);
		icon.addPixmap(getSelectedPixmap(pixmap), QIcon::Selected);
		QListWidgetItem * lwi = new QListWidgetItem(icon, QString());
		setDelimiterName(lwi, delim);
		left_list_items_[ms.unicode] = lwi;
		lwi->setToolTip(toqstr(delim));
		lwi->setSizeHint(icon_size);
		leftLW->addItem(lwi);
	}

	for (int i = 0; i != leftLW->count(); ++i) {
		MathSymbol const & ms = mathSymbol(getDelimiterName(leftLW->item(i)));
		QListWidgetItem * rwi = left_list_items_[doMatch(ms.unicode)]->clone();
		right_list_items_[ms.unicode] = rwi;
		rightLW->addItem(rwi);
	}

	// The last element is the empty one.
	QListWidgetItem * lwi = new QListWidgetItem(qt_("(None)"));
	lwi->setToolTip(qt_("No Delimiter"));
	left_list_items_['?'] = lwi;
	QListWidgetItem * rwi = new QListWidgetItem(qt_("(None)"));
	rwi->setToolTip(qt_("No Delimiter"));
	right_list_items_['?'] = rwi;
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
	if (str == "(")
		match = ")";
	else if (str == ")")
		match = "(";
	else if (str == "[")
		match = "]";
	else if (str == "]")
		match = "[";
	else if (str == "{")
		match = "}";
	else if (str == "}")
		match = "{";
	else if (str == "l")
		match = "r";
	else if (str == "rceil")
		match = "lceil";
	else if (str == "lceil")
		match = "rceil";
	else if (str == "rfloor")
		match = "lfloor";
	else if (str == "lfloor")
		match = "rfloor";
	else if (str == "rangle")
		match = "langle";
	else if (str == "langle")
		match = "rangle";
	else if (str == "llbracket")
		match = "rrbracket";
	else if (str == "rrbracket")
		match = "llbracket";
	else if (str == "backslash")
		match = "/";
	else if (str == "/")
		match = "backslash";
	else
		return symbol;

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

	// Locate matching QListWidgetItem.
	QListWidgetItem * lwi = left_list_items_[nlms.unicode];
	QListWidgetItem * rwi = right_list_items_[nrms.unicode];

	// Select.
	leftLW->setCurrentItem(lwi);
	rightLW->setCurrentItem(rwi);

	updateTeXCode(sizeCO->currentIndex());
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiDelimiter.cpp"
