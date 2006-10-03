/**
 * \file QViewSource.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QViewSource.h"
#include "QViewSourceDialog.h"
#include "qt_helpers.h"

#include "frontends/Application.h"

#include "controllers/ControlViewSource.h"

#include <sstream>

#include <QTextEdit>
#include <QPushButton>

namespace lyx {
namespace frontend {

typedef QController<ControlViewSource, QView<QViewSourceDialog> > base_class;


QViewSource::QViewSource(Dialog & parent)
	: base_class(parent, "")
{}


latexHighlighter::latexHighlighter(QTextDocument * parent) :
	QSyntaxHighlighter(parent)
{
	keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setFontWeight(QFont::Bold);
	commentFormat.setForeground(Qt::gray);
	mathFormat.setForeground(Qt::red);
}


void latexHighlighter::highlightBlock(QString const & text)
{
	// comment
	QRegExp exprComment("^%.*$");
	int index = text.indexOf(exprComment);
	while (index >= 0) {
		int length = exprComment.matchedLength();
		setFormat(index, length, commentFormat);
		index = text.indexOf(exprComment, index + length);
	}
	// $ $ 
	QRegExp exprMath("\\$[^\\$]*\\$");
	index = text.indexOf(exprMath);
	while (index >= 0) {
		int length = exprMath.matchedLength();
		setFormat(index, length, mathFormat);
		index = text.indexOf(exprMath, index + length);
	}
	// [ ]
	QRegExp exprDispMath("\\[[^\\]]*\\]");
	index = text.indexOf(exprDispMath);
	while (index >= 0) {
		int length = exprDispMath.matchedLength();
		setFormat(index, length, mathFormat);
		index = text.indexOf(exprDispMath, index + length);
	}
	// \whatever
	QRegExp exprKeyword("\\\\[A-Za-z]+");
	index = text.indexOf(exprKeyword);
	while (index >= 0) {
		int length = exprKeyword.matchedLength();
		setFormat(index, length, keywordFormat);
		index = text.indexOf(exprKeyword, index + length);
	}
}


void QViewSource::build_dialog()
{
	dialog_.reset(new QViewSourceDialog(this));
	// set syntex highlighting
	highlighter = new latexHighlighter(dialog_->viewSourceTV->document());
	//
	dialog_->viewSourceTV->setReadOnly(true);
	///dialog_->viewSourceTV->setAcceptRichText(false);
	// this is personal. I think source code should be in fixed-size font
	QFont font(toqstr(theApp->typewriterFontName()));
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	dialog_->viewSourceTV->setFont(font);
	// again, personal taste
	dialog_->viewSourceTV->setWordWrapMode(QTextOption::NoWrap);
}

 
void QViewSource::update_source()
{
	bool fullSource = dialog_->viewFullSourceCB->isChecked();
	dialog_->viewSourceTV->setPlainText(toqstr(controller().updateContent(fullSource)));
}


void QViewSource::update_contents()
{
	setTitle(controller().title());
	if (dialog_->autoUpdateCB->isChecked())
		update_source();
}


} // namespace frontend
} // namespace lyx

#include "QViewSource_moc.cpp"
