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

#include "debug.h"

#include "qt_helpers.h"

#include <QVBoxLayout>
#include <QPixmap>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QResizeEvent>
#include <QLayout>
#include <QPushButton>
#include <QToolTip>

using std::endl;
using std::make_pair;
using std::max;
using std::string;
using std::vector;


namespace lyx {

int const button_size = 32;


IconPalette::IconPalette(QWidget * parent)
	: QWidget(parent), maxcol_(-1)
{
	QVBoxLayout * top = new QVBoxLayout(this);
	QHBoxLayout * row = new QHBoxLayout(this);
	layout_ = new QGridLayout(this);
	top->insertLayout(-1, row);
	row->insertLayout(-1, layout_);
	row->addStretch(0);
	top->addStretch(0);
}


void IconPalette::add(QPixmap const & pixmap, string name, string tooltip)
{
	QPushButton * p = new QPushButton(this);
	p->setFixedSize(button_size, button_size);
	p->setIcon(QIcon(pixmap));
	p->setToolTip(toqstr(tooltip));
	connect(p, SIGNAL(clicked()), this, SLOT(clicked()));
	buttons_.push_back(make_pair(p, name));
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


void IconPalette::resizeEvent(QResizeEvent * e)
{
	QWidget::resizeEvent(e);

	lyxerr[Debug::GUI] << "resize panel to "
		<< e->size().width() << ',' << e->size().height() << endl;

	int maxcol = e->size().width() / button_size;

	if (!layout_->isEmpty() && maxcol == maxcol_)
		return;

	int cols = max(width() / button_size, 1);
	int rows = max(int(buttons_.size() / cols), 1);
	if (buttons_.size() % cols)
		++rows;

	lyxerr[Debug::GUI] << "Laying out " << buttons_.size() << " widgets in a "
		<< cols << 'x' << rows << " grid." << endl;

	setUpdatesEnabled(false);

	// clear layout
	int i = 0;
	QLayoutItem *child;
	while ((child = layout_->itemAt(i)) != 0) {
		layout_->takeAt(i);
		++i;
	} 

	layout_->invalidate();

	vector<Button>::const_iterator it(buttons_.begin());
	vector<Button>::const_iterator const end(buttons_.end());

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			layout_->addWidget(it->first, i, j);
			++it;
			if (it == end)
				goto out;
		}
	}

out:

	resize(cols * button_size, rows * button_size);

	maxcol_ = cols;

	setUpdatesEnabled(true);
	update();
}


} // namespace lyx

#include "iconpalette_moc.cpp"
