/**
 * \file QParagraph.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QParagraph.h"
#include "QParagraphDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "ParagraphParameters.h"
#include "Spacing.h"

#include "controllers/ControlParagraph.h"
#include "controllers/helper_funcs.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>


using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlParagraph, QView<QParagraphDialog> > base_class;


QParagraph::QParagraph(Dialog & parent)
	: base_class(parent, _("Paragraph Settings"))
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

	// alignment
	LyXAlignment align;
	switch (dialog_->align->currentItem()) {
	case 0:
		align = LYX_ALIGN_BLOCK;
		break;
	case 1:
		align = LYX_ALIGN_LEFT;
		break;
	case 2:
		align = LYX_ALIGN_RIGHT;
		break;
	case 3:
		align = LYX_ALIGN_CENTER;
		break;
	default:
		align = LYX_ALIGN_BLOCK;
	}
	params.align(align);

	// get spacing
	Spacing::Space linespacing = Spacing::Default;
	string other;
	switch (dialog_->linespacing->currentItem()) {
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
	params.labelWidthString(fromqstr(dialog_->labelWidth->text()));
	// indendation
	params.noindent(!dialog_->indentCB->isChecked());
}


void QParagraph::update_contents()
{
	ParagraphParameters const & params = controller().params();

	// label width
	string const & labelwidth = params.labelWidthString();
	// _() is correct here (this is stupid though !)
	if (labelwidth != _("Senseless with this layout!")) {
		dialog_->labelwidthGB->setEnabled(true);
		dialog_->labelWidth->setText(toqstr(labelwidth));
	} else {
		dialog_->labelwidthGB->setEnabled(false);
		dialog_->labelWidth->setText("");
	}

	// alignment
	int i;
	switch (params.align()) {
	case LYX_ALIGN_LEFT:
		i = 1;
		break;
	case LYX_ALIGN_RIGHT:
		i = 2;
		break;
	case LYX_ALIGN_CENTER:
		i = 3;
		break;
	default:
		i = 0;
		break;
	}
	dialog_->align->setCurrentItem(i);


	//LyXAlignment alignpos = controller().alignPossible();

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
	dialog_->linespacing->setCurrentItem(linespacing);
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
