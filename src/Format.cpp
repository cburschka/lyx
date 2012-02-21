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
#include "ServerSocket.h"

#include "frontends/alert.h" //to be removed?

#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Path.h"
#include "support/Systemcall.h"
#include "support/textutils.h"
#include "support/Translator.h"

#include <algorithm>
#include <map>
#include <ctime>

// FIXME: Q_WS_MACX is not available, it's in Qt
#ifdef USE_MACOSX_PACKAGING
#include "support/linkback/LinkBackProxy.h"
#endif

#ifdef HAVE_MAGIC_H
#include <magic.h>
#endif

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;
namespace os = support::os;

namespace {

string const token_from_format("$$i");
string const token_path_format("$$p");
string const token_socket_format("$$a");


class FormatNamesEqual : public unary_function<Format, bool> {
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


class FormatExtensionsEqual : public unary_function<Format, bool> {
public:
	FormatExtensionsEqual(string const & extension)
		: extension_(extension) {}
	bool operator()(Format const & f) const
	{
		return f.hasExtension(extension_);
	}
private:
	string extension_;
};


class FormatMimeEqual : public unary_function<Format, bool> {
public:
	FormatMimeEqual(string const & mime)
		: mime_(mime) {}
	bool operator()(Format const & f) const
	{
		// The test for empty mime strings is needed since we allow
		// formats with empty mime types.
		return f.mime() == mime_ && !mime_.empty();
	}
private:
	string mime_;
};


class FormatPrettyNameEqual : public unary_function<Format, bool> {
public:
	FormatPrettyNameEqual(string const & prettyname)
		: prettyname_(prettyname) {}
	bool operator()(Format const & f) const
	{
		return f.prettyname() == prettyname_;
	}
private:
	string prettyname_;
};

} //namespace anon


bool operator<(Format const & a, Format const & b)
{
	return _(a.prettyname()) < _(b.prettyname());
}


Format::Format(string const & n, string const & e, string const & p,
	       string const & s, string const & v, string const & ed,
	       string const & m, int flags)
	: name_(n), prettyname_(p), shortcut_(s), viewer_(v),
	  editor_(ed), mime_(m), flags_(flags)
{
	extension_list_ = getVectorFromString(e, ",");
	LYXERR(Debug::GRAPHICS, "New Format: n=" << n << ", flags=" << flags);
}


bool Format::dummy() const
{
	return extension().empty();
}


string const Format::extensions() const
{
	return getStringFromVector(extension_list_, ", ");
}


bool Format::hasExtension(string const & e) const
{
	return (find(extension_list_.begin(), extension_list_.end(), e)
		!= extension_list_.end());
}


bool Format::isChildFormat() const
{
	if (name_.empty())
		return false;
	return isDigitASCII(name_[name_.length() - 1]);
}


string const Format::parentFormat() const
{
	return name_.substr(0, name_.length() - 1);
}


void Format::setExtensions(string const & e)
{
	extension_list_ = getVectorFromString(e, ",");
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

#ifdef HAVE_MAGIC_H
	magic_t magic_cookie = magic_open(MAGIC_MIME);
	if (magic_cookie) {
		string format;
		if (magic_load(magic_cookie, NULL) != 0) {
			LYXERR(Debug::GRAPHICS, "Formats::getFormatFromFile\n"
				<< "\tCouldn't load magic database - "
				<< magic_error(magic_cookie));
		} else {
			string mime = magic_file(magic_cookie,
				filename.toFilesystemEncoding().c_str());
			mime = token(mime, ';', 0);
			// we need our own ps/eps detection
			if (mime != "application/postscript") {
				Formats::const_iterator cit =
					find_if(formatlist.begin(), formatlist.end(),
					        FormatMimeEqual(mime));
				if (cit != formats.end()) {
					LYXERR(Debug::GRAPHICS, "\tgot format from MIME type: "
						<< mime << " -> " << cit->name());
					format = cit->name();
				}
			}
		}
		magic_close(magic_cookie);
		if (!format.empty())
			return format;
	}
#endif

	string const format = filename.guessFormatFromContents();
	string const ext = getExtension(filename.absFileName());
	if (isZippedFileFormat(format) && !ext.empty()) {
		string const & fmt_name = formats.getFormatFromExtension(ext);
		if (!fmt_name.empty()) {
			Format const * p_format = formats.getFormat(fmt_name);
			if (p_format && p_format->zippedNative())
				return p_format->name();
		}
	}
	if (!format.empty())
		return format;

	// try to find a format from the file extension.
	return getFormatFromExtension(ext);
}


string Formats::getFormatFromExtension(string const & ext) const
{
	if (!ext.empty()) {
		// this is ambigous if two formats have the same extension,
		// but better than nothing
		Formats::const_iterator cit =
			find_if(formatlist.begin(), formatlist.end(),
				FormatExtensionsEqual(ext));
		if (cit != formats.end()) {
			LYXERR(Debug::GRAPHICS, "\twill guess format from file extension: "
				<< ext << " -> " << cit->name());
			return cit->name();
		}
	}
	return string();
}


string Formats::getFormatFromPrettyName(string const & prettyname) const
{
	if (!prettyname.empty()) {
		Formats::const_iterator cit =
			find_if(formatlist.begin(), formatlist.end(),
				FormatPrettyNameEqual(prettyname));
		if (cit != formats.end())
			return cit->name();
	}
	return string();
}


/// Used to store last timestamp of file and whether it is (was) zipped
struct ZippedInfo {
	bool zipped;
	std::time_t timestamp;
	ZippedInfo(bool zipped, std::time_t timestamp)
	: zipped(zipped), timestamp(timestamp) { }
};


/// Mapping absolute pathnames of files to their ZippedInfo metadata.
static std::map<std::string, ZippedInfo> zipped_;


bool Formats::isZippedFile(support::FileName const & filename) const {
	string const & fname = filename.absFileName();
	time_t timestamp = filename.lastModified();
	map<string, ZippedInfo>::iterator it = zipped_.find(fname);
	if (it != zipped_.end() && it->second.timestamp == timestamp)
		return it->second.zipped;
	string const & format = getFormatFromFile(filename);
	bool zipped = (format == "gzip" || format == "zip");
	zipped_.insert(pair<string, ZippedInfo>(fname, ZippedInfo(zipped, timestamp)));
	return zipped;
}


bool Formats::isZippedFileFormat(string const & format)
{
	return contains("gzip zip compress", format) && !format.empty();
}


bool Formats::isPostScriptFileFormat(string const & format)
{
	return format == "ps" || format == "eps";
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
		    string(), Format::document);
}


void Formats::add(string const & name, string const & extensions,
		  string const & prettyname, string const & shortcut,
		  string const & viewer, string const & editor,
		  string const & mime, int flags)
{
	FormatList::iterator it =
		find_if(formatlist.begin(), formatlist.end(),
			FormatNamesEqual(name));
	if (it == formatlist.end())
		formatlist.push_back(Format(name, extensions, prettyname,
					    shortcut, viewer, editor, mime, flags));
	else
		*it = Format(name, extensions, prettyname, shortcut, viewer,
			     editor, mime, flags);
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


void Formats::setEditor(string const & name, string const & command)
{
	add(name);
	FormatList::iterator it =
		find_if(formatlist.begin(), formatlist.end(),
			FormatNamesEqual(name));
	if (it != formatlist.end())
		it->setEditor(command);
}


bool Formats::view(Buffer const & buffer, FileName const & filename,
		   string const & format_name) const
{
	if (filename.empty() || !filename.exists()) {
		Alert::error(_("Cannot view file"),
			bformat(_("File does not exist: %1$s"),
				from_utf8(filename.absFileName())));
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
		if (os::autoOpenFile(filename.absFileName(), os::VIEW, buffer.filePath()))
			return true;
		else {
			Alert::error(_("Cannot view file"),
				bformat(_("Auto-view file %1$s failed"),
					from_utf8(filename.absFileName())));
			return false;
		}
	}

	string command = libScriptSearch(format->viewer());

	if (format_name == "dvi" &&
	    !lyxrc.view_dvi_paper_option.empty()) {
		string paper_size = buffer.params().paperSizeName(BufferParams::XDVI);
		if (!paper_size.empty()) {
			command += ' ' + lyxrc.view_dvi_paper_option;
			command += ' ' + paper_size;
			if (buffer.params().orientation == ORIENTATION_LANDSCAPE &&
			    buffer.params().papersize != PAPER_CUSTOM)
				command += 'r';
		}
	}

	if (!contains(command, token_from_format))
		command += ' ' + token_from_format;

	command = subst(command, token_from_format, quoteName(onlyFileName(filename.toFilesystemEncoding())));
	command = subst(command, token_path_format, quoteName(onlyPath(filename.toFilesystemEncoding())));
	command = subst(command, token_socket_format, quoteName(theServerSocket().address()));
	LYXERR(Debug::FILES, "Executing command: " << command);
	// FIXME UNICODE utf8 can be wrong for files
	buffer.message(_("Executing command: ") + from_utf8(command));

	PathChanger p(filename.onlyPath());
	Systemcall one;
	one.startscript(Systemcall::DontWait, command, buffer.filePath());

	// we can't report any sort of error, since we aren't waiting
	return true;
}


bool Formats::edit(Buffer const & buffer, FileName const & filename,
			 string const & format_name) const
{
	if (filename.empty() || !filename.exists()) {
		Alert::error(_("Cannot edit file"),
			bformat(_("File does not exist: %1$s"),
				from_utf8(filename.absFileName())));
		return false;
	}

	// LinkBack files look like PDF, but have the .linkback extension
	string const ext = getExtension(filename.absFileName());
	if (format_name == "pdf" && ext == "linkback") {
#ifdef USE_MACOSX_PACKAGING
		return editLinkBackFile(filename.absFileName().c_str());
#else
		Alert::error(_("Cannot edit file"),
			     _("LinkBack files can only be edited on Apple Mac OSX."));
		return false;
#endif // USE_MACOSX_PACKAGING
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
		if (os::autoOpenFile(filename.absFileName(), os::EDIT, buffer.filePath()))
			return true;
		else {
			Alert::error(_("Cannot edit file"),
				bformat(_("Auto-edit file %1$s failed"),
					from_utf8(filename.absFileName())));
			return false;
		}
	}

	string command = format->editor();

	if (!contains(command, token_from_format))
		command += ' ' + token_from_format;

	command = subst(command, token_from_format, quoteName(filename.toFilesystemEncoding()));
	command = subst(command, token_path_format, quoteName(onlyPath(filename.toFilesystemEncoding())));
	command = subst(command, token_socket_format, quoteName(theServerSocket().address()));
	LYXERR(Debug::FILES, "Executing command: " << command);
	// FIXME UNICODE utf8 can be wrong for files
	buffer.message(_("Executing command: ") + from_utf8(command));

	Systemcall one;
	one.startscript(Systemcall::DontWait, command, buffer.filePath());

	// we can't report any sort of error, since we aren't waiting
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


string const Formats::extensions(string const & name) const
{
	Format const * format = getFormat(name);
	if (format)
		return format->extensions();
	else
		return name;
}


namespace {
typedef Translator<OutputParams::FLAVOR, string> FlavorTranslator;

FlavorTranslator initFlavorTranslator()
{
	FlavorTranslator f(OutputParams::LATEX, "latex");
	f.addPair(OutputParams::DVILUATEX, "dviluatex");
	f.addPair(OutputParams::LUATEX, "luatex");
	f.addPair(OutputParams::PDFLATEX, "pdflatex");
	f.addPair(OutputParams::XETEX, "xetex");
	f.addPair(OutputParams::XML, "docbook-xml");
	f.addPair(OutputParams::HTML, "xhtml");
	f.addPair(OutputParams::TEXT, "text");
	return f;
}


FlavorTranslator const & flavorTranslator()
{
	static FlavorTranslator translator = initFlavorTranslator();
	return translator;
}
}


std::string flavor2format(OutputParams::FLAVOR flavor)
{
	return flavorTranslator().find(flavor);
}


/* Not currently needed, but I'll leave the code in case it is.
OutputParams::FLAVOR format2flavor(std::string fmt)
{
	return flavorTranslator().find(fmt);
} */

Formats formats;

Formats system_formats;


} // namespace lyx
