/**
 * \file ControlNote.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlNote.h"
#include "funcrequest.h"
#include "insets/insetnote.h"
#include "gettext.h"


using std::vector;
using std::string;


ControlNote::ControlNote(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlNote::initialiseParams(string const & data)
{
	InsetNoteParams params;
	InsetNoteMailer::string2params(data, params);
	params_.reset(new InsetNoteParams(params));
	return true;
}


void ControlNote::clearParams()
{
	params_.reset();
}


void ControlNote::dispatchParams()
{
	string const lfun = InsetNoteMailer::params2string(params());
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}


void note_gui_tokens(vector<string> & ids, vector<string> & gui_names)
{
	char const * const ids_[] = {"Note", "Comment", "Greyedout"};
	size_t const ids_size = sizeof(ids_) / sizeof(char *);
	ids = vector<string>(ids_, ids_ + ids_size);
	gui_names.clear();
	gui_names.push_back(_("LyX Note"));
	gui_names.push_back(_("Comment"));
	gui_names.push_back(_("Greyed Out"));
}
