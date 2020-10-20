/**
 * \file Converter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Converter.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "ConverterCache.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Format.h"
#include "InsetList.h"
#include "Language.h"
#include "LaTeX.h"
#include "LyXRC.h"
#include "Mover.h"
#include "ParagraphList.h"
#include "Session.h"

#include "frontends/alert.h"

#include "insets/InsetInclude.h"

#include "support/debug.h"
#include "support/FileNameList.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/PathChanger.h"
#include "support/Systemcall.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = lyx::frontend::Alert;


namespace {

string const token_from("$$i");
string const token_base("$$b");
string const token_to("$$o");
string const token_path("$$p");
string const token_orig_path("$$r");
string const token_orig_from("$$f");
string const token_encoding("$$e");
string const token_latex_encoding("$$E");


string const add_options(string const & command, string const & options)
{
	string head;
	string const tail = split(command, head, ' ');
	return head + ' ' + options + ' ' + tail;
}


string const dvipdfm_options(BufferParams const & bp)
{
	string result;

	if (bp.papersize != PAPER_CUSTOM) {
		string const paper_size = bp.paperSizeName(BufferParams::DVIPDFM);
		if (!paper_size.empty())
			result = "-p "+ paper_size;

		if (bp.orientation == ORIENTATION_LANDSCAPE)
			result += " -l";
	}

	return result;
}


class ConverterEqual {
public:
	ConverterEqual(string const & from, string const & to)
		: from_(from), to_(to) {}
	bool operator()(Converter const & c) const {
		return c.from() == from_ && c.to() == to_;
	}
private:
	string const from_;
	string const to_;
};

} // namespace


Converter::Converter(string const & f, string const & t,
		     string const & c, string const & l)
	: from_(f), to_(t), command_(c), flags_(l),
	  From_(nullptr), To_(nullptr), latex_(false), docbook_(false),
	  need_aux_(false), nice_(false), need_auth_(false)
{}


void Converter::readFlags()
{
	string flag_list(flags_);
	while (!flag_list.empty()) {
		string flag_name, flag_value;
		flag_list = split(flag_list, flag_value, ',');
		flag_value = split(flag_value, flag_name, '=');
		if (flag_name == "latex") {
			latex_ = true;
			latex_flavor_ = flag_value.empty() ?
				"latex" : flag_value;
		} else if (flag_name == "xml")
			docbook_ = true;
		else if (flag_name == "needaux") {
			need_aux_ = true;
			latex_flavor_ = flag_value.empty() ?
				"latex" : flag_value;
		} else if (flag_name == "resultdir")
			result_dir_ = (flag_value.empty())
				? token_base : flag_value;
		else if (flag_name == "resultfile")
			result_file_ = flag_value;
		else if (flag_name == "parselog")
			parselog_ = flag_value;
		else if (flag_name == "nice")
			nice_ = true;
		else if (flag_name == "needauth")
			need_auth_ = true;
		else if (flag_name == "hyperref-driver")
			href_driver_ = flag_value;
	}
	if (!result_dir_.empty() && result_file_.empty())
		result_file_ = "index." + theFormats().extension(to_);
	//if (!contains(command, token_from))
	//	latex = true;
}


Converter const * Converters::getConverter(string const & from,
					    string const & to) const
{
	ConverterList::const_iterator const cit =
		find_if(converterlist_.begin(), converterlist_.end(),
			ConverterEqual(from, to));
	if (cit != converterlist_.end())
		return &(*cit);
	else
		return nullptr;
}


int Converters::getNumber(string const & from, string const & to) const
{
	ConverterList::const_iterator const cit =
		find_if(converterlist_.begin(), converterlist_.end(),
			ConverterEqual(from, to));
	if (cit != converterlist_.end())
		return distance(converterlist_.begin(), cit);
	else
		return -1;
}


void Converters::add(string const & from, string const & to,
		     string const & command, string const & flags)
{
	theFormats().add(from);
	theFormats().add(to);
	ConverterList::iterator it = find_if(converterlist_.begin(),
					     converterlist_.end(),
					     ConverterEqual(from , to));

	Converter converter(from, to, command, flags);
	if (it != converterlist_.end() && !flags.empty() && flags[0] == '*') {
		converter = *it;
		converter.setCommand(command);
		converter.setFlags(flags);
	}
	converter.readFlags();

	// The latex_command is used to update the .aux file when running
	// a converter that uses it.
	if (converter.latex()) {
		if (latex_command_.empty() ||
		    converter.latex_flavor() == "latex")
			latex_command_ = subst(command, token_from, "");
		if (dvilualatex_command_.empty() ||
		    converter.latex_flavor() == "dvilualatex")
			dvilualatex_command_ = subst(command, token_from, "");
		if (lualatex_command_.empty() ||
		    converter.latex_flavor() == "lualatex")
			lualatex_command_ = subst(command, token_from, "");
		if (pdflatex_command_.empty() ||
		    converter.latex_flavor() == "pdflatex")
			pdflatex_command_ = subst(command, token_from, "");
		if (xelatex_command_.empty() ||
		    converter.latex_flavor() == "xelatex")
			xelatex_command_ = subst(command, token_from, "");
	}

	if (it == converterlist_.end()) {
		converterlist_.push_back(converter);
	} else {
		converter.setFrom(it->From());
		converter.setTo(it->To());
		*it = converter;
	}
}


void Converters::erase(string const & from, string const & to)
{
	ConverterList::iterator const it =
		find_if(converterlist_.begin(),
			converterlist_.end(),
			ConverterEqual(from, to));
	if (it != converterlist_.end())
		converterlist_.erase(it);
}


// This method updates the pointers From and To in all the converters.
// The code is not very efficient, but it doesn't matter as the number
// of formats and converters is small.
// Furthermore, this method is called only on startup, or after
// adding/deleting a format in FormPreferences (the latter calls can be
// eliminated if the formats in the Formats class are stored using a map or
// a list (instead of a vector), but this will cause other problems).
void Converters::update(Formats const & formats)
{
	for (auto & cv : converterlist_) {
		cv.setFrom(formats.getFormat(cv.from()));
		cv.setTo(formats.getFormat(cv.to()));
	}
}


// This method updates the pointers From and To in the last converter.
// It is called when adding a new converter in FormPreferences
void Converters::updateLast(Formats const & formats)
{
	if (converterlist_.begin() != converterlist_.end()) {
		ConverterList::iterator it = converterlist_.end() - 1;
		it->setFrom(formats.getFormat(it->from()));
		it->setTo(formats.getFormat(it->to()));
	}
}


OutputParams::FLAVOR Converters::getFlavor(Graph::EdgePath const & path,
					   Buffer const * buffer)
{
	for (auto const & edge : path) {
		Converter const & conv = converterlist_[edge];
		if (conv.latex() || conv.need_aux()) {
			if (conv.latex_flavor() == "latex")
				return OutputParams::LATEX;
			if (conv.latex_flavor() == "xelatex")
				return OutputParams::XETEX;
			if (conv.latex_flavor() == "lualatex")
				return OutputParams::LUATEX;
			if (conv.latex_flavor() == "dvilualatex")
				return OutputParams::DVILUATEX;
			if (conv.latex_flavor() == "pdflatex")
				return OutputParams::PDFLATEX;
		}
		if (conv.docbook())
			return OutputParams::DOCBOOK5;
	}
	return buffer ? buffer->params().getOutputFlavor()
		      : OutputParams::LATEX;
}


string Converters::getHyperrefDriver(Graph::EdgePath const & path)
{
	for (auto const & edge : path) {
		Converter const & conv = converterlist_[edge];
		if (!conv.hyperref_driver().empty())
			return conv.hyperref_driver();
	}
	return string();
}


bool Converters::checkAuth(Converter const & conv, string const & doc_fname,
			   bool use_shell_escape)
{
	string conv_command = conv.command();
	bool const has_shell_escape = contains(conv_command, "-shell-escape")
				|| contains(conv_command, "-enable-write18");
	if (conv.latex() && has_shell_escape && !use_shell_escape) {
		docstring const shellescape_warning =
		      bformat(_("<p>The following LaTeX backend has been "
		        "configured to allow execution of external programs "
		        "for any document:</p>"
		        "<center><p><tt>%1$s</tt></p></center>"
		        "<p>This is a dangerous configuration. Please, "
		        "consider using the support offered by LyX for "
		        "allowing this privilege only to documents that "
			"actually need it, instead.</p>"),
		        from_utf8(conv_command));
		frontend::Alert::error(_("Security Warning"),
					shellescape_warning , false);
	} else if (!conv.latex())
		use_shell_escape = false;
	if (!conv.need_auth() && !use_shell_escape)
		return true;
	size_t const token_pos = conv_command.find("$$");
	bool const has_token = token_pos != string::npos;
	string const command = use_shell_escape && !has_shell_escape
		? (has_token ? conv_command.insert(token_pos, "-shell-escape ")
			     : conv_command.append(" -shell-escape"))
		: conv_command;
	docstring const security_warning = (use_shell_escape
	    ? bformat(_("<p>The following LaTeX backend has been requested "
	        "to allow execution of external programs:</p>"
	        "<center><p><tt>%1$s</tt></p></center>"
	        "<p>The external programs can execute arbitrary commands on "
	        "your system, including dangerous ones, if instructed to do "
	        "so by a maliciously crafted LyX document.</p>"),
	      from_utf8(command))
	    : bformat(_("<p>The requested operation requires the use of a "
	        "converter from %2$s to %3$s:</p>"
	        "<blockquote><p><tt>%1$s</tt></p></blockquote>"
	        "<p>This external program can execute arbitrary commands on "
	        "your system, including dangerous ones, if instructed to do "
	        "so by a maliciously crafted LyX document.</p>"),
	      from_utf8(command), from_utf8(conv.from()),
	      from_utf8(conv.to())));
	if (lyxrc.use_converter_needauth_forbidden && !use_shell_escape) {
		frontend::Alert::error(
		    _("An external converter is disabled for security reasons"),
		    security_warning + _(
		    "<p><b>Your current preference settings forbid its execution.</b></p>"
		    "<p>(To change this setting, go to <i>Preferences &#x25b9; File "
		    "Handling &#x25b9; Converters</i> and uncheck <i>Security &#x25b9; "
		    "Forbid needauth converters</i>.)"), false);
		return false;
	}
	if (!lyxrc.use_converter_needauth && !use_shell_escape)
		return true;
	docstring const security_title = use_shell_escape
		? _("A LaTeX backend requires your authorization")
		: _("An external converter requires your authorization");
	int choice;
	docstring const security_warning2 = security_warning + (use_shell_escape
		? _("<p>Should LaTeX backends be allowed to run external "
		    "programs?</p><p><b>Allow them only if you trust the "
		    "origin/sender of the LyX document!</b></p>")
		: _("<p>Would you like to run this converter?</p>"
		    "<p><b>Only run if you trust the origin/sender of the LyX "
		    "document!</b></p>"));
	docstring const no = use_shell_escape
				? _("Do &not allow") : _("Do &not run");
	docstring const yes = use_shell_escape ? _("A&llow") : _("&Run");
	docstring const always = use_shell_escape
					? _("&Always allow for this document")
					: _("&Always run for this document");
	if (!doc_fname.empty()) {
		LYXERR(Debug::FILES, "looking up: " << doc_fname);
		bool authorized = use_shell_escape
			? theSession().shellescapeFiles().findAuth(doc_fname)
			: theSession().authFiles().find(doc_fname);
		if (!authorized) {
			choice = frontend::Alert::prompt(security_title,
							 security_warning2,
							 0, 0, no, yes, always);
			if (choice == 2) {
				if (use_shell_escape)
					theSession().shellescapeFiles().insert(doc_fname, true);
				else
					theSession().authFiles().insert(doc_fname);
			}
		} else {
			choice = 1;
		}
	} else {
		choice = frontend::Alert::prompt(security_title,
						 security_warning2,
						 0, 0, no, yes);
	}
	return choice != 0;
}


Converters::RetVal Converters::convert(Buffer const * buffer,
			 FileName const & from_file, FileName const & to_file,
			 FileName const & orig_from,
			 string const & from_format, string const & to_format,
			 ErrorList & errorList, int conversionflags, bool includeall)
{
	if (from_format == to_format)
		return move(from_format, from_file, to_file, false) ?
		      SUCCESS : FAILURE;

	if ((conversionflags & try_cache) &&
	    ConverterCache::get().inCache(orig_from, to_format))
		return ConverterCache::get().copy(orig_from, to_format, to_file) ?
		      SUCCESS : FAILURE;

	Graph::EdgePath edgepath = getPath(from_format, to_format);
	if (edgepath.empty()) {
		if (conversionflags & try_default) {
			// if no special converter defined, then we take the
			// default one from ImageMagic.
			string const from_ext = from_format.empty() ?
				getExtension(from_file.absFileName()) :
				theFormats().extension(from_format);
			string const to_ext = theFormats().extension(to_format);
			string const command =
				os::python() + ' ' +
				quoteName(libFileSearch("scripts", "convertDefault.py").toFilesystemEncoding()) +
				' ' + from_ext + ' ' +
				quoteName(from_file.toFilesystemEncoding()) +
				' ' + to_ext + ' ' +
				quoteName(to_file.toFilesystemEncoding());
			LYXERR(Debug::FILES, "No converter defined! "
				   "I use convertDefault.py:\n\t" << command);
			Systemcall one;
			Systemcall::Starttype starttype =
				(buffer && buffer->isClone()) ?
					Systemcall::WaitLoop : Systemcall::Wait;
			int const exitval = one.startscript(starttype, command,
					buffer ? buffer->filePath() : string(),
					buffer ? buffer->layoutPos() : string());
			if (exitval == Systemcall::KILLED) {
				frontend::Alert::warning(
					_("Converter killed"),
					bformat(_("The following converter was killed by the user.\n %1$s\n"),
						from_utf8(command)));
				return KILLED;
			}
			if (to_file.isReadableFile()) {
				if (conversionflags & try_cache)
					ConverterCache::get().add(orig_from,
							to_format, to_file);
				return SUCCESS;
			}
		}

		// only warn once per session and per file type
		static std::map<string, string> warned;
		if (warned.find(from_format) != warned.end() && warned.find(from_format)->second == to_format) {
			return FAILURE;
		}
		warned.insert(make_pair(from_format, to_format));

		Alert::error(_("Cannot convert file"),
			     bformat(_("No information for converting %1$s "
						    "format files to %2$s.\n"
						    "Define a converter in the preferences."),
							from_ascii(from_format), from_ascii(to_format)));
		return FAILURE;
	}

	// buffer is only invalid for importing, and then runparams is not
	// used anyway.
	OutputParams runparams(buffer ? &buffer->params().encoding() : nullptr);
	runparams.flavor = getFlavor(edgepath, buffer);

	if (buffer) {
		BufferParams const & bp = buffer->params();
		runparams.use_japanese =
			(bp.bufferFormat() == "latex"
			 || suffixIs(bp.bufferFormat(), "-ja"))
			&& bp.encoding().package() == Encoding::japanese;
		runparams.use_indices = bp.use_indices;
		runparams.bibtex_command = bp.bibtexCommand();
		runparams.index_command = (bp.index_command == "default") ?
			string() : bp.index_command;
		runparams.document_language = bp.language->lang();
		// Some macros rely on font encoding
		runparams.main_fontenc = bp.main_font_encoding();
		runparams.only_childbibs = !bp.useBiblatex()
				&& !bp.useBibtopic()
				&& bp.multibib == "child";
		runparams.includeall = includeall;
	}

	// Some converters (e.g. lilypond) can only output files to the
	// current directory, so we need to change the current directory.
	// This has the added benefit that all other files that may be
	// generated by the converter are deleted when LyX closes and do not
	// clutter the real working directory.
	// FIXME: This does not work if path is an UNC path on windows
	//        (bug 6127).
	string const path(onlyPath(from_file.absFileName()));
	// Prevent the compiler from optimizing away p
	FileName pp(path);
	PathChanger p(pp);

	// empty the error list before any new conversion takes place.
	errorList.clear();

	bool run_latex = false;
	string from_base = changeExtension(from_file.absFileName(), "");
	string to_base = changeExtension(to_file.absFileName(), "");
	FileName infile;
	FileName outfile = from_file;
	for (auto const & edge : edgepath) {
		Converter const & conv = converterlist_[edge];
		bool dummy = conv.To()->dummy() && conv.to() != "program";
		if (!dummy) {
			LYXERR(Debug::FILES, "Converting from  "
			       << conv.from() << " to " << conv.to());
		}
		infile = outfile;
		outfile = FileName(conv.result_file().empty()
			? changeExtension(from_file.absFileName(), conv.To()->extension())
			: addName(subst(conv.result_dir(),
					token_base, from_base),
				  subst(conv.result_file(),
					token_base, onlyFileName(from_base))));

		// if input and output files are equal, we use a
		// temporary file as intermediary (JMarc)
		FileName real_outfile;
		if (!conv.result_file().empty())
			real_outfile = FileName(changeExtension(from_file.absFileName(),
				conv.To()->extension()));
		if (outfile == infile) {
			real_outfile = infile;
			// when importing, a buffer does not necessarily exist
			if (buffer)
				outfile = FileName(addName(buffer->temppath(), "tmpfile.out"));
			else
				outfile = FileName(addName(package().temp_dir().absFileName(),
						   "tmpfile.out"));
		}

		if (buffer && buffer->params().use_minted
		    && lyxrc.pygmentize_command.empty() && conv.latex()) {
			bool dowarn = false;
			// Warn only if listings insets are actually used
			for (Paragraph const & par : buffer->paragraphs()) {
				InsetList const & insets = par.insetList();
				pos_type lstpos = insets.find(LISTINGS_CODE, 0);
				pos_type incpos = insets.find(INCLUDE_CODE, 0);
				if (incpos >= 0) {
					InsetInclude const * include =
						static_cast<InsetInclude *>
						        (insets.get(incpos));
					if (include->params().getCmdName() !=
					                        "inputminted") {
						incpos = -1;
					}
				}
				if (lstpos >= 0 || incpos >= 0) {
					dowarn = true;
					break;
				}
			}
			if (dowarn) {
				Alert::warning(_("Pygments driver command not found!"),
				    _("The driver command necessary to use the minted package\n"
				      "(pygmentize) has not been found. Make sure you have\n"
				      "the python-pygments module installed or, if the driver\n"
				      "is named differently, to add the following line to the\n"
				      "document preamble:\n\n"
				      "\\AtBeginDocument{\\renewcommand{\\MintedPygmentize}{driver}}\n\n"
				      "where 'driver' is name of the driver command."));
			}
		}

		if (!checkAuth(conv, buffer ? buffer->absFileName() : string(),
			       buffer && buffer->params().shell_escape))
			return FAILURE;

		if (conv.latex()) {
			// We are not importing, we have a buffer
			LASSERT(buffer, return FAILURE);
			run_latex = true;
			string command = conv.command();
			command = subst(command, token_from, "");
			command = subst(command, token_latex_encoding,
			                buffer->params().encoding().latexName());
			if (buffer->params().shell_escape
			    && !contains(command, "-shell-escape"))
				command += " -shell-escape ";
			LYXERR(Debug::FILES, "Running " << command);
			// FIXME KILLED
			// Check changed return value here.
			RetVal const retval = runLaTeX(*buffer, command, runparams, errorList);
			if (retval != SUCCESS)
				return retval;
		} else {
			if (conv.need_aux() && !run_latex) {
				// We are not importing, we have a buffer
				LASSERT(buffer, return FAILURE);
				string command;
				switch (runparams.flavor) {
				case OutputParams::DVILUATEX:
					command = dvilualatex_command_;
					break;
				case OutputParams::LUATEX:
					command = lualatex_command_;
					break;
				case OutputParams::PDFLATEX:
					command = pdflatex_command_;
					break;
				case OutputParams::XETEX:
					command = xelatex_command_;
					break;
				default:
					command = latex_command_;
					break;
				}
				if (!command.empty()) {
					LYXERR(Debug::FILES, "Running "
						<< command
						<< " to update aux file");
					// FIXME KILLED
					// Check changed return value here.
					RetVal const retval = runLaTeX(*buffer, command, runparams, errorList);
						if (retval != SUCCESS)
							return retval;
				}
			}

			// FIXME UNICODE
			string const infile2 =
				to_utf8(makeRelPath(from_utf8(infile.absFileName()), from_utf8(path)));
			string const outfile2 =
				to_utf8(makeRelPath(from_utf8(outfile.absFileName()), from_utf8(path)));

			string command = conv.command();
			command = subst(command, token_from, quoteName(infile2));
			command = subst(command, token_base, quoteName(from_base));
			command = subst(command, token_to, quoteName(outfile2));
			command = subst(command, token_path, quoteName(onlyPath(infile.absFileName())));
			command = subst(command, token_orig_path, quoteName(onlyPath(orig_from.absFileName())));
			command = subst(command, token_orig_from, quoteName(onlyFileName(orig_from.absFileName())));
			command = subst(command, token_encoding, buffer ? buffer->params().encoding().iconvName() : string());

			if (!conv.parselog().empty())
				command += " 2> " + quoteName(infile2 + ".out");

			// it is not actually not necessary to test for buffer here,
			// but it pleases coverity.
			if (buffer && conv.from() == "dvi" && conv.to() == "ps")
				command = add_options(command,
						      buffer->params().dvips_options());
			else if (buffer && conv.from() == "dvi" && prefixIs(conv.to(), "pdf"))
				command = add_options(command,
						      dvipdfm_options(buffer->params()));

			LYXERR(Debug::FILES, "Calling " << command);
			if (buffer)
				buffer->message(_("Executing command: ")
				+ from_utf8(command));

			Systemcall one;
			int res;
			if (dummy) {
				res = one.startscript(Systemcall::DontWait,
					to_filesystem8bit(from_utf8(command)),
					buffer ? buffer->filePath() : string(),
					buffer ? buffer->layoutPos() : string());
				// We're not waiting for the result, so we can't do anything
				// else here.
			} else {
				Systemcall::Starttype starttype =
						(buffer && buffer->isClone()) ?
							Systemcall::WaitLoop : Systemcall::Wait;
				res = one.startscript(starttype,
						to_filesystem8bit(from_utf8(command)),
						buffer ? buffer->filePath()
						       : string(),
						buffer ? buffer->layoutPos()
						       : string());
				if (res == Systemcall::KILLED) {
					frontend::Alert::warning(
						_("Converter killed"),
						bformat(_("The following converter was killed by the user.\n %1$s\n"), 
							from_utf8(command)));
					return KILLED;
				}
				
				if (!real_outfile.empty()) {
					Mover const & mover = getMover(conv.to());
					if (!mover.rename(outfile, real_outfile))
						res = -1;
					else
						LYXERR(Debug::FILES, "renaming file " << outfile
							<< " to " << real_outfile);
					// Finally, don't forget to tell any future
					// converters to use the renamed file...
					outfile = real_outfile;
				}

				if (!conv.parselog().empty()) {
					string const logfile =  infile2 + ".log";
					string const command2 = conv.parselog() +
						" < " + quoteName(infile2 + ".out") +
						" > " + quoteName(logfile);
					res = one.startscript(starttype,
						to_filesystem8bit(from_utf8(command2)),
						buffer ? buffer->filePath() : string(),
						buffer ? buffer->layoutPos() : string());
					if (res == Systemcall::KILLED) {
						frontend::Alert::warning(
							_("Converter killed"),
							bformat(_("The following converter was killed by the user.\n %1$s\n"), 
								from_utf8(command)));
						return KILLED;
					}
					if (buffer && !scanLog(*buffer, command, makeAbsPath(logfile, path), errorList))
						return FAILURE;
				}
			}

			if (res) {
				if (res == Systemcall::KILLED) {
					Alert::information(_("Process Killed"),
						bformat(_("The conversion process was killed while running:\n%1$s"),
							wrapParas(from_utf8(command))));
					return KILLED;
				} 
				if (res == Systemcall::TIMEOUT) {
					Alert::information(_("Process Timed Out"),
						bformat(_("The conversion process:\n%1$s\ntimed out before completing."),
							wrapParas(from_utf8(command))));
					return KILLED;
				} 
				if (conv.to() == "program") {
					Alert::error(_("Build errors"),
						_("There were errors during the build process."));
				} else {
// FIXME: this should go out of here. For example, here we cannot say if
// it is a document (.lyx) or something else. Same goes for elsewhere.
					Alert::error(_("Cannot convert file"),
						bformat(_("An error occurred while running:\n%1$s"),
						wrapParas(from_utf8(command))));
				}
				return FAILURE;
			}
		}
	}

	Converter const & conv = converterlist_[edgepath.back()];
	if (conv.To()->dummy())
		return SUCCESS;

	if (!conv.result_dir().empty()) {
		// The converter has put the file(s) in a directory.
		// In this case we ignore the given to_file.
		if (from_base != to_base) {
			string const from = subst(conv.result_dir(),
					    token_base, from_base);
			string const to = subst(conv.result_dir(),
					  token_base, to_base);
			Mover const & mover = getMover(conv.from());
			if (!mover.rename(FileName(from), FileName(to))) {
				Alert::error(_("Cannot convert file"),
					bformat(_("Could not move a temporary directory from %1$s to %2$s."),
						from_utf8(from), from_utf8(to)));
				return FAILURE;
			}
		}
		return SUCCESS;
	} else {
		if (conversionflags & try_cache)
			ConverterCache::get().add(orig_from, to_format, outfile);
		return move(conv.to(), outfile, to_file, conv.latex()) ? SUCCESS : FAILURE;
	}
}


bool Converters::move(string const & fmt,
		      FileName const & from, FileName const & to, bool copy)
{
	if (from == to)
		return true;

	bool no_errors = true;
	string const path = onlyPath(from.absFileName());
	string const base = onlyFileName(removeExtension(from.absFileName()));
	string const to_base = removeExtension(to.absFileName());
	string const to_extension = getExtension(to.absFileName());

	support::FileNameList const files = FileName(path).dirList(getExtension(from.absFileName()));
	for (auto const & f : files) {
		string const from2 = f.absFileName();
		string const file2 = onlyFileName(from2);
		if (prefixIs(file2, base)) {
			string const to2 = changeExtension(
				to_base + file2.substr(base.length()),
				to_extension);
			LYXERR(Debug::FILES, "moving " << from2 << " to " << to2);

			Mover const & mover = getMover(fmt);
			bool const moved = copy
				? mover.copy(f, FileName(to2))
				: mover.rename(f, FileName(to2));
			if (!moved && no_errors) {
				Alert::error(_("Cannot convert file"),
					bformat(copy ?
						_("Could not copy a temporary file from %1$s to %2$s.") :
						_("Could not move a temporary file from %1$s to %2$s."),
						from_utf8(from2), from_utf8(to2)));
				no_errors = false;
			}
		}
	}
	return no_errors;
}


bool Converters::formatIsUsed(string const & format)
{
	for (auto const & cvt : converterlist_) {
		if (cvt.from() == format || cvt.to() == format)
			return true;
	}
	return false;
}


bool Converters::scanLog(Buffer const & buffer, string const & /*command*/,
			 FileName const & filename, ErrorList & errorList)
{
	OutputParams runparams(nullptr);
	runparams.flavor = OutputParams::LATEX;
	LaTeX latex("", runparams, filename);
	TeXErrors terr;
	int const result = latex.scanLogFile(terr);

	if (result & LaTeX::ERRORS)
		buffer.bufferErrors(terr, errorList);

	return true;
}


Converters::RetVal Converters::runLaTeX(Buffer const & buffer, string const & command,
			  OutputParams const & runparams, ErrorList & errorList)
{
	buffer.setBusy(true);
	buffer.message(_("Running LaTeX..."));

	// do the LaTeX run(s)
	string const name = buffer.latexName();
	LaTeX latex(command, runparams, FileName(makeAbsPath(name)),
	            buffer.filePath(), buffer.layoutPos(),
	            buffer.isClone(), buffer.freshStartRequired());
	TeXErrors terr;
	// The connection closes itself at the end of the scope when latex is
	// destroyed. One cannot close (and destroy) buffer while the converter is
	// running.
	latex.message.connect([&buffer](docstring const & msg){
			buffer.message(msg);
		});
	int const result = latex.run(terr);

	if (result == Systemcall::KILLED || result == Systemcall::TIMEOUT) {
		Alert::error(_("Export canceled"),
			_("The export process was terminated by the user."));
		return KILLED;
	}

	if (result & LaTeX::ERRORS)
		buffer.bufferErrors(terr, errorList);

	if ((result & LaTeX::UNDEF_CIT) || (result & LaTeX::UNDEF_UNKNOWN_REF)) {
		buffer.bufferRefs(terr, errorList);
		if (errorList.empty())
			errorList.push_back(ErrorItem(_("Undefined reference"),
				_("Undefined references or citations were found during the build.\n"
				  "Please check the warnings in the LaTeX log (Document > LaTeX Log)."),
				&buffer));
	}

	if (!errorList.empty()) {
	  // We will show the LaTeX Errors GUI later which contains
	  // specific error messages so it would be repetitive to give
	  // e.g. the "finished with an error" dialog in addition.
	}
	else if (result & LaTeX::NO_LOGFILE) {
		docstring const str =
			bformat(_("LaTeX did not run successfully. "
					       "Additionally, LyX could not locate "
					       "the LaTeX log %1$s."), from_utf8(name));
		Alert::error(_("LaTeX failed"), str);
	} else if (result & LaTeX::NONZERO_ERROR) {
		docstring const str =
			bformat(_( "The external program\n%1$s\n"
			      "finished with an error. "
			      "It is recommended you fix the cause of the external "
			      "program's error (check the logs). "), from_utf8(command));
		Alert::error(_("LaTeX failed"), str);
	} else if (result & LaTeX::NO_OUTPUT) {
		Alert::warning(_("Output is empty"),
			       _("No output file was generated."));
	}

	buffer.setBusy(false);

	int const ERROR_MASK =
			LaTeX::NO_LOGFILE |
			LaTeX::ERRORS |
			LaTeX::UNDEF_CIT |
			LaTeX::UNDEF_UNKNOWN_REF |
			LaTeX::NO_OUTPUT;

	return (result & ERROR_MASK) == 0 ? SUCCESS : FAILURE;
}



void Converters::buildGraph()
{
	// clear graph's data structures
	G_.init(theFormats().size());
	// each of the converters knows how to convert one format to another
	// so, for each of them, we create an arrow on the graph, going from
	// the one to the other
	for (auto const & cvt : converterlist_) {
		int const from = theFormats().getNumber(cvt.from());
		int const to   = theFormats().getNumber(cvt.to());
		LASSERT(from >= 0, continue);
		LASSERT(to >= 0, continue);
		G_.addEdge(from, to);
	}
}


FormatList const Converters::intToFormat(vector<int> const & input)
{
	FormatList result(input.size());

	vector<int>::const_iterator it = input.begin();
	vector<int>::const_iterator const end = input.end();
	FormatList::iterator rit = result.begin();
	for ( ; it != end; ++it, ++rit) {
		*rit = &theFormats().get(*it);
	}
	return result;
}


FormatList const Converters::getReachableTo(string const & target,
		bool const clear_visited)
{
	vector<int> const & reachablesto =
		G_.getReachableTo(theFormats().getNumber(target), clear_visited);

	return intToFormat(reachablesto);
}


FormatList const Converters::getReachable(string const & from,
		bool const only_viewable, bool const clear_visited,
		set<string> const & excludes)
{
	set<int> excluded_numbers;

	for (auto const & ex : excludes)
		excluded_numbers.insert(theFormats().getNumber(ex));

	vector<int> const & reachables =
		G_.getReachable(theFormats().getNumber(from),
				only_viewable,
				clear_visited,
				excluded_numbers);

	return intToFormat(reachables);
}


bool Converters::isReachable(string const & from, string const & to)
{
	return G_.isReachable(theFormats().getNumber(from),
			      theFormats().getNumber(to));
}


Graph::EdgePath Converters::getPath(string const & from, string const & to)
{
	return G_.getPath(theFormats().getNumber(from),
			  theFormats().getNumber(to));
}


FormatList Converters::importableFormats()
{
	vector<string> l = loaders();
	FormatList result = getReachableTo(l[0], true);
	vector<string>::const_iterator it = l.begin() + 1;
	vector<string>::const_iterator en = l.end();
	for (; it != en; ++it) {
		FormatList r = getReachableTo(*it, false);
		result.insert(result.end(), r.begin(), r.end());
	}
	return result;
}


FormatList Converters::exportableFormats(bool only_viewable)
{
	vector<string> s = savers();
	FormatList result = getReachable(s[0], only_viewable, true);
	vector<string>::const_iterator it = s.begin() + 1;
	vector<string>::const_iterator en = s.end();
	for (; it != en; ++it) {
		 FormatList r = getReachable(*it, only_viewable, false);
		result.insert(result.end(), r.begin(), r.end());
	}
	return result;
}


vector<string> Converters::loaders() const
{
	vector<string> v;
	v.push_back("lyx");
	v.push_back("text");
	v.push_back("textparagraph");
	return v;
}


vector<string> Converters::savers() const
{
	vector<string> v;
	v.push_back("docbook");
	v.push_back("latex");
	v.push_back("literate");
	v.push_back("luatex");
	v.push_back("dviluatex");
	v.push_back("lyx");
	v.push_back("xhtml");
	v.push_back("pdflatex");
	v.push_back("platex");
	v.push_back("text");
	v.push_back("xetex");
	return v;
}


} // namespace lyx
