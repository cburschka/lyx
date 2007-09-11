/**
 * \file ControlTexinfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlTexinfo.h"
#include "FuncRequest.h"

#include "support/filetools.h"
#include "support/FileName.h"
#include "support/lstrings.h"

#include <algorithm>

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

using support::FileName;
using support::contains;
using support::split;
using support::token;


ControlTexinfo::ControlTexinfo(Dialog & parent)
	: Controller(parent)
{}


void ControlTexinfo::viewFile(string const & filename) const
{
	string const arg = "file " + filename;
	dispatch(FuncRequest(LFUN_DIALOG_SHOW, arg));
}


string const ControlTexinfo::getClassOptions(string const & classname) const
{
	FileName const filename(getTexFileFromList(classname, "cls"));
	if (filename.empty())
		return string();
	string optionList = string();
	std::ifstream is(filename.toFilesystemEncoding().c_str());
	while (is) {
		string s;
		is >> s;
		if (contains(s, "DeclareOption")) {
			s = s.substr(s.find("DeclareOption"));
			s = split(s, '{');		// cut front
			s = token(s, '}', 0);		// cut end
			optionList += (s + '\n');
		}
	}
	return optionList;
}


string const ControlTexinfo::getFileType(ControlTexinfo::texFileSuffix type) const
{
	string ftype;
	switch (type) {
	case ControlTexinfo::bst:
		ftype = "bst";
		break;
	case ControlTexinfo::cls:
		ftype = "cls";
		break;
	case ControlTexinfo::sty:
		ftype = "sty";
		break;
	}
	return ftype;
}

} // namespace frontend
} // namespace lyx
