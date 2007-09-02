/**
 * \file CheckedLineEdit.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "CheckedLineEdit.h"
#include "Qt2BC.h"

#include <QLabel>
#include <QLineEdit>
#include <QValidator>

namespace lyx {
namespace frontend {

void addCheckedLineEdit(BCView & bcview, QLineEdit * input, QWidget * label)
{
	Qt2BC * bc = static_cast<Qt2BC *>(&bcview);
	bc->addCheckedWidget(new CheckedLineEdit(input, label));
}


namespace {

void setWarningColor(QWidget * widget)
{
	QPalette pal = widget->palette();
	pal.setColor(QPalette::Active, QPalette::Foreground, QColor(255, 0, 0));
	widget->setPalette(pal);
}


void setWidget(bool valid, QLineEdit * input, QWidget * label)
{
	if (valid)
		input->setPalette(QPalette());
	else
		setWarningColor(input);

	if (!label)
		return;

	if (valid)
		label->setPalette(QPalette());
	else
		setWarningColor(label);
}

} // namespace anon


CheckedLineEdit::CheckedLineEdit(QLineEdit * input, QWidget * label)
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

} // namespace frontend
} // namespace lyx
