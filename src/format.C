/**
 * \file format.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#include "format.h"
#include "lyxrc.h"
#include "debug.h"
#include "lyx_cb.h" // for ShowMessage() ... to be removed?
#include "gettext.h"
#include "LString.h"

#include "frontends/Alert.h" //to be removed?

#include "support/filetools.h"
#include "support/path.h"
#include "support/systemcall.h"
#include "support/lyxfunctional.h"


namespace {

string const token_from("$$i");
string const token_path("$$p");

} //namespace anon

bool operator<(Format const & a, Format const & b)
{
	// use the compare_ascii_no_case instead of compare_no_case,
	// because in turkish, 'i' is not the lowercase version of 'I',
	// and thus turkish locale breaks parsing of tags.

	return compare_ascii_no_case(a.prettyname(), b.prettyname()) < 0;
}

Format::Format(string const & n, string const & e, string const & p,
       string const & s, string const & v): name_(n), 
					    extension_(e), 
					    prettyname_(p), 
					    shortcut_(s),
					    viewer_(v) 
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
			lyx::compare_memfun(&Format::name, name));
	if (cit != formatlist.end())
		return &(*cit);
	else
		return 0;
}


int Formats::getNumber(string const & name) const
{
	FormatList::const_iterator cit =
		find_if(formatlist.begin(), formatlist.end(),
			lyx::compare_memfun(&Format::name, name));
	if (cit != formatlist.end())
		return cit - formatlist.begin();
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
			lyx::compare_memfun(&Format::name, name));
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
			lyx::compare_memfun(&Format::name, name));
	if (it != formatlist.end())
		formatlist.erase(it);
}


void Formats::sort()
{
	cout << "here" << endl;
	std::sort(formatlist.begin(), formatlist.end());
	cout << "and here " << formatlist.size() << endl;
}


void Formats::setViewer(string const & name, string const & command)
{
	add(name);
	FormatList::iterator it =
		find_if(formatlist.begin(), formatlist.end(),
			lyx::compare_memfun(&Format::name, name));
	if (it != formatlist.end())
		it->setViewer(command);
}


bool Formats::view(Buffer const * buffer, string const & filename,
		   string const & format_name) const
{
	if (filename.empty())
		return false;

	Format const * format = getFormat(format_name);
	if (format && format->viewer().empty() &&
	    format->isChildFormat())
		format = getFormat(format->parentFormat());
	if (!format || format->viewer().empty()) {
#if USE_BOOST_FORMAT
		Alert::alert(_("Cannot view file"),
			     boost::io::str(boost::format(_("No information for viewing %1$s"))
			   % prettyName(format_name)));
#else
		Alert::alert(_("Cannot view file"),
			     _("No information for viewing ")
			     + prettyName(format_name));
#endif
			   return false;
	}

	string command = format->viewer();

	if (format_name == "dvi" &&
	    !lyxrc.view_dvi_paper_option.empty()) {
		command += ' ' + lyxrc.view_dvi_paper_option;
		string paper_size = papersize(buffer);
		if (paper_size == "letter")
			paper_size = "us";
		command += ' ' + paper_size;
		if (buffer->params.orientation
		    == BufferParams::ORIENTATION_LANDSCAPE)
			command += 'r';
	}

	if (!contains(command, token_from))
		command += ' ' + token_from;

	command = subst(command, token_from,
			QuoteName(OnlyFilename(filename)));
	command = subst(command, token_path, QuoteName(OnlyPath(filename)));

	lyxerr[Debug::FILES] << "Executing command: " << command << endl;
	ShowMessage(buffer, _("Executing command:"), command);

	Path p(OnlyPath(filename));
	Systemcall one;
	int const res = one.startscript(Systemcall::DontWait, command);

	if (res) {
		Alert::alert(_("Cannot view file"),
			   _("Error while executing"),
			   command.substr(0, 50));
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


string const papersize(Buffer const * buffer)
{
	char real_papersize = buffer->params.papersize;
	if (real_papersize == BufferParams::PAPER_DEFAULT)
		real_papersize = lyxrc.default_papersize;

	switch (real_papersize) {
	case BufferParams::PAPER_A3PAPER:
		return "a3";
	case BufferParams::PAPER_A4PAPER:
		return "a4";
	case BufferParams::PAPER_A5PAPER:
		return "a5";
	case BufferParams::PAPER_B5PAPER:
		return "b5";
	case BufferParams::PAPER_EXECUTIVEPAPER:
		return "foolscap";
	case BufferParams::PAPER_LEGALPAPER:
		return "legal";
	case BufferParams::PAPER_USLETTER:
	default:
		return "letter";
	}
}


Formats formats;

Formats system_formats;
