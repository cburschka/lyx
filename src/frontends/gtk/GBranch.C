/**
 * \file GBranch.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Reiter
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GBranch.h"
#include "ControlBranch.h"

#include "BranchList.h"
#include "insets/insetbranch.h"

#include "ghelpers.h"

#include <libglademm.h>

using std::string;

namespace lyx {
namespace frontend {

GBranch::GBranch(Dialog & parent)
	: GViewCB<ControlBranch, GViewGladeB>(parent, _("Branch Settings"), false)
{}


void GBranch::doBuild()
{
	string const gladeName = findGladeFile("branch");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	xml_->get_widget("Cancel", cancelbutton_);
	setCancel(cancelbutton_);
	xml_->get_widget("OK", okbutton_);
	setOK(okbutton_);

	Gtk::Box * box = NULL;
	xml_->get_widget("innerbox", box);
	box->pack_start(branchescombo_, true, true, 0);
	box->show_all();

	// Set shortcut target
	xml_->get_widget("BranchesLabel", brancheslabel_);
	brancheslabel_->set_mnemonic_widget(branchescombo_);

	// Single click in branches list
	branchescombo_.signal_changed().connect(
		sigc::mem_fun(*this, &GBranch::selection_changed));

}


void GBranch::update()
{
	applylock_ = true;

	typedef BranchList::const_iterator const_iterator;

	BranchList const branchlist = controller().branchlist();
	string const cur_branch = controller().params().branch;

	// FIXME: deprecated in favor of clear_items since gtkmm 2.8
	branchescombo_.clear();

	const_iterator const begin = branchlist.begin();
	const_iterator const end = branchlist.end();
	for (const_iterator it = begin; it != end; ++it)
		branchescombo_.append_text(it->getBranch());
	branchescombo_.set_active_text(cur_branch);

	applylock_ = false;
}


void GBranch::apply()
{
	controller().params().branch = branchescombo_.get_active_text();;
}


void GBranch::selection_changed()
{
	if (!applylock_)
		bc().valid(true);
}


} // namespace frontend
} // namespace lyx
