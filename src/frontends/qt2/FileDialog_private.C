/**
 * \file FileDialog_private.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "LString.h"
#include "support/lstrings.h"

#include <qapplication.h>
#include <qfiledialog.h>
#include <qtoolbutton.h> 

#include "QtLyXView.h"
#include "debug.h"
#include "funcrequest.h"

#include "FileDialog_private.h"

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
		label.insert(pos, "&");
		return label;
	}
}
 
LyXFileDialog::LyXFileDialog(string const & p, string const & m, string const & t,
		FileDialog::Button const & b1, FileDialog::Button const & b2)
	: QFileDialog(p.c_str(), m.c_str(), qApp->mainWidget(), t.c_str(), true),
	  b1_(0), b2_(0)
{
	setCaption(t.c_str());

	if (!b1.first.empty()) { 
		b1_dir_ = b1.second;
		b1_ = new QToolButton(this);
		connect(b1_, SIGNAL(clicked()), this, SLOT(buttonClicked()));
		b1_->setText(getLabel(b1.first).c_str());
		addToolButton(b1_, true);
	}

	if (!b2.first.empty()) {
		b2_dir_ = b2.second;
		b2_ = new QToolButton(this);
		connect(b2_, SIGNAL(clicked()), this, SLOT(buttonClicked()));
		b2_->setText(getLabel(b2.first).c_str());
		addToolButton(b2_);
	}
}


void LyXFileDialog::buttonClicked()
{
	if (sender() == b1_)
		setDir(b1_dir_.c_str());
	else if (sender() == b2_)
		setDir(b2_dir_.c_str());
}
