/**
 * \file ControlExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlExternal.h"
#include "buffer.h"
#include "funcrequest.h"
#include "gettext.h"
#include "helper_funcs.h"
#include "lyxrc.h"
#include <vector>

using std::vector;


ControlExternal::ControlExternal(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlExternal::initialiseParams(string const & data)
{
	inset_.reset(new InsetExternal);
	InsetExternal::Params params;
	InsetExternalMailer::string2params(data, params);
	inset_->setFromParams(params);
	inset_->setView(kernel().bufferview());
	return true;
}


void ControlExternal::clearParams()
{
	inset_.reset();
}


void ControlExternal::dispatchParams()
{
	string const lfun = InsetExternalMailer::params2string(params());
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}


void ControlExternal::setParams(InsetExternal::Params const & p)
{
	inset_->setFromParams(p);
}


void ControlExternal::editExternal()
{
	dialog().view().apply();
	inset_->editExternal();
}


void ControlExternal::viewExternal()
{
	dialog().view().apply();
	inset_->viewExternal();
}


void ControlExternal::updateExternal()
{
	dialog().view().apply();
	inset_->updateExternal();
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
	ExternalTemplateManager::Templates::const_iterator i1, i2;
	i1 = ExternalTemplateManager::get().getTemplates().begin();
	i2 = ExternalTemplateManager::get().getTemplates().end();
	for (int i = 0; i1 != i2; ++i1, ++i) {
		if (i1->second.lyxName == name)
			return i;
	}

	// we can get here if a LyX document has a template not installed
	// on this machine.
	return -1;
}


ExternalTemplate ControlExternal::getTemplate(int i) const
{
	ExternalTemplateManager::Templates::const_iterator i1
		= ExternalTemplateManager::get().getTemplates().begin();

	std::advance(i1,  i);

	return i1->second;
}


string const ControlExternal::Browse(string const & input) const
{
	string const title =  _("Select external file");

	string const bufpath = kernel().buffer()->filePath();

	/// Determine the template file extension
	ExternalTemplate const & et = params().templ;
	string pattern = et.fileRegExp;
	if (pattern.empty())
		pattern = "*";

	// FIXME: a temporary hack until the FileDialog interface is updated
	pattern += '|';

	std::pair<string, string> dir1(N_("Documents|#o#O"),
				  string(lyxrc.document_path));

	return browseRelFile(input, bufpath, title, pattern, false, dir1);
}
