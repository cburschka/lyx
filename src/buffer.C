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
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "bufferparams.h"
#include "counters.h"
#include "Bullet.h"
#include "Chktex.h"
#include "debug.h"
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

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/gzstream.h"
#include "support/lyxlib.h"
#include "support/os.h"
#include "support/path.h"
#include "support/textutils.h"
#include "support/tostr.h"
#include "support/std_sstream.h"

#include <boost/bind.hpp>

#include <iomanip>
#include <stack>

#include <utime.h>

using lyx::pos_type;
using lyx::par_type;

using lyx::support::AddName;
using lyx::support::atoi;
using lyx::support::bformat;
using lyx::support::ChangeExtension;
using lyx::support::cmd_ret;
using lyx::support::createBufferTmpDir;
using lyx::support::destroyDir;
using lyx::support::FileInfo;
using lyx::support::FileInfo;
using lyx::support::getExtFromContents;
using lyx::support::IsDirWriteable;
using lyx::support::IsFileWriteable;
using lyx::support::LibFileSearch;
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
using lyx::support::strToInt;
using lyx::support::subst;
using lyx::support::tempName;
using lyx::support::trim;

namespace os = lyx::support::os;

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

const int LYX_FORMAT = 232;

} // namespace anon


typedef std::map<string, bool> DepClean;

struct Buffer::Impl
{
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
	lyxvc.buffer(&parent);
	temppath = createBufferTmpDir();
	// FIXME: And now do something if temppath == string(), because we
	// assume from now on that temppath points to a valid temp dir.
	// See http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg67406.html
}


Buffer::Buffer(string const & file, bool ronly)
	: pimpl_(new Impl(*this, file, ronly))
{
	lyxerr[Debug::INFO] << "Buffer::Buffer()" << endl;
}


Buffer::~Buffer()
{
	lyxerr[Debug::INFO] << "Buffer::~Buffer()" << endl;
	// here the buffer should take care that it is
	// saved properly, before it goes into the void.

	closing();

	if (!temppath().empty() && destroyDir(temppath()) != 0) {
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


string const Buffer::getLatexName(bool no_path) const
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

	FileInfo const f_fi(fname);
	FileInfo const b_fi(bname);

	if (b_fi.exist() &&
	    (!f_fi.exist() || f_fi.getModificationTime() < b_fi.getModificationTime())) {
		lyxerr[Debug::FILES] << "Log name calculated as: " << bname << endl;
		return make_pair(Buffer::buildlog, bname);
	}
	lyxerr[Debug::FILES] << "Log name calculated as: " << fname << endl;
	return make_pair(Buffer::latexlog, fname);
}


void Buffer::setReadonly(bool flag)
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
	setReadonly(IsFileWriteable(pimpl_->filename) == 0);
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

	while (lex.isOK()) {
		lex.nextToken();
		string const token = lex.getString();

		if (token.empty())
			continue;

		if (token == "\\end_header")
			break;

		lyxerr[Debug::PARSER] << "Handling header token: `"
				      << token << '\'' << endl;


		string unknown = params().readToken(lex, token);
		if (!unknown.empty()) {
			if (unknown[0] != '\\') {
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
	return unknown_tokens;
}


// Uwe C. Schroeder
// changed to be public and have one parameter
// Returns false if "\end_document" is not read (Asger)
bool Buffer::readBody(LyXLex & lex)
{
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
	par_type & par, pos_type & pos,
	LyXFont const & fn, string const & str)
{
	LyXLayout_ptr const & layout = pars[par].layout();

	LyXFont font = fn;

	pars[par].checkInsertChar(font);
	// insert the string, don't insert doublespace
	bool space_inserted = true;
	bool autobreakrows = pars[par].autoBreakRows();
	for (string::const_iterator cit = str.begin();
	    cit != str.end(); ++cit) {
		if (*cit == '\n') {
			if (autobreakrows && (!pars[par].empty() || pars[par].allowEmpty())) {
				breakParagraph(params(), paragraphs(), par, pos,
					       layout->isEnvironment());
				++par;
				pos = 0;
				space_inserted = true;
			} else {
				continue;
			}
			// do not insert consecutive spaces if !free_spacing
		} else if ((*cit == ' ' || *cit == '\t') &&
			   space_inserted && !pars[par].isFreeSpacing()) {
			continue;
		} else if (*cit == '\t') {
			if (!pars[par].isFreeSpacing()) {
				// tabs are like spaces here
				pars[par].insertChar(pos, ' ', font);
				++pos;
				space_inserted = true;
			} else {
				const pos_type n = 8 - pos % 8;
				for (pos_type i = 0; i < n; ++i) {
					pars[par].insertChar(pos, ' ', font);
					++pos;
				}
				space_inserted = true;
			}
		} else if (!IsPrintable(*cit)) {
			// Ignore unprintables
			continue;
		} else {
			// just insert the character
			pars[par].insertChar(pos, *cit, font);
			++pos;
			space_inserted = (*cit == ' ');
		}

	}
}


bool Buffer::readFile(string const & filename)
{
	// Check if the file is compressed.
	string const format = getExtFromContents(filename);
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


bool Buffer::readFile(string const & filename, par_type pit)
{
	LyXLex lex(0, 0);
	lex.setFile(filename);
	return readFile(lex, filename, pit);
}


bool Buffer::fully_loaded() const
{
	return pimpl_->file_fully_loaded;
}


void Buffer::fully_loaded(bool value)
{
	pimpl_->file_fully_loaded = value;
}


bool Buffer::readFile(LyXLex & lex, string const & filename, par_type pit)
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

	lex.eatLine();
	string tmp_format = lex.getString();
	//lyxerr << "LyX Format: `" << tmp_format << '\'' << endl;
	// if present remove ".," from string.
	string::size_type dot = tmp_format.find_first_of(".,");
	//lyxerr << "           dot found at " << dot << endl;
	if (dot != string::npos)
			tmp_format.erase(dot, 1);
	int file_format = strToInt(tmp_format);
	//lyxerr << "format: " << file_format << endl;

	if (file_format > LYX_FORMAT) {
		Alert::warning(_("Document format failure"),
			       bformat(_("%1$s was created with a newer"
					 " version of LyX. This is likely to"
					 " cause problems."),
					 filename));
	} else if (file_format < LYX_FORMAT) {
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
		string command = LibFileSearch("lyx2lyx", "lyx2lyx");
		if (command.empty()) {
			Alert::error(_("Conversion script not found"),
				     bformat(_("%1$s is from an earlier"
					       " version of LyX, but the"
					       " conversion script lyx2lyx"
					       " could not be found."),
					       filename));
			return false;
		}
		command += " -t"
			+ tostr(LYX_FORMAT)
			+ " -o " + tmpfile + ' '
			+ QuoteName(filename);
		lyxerr[Debug::INFO] << "Running '"
				    << command << '\''
				    << endl;
		cmd_ret const ret = RunCommand(command);
		if (ret.first != 0) {
			Alert::error(_("Conversion script failed"),
				     bformat(_("%1$s is from an earlier version"
					      " of LyX, but the lyx2lyx script"
					      " failed to convert it."),
					      filename));
			return false;
		} else {
			bool ret = readFile(tmpfile, pit);
			// Do stuff with tmpfile name and buffer name here.
			return ret;
		}

	}

	bool the_end = readBody(lex);
	params().setPaperStuff();

#ifdef WITH_WARNINGS
#warning Look here!
#endif
#if 0
	if (token == "\\end_document")
		the_end_read = true;

	if (!the_end) {
		Alert::error(_("Document format failure"),
			     bformat(_("%1$s ended unexpectedly, which means"
				       " that it is probably corrupted."),
				       filename));
	}
#endif
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
				    subst(os::slashify_path(s),'/','!'));

		// Rename is the wrong way of making a backup,
		// this is the correct way.
		/* truss cp fil fil2:
		   lstat("LyXVC3.lyx", 0xEFFFF898)                 Err#2 ENOENT
		   stat("LyXVC.lyx", 0xEFFFF688)                   = 0
		   open("LyXVC.lyx", O_RDONLY)                     = 3
		   open("LyXVC3.lyx", O_WRONLY|O_CREAT|O_TRUNC, 0600) = 4
		   fstat(4, 0xEFFFF508)                            = 0
		   fstat(3, 0xEFFFF508)                            = 0
		   read(3, " # T h i s   f i l e   w".., 8192)     = 5579
		   write(4, " # T h i s   f i l e   w".., 5579)    = 5579
		   read(3, 0xEFFFD4A0, 8192)                       = 0
		   close(4)                                        = 0
		   close(3)                                        = 0
		   chmod("LyXVC3.lyx", 0100644)                    = 0
		   lseek(0, 0, SEEK_CUR)                           = 46440
		   _exit(0)
		*/

		// Should probably have some more error checking here.
		// Doing it this way, also makes the inodes stay the same.
		// This is still not a very good solution, in particular we
		// might loose the owner of the backup.
		FileInfo finfo(fileName());
		if (finfo.exist()) {
			mode_t fmode = finfo.getMode();
			struct utimbuf times = {
				finfo.getAccessTime(),
				finfo.getModificationTime() };

			ifstream ifs(fileName().c_str());
			ofstream ofs(s.c_str(), ios::out|ios::trunc);
			if (ifs && ofs) {
				ofs << ifs.rdbuf();
				ifs.close();
				ofs.close();
				::chmod(s.c_str(), fmode);

				if (::utime(s.c_str(), &times)) {
					lyxerr << "utime error." << endl;
				}
			} else {
				lyxerr << "LyX was not able to make "
					"backup copy. Beware." << endl;
			}
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

	FileInfo finfo(fname);
	if (finfo.exist() && !finfo.writable())
		return false;

	bool retval;

	if (params().compressed) {
		gz::ogzstream ofs(fname.c_str());
		if (!ofs)
			return false;

		retval = do_writeFile(ofs);

	} else {
		ofstream ofs(fname.c_str());
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
	    << "\\lyxformat " << LYX_FORMAT << "\n";

	// now write out the buffer parameters.
	params().writeFile(ofs);

	ofs << "\\end_header\n";

	// write the text
	text().write(*this, ofs);

	// Write marker that shows file is complete
	ofs << "\n\\end_document" << endl;

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
			   bool output_preamble, bool output_body)
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
			string inputpath = os::external_path(original_path);
			subst(inputpath, "~", "\\string~");
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
                              bool body_only)
{
	ofstream ofs;
	if (!openFileWrite(ofs, fname))
		return;

	LaTeXFeatures features(*this, params(), runparams.nice);
	validate(features);

	texrow().reset();

	LyXTextClass const & tclass = params().getLyXTextClass();

	string top_element = tclass.latexname();

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
			sgml::openTag(ofs, 0, false, top_element);
		else {
			string top = top_element;
			top += ' ';
			top += params().options;
			sgml::openTag(ofs, 0, false, top);
		}
	}

	ofs << "<!-- LyX "  << lyx_version
	    << " created this file. For more info see http://www.lyx.org/"
	    << " -->\n";

	linuxdocParagraphs(*this, paragraphs(), ofs, runparams);

	if (!body_only) {
		ofs << "\n\n";
		sgml::closeTag(ofs, 0, false, top_element);
	}

	ofs.close();
	if (ofs.fail())
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
}


void Buffer::makeDocBookFile(string const & fname,
			     OutputParams const & runparams,
			     bool only_body)
{
	ofstream ofs;
	if (!openFileWrite(ofs, fname))
		return;

	LaTeXFeatures features(*this, params(), runparams.nice);
	validate(features);

	texrow().reset();

	LyXTextClass const & tclass = params().getLyXTextClass();
	string top_element = tclass.latexname();

	if (!only_body) {
		ofs << subst(tclass.class_header(), "#", top_element);

		string preamble = params().preamble;
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
	top += params().language->code();
	top += '"';

	if (!params().options.empty()) {
		top += ' ';
		top += params().options;
	}
	sgml::openTag(ofs, 0, false, top);

	ofs << "<!-- SGML/XML file was created by LyX " << lyx_version
	    << "\n  See http://www.lyx.org/ for more information -->\n";

	params().getLyXTextClass().counters().reset();
	docbookParagraphs(*this, paragraphs(), ofs, runparams);

	ofs << "\n\n";
	sgml::closeTag(ofs, 0, false, top_element);

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
	int res = chktex.run(terr); // run chktex

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

	if (params().tracking_changes) {
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


void Buffer::getLabelList(std::vector<string> & list) const
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
void Buffer::fillWithBibKeys(std::vector<std::pair<string, string> > & keys)
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
		if (it->lyxCode() == InsetOld::BIBTEX_CODE) {
			InsetBibtex const & inset =
				dynamic_cast<InsetBibtex const &>(*it);
			inset.fillWithBibKeys(*this, keys);
		} else if (it->lyxCode() == InsetOld::INCLUDE_CODE) {
			InsetInclude const & inset =
				dynamic_cast<InsetInclude const &>(*it);
			inset.fillWithBibKeys(*this, keys);
		} else if (it->lyxCode() == InsetOld::BIBITEM_CODE) {
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
	DepClean::const_iterator it = pimpl_->dep_clean.find(name);
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
	lyxerr << "Changing Language!" << endl;

	// Take care of l10n/i18n
	updateDocLang(to);

	ParIterator end = par_iterator_end();
	for (ParIterator it = par_iterator_begin(); it != end; ++it)
		it->changeLanguage(params(), from, to);
}


void Buffer::updateDocLang(Language const * nlang)
{
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


ParIterator Buffer::getParFromID(int id) const
{
	ParConstIterator it = par_iterator_begin();
	ParConstIterator end = par_iterator_end();

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


bool Buffer::hasParWithID(int id) const
{
	ParConstIterator it = par_iterator_begin();
	ParConstIterator end = par_iterator_end();

	if (id < 0) {
		// John says this is called with id == -1 from undo
		lyxerr << "hasParWithID(), id: " << id << endl;
		return 0;
	}

	for (; it != end; ++it)
		if (it->id() == id)
			return true;

	return false;
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
