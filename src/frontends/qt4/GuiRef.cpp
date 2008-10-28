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

#include "Buffer.h"
#include "BufferList.h"
#include "FuncRequest.h"

#include "qt_helpers.h"

#include "insets/InsetRef.h"

#include "support/FileName.h"
#include "support/FileNameList.h"
#include "support/filetools.h" // makeAbsPath, makeDisplayPath

#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QToolTip>
#include <QCloseEvent>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

GuiRef::GuiRef(GuiView & lv)
	: GuiDialog(lv, "ref", qt_("Cross-reference")),
	  params_(insetCode("ref"))
{
	setupUi(this);

	sort_ = false;
	at_ref_ = false;

	//FIXME: when/if we support the xr package for cross-reference
	//between independant files. Those can be re-enabled.
	refsL->setEnabled(false);
	refsL->hide();
	bufferCO->setEnabled(false);
	bufferCO->hide();

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(reset_dialog()));
	connect(this, SIGNAL(rejected()), this, SLOT(dialog_rejected()));

	connect(typeCO, SIGNAL(activated(int)),
		this, SLOT(changed_adaptor()));
	connect(referenceED, SIGNAL(textChanged(QString)),
		this, SLOT(changed_adaptor()));
	connect(nameED, SIGNAL(textChanged(QString)),
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
	active_buffer_ = -1;
}


void GuiRef::changed_adaptor()
{
	changed();
}


void GuiRef::gotoClicked()
{
	gotoRef();
}


void GuiRef::selectionChanged()
{
	if (isBufferReadonly())
		return;

	QList<QListWidgetItem *> selections = refsLW->selectedItems();
	if (selections.isEmpty())
		return;
	QListWidgetItem * sel = selections.first();
	refHighlighted(sel);
	return;
}


void GuiRef::refHighlighted(QListWidgetItem * sel)
{
	if (isBufferReadonly())
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


void GuiRef::refSelected(QListWidgetItem * sel)
{
	if (isBufferReadonly())
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


void GuiRef::sortToggled(bool on)
{
	sort_ = on;
	redoRefs();
}


void GuiRef::updateClicked()
{
	updateRefs();
}


void GuiRef::dialog_rejected()
{
	reset_dialog();
	// We have to do this manually, instead of calling slotClose(), because
	// the dialog has already been made invisible before rejected() triggers.
	Dialog::disconnect();
}


void GuiRef::reset_dialog()
{
	at_ref_ = false;
	setGotoRef();
}


void GuiRef::closeEvent(QCloseEvent * e)
{
	slotClose();
	reset_dialog();
	e->accept();
}


void GuiRef::updateContents()
{
	int orig_type = typeCO->currentIndex();

	referenceED->setText(toqstr(params_["reference"]));

	nameED->setText(toqstr(params_["name"]));
	nameED->setReadOnly(!nameAllowed() && !isBufferReadonly());

	// restore type settings for new insets
	if (params_["reference"].empty())
		typeCO->setCurrentIndex(orig_type);
	else
		typeCO->setCurrentIndex(InsetRef::getType(params_.getCmdName()));
	typeCO->setEnabled(typeAllowed() && !isBufferReadonly());
	if (!typeAllowed())
		typeCO->setCurrentIndex(0);

	sortCB->setChecked(sort_);

	// insert buffer list
	bufferCO->clear();
	FileNameList const & buffers = theBufferList().fileNames();
	for (FileNameList::const_iterator it = buffers.begin();
	     it != buffers.end(); ++it) {
		bufferCO->addItem(toqstr(makeDisplayPath(it->absFilename())));
	}

	int thebuffer = theBufferList().bufferNum(buffer().fileName());
	// restore the buffer combo setting for new insets
	if (params_["reference"].empty() && restored_buffer_ != -1
	    && restored_buffer_ < bufferCO->count() && thebuffer == active_buffer_)
		bufferCO->setCurrentIndex(restored_buffer_);
	else {
		int num = theBufferList().bufferNum(buffer().fileName());
		bufferCO->setCurrentIndex(num);
		if (thebuffer != active_buffer_)
			restored_buffer_ = num;
	}
	active_buffer_ = thebuffer;

	updateRefs();
	bc().setValid(false);
}


void GuiRef::applyView()
{
	last_reference_ = referenceED->text();

	params_.setCmdName(InsetRef::getName(typeCO->currentIndex()));
	params_["reference"] = qstring_to_ucs4(last_reference_);
	params_["name"] = qstring_to_ucs4(nameED->text());

	restored_buffer_ = bufferCO->currentIndex();
}


bool GuiRef::nameAllowed()
{
	KernelDocType const doc_type = docType();
	return doc_type != LATEX && doc_type != LITERATE;
}


bool GuiRef::typeAllowed()
{
	return docType() != DOCBOOK;
}


void GuiRef::setGoBack()
{
	gotoPB->setText(qt_("&Go Back"));
	gotoPB->setToolTip("");
	gotoPB->setToolTip(qt_("Jump back"));
}


void GuiRef::setGotoRef()
{
	gotoPB->setText(qt_("&Go to Label"));
	gotoPB->setToolTip("");
	gotoPB->setToolTip(qt_("Jump to label"));
}


void GuiRef::gotoRef()
{
	string ref = fromqstr(referenceED->text());

	if (at_ref_) {
		// go back
		setGotoRef();
		gotoBookmark();
	} else {
		// go to the ref
		setGoBack();
		gotoRef(ref);
	}
	at_ref_ = !at_ref_;
}


void GuiRef::redoRefs()
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

	for (vector<docstring>::const_iterator iter = refs_.begin();
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
		last_reference_.clear();
		for (int i = 0; i != refsLW->count(); ++i) {
			QListWidgetItem * item = refsLW->item(i);
			if (textToFind == item->text()) {
				refsLW->setCurrentItem(item);
				refsLW->setItemSelected(item, !newInset);
				//Make sure selected item is visible
				refsLW->scrollToItem(item);
				last_reference_ = textToFind;
				break;
			}
		}
	}
	refsLW->setUpdatesEnabled(true);
	refsLW->update();

	// Re-activate the emission of signals by these widgets.
	refsLW->blockSignals(false);
	referenceED->blockSignals(false);
}


void GuiRef::updateRefs()
{
	refs_.clear();
	FileName const & name = theBufferList().fileNames()[bufferCO->currentIndex()];
	Buffer const * buf = theBufferList().getBuffer(name);
	buf->getLabelList(refs_);
	sortCB->setEnabled(!refs_.empty());
	refsLW->setEnabled(!refs_.empty());
	// refsLW should only be the focus proxy when it is enabled
	setFocusProxy(refs_.empty() ? 0 : refsLW);
	gotoPB->setEnabled(!refs_.empty());
	redoRefs();
}


bool GuiRef::isValid()
{
	return !referenceED->text().isEmpty();
}


void GuiRef::gotoRef(string const & ref)
{
	dispatch(FuncRequest(LFUN_BOOKMARK_SAVE, "0"));
	dispatch(FuncRequest(LFUN_LABEL_GOTO, ref));
}


void GuiRef::gotoBookmark()
{
	dispatch(FuncRequest(LFUN_BOOKMARK_GOTO, "0"));
}


bool GuiRef::initialiseParams(std::string const & data)
{
	InsetCommand::string2params("ref", data, params_);
	return true;
}


void GuiRef::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string("ref", params_);
	dispatch(FuncRequest(getLfun(), lfun));
}



Dialog * createGuiRef(GuiView & lv) { return new GuiRef(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiRef_moc.cpp"
