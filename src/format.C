/**
 * \file format.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "format.h"
#include "buffer.h"
#include "bufferparams.h"
#include "lyxrc.h"
#include "debug.h"
#include "gettext.h"
#include "lyxsocket.h"

#include "frontends/Alert.h" //to be removed?

#include "support/filetools.h"
#include "support/path.h"
#include "support/systemcall.h"

using lyx::support::bformat;
using lyx::support::compare_ascii_no_case;
using lyx::support::contains;
using lyx::support::OnlyFilename;
using lyx::support::OnlyPath;
using lyx::support::Path;
using lyx::support::QuoteName;
using lyx::support::subst;
using lyx::support::Systemcall;

using std::string;
using std::distance;

extern LyXServerSocket * lyxsocket;

namespace {

string const token_from("$$i");
string const token_path("$$p");
string const token_socket("$$a");


class FormatNamesEqual : public std::unary_function<Format, bool> {
public:
	FormatNamesEqual(string const & name)
		: name_(name) {}
	bool operator()(Format const & f) const
	{
		return f.name() == name_;
	}
private:
	string name_;
};

} //namespace anon

bool operator<(Format const & a, Format const & b)
{
	// use the compare_ascii_no_case instead of compare_no_case,
	// because in turkish, 'i' is not the lowercase version of 'I',
	// and thus turkish locale breaks parsing of tags.

	return compare_ascii_no_case(a.prettyname(), b.prettyname()) < 0;
}

Format::Format(string const & n, string const & e, string const & p,
	       string const & s, string const & v)
	: name_(n), extension_(e), prettyname_(p),shortcut_(s), viewer_(v)
{}


bool Format::dummy() const
{
	return extension().empty();
}


bool Format::isChildFormat() const
{
	if (name_.empty())
		return false;
	return isdigit(name_[name_.length() - 1]);
}


string const Format::parentFormat() const
{
	return name_.substr(0, name_.length() - 1);
}


// This method should return a reference, and throw an exception
// if the format named name cannot be found (Lgb)
Format const * Formats::getFormat(string const & name) const
{
	FormatList::const_iterator cit =
		find_if(formatlist.begin(), formatlist.end(),
			FormatNamesEqual(name));
	if (cit != formatlist.end())
		return &(*cit);
	else
		return 0;
}


int Formats::getNumber(string const & name) const
{
	FormatList::const_iterator cit =
		find_if(formatlist.begin(), formatlist.end(),
			FormatNamesEqual(name));
	if (cit != formatlist.end())
		return distance(formatlist.begin(), cit);
	else
		return -1;
}


void Formats::add(string const & name)
{
	if (!getFormat(name))
		add(name, name, name, string());
}


void Formats::add(string const & name, string const & extension,
		  string const & prettyname, string const & shortcut)
{
	FormatList::iterator it =
		find_if(formatlist.begin(), formatlist.end(),
			FormatNamesEqual(name));
	if (it == formatlist.end())
		formatlist.push_back(Format(name, extension, prettyname,
					    shortcut, ""));
	else {
		string viewer = it->viewer();
		*it = Format(name, extension, prettyname, shortcut, viewer);
	}
}


void Formats::erase(string const & name)
{
	FormatList::iterator it =
		find_if(formatlist.begin(), formatlist.end(),
			FormatNamesEqual(name));
	if (it != formatlist.end())
		formatlist.erase(it);
}


void Formats::sort()
{
	std::sort(formatlist.begin(), formatlist.end());
}


void Formats::setViewer(string const & name, string const & command)
{
	add(name);
	FormatList::iterator it =
		find_if(formatlist.begin(), formatlist.end(),
			FormatNamesEqual(name));
	if (it != formatlist.end())
		it->setViewer(command);
}


bool Formats::view(Buffer const & buffer, string const & filename,
		   string const & format_name) const
{
	if (filename.empty())
		return false;

	Format const * format = getFormat(format_name);
	if (format && format->viewer().empty() &&
	    format->isChildFormat())
		format = getFormat(format->parentFormat());
	if (!format || format->viewer().empty()) {
// I believe this is the wrong place to show alerts, it should be done by
// the caller (this should be "utility" code
		Alert::error(_("Cannot view file"),
			bformat(_("No information for viewing %1$s"),
				prettyName(format_name)));
		return false;
	}

	string command = format->viewer();

	if (format_name == "dvi" &&
	    !lyxrc.view_dvi_paper_option.empty()) {
		command += ' ' + lyxrc.view_dvi_paper_option;
		string paper_size = buffer.params().paperSizeName();
		if (paper_size == "letter")
			paper_size = "us";
		command += ' ' + paper_size;
		if (buffer.params().orientation == ORIENTATION_LANDSCAPE)
			command += 'r';
	}

	if (!contains(command, token_from))
		command += ' ' + token_from;

	command = subst(command, token_from,
			QuoteName(OnlyFilename(filename)));
	command = subst(command, token_path, QuoteName(OnlyPath(filename)));
	command = subst(command, token_socket, QuoteName(lyxsocket->address()));
	lyxerr[Debug::FILES] << "Executing command: " << command << std::endl;
	buffer.message(_("Executing command: ") + command);

	Path p(OnlyPath(filename));
	Systemcall one;
	int const res = one.startscript(Systemcall::DontWait, command);

	if (res) {
		Alert::error(_("Cannot view file"),
			     bformat(_("An error occurred whilst running %1$s"),
			       command.substr(0, 50)));
		return false;
	}
	return true;
}


string const Formats::prettyName(string const & name) const
{
	Format const * format = getFormat(name);
	if (format)
		return format->prettyname();
	else
		return name;
}


string const Formats::extension(string const & name) const
{
	Format const * format = getFormat(name);
	if (format)
		return format->extension();
	else
		return name;
}




Formats formats;

Formats system_formats;
