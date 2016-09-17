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

#include "GuiApplication.h"

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
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QToolTip>
#include <QCloseEvent>
#include <QHeaderView>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

GuiRef::GuiRef(GuiView & lv)
	: GuiDialog(lv, "ref", qt_("Cross-reference")),
	  params_(insetCode("ref"))
{
	setupUi(this);

	at_ref_ = false;

	// The filter bar
	filter_ = new FancyLineEdit(this);
#if QT_VERSION >= 0x040600
	filter_->setButtonPixmap(FancyLineEdit::Right, getPixmap("images/", "editclear", "svgz,png"));
	filter_->setButtonVisible(FancyLineEdit::Right, true);
	filter_->setButtonToolTip(FancyLineEdit::Right, qt_("Clear text"));
	filter_->setAutoHideButton(FancyLineEdit::Right, true);
#endif
#if QT_VERSION >= 0x040700
	filter_->setPlaceholderText(qt_("All avail. Labels"));
#endif
	filter_->setToolTip(qt_("Enter string to filter the list of available labels"));

	filterBarL->addWidget(filter_, 0);
	findKeysLA->setBuddy(filter_);

	sortingCO->addItem(qt_("By Occurrence"), "unsorted");
	sortingCO->addItem(qt_("Alphabetically (Case-Insensitive)"), "nocase");
	sortingCO->addItem(qt_("Alphabetically (Case-Sensitive)"), "case");

	refsTW->setColumnCount(1);
	refsTW->header()->setVisible(false);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(resetDialog()));
	connect(this, SIGNAL(rejected()), this, SLOT(dialogRejected()));

	connect(typeCO, SIGNAL(activated(int)),
		this, SLOT(changed_adaptor()));
	connect(referenceED, SIGNAL(textChanged(QString)),
		this, SLOT(refTextChanged(QString)));
	connect(referenceED, SIGNAL(textChanged(QString)),
		this, SLOT(changed_adaptor()));
	connect(filter_, SIGNAL(textEdited(QString)),
		this, SLOT(filterLabels()));
	connect(csFindCB, SIGNAL(clicked()),
		this, SLOT(filterLabels()));
	connect(nameED, SIGNAL(textChanged(QString)),
		this, SLOT(changed_adaptor()));
	connect(refsTW, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
		this, SLOT(refHighlighted(QTreeWidgetItem *)));
	connect(refsTW, SIGNAL(itemSelectionChanged()),
		this, SLOT(selectionChanged()));
	connect(refsTW, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		this, SLOT(refSelected(QTreeWidgetItem *)));
	connect(sortingCO, SIGNAL(activated(int)),
		this, SLOT(sortToggled()));
	connect(groupCB, SIGNAL(clicked()),
		this, SLOT(groupToggled()));
	connect(gotoPB, SIGNAL(clicked()),
		this, SLOT(gotoClicked()));
	connect(updatePB, SIGNAL(clicked()),
		this, SLOT(updateClicked()));
	connect(bufferCO, SIGNAL(activated(int)),
		this, SLOT(updateClicked()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	bc().addReadOnly(typeCO);

	restored_buffer_ = -1;
	active_buffer_ = -1;
}


void GuiRef::enableView(bool enable)
{
	if (!enable)
		// In the opposite case, updateContents() will be called anyway.
		updateContents();
	GuiDialog::enableView(enable);
}


void GuiRef::changed_adaptor()
{
	changed();
}


void GuiRef::gotoClicked()
{
	// By setting last_reference_, we ensure that the reference
	// to which we are going (or from which we are returning) is
	// restored in the dialog. It's a bit of a hack, but it works,
	// and no-one seems to have any better idea.
	bool const toggled = 
		last_reference_.isEmpty() || last_reference_.isNull();
	if (toggled)
		last_reference_ = referenceED->text();
	gotoRef();
	if (toggled)
		last_reference_.clear();
}


void GuiRef::selectionChanged()
{
	if (isBufferReadonly())
		return;

	QList<QTreeWidgetItem *> selections = refsTW->selectedItems();
	if (selections.isEmpty())
		return;
	QTreeWidgetItem * sel = selections.first();
	refHighlighted(sel);
	return;
}


void GuiRef::refHighlighted(QTreeWidgetItem * sel)
{
	if (sel->childCount() > 0) {
		sel->setExpanded(true);
		return;
	}

/*	int const cur_item = refsTW->currentRow();
	bool const cur_item_selected = cur_item >= 0 ?
		refsLB->isSelected(cur_item) : false;*/
	bool const cur_item_selected = refsTW->isItemSelected(sel);

	if (cur_item_selected)
		referenceED->setText(sel->text(0));

	if (at_ref_)
		gotoRef();
	gotoPB->setEnabled(true);
	if (typeAllowed() && !isBufferReadonly())
		typeCO->setEnabled(true);
	nameED->setHidden(!nameAllowed());
	nameL->setHidden(!nameAllowed());
}


void GuiRef::refTextChanged(QString const & str)
{
	gotoPB->setEnabled(!str.isEmpty());
	typeCO->setEnabled(!str.isEmpty());
	typeLA->setEnabled(!str.isEmpty());
}


void GuiRef::refSelected(QTreeWidgetItem * sel)
{
	if (isBufferReadonly())
		return;

	if (sel->childCount()) {
		sel->setExpanded(false);
		return;
	}

/*	int const cur_item = refsTW->currentRow();
	bool const cur_item_selected = cur_item >= 0 ?
		refsLB->isSelected(cur_item) : false;*/
	bool const cur_item_selected = refsTW->isItemSelected(sel);

	if (cur_item_selected)
		referenceED->setText(sel->text(0));
	// <enter> or double click, inserts ref and closes dialog
	slotOK();
}


void GuiRef::sortToggled()
{
	redoRefs();
}


void GuiRef::groupToggled()
{
	redoRefs();
}


void GuiRef::updateClicked()
{
	updateRefs();
}


void GuiRef::dialogRejected()
{
	resetDialog();
	// We have to do this manually, instead of calling slotClose(), because
	// the dialog has already been made invisible before rejected() triggers.
	Dialog::disconnect();
}


void GuiRef::resetDialog()
{
	at_ref_ = false;
	setGotoRef();
}


void GuiRef::closeEvent(QCloseEvent * e)
{
	slotClose();
	resetDialog();
	e->accept();
}


void GuiRef::updateContents()
{
	int orig_type = typeCO->currentIndex();

	referenceED->clear();
	nameED->clear();

	referenceED->setText(toqstr(params_["reference"]));
	nameED->setText(toqstr(params_["name"]));
	nameED->setHidden(!nameAllowed());
	nameL->setHidden(!nameAllowed());

	// restore type settings for new insets
	bool const new_inset = params_["reference"].empty();
	if (new_inset)
		typeCO->setCurrentIndex(orig_type);
	else
		typeCO->setCurrentIndex(InsetRef::getType(params_.getCmdName()));
	typeCO->setEnabled(typeAllowed() && !isBufferReadonly());
	if (!typeAllowed())
		typeCO->setCurrentIndex(0);

	// insert buffer list
	bufferCO->clear();
	FileNameList const buffers(theBufferList().fileNames());
	for (FileNameList::const_iterator it = buffers.begin();
	     it != buffers.end(); ++it) {
		bufferCO->addItem(toqstr(makeDisplayPath(it->absFileName())));
	}

	int const thebuffer = theBufferList().bufferNum(buffer().fileName());
	// restore the buffer combo setting for new insets
	if (new_inset && restored_buffer_ != -1
	    && restored_buffer_ < bufferCO->count() && thebuffer == active_buffer_)
		bufferCO->setCurrentIndex(restored_buffer_);
	else {
		int const num = theBufferList().bufferNum(buffer().fileName());
		bufferCO->setCurrentIndex(num);
		if (thebuffer != active_buffer_)
			restored_buffer_ = num;
	}
	active_buffer_ = thebuffer;

	updateRefs();
	// Activate OK/Apply buttons if the users inserts a new ref
	// and we have a valid pre-setting.
	bc().setValid(isValid() && new_inset);
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
	gotoPB->setToolTip(qt_("Jump back to the original cursor location"));
}


void GuiRef::setGotoRef()
{
	gotoPB->setText(qt_("&Go to Label"));
	gotoPB->setToolTip(qt_("Jump to the selected label"));
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

inline bool caseInsensitiveLessThan(QString const & s1, QString const & s2)
{
	return s1.toLower() < s2.toLower();
}


void GuiRef::redoRefs()
{
	// Prevent these widgets from emitting any signals whilst
	// we modify their state.
	refsTW->blockSignals(true);
	referenceED->blockSignals(true);
	refsTW->setUpdatesEnabled(false);

	refsTW->clear();

	// need this because Qt will send a highlight() here for
	// the first item inserted
	QString const oldSelection(referenceED->text());

	QStringList refsStrings;
	QStringList refsCategories;
	vector<docstring>::const_iterator iter;
	bool noprefix = false;
	for (iter = refs_.begin(); iter != refs_.end(); ++iter) {
		QString const lab = toqstr(*iter);
		refsStrings.append(lab);
		if (groupCB->isChecked()) {
			if (lab.contains(":")) {
				QString const pref = lab.split(':')[0];
				if (!refsCategories.contains(pref)) {
					if (!pref.isEmpty())
						refsCategories.append(pref);
					else
						noprefix = true;
				}
			}
			else
				noprefix = true;
		}
	}
	// sort categories case-intensively
	qSort(refsCategories.begin(), refsCategories.end(),
	      caseInsensitiveLessThan /*defined above*/);
	if (noprefix)
		refsCategories.insert(0, qt_("<No prefix>"));

	QString const sort = sortingCO->isEnabled() ?
				sortingCO->itemData(sortingCO->currentIndex()).toString()
				: QString();
	if (sort == "nocase")
		qSort(refsStrings.begin(), refsStrings.end(),
		      caseInsensitiveLessThan /*defined above*/);
	else if (sort == "case")
		qSort(refsStrings.begin(), refsStrings.end());

	if (groupCB->isChecked()) {
		QList<QTreeWidgetItem *> refsCats;
		for (int i = 0; i < refsCategories.size(); ++i) {
			QString const cat = refsCategories.at(i);
			QTreeWidgetItem * item = new QTreeWidgetItem(refsTW);
			item->setText(0, cat);
			for (int i = 0; i < refsStrings.size(); ++i) {
				QString const ref = refsStrings.at(i);
				if ((ref.startsWith(cat + QString(":")))
				    || (cat == qt_("<No prefix>")
				       && (!ref.mid(1).contains(":") || ref.left(1).contains(":")))) {
						QTreeWidgetItem * child =
							new QTreeWidgetItem(item);
						child->setText(0, ref);
						item->addChild(child);
				}
			}
			refsCats.append(item);
		}
		refsTW->addTopLevelItems(refsCats);
	} else {
		QList<QTreeWidgetItem *> refsItems;
		for (int i = 0; i < refsStrings.size(); ++i) {
			QTreeWidgetItem * item = new QTreeWidgetItem(refsTW);
			item->setText(0, refsStrings.at(i));
			refsItems.append(item);
		}
		refsTW->addTopLevelItems(refsItems);
	}

	// restore the last selection or, for new insets, highlight
	// the previous selection
	if (!oldSelection.isEmpty() || !last_reference_.isEmpty()) {
		bool const newInset = oldSelection.isEmpty();
		QString textToFind = newInset ? last_reference_ : oldSelection;
		referenceED->setText(textToFind);
		last_reference_.clear();
		QTreeWidgetItemIterator it(refsTW);
		while (*it) {
			if ((*it)->text(0) == textToFind) {
				refsTW->setCurrentItem(*it);
				refsTW->setItemSelected(*it, true);
				//Make sure selected item is visible
				refsTW->scrollToItem(*it);
				last_reference_ = textToFind;
				break;
			}
			++it;
		}
	}
	refsTW->setUpdatesEnabled(true);
	refsTW->update();

	// redo filter
	filterLabels();

	// Re-activate the emission of signals by these widgets.
	refsTW->blockSignals(false);
	referenceED->blockSignals(false);

	gotoPB->setEnabled(!referenceED->text().isEmpty());
	typeCO->setEnabled(!referenceED->text().isEmpty());
	typeLA->setEnabled(!referenceED->text().isEmpty());
}


void GuiRef::updateRefs()
{
	refs_.clear();
	int const the_buffer = bufferCO->currentIndex();
	if (the_buffer != -1) {
		FileNameList const names(theBufferList().fileNames());
		FileName const & name = names[the_buffer];
		Buffer const * buf = theBufferList().getBuffer(name);
		buf->getLabelList(refs_);
	}
	sortingCO->setEnabled(!refs_.empty());
	refsTW->setEnabled(!refs_.empty());
	groupCB->setEnabled(!refs_.empty());
	// refsTW should only be the focus proxy when it is enabled
	setFocusProxy(refs_.empty() ? 0 : refsTW);
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


void GuiRef::filterLabels()
{
	Qt::CaseSensitivity cs = csFindCB->isChecked() ?
		Qt::CaseSensitive : Qt::CaseInsensitive;
	QTreeWidgetItemIterator it(refsTW);
	while (*it) {
		(*it)->setHidden(
			(*it)->childCount() == 0
			&& !(*it)->text(0).contains(filter_->text(), cs)
		);
		++it;
	}
}


bool GuiRef::initialiseParams(std::string const & data)
{
	InsetCommand::string2params(data, params_);
	return true;
}


void GuiRef::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string(params_);
	dispatch(FuncRequest(getLfun(), lfun));
}



Dialog * createGuiRef(GuiView & lv) { return new GuiRef(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiRef.cpp"
