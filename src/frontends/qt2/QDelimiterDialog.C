/**
 * \file QDelimiterDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/filetools.h"
#include "qt_helpers.h"
#include "debug.h"

#include "QMath.h"
#include "ControlMath.h"
#include "QDelimiterDialog.h"

#include "iconpalette.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

namespace {

char const * delim[] = {
	"(", ")", "{", "}", "[", "]",
	"lceil", "rceil", "lfloor", "rfloor", "langle", "rangle",
	"uparrow", "Uparrow", "downarrow", "Downarrow",
	"|", "Vert", "slash", "backslash", ""
};


string do_match(string const & str)
{
	if (str == "(") return ")";
	if (str == ")") return "(";
	if (str == "[") return "]";
	if (str == "]") return "[";
	if (str == "{") return "}";
	if (str == "}") return "{";
	if (str == "l") return "r";
	if (str == "rceil") return "lceil";
	if (str == "lceil") return "rceil";
	if (str == "rfloor") return "lfloor";
	if (str == "lfloor") return "rfloor";
	if (str == "rangle") return "langle";
	if (str == "langle") return "rangle";
	if (str == "backslash") return "slash";
	if (str == "slash") return "backslash";
	return str;
}


string fix_name(string const & str)
{
	if (str == "slash")
		return "/";
	if (str == "backslash")
		return "\\";
	if (str == "empty")
		return ".";
	return str;
}

} // namespace anon


QDelimiterDialog::QDelimiterDialog(QMath * form)
	: QDelimiterDialogBase(0, 0, false, 0),
	form_(form)
{
	setCaption(qt_("LyX: Delimiters"));

	for (int i = 0; *delim[i]; ++i) {
		string xpm(find_xpm(delim[i]));
		leftIP->add(QPixmap(toqstr(xpm)), delim[i], delim[i]);
		rightIP->add(QPixmap(toqstr(xpm)), delim[i], delim[i]);
	}

	string empty_xpm(find_xpm("empty"));

	leftIP->add(QPixmap(toqstr(empty_xpm)), "empty", "empty");
	rightIP->add(QPixmap(toqstr(empty_xpm)), "empty", "empty");
	connect(leftIP, SIGNAL(button_clicked(const string &)), this, SLOT(ldelim_clicked(const string &)));
	connect(rightIP, SIGNAL(button_clicked(const string &)), this, SLOT(rdelim_clicked(const string &)));
	ldelim_clicked("(");
	rdelim_clicked(")");
}


void QDelimiterDialog::insertClicked()
{
	form_->insertDelim(fix_name(left_) + ' ' + fix_name(right_));
}


void QDelimiterDialog::set_label(QLabel * label, string const & str)
{
	label->setUpdatesEnabled(false);
	label->setPixmap(QPixmap(toqstr(find_xpm(str))));
	label->setUpdatesEnabled(true);
	label->update();
}


void QDelimiterDialog::ldelim_clicked(string const & str)
{
	left_ = str;

	set_label(leftPI, left_);
	if (matchCB->isChecked()) {
		right_ = do_match(left_);
		set_label(rightPI, right_);
	}
}


void QDelimiterDialog::rdelim_clicked(string const & str)
{
	right_ = str;

	set_label(rightPI, right_);
	if (matchCB->isChecked()) {
		left_ = do_match(right_);
		set_label(leftPI, left_);
	}
}
