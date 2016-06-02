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

#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "Format.h"
#include "Paragraph.h"

#include "support/debug.h"
#include "support/lassert.h"
#include "support/docstream.h"
#include "support/docstring_list.h"
#include "support/gettext.h"

#include <boost/crc.hpp>

#include <QBoxLayout>
#include <QComboBox>
#include <QScrollBar>
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
		update_timer_(new QTimer(this))
{
	setupUi(this);

	connect(contentsCO, SIGNAL(activated(int)),
		this, SLOT(contentsChanged()));
	connect(autoUpdateCB, SIGNAL(toggled(bool)),
		updatePB, SLOT(setDisabled(bool)));
	connect(autoUpdateCB, SIGNAL(toggled(bool)),
		this, SLOT(contentsChanged()));
	connect(masterPerspectiveCB, SIGNAL(toggled(bool)),
		this, SLOT(contentsChanged()));
	connect(updatePB, SIGNAL(clicked()),
		this, SLOT(updateViewNow()));
	connect(outputFormatCO, SIGNAL(activated(int)),
		this, SLOT(setViewFormat(int)));
	connect(outputFormatCO, SIGNAL(activated(int)),
		this, SLOT(contentsChanged()));
#ifdef DEVEL_VERSION
	if (lyx::lyxerr.debugging(Debug::LATEX))
		connect(viewSourceTV, SIGNAL(cursorPositionChanged()),
				this, SLOT(gotoCursor()));
#endif

	// setting the update timer
	update_timer_->setSingleShot(true);
	connect(update_timer_, SIGNAL(timeout()),
		this, SLOT(realUpdateView()));

	// setting a document at this point trigger an assertion in Qt
	// so we disable the signals here:
	document_->blockSignals(true);
	viewSourceTV->setDocument(document_);
	// reset selections
	setText();
	document_->blockSignals(false);
	viewSourceTV->setReadOnly(true);
	///dialog_->viewSourceTV->setAcceptRichText(false);
	// this is personal. I think source code should be in fixed-size font
	viewSourceTV->setFont(guiApp->typewriterSystemFont());
	// again, personal taste
	viewSourceTV->setWordWrapMode(QTextOption::NoWrap);
}


void ViewSourceWidget::getContent(BufferView const * view,
			Buffer::OutputWhat output, docstring & str, string const & format,
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
	texrow_ = view->buffer()
		.getSourceCode(ostr, format, par_begin, par_end + 1, output, master);
	//ensure that the last line can always be selected in its full width
	str = ostr.str() + "\n";
}


void ViewSourceWidget::setBufferView(BufferView const * bv)
{
	if (bv_ != bv) {
		setText();
		bv_ = bv;
	}
	setEnabled(bv ?  true : false);
}


bool ViewSourceWidget::setText(QString const & qstr)
{
	bool const changed = document_->toPlainText() != qstr;
	viewSourceTV->setExtraSelections(QList<QTextEdit::ExtraSelection>());
	if (changed)
		document_->setPlainText(qstr);
	return changed;
}


void ViewSourceWidget::contentsChanged()
{
	if (autoUpdateCB->isChecked())
		updateViewNow();
}


void ViewSourceWidget::setViewFormat(int const index)
{
	outputFormatCO->setCurrentIndex(index);
	view_format_ = outputFormatCO->itemData(index).toString();
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
		setText();
		setEnabled(false);
		return;
	}

	setEnabled(true);

	// we will try to get that much space around the cursor
	int const v_margin = 3;
	int const h_margin = 10;
	// we will try to preserve this
	int const h_scroll = viewSourceTV->horizontalScrollBar()->value();

	string const format = fromqstr(view_format_);

	Buffer::OutputWhat output = Buffer::CurrentParagraph;
	if (contentsCO->currentIndex() == 1)
		output = Buffer::FullSource;
	else if (contentsCO->currentIndex() == 2)
		output = Buffer::OnlyPreamble;
	else if (contentsCO->currentIndex() == 3)
		output = Buffer::OnlyBody;

	docstring content;
	getContent(bv_, output, content, format, masterPerspectiveCB->isChecked());
	QString old = document_->toPlainText();
	QString qcontent = toqstr(content);
#ifdef DEVEL_VERSION
	// output tex<->row correspondences in the source panel if the "-dbg latex"
	// option is given.
	if (texrow_ && lyx::lyxerr.debugging(Debug::LATEX)) {
		QStringList list = qcontent.split(QChar('\n'));
		docstring_list dlist;
		for (QStringList::const_iterator it = list.begin(); it != list.end(); ++it)
			dlist.push_back(from_utf8(fromqstr(*it)));
		texrow_->prepend(dlist);
		qcontent.clear();
		for (docstring_list::iterator it = dlist.begin(); it != dlist.end(); ++it)
			qcontent += toqstr(*it) + '\n';
	}
#endif
	// prevent gotoCursor()
	viewSourceTV->blockSignals(true);
	bool const changed = setText(qcontent);

	if (changed && !texrow_) {
		// position-to-row is unavailable
		// we jump to the first modification
		const QChar * oc = old.constData();
		const QChar * nc = qcontent.constData();
		int pos = 0;
		while (*oc != '\0' && *nc != '\0' && *oc == *nc) {
			++oc;
			++nc;
			++pos;
		}
		QTextCursor c = QTextCursor(viewSourceTV->document());
		//get some space below the cursor
		c.setPosition(pos);
		c.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor,v_margin);
		viewSourceTV->setTextCursor(c);
		//get some space on the right of the cursor
		viewSourceTV->horizontalScrollBar()->setValue(h_scroll);
		c.setPosition(pos);
		const int block = c.blockNumber();
		for (int i = h_margin; i && block == c.blockNumber(); --i) {
			c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
		}
		c.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
		viewSourceTV->setTextCursor(c);
		//back to the position
		c.setPosition(pos);
		//c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,1);
		viewSourceTV->setTextCursor(c);

	} else if (texrow_) {
		// Use the available position-to-row conversion to highlight
		// the current selection in the source
		std::pair<int,int> rows = texrow_->rowFromCursor(bv_->cursor());
		int const beg_row = rows.first;
		int const end_row = rows.second;

		QTextCursor c = QTextCursor(viewSourceTV->document());

		c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor,
					   beg_row - 1);
		const int beg_sel = c.position();
		//get some space above the cursor
		c.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor,
					   v_margin);
		viewSourceTV->setTextCursor(c);
		c.setPosition(beg_sel, QTextCursor::MoveAnchor);

		c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor,
					   end_row - beg_row +1);
		const int end_sel = c.position();
		//get some space below the cursor
		c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor,
					   v_margin - 1);
		viewSourceTV->setTextCursor(c);
		c.setPosition(end_sel, QTextCursor::KeepAnchor);

		viewSourceTV->setTextCursor(c);

		//the real highlighting is done with an ExtraSelection
		QTextCharFormat format;
		{
		// We create a new color with the lightness of AlternateBase and
		// the hue and saturation of Highlight
		QPalette palette = viewSourceTV->palette();
		QBrush alt = palette.alternateBase();
		QColor high = palette.highlight().color().toHsl();
		QColor col = QColor::fromHsl(high.hue(),
		                             high.hslSaturation(),
		                             alt.color().lightness());
		alt.setColor(col);
		format.setBackground(alt);
		}
		format.setProperty(QTextFormat::FullWidthSelection, true);
		QTextEdit::ExtraSelection sel;
		sel.format = format;
		sel.cursor = c;
		viewSourceTV->setExtraSelections(
			QList<QTextEdit::ExtraSelection>() << sel);

		//clean up
		c.clearSelection();
		viewSourceTV->setTextCursor(c);
		viewSourceTV->horizontalScrollBar()->setValue(h_scroll);
	} // else if (texrow)
	viewSourceTV->blockSignals(false);
}


// only used in DEVEL_MODE for debugging
// need a proper LFUN if we want to implement it in release mode
void ViewSourceWidget::gotoCursor()
{
	if (!bv_ || !texrow_)
		return;
	int row = viewSourceTV->textCursor().blockNumber() + 1;
	const_cast<BufferView *>(bv_)->setCursorFromRow(row, *texrow_);
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
	setViewFormat(index);

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

void ViewSourceWidget::saveSession(QString const & session_key) const
{
	QSettings settings;
	settings.setValue(session_key + "/output", view_format_);
	settings.setValue(session_key + "/contents", contentsCO->currentIndex());
	settings.setValue(session_key + "/autoupdate", autoUpdateCB->isChecked());
	settings.setValue(session_key + "/masterview",
					  masterPerspectiveCB->isChecked());
}


void ViewSourceWidget::restoreSession(QString const & session_key)
{
	QSettings settings;
    view_format_ = settings.value(session_key + "/output", 0).toString();
	contentsCO->setCurrentIndex(settings
								.value(session_key + "/contents", 0)
								.toInt());
	masterPerspectiveCB->setChecked(settings
									.value(session_key + "/masterview", false)
									.toBool());
	bool const checked = settings
		.value(session_key + "/autoupdate", true)
		.toBool();
	autoUpdateCB->setChecked(checked);
	if (checked)
		updateView();
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
		widget_->contentsChanged();
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
			//FIXME: this is shown for LyXHTML source, LyX source, etc.
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
	widget_->saveSession(sessionKey());
}


void GuiViewSource::restoreSession()
{
	DockView::restoreSession();
	widget_->restoreSession(sessionKey());
}


Dialog * createGuiViewSource(GuiView & lv)
{
	return new GuiViewSource(lv);
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiViewSource.cpp"
