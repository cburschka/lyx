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

#ifdef __GNUG__
#pragma implementation
#endif

#include "BufferView.h"
#include "ButtonControllerBase.h"
#include "ControlExternal.h"
#include "ControlInset.tmpl"
#include "Dialogs.h"
#include "Liason.h"
#include "LyXView.h"
#include "ViewBase.h"
#include "buffer.h"
#include "frontends/Alert.h"
#include "gettext.h"
#include "helper_funcs.h"
#include "lyxrc.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include <boost/scoped_ptr.hpp>

#include <utility>
#include <vector>

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
	ie.reset(static_cast<InsetExternal *>(inset()->clone(*lv_.buffer())));
	ie->setFromParams(params());

	ie->editExternal();
}

void ControlExternal::viewExternal()
{
	view().apply();

	boost::scoped_ptr<InsetExternal> ie;
	ie.reset(static_cast<InsetExternal *>(inset()->clone(*lv_.buffer())));
	ie->setFromParams(params());

	ie->viewExternal();
}

void ControlExternal::updateExternal()
{
	view().apply();

	boost::scoped_ptr<InsetExternal> ie;
	ie.reset(static_cast<InsetExternal *>(inset()->clone(*lv_.buffer())));
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

	return i1->second;
}


string const ControlExternal::Browse(string const & input) const
{
	string const title =  _("Select external file");

	string const bufpath = lv_.buffer()->filePath();

	/// Determine the template file extension
	ExternalTemplate const & et = params().templ;
	string pattern = et.fileRegExp;
	if (pattern.empty())
		pattern = "*";

	// FIXME: a temporary hack until the FileDialog interface is updated
	pattern += "|";

	std::pair<string, string> dir1(N_("Documents|#o#O"),
				  string(lyxrc.document_path));

	return browseRelFile(&lv_, input, bufpath, title, pattern, dir1);
}
