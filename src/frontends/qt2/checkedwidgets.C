/**
 * \file qt2/checkedwidgets.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "checkedwidgets.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qvalidator.h>


void addCheckedLineEdit(BCView & bcview,
			QLineEdit * input, QLabel * label)
{
	bcview.addCheckedWidget(new CheckedLineEdit(input, label));
}
	

namespace {

void setWidget(bool valid, QLineEdit * input, QLabel * label)
{
	QColor const red(255, 0, 0);

	if (valid)
		input->unsetPalette();
	else
		input->setPaletteForegroundColor(red);
	
	if (!label)
		return;

	if (valid)
		label->unsetPalette();
	else
		label->setPaletteForegroundColor(red);
}

} // namespace anon


CheckedLineEdit::CheckedLineEdit(QLineEdit * input, QLabel * label)
	: input_(input), label_(label)
{}


bool CheckedLineEdit::check() const
{
	QValidator const * validator = input_->validator();
	if (!validator)
		return true;

	QString t = input_->text();
	int p = 0;
	bool const valid = validator->validate(t, p) == QValidator::Acceptable;

	// Visual feedback.
	setWidget(valid, input_, label_);

	return valid;
}
