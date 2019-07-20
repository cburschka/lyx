/**
 * \file LyXFileDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LyXFileDialog.h"

#include "qt_helpers.h"

#include "support/lstrings.h"

#include <QApplication>
#include <QToolButton>
#include <QHBoxLayout>

using namespace std;
using namespace lyx::support;

namespace lyx {


LyXFileDialog::LyXFileDialog(QString const & title,
			     QString const & path,
			     QStringList const & filters,
			     FileDialog::Button const & b1,
			     FileDialog::Button const & b2)
				 // FIXME replace that with guiApp->currentView()
	: QFileDialog(qApp->focusWidget(), title, path)
{
	setNameFilters(filters);
	setWindowTitle(title);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setOption(QFileDialog::DontUseNativeDialog);

	QList<QHBoxLayout *> layout = findChildren<QHBoxLayout *>();

	if (!b1.first.isEmpty()) {
		b1_dir_ = b1.second;
		QToolButton * tb = new QToolButton(this);
		connect(tb, SIGNAL(clicked()), this, SLOT(button1Clicked()));
		tb->setText(b1.first);
		layout.at(0)->addWidget(tb);
	}

	if (!b2.first.isEmpty()) {
		b2_dir_ = b2.second;
		QToolButton * tb = new QToolButton(this);
		connect(tb, SIGNAL(clicked()), this, SLOT(button2Clicked()));
		tb->setText(b2.first);
		layout.at(0)->addWidget(tb);
	}
}


void LyXFileDialog::button1Clicked()
{
	setDirectory(b1_dir_);
}


void LyXFileDialog::button2Clicked()
{
	setDirectory(b2_dir_);
}

} // namespace lyx

#include "moc_LyXFileDialog.cpp"
