/**
 * \file ControlExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlExternal.h"
#include "funcrequest.h"
#include "gettext.h"
#include "helper_funcs.h"
#include "lyxrc.h"

#include "insets/ExternalTemplate.h"


namespace external = lyx::external;

using std::vector;
using std::string;


ControlExternal::ControlExternal(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlExternal::initialiseParams(string const & data)
{
	params_.reset(new InsetExternal::Params);
	InsetExternalMailer::string2params(data, kernel().buffer(), *params_);
	return true;
}


void ControlExternal::clearParams()
{
	params_.reset();
}


void ControlExternal::dispatchParams()
{
	string const lfun = InsetExternalMailer::params2string(params(),
							       kernel().buffer());
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}


void ControlExternal::setParams(InsetExternal::Params const & p)
{
	BOOST_ASSERT(params_.get());
	*params_ = p;
}


InsetExternal::Params const & ControlExternal::params() const
{
	BOOST_ASSERT(params_.get());
	return *params_;
}


void ControlExternal::editExternal()
{
	BOOST_ASSERT(params_.get());

	dialog().view().apply();
	string const lfun =
		InsetExternalMailer::params2string(params(), kernel().buffer());
	kernel().dispatch(FuncRequest(LFUN_EXTERNAL_EDIT, lfun));
}


vector<string> const ControlExternal::getTemplates() const
{
	vector<string> result;

	external::TemplateManager::Templates::const_iterator i1, i2;
	i1 = external::TemplateManager::get().getTemplates().begin();
	i2 = external::TemplateManager::get().getTemplates().end();

	for (; i1 != i2; ++i1) {
		result.push_back(i1->second.lyxName);
	}
	return result;
}


int ControlExternal::getTemplateNumber(string const & name) const
{
	external::TemplateManager::Templates::const_iterator i1, i2;
	i1 = external::TemplateManager::get().getTemplates().begin();
	i2 = external::TemplateManager::get().getTemplates().end();
	for (int i = 0; i1 != i2; ++i1, ++i) {
		if (i1->second.lyxName == name)
			return i;
	}

	// we can get here if a LyX document has a template not installed
	// on this machine.
	return -1;
}


external::Template ControlExternal::getTemplate(int i) const
{
	external::TemplateManager::Templates::const_iterator i1
		= external::TemplateManager::get().getTemplates().begin();

	std::advance(i1, i);

	return i1->second;
}


namespace {

external::Template const * getTemplatePtr(InsetExternal::Params const & params)
{
	external::TemplateManager const & etm =
		external::TemplateManager::get();
	return etm.getTemplateByName(params.templatename());
}

} // namespace anon


string const ControlExternal::Browse(string const & input) const
{
	string const title =  _("Select external file");

	string const bufpath = kernel().bufferFilepath();

	/// Determine the template file extension
	string pattern = "*";
	external::Template const * const et_ptr = getTemplatePtr(params());
	if (et_ptr)
		pattern = et_ptr->fileRegExp;

	// FIXME: a temporary hack until the FileDialog interface is updated
	pattern += '|';

	std::pair<string, string> dir1(N_("Documents|#o#O"),
				  string(lyxrc.document_path));

	return browseRelFile(input, bufpath, title, pattern, false, dir1);
}
