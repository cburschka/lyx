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

#include "GuiApplication.h"
#include "LaTeXHighlighter.h"
#include "qt_helpers.h"

#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "Format.h"
#include "GuiView.h"
#include "FuncRequest.h"
#include "LyX.h"
#include "TexRow.h"

#include "support/debug.h"
#include "support/lassert.h"
#include "support/docstream.h"
#include "support/docstring_list.h"
#include "support/gettext.h"

#include <QBoxLayout>
#include <QScrollBar>
#include <QSettings>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QVariant>

using namespace std;

namespace lyx {
namespace frontend {

ViewSourceWidget::ViewSourceWidget(QWidget * parent)
	:	QWidget(parent),
		document_(new QTextDocument(this)),
		highlighter_(new LaTeXHighlighter(document_))
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
		this, SIGNAL(needUpdate()));
	connect(outputFormatCO, SIGNAL(activated(int)),
		this, SLOT(setViewFormat(int)));

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

	// catch double click events
	viewSourceTV->viewport()->installEventFilter(this);
}


void ViewSourceWidget::getContent(BufferView const & view,
			Buffer::OutputWhat output, docstring & str, string const & format,
			bool master)
{
	// get the *top* level paragraphs that contain the cursor,
	// or the selected text
	pit_type par_begin;
	pit_type par_end;

	if (!view.cursor().selection()) {
		par_begin = view.cursor().bottom().pit();
		par_end = par_begin;
	} else {
		par_begin = view.cursor().selectionBegin().bottom().pit();
		par_end = view.cursor().selectionEnd().bottom().pit();
	}
	if (par_begin > par_end)
		swap(par_begin, par_end);
	odocstringstream ostr;
	texrow_ = view.buffer()
		.getSourceCode(ostr, format, par_begin, par_end + 1, output, master);
	//ensure that the last line can always be selected in its full width
	str = ostr.str() + "\n";
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
		Q_EMIT needUpdate();
}


void ViewSourceWidget::setViewFormat(int const index)
{
	outputFormatCO->setCurrentIndex(index);
	string format = fromqstr(outputFormatCO->itemData(index).toString());
	if (view_format_ != format) {
		view_format_ = format;
		Q_EMIT needUpdate();
	}
}


int ViewSourceWidget::updateDelay() const
{
	const int long_delay = 400;
	const int short_delay = 60;
	// a shorter delay if just the current paragraph is shown
	return (contentsCO->currentIndex() == 0) ? short_delay : long_delay;
}


void GuiViewSource::scheduleUpdate()
{
	update_timer_->start(widget_->updateDelay());
}


void GuiViewSource::scheduleUpdateNow()
{
	update_timer_->start(0);
}


void GuiViewSource::realUpdateView()
{
	widget_->updateView(bufferview());
	updateTitle();
}


void ViewSourceWidget::updateView(BufferView const * bv)
{
	if (!bv) {
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

	Buffer::OutputWhat output = Buffer::CurrentParagraph;
	if (contentsCO->currentIndex() == 1)
		output = Buffer::FullSource;
	else if (contentsCO->currentIndex() == 2)
		output = Buffer::OnlyPreamble;
	else if (contentsCO->currentIndex() == 3)
		output = Buffer::OnlyBody;

	docstring content;
	getContent(*bv, output, content, view_format_,
	           masterPerspectiveCB->isChecked());
	QString old = document_->toPlainText();
	QString qcontent = toqstr(content);
	if (guiApp->currentView()->develMode()) {
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
	}

	// prevent gotoCursor()
	QSignalBlocker blocker(viewSourceTV);
	bool const changed = setText(qcontent);

	if (changed && !texrow_) {
		// position-to-row is unavailable
		// we jump to the first modification
		int length = min(old.length(), qcontent.length());
		int pos = 0;
		for (; pos < length && old.at(pos) == qcontent.at(pos); ++pos) {}
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
		std::pair<int,int> rows = texrow_->rowFromCursor(bv->cursor());
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
}


docstring ViewSourceWidget::currentFormatName(BufferView const * bv) const
{
	// Compute the actual format used
	string const format = !bv ? ""
		: flavor2format(bv->buffer().params().getOutputFlavor(view_format_));
	Format const * f = theFormats().getFormat(format.empty() ? view_format_ : format);
	return f ? f->prettyname() : from_utf8(view_format_);
}


bool ViewSourceWidget::eventFilter(QObject * obj, QEvent * ev)
{
	// this event filter is installed on the viewport of the QTextView
	if (obj == viewSourceTV->viewport() &&
	    ev->type() == QEvent::MouseButtonDblClick) {
		goToCursor();
		return true;
	}
	return false;
}


void ViewSourceWidget::goToCursor() const
{
	if (!texrow_)
		return;
	int row = viewSourceTV->textCursor().blockNumber() + 1;
	dispatch(texrow_->goToFuncFromRow(row));
}



void ViewSourceWidget::updateDefaultFormat(BufferView const & bv)
{
	QSignalBlocker blocker(outputFormatCO);
	outputFormatCO->clear();
	outputFormatCO->addItem(qt_("Default"),
	                        QVariant(QString("default")));

	int index = 0;
	for (string const & fmt_name : bv.buffer().params().backends()) {
		Format const * fmt = theFormats().getFormat(fmt_name);
		if (!fmt) {
			LYXERR0("Can't find format for backend " << fmt_name << "!");
			continue;
		}
		QString const pretty = toqstr(translateIfPossible(fmt->prettyname()));
		outputFormatCO->addItem(pretty, QVariant(toqstr(fmt_name)));
		if (fmt_name == view_format_)
			index = outputFormatCO->count() - 1;
	}
	setViewFormat(index);
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


void ViewSourceWidget::saveSession(QSettings & settings, QString const & session_key) const
{
	settings.setValue(session_key + "/output", toqstr(view_format_));
	settings.setValue(session_key + "/contents", contentsCO->currentIndex());
	settings.setValue(session_key + "/autoupdate", autoUpdateCB->isChecked());
	settings.setValue(session_key + "/masterview",
					  masterPerspectiveCB->isChecked());
}


void ViewSourceWidget::restoreSession(QString const & session_key)
{
	QSettings settings;
	view_format_ = fromqstr(settings.value(session_key + "/output", 0)
	                        .toString());
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
		Q_EMIT needUpdate();
}


GuiViewSource::GuiViewSource(GuiView & parent,
		Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "view-source", qt_("Code Preview"), area, flags),
	  widget_(new ViewSourceWidget(this)),
	  update_timer_(new QTimer(this))
{
	setWidget(widget_);

	// setting the update timer
	update_timer_->setSingleShot(true);
	connect(update_timer_, SIGNAL(timeout()),
	        this, SLOT(realUpdateView()));

	connect(widget_, SIGNAL(needUpdate()), this, SLOT(scheduleUpdateNow()));
}


void GuiViewSource::onBufferViewChanged()
{
	widget_->setText();
	widget_->setEnabled((bool)bufferview());
}


void GuiViewSource::updateView()
{
	if (widget_->autoUpdateCB->isChecked()) {
		widget_->setEnabled((bool)bufferview());
		scheduleUpdate();
	}
	widget_->masterPerspectiveCB->setEnabled(buffer().parent());
	updateTitle();
}


void GuiViewSource::enableView(bool enable)
{
	widget_->setEnabled((bool)bufferview());
	if (bufferview())
		widget_->updateDefaultFormat(*bufferview());
	if (!enable)
		// In the opposite case, updateView() will be called anyway.
		widget_->contentsChanged();
}


bool GuiViewSource::initialiseParams(string const & /*source*/)
{
	updateTitle();
	return true;
}


void GuiViewSource::updateTitle()
{
	docstring const format = widget_->currentFormatName(bufferview());
	QString const title = format.empty() ? qt_("Code Preview")
		: qt_("%1[[preview format name]] Preview")
		  .arg(toqstr(translateIfPossible(format)));
	setTitle(title);
	setWindowTitle(title);
}


void GuiViewSource::saveSession(QSettings & settings) const
{
	Dialog::saveSession(settings);
	widget_->saveSession(settings, sessionKey());
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
