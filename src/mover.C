/**
 * \file mover.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include "mover.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/systemcall.h"

#include <sstream>

using std::ostringstream;
using std::string;

namespace support = lyx::support;

Movers movers;
Movers system_movers;


bool Mover::do_copy(string const & from, string const & to) const
{
	return support::copy(from, to);
}


bool Mover::do_rename(string const & from, string const & to) const
{
	return support::rename(from, to);
}


bool SpecialisedMover::do_copy(string const & from, string const & to) const
{
	if (command_.empty())
		return Mover::do_copy(from, to);

	string command = support::LibScriptSearch(command_);
	command = support::subst(command, "$$i", from);
	command = support::subst(command, "$$o", to);

	support::Systemcall one;
	return one.startscript(support::Systemcall::Wait, command) == 0;
}


bool SpecialisedMover::do_rename(string const & from, string const & to) const
{
	if (command_.empty())
		return Mover::do_rename(from, to);

	if (!do_copy(from, to))
		return false;
	return support::unlink(from) == 0;
}


void Movers::set(string const & fmt, string const & command)
{
	specials_[fmt] = SpecialisedMover(command);
}


Mover const & Movers::operator()(string const & fmt) const
{
	SpecialsMap::const_iterator const it = specials_.find(fmt);
	return (it == specials_.end()) ? default_ : it->second;
}


string const Movers::command(string  const & fmt) const
{
	SpecialsMap::const_iterator const it = specials_.find(fmt);
	return (it == specials_.end()) ? string() : it->second.command();
}
