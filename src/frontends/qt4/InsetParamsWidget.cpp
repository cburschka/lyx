// -*- C++ -*-
/**
 * \file InsetParamsWidget.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetParamsWidget.h"

#include "insets/InsetCommand.h"

#include "qt_helpers.h"

#include <QLineEdit>

namespace lyx {

namespace frontend {

CheckedWidget::CheckedWidget(QLineEdit * input, QWidget * label)
	: input_(input), label_(label)
{
}


bool CheckedWidget::check() const
{
	bool const valid = input_->hasAcceptableInput();
	// Visual feedback.
	setValid(input_, valid);
	if (label_)
		setValid(label_, valid);
	return valid;
}



InsetParamsWidget::InsetParamsWidget(QWidget * parent) : QWidget(parent)
{
}


void InsetParamsWidget::addCheckedWidget(QLineEdit * input, QWidget * label)
{
	checked_widgets_.append(CheckedWidget(input, label));
}


bool InsetParamsWidget::checkWidgets() const
{
	bool valid = true;
	Q_FOREACH(CheckedWidget const & le, checked_widgets_)
			valid &= le.check();
	return valid;
}


bool InsetParamsWidget::initialiseParams(std::string const & data)
{
	InsetCommandParams p(insetCode());
	if (!InsetCommand::string2params(data, p))
		return false;
	paramsToDialog(p);
	return true;
}

} // namespace frontend
} // namespace lyx

#include "moc_InsetParamsWidget.cpp"
