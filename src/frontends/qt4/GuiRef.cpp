/**
 * \file GuiRef.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiRef.h"

#include "ControlRef.h"
#include "qt_helpers.h"

#include "insets/InsetRef.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QToolTip>
#include <QCloseEvent>


using std::vector;
using std::string;


namespace lyx {
namespace frontend {

GuiRefDialog::GuiRefDialog(LyXView & lv)
	: GuiDialog(lv, "ref")
{
	setupUi(this);
	setController(new ControlRef(*this));
	setViewTitle(_("Cross-reference"));

	sort_ = false;
	at_ref_ = false;

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(reset_dialog()));
	connect(this, SIGNAL(rejected()), this, SLOT(reset_dialog()));

	connect(typeCO, SIGNAL(activated(int)),
		this, SLOT(changed_adaptor()));
	connect(referenceED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));
	connect(nameED, SIGNAL(textChanged(const QString &)),
		this, SLOT(changed_adaptor()));
	connect(refsLW, SIGNAL(itemClicked(QListWidgetItem *)),
		this, SLOT(refHighlighted(QListWidgetItem *)));
	connect(refsLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(selectionChanged()));
	connect(refsLW, SIGNAL(itemActivated(QListWidgetItem *)),
		this, SLOT(refSelected(QListWidgetItem *)));
	connect(sortCB, SIGNAL(clicked(bool)),
		this, SLOT(sortToggled(bool)));
	connect(gotoPB, SIGNAL(clicked()),
		this, SLOT(gotoClicked()));
	connect(updatePB, SIGNAL(clicked()),
		this, SLOT(updateClicked()));
	connect(bufferCO, SIGNAL(activated(int)),
		this, SLOT(updateClicked()));

	setFocusProxy(refsLW);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	bc().addReadOnly(refsLW);
	bc().addReadOnly(sortCB);
	bc().addReadOnly(nameED);
	bc().addReadOnly(referenceED);
	bc().addReadOnly(typeCO);
	bc().addReadOnly(bufferCO);

	restored_buffer_ = -1;
}


ControlRef & GuiRefDialog::controller() const
{
	return static_cast<ControlRef &>(GuiDialog::controller());
}


void GuiRefDialog::changed_adaptor()
{
	changed();
}


void GuiRefDialog::gotoClicked()
{
	gotoRef();
}

void GuiRefDialog::selectionChanged()
{
	if (readOnly())
		return;

	QList<QListWidgetItem *> selections = refsLW->selectedItems();
	if (selections.isEmpty())
		return;
	QListWidgetItem * sel = selections.first();
	refHighlighted(sel);
	return;
}


void GuiRefDialog::refHighlighted(QListWidgetItem * sel)
{
	if (readOnly())
		return;

/*	int const cur_item = refsLW->currentRow();
	bool const cur_item_selected = cur_item >= 0 ?
		refsLB->isSelected(cur_item) : false;*/
	bool const cur_item_selected = refsLW->isItemSelected(sel);

	if (cur_item_selected)
		referenceED->setText(sel->text());

	if (at_ref_)
		gotoRef();
	gotoPB->setEnabled(true);
	if (typeAllowed())
		typeCO->setEnabled(true);
	if (nameAllowed())
		nameED->setEnabled(true);
}


void GuiRefDialog::refSelected(QListWidgetItem * sel)
{
	if (readOnly())
		return;

/*	int const cur_item = refsLW->currentRow();
	bool const cur_item_selected = cur_item >= 0 ?
		refsLB->isSelected(cur_item) : false;*/
	bool const cur_item_selected = refsLW->isItemSelected(sel);

	if (cur_item_selected)
		referenceED->setText(sel->text());
	// <enter> or double click, inserts ref and closes dialog
	slotOK();
}


void GuiRefDialog::sortToggled(bool on)
{
	sort_ = on;
	redoRefs();
}


void GuiRefDialog::updateClicked()
{
	updateRefs();
}


void GuiRefDialog::reset_dialog()
{
	at_ref_ = false;
	setGotoRef();
}


void GuiRefDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	reset_dialog();
	e->accept();
}


void GuiRefDialog::update_contents()
{
	InsetCommandParams const & params = controller().params();

	int orig_type = typeCO->currentIndex();

	referenceED->setText(toqstr(params["reference"]));

	nameED->setText(toqstr(params["name"]));
	nameED->setReadOnly(!nameAllowed() && !readOnly());

	// restore type settings for new insets
	if (params["reference"].empty())
		typeCO->setCurrentIndex(orig_type);
	else
		typeCO->setCurrentIndex(InsetRef::getType(params.getCmdName()));
	typeCO->setEnabled(typeAllowed() && !readOnly());
	if (!typeAllowed())
		typeCO->setCurrentIndex(0);

	sortCB->setChecked(sort_);

	// insert buffer list
	bufferCO->clear();
	vector<string> const buffers = controller().getBufferList();
	for (vector<string>::const_iterator it = buffers.begin();
		it != buffers.end(); ++it) {
		bufferCO->addItem(toqstr(*it));
	}
	// restore the buffer combo setting for new insets
	if (params["reference"].empty() && restored_buffer_ != -1
	&& restored_buffer_ < bufferCO->count())
		bufferCO->setCurrentIndex(restored_buffer_);
	else
		bufferCO->setCurrentIndex(controller().getBufferNum());

	updateRefs();
	bc().setValid(false);
}


void GuiRefDialog::applyView()
{
	InsetCommandParams & params = controller().params();

	last_reference_ = referenceED->text();

	params.setCmdName(InsetRef::getName(typeCO->currentIndex()));
	params["reference"] = qstring_to_ucs4(last_reference_);
	params["name"] = qstring_to_ucs4(nameED->text());

	restored_buffer_ = bufferCO->currentIndex();
}


bool GuiRefDialog::nameAllowed()
{
	KernelDocType const doc_type = controller().docType();
	return doc_type != LATEX && doc_type != LITERATE;
}


bool GuiRefDialog::typeAllowed()
{
	return controller().docType() != DOCBOOK;
}


void GuiRefDialog::setGoBack()
{
	gotoPB->setText(qt_("&Go Back"));
	gotoPB->setToolTip("");
	gotoPB->setToolTip(qt_("Jump back"));
}


void GuiRefDialog::setGotoRef()
{
	gotoPB->setText(qt_("&Go to Label"));
	gotoPB->setToolTip("");
	gotoPB->setToolTip(qt_("Jump to label"));
}


void GuiRefDialog::gotoRef()
{
	string ref = fromqstr(referenceED->text());

	if (at_ref_) {
		// go back
		setGotoRef();
		controller().gotoBookmark();
	} else {
		// go to the ref
		setGoBack();
		controller().gotoRef(ref);
	}
	at_ref_ = !at_ref_;
}


void GuiRefDialog::redoRefs()
{
	// Prevent these widgets from emitting any signals whilst
	// we modify their state.
	refsLW->blockSignals(true);
	referenceED->blockSignals(true);
	refsLW->setUpdatesEnabled(false);

	refsLW->clear();

	// need this because Qt will send a highlight() here for
	// the first item inserted
	QString const oldSelection(referenceED->text());

	for (std::vector<docstring>::const_iterator iter = refs_.begin();
		iter != refs_.end(); ++iter) {
		refsLW->addItem(toqstr(*iter));
	}

	if (sort_)
		refsLW->sortItems();

	referenceED->setText(oldSelection);

	// restore the last selection or, for new insets, highlight
	// the previous selection
	if (!oldSelection.isEmpty() || !last_reference_.isEmpty()) {
		bool const newInset = oldSelection.isEmpty();
		QString textToFind = newInset ? last_reference_ : oldSelection;
		bool foundItem = false;
		for (int i = 0; !foundItem && i < refsLW->count(); ++i) {
			QListWidgetItem * item = refsLW->item(i);
			if (textToFind == item->text()) {
				refsLW->setCurrentItem(item);
				refsLW->setItemSelected(item, !newInset);
				//Make sure selected item is visible
				refsLW->scrollToItem(item);
				foundItem = true;
			}
		}
		if (foundItem)
			last_reference_ = textToFind;
		else last_reference_ = "";
	}
	refsLW->setUpdatesEnabled(true);
	refsLW->update();

	// Re-activate the emission of signals by these widgets.
	refsLW->blockSignals(false);
	referenceED->blockSignals(false);
}


void GuiRefDialog::updateRefs()
{
	refs_.clear();
	string const name = controller().getBufferName(bufferCO->currentIndex());
	refs_ = controller().getLabelList(name);
	sortCB->setEnabled(!refs_.empty());
	refsLW->setEnabled(!refs_.empty());
	gotoPB->setEnabled(!refs_.empty());
	redoRefs();
}


bool GuiRefDialog::isValid()
{
	return !referenceED->text().isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "GuiRef_moc.cpp"
