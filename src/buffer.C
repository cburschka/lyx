/**
 * \file buffer.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "LyXAction.h"
#include "lyxrc.h"
#include "lyxlex.h"
#include "tex-strings.h"
#include "layout.h"
#include "bufferview_funcs.h"
#include "lyxfont.h"
#include "version.h"
#include "LaTeX.h"
#include "Chktex.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "lyxtext.h"
#include "gettext.h"
#include "language.h"
#include "exporter.h"
#include "errorlist.h"
#include "Lsstream.h"
#include "format.h"
#include "BufferView.h"
#include "ParagraphParameters.h"
#include "iterators.h"
#include "lyxtextclasslist.h"
#include "sgml.h"
#include "paragraph_funcs.h"
#include "messages.h"
#include "author.h"

#include "frontends/LyXView.h"

#include "mathed/formulamacro.h"
#include "mathed/formula.h"

#include "insets/insetbibitem.h"
#include "insets/insetbibtex.h"
#include "insets/insetinclude.h"
#include "insets/insettext.h"

#include "frontends/Dialogs.h"
#include "frontends/Alert.h"

#include "graphics/Previews.h"

#include "support/textutils.h"
#include "support/filetools.h"
#include "support/path.h"
#include "support/os.h"
#include "support/tostr.h"
#include "support/lyxlib.h"
#include "support/FileInfo.h"
#include "support/lyxmanip.h"
#include "support/lyxtime.h"

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include <fstream>
#include <iomanip>
#include <map>
#include <stack>
#include <list>
#include <algorithm>

#include <cstdlib>
#include <cmath>
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>

#ifdef HAVE_LOCALE
#include <locale>
#endif

using namespace lyx::support;

#ifndef CXX_GLOBAL_CSTD
using std::pow;
#endif

using std::ostream;
using std::ofstream;
using std::ifstream;
using std::fstream;
using std::ios;
using std::setw;
using std::endl;
using std::pair;
using std::make_pair;
using std::vector;
using std::map;
using std::stack;
using std::list;
using std::for_each;

using lyx::pos_type;
using lyx::textclass_type;

namespace grfx = lyx::graphics;

// all these externs should eventually be removed.
extern BufferList bufferlist;

namespace {

const int LYX_FORMAT = 224;

} // namespace anon

Buffer::Buffer(string const & file, bool ronly)
	: niceFile(true), lyx_clean(true), bak_clean(true),
	  unnamed(false), read_only(ronly),
	  filename_(file), users(0)
{
	lyxerr[Debug::INFO] << "Buffer::Buffer()" << endl;
	filepath_ = OnlyPath(file);
	lyxvc.buffer(this);
	if (read_only || lyxrc.use_tempdir) {
		tmppath = CreateBufferTmpDir();
	} else {
		tmppath.erase();
	}

	// set initial author
	authors().record(Author(lyxrc.user_name, lyxrc.user_email));
}


Buffer::~Buffer()
{
	lyxerr[Debug::INFO] << "Buffer::~Buffer()" << endl;
	// here the buffer should take care that it is
	// saved properly, before it goes into the void.

	// make sure that views using this buffer
	// forgets it.
	if (users)
		users->buffer(0);

	if (!tmppath.empty() && destroyDir(tmppath) != 0) {
		Alert::warning(_("Could not remove temporary directory"),
			bformat(_("Could not remove the temporary directory %1$s"), tmppath));
	}

	paragraphs.clear();

	// Remove any previewed LaTeX snippets assocoated with this buffer.
	grfx::Previews::get().removeLoader(this);
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

	string path = OnlyPath(filename);

	if (lyxrc.use_tempdir || !IsDirWriteable(path))
		path = tmppath;

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
	if (read_only != flag) {
		read_only = flag;
		updateTitles();
		if (users)
			users->owner()->getDialogs().updateBufferDependent(false);
	}
}


AuthorList & Buffer::authors()
{
	return params.authorlist;
}


/// Update window titles of all users
// Should work on a list
void Buffer::updateTitles() const
{
	if (users)
		users->owner()->updateWindowTitle();
}


/// Reset autosave timer of all users
// Should work on a list
void Buffer::resetAutosaveTimers() const
{
	if (users)
		users->owner()->resetAutosaveTimer();
}


void Buffer::setFileName(string const & newfile)
{
	filename_ = MakeAbsPath(newfile);
	filepath_ = OnlyPath(filename_);
	setReadonly(IsFileWriteable(filename_) == 0);
	updateTitles();
}


// We'll remove this later. (Lgb)
namespace {

void unknownClass(string const & unknown)
{
	Alert::warning(_("Unknown document class"),
		bformat(_("Using the default document class, because the "
			" class %1$s is unknown."), unknown));
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


		string unknown = params.readToken(lex, token);
		if (!unknown.empty()) {
			if (unknown[0] != '\\') {
				unknownClass(unknown);
			} else {
				++unknown_tokens;
				string const s = bformat(_("Unknown token: "
							   "%1$s %2$s\n"),
							 token,
							 lex.getString());
				parseError(ErrorItem(_("Header error"), s,
						     -1, 0, 0));
			}
		}
	}
	return unknown_tokens;
}


// candidate for move to BufferView
// (at least some parts in the beginning of the func)
//
// Uwe C. Schroeder
// changed to be public and have one parameter
// if par = 0 normal behavior
// else insert behavior
// Returns false if "\the_end" is not read (Asger)
bool Buffer::readBody(LyXLex & lex, ParagraphList::iterator pit)
{
	Paragraph::depth_type depth = 0;
	bool the_end_read = false;

	if (paragraphs.empty()) {
		readHeader(lex);
		if (!params.getLyXTextClass().load()) {
			string theclass = params.getLyXTextClass().name();
			Alert::error(_("Can't load document class"), bformat(
					"Using the default document class, because the "
					" class %1$s could not be loaded.", theclass));
			params.textclass = 0;
		}
	} else {
		// We are inserting into an existing document
		users->text->breakParagraph(paragraphs);

		// We don't want to adopt the parameters from the
		// document we insert, so read them into a temporary buffer
		// and then discard it

		Buffer tmpbuf("", false);
		tmpbuf.readHeader(lex);
	}

	while (lex.isOK()) {
		lex.nextToken();
		string const token = lex.getString();

		if (token.empty())
			continue;

		lyxerr[Debug::PARSER] << "Handling token: `"
				      << token << '\'' << endl;

		if (token == "\\the_end") {
			the_end_read = true;
			continue;
		}

		readParagraph(lex, token, paragraphs, pit, depth);
	}

	return the_end_read;
}


int Buffer::readParagraph(LyXLex & lex, string const & token,
			  ParagraphList & pars, ParagraphList::iterator & pit,
			  Paragraph::depth_type & depth)
{
	static Change current_change;
	int unknown = 0;

	if (token == "\\layout") {
		lex.pushToken(token);

		Paragraph par;
		par.params().depth(depth);
		if (params.tracking_changes)
			par.trackChanges();
		LyXFont f(LyXFont::ALL_INHERIT, params.language);
		par.setFont(0, f);

		// insert after
		if (pit != pars.end())
			++pit;

		pit = pars.insert(pit, par);

		// FIXME: goddamn InsetTabular makes us pass a Buffer
		// not BufferParams
		::readParagraph(*this, *pit, lex);

	} else if (token == "\\begin_deeper") {
		++depth;
	} else if (token == "\\end_deeper") {
		if (!depth) {
			lex.printError("\\end_deeper: " "depth is already null");
		} else {
			--depth;
		}
	} else {
		++unknown;
	}
	return unknown;
}


// needed to insert the selection
void Buffer::insertStringAsLines(ParagraphList::iterator & par, pos_type & pos,
				 LyXFont const & fn,string const & str)
{
	LyXLayout_ptr const & layout = par->layout();

	LyXFont font = fn;

	par->checkInsertChar(font);
	// insert the string, don't insert doublespace
	bool space_inserted = true;
	bool autobreakrows = !par->inInset() ||
		static_cast<InsetText *>(par->inInset())->getAutoBreakRows();
	for(string::const_iterator cit = str.begin();
	    cit != str.end(); ++cit) {
		if (*cit == '\n') {
			if (autobreakrows && (!par->empty() || par->allowEmpty())) {
				breakParagraph(params, paragraphs, par, pos,
					       layout->isEnvironment());
				++par;
				pos = 0;
				space_inserted = true;
			} else {
				continue;
			}
			// do not insert consecutive spaces if !free_spacing
		} else if ((*cit == ' ' || *cit == '\t') &&
			   space_inserted && !par->isFreeSpacing()) {
			continue;
		} else if (*cit == '\t') {
			if (!par->isFreeSpacing()) {
				// tabs are like spaces here
				par->insertChar(pos, ' ', font);
				++pos;
				space_inserted = true;
			} else {
				const pos_type nb = 8 - pos % 8;
				for (pos_type a = 0; a < nb ; ++a) {
					par->insertChar(pos, ' ', font);
					++pos;
				}
				space_inserted = true;
			}
		} else if (!IsPrintable(*cit)) {
			// Ignore unprintables
			continue;
		} else {
			// just insert the character
			par->insertChar(pos, *cit, font);
			++pos;
			space_inserted = (*cit == ' ');
		}

	}
}


bool Buffer::readFile(LyXLex & lex, string const & filename)
{
	bool ret = readFile(lex, filename, paragraphs.begin());

	// After we have read a file, we must ensure that the buffer
	// language is set and used in the gui.
	// If you know of a better place to put this, please tell me. (Lgb)
	updateDocLang(params.language);

	return ret;
}


// FIXME: all the below Alerts should give the filename..
bool Buffer::readFile(LyXLex & lex, string const & filename,
		      ParagraphList::iterator pit)
{
	if (!lex.isOK()) {
		Alert::error(_("Document could not be read"),
			_("The specified document could not be read."));
		return false;
	}

	lex.next();
	string const token(lex.getString());

	if (!lex.isOK()) {
		Alert::error(_("Document could not be read"),
			_("The specified document could not be read."));
		return false;
	}

	// the first token _must_ be...
	if (token != "\\lyxformat") {
		Alert::error(_("Document format failure"),
			_("The specified document is not a LyX document."));
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
	file_format = strToInt(tmp_format);
	//lyxerr << "format: " << file_format << endl;
	if (file_format == LYX_FORMAT) {
		// current format
	} else if (file_format > LYX_FORMAT) {
		Alert::warning(_("Document format failure"),
			_("This document was created with a newer version of "
			"LyX. This is likely to cause problems."));
	} else if (file_format < LYX_FORMAT) {
		// old formats
		if (file_format < 200) {
			Alert::error(_("Document format failure"),
				_("This LyX document is too old to be read "
				"by this version of LyX. Try LyX 0.10."));
			return false;
		} else if (!filename.empty()) {
			string command =
				LibFileSearch("lyx2lyx", "lyx2lyx");
			if (command.empty()) {
				Alert::error(_("Conversion script not found"),
					_("The document is from an earlier version "
					  "of LyX, but the conversion script lyx2lyx "
					  "could not be found."));
				return false;
			}
			command += " -t"
				+tostr(LYX_FORMAT) + ' '
				+ QuoteName(filename);
			lyxerr[Debug::INFO] << "Running '"
					    << command << '\''
					    << endl;
			cmd_ret const ret = RunCommand(command);
			if (ret.first) {
				Alert::error(_("Conversion script failed"),
					_("The document is from an earlier version "
					  "of LyX, but the lyx2lyx script failed "
					  "to convert it."));
				return false;
			}
			istringstream is(STRCONV(ret.second));
			LyXLex tmplex(0, 0);
			tmplex.setStream(is);
			return readFile(tmplex, string(), pit);
		} else {
			// This code is reached if lyx2lyx failed (for
			// some reason) to change the file format of
			// the file.
			Assert(false);
			return false;
		}
	}
	bool the_end = readBody(lex, pit);
	params.setPaperStuff();

	if (!the_end) {
		Alert::error(_("Document format failure"),
			_("The document ended unexpectedly, which means "
			  "that it is probably corrupted."));
	}
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
		if (lyxrc.make_backup) {
			rename(s, fileName());
		}
		return false;
	}
	return true;
}


bool Buffer::writeFile(string const & fname) const
{
	if (read_only && (fname == fileName())) {
		return false;
	}

	FileInfo finfo(fname);
	if (finfo.exist() && !finfo.writable()) {
		return false;
	}

	ofstream ofs(fname.c_str());
	if (!ofs) {
		return false;
	}

#ifdef HAVE_LOCALE
	// Use the standard "C" locale for file output.
	ofs.imbue(std::locale::classic());
#endif

	// The top of the file should not be written by params.

	// write out a comment in the top of the file
	ofs << '#' << lyx_docversion
	    << " created this file. For more info see http://www.lyx.org/\n"
	    << "\\lyxformat " << LYX_FORMAT << "\n";

	// now write out the buffer paramters.
	params.writeFile(ofs);

	ofs << "\\end_header\n";

	Paragraph::depth_type depth = 0;

	// this will write out all the paragraphs
	// using recursive descent.
	ParagraphList::const_iterator pit = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();
	for (; pit != pend; ++pit)
		pit->write(this, ofs, params, depth);

	// Write marker that shows file is complete
	ofs << "\n\\the_end" << endl;

	ofs.close();

	// how to check if close went ok?
	// Following is an attempt... (BE 20001011)

	// good() returns false if any error occured, including some
	//        formatting error.
	// bad()  returns true if something bad happened in the buffer,
	//        which should include file system full errors.

	bool status = true;
	if (!ofs.good()) {
		status = false;
#if 0
		if (ofs.bad()) {
			lyxerr << "Buffer::writeFile: BAD ERROR!" << endl;
		} else {
			lyxerr << "Buffer::writeFile: NOT SO BAD ERROR!"
			       << endl;
		}
#endif
	}

	return status;
}


namespace {

pair<int, string> const addDepth(int depth, int ldepth)
{
	int d = depth * 2;
	if (ldepth > depth)
		d += (ldepth - depth) * 2;
	return make_pair(d, string(d, ' '));
}

}


string const Buffer::asciiParagraph(Paragraph const & par,
				    unsigned int linelen,
				    bool noparbreak) const
{
	ostringstream buffer;
	int ltype = 0;
	Paragraph::depth_type ltype_depth = 0;
	bool ref_printed = false;
	Paragraph::depth_type depth = par.params().depth();

	// First write the layout
	string const & tmp = par.layout()->name();
	if (compare_no_case(tmp, "itemize") == 0) {
		ltype = 1;
		ltype_depth = depth + 1;
	} else if (compare_ascii_no_case(tmp, "enumerate") == 0) {
		ltype = 2;
		ltype_depth = depth + 1;
	} else if (contains(ascii_lowercase(tmp), "ection")) {
		ltype = 3;
		ltype_depth = depth + 1;
	} else if (contains(ascii_lowercase(tmp), "aragraph")) {
		ltype = 4;
		ltype_depth = depth + 1;
	} else if (compare_ascii_no_case(tmp, "description") == 0) {
		ltype = 5;
		ltype_depth = depth + 1;
	} else if (compare_ascii_no_case(tmp, "abstract") == 0) {
		ltype = 6;
		ltype_depth = 0;
	} else if (compare_ascii_no_case(tmp, "bibliography") == 0) {
		ltype = 7;
		ltype_depth = 0;
	} else {
		ltype = 0;
		ltype_depth = 0;
	}

	/* maybe some vertical spaces */

	/* the labelwidthstring used in lists */

	/* some lines? */

	/* some pagebreaks? */

	/* noindent ? */

	/* what about the alignment */

	// linelen <= 0 is special and means we don't have paragraph breaks

	string::size_type currlinelen = 0;

	if (!noparbreak) {
		if (linelen > 0)
			buffer << "\n\n";

		buffer << string(depth * 2, ' ');
		currlinelen += depth * 2;

		//--
		// we should probably change to the paragraph language in the
		// gettext here (if possible) so that strings are outputted in
		// the correct language! (20012712 Jug)
		//--
		switch (ltype) {
		case 0: // Standard
		case 4: // (Sub)Paragraph
		case 5: // Description
			break;
		case 6: // Abstract
			if (linelen > 0) {
				buffer << _("Abstract") << "\n\n";
				currlinelen = 0;
			} else {
				string const abst = _("Abstract: ");
				buffer << abst;
				currlinelen += abst.length();
			}
			break;
		case 7: // Bibliography
			if (!ref_printed) {
				if (linelen > 0) {
					buffer << _("References") << "\n\n";
					currlinelen = 0;
				} else {
					string const refs = _("References: ");
					buffer << refs;
					currlinelen += refs.length();
				}

				ref_printed = true;
			}
			break;
		default:
		{
			string const parlab = par.params().labelString();
			buffer << parlab << ' ';
			currlinelen += parlab.length() + 1;
		}
		break;

		}
	}

	if (!currlinelen) {
		pair<int, string> p = addDepth(depth, ltype_depth);
		buffer << p.second;
		currlinelen += p.first;
	}

	// this is to change the linebreak to do it by word a bit more
	// intelligent hopefully! (only in the case where we have a
	// max linelength!) (Jug)

	string word;

	for (pos_type i = 0; i < par.size(); ++i) {
		char c = par.getUChar(params, i);
		switch (c) {
		case Paragraph::META_INSET:
		{
			Inset const * inset = par.getInset(i);
			if (inset) {
				if (linelen > 0) {
					buffer << word;
					currlinelen += word.length();
					word.erase();
				}
				if (inset->ascii(this, buffer, linelen)) {
					// to be sure it breaks paragraph
					currlinelen += linelen;
				}
			}
		}
		break;

		default:
			if (c == ' ') {
				if (linelen > 0 &&
				    currlinelen + word.length() > linelen - 10) {
					buffer << "\n";
					pair<int, string> p = addDepth(depth, ltype_depth);
					buffer << p.second;
					currlinelen = p.first;
				}

				buffer << word << ' ';
				currlinelen += word.length() + 1;
				word.erase();

			} else {
				if (c != '\0') {
					word += c;
				} else {
					lyxerr[Debug::INFO] <<
						"writeAsciiFile: NULL char in structure." << endl;
				}
				if ((linelen > 0) &&
					(currlinelen + word.length()) > linelen)
				{
					buffer << "\n";

					pair<int, string> p =
						addDepth(depth, ltype_depth);
					buffer << p.second;
					currlinelen = p.first;
				}
			}
			break;
		}
	}
	buffer << word;
	return STRCONV(buffer.str());
}


void Buffer::writeFileAscii(string const & fname, int linelen)
{
	ofstream ofs(fname.c_str());
	if (!ofs) {
		string const file = MakeDisplayPath(fname, 50);
		string text = bformat(_("Could not save the document\n%1$s."), file);
		Alert::error(_("Could not save document"), text);
		return;
	}
	writeFileAscii(ofs, linelen);
}


void Buffer::writeFileAscii(ostream & os, int linelen)
{
	ParagraphList::iterator beg = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	ParagraphList::iterator it = beg;
	for (; it != end; ++it) {
		os << asciiParagraph(*it, linelen, it == beg);
	}
	os << "\n";
}



void Buffer::makeLaTeXFile(string const & fname,
			   string const & original_path,
			   LatexRunParams const & runparams,
			   bool only_body, bool only_preamble)
{
	lyxerr[Debug::LATEX] << "makeLaTeXFile..." << endl;

	ofstream ofs(fname.c_str());
	if (!ofs) {
		string const file = MakeDisplayPath(fname, 50);
		string text = bformat(_("Could not open the specified document\n%1$s."),
			file);
		Alert::error(_("Could not open file"), text);
		return;
	}

	makeLaTeXFile(ofs, original_path,
		      runparams, only_body, only_preamble);

	ofs.close();
	if (ofs.fail()) {
		lyxerr << "File was not closed properly." << endl;
	}
}


void Buffer::makeLaTeXFile(ostream & os,
			   string const & original_path,
			   LatexRunParams const & runparams_in,
			   bool only_body, bool only_preamble)
{
	LatexRunParams runparams = runparams_in;
	niceFile = runparams.nice; // this will be used by Insetincludes.

	// validate the buffer.
	lyxerr[Debug::LATEX] << "  Validating buffer..." << endl;
	LaTeXFeatures features(params);
	validate(features);
	lyxerr[Debug::LATEX] << "  Buffer validation done." << endl;

	texrow.reset();
	// The starting paragraph of the coming rows is the
	// first paragraph of the document. (Asger)
	texrow.start(paragraphs.begin()->id(), 0);

	if (!only_body && runparams.nice) {
		os << "%% " << lyx_docversion << " created this file.  "
			"For more info, see http://www.lyx.org/.\n"
			"%% Do not edit unless you really know what "
			"you are doing.\n";
		texrow.newline();
		texrow.newline();
	}
	lyxerr[Debug::INFO] << "lyx header finished" << endl;
	// There are a few differences between nice LaTeX and usual files:
	// usual is \batchmode and has a
	// special input@path to allow the including of figures
	// with either \input or \includegraphics (what figinsets do).
	// input@path is set when the actual parameter
	// original_path is set. This is done for usual tex-file, but not
	// for nice-latex-file. (Matthias 250696)
	if (!only_body) {
		if (!runparams.nice) {
			// code for usual, NOT nice-latex-file
			os << "\\batchmode\n"; // changed
			// from \nonstopmode
			texrow.newline();
		}
		if (!original_path.empty()) {
			string inputpath = os::external_path(original_path);
			subst(inputpath, "~", "\\string~");
			os << "\\makeatletter\n"
			    << "\\def\\input@path{{"
			    << inputpath << "/}}\n"
			    << "\\makeatother\n";
			texrow.newline();
			texrow.newline();
			texrow.newline();
		}

		// Write the preamble
		runparams.use_babel = params.writeLaTeX(os, features, texrow);

		if (only_preamble)
			return;

		// make the body.
		os << "\\begin{document}\n";
		texrow.newline();
	} // only_body
	lyxerr[Debug::INFO] << "preamble finished, now the body." << endl;

	if (!lyxrc.language_auto_begin) {
		os << subst(lyxrc.language_command_begin, "$$lang",
			     params.language->babel())
		    << endl;
		texrow.newline();
	}

	latexParagraphs(this, paragraphs, os, texrow, runparams);

	// add this just in case after all the paragraphs
	os << endl;
	texrow.newline();

	if (!lyxrc.language_auto_end) {
		os << subst(lyxrc.language_command_end, "$$lang",
			     params.language->babel())
		    << endl;
		texrow.newline();
	}

	if (!only_body) {
		os << "\\end{document}\n";
		texrow.newline();

		lyxerr[Debug::LATEX] << "makeLaTeXFile...done" << endl;
	} else {
		lyxerr[Debug::LATEX] << "LaTeXFile for inclusion made."
				     << endl;
	}

	// Just to be sure. (Asger)
	texrow.newline();

	lyxerr[Debug::INFO] << "Finished making LaTeX file." << endl;
	lyxerr[Debug::INFO] << "Row count was " << texrow.rows() - 1
			    << '.' << endl;

	// we want this to be true outside previews (for insetexternal)
	niceFile = true;
}


bool Buffer::isLatex() const
{
	return params.getLyXTextClass().outputType() == LATEX;
}


bool Buffer::isLinuxDoc() const
{
	return params.getLyXTextClass().outputType() == LINUXDOC;
}


bool Buffer::isLiterate() const
{
	return params.getLyXTextClass().outputType() == LITERATE;
}


bool Buffer::isDocBook() const
{
	return params.getLyXTextClass().outputType() == DOCBOOK;
}


bool Buffer::isSGML() const
{
	LyXTextClass const & tclass = params.getLyXTextClass();

	return tclass.outputType() == LINUXDOC ||
	       tclass.outputType() == DOCBOOK;
}


void Buffer::makeLinuxDocFile(string const & fname, bool nice, bool body_only)
{
	ofstream ofs(fname.c_str());

	if (!ofs) {
		string const file = MakeDisplayPath(fname, 50);
		string text = bformat(_("Could not save the specified document\n%1$s.\n"),
			file);
		Alert::error(_("Could not save document"), text);
		return;
	}

	niceFile = nice; // this will be used by included files.

	LaTeXFeatures features(params);

	validate(features);

	texrow.reset();

	LyXTextClass const & tclass = params.getLyXTextClass();

	string top_element = tclass.latexname();

	if (!body_only) {
		ofs << "<!doctype linuxdoc system";

		string preamble = params.preamble;
		string const name = nice ? ChangeExtension(filename_, ".sgml")
			 : fname;
		preamble += features.getIncludedFiles(name);
		preamble += features.getLyXSGMLEntities();

		if (!preamble.empty()) {
			ofs << " [ " << preamble << " ]";
		}
		ofs << ">\n\n";

		if (params.options.empty())
			sgml::openTag(ofs, 0, false, top_element);
		else {
			string top = top_element;
			top += ' ';
			top += params.options;
			sgml::openTag(ofs, 0, false, top);
		}
	}

	ofs << "<!-- "  << lyx_docversion
	    << " created this file. For more info see http://www.lyx.org/"
	    << " -->\n";

	Paragraph::depth_type depth = 0; // paragraph depth
	string item_name;
	vector<string> environment_stack(5);

	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		LyXLayout_ptr const & style = pit->layout();
		// treat <toc> as a special case for compatibility with old code
		if (pit->isInset(0)) {
			Inset * inset = pit->getInset(0);
			Inset::Code lyx_code = inset->lyxCode();
			if (lyx_code == Inset::TOC_CODE) {
				string const temp = "toc";
				sgml::openTag(ofs, depth, false, temp);
				continue;
			}
		}

		// environment tag closing
		for (; depth > pit->params().depth(); --depth) {
			sgml::closeTag(ofs, depth, false, environment_stack[depth]);
			environment_stack[depth].erase();
		}

		// write opening SGML tags
		switch (style->latextype) {
		case LATEX_PARAGRAPH:
			if (depth == pit->params().depth()
			   && !environment_stack[depth].empty()) {
				sgml::closeTag(ofs, depth, false, environment_stack[depth]);
				environment_stack[depth].erase();
				if (depth)
					--depth;
				else
					ofs << "</p>";
			}
			sgml::openTag(ofs, depth, false, style->latexname());
			break;

		case LATEX_COMMAND:
			if (depth != 0)
				parseError(ErrorItem(_("Error:"), _("Wrong depth for LatexType Command.\n"), pit->id(), 0, pit->size()));

			if (!environment_stack[depth].empty()) {
				sgml::closeTag(ofs, depth, false, environment_stack[depth]);
				ofs << "</p>";
			}

			environment_stack[depth].erase();
			sgml::openTag(ofs, depth, false, style->latexname());
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
		case LATEX_BIB_ENVIRONMENT:
		{
			string const & latexname = style->latexname();

			if (depth == pit->params().depth()
			    && environment_stack[depth] != latexname) {
				sgml::closeTag(ofs, depth, false,
					     environment_stack[depth]);
				environment_stack[depth].erase();
			}
			if (depth < pit->params().depth()) {
			       depth = pit->params().depth();
			       environment_stack[depth].erase();
			}
			if (environment_stack[depth] != latexname) {
				if (depth == 0) {
					sgml::openTag(ofs, depth, false, "p");
				}
				sgml::openTag(ofs, depth, false, latexname);

				if (environment_stack.size() == depth + 1)
					environment_stack.push_back("!-- --");
				environment_stack[depth] = latexname;
			}

			if (style->latexparam() == "CDATA")
				ofs << "<![CDATA[";

			if (style->latextype == LATEX_ENVIRONMENT) break;

			if (style->labeltype == LABEL_MANUAL)
				item_name = "tag";
			else
				item_name = "item";

			sgml::openTag(ofs, depth + 1, false, item_name);
		}
		break;

		default:
			sgml::openTag(ofs, depth, false, style->latexname());
			break;
		}

		simpleLinuxDocOnePar(ofs, pit, depth);

		ofs << "\n";
		// write closing SGML tags
		switch (style->latextype) {
		case LATEX_COMMAND:
			break;
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
		case LATEX_BIB_ENVIRONMENT:
			if (style->latexparam() == "CDATA")
				ofs << "]]>";
			break;
		default:
			sgml::closeTag(ofs, depth, false, style->latexname());
			break;
		}
	}

	// Close open tags
	for (int i = depth; i >= 0; --i)
		sgml::closeTag(ofs, depth, false, environment_stack[i]);

	if (!body_only) {
		ofs << "\n\n";
		sgml::closeTag(ofs, 0, false, top_element);
	}

	ofs.close();
	// How to check for successful close

	// we want this to be true outside previews (for insetexternal)
	niceFile = true;
}


// checks, if newcol chars should be put into this line
// writes newline, if necessary.
namespace {

void sgmlLineBreak(ostream & os, string::size_type & colcount,
			  string::size_type newcol)
{
	colcount += newcol;
	if (colcount > lyxrc.ascii_linelen) {
		os << "\n";
		colcount = newcol; // assume write after this call
	}
}

enum PAR_TAG {
	NONE=0,
	TT = 1,
	SF = 2,
	BF = 4,
	IT = 8,
	SL = 16,
	EM = 32
};


string tag_name(PAR_TAG const & pt) {
	switch (pt) {
	case NONE: return "!-- --";
	case TT: return "tt";
	case SF: return "sf";
	case BF: return "bf";
	case IT: return "it";
	case SL: return "sl";
	case EM: return "em";
	}
	return "";
}


inline
void operator|=(PAR_TAG & p1, PAR_TAG const & p2)
{
	p1 = static_cast<PAR_TAG>(p1 | p2);
}


inline
void reset(PAR_TAG & p1, PAR_TAG const & p2)
{
	p1 = static_cast<PAR_TAG>(p1 & ~p2);
}

} // anon


// Handle internal paragraph parsing -- layout already processed.
void Buffer::simpleLinuxDocOnePar(ostream & os,
	ParagraphList::iterator par,
	Paragraph::depth_type /*depth*/) const
{
	LyXLayout_ptr const & style = par->layout();

	string::size_type char_line_count = 5;     // Heuristic choice ;-)

	// gets paragraph main font
	LyXFont font_old;
	bool desc_on;
	if (style->labeltype == LABEL_MANUAL) {
		font_old = style->labelfont;
		desc_on = true;
	} else {
		font_old = style->font;
		desc_on = false;
	}

	LyXFont::FONT_FAMILY family_type = LyXFont::ROMAN_FAMILY;
	LyXFont::FONT_SERIES series_type = LyXFont::MEDIUM_SERIES;
	LyXFont::FONT_SHAPE  shape_type  = LyXFont::UP_SHAPE;
	bool is_em = false;

	stack<PAR_TAG> tag_state;
	// parsing main loop
	for (pos_type i = 0; i < par->size(); ++i) {

		PAR_TAG tag_close = NONE;
		list < PAR_TAG > tag_open;

		LyXFont const font = par->getFont(params, i, outerFont(par, paragraphs));

		if (font_old.family() != font.family()) {
			switch (family_type) {
			case LyXFont::SANS_FAMILY:
				tag_close |= SF;
				break;
			case LyXFont::TYPEWRITER_FAMILY:
				tag_close |= TT;
				break;
			default:
				break;
			}

			family_type = font.family();

			switch (family_type) {
			case LyXFont::SANS_FAMILY:
				tag_open.push_back(SF);
				break;
			case LyXFont::TYPEWRITER_FAMILY:
				tag_open.push_back(TT);
				break;
			default:
				break;
			}
		}

		if (font_old.series() != font.series()) {
			switch (series_type) {
			case LyXFont::BOLD_SERIES:
				tag_close |= BF;
				break;
			default:
				break;
			}

			series_type = font.series();

			switch (series_type) {
			case LyXFont::BOLD_SERIES:
				tag_open.push_back(BF);
				break;
			default:
				break;
			}

		}

		if (font_old.shape() != font.shape()) {
			switch (shape_type) {
			case LyXFont::ITALIC_SHAPE:
				tag_close |= IT;
				break;
			case LyXFont::SLANTED_SHAPE:
				tag_close |= SL;
				break;
			default:
				break;
			}

			shape_type = font.shape();

			switch (shape_type) {
			case LyXFont::ITALIC_SHAPE:
				tag_open.push_back(IT);
				break;
			case LyXFont::SLANTED_SHAPE:
				tag_open.push_back(SL);
				break;
			default:
				break;
			}
		}
		// handle <em> tag
		if (font_old.emph() != font.emph()) {
			if (font.emph() == LyXFont::ON) {
				tag_open.push_back(EM);
				is_em = true;
			}
			else if (is_em) {
				tag_close |= EM;
				is_em = false;
			}
		}

		list < PAR_TAG > temp;
		while (!tag_state.empty() && tag_close) {
			PAR_TAG k =  tag_state.top();
			tag_state.pop();
			os << "</" << tag_name(k) << '>';
			if (tag_close & k)
				reset(tag_close,k);
			else
				temp.push_back(k);
		}

		for(list< PAR_TAG >::const_iterator j = temp.begin();
		    j != temp.end(); ++j) {
			tag_state.push(*j);
			os << '<' << tag_name(*j) << '>';
		}

		for(list< PAR_TAG >::const_iterator j = tag_open.begin();
		    j != tag_open.end(); ++j) {
			tag_state.push(*j);
			os << '<' << tag_name(*j) << '>';
		}

		char c = par->getChar(i);

		if (c == Paragraph::META_INSET) {
			Inset * inset = par->getInset(i);
			inset->linuxdoc(this, os);
			font_old = font;
			continue;
		}

		if (style->latexparam() == "CDATA") {
			// "TeX"-Mode on == > SGML-Mode on.
			if (c != '\0')
				os << c;
			++char_line_count;
		} else {
			bool ws;
			string str;
			boost::tie(ws, str) = sgml::escapeChar(c);
			if (ws && !par->isFreeSpacing()) {
				// in freespacing mode, spaces are
				// non-breaking characters
				if (desc_on) {// if char is ' ' then...

					++char_line_count;
					sgmlLineBreak(os, char_line_count, 6);
					os << "</tag>";
					desc_on = false;
				} else  {
					sgmlLineBreak(os, char_line_count, 1);
					os << c;
				}
			} else {
				os << str;
				char_line_count += str.length();
			}
		}
		font_old = font;
	}

	while (!tag_state.empty()) {
		os << "</" << tag_name(tag_state.top()) << '>';
		tag_state.pop();
	}

	// resets description flag correctly
	if (desc_on) {
		// <tag> not closed...
		sgmlLineBreak(os, char_line_count, 6);
		os << "</tag>";
	}
}


void Buffer::makeDocBookFile(string const & fname, bool nice, bool only_body)
{
	ofstream ofs(fname.c_str());
	if (!ofs) {
		string const file = MakeDisplayPath(fname, 50);
		string text = bformat(_("Could not save the specified document\n%1$s.\n"),
			file);
		Alert::error(_("Could not save document"), text);
		return;
	}

	niceFile = nice; // this will be used by Insetincludes.

	LaTeXFeatures features(params);
	validate(features);

	texrow.reset();

	LyXTextClass const & tclass = params.getLyXTextClass();
	string top_element = tclass.latexname();

	if (!only_body) {
		ofs << "<!DOCTYPE " << top_element
		    << "  PUBLIC \"-//OASIS//DTD DocBook V4.1//EN\"";

		string preamble = params.preamble;
		string const name = nice ? ChangeExtension(filename_, ".sgml")
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
	top += params.language->code();
	top += '"';

	if (!params.options.empty()) {
		top += ' ';
		top += params.options;
	}
	sgml::openTag(ofs, 0, false, top);

	ofs << "<!-- DocBook file was created by " << lyx_docversion
	    << "\n  See http://www.lyx.org/ for more information -->\n";

	vector<string> environment_stack(10);
	vector<string> environment_inner(10);
	vector<string> command_stack(10);

	bool command_flag = false;
	Paragraph::depth_type command_depth = 0;
	Paragraph::depth_type command_base = 0;
	Paragraph::depth_type cmd_depth = 0;
	Paragraph::depth_type depth = 0; // paragraph depth

	string item_name;
	string command_name;

	ParagraphList::iterator par = paragraphs.begin();
	ParagraphList::iterator pend = paragraphs.end();

	for (; par != pend; ++par) {
		string sgmlparam;
		string c_depth;
		string c_params;
		int desc_on = 0; // description mode

		LyXLayout_ptr const & style = par->layout();

		// environment tag closing
		for (; depth > par->params().depth(); --depth) {
			if (environment_inner[depth] != "!-- --" && !environment_inner[depth].empty()) {
				item_name = "listitem";
				sgml::closeTag(ofs, command_depth + depth, false, item_name);
				if (environment_inner[depth] == "varlistentry")
					sgml::closeTag(ofs, depth+command_depth, false, environment_inner[depth]);
			}
			sgml::closeTag(ofs, depth + command_depth, false, environment_stack[depth]);
			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		if (depth == par->params().depth()
		   && environment_stack[depth] != style->latexname()
		   && !environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name= "listitem";
				sgml::closeTag(ofs, command_depth+depth, false, item_name);
				if (environment_inner[depth] == "varlistentry")
					sgml::closeTag(ofs, depth + command_depth, false, environment_inner[depth]);
			}

			sgml::closeTag(ofs, depth + command_depth, false, environment_stack[depth]);

			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		// Write opening SGML tags.
		switch (style->latextype) {
		case LATEX_PARAGRAPH:
			sgml::openTag(ofs, depth + command_depth,
				    false, style->latexname());
			break;

		case LATEX_COMMAND:
			if (depth != 0)
				parseError(ErrorItem(_("Error"), _("Wrong depth for LatexType Command."), par->id(), 0, par->size()));

			command_name = style->latexname();

			sgmlparam = style->latexparam();
			c_params = split(sgmlparam, c_depth,'|');

			cmd_depth = atoi(c_depth);

			if (command_flag) {
				if (cmd_depth < command_base) {
					for (Paragraph::depth_type j = command_depth;
					     j >= command_base; --j) {
						sgml::closeTag(ofs, j, false, command_stack[j]);
						ofs << endl;
					}
					command_depth = command_base = cmd_depth;
				} else if (cmd_depth <= command_depth) {
					for (int j = command_depth;
					     j >= int(cmd_depth); --j) {
						sgml::closeTag(ofs, j, false, command_stack[j]);
						ofs << endl;
					}
					command_depth = cmd_depth;
				} else
					command_depth = cmd_depth;
			} else {
				command_depth = command_base = cmd_depth;
				command_flag = true;
			}
			if (command_stack.size() == command_depth + 1)
				command_stack.push_back(string());
			command_stack[command_depth] = command_name;

			// treat label as a special case for
			// more WYSIWYM handling.
			// This is a hack while paragraphs can't have
			// attributes, like id in this case.
			if (par->isInset(0)) {
				Inset * inset = par->getInset(0);
				Inset::Code lyx_code = inset->lyxCode();
				if (lyx_code == Inset::LABEL_CODE) {
					command_name += " id=\"";
					command_name += (static_cast<InsetCommand *>(inset))->getContents();
					command_name += '"';
					desc_on = 3;
				}
			}

			sgml::openTag(ofs, depth + command_depth, false, command_name);

			item_name = c_params.empty() ? "title" : c_params;
			sgml::openTag(ofs, depth + 1 + command_depth, false, item_name);
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (depth < par->params().depth()) {
				depth = par->params().depth();
				environment_stack[depth].erase();
			}

			if (environment_stack[depth] != style->latexname()) {
				if (environment_stack.size() == depth + 1) {
					environment_stack.push_back("!-- --");
					environment_inner.push_back("!-- --");
				}
				environment_stack[depth] = style->latexname();
				environment_inner[depth] = "!-- --";
				sgml::openTag(ofs, depth + command_depth, false, environment_stack[depth]);
			} else {
				if (environment_inner[depth] != "!-- --") {
					item_name= "listitem";
					sgml::closeTag(ofs, command_depth + depth, false, item_name);
					if (environment_inner[depth] == "varlistentry")
						sgml::closeTag(ofs, depth + command_depth, false, environment_inner[depth]);
				}
			}

			if (style->latextype == LATEX_ENVIRONMENT) {
				if (!style->latexparam().empty()) {
					if (style->latexparam() == "CDATA")
						ofs << "<![CDATA[";
					else
						sgml::openTag(ofs, depth + command_depth, false, style->latexparam());
				}
				break;
			}

			desc_on = (style->labeltype == LABEL_MANUAL);

			environment_inner[depth] = desc_on ? "varlistentry" : "listitem";
			sgml::openTag(ofs, depth + 1 + command_depth,
				    false, environment_inner[depth]);

			item_name = desc_on ? "term" : "para";
			sgml::openTag(ofs, depth + 1 + command_depth,
				    false, item_name);
			break;
		default:
			sgml::openTag(ofs, depth + command_depth,
				    false, style->latexname());
			break;
		}

		simpleDocBookOnePar(ofs, par, desc_on,
				    depth + 1 + command_depth);

		string end_tag;
		// write closing SGML tags
		switch (style->latextype) {
		case LATEX_COMMAND:
			end_tag = c_params.empty() ? "title" : c_params;
			sgml::closeTag(ofs, depth + command_depth,
				     false, end_tag);
			break;
		case LATEX_ENVIRONMENT:
			if (!style->latexparam().empty()) {
				if (style->latexparam() == "CDATA")
					ofs << "]]>";
				else
					sgml::closeTag(ofs, depth + command_depth, false, style->latexparam());
			}
			break;
		case LATEX_ITEM_ENVIRONMENT:
			if (desc_on == 1) break;
			end_tag = "para";
			sgml::closeTag(ofs, depth + 1 + command_depth, false, end_tag);
			break;
		case LATEX_PARAGRAPH:
			sgml::closeTag(ofs, depth + command_depth, false, style->latexname());
			break;
		default:
			sgml::closeTag(ofs, depth + command_depth, false, style->latexname());
			break;
		}
	}

	// Close open tags
	for (int d = depth; d >= 0; --d) {
		if (!environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				sgml::closeTag(ofs, command_depth + depth, false, item_name);
			       if (environment_inner[depth] == "varlistentry")
				       sgml::closeTag(ofs, depth + command_depth, false, environment_inner[depth]);
			}

			sgml::closeTag(ofs, depth + command_depth, false, environment_stack[depth]);
		}
	}

	for (int j = command_depth; j >= 0 ; --j)
		if (!command_stack[j].empty()) {
			sgml::closeTag(ofs, j, false, command_stack[j]);
			ofs << endl;
		}

	ofs << "\n\n";
	sgml::closeTag(ofs, 0, false, top_element);

	ofs.close();
	// How to check for successful close

	// we want this to be true outside previews (for insetexternal)
	niceFile = true;
}


void Buffer::simpleDocBookOnePar(ostream & os,
				 ParagraphList::iterator par, int & desc_on,
				 Paragraph::depth_type depth) const
{
	bool emph_flag = false;

	LyXLayout_ptr const & style = par->layout();

	LyXFont font_old = (style->labeltype == LABEL_MANUAL ? style->labelfont : style->font);

	int char_line_count = depth;
	//if (!style.free_spacing)
	//	os << string(depth,' ');

	// parsing main loop
	for (pos_type i = 0; i < par->size(); ++i) {
		LyXFont font = par->getFont(params, i, outerFont(par, paragraphs));

		// handle <emphasis> tag
		if (font_old.emph() != font.emph()) {
			if (font.emph() == LyXFont::ON) {
				if (style->latexparam() == "CDATA")
					os << "]]>";
				os << "<emphasis>";
				if (style->latexparam() == "CDATA")
					os << "<![CDATA[";
				emph_flag = true;
			} else if (i) {
				if (style->latexparam() == "CDATA")
					os << "]]>";
				os << "</emphasis>";
				if (style->latexparam() == "CDATA")
					os << "<![CDATA[";
				emph_flag = false;
			}
		}


		if (par->isInset(i)) {
			Inset * inset = par->getInset(i);
			// don't print the inset in position 0 if desc_on == 3 (label)
			if (i || desc_on != 3) {
				if (style->latexparam() == "CDATA")
					os << "]]>";
				inset->docbook(this, os, false);
				if (style->latexparam() == "CDATA")
					os << "<![CDATA[";
			}
		} else {
			char c = par->getChar(i);
			bool ws;
			string str;
			boost::tie(ws, str) = sgml::escapeChar(c);

			if (style->pass_thru) {
				os << c;
			} else if (par->isFreeSpacing() || c != ' ') {
					os << str;
			} else if (desc_on == 1) {
				++char_line_count;
				os << "\n</term><listitem><para>";
				desc_on = 2;
			} else {
				os << ' ';
			}
		}
		font_old = font;
	}

	if (emph_flag) {
		if (style->latexparam() == "CDATA")
			os << "]]>";
		os << "</emphasis>";
		if (style->latexparam() == "CDATA")
			os << "<![CDATA[";
	}

	// resets description flag correctly
	if (desc_on == 1) {
		// <term> not closed...
		os << "</term>\n<listitem><para>&nbsp;</para>";
	}
	if (style->free_spacing)
		os << '\n';
}


// chktex should be run with these flags disabled: 3, 22, 25, 30, 38(?)
// Other flags: -wall -v0 -x
int Buffer::runChktex()
{
	if (!users->text) return 0;

	users->owner()->busy(true);

	// get LaTeX-Filename
	string const name = getLatexName();
	string path = filePath();

	string const org_path = path;
	if (lyxrc.use_tempdir || !IsDirWriteable(path)) {
		path = tmppath;
	}

	Path p(path); // path to LaTeX file
	users->owner()->message(_("Running chktex..."));

	// Generate the LaTeX file if neccessary
	LatexRunParams runparams;
	runparams.flavor = LatexRunParams::LATEX;
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
		parseErrors(*this, terr);
	}

	users->owner()->busy(false);

	return res;
}


void Buffer::validate(LaTeXFeatures & features) const
{
	LyXTextClass const & tclass = params.getLyXTextClass();

	if (params.tracking_changes) {
		features.require("dvipost");
		features.require("color");
	}

	// AMS Style is at document level
	if (params.use_amsmath == BufferParams::AMS_ON
	    || tclass.provides(LyXTextClass::amsmath))
		features.require("amsmath");

	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::validate, _1, boost::ref(features)));

	// the bullet shapes are buffer level not paragraph level
	// so they are tested here
	for (int i = 0; i < 4; ++i) {
		if (params.user_defined_bullets[i] != ITEMIZE_DEFAULTS[i]) {
			int const font = params.user_defined_bullets[i].getFont();
			if (font == 0) {
				int const c = params
					.user_defined_bullets[i]
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
	if (!params.parentname.empty()
	    && bufferlist.exists(params.parentname)) {
		Buffer const * tmp = bufferlist.getBuffer(params.parentname);
		if (tmp) {
			tmp->getLabelList(list);
			return;
		}
	}

	for (inset_iterator it = inset_const_iterator_begin();
	     it != inset_const_iterator_end(); ++it) {
		it->getLabelList(list);
	}
}


// This is also a buffer property (ale)
void Buffer::fillWithBibKeys(std::vector<std::pair<string, string> > & keys) const
{
	/// if this is a child document and the parent is already loaded
	/// use the parent's list instead  [ale990412]
	if (!params.parentname.empty() && bufferlist.exists(params.parentname)) {
		Buffer const * tmp = bufferlist.getBuffer(params.parentname);
		if (tmp) {
			tmp->fillWithBibKeys(keys);
			return;
		}
	}

	for (inset_iterator it = inset_const_iterator_begin();
		it != inset_const_iterator_end(); ++it) {
		if (it->lyxCode() == Inset::BIBTEX_CODE)
			static_cast<InsetBibtex &>(*it).fillWithBibKeys(this, keys);
		else if (it->lyxCode() == Inset::INCLUDE_CODE)
			static_cast<InsetInclude &>(*it).fillWithBibKeys(keys);
		else if (it->lyxCode() == Inset::BIBITEM_CODE) {
			InsetBibitem & bib = static_cast<InsetBibitem &>(*it);
			string const key = bib.getContents();
			string const opt = bib.getOptions();
			string const ref; // = pit->asString(this, false);
			string const info = opt + "TheBibliographyRef" + ref;
			keys.push_back(pair<string, string>(key, info));
		}
	}
}


bool Buffer::isDepClean(string const & name) const
{
	DepClean::const_iterator it = dep_clean_.find(name);
	if (it == dep_clean_.end())
		return true;
	return it->second;
}


void Buffer::markDepClean(string const & name)
{
	dep_clean_[name] = true;
}


bool Buffer::dispatch(string const & command, bool * result)
{
	// Split command string into command and argument
	string cmd;
	string line = ltrim(command);
	string const arg = trim(split(line, cmd, ' '));

	return dispatch(lyxaction.LookupFunc(cmd), arg, result);
}


bool Buffer::dispatch(int action, string const & argument, bool * result)
{
	bool dispatched = true;

	switch (action) {
		case LFUN_EXPORT: {
			bool const tmp = Exporter::Export(this, argument, false);
			if (result)
				*result = tmp;
			break;
		}

		default:
			dispatched = false;
	}
	return dispatched;
}


void Buffer::resizeInsets(BufferView * bv)
{
	/// then remove all LyXText in text-insets
	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::resizeInsetsLyXText, _1, bv));
}


void Buffer::redraw()
{
#warning repaint needed here, or do you mean update() ?
	users->repaint();
	users->fitCursor();
}


void Buffer::changeLanguage(Language const * from, Language const * to)
{
	lyxerr << "Changing Language!" << endl;

	// Take care of l10n/i18n
	updateDocLang(to);

	ParIterator end = par_iterator_end();
	for (ParIterator it = par_iterator_begin(); it != end; ++it)
		it->changeLanguage(params, from, to);
}


void Buffer::updateDocLang(Language const * nlang)
{
	messages_.reset(new Messages(nlang->code()));
}


bool Buffer::isMultiLingual()
{
	ParIterator end = par_iterator_end();
	for (ParIterator it = par_iterator_begin(); it != end; ++it)
		if (it->isMultiLingual(params))
			return true;

	return false;
}


void Buffer::inset_iterator::setParagraph()
{
	while (pit != pend) {
		it = pit->insetlist.begin();
		if (it != pit->insetlist.end())
			return;
		++pit;
	}
}


Inset * Buffer::getInsetFromID(int id_arg) const
{
	for (inset_iterator it = inset_const_iterator_begin();
		 it != inset_const_iterator_end(); ++it)
	{
		if (it->id() == id_arg)
			return &(*it);
		Inset * in = it->getInsetFromID(id_arg);
		if (in)
			return in;
	}
	return 0;
}


ParIterator Buffer::getParFromID(int id) const
{
#warning FIXME: const correctness! (Andre)
	ParIterator it = const_cast<Buffer*>(this)->par_iterator_begin();
	ParIterator end = const_cast<Buffer*>(this)->par_iterator_end();

#warning FIXME, perhaps this func should return a ParIterator? (Lgb)
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
	return ParIterator(paragraphs.begin(), paragraphs);
}


ParIterator Buffer::par_iterator_end()
{
	return ParIterator(paragraphs.end(), paragraphs);
}

ParConstIterator Buffer::par_iterator_begin() const
{
	return ParConstIterator(const_cast<ParagraphList&>(paragraphs).begin(), paragraphs);
}


ParConstIterator Buffer::par_iterator_end() const
{
	return ParConstIterator(const_cast<ParagraphList&>(paragraphs).end(), paragraphs);
}



void Buffer::addUser(BufferView * u)
{
	users = u;
}


void Buffer::delUser(BufferView *)
{
	users = 0;
}


Language const * Buffer::getLanguage() const
{
	return params.language;
}


string const Buffer::B_(string const & l10n) const
{
	if (messages_.get()) {
		return messages_->get(l10n);
	}

	return _(l10n);
}


bool Buffer::isClean() const
{
	return lyx_clean;
}


bool Buffer::isBakClean() const
{
	return bak_clean;
}


void Buffer::markClean() const
{
	if (!lyx_clean) {
		lyx_clean = true;
		updateTitles();
	}
	// if the .lyx file has been saved, we don't need an
	// autosave
	bak_clean = true;
}


void Buffer::markBakClean()
{
	bak_clean = true;
}


void Buffer::setUnnamed(bool flag)
{
	unnamed = flag;
}


bool Buffer::isUnnamed()
{
	return unnamed;
}


void Buffer::markDirty()
{
	if (lyx_clean) {
		lyx_clean = false;
		updateTitles();
	}
	bak_clean = false;

	DepClean::iterator it = dep_clean_.begin();
	DepClean::const_iterator const end = dep_clean_.end();

	for (; it != end; ++it) {
		it->second = false;
	}
}


string const & Buffer::fileName() const
{
	return filename_;
}


string const & Buffer::filePath() const
{
	return filepath_;
}


bool Buffer::isReadonly() const
{
	return read_only;
}


BufferView * Buffer::getUser() const
{
	return users;
}


void Buffer::setParentName(string const & name)
{
	params.parentname = name;
}


Buffer::inset_iterator::inset_iterator()
	: pit(), pend()
{}


Buffer::inset_iterator::inset_iterator(base_type p, base_type e)
	: pit(p), pend(e)
{
	setParagraph();
}


Buffer::inset_iterator Buffer::inset_iterator_begin()
{
	return inset_iterator(paragraphs.begin(), paragraphs.end());
}


Buffer::inset_iterator Buffer::inset_iterator_end()
{
	return inset_iterator(paragraphs.end(), paragraphs.end());
}


Buffer::inset_iterator Buffer::inset_const_iterator_begin() const
{
	return inset_iterator(const_cast<ParagraphList&>(paragraphs).begin(),
	                      const_cast<ParagraphList&>(paragraphs).end());
}


Buffer::inset_iterator Buffer::inset_const_iterator_end() const
{
	return inset_iterator(const_cast<ParagraphList&>(paragraphs).end(),
	                      const_cast<ParagraphList&>(paragraphs).end());
}


Buffer::inset_iterator & Buffer::inset_iterator::operator++()
{
	if (pit != pend) {
		++it;
		if (it == pit->insetlist.end()) {
			++pit;
			setParagraph();
		}
	}
	return *this;
}


Buffer::inset_iterator Buffer::inset_iterator::operator++(int)
{
	inset_iterator tmp = *this;
	++*this;
	return tmp;
}


Buffer::inset_iterator::reference Buffer::inset_iterator::operator*()
{
	return *it->inset;
}


Buffer::inset_iterator::pointer Buffer::inset_iterator::operator->()
{
	return it->inset;
}


ParagraphList::iterator Buffer::inset_iterator::getPar() const
{
	return pit;
}


lyx::pos_type Buffer::inset_iterator::getPos() const
{
	return it->pos;
}


bool operator==(Buffer::inset_iterator const & iter1,
		Buffer::inset_iterator const & iter2)
{
	return iter1.pit == iter2.pit
		&& (iter1.pit == iter1.pend || iter1.it == iter2.it);
}


bool operator!=(Buffer::inset_iterator const & iter1,
		Buffer::inset_iterator const & iter2)
{
	return !(iter1 == iter2);
}
