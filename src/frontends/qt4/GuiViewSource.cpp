/**
 * \file GuiViewSource.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiViewSource.h"
#include "qt_helpers.h"

#include "Application.h"
#include "BufferView.h"
#include "Buffer.h"
#include "Cursor.h"
#include "gettext.h"
#include "Paragraph.h"
#include "TexRow.h"

#include <QTextCursor>
#include <QTextDocument>
#include <boost/tuple/tuple.hpp>

using std::string;

namespace lyx {
namespace frontend {

GuiViewSourceDialog::GuiViewSourceDialog(ControlViewSource & controller)
	:	controller_(controller), document_(new QTextDocument(this)),
		highlighter_(new LaTeXHighlighter(document_))
{
	setupUi(this);
	setWindowTitle(qt_("LaTeX Source"));

	connect(viewFullSourceCB, SIGNAL(clicked()),
		this, SLOT(updateView()));
	connect(autoUpdateCB, SIGNAL(toggled(bool)),
		updatePB, SLOT(setDisabled(bool)));
	connect(updatePB, SIGNAL(clicked()),
		this, SLOT(updateView()));

	// setting a document at this point trigger an assertion in Qt
	// so we disable the signals here:
	document()->blockSignals(true);
	viewSourceTV->setDocument(document());
	document()->blockSignals(false);
	viewSourceTV->setReadOnly(true);
	///dialog_->viewSourceTV->setAcceptRichText(false);
	// this is personal. I think source code should be in fixed-size font
	QFont font(toqstr(theApp()->typewriterFontName()));
	font.setKerning(false);
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	viewSourceTV->setFont(font);
	// again, personal taste
	viewSourceTV->setWordWrapMode(QTextOption::NoWrap);
}


void GuiViewSourceDialog::updateView()
{
	if (autoUpdateCB->isChecked())
		update(viewFullSourceCB->isChecked());

	int beg, end;
	boost::tie(beg, end) = controller_.getRows();
	QTextCursor c = QTextCursor(viewSourceTV->document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, beg);
	c.select(QTextCursor::BlockUnderCursor);
	c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, end - beg + 1);
	viewSourceTV->setTextCursor(c);
	QWidget::update();
}


void GuiViewSourceDialog::update(bool full_source)
{
	document_->setPlainText(toqstr(controller_.updateContent(full_source)));
}



/////////////////////////////////////////////////////////////////////
//
// LaTeXHighlighter
//
/////////////////////////////////////////////////////////////////////


LaTeXHighlighter::LaTeXHighlighter(QTextDocument * parent)
	: QSyntaxHighlighter(parent)
{
	keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setFontWeight(QFont::Bold);
	commentFormat.setForeground(Qt::darkGray);
	mathFormat.setForeground(Qt::red);
}


void LaTeXHighlighter::highlightBlock(QString const & text)
{
	// $ $
	QRegExp exprMath("\\$[^\\$]*\\$");
	int index = text.indexOf(exprMath);
	while (index >= 0) {
		int length = exprMath.matchedLength();
		setFormat(index, length, mathFormat);
		index = text.indexOf(exprMath, index + length);
	}
	// [ ]
	QRegExp exprStartDispMath("(\\\\\\[|"
		"\\\\begin\\{equation\\**\\}|"
		"\\\\begin\\{eqnarray\\**\\}|"
		"\\\\begin\\{align(ed|at)*\\**\\}|"
		"\\\\begin\\{flalign\\**\\}|"
		"\\\\begin\\{gather\\**\\}|"
		"\\\\begin\\{multline\\**\\}|"
		"\\\\begin\\{array\\**\\}|"
		"\\\\begin\\{cases\\**\\}"
		")");
	QRegExp exprEndDispMath("(\\\\\\]|"
		"\\\\end\\{equation\\**\\}|"
		"\\\\end\\{eqnarray\\**\\}|"
		"\\\\end\\{align(ed|at)*\\**\\}|"
		"\\\\end\\{flalign\\**\\}|"
		"\\\\end\\{gather\\**\\}|"
		"\\\\end\\{multline\\**\\}|"
		"\\\\end\\{array\\**\\}|"
		"\\\\end\\{cases\\**\\}"
		")");
	int startIndex = 0;
	// if previous block was in 'disp math'
	// start search from 0 (for end disp math)
	// otherwise, start search from 'begin disp math'
	if (previousBlockState() != 1)
		startIndex = text.indexOf(exprStartDispMath);
	while (startIndex >= 0) {
		int endIndex = text.indexOf(exprEndDispMath, startIndex);
		int length;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			length = text.length() - startIndex;
		} else {
			length = endIndex - startIndex + exprEndDispMath.matchedLength();
		}
		setFormat(startIndex, length, mathFormat);
		startIndex = text.indexOf(exprStartDispMath, startIndex + length);
	}
	// \whatever
	QRegExp exprKeyword("\\\\[A-Za-z]+");
	index = text.indexOf(exprKeyword);
	while (index >= 0) {
		int length = exprKeyword.matchedLength();
		setFormat(index, length, keywordFormat);
		index = text.indexOf(exprKeyword, index + length);
	}
	// comment
	QRegExp exprComment("(^|[^\\\\])%.*$");
	index = text.indexOf(exprComment);
	while (index >= 0) {
		int const length = exprComment.matchedLength();
		setFormat(index, length, commentFormat);
		index = text.indexOf(exprComment, index + length);
	}
}


ControlViewSource::ControlViewSource(Dialog & parent)
	: Controller(parent)
{}


bool ControlViewSource::initialiseParams(string const & /*source*/)
{
	return true;
}


docstring const ControlViewSource::updateContent(bool fullSource)
{
	// get the *top* level paragraphs that contain the cursor,
	// or the selected text
	pit_type par_begin;
	pit_type par_end;

	BufferView * view = bufferview();
	if (!view->cursor().selection()) {
		par_begin = view->cursor().bottom().pit();
		par_end = par_begin;
	} else {
		par_begin = view->cursor().selectionBegin().bottom().pit();
		par_end = view->cursor().selectionEnd().bottom().pit();
	}
	if (par_begin > par_end)
		std::swap(par_begin, par_end);
	odocstringstream ostr;
	view->buffer().getSourceCode(ostr, par_begin, par_end + 1, fullSource);
	return ostr.str();
}


std::pair<int, int> ControlViewSource::getRows() const
{
	BufferView const * view = bufferview();
	CursorSlice beg = view->cursor().selectionBegin().bottom();
	CursorSlice end = view->cursor().selectionEnd().bottom();

	int begrow = view->buffer().texrow().
		getRowFromIdPos(beg.paragraph().id(), beg.pos());
	int endrow = view->buffer().texrow().
		getRowFromIdPos(end.paragraph().id(), end.pos());
	int nextendrow = view->buffer().texrow().
		getRowFromIdPos(end.paragraph().id(), end.pos() + 1);
	return std::make_pair(begrow, endrow == nextendrow ? endrow : (nextendrow - 1));
}


docstring const ControlViewSource::title() const
{
	switch (docType()) {
		case LATEX:
			return _("LaTeX Source");
		case DOCBOOK:
			return _("DocBook Source");
		case LITERATE:
			return _("Literate Source");
		default:
			BOOST_ASSERT(false);
			return docstring();
	}
}


Dialog * createGuiViewSource(LyXView & lv)
{
	return new GuiViewSource(static_cast<GuiViewBase &>(lv));
}


} // namespace frontend
} // namespace lyx

#include "GuiViewSource_moc.cpp"
