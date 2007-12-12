/**
 * \file Mover.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Mover.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/Systemcall.h"

#include <fstream>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {


bool Mover::copy(FileName const & from, FileName const & to,
		 unsigned long int mode) const
{
	return do_copy(from, to, to.absFilename(), mode);
}


bool Mover::do_copy(FileName const & from, FileName const & to,
		    string const &, unsigned long int mode) const
{
	return support::copy(from, to, mode);
}


bool Mover::rename(FileName const & from,
		   FileName const & to) const
{
	return do_rename(from, to, to.absFilename());
}


bool Mover::do_rename(FileName const & from, FileName const & to,
		      string const &) const
{
	return rename(from, to);
}


bool SpecialisedMover::do_copy(FileName const & from, FileName const & to,
			       string const & latex, unsigned long int mode) const
{
	if (command_.empty())
		return Mover::do_copy(from, to, latex, mode);

	if (mode != (unsigned long int)-1) {
		ofstream ofs(to.toFilesystemEncoding().c_str(), ios::binary | ios::out | ios::trunc);
		if (!ofs)
			return false;
		ofs.close();
		if (!chmod(to, mode))
			return false;
	}

	string command = libScriptSearch(command_);
	command = subst(command, "$$i", quoteName(from.toFilesystemEncoding()));
	command = subst(command, "$$o", quoteName(to.toFilesystemEncoding()));
	command = subst(command, "$$l", quoteName(latex));

	Systemcall one;
	return one.startscript(Systemcall::Wait, command) == 0;
}


bool SpecialisedMover::do_rename(FileName const & from, FileName const & to,
				 string const & latex) const
{
	if (command_.empty())
		return Mover::do_rename(from, to, latex);

	if (!do_copy(from, to, latex, (unsigned long int)-1))
		return false;
	return from.removeFile();
}


void Movers::set(string const & fmt, string const & command)
{
	specials_[fmt] = SpecialisedMover(command);
}


Mover const & Movers::operator()(string const & fmt) const
{
	SpecialsMap::const_iterator const it = specials_.find(fmt);
	if (it == specials_.end())
		return default_;
	return  it->second;
}


string const Movers::command(string  const & fmt) const
{
	SpecialsMap::const_iterator const it = specials_.find(fmt);
	return (it == specials_.end()) ? string() : it->second.command();
}


} // namespace lyx
