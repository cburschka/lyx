/**
 * \file GuiSymbols.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSymbols.h"

#include "GuiApplication.h"
#include "GuiView.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferView.h"
#include "Encoding.h"

#include "support/gettext.h"

#include <QPixmap>
#include <QListWidgetItem>

using namespace std;

namespace lyx {
namespace frontend {


namespace {

/// name of unicode block, start and end code point
struct UnicodeBlocks {
	char const * name;
	char_type start;
	char_type end;
};


/// all unicode blocks with start and end code point
UnicodeBlocks unicode_blocks[] = {
	{ N_("Basic Latin"), 0x0000, 0x007f },
	{ N_("Latin-1 Supplement"), 0x0080, 0x00ff },
	{ N_("Latin Extended-A"), 0x0100, 0x017f },
	{ N_("Latin Extended-B"), 0x0180, 0x024f },
	{ N_("IPA Extensions"), 0x0250, 0x02af },
	{ N_("Spacing Modifier Letters"), 0x02b0, 0x02ff },
	{ N_("Combining Diacritical Marks"), 0x0300, 0x036f },
	{ N_("Greek"), 0x0370, 0x03ff },
	{ N_("Cyrillic"), 0x0400, 0x04ff },
	{ N_("Armenian"), 0x0530, 0x058f },
	{ N_("Hebrew"), 0x0590, 0x05ff },
	{ N_("Arabic"), 0x0600, 0x06ff },
	{ N_("Devanagari"), 0x0900, 0x097f },
	{ N_("Bengali"), 0x0980, 0x09ff },
	{ N_("Gurmukhi"), 0x0a00, 0x0a7f },
	{ N_("Gujarati"), 0x0a80, 0x0aff },
	{ N_("Oriya"), 0x0b00, 0x0b7f },
	{ N_("Tamil"), 0x0b80, 0x0bff },
	{ N_("Telugu"), 0x0c00, 0x0c7f },
	{ N_("Kannada"), 0x0c80, 0x0cff },
	{ N_("Malayalam"), 0x0d00, 0x0d7f },
	{ N_("Thai"), 0x0e00, 0x0e7f },
	{ N_("Lao"), 0x0e80, 0x0eff },
	{ N_("Tibetan"), 0x0f00, 0x0fbf },
	{ N_("Georgian"), 0x10a0, 0x10ff },
	{ N_("Hangul Jamo"), 0x1100, 0x11ff },
	{ N_("Latin Extended Additional"), 0x1e00, 0x1eff },
	{ N_("Greek Extended"), 0x1f00, 0x1fff },
	{ N_("General Punctuation"), 0x2000, 0x206f },
	{ N_("Superscripts and Subscripts"), 0x2070, 0x209f },
	{ N_("Currency Symbols"), 0x20a0, 0x20cf },
	{ N_("Combining Diacritical Marks for Symbols"), 0x20d0, 0x20ff },
	{ N_("Letterlike Symbols"), 0x2100, 0x214f },
	{ N_("Number Forms"), 0x2150, 0x218f },
	{ N_("Arrows"), 0x2190, 0x21ff },
	{ N_("Mathematical Operators"), 0x2200, 0x22ff },
	{ N_("Miscellaneous Technical"), 0x2300, 0x23ff },
	{ N_("Control Pictures"), 0x2400, 0x243f },
	{ N_("Optical Character Recognition"), 0x2440, 0x245f },
	{ N_("Enclosed Alphanumerics"), 0x2460, 0x24ff },
	{ N_("Box Drawing"), 0x2500, 0x257f },
	{ N_("Block Elements"), 0x2580, 0x259f },
	{ N_("Geometric Shapes"), 0x25a0, 0x25ff },
	{ N_("Miscellaneous Symbols"), 0x2600, 0x26ff },
	{ N_("Dingbats"), 0x2700, 0x27bf },
	{ N_("Miscellaneous Mathematical Symbols-A"), 0x27c0, 0x27ef },
	{ N_("CJK Symbols and Punctuation"), 0x3000, 0x303f },
	{ N_("Hiragana"), 0x3040, 0x309f },
	{ N_("Katakana"), 0x30a0, 0x30ff },
	{ N_("Bopomofo"), 0x3100, 0x312f },
	{ N_("Hangul Compatibility Jamo"), 0x3130, 0x318f },
	{ N_("Kanbun"), 0x3190, 0x319f },
	{ N_("Enclosed CJK Letters and Months"), 0x3200, 0x32ff },
	{ N_("CJK Compatibility"), 0x3300, 0x33ff },
	{ N_("CJK Unified Ideographs"), 0x4e00, 0x9fa5 },
	{ N_("Hangul Syllables"), 0xac00, 0xd7a3 },
	{ N_("High Surrogates"), 0xd800, 0xdb7f },
	{ N_("Private Use High Surrogates"), 0xdb80, 0xdbff },
	{ N_("Low Surrogates"), 0xdc00, 0xdfff },
	{ N_("Private Use Area"), 0xe000, 0xf8ff },
	{ N_("CJK Compatibility Ideographs"), 0xf900, 0xfaff },
	{ N_("Alphabetic Presentation Forms"), 0xfb00, 0xfb4f },
	{ N_("Arabic Presentation Forms-A"), 0xfb50, 0xfdff },
	{ N_("Combining Half Marks"), 0xfe20, 0xfe2f },
	{ N_("CJK Compatibility Forms"), 0xfe30, 0xfe4f },
	{ N_("Small Form Variants"), 0xfe50, 0xfe6f },
	{ N_("Arabic Presentation Forms-B"), 0xfe70, 0xfeff },
	{ N_("Halfwidth and Fullwidth Forms"), 0xff00, 0xffef },
	{ N_("Specials"), 0xfff0, 0xffff },
	{ N_("Linear B Syllabary"), 0x10000, 0x1007f },
	{ N_("Linear B Ideograms"), 0x10080, 0x100ff },
	{ N_("Aegean Numbers"), 0x10100, 0x1013f },
	{ N_("Ancient Greek Numbers"), 0x10140, 0x1018f },
	{ N_("Old Italic"), 0x10300, 0x1032f },
	{ N_("Gothic"), 0x10330, 0x1034f },
	{ N_("Ugaritic"), 0x10380, 0x1039f },
	{ N_("Old Persian"), 0x103a0, 0x103df },
	{ N_("Deseret"), 0x10400, 0x1044f },
	{ N_("Shavian"), 0x10450, 0x1047f },
	{ N_("Osmanya"), 0x10480, 0x104af },
	{ N_("Cypriot Syllabary"), 0x10800, 0x1083f },
	{ N_("Kharoshthi"), 0x10a00, 0x10a5f },
	{ N_("Byzantine Musical Symbols"), 0x1d000, 0x1d0ff },
	{ N_("Musical Symbols"), 0x1d100, 0x1d1ff },
	{ N_("Ancient Greek Musical Notation"), 0x1d200, 0x1d24f },
	{ N_("Tai Xuan Jing Symbols"), 0x1d300, 0x1d35f },
	{ N_("Mathematical Alphanumeric Symbols"), 0x1d400, 0x1d7ff },
	{ N_("CJK Unified Ideographs Extension B"), 0x20000, 0x2a6d6 },
	{ N_("CJK Compatibility Ideographs Supplement"), 0x2f800, 0x2fa1f },
	{ N_("Tags"), 0xe0000, 0xe007f },
	{ N_("Variation Selectors Supplement"), 0xe0100, 0xe01ef },
	{ N_("Supplementary Private Use Area-A"), 0xf0000, 0xe01ef },
	{ N_("Supplementary Private Use Area-B"), 0x100000, 0x10ffff }
};

const int no_blocks = sizeof(unicode_blocks) / sizeof(UnicodeBlocks);

} // namespace anon


GuiSymbols::GuiSymbols(GuiView & lv)
	: DialogView(lv, "symbols", qt_("Symbols")), encoding_("ascii")
{
	setupUi(this);

	setFocusProxy(symbolsLW);

	symbolsLW->setViewMode(QListView::IconMode);
	symbolsLW->setUniformItemSizes(true);
	// increase the display size of the symbols a bit
	QFont font= symbolsLW->font();
	int size = font.pointSize() + 3;
	font.setPointSize(size);
	symbolsLW->setFont(font);
}


void GuiSymbols::updateView()
{
	chosenLE->clear();

	string const & new_encoding = bufferview()->cursor().getEncoding()->name();
	if (new_encoding == encoding_)
		// everything up to date
		return;
	if (!new_encoding.empty())
		encoding_ = new_encoding;
	updateSymbolList();
}


void GuiSymbols::enableView(bool enable)
{
	chosenLE->setEnabled(enable);
	okPB->setEnabled(enable);
	applyPB->setEnabled(enable);
}


void GuiSymbols::on_applyPB_clicked()
{
	dispatchParams();
}


void GuiSymbols::on_okPB_clicked()
{
	dispatchParams();
	hide();
}


void GuiSymbols::on_closePB_clicked()
{
	hide();
}


void GuiSymbols::on_symbolsLW_itemActivated(QListWidgetItem *)
{
	on_okPB_clicked();
}


void GuiSymbols::on_chosenLE_textChanged(QString const & text)
{
	bool const empty_sel = text.isEmpty();
	okPB->setEnabled(!empty_sel);
	applyPB->setEnabled(!empty_sel);
}


void GuiSymbols::on_chosenLE_returnPressed()
{
	on_okPB_clicked();
}


void GuiSymbols::on_symbolsLW_itemClicked(QListWidgetItem * item)
{
	QString const text = item->text();
	if (text.isEmpty())
		return;
	if (chosenLE->isEnabled())
		chosenLE->insert(text);
	QString const category = getBlock(text.data()->unicode());
	categoryCO->setCurrentIndex(categoryCO->findText(category));
}


void GuiSymbols::on_categoryCO_activated(QString const & text)
{
	if (used_blocks.find(text) != used_blocks.end())
		symbolsLW->scrollToItem(used_blocks[text]);
}


void GuiSymbols::updateSymbolList()
{
	symbolsLW->clear();
	used_blocks.clear();
	categoryCO->clear();

	typedef set<char_type> SymbolsList;
	Encoding enc = *(encodings.getFromLyXName(encoding_));
	SymbolsList symbols = enc.getSymbolsList();

	SymbolsList::const_iterator const end = symbols.end();
	for (SymbolsList::const_iterator it = symbols.begin(); it != end; ++it) {
		char_type c = *it;
		QChar::Category cat = QChar::category((uint) c);
		// we do not want control or space characters
		if (cat == QChar::Other_Control || cat == QChar::Separator_Space)
			continue;
		QListWidgetItem * lwi = new QListWidgetItem(
			QString::fromUcs4((uint const *) &c, 1));
		lwi->setTextAlignment(Qt::AlignCenter);
		symbolsLW->addItem(lwi);
		QString block = getBlock(c);
		if (used_blocks.find(block) == used_blocks.end())
			used_blocks[block] = lwi;
	}

	// update category combo
	for (UsedBlocks::iterator it = used_blocks.begin(); it != used_blocks.end(); ++it) {
		categoryCO->addItem(it->first);
	}
}


QString const GuiSymbols::getBlock(char_type c) const
{
	int i = 0;
	while (c > unicode_blocks[i].end && i < no_blocks)
		++i;
	if (unicode_blocks[i].name)
		return toqstr(unicode_blocks[i].name);
	return QString();
}


void GuiSymbols::dispatchParams()
{
	dispatch(FuncRequest(LFUN_SELF_INSERT, fromqstr(chosenLE->text())));
}


Dialog * createGuiSymbols(GuiView & lv)
{
	return new GuiSymbols(lv);
}


} // namespace frontend
} // namespace lyx

#include "GuiSymbols_moc.cpp"
