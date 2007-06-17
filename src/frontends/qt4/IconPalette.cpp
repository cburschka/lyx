/**
 * \file IconPalette.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "IconPalette.h"
#include "qt_helpers.h"
#include "controllers/ControlMath.h" // for find_xpm

#include <QPixmap>
#include <QGridLayout>
#include <QToolButton>
#include <QToolTip>
#include <QToolBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QMouseEvent>

namespace lyx {
namespace frontend {

#if QT_VERSION >= 0x040200


class MathButton : public QToolButton
{
public:
	MathButton(QWidget * parent = 0) {}
	void mouseReleaseEvent(QMouseEvent *event); 
	void mousePressEvent(QMouseEvent *event); 
};


void MathButton::mouseReleaseEvent(QMouseEvent *event)
{
	QToolButton::mouseReleaseEvent(event);
	event->ignore();
}


void MathButton::mousePressEvent(QMouseEvent *event)
{
	QToolButton::mousePressEvent(event);
	event->ignore();
}


IconPalette::IconPalette(QWidget * parent)
	: QWidgetAction(parent), size_(QSize(22, 22))
{
}


void IconPalette::addButton(QAction * action)
{
	actions_.push_back(action);
}


QWidget * IconPalette::createWidget(QWidget * parent)
{
	QWidget * widget = new QWidget(parent);
	QGridLayout * layout = new QGridLayout(widget);
	layout->setSpacing(0);

	for (int i = 0; i < actions_.size(); ++i) {
		MathButton * tb = new MathButton(widget);
		tb->setAutoRaise(true);
		tb->setDefaultAction(actions_.at(i));
		tb->setIconSize(size_);
		connect(this, SIGNAL(iconSizeChanged(const QSize &)),
			tb, SLOT(setIconSize(const QSize &)));
	
		int const row = i/qMin(6, i + 1) + 1;
		int const col = qMax(1, i + 1 - (row - 1) * 6);
		layout->addWidget(tb, row, col);
	}

	return widget;
}


void IconPalette::setIconSize(const QSize & size)
{
	size_ = size;
	// signal
	iconSizeChanged(size);
}


void IconPalette::updateParent()
{
	bool enable = false;
	for (int i = 0; i < actions_.size(); ++i)
		if (actions_.at(i)->isEnabled()) {
			enable = true;
			break;
		}
	// signal
	enabled(enable);
}

#else  // QT_VERSION >= 0x040200

IconPalette::IconPalette(QWidget * parent)
	: QWidget(parent, Qt::Popup)
{
	layout_ = new QGridLayout(this);
	layout_->setSpacing(0);
	layout_->setMargin(3);
}


void IconPalette::addButton(QAction * action)
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


void IconPalette::clicked(QAction * action)
{
	triggered(action);
	setVisible(false);
}


void IconPalette::showEvent(QShowEvent * event)
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


void IconPalette::hideEvent(QHideEvent * event )
{
	visible(false);
	QWidget::hideEvent(event);
}


void IconPalette::updateParent()
{
	bool enable = false;
	for (int i = 0; i < actions_.size(); ++i)
		if (actions_.at(i)->isEnabled()) {
			enable = true;
			break;
		}

	parentWidget()->setEnabled(enable);
}


void IconPalette::paintEvent(QPaintEvent * event)
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
#endif // QT_VERSION >= 0x040200


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

#include "IconPalette_moc.cpp"
