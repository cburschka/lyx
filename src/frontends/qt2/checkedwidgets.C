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

void setWarningColor(QWidget * widget)
{
	// Qt 2.3 does not have
	// widget->setPaletteForegroundColor(QColor(255, 0, 0));
	// So copy the appropriate parts of the function here:
	QPalette pal = widget->palette();
	pal.setColor(QPalette::Active,
		     QColorGroup::Foreground,
		     QColor(255, 0, 0));
	widget->setPalette(pal);
}

	
void setWidget(bool valid, QLineEdit * input, QLabel * label)
{
	if (valid)
		input->unsetPalette();
	else
		setWarningColor(input);
	
	if (!label)
		return;

	if (valid)
		label->unsetPalette();
	else
		setWarningColor(label);
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
