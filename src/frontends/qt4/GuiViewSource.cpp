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

#include "GuiApplication.h"
#include "GuiViewSource.h"
#include "LaTeXHighlighter.h"
#include "qt_helpers.h"

#include "BufferView.h"
#include "Buffer.h"
#include "Cursor.h"
#include "Paragraph.h"
#include "TexRow.h"

#include "support/debug.h"
#include "support/lassert.h"
#include "support/docstream.h"
#include "support/gettext.h"

#include <boost/crc.hpp>

#include <QSettings>
#include <QTextCursor>
#include <QTextDocument>
#include <QVariant>

using namespace std;

namespace lyx {
namespace frontend {

ViewSourceWidget::ViewSourceWidget()
	:	bv_(0), document_(new QTextDocument(this)),
		highlighter_(new LaTeXHighlighter(document_))
{
	setupUi(this);

	connect(viewFullSourceCB, SIGNAL(clicked()),
		this, SLOT(updateView()));
	connect(autoUpdateCB, SIGNAL(toggled(bool)),
		updatePB, SLOT(setDisabled(bool)));
	connect(autoUpdateCB, SIGNAL(toggled(bool)),
		this, SLOT(updateView()));
	connect(updatePB, SIGNAL(clicked()),
		this, SLOT(updateView()));

	// setting a document at this point trigger an assertion in Qt
	// so we disable the signals here:
	document_->blockSignals(true);
	viewSourceTV->setDocument(document_);
	document_->blockSignals(false);
	viewSourceTV->setReadOnly(true);
	///dialog_->viewSourceTV->setAcceptRichText(false);
	// this is personal. I think source code should be in fixed-size font
	QFont font(guiApp->typewriterFontName());
	font.setKerning(false);
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	viewSourceTV->setFont(font);
	// again, personal taste
	viewSourceTV->setWordWrapMode(QTextOption::NoWrap);
}


static size_t crcCheck(docstring const & s)
{
	boost::crc_32_type crc;
	crc.process_bytes(&s[0], sizeof(char_type) * s.size());
	return crc.checksum();
}


/** get the source code of selected paragraphs, or the whole document
	\param fullSource get full source code
	\return true if the content has changed since last call.
 */
static bool getContent(BufferView const * view, bool fullSource, QString & qstr)
{
	// get the *top* level paragraphs that contain the cursor,
	// or the selected text
	pit_type par_begin;
	pit_type par_end;

	if (!view->cursor().selection()) {
		par_begin = view->cursor().bottom().pit();
		par_end = par_begin;
	} else {
		par_begin = view->cursor().selectionBegin().bottom().pit();
		par_end = view->cursor().selectionEnd().bottom().pit();
	}
	if (par_begin > par_end)
		swap(par_begin, par_end);
	odocstringstream ostr;
	view->buffer().getSourceCode(ostr, par_begin, par_end + 1, fullSource);
	docstring s = ostr.str();
	static size_t crc = 0;
	size_t newcrc = crcCheck(s);
	if (newcrc == crc)
		return false;
	crc = newcrc;
	qstr = toqstr(s);
	return true;
}


void ViewSourceWidget::setBufferView(BufferView const * bv)
{
	bv_ = bv;
	setEnabled(bv ?  true : false);
}


void ViewSourceWidget::updateView()
{
	if (!bv_) {
		document_->setPlainText(QString());
		setEnabled(false);
		return;
	}
	
	setEnabled(true);

	QString content;
	if (getContent(bv_, viewFullSourceCB->isChecked(), content))
		document_->setPlainText(content);

	CursorSlice beg = bv_->cursor().selectionBegin().bottom();
	CursorSlice end = bv_->cursor().selectionEnd().bottom();
	int const begrow = bv_->buffer().texrow().
		getRowFromIdPos(beg.paragraph().id(), beg.pos());
	int endrow = bv_->buffer().texrow().
		getRowFromIdPos(end.paragraph().id(), end.pos());
	int const nextendrow = bv_->buffer().texrow().
		getRowFromIdPos(end.paragraph().id(), end.pos() + 1);
	if (endrow != nextendrow)
		endrow = nextendrow - 1;

	QTextCursor c = QTextCursor(viewSourceTV->document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, begrow);
	c.select(QTextCursor::BlockUnderCursor);
	c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor,
		endrow - begrow + 1);
	viewSourceTV->setTextCursor(c);
}


GuiViewSource::GuiViewSource(GuiView & parent,
		Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "view-source", qt_("LaTeX Source"), area, flags)
{
	widget_ = new ViewSourceWidget();
	setWidget(widget_);
}


GuiViewSource::~GuiViewSource()
{
	delete widget_;
}


void GuiViewSource::updateView()
{
	if (widget_->autoUpdateCB->isChecked()) {
		widget_->setBufferView(bufferview());
		widget_->updateView();
	}
}


void GuiViewSource::enableView(bool enable)
{
	widget_->setBufferView(bufferview());
	if (!enable)
		// In the opposite case, updateView() will be called anyway.
		widget_->updateView();
}


bool GuiViewSource::initialiseParams(string const & /*source*/)
{
	setWindowTitle(title());
	return true;
}


QString GuiViewSource::title() const
{
	switch (docType()) {
		case LATEX:
			return qt_("LaTeX Source");
		case DOCBOOK:
			return qt_("DocBook Source");
		case LITERATE:
			return qt_("Literate Source");
	}
	LASSERT(false, /**/);
	return QString();
}


void GuiViewSource::saveSession() const
{
	Dialog::saveSession();
	QSettings settings;
	settings.setValue(
		sessionKey() + "/fullsource", widget_->viewFullSourceCB->isChecked());
	settings.setValue(
		sessionKey() + "/autoupdate", widget_->autoUpdateCB->isChecked());
}


void GuiViewSource::restoreSession()
{
	DockView::restoreSession();
	// FIXME: Full source updating is too slow to be done at startup.
	//widget_->viewFullSourceCB->setChecked(
	//	settings.value(sessionKey() + "/fullsource", false).toBool());
	widget_->viewFullSourceCB->setChecked(false);
	QSettings settings;
	widget_->autoUpdateCB->setChecked(
		settings.value(sessionKey() + "/autoupdate", true).toBool());
	widget_->updateView();
}


Dialog * createGuiViewSource(GuiView & lv)
{
	return new GuiViewSource(lv);
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiViewSource.cpp"
