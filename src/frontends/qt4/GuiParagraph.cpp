/**
 * \file GuiParagraph.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Richard Heck
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiParagraph.h"
#include "qt_helpers.h"

#include "ControlParagraph.h"

#include "debug.h"
#include "frontend_helpers.h"
#include "gettext.h"
#include "ParagraphParameters.h"
#include "Spacing.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>
#include <QPushButton>

#include <boost/current_function.hpp>

using std::string;
using std::endl;

namespace lyx {
namespace frontend {

GuiParagraph::GuiParagraph(ControlParagraph & controller)
	: controller_(controller)
{
	setupUi(this);
	setWindowTitle(qt_("Paragraph Settings"));

	connect(alignDefaultRB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(alignJustRB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(alignLeftRB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(alignRightRB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(alignCenterRB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(linespacing, SIGNAL(activated(int)), this, SLOT(changed()));
	connect(linespacingValue, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed()));
	connect(indentCB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(labelWidth, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed()));


	synchronizedViewCB->setChecked(true);
	on_synchronizedViewCB_toggled();
	linespacingValue->setValidator(new QDoubleValidator(linespacingValue));

	labelWidth->setWhatsThis( qt_(
		"As described in the User Guide, the width of"
		" this text determines the width of the label part"
		" of each item in environments like List and"
		" Description.\n"
		"\n"
		" Normally, you won't need to set this,"
		" since the largest label width of all the"
		" items is used."
	));

	radioMap[LYX_ALIGN_LAYOUT] = alignDefaultRB;
	radioMap[LYX_ALIGN_BLOCK]  = alignJustRB;
	radioMap[LYX_ALIGN_LEFT]   = alignLeftRB;
	radioMap[LYX_ALIGN_RIGHT]  = alignRightRB;
	radioMap[LYX_ALIGN_CENTER] = alignCenterRB;

	const_cast<QString &>(alignDefaultLabel) = alignDefaultRB->text();
}


void GuiParagraph::on_linespacing_activated(int index)
{
	linespacingValue->setEnabled(index == 4);
}


void GuiParagraph::checkAlignmentRadioButtons() {
	LyXAlignment const alignPossible = controller_.alignPossible();

	RadioMap::iterator it = radioMap.begin();
	for (; it != radioMap.end(); ++it) {
		LyXAlignment const align = it->first;
		it->second->setEnabled(align & alignPossible);
	}
	if (controller_.haveMulitParSelection())
		alignDefaultRB->setText(alignDefaultLabel);
	else
		alignDefaultRB->setText(alignDefaultLabel + " (" 
			+ radioMap[controller_.alignDefault()]->text() + ")");
}


void GuiParagraph::alignmentToRadioButtons(LyXAlignment align)
{
	RadioMap::const_iterator it = radioMap.begin();
	for (;it != radioMap.end(); ++it) {
		if (!it->second->isEnabled())
			continue;
		it->second->blockSignals(true);
		it->second->setChecked(align == it->first);
		it->second->blockSignals(false);
	}
}


LyXAlignment GuiParagraph::getAlignmentFromDialog()
{
	LyXAlignment alignment = LYX_ALIGN_NONE;
	RadioMap::const_iterator it = radioMap.begin();
	for (; it != radioMap.end(); ++it) {
		if (it->second->isChecked()) {
			alignment = it->first;
			break;
		}
	}
	return alignment;
}


void GuiParagraph::on_synchronizedViewCB_toggled()
{
	bool in_sync = synchronizedViewCB->isChecked();
	restorePB->setEnabled(!in_sync);
	applyPB->setEnabled(!in_sync);
}


void GuiParagraph::changed()
{
	if (synchronizedViewCB->isChecked())
		on_applyPB_clicked();
}


void GuiParagraph::on_applyPB_clicked()
{
	ParagraphParameters & params = controller_.params();

	params.align(getAlignmentFromDialog());

	// get spacing
	Spacing::Space ls = Spacing::Default;
	string other;
	switch (linespacing->currentIndex()) {
	case 0:
		ls = Spacing::Default;
		break;
	case 1:
		ls = Spacing::Single;
		break;
	case 2:
		ls = Spacing::Onehalf;
		break;
	case 3:
		ls = Spacing::Double;
		break;
	case 4:
		ls = Spacing::Other;
		other = fromqstr(linespacingValue->text());
		break;
	}

	Spacing const spacing(ls, other);
	params.spacing(spacing);

	// label width
	params.labelWidthString(qstring_to_ucs4(labelWidth->text()));
	// indendation
	params.noindent(!indentCB->isChecked());

	controller_.dispatchParams();
}


void GuiParagraph::on_restorePB_clicked()
{
	updateView();
}


void GuiParagraph::updateView()
{
	on_synchronizedViewCB_toggled();

	ParagraphParameters const & params = controller_.params();

	// label width
	docstring const & labelwidth = params.labelWidthString();
	// FIXME We should not compare translated strings
	if (labelwidth != _("Senseless with this layout!")) {
		labelwidthGB->setEnabled(true);
		labelWidth->setText(toqstr(labelwidth));
	} else {
		labelwidthGB->setEnabled(false);
		labelWidth->setText("");
	}

	// alignment
	checkAlignmentRadioButtons();
	alignmentToRadioButtons(params.align());

	//indentation
	bool const canindent = controller_.canIndent();
	indentCB->setEnabled(canindent);
	indentCB->setChecked(canindent && !params.noindent());

	// linespacing
	int ls;
	Spacing const & space = params.spacing();
	switch (space.getSpace()) {
	case Spacing::Single:
		ls = 1;
		break;
	case Spacing::Onehalf:
		ls = 2;
		break;
	case Spacing::Double:
		ls = 3;
		break;
	case Spacing::Other:
		ls = 4;
		break;
	default:
		ls = 0;
		break;
	}
	linespacing->setCurrentIndex(ls);
	if (space.getSpace() == Spacing::Other) {
		linespacingValue->setText(toqstr(space.getValueAsString()));
		linespacingValue->setEnabled(true);
	} else {
		linespacingValue->setText(QString());
		linespacingValue->setEnabled(false);
	}
}

} // namespace frontend
} // namespace lyx

#include "GuiParagraph_moc.cpp"
