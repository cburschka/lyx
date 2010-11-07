/**
 * \file GuiHyperlink.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiHyperlink.h"

#include "qt_helpers.h"

#include "insets/InsetHyperlink.h"

#if defined(LYX_MERGE_FILES) && !defined(Q_CC_MSVC)
// GCC couldn't find operator==
namespace lyx {
	bool operator==(lyx::docstring const & d, char const * c);
	namespace frontend {
		bool operator==(lyx::docstring const & d, char const * c)
		  { return lyx::operator ==(d, c); }
	}
}
#endif


namespace lyx {
namespace frontend {

GuiHyperlink::GuiHyperlink(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	connect(targetED, SIGNAL(textChanged(const QString &)),
		this, SIGNAL(changed()));
	connect(nameED, SIGNAL(textChanged(const QString &)),
		this, SIGNAL(changed()));
	connect(webRB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(emailRB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(fileRB, SIGNAL(clicked()),
		this, SIGNAL(changed()));

	setFocusProxy(targetED);
}


void GuiHyperlink::paramsToDialog(Inset const * inset)
{
	InsetHyperlink const * hlink = static_cast<InsetHyperlink const *>(inset);
	InsetCommandParams const & params = hlink->params();

	targetED->setText(toqstr(params["target"]));
	nameED->setText(toqstr(params["name"]));
	docstring const & type = params["type"];
	if (type.empty())
		webRB->setChecked(true);
	else if (type == "mailto:")
		emailRB->setChecked(true);
	else if (type == "file:")
		fileRB->setChecked(true);
}


docstring GuiHyperlink::dialogToParams() const
{
	InsetCommandParams params(insetCode());

	params["target"] = qstring_to_ucs4(targetED->text());
	params["name"] = qstring_to_ucs4(nameED->text());
	if (webRB->isChecked())
		params["type"] = qstring_to_ucs4("");
	else if (emailRB->isChecked())
		params["type"] = qstring_to_ucs4("mailto:");
	else if (fileRB->isChecked())
		params["type"] = qstring_to_ucs4("file:");
	params.setCmdName("href");
	return from_utf8(InsetHyperlink::params2string(params));
}


bool GuiHyperlink::checkWidgets() const
{
	if (!InsetParamsWidget::checkWidgets())
		return false;
	return !targetED->text().isEmpty() || !nameED->text().isEmpty();
}


} // namespace frontend
} // namespace lyx


#include "moc_GuiHyperlink.cpp"
