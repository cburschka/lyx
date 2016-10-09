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

/// return the Qt form of the label
static QString getLabel(QString const & qstr)
{
	// FIXME UNICODE (or "qt-ify")
	string str = fromqstr(qstr);
	string label;
	string sc = split(str, label, '|');
	if (sc.length() < 2)
		return toqstr(label);
	size_t pos = label.find(sc[1]);
	if (pos != string::npos)
		label.insert(pos, 1, '&');
	return toqstr(label);
}


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
		tb->setText(getLabel(b1.first));
		layout.at(0)->addWidget(tb);
	}

	if (!b2.first.isEmpty()) {
		b2_dir_ = b2.second;
		QToolButton * tb = new QToolButton(this);
		connect(tb, SIGNAL(clicked()), this, SLOT(button2Clicked()));
		tb->setText(getLabel(b2.first));
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
