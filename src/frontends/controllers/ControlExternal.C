/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlExternal.C
 * \author Asger Alstrup
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#include <utility>
#include <vector>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlExternal.h"
#include "buffer.h"
#include "Dialogs.h"
#include "Liason.h"
#include "LyXView.h"
#include "support/filetools.h"
#include "frontends/FileDialog.h"
#include "lyx_gui_misc.h" // WriteAlert
#include "gettext.h"

using std::make_pair;
using std::vector;

ControlExternal::ControlExternal(LyXView & lv, Dialogs & d)
	: ControlInset<InsetExternal, InsetExternal::Params>(lv, d)
{
	d_.showExternal.connect(SigC::slot(this, &ControlExternal::showInset));
}


InsetExternal::Params const ControlExternal::getParams(string const &)
{
	return InsetExternal::Params();
}

InsetExternal::Params const 
ControlExternal::getParams(InsetExternal const & inset)
{
	return inset.params();
}


void ControlExternal::applyParamsToInset()
{
	inset()->setFromParams(params());
	lv_.view()->updateInset(inset(), true);
}

void ControlExternal::editExternal()
{
	// fill the local, controller's copy of the Params struct with
	// the contents of the dialog's fields.
	view().apply();

	// Create a local copy of the inset and initialise it with this
	// params struct.
	boost::scoped_ptr<InsetExternal> ie;
	ie.reset(static_cast<InsetExternal *>(inset()->Clone(*lv_.buffer())));
	ie->setFromParams(params());

	ie->editExternal();
}

void ControlExternal::viewExternal()
{
	view().apply();

	boost::scoped_ptr<InsetExternal> ie;
	ie.reset(static_cast<InsetExternal *>(inset()->Clone(*lv_.buffer())));
	ie->setFromParams(params());

	ie->viewExternal();
}

void ControlExternal::updateExternal()
{
	view().apply();

	boost::scoped_ptr<InsetExternal> ie;
	ie.reset(static_cast<InsetExternal *>(inset()->Clone(*lv_.buffer())));
	ie->setFromParams(params());

	ie->updateExternal();
}

vector<string> const ControlExternal::getTemplates() const
{
	vector<string> result;

	ExternalTemplateManager::Templates::const_iterator i1, i2;
	i1 = ExternalTemplateManager::get().getTemplates().begin();
	i2 = ExternalTemplateManager::get().getTemplates().end();

	for (; i1 != i2; ++i1) {
		result.push_back(i1->second.lyxName);
	}
	return result;
}


int ControlExternal::getTemplateNumber(string const & name) const
{
	int i = 0;

	ExternalTemplateManager::Templates::const_iterator i1, i2;
	i1 = ExternalTemplateManager::get().getTemplates().begin();
	i2 = ExternalTemplateManager::get().getTemplates().end();
	for (; i1 != i2; ++i1) {
		if (i1->second.lyxName == name)
			return i;
		++i;
	}

	// we can get here if a LyX document has a template not installed
	// on this machine.
	return -1;
}


ExternalTemplate ControlExternal::getTemplate(int i) const
{
	ExternalTemplateManager::Templates::const_iterator i1;
	i1 = ExternalTemplateManager::get().getTemplates().begin();
	for (int n = 1; n < i; ++n)
		++i1;

	return (*i1).second;
}


string const ControlExternal::Browse(string const & input) const
{
	string buf  = MakeAbsPath(lv_.buffer()->fileName());
	string buf2 = OnlyPath(buf);

	if (!input.empty()) {
		buf = MakeAbsPath(input, buf2);
		buf = OnlyPath(buf);
	} else {
		buf = OnlyPath(lv_.buffer()->fileName());
	}
    
	FileDialog fileDlg(&lv_,
			   _("Select external file"),
			   LFUN_SELECT_FILE_SYNC,
			   make_pair(string(_("Document")), string(buf)));
	
	/// Determine the template file extension
	ExternalTemplate const & et = params().templ;

	string regexp = et.fileRegExp;
	if (regexp.empty())
		regexp = "*";

	// FIXME: a temporary hack until the FileDialog interface is updated
	regexp += "|";

	static int once;
	string current_path;

	while (1) {
		string const path = (once) ? current_path : buf;
		FileDialog::Result result = fileDlg.Select(path, regexp, input);

		if (result.second.empty())
			return string();

		string p = result.second;

		buf = MakeRelPath(p, buf2);
		current_path = OnlyPath(p);
		once = 1;
		
		if (contains(p, "#") ||
		    contains(p, "~") ||
		    contains(p, "$") ||
		    contains(p, "%")) {
			WriteAlert(_("Filename can't contain any "
				     "of these characters:"),
				   // xgettext:no-c-format
				   _("'#', '~', '$' or '%'."));
		} else
			break;
	}

	return buf;
}
