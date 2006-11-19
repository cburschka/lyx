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

IconPalette::IconPalette(QWidget * parent, char const ** entries)
	: QWidget(parent)
{
	QGridLayout * layout_ = new QGridLayout(this);
	layout_->setSpacing(0);

	int const button_size = 40;
	for (int i = 0; *entries[i]; ++i) {
		QPushButton * p = new QPushButton;
		p->setFixedSize(button_size, button_size);
		p->setIcon(QPixmap(toqstr(lyx::frontend::find_xpm(entries[i]))));
		p->setToolTip(toqstr(string("\\") + entries[i]));
		connect(p, SIGNAL(clicked()), this, SLOT(clicked()));
		buttons_.push_back(make_pair(p, entries[i]));
		// put in a grid layout with 5 cols
		int const row = i/5;
		layout_->addWidget(p, row, i - 5*row);
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
