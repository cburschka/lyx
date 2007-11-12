/**
 * \file Format.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Format.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "LyXRC.h"
#include "debug.h"
#include "gettext.h"
#include "ServerSocket.h"

#include "frontends/alert.h" //to be removed?

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Systemcall.h"

using std::find_if;
using std::string;
using std::distance;


namespace lyx {

using support::absolutePath;
using support::bformat;
using support::compare_ascii_no_case;
using support::contains;
using support::FileName;
using support::libScriptSearch;
using support::makeDisplayPath;
using support::onlyPath;
using support::quoteName;
using support::subst;
using support::Systemcall;
using support::token;

namespace Alert = frontend::Alert;
namespace os = support::os;

namespace {

string const token_from_format("$$i");
string const token_path_format("$$p");
string const token_socket_format("$$a");


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


class FormatExtensionsEqual : public std::unary_function<Format, bool> {
public:
	FormatExtensionsEqual(string const & extension)
		: extension_(extension) {}
	bool operator()(Format const & f) const
	{
		return f.extension() == extension_;
	}
private:
	string extension_;
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
	       string const & s, string const & v, string const & ed,
	       int flags)
	: name_(n), extension_(e), prettyname_(p), shortcut_(s), viewer_(v),
	  editor_(ed), flags_(flags)
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


string Formats::getFormatFromFile(FileName const & filename) const
{
	if (filename.empty())
		return string();

	string const format = filename.guessFormatFromContents();
	if (!format.empty())
		return format;

	// try to find a format from the file extension.
	string const ext = support::getExtension(filename.absFilename());
	if (!ext.empty()) {
		// this is ambigous if two formats have the same extension,
		// but better than nothing
		Formats::const_iterator cit =
			find_if(formatlist.begin(), formatlist.end(),
				FormatExtensionsEqual(ext));
		if (cit != formats.end()) {
			LYXERR(Debug::GRAPHICS)
				<< "\twill guess format from file extension: "
				<< ext << " -> " << cit->name() << std::endl;
			return cit->name();
		}
	}
	return string();
}


static string fixCommand(string const & cmd, string const & ext,
		  os::auto_open_mode mode)
{
	// configure.py says we do not want a viewer/editor
	if (cmd.empty())
		return cmd;

	// Does the OS manage this format?
	if (os::canAutoOpenFile(ext, mode))
		return "auto";

	// if configure.py found nothing, clear the command
	if (token(cmd, ' ', 0) == "auto")
		return string();

	// use the command found by configure.py
	return cmd;
}


void Formats::setAutoOpen()
{
	FormatList::iterator fit = formatlist.begin();
	FormatList::iterator const fend = formatlist.end();
	for ( ; fit != fend ; ++fit) {
		fit->setViewer(fixCommand(fit->viewer(), fit->extension(), os::VIEW));
		fit->setEditor(fixCommand(fit->editor(), fit->extension(), os::EDIT));
	}
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
		add(name, name, name, string(), string(), string(),
		    Format::document);
}


void Formats::add(string const & name, string const & extension,
		  string const & prettyname, string const & shortcut,
		  string const & viewer, string const & editor,
		  int flags)
{
	FormatList::iterator it =
		find_if(formatlist.begin(), formatlist.end(),
			FormatNamesEqual(name));
	if (it == formatlist.end())
		formatlist.push_back(Format(name, extension, prettyname,
					    shortcut, viewer, editor, flags));
	else
		*it = Format(name, extension, prettyname, shortcut, viewer,
			     editor, flags);
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


bool Formats::view(Buffer const & buffer, FileName const & filename,
		   string const & format_name) const
{
	if (filename.empty() || !filename.exists()) {
		Alert::error(_("Cannot view file"),
			bformat(_("File does not exist: %1$s"),
				from_utf8(filename.absFilename())));
		return false;
	}

	Format const * format = getFormat(format_name);
	if (format && format->viewer().empty() &&
	    format->isChildFormat())
		format = getFormat(format->parentFormat());
	if (!format || format->viewer().empty()) {
// FIXME: I believe this is the wrong place to show alerts, it should be done
// by the caller (this should be "utility" code)
		Alert::error(_("Cannot view file"),
			bformat(_("No information for viewing %1$s"),
				prettyName(format_name)));
		return false;
	}
	// viewer is 'auto'
	if (format->viewer() == "auto") {
		if (os::autoOpenFile(filename.absFilename(), os::VIEW))
			return true;
		else {
			Alert::error(_("Cannot view file"),
				bformat(_("Auto-view file %1$s failed"),
					from_utf8(filename.absFilename())));
			return false;
		}
	}

	string command = libScriptSearch(format->viewer());

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

	if (!contains(command, token_from_format))
		command += ' ' + token_from_format;

	command = subst(command, token_from_format, quoteName(filename.toFilesystemEncoding()));
	command = subst(command, token_path_format, quoteName(onlyPath(filename.toFilesystemEncoding())));
	command = subst(command, token_socket_format, quoteName(theServerSocket().address()));
	LYXERR(Debug::FILES) << "Executing command: " << command << std::endl;
	// FIXME UNICODE utf8 can be wrong for files
	buffer.message(_("Executing command: ") + from_utf8(command));

	Systemcall one;
	int const res = one.startscript(Systemcall::DontWait, command);

	if (res) {
		Alert::error(_("Cannot view file"),
			     bformat(_("An error occurred whilst running %1$s"),
			       makeDisplayPath(command, 50)));
		return false;
	}
	return true;
}


bool Formats::edit(Buffer const & buffer, FileName const & filename,
			 string const & format_name) const
{
	if (filename.empty() || !filename.exists()) {
		Alert::error(_("Cannot edit file"),
			bformat(_("File does not exist: %1$s"),
				from_utf8(filename.absFilename())));
		return false;
	}

	Format const * format = getFormat(format_name);
	if (format && format->editor().empty() &&
	    format->isChildFormat())
		format = getFormat(format->parentFormat());
	if (!format || format->editor().empty()) {
// FIXME: I believe this is the wrong place to show alerts, it should
// be done by the caller (this should be "utility" code)
		Alert::error(_("Cannot edit file"),
			bformat(_("No information for editing %1$s"),
				prettyName(format_name)));
		return false;
	}
	// editor is 'auto'
	if (format->editor() == "auto") {
		if (os::autoOpenFile(filename.absFilename(), os::EDIT))
			return true;
		else {
			Alert::error(_("Cannot edit file"),
				bformat(_("Auto-edit file %1$s failed"),
					from_utf8(filename.absFilename())));
			return false;
		}
	}

	string command = format->editor();

	if (!contains(command, token_from_format))
		command += ' ' + token_from_format;

	command = subst(command, token_from_format, quoteName(filename.toFilesystemEncoding()));
	command = subst(command, token_path_format, quoteName(onlyPath(filename.toFilesystemEncoding())));
	command = subst(command, token_socket_format, quoteName(theServerSocket().address()));
	LYXERR(Debug::FILES) << "Executing command: " << command << std::endl;
	// FIXME UNICODE utf8 can be wrong for files
	buffer.message(_("Executing command: ") + from_utf8(command));

	Systemcall one;
	int const res = one.startscript(Systemcall::DontWait, command);

	if (res) {
		Alert::error(_("Cannot edit file"),
			     bformat(_("An error occurred whilst running %1$s"),
			       makeDisplayPath(command, 50)));
		return false;
	}
	return true;
}


docstring const Formats::prettyName(string const & name) const
{
	Format const * format = getFormat(name);
	if (format)
		return from_utf8(format->prettyname());
	else
		return from_utf8(name);
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


} // namespace lyx
