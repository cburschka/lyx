/**
 * \file FormAboutlyx.C
 * See the file COPYING.
 *
 * \author Edwin Leuven
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormAboutlyx.h"
#include "xformsBC.h"
#include "ControlAboutlyx.h"
#include "forms/form_aboutlyx.h"
#include "xforms_helpers.h"
#include FORMS_H_LOCATION
#include "Lsstream.h"

using std::getline;

typedef FormCB<ControlAboutlyx, FormDB<FD_aboutlyx> > base_class;

FormAboutlyx::FormAboutlyx()
	: base_class(_("About LyX"), false)
{}


void FormAboutlyx::build()
{
	dialog_.reset(build_aboutlyx(this));

	// create version tab
	version_.reset(build_aboutlyx_version(this));
	fl_set_object_label(version_->text_version,
			    controller().getVersion().c_str());
	fl_set_object_label(version_->text_copyright,
			    controller().getCopyright().c_str());

	// create license and warranty tab
	license_.reset(build_aboutlyx_license(this));

	string str = formatted(controller().getLicense(),
			       license_->text_license->w - 10);
	fl_set_object_label(license_->text_license, str.c_str());

	str = formatted(controller().getDisclaimer(),
			license_->text_warranty->w - 10);
	fl_set_object_label(license_->text_warranty, str.c_str());

	// create credits
	credits_.reset(build_aboutlyx_credits(this));
	stringstream ss;
	fl_add_browser_line(credits_->browser_credits,
			    controller().getCredits(ss).str().c_str());

	// stack tabs
	fl_addto_tabfolder(dialog_->tabfolder,_("Copyright and Version"),
			   version_->form);
	fl_addto_tabfolder(dialog_->tabfolder,_("License and Warranty"),
			   license_->form);
	fl_addto_tabfolder(dialog_->tabfolder,_("Credits"),
			   credits_->form);

	// Manage the cancel/close button
	bc().setCancel(dialog_->button_close);
}
