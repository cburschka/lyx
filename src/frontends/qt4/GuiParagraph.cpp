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

#include "ControlParagraph.h"

#include "debug.h"
#include "frontend_helpers.h"
#include "ParagraphParameters.h"
#include "qt_helpers.h"
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

GuiParagraphDialog::GuiParagraphDialog(LyXView & lv)
	: GuiDialog(lv, "paragraph")
{
	setupUi(this);
	setViewTitle(_("Paragraph Settings"));
	setController(new ControlParagraph(*this));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));
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

	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	bc().setRestore(restorePB);
}


ControlParagraph & GuiParagraphDialog::controller()
{
	return static_cast<ControlParagraph &>(GuiDialog::controller());
}


void GuiParagraphDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiParagraphDialog::change_adaptor()
{
	changed();
}


void GuiParagraphDialog::enableLinespacingValue(int)
{
	bool const enable = linespacing->currentIndex() == 4;
	linespacingValue->setEnabled(enable);
}


void GuiParagraphDialog::checkAlignmentRadioButtons() {
	LyXAlignment const alignPossible = controller().alignPossible();

	RadioMap::iterator it = radioMap.begin();
	for (; it != radioMap.end(); ++it) {
		LyXAlignment const align = it->first;
		it->second->setEnabled(align & alignPossible);
	}
	docstring label = labelMap[LYX_ALIGN_LAYOUT];
	if (!controller().haveMulitParSelection())
		label += (" (" + labelMap[controller().alignDefault()] + ")");
	alignDefaultRB->setText(toqstr(label));
}


void GuiParagraphDialog::alignmentToRadioButtons(LyXAlignment align)
{
	RadioMap::const_iterator it = radioMap.begin();
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
	RadioMap::const_iterator it = radioMap.begin();
	for (; it != radioMap.end(); ++it) {
		if (it->second->isChecked()) {
			alignment = it->first;
			break;
		}
	}
	return alignment;
}


void GuiParagraphDialog::applyView()
{
	// FIXME: We should not access params() at all!
	// we should just pass the relevant GUI information
	// and let the controller do the rest.
	// With this architecture, only current parent paragraph will be
	// modified when we have a multi-paragraph selection.
	ParagraphParameters & params = controller().params();

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
}


void GuiParagraphDialog::updateContents()
{
	ParagraphParameters const & params = controller().params();

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
	bool const canindent = controller().canIndent();
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
