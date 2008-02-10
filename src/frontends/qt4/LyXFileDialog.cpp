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

#include "support/FileFilterList.h"
#include "support/lstrings.h"

#include <QApplication>
#include <QToolButton>
#include <QHBoxLayout>

using namespace std;
using namespace lyx::support;

namespace lyx {

/// return the Qt form of the label
static docstring const getLabel(docstring const & ucs4str)
{
	// FIXME UNICODE
	string str = to_utf8(ucs4str);
	string label;
	string sc = split(str, label, '|');
	if (sc.length() < 2)
		return from_utf8(label);
	size_t pos = label.find(sc[1]);
	if (pos != string::npos)
		label.insert(pos, 1, '&');
	return from_utf8(label);
}


LyXFileDialog::LyXFileDialog(docstring const & t,
			     docstring const & p,
			     support::FileFilterList const & filters,
			     FileDialog::Button const & b1,
			     FileDialog::Button const & b2)
				 // FIXME replace that with guiApp->currentView()
	: QFileDialog(qApp->focusWidget(),
		      toqstr(t), toqstr(p), toqstr(filters.as_string()))
{
	QString const path = toqstr(p);
	QDir dir(path);
	// FIXME: workaround for a bug in qt which makes LyX crash
	// with hidden paths (bug 4513). Recheck with recent Qt versions.
	if (path.contains("/."))
		dir.setFilter(QDir::Hidden);
	setDirectory(dir);
	setWindowTitle(toqstr(t));

	QList<QHBoxLayout *> layout = findChildren<QHBoxLayout *>();

	if (!b1.first.empty()) {
		b1_dir_ = b1.second;
		QToolButton * tb = new QToolButton(this);
		connect(tb, SIGNAL(clicked()), this, SLOT(button1Clicked()));
		tb->setText(toqstr(getLabel(b1.first)));
		layout.at(0)->addWidget(tb);
	}

	if (!b2.first.empty()) {
		b2_dir_ = b2.second;
		QToolButton * tb = new QToolButton(this);
		connect(tb, SIGNAL(clicked()), this, SLOT(button2Clicked()));
		tb->setText(toqstr(getLabel(b2.first)));
		layout.at(0)->addWidget(tb);
	}
}


void LyXFileDialog::button1Clicked()
{
	setDirectory(toqstr(b1_dir_));
}


void LyXFileDialog::button2Clicked()
{
	setDirectory(toqstr(b2_dir_));
}

} // namespace lyx

#include "LyXFileDialog_moc.cpp"
