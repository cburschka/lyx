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
#include <QVBoxLayout>

namespace lyx {
namespace frontend {

TearOff::TearOff(QWidget * parent) 
	: QWidget(parent)
{
	highlighted_ = false;
	// + 2 because the default is a bit tight, see also:
	// http://trolltech.com/developer/task-tracker/index_html?id=167954&method=entry
	setMinimumHeight(style()->pixelMetric(QStyle::PM_MenuTearoffHeight) + 2);
	setToolTip(qt_("Click to detach"));
	// trigger tooltip (children of popups do not receive mousemove events)
	setMouseTracking(true);
}


void TearOff::mouseReleaseEvent(QMouseEvent * event)
{
	// signal
	tearOff();
	event->accept();
}


void TearOff::enterEvent(QEvent * event)
{
	highlighted_ = true;
	update();
	event->ignore();
}


void TearOff::leaveEvent(QEvent * event)
{
	highlighted_ = false;
	update();
	event->ignore();
}


void TearOff::paintEvent(QPaintEvent * event)
{
	QPainter p(this);
	const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
	QStyleOptionMenuItem menuOpt;
	menuOpt.initFrom(this);
	menuOpt.palette = palette();
	menuOpt.state = QStyle::State_None;
	menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
	menuOpt.menuRect = rect();
	menuOpt.maxIconWidth = 0;
	menuOpt.tabWidth = 0;
	menuOpt.menuItemType = QStyleOptionMenuItem::TearOff;
	menuOpt.rect.setRect(fw, fw, width() - (fw * 2),
		style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, this));
	p.setClipRect(menuOpt.rect);
	menuOpt.state = QStyle::State_None;
	if (highlighted_)
		menuOpt.state |= QStyle::State_Selected;
	style()->drawControl(QStyle::CE_MenuTearoff, &menuOpt, &p, this);
	event->accept();
}


IconPalette::IconPalette(QWidget * parent)
	: QWidget(parent, Qt::Popup), tornoff_(false)
{
	QVBoxLayout * v = new QVBoxLayout(this);
	v->setMargin(0);
	v->setSpacing(0);
	layout_ = new QGridLayout;
	layout_->setSpacing(0);
	const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
	layout_->setMargin(fw);
	tearoffwidget_ = new TearOff(this);
	connect(tearoffwidget_, SIGNAL(tearOff()), this, SLOT(tearOff()));
	v->addWidget(tearoffwidget_);
	v->addLayout(layout_);
}


void IconPalette::addButton(QAction * action)
{
	actions_.push_back(action);
	QToolButton * tb = new QToolButton;
	tb->setAutoRaise(true);
	tb->setDefaultAction(action);
	// trigger tooltip (children of popups do not receive mousemove events)
	tb->setMouseTracking(true);

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


void IconPalette::tearOff()
{
	blockSignals(true);
	hide();
	setWindowFlags(Qt::Tool);
	tornoff_ = true;
	tearoffwidget_->setVisible(!tornoff_);
	show();
	blockSignals(false);
}


void IconPalette::clicked(QAction * action)
{
	triggered(action);
	if (!tornoff_)
		setVisible(false);
}


void IconPalette::showEvent(QShowEvent * event)
{
	resize(sizeHint());
	setMaximumSize(sizeHint());

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

	QRect r = rect();
	r.moveTo(gpos.x() + hoffset, gpos.y() + voffset);
	setGeometry(r);	
	event->accept();
}


void IconPalette::hideEvent(QHideEvent * event )
{
	QWidget::hideEvent(event);
	visible(false);
	if (tornoff_) {
		setWindowFlags(Qt::Popup);
		tornoff_ = false;
		tearoffwidget_->setVisible(!tornoff_);
	}
}


void IconPalette::updateParent()
{
	bool enable = false;

	// FIXME: so this is commented out for speed considerations
	// true fix is to repair the updating mechanism of the toolbar
#if 0
	for (int i = 0; i < actions_.size(); ++i)
		if (actions_.at(i)->isEnabled()) {
			enable = true;
			break;
		}
#else
	// we check only the first action to enable/disable the panel
	if (!actions_.isEmpty())
		enable = actions_.at(0)->isEnabled();
#endif

	parentWidget()->setEnabled(enable);
}


void IconPalette::paintEvent(QPaintEvent * event)
{
	// draw border
	const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
	if (fw && !tornoff_) {
		QPainter p(this);
		QRegion borderReg;
		borderReg += QRegion(QRect(0, 0, fw, height())); //left
		borderReg += QRegion(QRect(width() - fw, 0, fw, height())); //right
		borderReg += QRegion(QRect(0, 0, width(), fw)); //top
		borderReg += QRegion(QRect(0, height() - fw, width(), fw)); //bottom
		p.setClipRegion(borderReg);
		QStyleOptionFrame frame;
		frame.rect = rect();
		frame.palette = palette();
		frame.state = QStyle::State_None;
		frame.lineWidth = style()->pixelMetric(QStyle::PM_MenuPanelWidth);
		frame.midLineWidth = 0;
		style()->drawPrimitive(QStyle::PE_FrameMenu, &frame, &p, this);
	}
	event->accept();
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
	// FIXME: so this is commented out for speed considerations
	// true fix is to repair the updating mechanism of the toolbar
#if 0
	for (int i = 0; i < actions_.size(); ++i)
		if (actions_.at(i)->isEnabled()) {
			enable = true;
			break;
		}
#else
	// we check only the first action to enable/disable the menu
	if (!actions_.isEmpty())
		enable = actions_.at(0)->isEnabled();
#endif

	parentWidget()->setEnabled(enable);
}


} // namespace frontend
} // namespace lyx

#include "IconPalette_moc.cpp"
