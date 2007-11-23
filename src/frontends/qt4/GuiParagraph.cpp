/**
 * \file GuiParagraph.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Richard Heck
 * \author Abdelrazak Younes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiParagraph.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "debug.h"
//#include "DialogView.h"
//#include "DockView.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "GuiView.h"
#include "Lexer.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "qt_helpers.h"
#include "Spacing.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>
#include <QPushButton>

#include <boost/current_function.hpp>

#include <sstream>

using std::istringstream;
using std::ostringstream;
using std::string;
using std::endl;

namespace lyx {
namespace frontend {

GuiParagraph::GuiParagraph(GuiView & lv)
	: Dialog(lv, "paragraph")
{
	setupUi(this);
	setWindowTitle(qt_("Paragraph Settings"));

	//setModal(modal);
	QGridLayout * gridLayout = new QGridLayout(this);
	gridLayout->setMargin(0);
	gridLayout->addWidget(this);

	connect(alignDefaultRB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(alignJustRB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(alignLeftRB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(alignRightRB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(alignCenterRB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(linespacing, SIGNAL(activated(int)), this, SLOT(changed()));
	connect(linespacingValue, SIGNAL(textChanged(QString)),
		this, SLOT(changed()));
	connect(indentCB, SIGNAL(clicked()), this, SLOT(changed()));
	connect(labelWidth, SIGNAL(textChanged(QString)),
		this, SLOT(changed()));


	synchronizedViewCB->setChecked(false);
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

	labelMap[LYX_ALIGN_BLOCK]  = _("Justified");
	labelMap[LYX_ALIGN_LEFT]   = _("Left");
	labelMap[LYX_ALIGN_RIGHT]  = _("Right");
	labelMap[LYX_ALIGN_CENTER] = _("Center");

	const_cast<QString &>(alignDefaultLabel) = alignDefaultRB->text();
}


void GuiParagraph::on_linespacing_activated(int index)
{
	linespacingValue->setEnabled(index == 4);
}


void GuiParagraph::checkAlignmentRadioButtons()
{
	RadioMap::iterator it = radioMap.begin();
	for (; it != radioMap.end(); ++it) {
		LyXAlignment const align = it->first;
		it->second->setEnabled(align & alignPossible());
	}
	if (haveMulitParSelection())
		alignDefaultRB->setText(alignDefaultLabel);
	else
		alignDefaultRB->setText(alignDefaultLabel + " ("
			+ toqstr(labelMap[alignDefault()]) + ")");
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
	ParagraphParameters & pp = params();

	pp.align(getAlignmentFromDialog());

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
	pp.spacing(spacing);

	// label width
	pp.labelWidthString(qstring_to_ucs4(labelWidth->text()));
	// indendation
	pp.noindent(!indentCB->isChecked());

	dispatchParams();
}


void GuiParagraph::on_restorePB_clicked()
{
	updateView();
}


void GuiParagraph::updateView()
{
	on_synchronizedViewCB_toggled();

	ParagraphParameters const & pp = params();

	// label width
	docstring const & labelwidth = pp.labelWidthString();
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
	alignmentToRadioButtons(pp.align());

	//indentation
	bool const canindent = canIndent();
	indentCB->setEnabled(canindent);
	indentCB->setChecked(canindent && !pp.noindent());

	// linespacing
	int ls;
	Spacing const & space = pp.spacing();
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


ParagraphParameters & GuiParagraph::params()
{
	if (haveMulitParSelection()) {
		multiparsel_ = ParagraphParameters();
		// FIXME: It would be nice to initialise the parameters that
		// are common to all paragraphs.
		return multiparsel_;
	}

	return bufferview()->cursor().innerParagraph().params();
}


ParagraphParameters const & GuiParagraph::params() const
{
	return bufferview()->cursor().innerParagraph().params();
}


void GuiParagraph::dispatchParams()
{
	if (haveMulitParSelection()) {
		ostringstream data;
		multiparsel_.write(data);
		FuncRequest const fr(LFUN_PARAGRAPH_PARAMS_APPLY, data.str());
		dispatch(fr);
		return;
	}

	bufferview()->updateMetrics();
	bufferview()->buffer().changed();
}


bool GuiParagraph::haveMulitParSelection()
{
	Cursor cur = bufferview()->cursor();
	return cur.selection() && cur.selBegin().pit() != cur.selEnd().pit();
}

	
bool GuiParagraph::canIndent() const
{
	return buffer().params().paragraph_separation ==
		BufferParams::PARSEP_INDENT;
}


LyXAlignment GuiParagraph::alignPossible() const
{
	return bufferview()->cursor().innerParagraph().layout()->alignpossible;
}


LyXAlignment GuiParagraph::alignDefault() const
{
	return bufferview()->cursor().innerParagraph().layout()->align;
}


Dialog * createGuiParagraph(GuiView & lv)
{
#if 0
	GuiView & guiview = static_cast<GuiView &>(lv);
#ifdef USE_DOCK_WIDGET
	return new DockView<ControlParagraph, GuiParagraph>(guiview, "paragraph",
		Qt::TopDockWidgetArea);
#else
	return new DialogView<ControlParagraph, GuiParagraph>(guiview, "paragraph");
#endif
#endif

	return new GuiParagraph(lv);
}


} // namespace frontend
} // namespace lyx

#include "GuiParagraph_moc.cpp"
