/**
 * \file buffer.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "buffer.h"

#include "author.h"
#include "BranchList.h"
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "bufferparams.h"
#include "counters.h"
#include "Bullet.h"
#include "Chktex.h"
#include "debug.h"
#include "encoding.h"
#include "errorlist.h"
#include "exporter.h"
#include "format.h"
#include "funcrequest.h"
#include "gettext.h"
#include "insetiterator.h"
#include "language.h"
#include "LaTeX.h"
#include "LaTeXFeatures.h"
#include "LyXAction.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "lyxrc.h"
#include "lyxvc.h"
#include "lyx_main.h"
#include "messages.h"
#include "output.h"
#include "output_docbook.h"
#include "output_latex.h"
#include "output_linuxdoc.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "pariterator.h"
#include "sgml.h"
#include "texrow.h"
#include "undo.h"
#include "version.h"

#include "insets/insetbibitem.h"
#include "insets/insetbibtex.h"
#include "insets/insetinclude.h"
#include "insets/insettext.h"

#include "mathed/math_macrotemplate.h"
#include "mathed/math_macrotable.h"
#include "mathed/math_support.h"

#include "frontends/Alert.h"

#include "graphics/Previews.h"

#include "support/filetools.h"
#include "support/fs_extras.h"
#ifdef USE_COMPRESSION
# include "support/gzstream.h"
#endif
#include "support/lyxlib.h"
#include "support/os.h"
#include "support/path.h"
#include "support/textutils.h"
#include "support/convert.h"

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

#if defined (HAVE_UTIME_H)
# include <utime.h>
#elif defined (HAVE_SYS_UTIME_H)
# include <sys/utime.h>
#endif

#include <iomanip>
#include <stack>
#include <sstream>
#include <fstream>


using lyx::pos_type;
using lyx::pit_type;

using lyx::support::AddName;
using lyx::support::bformat;
using lyx::support::ChangeExtension;
using lyx::support::cmd_ret;
using lyx::support::createBufferTmpDir;
using lyx::support::destroyDir;
using lyx::support::getFormatFromContents;
using lyx::support::IsDirWriteable;
using lyx::support::LibFileSearch;
using lyx::support::latex_path;
using lyx::support::ltrim;
using lyx::support::MakeAbsPath;
using lyx::support::MakeDisplayPath;
using lyx::support::MakeLatexName;
using lyx::support::OnlyFilename;
using lyx::support::OnlyPath;
using lyx::support::Path;
using lyx::support::QuoteName;
using lyx::support::removeAutosaveFile;
using lyx::support::rename;
using lyx::support::RunCommand;
using lyx::support::split;
using lyx::support::subst;
using lyx::support::tempName;
using lyx::support::trim;

namespace os = lyx::support::os;
namespace fs = boost::filesystem;

using std::endl;
using std::for_each;
using std::make_pair;

using std::ifstream;
using std::ios;
using std::map;
using std::ostream;
using std::ostringstream;
using std::ofstream;
using std::pair;
using std::stack;
using std::vector;
using std::string;


// all these externs should eventually be removed.
extern BufferList bufferlist;

namespace {

int const LYX_FORMAT = 242;

} // namespace anon


typedef std::map<string, bool> DepClean;

class Buffer::Impl
{
public:
	Impl(Buffer & parent, string const & file, bool readonly);

	limited_stack<Undo> undostack;
	limited_stack<Undo> redostack;
	BufferParams params;
	LyXVC lyxvc;
	string temppath;
	TexRow texrow;

	/// need to regenerate .tex?
	DepClean dep_clean;

	/// is save needed?
	mutable bool lyx_clean;

	/// is autosave needed?
	mutable bool bak_clean;

	/// is this a unnamed file (New...)?
	bool unnamed;

	/// buffer is r/o
	bool read_only;

	/// name of the file the buffer is associated with.
	string filename;

	/// The path to the document file.
	string filepath;

	boost::scoped_ptr<Messages> messages;

	/** Set to true only when the file is fully loaded.
	 *  Used to prevent the premature generation of previews
	 *  and by the citation inset.
	 */
	bool file_fully_loaded;

	/// our LyXText that should be wrapped in an InsetText
	InsetText inset;

	///
	MacroTable macros;
};


Buffer::Impl::Impl(Buffer & parent, string const & file, bool readonly_)
	: lyx_clean(true), bak_clean(true), unnamed(false), read_only(readonly_),
	  filename(file), filepath(OnlyPath(file)), file_fully_loaded(false),
		inset(params)
{
	inset.setAutoBreakRows(true);
	lyxvc.buffer(&parent);
	temppath = createBufferTmpDir();
	// FIXME: And now do something if temppath == string(), because we
	// assume from now on that temppath points to a valid temp dir.
	// See http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg67406.html
}


Buffer::Buffer(string const & file, bool readonly)
	: pimpl_(new Impl(*this, file, readonly))
{
	lyxerr[Debug::INFO] << "Buffer::Buffer()" << endl;
}


Buffer::~Buffer()
{
	lyxerr[Debug::INFO] << "Buffer::~Buffer()" << endl;
	// here the buffer should take care that it is
	// saved properly, before it goes into the void.

	closing();

	if (!temppath().empty() && !destroyDir(temppath())) {
		Alert::warning(_("Could not remove temporary directory"),
			bformat(_("Could not remove the temporary directory %1$s"), temppath()));
	}

	// Remove any previewed LaTeX snippets associated with this buffer.
	lyx::graphics::Previews::get().removeLoader(*this);
}


LyXText & Buffer::text() const
{
	return const_cast<LyXText &>(pimpl_->inset.text_);
}


InsetBase & Buffer::inset() const
{
	return const_cast<InsetText &>(pimpl_->inset);
}


limited_stack<Undo> & Buffer::undostack()
{
	return pimpl_->undostack;
}


limited_stack<Undo> const & Buffer::undostack() const
{
	return pimpl_->undostack;
}


limited_stack<Undo> & Buffer::redostack()
{
	return pimpl_->redostack;
}


limited_stack<Undo> const & Buffer::redostack() const
{
	return pimpl_->redostack;
}


BufferParams & Buffer::params()
{
	return pimpl_->params;
}


BufferParams const & Buffer::params() const
{
	return pimpl_->params;
}


ParagraphList & Buffer::paragraphs()
{
	return text().paragraphs();
}


ParagraphList const & Buffer::paragraphs() const
{
	return text().paragraphs();
}


LyXVC & Buffer::lyxvc()
{
	return pimpl_->lyxvc;
}


LyXVC const & Buffer::lyxvc() const
{
	return pimpl_->lyxvc;
}


string const & Buffer::temppath() const
{
	return pimpl_->temppath;
}


TexRow & Buffer::texrow()
{
	return pimpl_->texrow;
}


TexRow const & Buffer::texrow() const
{
	return pimpl_->texrow;
}


string const Buffer::getLatexName(bool const no_path) const
{
	string const name = ChangeExtension(MakeLatexName(fileName()), ".tex");
	return no_path ? OnlyFilename(name) : name;
}


pair<Buffer::LogType, string> const Buffer::getLogName() const
{
	string const filename = getLatexName(false);

	if (filename.empty())
		return make_pair(Buffer::latexlog, string());

	string const path = temppath();

	string const fname = AddName(path,
				     OnlyFilename(ChangeExtension(filename,
								  ".log")));
	string const bname =
		AddName(path, OnlyFilename(
			ChangeExtension(filename,
					formats.extension("literate") + ".out")));

	// If no Latex log or Build log is newer, show Build log

	if (fs::exists(bname) &&
	    (!fs::exists(fname) || fs::last_write_time(fname) < fs::last_write_time(bname))) {
		lyxerr[Debug::FILES] << "Log name calculated as: " << bname << endl;
		return make_pair(Buffer::buildlog, bname);
	}
	lyxerr[Debug::FILES] << "Log name calculated as: " << fname << endl;
	return make_pair(Buffer::latexlog, fname);
}


void Buffer::setReadonly(bool const flag)
{
	if (pimpl_->read_only != flag) {
		pimpl_->read_only = flag;
		readonly(flag);
	}
}


void Buffer::setFileName(string const & newfile)
{
	pimpl_->filename = MakeAbsPath(newfile);
	pimpl_->filepath = OnlyPath(pimpl_->filename);
	setReadonly(fs::is_readonly(pimpl_->filename));
	updateTitles();
}


// We'll remove this later. (Lgb)
namespace {

void unknownClass(string const & unknown)
{
	Alert::warning(_("Unknown document class"),
		bformat(_("Using the default document class, because the "
			"class %1$s is unknown."), unknown));
}

} // anon


int Buffer::readHeader(LyXLex & lex)
{
	int unknown_tokens = 0;
	int line = -1;
	int begin_header_line = -1;

	// Initialize parameters that may be/go lacking in header:
	params().branchlist().clear();
	params().options.erase();
	params().float_placement.erase();
	params().paperwidth.erase();
	params().paperheight.erase();
	params().leftmargin.erase();
	params().rightmargin.erase();
	params().topmargin.erase();
	params().bottommargin.erase();
	params().headheight.erase();
	params().headsep.erase();
	params().footskip.erase();

	while (lex.isOK()) {
		lex.next();
		string const token = lex.getString();

		if (token.empty())
			continue;

		if (token == "\\end_header")
			break;

		++line;
		if (token == "\\begin_header") {
			begin_header_line = line;
			continue;
		}

		lyxerr[Debug::PARSER] << "Handling header token: `"
				      << token << '\'' << endl;

		string unknown = params().readToken(lex, token);
		if (!unknown.empty()) {
			if (unknown[0] != '\\' && token == "\\textclass") {
				unknownClass(unknown);
			} else {
				++unknown_tokens;
				string const s = bformat(_("Unknown token: "
							   "%1$s %2$s\n"),
							 token,
							 lex.getString());
				error(ErrorItem(_("Header error"), s,
						-1, 0, 0));
			}
		}
	}
	if (begin_header_line) {
		string const s = _("\\begin_header is missing");
		error(ErrorItem(_("Header error"), s, -1, 0, 0));
	}
	return unknown_tokens;
}


// Uwe C. Schroeder
// changed to be public and have one parameter
// Returns false if "\end_document" is not read (Asger)
bool Buffer::readDocument(LyXLex & lex)
{
	lex.next();
	string const token = lex.getString();
	if (token != "\\begin_document") {
		string const s = _("\\begin_document is missing");
		error(ErrorItem(_("Header error"), s, -1, 0, 0));
	}

	if (paragraphs().empty()) {
		readHeader(lex);
		if (!params().getLyXTextClass().load()) {
			string theclass = params().getLyXTextClass().name();
			Alert::error(_("Can't load document class"), bformat(
					"Using the default document class, because the "
					" class %1$s could not be loaded.", theclass));
			params().textclass = 0;
		}
	} else {
		// We don't want to adopt the parameters from the
		// document we insert, so read them into a temporary buffer
		// and then discard it

		Buffer tmpbuf("", false);
		tmpbuf.readHeader(lex);
	}

	return text().read(*this, lex);
}


// needed to insert the selection
void Buffer::insertStringAsLines(ParagraphList & pars,
	pit_type & pit, pos_type & pos,
	LyXFont const & fn, string const & str, bool autobreakrows)
{
	LyXFont font = fn;

	// insert the string, don't insert doublespace
	bool space_inserted = true;
	for (string::const_iterator cit = str.begin();
	    cit != str.end(); ++cit) {
		Paragraph & par = pars[pit];
		if (*cit == '\n') {
			if (autobreakrows && (!par.empty() || par.allowEmpty())) {
				breakParagraph(params(), pars, pit, pos,
					       par.layout()->isEnvironment());
				++pit;
				pos = 0;
				space_inserted = true;
			} else {
				continue;
			}
			// do not insert consecutive spaces if !free_spacing
		} else if ((*cit == ' ' || *cit == '\t') &&
			   space_inserted && !par.isFreeSpacing()) {
			continue;
		} else if (*cit == '\t') {
			if (!par.isFreeSpacing()) {
				// tabs are like spaces here
				par.insertChar(pos, ' ', font);
				++pos;
				space_inserted = true;
			} else {
				const pos_type n = 8 - pos % 8;
				for (pos_type i = 0; i < n; ++i) {
					par.insertChar(pos, ' ', font);
					++pos;
				}
				space_inserted = true;
			}
		} else if (!IsPrintable(*cit)) {
			// Ignore unprintables
			continue;
		} else {
			// just insert the character
			par.insertChar(pos, *cit, font);
			++pos;
			space_inserted = (*cit == ' ');
		}

	}
}


bool Buffer::readFile(string const & filename)
{
	// Check if the file is compressed.
	string const format = getFormatFromContents(filename);
	if (format == "gzip" || format == "zip" || format == "compress") {
		params().compressed = true;
	}

	// remove dummy empty par
	paragraphs().clear();
	bool ret = readFile(filename, paragraphs().size());

	// After we have read a file, we must ensure that the buffer
	// language is set and used in the gui.
	// If you know of a better place to put this, please tell me. (Lgb)
	updateDocLang(params().language);

	return ret;
}


bool Buffer::readFile(string const & filename, pit_type const pit)
{
	LyXLex lex(0, 0);
	lex.setFile(filename);
	return readFile(lex, filename, pit);
}


bool Buffer::fully_loaded() const
{
	return pimpl_->file_fully_loaded;
}


void Buffer::fully_loaded(bool const value)
{
	pimpl_->file_fully_loaded = value;
}


bool Buffer::readFile(LyXLex & lex, string const & filename, pit_type const pit)
{
	BOOST_ASSERT(!filename.empty());

	if (!lex.isOK()) {
		Alert::error(_("Document could not be read"),
			     bformat(_("%1$s could not be read."), filename));
		return false;
	}

	lex.next();
	string const token(lex.getString());

	if (!lex.isOK()) {
		Alert::error(_("Document could not be read"),
			     bformat(_("%1$s could not be read."), filename));
		return false;
	}

	// the first token _must_ be...
	if (token != "\\lyxformat") {
		lyxerr << "Token: " << token << endl;

		Alert::error(_("Document format failure"),
			     bformat(_("%1$s is not a LyX document."),
				       filename));
		return false;
	}

	lex.next();
	string tmp_format = lex.getString();
	//lyxerr << "LyX Format: `" << tmp_format << '\'' << endl;
	// if present remove ".," from string.
	string::size_type dot = tmp_format.find_first_of(".,");
	//lyxerr << "           dot found at " << dot << endl;
	if (dot != string::npos)
			tmp_format.erase(dot, 1);
	int const file_format = convert<int>(tmp_format);
	//lyxerr << "format: " << file_format << endl;

	if (file_format != LYX_FORMAT) {
		string const tmpfile = tempName();
		if (tmpfile.empty()) {
			Alert::error(_("Conversion failed"),
				     bformat(_("%1$s is from an earlier"
					      " version of LyX, but a temporary"
					      " file for converting it could"
					      " not be created."),
					      filename));
			return false;
		}
		string const lyx2lyx = LibFileSearch("lyx2lyx", "lyx2lyx");
		if (lyx2lyx.empty()) {
			Alert::error(_("Conversion script not found"),
				     bformat(_("%1$s is from an earlier"
					       " version of LyX, but the"
					       " conversion script lyx2lyx"
					       " could not be found."),
					       filename));
			return false;
		}
		ostringstream command;
		command << "python " << QuoteName(lyx2lyx)
			<< " -t " << convert<string>(LYX_FORMAT)
			<< " -o " << QuoteName(tmpfile) << ' '
			<< QuoteName(filename);
		string const command_str = command.str();

		lyxerr[Debug::INFO] << "Running '"
				    << command_str << '\''
				    << endl;

		cmd_ret const ret = RunCommand(command_str);
		if (ret.first != 0) {
			Alert::error(_("Conversion script failed"),
				     bformat(_("%1$s is from an earlier version"
					      " of LyX, but the lyx2lyx script"
					      " failed to convert it."),
					      filename));
			return false;
		} else {
			bool const ret = readFile(tmpfile, pit);
			// Do stuff with tmpfile name and buffer name here.
			return ret;
		}

	}

	if (readDocument(lex)) {
		Alert::error(_("Document format failure"),
			     bformat(_("%1$s ended unexpectedly, which means"
				       " that it is probably corrupted."),
				       filename));
	}

	//lyxerr << "removing " << MacroTable::localMacros().size()
	//	<< " temporary macro entries" << endl;
	//MacroTable::localMacros().clear();

	pimpl_->file_fully_loaded = true;
	return true;
}


// Should probably be moved to somewhere else: BufferView? LyXView?
bool Buffer::save() const
{
	// We don't need autosaves in the immediate future. (Asger)
	resetAutosaveTimers();

	// make a backup
	string s;
	if (lyxrc.make_backup) {
		s = fileName() + '~';
		if (!lyxrc.backupdir_path.empty())
			s = AddName(lyxrc.backupdir_path,
				    subst(os::internal_path(s),'/','!'));

		// It might very well be that this variant is just
		// good enough. (Lgb)
		// But to use this we need fs::copy_file to actually do a copy,
		// even when the target file exists. (Lgb)
		if (fs::exists(fileName()) && fs::is_writable(fs::path(fileName()).branch_path())) {
		  //try {
		    fs::copy_file(fileName(), s, false);
		    //}
		    //catch (fs::filesystem_error const & fe) {
		    //lyxerr << "LyX was not able to make backup copy. Beware.\n"
		    //	   << fe.what() << endl;
		    //}
		}
	}

	if (writeFile(fileName())) {
		markClean();
		removeAutosaveFile(fileName());
	} else {
		// Saving failed, so backup is not backup
		if (lyxrc.make_backup)
			rename(s, fileName());
		return false;
	}
	return true;
}


bool Buffer::writeFile(string const & fname) const
{
	if (pimpl_->read_only && fname == fileName())
		return false;

	bool retval = false;

	if (params().compressed) {
#ifdef USE_COMPRESSION
		gz::ogzstream ofs(fname.c_str(), ios::out|ios::trunc);
		if (!ofs)
			return false;

		retval = do_writeFile(ofs);
#else
		return false;
#endif
	} else {
		ofstream ofs(fname.c_str(), ios::out|ios::trunc);
		if (!ofs)
			return false;

		retval = do_writeFile(ofs);
	}

	return retval;
}


bool Buffer::do_writeFile(ostream & ofs) const
{
#ifdef HAVE_LOCALE
	// Use the standard "C" locale for file output.
	ofs.imbue(std::locale::classic());
#endif

	// The top of the file should not be written by params().

	// write out a comment in the top of the file
	ofs << "#LyX " << lyx_version
	    << " created this file. For more info see http://www.lyx.org/\n"
	    << "\\lyxformat " << LYX_FORMAT << "\n"
	    << "\\begin_document\n";

	// now write out the buffer parameters.
	ofs << "\\begin_header\n";
	params().writeFile(ofs);
	ofs << "\\end_header\n";

	// write the text
	ofs << "\n\\begin_body\n";
	text().write(*this, ofs);
	ofs << "\n\\end_body\n";

	// Write marker that shows file is complete
	ofs << "\\end_document" << endl;

	// Shouldn't really be needed....
	//ofs.close();

	// how to check if close went ok?
	// Following is an attempt... (BE 20001011)

	// good() returns false if any error occured, including some
	//        formatting error.
	// bad()  returns true if something bad happened in the buffer,
	//        which should include file system full errors.

	bool status = true;
	if (!ofs) {
		status = false;
		lyxerr << "File was not closed properly." << endl;
	}

	return status;
}


void Buffer::makeLaTeXFile(string const & fname,
			   string const & original_path,
			   OutputParams const & runparams,
			   bool output_preamble, bool output_body)
{
	lyxerr[Debug::LATEX] << "makeLaTeXFile..." << endl;

	ofstream ofs;
	if (!openFileWrite(ofs, fname))
		return;

	makeLaTeXFile(ofs, original_path,
		      runparams, output_preamble, output_body);

	ofs.close();
	if (ofs.fail())
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
}


void Buffer::makeLaTeXFile(ostream & os,
			   string const & original_path,
			   OutputParams const & runparams_in,
			   bool const output_preamble, bool const output_body)
{
	OutputParams runparams = runparams_in;

	// validate the buffer.
	lyxerr[Debug::LATEX] << "  Validating buffer..." << endl;
	LaTeXFeatures features(*this, params(), runparams.nice);
	validate(features);
	lyxerr[Debug::LATEX] << "  Buffer validation done." << endl;

	texrow().reset();

	// The starting paragraph of the coming rows is the
	// first paragraph of the document. (Asger)
	texrow().start(paragraphs().begin()->id(), 0);

	if (output_preamble && runparams.nice) {
		os << "%% LyX " << lyx_version << " created this file.  "
			"For more info, see http://www.lyx.org/.\n"
			"%% Do not edit unless you really know what "
			"you are doing.\n";
		texrow().newline();
		texrow().newline();
	}
	lyxerr[Debug::INFO] << "lyx header finished" << endl;
	// There are a few differences between nice LaTeX and usual files:
	// usual is \batchmode and has a
	// special input@path to allow the including of figures
	// with either \input or \includegraphics (what figinsets do).
	// input@path is set when the actual parameter
	// original_path is set. This is done for usual tex-file, but not
	// for nice-latex-file. (Matthias 250696)
	// Note that input@path is only needed for something the user does
	// in the preamble, included .tex files or ERT, files included by
	// LyX work without it.
	if (output_preamble) {
		if (!runparams.nice) {
			// code for usual, NOT nice-latex-file
			os << "\\batchmode\n"; // changed
			// from \nonstopmode
			texrow().newline();
		}
		if (!original_path.empty()) {
			string const inputpath = latex_path(original_path);
			os << "\\makeatletter\n"
			    << "\\def\\input@path{{"
			    << inputpath << "/}}\n"
			    << "\\makeatother\n";
			texrow().newline();
			texrow().newline();
			texrow().newline();
		}

		// Write the preamble
		runparams.use_babel = params().writeLaTeX(os, features, texrow());

		if (!output_body)
			return;

		// make the body.
		os << "\\begin{document}\n";
		texrow().newline();
	} // output_preamble
	lyxerr[Debug::INFO] << "preamble finished, now the body." << endl;

	if (!lyxrc.language_auto_begin) {
		os << subst(lyxrc.language_command_begin, "$$lang",
			     params().language->babel())
		    << endl;
		texrow().newline();
	}

	// if we are doing a real file with body, even if this is the
	// child of some other buffer, let's cut the link here.
	// This happens for example if only a child document is printed.
	string save_parentname;
	if (output_preamble) {
		save_parentname = params().parentname;
		params().parentname.erase();
	}

	// the real stuff
	latexParagraphs(*this, paragraphs(), os, texrow(), runparams);

	// Restore the parenthood if needed
	if (output_preamble)
		params().parentname = save_parentname;

	// add this just in case after all the paragraphs
	os << endl;
	texrow().newline();

	if (!lyxrc.language_auto_end) {
		os << subst(lyxrc.language_command_end, "$$lang",
			     params().language->babel())
		    << endl;
		texrow().newline();
	}

	if (output_preamble) {
		os << "\\end{document}\n";
		texrow().newline();

		lyxerr[Debug::LATEX] << "makeLaTeXFile...done" << endl;
	} else {
		lyxerr[Debug::LATEX] << "LaTeXFile for inclusion made."
				     << endl;
	}

	// Just to be sure. (Asger)
	texrow().newline();

	lyxerr[Debug::INFO] << "Finished making LaTeX file." << endl;
	lyxerr[Debug::INFO] << "Row count was " << texrow().rows() - 1
			    << '.' << endl;
}


bool Buffer::isLatex() const
{
	return params().getLyXTextClass().outputType() == LATEX;
}


bool Buffer::isLinuxDoc() const
{
	return params().getLyXTextClass().outputType() == LINUXDOC;
}


bool Buffer::isLiterate() const
{
	return params().getLyXTextClass().outputType() == LITERATE;
}


bool Buffer::isDocBook() const
{
	return params().getLyXTextClass().outputType() == DOCBOOK;
}


bool Buffer::isSGML() const
{
	LyXTextClass const & tclass = params().getLyXTextClass();

	return tclass.outputType() == LINUXDOC ||
	       tclass.outputType() == DOCBOOK;
}


void Buffer::makeLinuxDocFile(string const & fname,
			      OutputParams const & runparams,
			      bool const body_only)
{
	ofstream ofs;
	if (!openFileWrite(ofs, fname))
		return;

	LaTeXFeatures features(*this, params(), runparams.nice);
	validate(features);

	texrow().reset();

	LyXTextClass const & tclass = params().getLyXTextClass();

	string const & top_element = tclass.latexname();

	if (!body_only) {
		ofs << tclass.class_header();

		string preamble = params().preamble;
		string const name = runparams.nice ? ChangeExtension(pimpl_->filename, ".sgml")
			 : fname;
		preamble += features.getIncludedFiles(name);
		preamble += features.getLyXSGMLEntities();

		if (!preamble.empty()) {
			ofs << " [ " << preamble << " ]";
		}
		ofs << ">\n\n";

		if (params().options.empty())
			sgml::openTag(ofs, top_element);
		else {
			string top = top_element;
			top += ' ';
			top += params().options;
			sgml::openTag(ofs, top);
		}
	}

	ofs << "<!-- LyX "  << lyx_version
	    << " created this file. For more info see http://www.lyx.org/"
	    << " -->\n";

	linuxdocParagraphs(*this, paragraphs(), ofs, runparams);

	if (!body_only) {
		ofs << "\n\n";
		sgml::closeTag(ofs, top_element);
	}

	ofs.close();
	if (ofs.fail())
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
}


void Buffer::makeDocBookFile(string const & fname,
			     OutputParams const & runparams,
			     bool const only_body)
{
	ofstream ofs;
	if (!openFileWrite(ofs, fname))
		return;

	LaTeXFeatures features(*this, params(), runparams.nice);
	validate(features);

	texrow().reset();

	LyXTextClass const & tclass = params().getLyXTextClass();
	string const & top_element = tclass.latexname();

	if (!only_body) {
		if (runparams.flavor == OutputParams::XML)
			ofs << "<?xml version=\"1.0\" encoding=\""
			    << params().language->encoding()->Name() << "\"?>\n";

		ofs << "<!DOCTYPE " << top_element << " ";

		if (! tclass.class_header().empty()) ofs << tclass.class_header();
		else if (runparams.flavor == OutputParams::XML)
			ofs << "PUBLIC \"-//OASIS//DTD DocBook XML//EN\" "
			    << "\"http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd\"";
		else
			ofs << " PUBLIC \"-//OASIS//DTD DocBook V4.2//EN\"";

		string preamble = params().preamble;
		if (runparams.flavor != OutputParams::XML ) {
			preamble += "<!ENTITY % output.print.png \"IGNORE\">\n";
			preamble += "<!ENTITY % output.print.pdf \"IGNORE\">\n";
			preamble += "<!ENTITY % output.print.eps \"IGNORE\">\n";
			preamble += "<!ENTITY % output.print.bmp \"IGNORE\">\n";
		}

		string const name = runparams.nice ? ChangeExtension(pimpl_->filename, ".sgml")
			 : fname;
		preamble += features.getIncludedFiles(name);
		preamble += features.getLyXSGMLEntities();

		if (!preamble.empty()) {
			ofs << "\n [ " << preamble << " ]";
		}
		ofs << ">\n\n";
	}

	string top = top_element;
	top += " lang=\"";
	if (runparams.flavor == OutputParams::XML)
		top += params().language->code();
	else
		top += params().language->code().substr(0,2);
	top += '"';

	if (!params().options.empty()) {
		top += ' ';
		top += params().options;
	}

	ofs << "<!-- " << ((runparams.flavor == OutputParams::XML)? "XML" : "SGML")
	    << " file was created by LyX " << lyx_version
	    << "\n  See http://www.lyx.org/ for more information -->\n";

	params().getLyXTextClass().counters().reset();

	sgml::openTag(ofs, top);
	ofs << '\n';
	docbookParagraphs(paragraphs(), *this, ofs, runparams);
	sgml::closeTag(ofs, top_element);

	ofs.close();
	if (ofs.fail())
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
}


// chktex should be run with these flags disabled: 3, 22, 25, 30, 38(?)
// Other flags: -wall -v0 -x
int Buffer::runChktex()
{
	busy(true);

	// get LaTeX-Filename
	string const name = getLatexName();
	string const path = temppath();
	string const org_path = filePath();

	Path p(path); // path to LaTeX file
	message(_("Running chktex..."));

	// Generate the LaTeX file if neccessary
	OutputParams runparams;
	runparams.flavor = OutputParams::LATEX;
	runparams.nice = false;
	makeLaTeXFile(name, org_path, runparams);

	TeXErrors terr;
	Chktex chktex(lyxrc.chktex_command, name, filePath());
	int const res = chktex.run(terr); // run chktex

	if (res == -1) {
		Alert::error(_("chktex failure"),
			     _("Could not run chktex successfully."));
	} else if (res > 0) {
		// Insert all errors as errors boxes
		bufferErrors(*this, terr);
	}

	busy(false);

	return res;
}


void Buffer::validate(LaTeXFeatures & features) const
{
	LyXTextClass const & tclass = params().getLyXTextClass();

	if (features.isAvailable("dvipost") && params().tracking_changes
		&& params().output_changes) {
		features.require("dvipost");
		features.require("color");
	}

	// AMS Style is at document level
	if (params().use_amsmath == BufferParams::AMS_ON
	    || tclass.provides(LyXTextClass::amsmath))
		features.require("amsmath");

	for_each(paragraphs().begin(), paragraphs().end(),
		 boost::bind(&Paragraph::validate, _1, boost::ref(features)));

	// the bullet shapes are buffer level not paragraph level
	// so they are tested here
	for (int i = 0; i < 4; ++i) {
		if (params().user_defined_bullet(i) != ITEMIZE_DEFAULTS[i]) {
			int const font = params().user_defined_bullet(i).getFont();
			if (font == 0) {
				int const c = params()
					.user_defined_bullet(i)
					.getCharacter();
				if (c == 16
				   || c == 17
				   || c == 25
				   || c == 26
				   || c == 31) {
					features.require("latexsym");
				}
			} else if (font == 1) {
				features.require("amssymb");
			} else if ((font >= 2 && font <= 5)) {
				features.require("pifont");
			}
		}
	}

	if (lyxerr.debugging(Debug::LATEX)) {
		features.showStruct();
	}
}


void Buffer::getLabelList(vector<string> & list) const
{
	/// if this is a child document and the parent is already loaded
	/// Use the parent's list instead  [ale990407]
	Buffer const * tmp = getMasterBuffer();
	if (!tmp) {
		lyxerr << "getMasterBuffer() failed!" << endl;
		BOOST_ASSERT(tmp);
	}
	if (tmp != this) {
		tmp->getLabelList(list);
		return;
	}

	for (InsetIterator it = inset_iterator_begin(inset()); it; ++it)
		it.nextInset()->getLabelList(*this, list);
}


// This is also a buffer property (ale)
void Buffer::fillWithBibKeys(vector<pair<string, string> > & keys)
	const
{
	/// if this is a child document and the parent is already loaded
	/// use the parent's list instead  [ale990412]
	Buffer const * tmp = getMasterBuffer();
	BOOST_ASSERT(tmp);
	if (tmp != this) {
		tmp->fillWithBibKeys(keys);
		return;
	}

	for (InsetIterator it = inset_iterator_begin(inset()); it; ++it) {
		if (it->lyxCode() == InsetBase::BIBTEX_CODE) {
			InsetBibtex const & inset =
				dynamic_cast<InsetBibtex const &>(*it);
			inset.fillWithBibKeys(*this, keys);
		} else if (it->lyxCode() == InsetBase::INCLUDE_CODE) {
			InsetInclude const & inset =
				dynamic_cast<InsetInclude const &>(*it);
			inset.fillWithBibKeys(*this, keys);
		} else if (it->lyxCode() == InsetBase::BIBITEM_CODE) {
			InsetBibitem const & inset =
				dynamic_cast<InsetBibitem const &>(*it);
			string const key = inset.getContents();
			string const opt = inset.getOptions();
			string const ref; // = pit->asString(this, false);
			string const info = opt + "TheBibliographyRef" + ref;
			keys.push_back(pair<string, string>(key, info));
		}
	}
}


bool Buffer::isDepClean(string const & name) const
{
	DepClean::const_iterator const it = pimpl_->dep_clean.find(name);
	if (it == pimpl_->dep_clean.end())
		return true;
	return it->second;
}


void Buffer::markDepClean(string const & name)
{
	pimpl_->dep_clean[name] = true;
}


bool Buffer::dispatch(string const & command, bool * result)
{
	return dispatch(lyxaction.lookupFunc(command), result);
}


bool Buffer::dispatch(FuncRequest const & func, bool * result)
{
	bool dispatched = true;

	switch (func.action) {
		case LFUN_EXPORT: {
			bool const tmp = Exporter::Export(this, func.argument, false);
			if (result)
				*result = tmp;
			break;
		}

		default:
			dispatched = false;
	}
	return dispatched;
}


void Buffer::changeLanguage(Language const * from, Language const * to)
{
	BOOST_ASSERT(from);
	BOOST_ASSERT(to);

	lyxerr << "Changing Language!" << endl;

	// Take care of l10n/i18n
	updateDocLang(to);

	for_each(par_iterator_begin(),
		 par_iterator_end(),
		 bind(&Paragraph::changeLanguage, _1, params(), from, to));
}


void Buffer::updateDocLang(Language const * nlang)
{
	BOOST_ASSERT(nlang);

	pimpl_->messages.reset(new Messages(nlang->code()));
}


bool Buffer::isMultiLingual() const
{
	ParConstIterator end = par_iterator_end();
	for (ParConstIterator it = par_iterator_begin(); it != end; ++it)
		if (it->isMultiLingual(params()))
			return true;

	return false;
}


ParIterator Buffer::getParFromID(int const id) const
{
	ParConstIterator it = par_iterator_begin();
	ParConstIterator const end = par_iterator_end();

	if (id < 0) {
		// John says this is called with id == -1 from undo
		lyxerr << "getParFromID(), id: " << id << endl;
		return end;
	}

	for (; it != end; ++it)
		if (it->id() == id)
			return it;

	return end;
}


bool Buffer::hasParWithID(int const id) const
{
	ParConstIterator const it = getParFromID(id);
	return it != par_iterator_end();
}


ParIterator Buffer::par_iterator_begin()
{
	return ::par_iterator_begin(inset());
}


ParIterator Buffer::par_iterator_end()
{
	return ::par_iterator_end(inset());
}


ParConstIterator Buffer::par_iterator_begin() const
{
	return ::par_const_iterator_begin(inset());
}


ParConstIterator Buffer::par_iterator_end() const
{
	return ::par_const_iterator_end(inset());
}


Language const * Buffer::getLanguage() const
{
	return params().language;
}


string const Buffer::B_(string const & l10n) const
{
	if (pimpl_->messages.get()) {
		return pimpl_->messages->get(l10n);
	}

	return _(l10n);
}


bool Buffer::isClean() const
{
	return pimpl_->lyx_clean;
}


bool Buffer::isBakClean() const
{
	return pimpl_->bak_clean;
}


void Buffer::markClean() const
{
	if (!pimpl_->lyx_clean) {
		pimpl_->lyx_clean = true;
		updateTitles();
	}
	// if the .lyx file has been saved, we don't need an
	// autosave
	pimpl_->bak_clean = true;
}


void Buffer::markBakClean()
{
	pimpl_->bak_clean = true;
}


void Buffer::setUnnamed(bool flag)
{
	pimpl_->unnamed = flag;
}


bool Buffer::isUnnamed() const
{
	return pimpl_->unnamed;
}


#ifdef WITH_WARNINGS
#warning this function should be moved to buffer_pimpl.C
#endif
void Buffer::markDirty()
{
	if (pimpl_->lyx_clean) {
		pimpl_->lyx_clean = false;
		updateTitles();
	}
	pimpl_->bak_clean = false;

	DepClean::iterator it = pimpl_->dep_clean.begin();
	DepClean::const_iterator const end = pimpl_->dep_clean.end();

	for (; it != end; ++it) {
		it->second = false;
	}
}


string const & Buffer::fileName() const
{
	return pimpl_->filename;
}


string const & Buffer::filePath() const
{
	return pimpl_->filepath;
}


bool Buffer::isReadonly() const
{
	return pimpl_->read_only;
}


void Buffer::setParentName(string const & name)
{
	params().parentname = name;
}


Buffer const * Buffer::getMasterBuffer() const
{
	if (!params().parentname.empty()
	    && bufferlist.exists(params().parentname)) {
		Buffer const * buf = bufferlist.getBuffer(params().parentname);
		if (buf)
			return buf->getMasterBuffer();
	}

	return this;
}


MacroData const & Buffer::getMacro(std::string const & name) const
{
	return pimpl_->macros.get(name);
}


bool Buffer::hasMacro(string const & name) const
{
	return pimpl_->macros.has(name);
}


void Buffer::insertMacro(string const & name, MacroData const & data)
{
	MacroTable::globalMacros().insert(name, data);
	pimpl_->macros.insert(name, data);
}


void Buffer::buildMacros()
{
	// Start with global table.
	pimpl_->macros = MacroTable::globalMacros();

	// Now add our own.
	ParagraphList & pars = text().paragraphs();
	for (size_t i = 0, n = pars.size(); i != n; ++i) {
		//lyxerr << "searching main par " << i
		//	<< " for macro definitions" << std::endl;
		InsetList::iterator it = pars[i].insetlist.begin();
		InsetList::iterator end = pars[i].insetlist.end();
		for ( ; it != end; ++it) {
			//lyxerr << "found inset code " << it->inset->lyxCode() << std::endl;
			if (it->inset->lyxCode() == InsetBase::MATHMACRO_CODE) {
				MathMacroTemplate & mac
					= static_cast<MathMacroTemplate &>(*it->inset);
				insertMacro(mac.name(), mac.asMacroData());
			}
		}
	}
}
