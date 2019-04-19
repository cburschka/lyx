/**
 * \file GuiBibitem.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBibitem.h"

#include "Buffer.h"
#include "BufferParams.h"

#include "qt_helpers.h"

#include "insets/InsetCommand.h"

#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {


GuiBibitem::GuiBibitem(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	connect(keyED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(labelED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(allAuthorsED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(yearED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(literalCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
}


void GuiBibitem::paramsToDialog(Inset const * inset)
{
	InsetCommand const * ic = static_cast<InsetCommand const *>(inset);
	InsetCommandParams const & params = ic->params();
	keyED->setText(toqstr(params["key"]));
	literalCB->setChecked(params["literal"] == "true");
	QString const label = toqstr(params["label"]);
	if (inset->buffer().masterParams().citeEngine() == "natbib") {
		yearED->setHidden(false);
		yearLA->setHidden(false);
		allAuthorsED->setHidden(false);
		allAuthorsLA->setHidden(false);
		labelLA->setText(qt_("Author &Names:"));
		labelLA->setMaximumWidth(labelLA->sizeHint().width());
		labelED->setToolTip(qt_("Insert the author name(s) for the author-year reference here. "
					"If you use an abbreviated list (with `et al.'), the full list can go below."));
		allAuthorsLA->setText(qt_("A&ll Author Names:"));
		allAuthorsLA->setMaximumWidth(allAuthorsLA->sizeHint().width());
		literalCB->setToolTip(qt_("Pass content of the `Author', `Year' and `All Authors' fields literally to LaTeX. "
					  "Check this if you want to enter LaTeX code."));
		int const i = label.lastIndexOf("(");
		int const j = label.lastIndexOf(")");
		if (i != -1 && j != -1 && i < j) {
			// Split Author(s) and Year
			// Natbib syntax is "Jones et al.(1990)Jones, Baker, and Williams"
			// (full list is optional)
			QString const year = label.left(j).mid(i + 1);
			QString const author = label.left(i);
			QString const allauthors = label.mid(j + 1);
			labelED->setText(author);
			yearED->setText(year);
			allAuthorsED->setText(allauthors);
		} else
			labelED->setText(label);
	} else {
		yearED->setHidden(true);
		yearLA->setHidden(true);
		allAuthorsED->setHidden(true);
		allAuthorsLA->setHidden(true);
		allAuthorsLA->setText(qt_("&Label:"));
		allAuthorsLA->setMaximumWidth(allAuthorsLA->sizeHint().width());
		labelLA->setText(qt_("&Label:"));
		labelLA->setMaximumWidth(labelLA->sizeHint().width());
		labelED->setToolTip(qt_("The label as it appears in the document"));
		labelED->setText(label);
		literalCB->setToolTip(qt_("Pass content of the `Label' field literally to LaTeX. "
					  "Check this if you want to enter LaTeX code."));
	}
}


docstring GuiBibitem::dialogToParams() const
{
	InsetCommandParams params(insetCode());
	QString label = labelED->text();
	if (!yearED->isHidden())
		label += "(" + yearED->text() + ")" + allAuthorsED->text();
	params["key"] = qstring_to_ucs4(keyED->text());
	params["label"] = qstring_to_ucs4(label);
	params["literal"] = literalCB->isChecked()
			? from_ascii("true") : from_ascii("false");
	return from_utf8(InsetCommand::params2string(params));
}


bool GuiBibitem::checkWidgets(bool readonly) const
{
	keyED->setReadOnly(readonly);
	labelED->setReadOnly(readonly);
	if (!InsetParamsWidget::checkWidgets())
		return false;
	return !keyED->text().isEmpty()
		&& (yearED->isHidden() || !yearED->text().isEmpty());
}

} // namespace frontend
} // namespace lyx

#include "moc_GuiBibitem.cpp"
