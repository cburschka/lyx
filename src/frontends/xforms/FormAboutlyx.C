/**
 * \file FormAboutlyx.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Angus Leeming
 * \author Rob Lahaye
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
#include "Lsstream.h"
#include "version.h"

#include FORMS_H_LOCATION

using std::string;


typedef FormCB<ControlAboutlyx, FormDB<FD_aboutlyx> > base_class;

FormAboutlyx::FormAboutlyx()
	: base_class(_("About LyX"), false)
{}


void FormAboutlyx::build()
{
	dialog_.reset(build_aboutlyx(this));

	// create version tab
	version_.reset(build_aboutlyx_version(this));

	ostringstream vs;
	vs << controller().getVersion()
	   << '\n' << lyx_version_info;

	fl_add_browser_line(version_->browser_version, vs.str().c_str());

	// create credits tab
 	credits_.reset(build_aboutlyx_credits(this));

	ostringstream crs;
	controller().getCredits(crs);

	fl_add_browser_line(credits_->browser_credits, crs.str().c_str());

	// create license tab
	license_.reset(build_aboutlyx_license(this));
	int const width = license_->browser_license->w - 10;

	ostringstream cs;
	cs << controller().getCopyright() << "\n\n"
	   << formatted(controller().getLicense(), width) << "\n\n"
	   << formatted(controller().getDisclaimer(), width);

	fl_add_browser_line(license_->browser_license, cs.str().c_str());

	// stack tabs
	fl_addto_tabfolder(dialog_->tabfolder, _("Version"),
			   version_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("Credits"),
			   credits_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("License"), 
			   license_->form);

	// work-around xforms bug re update of folder->x, folder->y coords.
	setPrehandler(dialog_->tabfolder);

	// Manage the cancel/close button
	bc().setCancel(dialog_->button_close);
}
