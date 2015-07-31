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

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "Format.h"
#include "Paragraph.h"
#include "TexRow.h"

#include "support/debug.h"
#include "support/lassert.h"
#include "support/docstream.h"
#include "support/gettext.h"

#include <boost/crc.hpp>

#include <QBoxLayout>
#include <QSettings>
#include <QTextCursor>
#include <QTextDocument>
#include <QVariant>

using namespace std;

namespace lyx {
namespace frontend {

ViewSourceWidget::ViewSourceWidget()
	:	bv_(0), document_(new QTextDocument(this)),
		highlighter_(new LaTeXHighlighter(document_)),
		force_getcontent_(true),
		update_timer_(new QTimer(this))
{
	setupUi(this);

	connect(contentsCO, SIGNAL(activated(int)),
		this, SLOT(contentsChanged()));
	connect(autoUpdateCB, SIGNAL(toggled(bool)),
		updatePB, SLOT(setDisabled(bool)));
	connect(autoUpdateCB, SIGNAL(toggled(bool)),
		this, SLOT(updateViewNow()));
	connect(masterPerspectiveCB, SIGNAL(toggled(bool)),
		this, SLOT(updateViewNow()));
	connect(updatePB, SIGNAL(clicked()),
		this, SLOT(updateViewNow()));
	connect(outputFormatCO, SIGNAL(activated(int)),
		this, SLOT(setViewFormat()));

	// setting the update timer
	update_timer_->setSingleShot(true);
	connect(update_timer_, SIGNAL(timeout()),
		this, SLOT(realUpdateView()));

	// setting a document at this point trigger an assertion in Qt
	// so we disable the signals here:
	document_->blockSignals(true);
	viewSourceTV->setDocument(document_);
	document_->blockSignals(false);
	viewSourceTV->setReadOnly(true);
	///dialog_->viewSourceTV->setAcceptRichText(false);
	// this is personal. I think source code should be in fixed-size font
	QFont font(guiApp->typewriterFontName());
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
static bool getContent(BufferView const * view, Buffer::OutputWhat output,
		       QString & qstr, string const & format, bool force_getcontent,
		       bool master)
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
	view->buffer().getSourceCode(ostr, format, par_begin, par_end + 1,
				     output, master);
	docstring s = ostr.str();
	// FIXME THREAD
	// Could this be private to this particular dialog? We could have
	// more than one of these, in different windows.
	static size_t crc = 0;
	size_t newcrc = crcCheck(s);
	if (newcrc == crc && !force_getcontent)
		return false;
	crc = newcrc;
	qstr = toqstr(s);
	return true;
}


void ViewSourceWidget::setBufferView(BufferView const * bv)
{
	if (bv_ != bv)
		force_getcontent_ = true;
	bv_ = bv;
	setEnabled(bv ?  true : false);
}


void ViewSourceWidget::contentsChanged()
{
	if (autoUpdateCB->isChecked())
		updateViewNow();
}


void ViewSourceWidget::setViewFormat()
{
	view_format_ = outputFormatCO->itemData(
	      outputFormatCO->currentIndex()).toString();
	updateViewNow();
}


void ViewSourceWidget::updateView()
{
	const int long_delay = 400;
	const int short_delay = 60;
	// a shorter delay if just the current paragraph is shown
	update_timer_->start((contentsCO->currentIndex() == 0) ?
						short_delay : long_delay);
}

void ViewSourceWidget::updateViewNow()
{
	update_timer_->start(0);
}

void ViewSourceWidget::realUpdateView()
{
	if (!bv_) {
		document_->setPlainText(QString());
		setEnabled(false);
		return;
	}

	setEnabled(true);

	string const format = fromqstr(view_format_);

	QString content;
	Buffer::OutputWhat output = Buffer::CurrentParagraph;
	if (contentsCO->currentIndex() == 1)
		output = Buffer::FullSource;
	else if (contentsCO->currentIndex() == 2)
		output = Buffer::OnlyPreamble;
	else if (contentsCO->currentIndex() == 3)
		output = Buffer::OnlyBody;

	if (getContent(bv_, output, content, format,
		      force_getcontent_, masterPerspectiveCB->isChecked()))
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


void ViewSourceWidget::updateDefaultFormat()
{
	if (!bv_)
		return;

	outputFormatCO->blockSignals(true);
	outputFormatCO->clear();
	outputFormatCO->addItem(qt_("Default"),
				QVariant(QString("default")));

	int index = 0;
	vector<string> tmp = bv_->buffer().params().backends();
	vector<string>::const_iterator it = tmp.begin();
	vector<string>::const_iterator en = tmp.end();
	for (; it != en; ++it) {
		string const format = *it;
		Format const * fmt = formats.getFormat(format);
		if (!fmt) {
			LYXERR0("Can't find format for backend " << format << "!");
			continue;
		} 

		QString const pretty = qt_(fmt->prettyname());
		QString const qformat = toqstr(format);
		outputFormatCO->addItem(pretty, QVariant(qformat));
		if (qformat == view_format_)
		   index = outputFormatCO->count() -1;
	}
	outputFormatCO->setCurrentIndex(index);

	outputFormatCO->blockSignals(false);
}


void ViewSourceWidget::resizeEvent (QResizeEvent * event)
{
	QSize const & formSize = formLayout->sizeHint();
	// minimize the size of the part that contains the buttons
	if (width() * formSize.height() < height() * formSize.width()) {
		layout_->setDirection(QBoxLayout::TopToBottom);
	} else {
		layout_->setDirection(QBoxLayout::LeftToRight);
	}
	QWidget::resizeEvent(event);
}


GuiViewSource::GuiViewSource(GuiView & parent,
		Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "view-source", qt_("LaTeX Source"), area, flags)
{
	widget_ = new ViewSourceWidget;
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
	widget_->masterPerspectiveCB->setEnabled(buffer().parent());
}


void GuiViewSource::enableView(bool enable)
{
	widget_->setBufferView(bufferview());
	widget_->updateDefaultFormat();
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
	LATTEST(false);
	return QString();
}


void GuiViewSource::saveSession() const
{
	Dialog::saveSession();
	QSettings settings;
	// see below
	// settings.setValue(
	//	sessionKey() + "/output", widget_->contentsCO->currentIndex());
	settings.setValue(
		sessionKey() + "/autoupdate", widget_->autoUpdateCB->isChecked());
}


void GuiViewSource::restoreSession()
{
	DockView::restoreSession();
	// FIXME: Full source updating is too slow to be done at startup.
	//widget_->outputCO-setCurrentIndex(
	//	settings.value(sessionKey() + "/output", false).toInt());
	widget_->contentsCO->setCurrentIndex(0);
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
