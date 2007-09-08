/**
 * \file ControlBox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer (with useful hints from Angus Leeming)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlBox.h"
#include "FuncRequest.h"
#include "insets/InsetBox.h"
#include "gettext.h"


using std::string;
using std::vector;

namespace lyx {
namespace frontend {

ControlBox::ControlBox(Dialog & parent)
	: Controller(parent)
{}


bool ControlBox::initialiseParams(string const & data)
{
	InsetBoxParams params("");
	InsetBoxMailer::string2params(data, params);
	params_.reset(new InsetBoxParams(params));

	return true;

}


void ControlBox::clearParams()
{
	params_.reset();
}


void ControlBox::dispatchParams()
{
	string const lfun = InsetBoxMailer::params2string(params());
	kernel().dispatch(FuncRequest(getLfun(), lfun));
}


void box_gui_tokens(vector<string> & ids, vector<docstring> & gui_names)
{
	char const * const ids_[] = {
		"Frameless", "Boxed", "ovalbox",
		"Ovalbox", "Shadowbox", "Doublebox"};
	size_t const ids_size = sizeof(ids_) / sizeof(char *);
	ids = vector<string>(ids_, ids_ + ids_size);
	gui_names.clear();
	gui_names.push_back(_("No frame drawn"));
	gui_names.push_back(_("Rectangular box"));
	gui_names.push_back(_("Oval box, thin"));
	gui_names.push_back(_("Oval box, thick"));
	gui_names.push_back(_("Shadow box"));
	gui_names.push_back(_("Double box"));
}

void box_gui_tokens_special_length(vector<string> & ids,
	vector<docstring> & gui_names)
{
	char const * const ids_[] = {
		"none", "height", "depth",
		"totalheight", "width"};
	size_t const ids_size = sizeof(ids_) / sizeof(char *);
	ids = vector<string>(ids_, ids_ + ids_size);
	gui_names.clear();
	gui_names.push_back(_("None"));
	gui_names.push_back(_("Height"));
	gui_names.push_back(_("Depth"));
	gui_names.push_back(_("Total Height"));
	gui_names.push_back(_("Width"));
}

} // namespace frontend
} // namespace lyx
