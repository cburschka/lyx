/**
 * \file iconpalette.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "iconpalette.h"
#include "qt_helpers.h"
#include "controllers/ControlMath.h" // for find_xpm

#include <QPixmap>
#include <QGridLayout>
#include <QPushButton>
#include <QToolButton>
#include <QToolTip>
#include <QToolBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>

using std::make_pair;
using std::string;
using std::vector;


namespace lyx {
namespace frontend {

FlowLayout::FlowLayout(QWidget *parent) : QLayout(parent)
{
	setMargin(0);
	setSpacing(-1);
}


FlowLayout::~FlowLayout()
{
	QLayoutItem *item;
	while ((item = takeAt(0)))
		delete item;
}


void FlowLayout::addItem(QLayoutItem *item)
{
	itemList.append(item);
}


int FlowLayout::count() const
{
	return itemList.size();
}


QLayoutItem *FlowLayout::itemAt(int index) const
{
	return itemList.value(index);
}


QLayoutItem *FlowLayout::takeAt(int index)
{
	if (index >= 0 && index < itemList.size())
		return itemList.takeAt(index);
	else
		return 0;
}


Qt::Orientations FlowLayout::expandingDirections() const
{
	return 0;
}


bool FlowLayout::hasHeightForWidth() const
{
	return true;
}


int FlowLayout::heightForWidth(int width) const
{
	int height = doLayout(QRect(0, 0, width, 0), true);
	return height;
}


void FlowLayout::setGeometry(const QRect &rect)
{
	QLayout::setGeometry(rect);
	doLayout(rect, false);
}


QSize FlowLayout::sizeHint() const
{
	// default to (max) 6 columns
	int const cols = qMin(itemList.size(), 6);
	int const rows = (itemList.size() - 1 )/6 + 1;
	return QSize(cols * minimumSize().width() + 1,
		rows * minimumSize().height() + 1);
}


QSize FlowLayout::minimumSize() const
{
	QSize size;
	int const n = itemList.size();
    for (int i = 0; i < n; ++i) {
		size = size.expandedTo(itemList.at(i)->minimumSize());
	}
	return size;
}


int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
	int x = rect.x();
	int y = rect.y();
	int lineHeight = 0;

    for (int i = 0; i < itemList.size(); ++i) {
		QLayoutItem * item = itemList.at(i);
		int nextX = x + item->sizeHint().width() + spacing();
		if (nextX - spacing() > rect.right() && lineHeight > 0) {
			x = rect.x();
			y = y + lineHeight + spacing();
	    	nextX = x + item->sizeHint().width() + spacing();
			lineHeight = 0;
		}

		if (!testOnly)
			item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

		x = nextX;
		lineHeight = qMax(lineHeight, item->sizeHint().height());
	}
	return y + lineHeight - rect.y();
}


IconPalette::IconPalette(QWidget * parent, char const ** entries)
	: QWidget(parent)
{
	FlowLayout * layout_ = new FlowLayout(this);
	layout_->setSpacing(0);

	int const button_size = 40;
	for (int i = 0; *entries[i]; ++i) {
		QPushButton * p = new QPushButton;
		p->setFixedSize(button_size, button_size);
		p->setIcon(QPixmap(toqstr(lyx::frontend::find_xpm(entries[i]))));
		p->setToolTip(toqstr(string("\\") + entries[i]));
		connect(p, SIGNAL(clicked()), this, SLOT(clicked()));
		buttons_.push_back(make_pair(p, entries[i]));
		layout_->addWidget(p);
	}
#ifdef Q_WS_WIN
	// FIXME: This is a hack to work around bug 2859
	// http://bugzilla.lyx.org/show_bug.cgi?id=2859
	// Short description of the bug:
	/*
	Open the math panel and detach the operator panel by pressing the
	"Detach Panel" button. The detached panel is then always set too
	high in the left upper corner of the screen as in the attached
	screenshot.
	*/
	move(50, 50);
#endif
}


void IconPalette::clicked()
{
	vector<Button>::const_iterator it = buttons_.begin();
	vector<Button>::const_iterator const end = buttons_.end();
	for (; it != end; ++it) {
		if (sender() == it->first) {
 			// emit signal
			button_clicked(it->second);
			return;
		}
	}
}


IconPanel::IconPanel(QWidget * parent)
	: QWidget(parent, Qt::Popup)
{
	layout_ = new QGridLayout(this);
	layout_->setSpacing(0);
	layout_->setMargin(3);
	setLayout(layout_);
}


void IconPanel::addButton(QAction * action)
{
	actions_.push_back(action);
	QToolButton * tb = new QToolButton;
	tb->setAutoRaise(true);
	tb->setDefaultAction(action);
	connect(tb, SIGNAL(triggered(QAction *)),
		this, SLOT(clicked(QAction *)));
	QToolBar * toolbar = qobject_cast<QToolBar *>(parentWidget()->parentWidget());
	connect(toolbar, SIGNAL(iconSizeChanged(const QSize &)),
		tb, SLOT(setIconSize(const QSize &)));

	int const i = actions_.size();
	int const ncols = qMin(6, i);
	int const row = (i - 1)/ncols + 1;
	int const col = qMax(1, i - (row - 1) * 6);
	layout_->addWidget(tb, row, col);
}


void IconPanel::clicked(QAction * action)
{
	triggered(action);
	setVisible(false);
}


void IconPanel::showEvent(QShowEvent * event)
{
	int hoffset = - parentWidget()->pos().x();
	int voffset = - parentWidget()->pos().y();
	int const parwidth = parentWidget()->geometry().width();
	int const parheight = parentWidget()->geometry().height();

	// vertical toolbar?
	QToolBar * toolbar = qobject_cast<QToolBar *>(parentWidget()->parentWidget());
	if (toolbar && toolbar->orientation() == Qt::Vertical) {
		hoffset += parwidth;
		voffset -= parheight;
	}

	QRect const screen = qApp->desktop()->availableGeometry(this);
	QPoint const gpos = parentWidget()->mapToGlobal(
		parentWidget()->geometry().bottomLeft());

	// space to the right?
	if (gpos.x() + hoffset + width() > screen.width()) {
		hoffset -= width();
		if (toolbar && toolbar->orientation() == Qt::Vertical)
			hoffset -= parwidth;
		else
			hoffset += parwidth;
	}
	// space at the bottom?
	if (gpos.y() + voffset + height() > screen.height()) {
		voffset -= height();
		if (toolbar && toolbar->orientation() == Qt::Horizontal)
			voffset -= parheight;
		else
			voffset += parheight;
	}

	move(gpos.x() + hoffset, gpos.y() + voffset);
	QWidget::showEvent(event);
}


void IconPanel::hideEvent(QHideEvent * event )
{
	visible(false);
	QWidget::hideEvent(event);
}


void IconPanel::updateParent()
{
	bool enable = false;
	for (int i = 0; i < actions_.size(); ++i)	
		if (actions_.at(i)->isEnabled()) {
			enable = true;
			break;
		}

	parentWidget()->setEnabled(enable);
}


void IconPanel::paintEvent(QPaintEvent * event)
{
	// draw border
	QPainter p(this);
	QRegion emptyArea = QRegion(rect());
	const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
	if (fw) {
		QRegion borderReg;
		borderReg += QRect(0, 0, fw, height()); //left
		borderReg += QRect(width()-fw, 0, fw, height()); //right
		borderReg += QRect(0, 0, width(), fw); //top
		borderReg += QRect(0, height()-fw, width(), fw); //bottom
		p.setClipRegion(borderReg);
		emptyArea -= borderReg;
		QStyleOptionFrame frame;
		frame.rect = rect();
		frame.palette = palette();
		frame.state = QStyle::State_None;
		frame.lineWidth = style()->pixelMetric(QStyle::PM_MenuPanelWidth);
		frame.midLineWidth = 0;
		style()->drawPrimitive(QStyle::PE_FrameMenu, &frame, &p, this);
	}
	p.end();
	// draw the rest (buttons)
	QWidget::paintEvent(event);
}


ButtonMenu::ButtonMenu(const QString & title, QWidget * parent)
	: QMenu(title, parent)
{
}


void ButtonMenu::add(QAction * action)
{
	addAction(action);
	actions_.push_back(action);
}


void ButtonMenu::updateParent()
{
	bool enable = false;
	for (int i = 0; i < actions_.size(); ++i)	
		if (actions_.at(i)->isEnabled()) {
			enable = true;
			break;
		}

	parentWidget()->setEnabled(enable);
}


} // namespace frontend
} // namespace lyx

#include "iconpalette_moc.cpp"
