/**
 * \file FormAboutlyx.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlAboutlyx.h"
#include "FormAboutlyx.h"
#include "form_aboutlyx.h"
#include "xforms_helpers.h"
#include "Lsstream.h"

using std::getline;

typedef FormCB<ControlAboutlyx, FormDB<FD_form_aboutlyx> > base_class;

FormAboutlyx::FormAboutlyx(ControlAboutlyx & c)
	: base_class(c, _("About LyX"), false)
{}


void FormAboutlyx::build()
{
	dialog_.reset(build_aboutlyx());

	// create version tab
	version_.reset(build_tab_version());
	fl_set_object_label(version_->text_version, 
			    controller().getVersion().c_str());
	fl_set_object_label(version_->text_copyright, 
			    controller().getCopyright().c_str());

	// create license and warranty tab
	license_.reset(build_tab_license());
	
	string str = formatted(controller().getLicense(),
			       license_->text_license->w-10);
	fl_set_object_label(license_->text_license, str.c_str());

	str = formatted(controller().getDisclaimer(),
			license_->text_warranty->w-10);
	fl_set_object_label(license_->text_warranty, str.c_str());

	// create credits
	credits_.reset(build_tab_credits());
	stringstream ss;
	fl_add_browser_line(credits_->browser_credits, 
			    controller().getCredits(ss).str().c_str());

	// stack tabs
	fl_addto_tabfolder(dialog_->tabbed_folder,_("Copyright and Version"),
		       version_->form);
	fl_addto_tabfolder(dialog_->tabbed_folder,_("License and Warranty"),
		       license_->form);
	fl_addto_tabfolder(dialog_->tabbed_folder,_("Credits"),
		       credits_->form);

	// Manage the cancel/close button
	bc().setCancel(dialog_->close);
}





