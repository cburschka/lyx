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
#include "support/mutex.h"
#include "support/docstream.h"
#include "support/os.h"
#include "support/PathChanger.h"
#include "support/Systemcall.h"
#include "support/textutils.h"
#include "support/Translator.h"

#include <algorithm>
#include <map>
#include <ctime>

// FIXME: Q_OS_MAC is not available, it's in Qt
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
		: name_(name)
	{}
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
		: extension_(extension)
	{}
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
		: mime_(mime)
	{}
	bool operator()(Format const & f) const
	{
		// The test for empty mime strings is needed since we allow
		// formats with empty mime types.
		return f.mime() == mime_ && !mime_.empty();
	}
private:
	string mime_;
};


} // namespace

bool Format::formatSorter(Format const * lhs, Format const * rhs)
{
	return compare_locale(translateIfPossible(lhs->prettyname()),
	                      translateIfPossible(rhs->prettyname())) < 0;
}

bool operator<(Format const & a, Format const & b)
{
	return compare_locale(translateIfPossible(a.prettyname()),
	                      translateIfPossible(b.prettyname())) < 0;
}


Format::Format(string const & n, string const & e, docstring const & p,
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
		find_if(formatlist_.begin(), formatlist_.end(),
			FormatNamesEqual(name));
	if (cit != formatlist_.end())
		return &(*cit);
	else
		return 0;
}


namespace {

/** Guess the file format name (as in Format::name()) from contents.
 *  Normally you don't want to use this directly, but rather
 *  Formats::getFormatFromFile().
 */
string guessFormatFromContents(FileName const & fn)
{
	// the different filetypes and what they contain in one of the first lines
	// (dots are any characters).		(Herbert 20020131)
	// AGR	Grace...
	// BMP	BM...
	// EPS	%!PS-Adobe-3.0 EPSF...
	// FIG	#FIG...
	// FITS ...BITPIX...
	// GIF	GIF...
	// JPG	\377\330...     (0xFFD8)
	// PDF	%PDF-...
	// PNG	.PNG...
	// PBM	P1... or P4	(B/W)
	// PGM	P2... or P5	(Grayscale)
	// PPM	P3... or P6	(color)
	// PS	%!PS-Adobe-2.0 or 1.0,  no "EPSF"!
	// SGI	\001\332...	(decimal 474)
	// TGIF	%TGIF...
	// TIFF	II... or MM...
	// XBM	..._bits[]...
	// XPM	/* XPM */    sometimes missing (f.ex. tgif-export)
	//      ...static char *...
	// XWD	\000\000\000\151	(0x00006900) decimal 105
	//
	// GZIP	\037\213	http://www.ietf.org/rfc/rfc1952.txt
	// ZIP	PK...			http://www.halyava.ru/document/ind_arch.htm
	// Z	\037\235		UNIX compress

	// paranoia check
	if (fn.empty() || !fn.isReadableFile())
		return string();

	ifstream ifs(fn.toFilesystemEncoding().c_str());
	if (!ifs)
		// Couldn't open file...
		return string();

	// gnuzip
	static string const gzipStamp = "\037\213";

	// PKZIP
	static string const zipStamp = "PK";

	// ZIP containers (koffice, openoffice.org etc).
	static string const nonzipStamp = "\010\0\0\0mimetypeapplication/";

	// compress
	static string const compressStamp = "\037\235";

	// DOS binary EPS according to Adobe TN-5002
	static string const binEPSStamp = "\xC5\xD0\xD3\xC6";


	// Maximum strings to read
	int const max_count = 50;
	int count = 0;

	string str;
	string format;
	bool firstLine = true;
	bool backslash = false;
	bool maybelatex = false;
	int dollars = 0;
	while ((count++ < max_count) && format.empty() && !maybelatex) {
		if (ifs.eof())
			break;

		getline(ifs, str);
		string const stamp = str.substr(0, 2);
		if (firstLine && str.size() >= 2) {
			// at first we check for a zipped file, because this
			// information is saved in the first bytes of the file!
			// also some graphic formats which save the information
			// in the first line, too.
			if (prefixIs(str, gzipStamp)) {
				format =  "gzip";

			} else if (stamp == zipStamp &&
			           !contains(str, nonzipStamp)) {
				format =  "zip";

			} else if (stamp == compressStamp) {
				format =  "compress";

			// the graphics part
			} else if (stamp == "BM") {
				format =  "bmp";

			} else if (stamp == "\377\330") {
				format =  "jpg";

			} else if (stamp == "\001\332") {
				format =  "sgi";
			} else if (prefixIs(str, binEPSStamp)) {
				format =  "eps";

			// PBM family
			// Don't need to use str.at(0), str.at(1) because
			// we already know that str.size() >= 2
			} else if (str[0] == 'P') {
				switch (str[1]) {
				case '1':
				case '4':
					format =  "pbm";
				    break;
				case '2':
				case '5':
					format =  "pgm";
				    break;
				case '3':
				case '6':
					format =  "ppm";
				}
				break;

			} else if ((stamp == "II") || (stamp == "MM")) {
				format =  "tiff";

			} else if (prefixIs(str,"%TGIF")) {
				format =  "tgif";

			} else if (prefixIs(str,"#FIG")) {
				format =  "fig";

			} else if (prefixIs(str,"GIF")) {
				format =  "gif";

			} else if (str.size() > 3) {
				int const c = ((str[0] << 24) & (str[1] << 16) &
					       (str[2] << 8)  & str[3]);
				if (c == 105) {
					format =  "xwd";
				}
			}

			firstLine = false;
		}

		if (!format.empty())
		    break;
		else if (contains(str,"EPSF"))
			// dummy, if we have wrong file description like
			// %!PS-Adobe-2.0EPSF"
			format = "eps";

		else if (contains(str, "Grace"))
			format = "agr";

		else if (contains(str, "%PDF"))
			// autodetect pdf format for graphics inclusion
			format = "pdf6";

		else if (contains(str, "PNG"))
			format = "png";

		else if (contains(str, "%!PS-Adobe")) {
			// eps or ps
			ifs >> str;
			if (contains(str,"EPSF"))
				format = "eps";
			else
			    format = "ps";
		}

		else if (contains(str, "_bits[]"))
			format = "xbm";

		else if (contains(str, "XPM") || contains(str, "static char *"))
			format = "xpm";

		else if (contains(str, "BITPIX"))
			format = "fits";

		else if (contains(str, "\\documentclass") ||
		         contains(str, "\\chapter") ||
		         contains(str, "\\section") ||
		         contains(str, "\\begin") ||
		         contains(str, "\\end") ||
		         contains(str, "$$") ||
		         contains(str, "\\[") ||
		         contains(str, "\\]"))
			maybelatex = true;
		else {
			if (contains(str, '\\'))
				backslash = true;
			dollars += count_char(str, '$');
			if (backslash && dollars > 1)
				// inline equation
				maybelatex = true;
		}
	}

	if (format.empty() && maybelatex && !isBinaryFile(fn))
		format = "latex";

	if (format.empty()) {
		if (ifs.eof())
			LYXERR(Debug::GRAPHICS, "filetools(getFormatFromContents)\n"
			       "\tFile type not recognised before EOF!");
	} else {
		LYXERR(Debug::GRAPHICS, "Recognised Fileformat: " << format);
		return format;
	}

	LYXERR(Debug::GRAPHICS, "filetools(getFormatFromContents)\n"
		<< "\tCouldn't find a known format!");
	return string();
}

} // namespace


string Formats::getFormatFromFile(FileName const & filename) const
{
	if (filename.empty())
		return string();

	string psformat;
	string format;
#ifdef HAVE_MAGIC_H
	if (filename.exists()) {
		magic_t magic_cookie = magic_open(MAGIC_MIME);
		if (magic_cookie) {
			if (magic_load(magic_cookie, NULL) != 0) {
				LYXERR(Debug::GRAPHICS, "Formats::getFormatFromFile\n"
					<< "\tCouldn't load magic database - "
					<< magic_error(magic_cookie));
			} else {
				char const * result = magic_file(magic_cookie,
					filename.toFilesystemEncoding().c_str());
				string mime;
				if (result)
					mime = token(result, ';', 0);
				else {
					LYXERR(Debug::GRAPHICS, "Formats::getFormatFromFile\n"
						<< "\tCouldn't query magic database - "
						<< magic_error(magic_cookie));
				}
				// our own detection is better for binary files (can be anything)
				// and different plain text formats
				if (!mime.empty() && mime != "application/octet-stream" &&
				    mime != "text/plain") {
					Formats::const_iterator cit =
						find_if(formatlist_.begin(), formatlist_.end(),
							FormatMimeEqual(mime));
					if (cit != formatlist_.end()) {
						LYXERR(Debug::GRAPHICS, "\tgot format from MIME type: "
							<< mime << " -> " << cit->name());
						// See special eps/ps handling below
						if (mime == "application/postscript")
							psformat = cit->name();
						else
							format = cit->name();
					}
				}
			}
			magic_close(magic_cookie);
			// libmagic recognizes as latex also some formats of ours
			// such as pstex and pdftex. Therefore we have to perform
			// additional checks in this case (bug 9244).
			if (!format.empty() && format != "latex")
				return format;
		}
	}
#endif

	string const ext = getExtension(filename.absFileName());
	if (format.empty()) {
		// libmagic does not distinguish eps and ps.
		// Therefore we need to use our own detection here, but only if it
		// recognizes either ps or eps. Otherwise the libmagic guess will
		// be better (bug 9146).
		format = guessFormatFromContents(filename);
		if (!psformat.empty()) {
			if (isPostScriptFileFormat(format))
				return format;
			else
				return psformat;
		}

		if (isZippedFileFormat(format) && !ext.empty()) {
			string const & fmt_name = getFormatFromExtension(ext);
			if (!fmt_name.empty()) {
				Format const * p_format = getFormat(fmt_name);
				if (p_format && p_format->zippedNative())
					return p_format->name();
			}
		}
		// Don't simply return latex (bug 9244).
		if (!format.empty() && format != "latex")
			return format;
	}

	// Both libmagic and our guessing from contents may return as latex
	// also lyx files and our pstex and pdftex formats. In this case we
	// give precedence to the format determined by the extension.
	if (format == "latex") {
		format = getFormatFromExtension(ext);
		return format.empty() ? "latex" : format;
	}

	// try to find a format from the file extension.
	return getFormatFromExtension(ext);
}


string Formats::getFormatFromExtension(string const & ext) const
{
	if (!ext.empty()) {
		// this is ambigous if two formats have the same extension,
		// but better than nothing
		Formats::const_iterator cit =
			find_if(formatlist_.begin(), formatlist_.end(),
				FormatExtensionsEqual(ext));
		if (cit != formatlist_.end()) {
			LYXERR(Debug::GRAPHICS, "\twill guess format from file extension: "
				<< ext << " -> " << cit->name());
			return cit->name();
		}
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
static Mutex zipped_mutex;


bool Formats::isZippedFile(support::FileName const & filename) const {
	string const & fname = filename.absFileName();
	time_t timestamp = filename.lastModified();
	Mutex::Locker lock(&zipped_mutex);
	map<string, ZippedInfo>::iterator it = zipped_.find(fname);
	if (it != zipped_.end() && it->second.timestamp == timestamp)
		return it->second.zipped;
	// FIXME perf: This very expensive function is called on startup on each
	// file whic is going to be parsed, and also on svgz icons. Maybe there is a
	// quicker way to check whether a file is zipped?  I.e. for icons we
	// probably just need to check the extension (svgz vs svg).
	string const & format = getFormatFromFile(filename);
	bool zipped = (format == "gzip" || format == "zip");
	zipped_.insert(make_pair(fname, ZippedInfo(zipped, timestamp)));
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
	FormatList::iterator fit = formatlist_.begin();
	FormatList::iterator const fend = formatlist_.end();
	for ( ; fit != fend ; ++fit) {
		fit->setViewer(fixCommand(fit->viewer(), fit->extension(), os::VIEW));
		fit->setEditor(fixCommand(fit->editor(), fit->extension(), os::EDIT));
	}
}


int Formats::getNumber(string const & name) const
{
	FormatList::const_iterator cit =
		find_if(formatlist_.begin(), formatlist_.end(),
			FormatNamesEqual(name));
	if (cit != formatlist_.end())
		return distance(formatlist_.begin(), cit);
	else
		return -1;
}


void Formats::add(string const & name)
{
	if (!getFormat(name))
		add(name, name, from_utf8(name), string(), string(), string(),
		    string(), Format::document);
}


void Formats::add(string const & name, string const & extensions,
		  docstring const & prettyname, string const & shortcut,
		  string const & viewer, string const & editor,
		  string const & mime, int flags)
{
	FormatList::iterator it =
		find_if(formatlist_.begin(), formatlist_.end(),
			FormatNamesEqual(name));
	if (it == formatlist_.end())
		formatlist_.push_back(Format(name, extensions, prettyname,
					    shortcut, viewer, editor, mime, flags));
	else
		*it = Format(name, extensions, prettyname, shortcut, viewer,
			     editor, mime, flags);
}


void Formats::erase(string const & name)
{
	FormatList::iterator it =
		find_if(formatlist_.begin(), formatlist_.end(),
			FormatNamesEqual(name));
	if (it != formatlist_.end())
		formatlist_.erase(it);
}


void Formats::sort()
{
	std::sort(formatlist_.begin(), formatlist_.end());
}


void Formats::setViewer(string const & name, string const & command)
{
	add(name);
	FormatList::iterator it =
		find_if(formatlist_.begin(), formatlist_.end(),
			FormatNamesEqual(name));
	if (it != formatlist_.end())
		it->setViewer(command);
}


void Formats::setEditor(string const & name, string const & command)
{
	add(name);
	FormatList::iterator it =
		find_if(formatlist_.begin(), formatlist_.end(),
			FormatNamesEqual(name));
	if (it != formatlist_.end())
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

	string command = format->viewer();

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

	command = subst(command, token_from_format,
	                quoteName(onlyFileName(filename.toFilesystemEncoding()), quote_shell_filename));
	command = subst(command, token_path_format,
	                quoteName(onlyPath(filename.toFilesystemEncoding()), quote_shell_filename));
	command = subst(command, token_socket_format, quoteName(theServerSocket().address()));
	LYXERR(Debug::FILES, "Executing command: " << command);
	// FIXME UNICODE utf8 can be wrong for files
	buffer.message(_("Executing command: ") + from_utf8(command));

	PathChanger p(filename.onlyPath());
	Systemcall one;
	one.startscript(Systemcall::DontWait, command,
	                buffer.filePath(), buffer.layoutPos());

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
	if (format_name == "pdf6" && ext == "linkback") {
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

	command = subst(command, token_from_format,
	                quoteName(filename.toFilesystemEncoding(), quote_shell_filename));
	command = subst(command, token_path_format,
	                quoteName(onlyPath(filename.toFilesystemEncoding()), quote_shell_filename));
	command = subst(command, token_socket_format, quoteName(theServerSocket().address()));
	LYXERR(Debug::FILES, "Executing command: " << command);
	// FIXME UNICODE utf8 can be wrong for files
	buffer.message(_("Executing command: ") + from_utf8(command));

	Systemcall one;
	one.startscript(Systemcall::DontWait, command,
	                buffer.filePath(), buffer.layoutPos());

	// we can't report any sort of error, since we aren't waiting
	return true;
}


docstring const Formats::prettyName(string const & name) const
{
	Format const * format = getFormat(name);
	if (format)
		return format->prettyname();
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
	f.addPair(OutputParams::LYX, "lyx");
	return f;
}


FlavorTranslator const & flavorTranslator()
{
	static FlavorTranslator const translator = initFlavorTranslator();
	return translator;
}

} // namespace


std::string flavor2format(OutputParams::FLAVOR flavor)
{
	return flavorTranslator().find(flavor);
}


/* Not currently needed, but I'll leave the code in case it is.
OutputParams::FLAVOR format2flavor(std::string fmt)
{
	return flavorTranslator().find(fmt);
} */

} // namespace lyx
