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
#include <QToolTip>

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


} // namespace frontend
} // namespace lyx

#include "iconpalette_moc.cpp"
