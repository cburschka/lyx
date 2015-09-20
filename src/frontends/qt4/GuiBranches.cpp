/**
 * \file GuiBranches.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBranches.h"

#include "ColorCache.h"
#include "GuiApplication.h"
#include "Validator.h"
#include "qt_helpers.h"

#include "ui_BranchesUnknownUi.h"

#include "frontends/alert.h"

#include "Buffer.h"
#include "BufferParams.h"

#include "support/gettext.h"
#include "support/lstrings.h"

#include <QKeyEvent>
#include <QListWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPixmap>
#include <QIcon>
#include <QColor>
#include <QColorDialog>

#ifdef KeyPress
#undef KeyPress
#endif

namespace lyx {
namespace frontend {


GuiBranches::GuiBranches(QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);
	branchesTW->setColumnCount(3);
	branchesTW->headerItem()->setText(0, qt_("Branch"));
	branchesTW->headerItem()->setText(1, qt_("Activated"));
	branchesTW->headerItem()->setText(2, qt_("Color"));
	branchesTW->headerItem()->setText(3, qt_("Filename Suffix"));
	branchesTW->setSortingEnabled(true);
	branchesTW->resizeColumnToContents(1);
	branchesTW->resizeColumnToContents(2);

	undef_ = new BranchesUnknownDialog(this);
	undef_bc_.setPolicy(ButtonPolicy::OkCancelPolicy);
	undef_bc_.setCancel(undef_->cancelPB);

	connect(undef_->branchesLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(unknownBranchSelChanged()));
	connect(undef_->addSelectedPB, SIGNAL(clicked()),
		this, SLOT(addUnknown()));
	connect(undef_->addAllPB, SIGNAL(clicked()),
		this, SLOT(addAllUnknown()));
	connect(undef_->addSelectedPB, SIGNAL(clicked()),
		undef_, SLOT(accept()));
	connect(undef_->addAllPB, SIGNAL(clicked()),
		undef_, SLOT(accept()));
	connect(undef_->cancelPB, SIGNAL(clicked()),
		undef_, SLOT(reject()));

	newBranchLE->installEventFilter(this);
	newBranchLE->setValidator(new NoNewLineValidator(newBranchLE));
}


bool GuiBranches::eventFilter(QObject * obj, QEvent * event) 
{
	QEvent::Type etype = event->type();
	if (etype == QEvent::KeyPress 
		  && obj == newBranchLE
		  && addBranchPB->isEnabled()) {
		QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
		int const keyPressed = keyEvent->key();
		Qt::KeyboardModifiers const keyModifiers = keyEvent->modifiers();

		if (keyPressed == Qt::Key_Enter || keyPressed == Qt::Key_Return) {
			if (!keyModifiers) {
				on_addBranchPB_pressed();
			} else if (keyModifiers == Qt::ControlModifier
				  || keyModifiers == Qt::KeypadModifier
				  || keyModifiers == (Qt::ControlModifier | Qt::KeypadModifier)) {
				on_addBranchPB_pressed();
				newBranchLE->clearFocus();
				okPressed();
			}
			event->accept();
			return true;
		}
	}
	return QObject::eventFilter(obj, event);
}


void GuiBranches::update(BufferParams const & params)
{
	branchlist_ = params.branchlist();
	updateView();
}


void GuiBranches::updateView()
{
	// store the selected branch
	QTreeWidgetItem * item = branchesTW->currentItem();
	QString sel_branch;
	if (item != 0)
		sel_branch = item->text(0);

	branchesTW->clear();

	BranchList::const_iterator it = branchlist_.begin();
	BranchList::const_iterator const end = branchlist_.end();
	for (; it != end; ++it) {
		QTreeWidgetItem * newItem = new QTreeWidgetItem(branchesTW);

		QString const bname = toqstr(it->branch());
		newItem->setText(0, bname);
		newItem->setText(1, it->isSelected() ? qt_("Yes") : qt_("No"));

		QColor const itemcolor = rgb2qcolor(it->color());
		if (itemcolor.isValid()) {
			QPixmap coloritem(30, 10);
			coloritem.fill(itemcolor);
			newItem->setIcon(2, QIcon(coloritem));
		}
		newItem->setText(3, it->hasFileNameSuffix() ? qt_("Yes") : qt_("No"));
		// restore selected branch
		if (bname == sel_branch) {
			branchesTW->setCurrentItem(newItem);
			branchesTW->setItemSelected(newItem, true);
		}
	}
	unknownPB->setEnabled(!unknown_branches_.isEmpty());
	bool const have_sel =
		!branchesTW->selectedItems().isEmpty();
	removePB->setEnabled(have_sel);
	renamePB->setEnabled(have_sel);
	colorPB->setEnabled(have_sel);
	activatePB->setEnabled(have_sel);
	suffixPB->setEnabled(have_sel);
	// emit signal
	changed();
}


void GuiBranches::apply(BufferParams & params) const
{
	params.branchlist() = branchlist_;
}


void GuiBranches::on_newBranchLE_textChanged(QString)
{
	QString const new_branch = newBranchLE->text();
	addBranchPB->setEnabled(!new_branch.isEmpty());
}


void GuiBranches::on_addBranchPB_pressed()
{
	QString const new_branch = newBranchLE->text();
	branchlist_.add(qstring_to_ucs4(new_branch));
	newBranchLE->clear();
	addBranchPB->setEnabled(false);
	updateView();
}


void GuiBranches::on_removePB_pressed()
{
	QTreeWidgetItem * selItem = branchesTW->currentItem();
	QString sel_branch;
	if (selItem != 0)
		sel_branch = selItem->text(0);
	if (!sel_branch.isEmpty()) {
		branchlist_.remove(qstring_to_ucs4(sel_branch));
		newBranchLE->clear();
		updateView();
	}
}


void GuiBranches::on_renamePB_pressed()
{
	QTreeWidgetItem * selItem = branchesTW->currentItem();
	QString sel_branch;
	if (selItem != 0)
		sel_branch = selItem->text(0);
	if (!sel_branch.isEmpty()) {
		docstring newname;
		docstring const oldname = qstring_to_ucs4(sel_branch);
		if (Alert::askForText(newname, _("Enter new branch name"), oldname)) {
			if (newname.empty() || oldname == newname)
				return;
			bool success = false;
			if (branchlist_.find(newname)) {
				docstring text = support::bformat(
					_("A branch with the name \"%1$s\" already exists.\n"
					  "Do you want to merge branch \"%2$s\" with that one?"),
					newname, oldname);
				if (frontend::Alert::prompt(_("Branch already exists"),
					  text, 0, 1, _("&Merge"), _("&Cancel")) == 0)
					success = branchlist_.rename(oldname, newname, true);
			} else
				success = branchlist_.rename(oldname, newname);
			newBranchLE->clear();
			updateView();

			if (!success)
				Alert::error(_("Renaming failed"), 
				      _("The branch could not be renamed."));
			else
				// emit signal
				renameBranches(oldname, newname);
		}
	}
}


void GuiBranches::on_activatePB_pressed()
{
	toggleBranch(branchesTW->currentItem());
}


void GuiBranches::on_suffixPB_pressed()
{
	toggleSuffix(branchesTW->currentItem());
}


void GuiBranches::on_branchesTW_itemDoubleClicked(QTreeWidgetItem * item, int col)
{
	if (col < 2)
		toggleBranch(item);
	else if (col == 2)
		toggleColor(item);
	else if (col == 3)
		toggleSuffix(item);
}


void GuiBranches::on_branchesTW_itemSelectionChanged()
{
	bool const have_sel =
		!branchesTW->selectedItems().isEmpty();
	removePB->setEnabled(have_sel);
	renamePB->setEnabled(have_sel);
	colorPB->setEnabled(have_sel);
	activatePB->setEnabled(have_sel);
	suffixPB->setEnabled(have_sel);
}


void GuiBranches::toggleBranch(QTreeWidgetItem * item)
{
	if (item == 0)
		return;

	QString sel_branch = item->text(0);
	if (sel_branch.isEmpty())
		return;

	Branch * branch = branchlist_.find(qstring_to_ucs4(sel_branch));
	if (branch && branch->setSelected(!branch->isSelected())) {
		newBranchLE->clear();
		updateView();
	}
}


void GuiBranches::on_colorPB_clicked()
{
	toggleColor(branchesTW->currentItem());
}


void GuiBranches::toggleColor(QTreeWidgetItem * item)
{
	if (item == 0)
		return;

	QString sel_branch = item->text(0);
	if (sel_branch.isEmpty())
		return;

	docstring current_branch = qstring_to_ucs4(sel_branch);
	Branch * branch = branchlist_.find(current_branch);
	if (!branch)
		return;

	QColor const initial = rgb2qcolor(branch->color());
	QColor ncol = QColorDialog::getColor(initial, qApp->focusWidget());
	if (!ncol.isValid())
		return;

	// add the color to the branchlist
	branch->setColor(fromqstr(ncol.name()));
	newBranchLE->clear();
	updateView();
}


void GuiBranches::toggleSuffix(QTreeWidgetItem * item)
{
	if (item == 0)
		return;

	QString sel_branch = item->text(0);
	if (sel_branch.isEmpty())
		return;

	Branch * branch = branchlist_.find(qstring_to_ucs4(sel_branch));
	if (branch) {
		branch->setFileNameSuffix(!branch->hasFileNameSuffix());
		newBranchLE->clear();
		updateView();
	}
}


void GuiBranches::on_unknownPB_pressed()
{
	undef_->branchesLW->clear();
	for (int i = 0; i != unknown_branches_.count(); ++i) {
		if (branchesTW->findItems(unknown_branches_[i], Qt::MatchExactly, 0).empty())
			undef_->branchesLW->addItem(unknown_branches_[i]);
	}
	unknownBranchSelChanged();
	undef_->exec();
}


void GuiBranches::addUnknown()
{
	QList<QListWidgetItem *> selItems =
		undef_->branchesLW->selectedItems();
	
	QList<QListWidgetItem *>::const_iterator it = selItems.begin();
	for (; it != selItems.end() ; ++it) {
		QListWidgetItem const * new_branch = *it;
		if (new_branch) {
			branchlist_.add(qstring_to_ucs4(new_branch->text()));
			updateView();
		}
	}
}


void GuiBranches::addAllUnknown()
{
	undef_->branchesLW->selectAll();
	addUnknown();
}


void GuiBranches::unknownBranchSelChanged()
{
	undef_->addSelectedPB->setEnabled(
		!undef_->branchesLW->selectedItems().isEmpty());
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiBranches.cpp"
