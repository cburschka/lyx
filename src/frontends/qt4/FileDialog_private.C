/**
 * \file FileDialog_private.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FileDialog_private.h"

#include "qt_helpers.h"

#include "support/filefilterlist.h"
#include "support/lstrings.h"

#include <QApplication>
#include <QToolButton>
#include <QHBoxLayout>

using lyx::support::split;

using std::string;


namespace {

/// return the Qt form of the label
string const getLabel(string const & str) {
	string label;
	string sc(split(str, label, '|'));
	if (sc.length() < 2)
		return label;
	string::size_type pos = label.find(sc[1]);
	if (pos == string::npos)
		return label;
	label.insert(pos, 1, '&');
	return label;
}

} // namespace anon


LyXFileDialog::LyXFileDialog(string const & t,
			     string const & p,
			     lyx::support::FileFilterList const & filters,
			     FileDialog::Button const & b1,
			     FileDialog::Button const & b2)
				 // FIXME replace that with theApp->gui()->currentView()
	: QFileDialog(qApp->focusWidget(),
		      toqstr(t), toqstr(p), toqstr(filters.as_string())),
		      b1_(0), b2_(0)
{
	setWindowTitle(toqstr(t));

	QList<QHBoxLayout *> layout = findChildren<QHBoxLayout *>();

	if (!b1.first.empty()) {
		b1_dir_ = b1.second;
		b1_ = new QToolButton(this);
		connect(b1_, SIGNAL(clicked()), this, SLOT(buttonClicked()));
		b1_->setText(toqstr(getLabel(b1.first)));
		layout.at(0)->addWidget(b1_);
	}

	if (!b2.first.empty()) {
		b2_dir_ = b2.second;
		b2_ = new QToolButton(this);
		connect(b2_, SIGNAL(clicked()), this, SLOT(buttonClicked()));
		b2_->setText(toqstr(getLabel(b2.first)));
		layout.at(0)->addWidget(b2_);
	}
}


void LyXFileDialog::buttonClicked()
{
	if (sender() == b1_)
		setDirectory(toqstr(b1_dir_));
	else if (sender() == b2_)
		setDirectory(toqstr(b2_dir_));
}

#include "FileDialog_private_moc.cpp"
