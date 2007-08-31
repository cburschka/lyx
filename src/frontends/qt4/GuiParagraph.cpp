/**
 * \file GuiParagraph.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiParagraph.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "debug.h"
#include "ParagraphParameters.h"
#include "Spacing.h"

#include "controllers/ControlParagraph.h"
#include "controllers/frontend_helpers.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

using std::string;
using std::endl;

/////////////////////////////////////////////////////////////////////
//
// GuiParagraphDialog
//
/////////////////////////////////////////////////////////////////////


#include <boost/current_function.hpp>

namespace lyx {
namespace frontend {

GuiParagraphDialog::GuiParagraphDialog(GuiParagraph * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), form_, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), form_, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()), form_, SLOT(slotRestore()));
	connect(alignDefaultRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(alignJustRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(alignLeftRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(alignRightRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(alignCenterRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(linespacing, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(linespacing, SIGNAL(activated(int)),
		this, SLOT(enableLinespacingValue(int)));
	connect(linespacingValue, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(indentCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(labelWidth, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));

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
	
	labelMap[LYX_ALIGN_LAYOUT] = _("Use Paragraph's Default Alignment");
	labelMap[LYX_ALIGN_BLOCK]  = _("Justified");
	labelMap[LYX_ALIGN_LEFT]   = _("Left");
	labelMap[LYX_ALIGN_RIGHT]  = _("Right");
	labelMap[LYX_ALIGN_CENTER] = _("Center");
}


void GuiParagraphDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiParagraphDialog::change_adaptor()
{
	form_->changed();
}


void GuiParagraphDialog::enableLinespacingValue(int)
{
	bool const enable = linespacing->currentIndex() == 4;
	linespacingValue->setEnabled(enable);
}


void GuiParagraphDialog::checkAlignmentRadioButtons() {
	LyXAlignment const alignPossible = form_->controller().alignPossible();

	QPRadioMap::iterator it = radioMap.begin();
	for (; it != radioMap.end(); ++it) {
		LyXAlignment const align = it->first;
		it->second->setEnabled(align & alignPossible);
	}
	docstring label = labelMap[LYX_ALIGN_LAYOUT];
	if (!form_->controller().haveMulitParSelection())
		label += (" (" + labelMap[form_->controller().alignDefault()] + ")");
	alignDefaultRB->setText(toqstr(label));
}


void GuiParagraphDialog::alignmentToRadioButtons(LyXAlignment align)
{
	QPRadioMap::const_iterator it = radioMap.begin();
	for (;it != radioMap.end(); ++it) {
		if (align == it->first) {
			it->second->blockSignals(true);
			it->second->setChecked(true);
			it->second->blockSignals(false);
			return;
		}
	}

	lyxerr << BOOST_CURRENT_FUNCTION << "Unknown alignment "
		<< align << std::endl;
}


LyXAlignment GuiParagraphDialog::getAlignmentFromDialog()
{
	LyXAlignment alignment = LYX_ALIGN_NONE;
	QPRadioMap::const_iterator it = radioMap.begin();
	for (; it != radioMap.end(); ++it) {
		if (it->second->isChecked()) {
			alignment = it->first;
			break;
		}
	}
	return alignment;
}


/////////////////////////////////////////////////////////////////////
//
// GuiParagraph
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlParagraph, GuiView<GuiParagraphDialog> >
	ParagraphBase;


GuiParagraph::GuiParagraph(Dialog & parent)
	: ParagraphBase(parent, _("Paragraph Settings"))
{}


void GuiParagraph::build_dialog()
{
	// the dialog
	dialog_.reset(new GuiParagraphDialog(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().setRestore(dialog_->restorePB);
}


void GuiParagraph::apply()
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


void GuiParagraph::update_contents()
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
	dialog_->checkAlignmentRadioButtons();
	dialog_->alignmentToRadioButtons(params.align());

	//indentation
	bool const canindent = controller().canIndent();
	dialog_->indentCB->setEnabled(canindent);
	dialog_->indentCB->setChecked(canindent && !params.noindent());

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

#include "GuiParagraph_moc.cpp"
