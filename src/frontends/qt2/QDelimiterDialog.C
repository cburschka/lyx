/**
 * \file QDelimiterDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "support/filetools.h"
#include "gettext.h"
#include "debug.h"
 
#include "QMath.h"
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

	string do_match(string str) {
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
 
}

QDelimiterDialog::QDelimiterDialog(QMath * form)
	: QDelimiterDialogBase(0, 0, false, 0),
	form_(form)
{
	setCaption(_("LyX: Delimiters"));
 
	for (int i = 0; *delim[i]; ++i) {
		string xpm_name = LibFileSearch("images/math/", delim[i], "xpm");
		leftIP->add(QPixmap(xpm_name.c_str()), delim[i], delim[i]);
	}
	leftIP->add(QPixmap(LibFileSearch("images/math/", "empty", "xpm").c_str()), "empty", "empty");
	connect(leftIP, SIGNAL(button_clicked(string)), this, SLOT(ldelim_clicked(string)));
	ldelim_clicked("(");

	for (int i = 0; *delim[i]; ++i) {
		string xpm_name = LibFileSearch("images/math/", delim[i], "xpm");
		rightIP->add(QPixmap(xpm_name.c_str()), delim[i], delim[i]);
	}
	rightIP->add(QPixmap(LibFileSearch("images/math/", "empty", "xpm").c_str()), "empty", "empty");
	connect(rightIP, SIGNAL(button_clicked(string)), this, SLOT(rdelim_clicked(string)));
	rdelim_clicked(")");
}

 
namespace {
	string fix_name(string const & str) {
		if (str == "slash")
			return "/";
		if (str == "backslash")
			return "\\";
		if (str == "empty")
			return "";
		return str;
	}
}
 
void QDelimiterDialog::insertClicked()
{
	form_->insertDelim(fix_name(left_) + " " + fix_name(right_));
}

 
void QDelimiterDialog::set_label(QLabel * label, string const & str)
{
	string xpm_name = LibFileSearch("images/math/", str, "xpm");
	label->setUpdatesEnabled(false);
	label->setPixmap(QPixmap(xpm_name.c_str()));
	label->setUpdatesEnabled(true);
	label->update();
}

 
void QDelimiterDialog::ldelim_clicked(string str)
{
	left_ = str;
 
	set_label(leftPI, left_);
	if (matchCB->isChecked()) {
		right_ = do_match(left_);
		set_label(rightPI, right_);
	}
}


void QDelimiterDialog::rdelim_clicked(string str)
{
	right_ = str;
 
	set_label(rightPI, right_);
	if (matchCB->isChecked()) {
		left_ = do_match(right_);
		set_label(leftPI, left_);
	}
}
