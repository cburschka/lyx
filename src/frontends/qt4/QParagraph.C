/**
 * \file QParagraph.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QParagraph.h"
#include "QParagraphDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "debug.h"
#include "ParagraphParameters.h"
#include "Spacing.h"
#include "layout.h"

#include "controllers/ControlParagraph.h"
#include "controllers/helper_funcs.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>


using std::string;
using std::endl;

namespace lyx {
namespace frontend {

typedef QController<ControlParagraph, QView<QParagraphDialog> > paragraph_base_class;


QParagraph::QParagraph(Dialog & parent)
	: paragraph_base_class(parent, _("Paragraph Settings"))
{}


void QParagraph::build_dialog()
{
	// the dialog
	dialog_.reset(new QParagraphDialog(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().setRestore(dialog_->restorePB);
}


void QParagraph::apply()
{
	ParagraphParameters & params = controller().params();

	params.align(dialog_->getAlignmentFromDialog());

	// get spacing
	Spacing::Space linespacing = Spacing::Default;
	string other;
	switch (dialog_->linespacing->currentIndex()) {
	case 0:
		linespacing = Spacing::Default;
		break;
	case 1:
		linespacing = Spacing::Single;
		break;
	case 2:
		linespacing = Spacing::Onehalf;
		break;
	case 3:
		linespacing = Spacing::Double;
		break;
	case 4:
		linespacing = Spacing::Other;
		other = fromqstr(dialog_->linespacingValue->text());
		break;
	}

	Spacing const spacing(linespacing, other);
	params.spacing(spacing);

	// label width
	params.labelWidthString(qstring_to_ucs4(dialog_->labelWidth->text()));
	// indendation
	params.noindent(!dialog_->indentCB->isChecked());
}


void QParagraph::update_contents()
{
	ParagraphParameters const & params = controller().params();

	// label width
	docstring const & labelwidth = params.labelWidthString();
	// FIXME We should not compare translated strings
	if (labelwidth != _("Senseless with this layout!")) {
		dialog_->labelwidthGB->setEnabled(true);
		dialog_->labelWidth->setText(toqstr(labelwidth));
	} else {
		dialog_->labelwidthGB->setEnabled(false);
		dialog_->labelWidth->setText("");
	}

	// alignment
	LyXAlignment newAlignment = params.align();
	LyXAlignment defaultAlignment = controller().alignDefault();
	bool alignmentIsDefault = 
		newAlignment == LYX_ALIGN_LAYOUT || newAlignment == defaultAlignment;
	dialog_->alignDefaultCB->blockSignals(true);
	dialog_->alignDefaultCB->setChecked(alignmentIsDefault);
	dialog_->alignDefaultCB->blockSignals(false);
	dialog_->checkAlignmentRadioButtons();
	dialog_->alignmentToRadioButtons(newAlignment);

	//indentation
	dialog_->indentCB->setChecked(!params.noindent());

	// linespacing
	int linespacing;
	Spacing const & space = params.spacing();
	switch (space.getSpace()) {
	case Spacing::Single:
		linespacing = 1;
		break;
	case Spacing::Onehalf:
		linespacing = 2;
		break;
	case Spacing::Double:
		linespacing = 3;
		break;
	case Spacing::Other:
		linespacing = 4;
		break;
	default:
		linespacing = 0;
		break;
	}
	dialog_->linespacing->setCurrentIndex(linespacing);
	if (space.getSpace() == Spacing::Other) {
		dialog_->linespacingValue->setText(toqstr(space.getValueAsString()));
		dialog_->linespacingValue->setEnabled(true);
	} else {
		dialog_->linespacingValue->setText("");
		dialog_->linespacingValue->setEnabled(false);
	}
}

} // namespace frontend
} // namespace lyx
