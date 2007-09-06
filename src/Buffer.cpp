/**
 * \file Buffer.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Buffer.h"

#include "Author.h"
#include "BiblioInfo.h"
#include "BranchList.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "Counters.h"
#include "Bullet.h"
#include "Chktex.h"
#include "debug.h"
#include "DocIterator.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Exporter.h"
#include "Format.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "InsetIterator.h"
#include "Language.h"
#include "LaTeX.h"
#include "LaTeXFeatures.h"
#include "LyXAction.h"
#include "Lexer.h"
#include "Text.h"
#include "LyX.h"
#include "LyXRC.h"
#include "LyXVC.h"
#include "Messages.h"
#include "output.h"
#include "output_docbook.h"
#include "output_latex.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "sgml.h"
#include "TexRow.h"
#include "TextClassList.h"
#include "TexStream.h"
#include "TocBackend.h"
#include "Undo.h"
#include "version.h"
#include "EmbeddedFiles.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetBibtex.h"
#include "insets/InsetInclude.h"
#include "insets/InsetText.h"

#include "mathed/MathMacroTemplate.h"
#include "mathed/MacroTable.h"
#include "mathed/MathSupport.h"

#include "frontends/alert.h"

#include "graphics/Previews.h"

#include "support/types.h"
#include "support/lyxalgo.h"
#include "support/filetools.h"
#include "support/fs_extras.h"
#include "support/gzstream.h"
#include "support/lyxlib.h"
#include "support/os.h"
#include "support/Path.h"
#include "support/textutils.h"
#include "support/convert.h"

#include <boost/bind.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <algorithm>
#include <iomanip>
#include <stack>
#include <sstream>
#include <fstream>

using std::endl;
using std::for_each;
using std::make_pair;

using std::ios;
using std::map;
using std::ostream;
using std::ostringstream;
using std::ofstream;
using std::ifstream;
using std::pair;
using std::stack;
using std::vector;
using std::string;
using std::time_t;


namespace lyx {

using support::addName;
using support::bformat;
using support::changeExtension;
using support::cmd_ret;
using support::createBufferTmpDir;
using support::destroyDir;
using support::FileName;
using support::getFormatFromContents;
using support::libFileSearch;
using support::latex_path;
using support::ltrim;
using support::makeAbsPath;
using support::makeDisplayPath;
using support::makeLatexName;
using support::onlyFilename;
using support::onlyPath;
using support::quoteName;
using support::removeAutosaveFile;
using support::rename;
using support::runCommand;
using support::split;
using support::subst;
using support::tempName;
using support::trim;
using support::sum;
using support::suffixIs;

namespace Alert = frontend::Alert;
namespace os = support::os;
namespace fs = boost::filesystem;

namespace {

int const LYX_FORMAT = 282;

} // namespace anon


typedef std::map<string, bool> DepClean;

class Buffer::Impl
{
public:
	Impl(Buffer & parent, FileName const & file, bool readonly);

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
	FileName filename;

	/** Set to true only when the file is fully loaded.
	 *  Used to prevent the premature generation of previews
	 *  and by the citation inset.
	 */
	bool file_fully_loaded;

	/// our Text that should be wrapped in an InsetText
	InsetText inset;

	///
	MacroTable macros;

	///
	TocBackend toc_backend;

	/// Container for all sort of Buffer dependant errors.
	map<string, ErrorList> errorLists;

	/// all embedded files of this buffer
	EmbeddedFiles embedded_files;

	/// timestamp and checksum used to test if the file has been externally
	/// modified. (Used to properly enable 'File->Revert to saved', bug 4114).
	time_t timestamp_;
	unsigned long checksum_;
};


Buffer::Impl::Impl(Buffer & parent, FileName const & file, bool readonly_)
	: lyx_clean(true), bak_clean(true), unnamed(false), read_only(readonly_),
	  filename(file), file_fully_loaded(false), inset(params),
	  toc_backend(&parent), embedded_files(&parent), timestamp_(0), checksum_(0)
{
	inset.setAutoBreakRows(true);
	lyxvc.buffer(&parent);
	temppath = createBufferTmpDir();
	params.filepath = onlyPath(file.absFilename());
	// FIXME: And now do something if temppath == string(), because we
	// assume from now on that temppath points to a valid temp dir.
	// See http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg67406.html
}


Buffer::Buffer(string const & file, bool readonly)
	: pimpl_(new Impl(*this, FileName(file), readonly))
{
	LYXERR(Debug::INFO) << "Buffer::Buffer()" << endl;
}


Buffer::~Buffer()
{
	LYXERR(Debug::INFO) << "Buffer::~Buffer()" << endl;
	// here the buffer should take care that it is
	// saved properly, before it goes into the void.

	Buffer * master = getMasterBuffer();
	if (master != this && use_gui)
		// We are closing buf which was a child document so we
		// must update the labels and section numbering of its master
		// Buffer.
		updateLabels(*master);

	if (!temppath().empty() && !destroyDir(FileName(temppath()))) {
		Alert::warning(_("Could not remove temporary directory"),
			bformat(_("Could not remove the temporary directory %1$s"),
			from_utf8(temppath())));
	}

	// Remove any previewed LaTeX snippets associated with this buffer.
	graphics::Previews::get().removeLoader(*this);

	closing(this);
}


Text & Buffer::text() const
{
	return const_cast<Text &>(pimpl_->inset.text_);
}


Inset & Buffer::inset() const
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


TocBackend & Buffer::tocBackend()
{
	return pimpl_->toc_backend;
}


TocBackend const & Buffer::tocBackend() const
{
	return pimpl_->toc_backend;
}


EmbeddedFiles & Buffer::embeddedFiles()
{
	return pimpl_->embedded_files;
}


EmbeddedFiles const & Buffer::embeddedFiles() const
{
	return pimpl_->embedded_files;
}


string const Buffer::getLatexName(bool const no_path) const
{
	string const name = changeExtension(makeLatexName(fileName()), ".tex");
	return no_path ? onlyFilename(name) : name;
}


pair<Buffer::LogType, string> const Buffer::getLogName() const
{
	string const filename = getLatexName(false);

	if (filename.empty())
		return make_pair(Buffer::latexlog, string());

	string const path = temppath();

	FileName const fname(addName(temppath(),
				     onlyFilename(changeExtension(filename,
								  ".log"))));
	FileName const bname(
		addName(path, onlyFilename(
			changeExtension(filename,
					formats.extension("literate") + ".out"))));

	// If no Latex log or Build log is newer, show Build log

	if (fs::exists(bname.toFilesystemEncoding()) &&
	    (!fs::exists(fname.toFilesystemEncoding()) ||
	     fs::last_write_time(fname.toFilesystemEncoding()) < fs::last_write_time(bname.toFilesystemEncoding()))) {
		LYXERR(Debug::FILES) << "Log name calculated as: " << bname << endl;
		return make_pair(Buffer::buildlog, bname.absFilename());
	}
	LYXERR(Debug::FILES) << "Log name calculated as: " << fname << endl;
	return make_pair(Buffer::latexlog, fname.absFilename());
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
	pimpl_->filename = makeAbsPath(newfile);
	params().filepath = onlyPath(pimpl_->filename.absFilename());
	setReadonly(fs::is_readonly(pimpl_->filename.toFilesystemEncoding()));
	updateTitles();
}


// We'll remove this later. (Lgb)
namespace {

void unknownClass(string const & unknown)
{
	Alert::warning(_("Unknown document class"),
		       bformat(_("Using the default document class, because the "
					      "class %1$s is unknown."), from_utf8(unknown)));
}

} // anon


int Buffer::readHeader(Lexer & lex)
{
	int unknown_tokens = 0;
	int line = -1;
	int begin_header_line = -1;

	// Initialize parameters that may be/go lacking in header:
	params().branchlist().clear();
	params().preamble.erase();
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
	params().listings_params.clear();
	params().clearLayoutModules();
	
	for (int i = 0; i < 4; ++i) {
		params().user_defined_bullet(i) = ITEMIZE_DEFAULTS[i];
		params().temp_bullet(i) = ITEMIZE_DEFAULTS[i];
	}

	ErrorList & errorList = pimpl_->errorLists["Parse"];

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

		LYXERR(Debug::PARSER) << "Handling document header token: `"
				      << token << '\'' << endl;

		string unknown = params().readToken(lex, token);
		if (!unknown.empty()) {
			if (unknown[0] != '\\' && token == "\\textclass") {
				unknownClass(unknown);
			} else {
				++unknown_tokens;
				docstring const s = bformat(_("Unknown token: "
									"%1$s %2$s\n"),
							 from_utf8(token),
							 lex.getDocString());
				errorList.push_back(ErrorItem(_("Document header error"),
					s, -1, 0, 0));
			}
		}
	}
	if (begin_header_line) {
		docstring const s = _("\\begin_header is missing");
		errorList.push_back(ErrorItem(_("Document header error"),
			s, -1, 0, 0));
	}

	return unknown_tokens;
}


// Uwe C. Schroeder
// changed to be public and have one parameter
// Returns false if "\end_document" is not read (Asger)
bool Buffer::readDocument(Lexer & lex)
{
	ErrorList & errorList = pimpl_->errorLists["Parse"];
	errorList.clear();

	lex.next();
	string const token = lex.getString();
	if (token != "\\begin_document") {
		docstring const s = _("\\begin_document is missing");
		errorList.push_back(ErrorItem(_("Document header error"),
			s, -1, 0, 0));
	}

	// we are reading in a brand new document
	BOOST_ASSERT(paragraphs().empty());

	readHeader(lex);
	if (!params().getTextClass().load(filePath())) {
		string theclass = params().getTextClass().name();
		Alert::error(_("Can't load document class"), bformat(
			_("Using the default document class, because the "
				     "class %1$s could not be loaded."), from_utf8(theclass)));
		params().setBaseClass(defaultTextclass());
	}

	if (params().outputChanges) {
		bool dvipost    = LaTeXFeatures::isAvailable("dvipost");
		bool xcolorsoul = LaTeXFeatures::isAvailable("soul") &&
				  LaTeXFeatures::isAvailable("xcolor");

		if (!dvipost && !xcolorsoul) {
			Alert::warning(_("Changes not shown in LaTeX output"),
				       _("Changes will not be highlighted in LaTeX output, "
					 "because neither dvipost nor xcolor/soul are installed.\n"
					 "Please install these packages or redefine "
					 "\\lyxadded and \\lyxdeleted in the LaTeX preamble."));
		} else if (!xcolorsoul) {
			Alert::warning(_("Changes not shown in LaTeX output"),
				       _("Changes will not be highlighted in LaTeX output "
					 "when using pdflatex, because xcolor and soul are not installed.\n"
					 "Please install both packages or redefine "
					 "\\lyxadded and \\lyxdeleted in the LaTeX preamble."));
		}
	}

	bool const res = text().read(*this, lex, errorList);
	for_each(text().paragraphs().begin(),
		 text().paragraphs().end(),
		 bind(&Paragraph::setInsetOwner, _1, &inset()));

	return res;
}


// needed to insert the selection
void Buffer::insertStringAsLines(ParagraphList & pars,
	pit_type & pit, pos_type & pos,
	Font const & fn, docstring const & str, bool autobreakrows)
{
	Font font = fn;

	// insert the string, don't insert doublespace
	bool space_inserted = true;
	for (docstring::const_iterator cit = str.begin();
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
				par.insertChar(pos, ' ', font, params().trackChanges);
				++pos;
				space_inserted = true;
			} else {
				const pos_type n = 8 - pos % 8;
				for (pos_type i = 0; i < n; ++i) {
					par.insertChar(pos, ' ', font, params().trackChanges);
					++pos;
				}
				space_inserted = true;
			}
		} else if (!isPrintable(*cit)) {
			// Ignore unprintables
			continue;
		} else {
			// just insert the character
			par.insertChar(pos, *cit, font, params().trackChanges);
			++pos;
			space_inserted = (*cit == ' ');
		}

	}
}


bool Buffer::readString(std::string const & s)
{
	params().compressed = false;

	// remove dummy empty par
	paragraphs().clear();
	Lexer lex(0, 0);
	std::istringstream is(s);
	lex.setStream(is);
	FileName const name(tempName());
	switch (readFile(lex, name, true)) {
	case failure:
		return false;
	case wrongversion: {
		// We need to call lyx2lyx, so write the input to a file
		std::ofstream os(name.toFilesystemEncoding().c_str());
		os << s;
		os.close();
		return readFile(name);
	}
	case success:
		break;
	}

	return true;
}


bool Buffer::readFile(FileName const & filename)
{
	FileName fname(filename);
	// Check if the file is compressed.
	string format = getFormatFromContents(filename);
	if (format == "zip") {
		// decompress to a temp directory
		LYXERR(Debug::FILES) << filename << " is in zip format. Unzip to " << temppath() << endl;
		::unzipToDir(filename.toFilesystemEncoding(), temppath());
		//
		FileName manifest(addName(temppath(), "manifest.txt"));
		FileName lyxfile(addName(temppath(), 
			onlyFilename(filename.toFilesystemEncoding())));
		// if both manifest.txt and file.lyx exist, this is am embedded file
		if (fs::exists(manifest.toFilesystemEncoding()) &&
			fs::exists(lyxfile.toFilesystemEncoding())) {
			params().embedded = true;
			fname = lyxfile;
			// read manifest file
			ifstream is(manifest.toFilesystemEncoding().c_str());
			is >> pimpl_->embedded_files;
			is.close();
			LYXERR(Debug::FILES) << filename << " is a embedded file. Its manifest is:\n"
					<< pimpl_->embedded_files;
		}
	}
	// The embedded lyx file can also be compressed, for backward compatibility
	format = getFormatFromContents(fname);
	if (format == "gzip" || format == "zip" || format == "compress") {
		params().compressed = true;
	}

	// remove dummy empty par
	paragraphs().clear();
	Lexer lex(0, 0);
	lex.setFile(fname);
	if (readFile(lex, fname) != success)
		return false;

	return true;
}


bool Buffer::fully_loaded() const
{
	return pimpl_->file_fully_loaded;
}


void Buffer::fully_loaded(bool const value)
{
	pimpl_->file_fully_loaded = value;
}


Buffer::ReadStatus Buffer::readFile(Lexer & lex, FileName const & filename,
		bool fromstring)
{
	BOOST_ASSERT(!filename.empty());

	if (!lex.isOK()) {
		Alert::error(_("Document could not be read"),
			     bformat(_("%1$s could not be read."), from_utf8(filename.absFilename())));
		return failure;
	}

	lex.next();
	string const token(lex.getString());

	if (!lex) {
		Alert::error(_("Document could not be read"),
			     bformat(_("%1$s could not be read."), from_utf8(filename.absFilename())));
		return failure;
	}

	// the first token _must_ be...
	if (token != "\\lyxformat") {
		lyxerr << "Token: " << token << endl;

		Alert::error(_("Document format failure"),
			     bformat(_("%1$s is not a LyX document."),
				       from_utf8(filename.absFilename())));
		return failure;
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

	// save timestamp and checksum of the original disk file, making sure
	// to not overwrite them with those of the file created in the tempdir
	// when it has to be converted to the current format.
	if (!pimpl_->checksum_) {
		// Save the timestamp and checksum of disk file. If filename is an
		// emergency file, save the timestamp and sum of the original lyx file
		// because isExternallyModified will check for this file. (BUG4193)
		string diskfile = filename.toFilesystemEncoding();
		if (suffixIs(diskfile, ".emergency"))
			diskfile = diskfile.substr(0, diskfile.size() - 10);
		saveCheckSum(diskfile);
	}

	if (file_format != LYX_FORMAT) {

		if (fromstring)
			// lyx2lyx would fail
			return wrongversion;

		FileName const tmpfile(tempName());
		if (tmpfile.empty()) {
			Alert::error(_("Conversion failed"),
				     bformat(_("%1$s is from a different"
					      " version of LyX, but a temporary"
					      " file for converting it could"
							    " not be created."),
					      from_utf8(filename.absFilename())));
			return failure;
		}
		FileName const lyx2lyx = libFileSearch("lyx2lyx", "lyx2lyx");
		if (lyx2lyx.empty()) {
			Alert::error(_("Conversion script not found"),
				     bformat(_("%1$s is from a different"
					       " version of LyX, but the"
					       " conversion script lyx2lyx"
							    " could not be found."),
					       from_utf8(filename.absFilename())));
			return failure;
		}
		ostringstream command;
		command << os::python()
			<< ' ' << quoteName(lyx2lyx.toFilesystemEncoding())
			<< " -t " << convert<string>(LYX_FORMAT)
			<< " -o " << quoteName(tmpfile.toFilesystemEncoding())
			<< ' ' << quoteName(filename.toFilesystemEncoding());
		string const command_str = command.str();

		LYXERR(Debug::INFO) << "Running '"
				    << command_str << '\''
				    << endl;

		cmd_ret const ret = runCommand(command_str);
		if (ret.first != 0) {
			Alert::error(_("Conversion script failed"),
				     bformat(_("%1$s is from a different version"
					      " of LyX, but the lyx2lyx script"
							    " failed to convert it."),
					      from_utf8(filename.absFilename())));
			return failure;
		} else {
			bool const ret = readFile(tmpfile);
			// Do stuff with tmpfile name and buffer name here.
			return ret ? success : failure;
		}

	}

	if (readDocument(lex)) {
		Alert::error(_("Document format failure"),
			     bformat(_("%1$s ended unexpectedly, which means"
						    " that it is probably corrupted."),
				       from_utf8(filename.absFilename())));
	}

	//lyxerr << "removing " << MacroTable::localMacros().size()
	//	<< " temporary macro entries" << endl;
	//MacroTable::localMacros().clear();

	pimpl_->file_fully_loaded = true;
	return success;
}


// Should probably be moved to somewhere else: BufferView? LyXView?
bool Buffer::save() const
{
	// We don't need autosaves in the immediate future. (Asger)
	resetAutosaveTimers();

	string const encodedFilename = pimpl_->filename.toFilesystemEncoding();

	FileName backupName;
	bool madeBackup = false;

	// make a backup if the file already exists
	if (lyxrc.make_backup && fs::exists(encodedFilename)) {
		backupName = FileName(fileName() + '~');
		if (!lyxrc.backupdir_path.empty())
			backupName = FileName(addName(lyxrc.backupdir_path,
					      subst(os::internal_path(backupName.absFilename()), '/', '!')));

		try {
			fs::copy_file(encodedFilename, backupName.toFilesystemEncoding(), false);
			madeBackup = true;
		} catch (fs::filesystem_error const & fe) {
			Alert::error(_("Backup failure"),
				     bformat(_("Cannot create backup file %1$s.\n"
					       "Please check whether the directory exists and is writeable."),
					     from_utf8(backupName.absFilename())));
			LYXERR(Debug::DEBUG) << "Fs error: " << fe.what() << endl;
		}
	}

	// ask if the disk file has been externally modified (use checksum method)
	if (fs::exists(encodedFilename) && isExternallyModified(checksum_method)) {
		docstring const file = makeDisplayPath(fileName(), 20);
		docstring text = bformat(_("Document %1$s has been externally modified. Are you sure "
							     "you want to overwrite this file?"), file);
		int const ret = Alert::prompt(_("Overwrite modified file?"),
			text, 1, 1, _("&Overwrite"), _("&Cancel"));
		if (ret == 1)
			return false;
	}

	if (writeFile(pimpl_->filename)) {
		markClean();
		removeAutosaveFile(fileName());
		saveCheckSum(pimpl_->filename.toFilesystemEncoding());
		return true;
	} else {
		// Saving failed, so backup is not backup
		if (madeBackup)
			rename(backupName, pimpl_->filename);
		return false;
	}
}


bool Buffer::writeFile(FileName const & fname) const
{
	if (pimpl_->read_only && fname == pimpl_->filename)
		return false;

	bool retval = false;

	FileName content;
	if (params().embedded)
		// first write the .lyx file to the temporary directory
		content = FileName(addName(temppath(), 
			onlyFilename(fname.toFilesystemEncoding())));
	else
		content = fname;
	
	if (params().compressed) {
		gz::ogzstream ofs(content.toFilesystemEncoding().c_str(), ios::out|ios::trunc);
		if (!ofs)
			return false;

		retval = write(ofs);
	} else {
		ofstream ofs(content.toFilesystemEncoding().c_str(), ios::out|ios::trunc);
		if (!ofs)
			return false;

		retval = write(ofs);
	}

	if (retval && params().embedded) {
		// write file.lyx and all the embedded files to the zip file fname
		// if embedding is enabled, and there is any embedded file
		pimpl_->embedded_files.update();
		return pimpl_->embedded_files.write(fname);
	}
	return retval;
}


bool Buffer::write(ostream & ofs) const
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


	/// For each author, set 'used' to true if there is a change
	/// by this author in the document; otherwise set it to 'false'.
	AuthorList::Authors::const_iterator a_it = params().authors().begin();
	AuthorList::Authors::const_iterator a_end = params().authors().end();
	for (; a_it != a_end; ++a_it)
		a_it->second.used(false);

	ParIterator const end = par_iterator_end();
	ParIterator it = par_iterator_begin();
	for ( ; it != end; ++it)
		it->checkAuthors(params().authors());

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


bool Buffer::makeLaTeXFile(FileName const & fname,
			   string const & original_path,
			   OutputParams const & runparams,
			   bool output_preamble, bool output_body)
{
	string const encoding = runparams.encoding->iconvName();
	LYXERR(Debug::LATEX) << "makeLaTeXFile encoding: "
		<< encoding << "..." << endl;

	odocfstream ofs(encoding);
	if (!openFileWrite(ofs, fname))
		return false;

	//TexStream ts(ofs.rdbuf(), &texrow());

	bool failed_export = false;
	try {
		texrow().reset();
		writeLaTeXSource(ofs, original_path,
		      runparams, output_preamble, output_body);
	}
	catch (iconv_codecvt_facet_exception & e) {
		lyxerr << "Caught iconv exception: " << e.what() << endl;
		failed_export = true;
	}
	catch (std::exception const & e) {
		lyxerr << "Caught \"normal\" exception: " << e.what() << endl;
		failed_export = true;
	}
	catch (...) {
		lyxerr << "Caught some really weird exception..." << endl;
		LyX::cref().emergencyCleanup();
		abort();
	}

	ofs.close();
	if (ofs.fail()) {
		failed_export = true;
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
	}

	if (failed_export) {
		Alert::error(_("Encoding error"),
			_("Some characters of your document are probably not "
			"representable in the chosen encoding.\n"
			"Changing the document encoding to utf8 could help."));
		return false;
	}
	return true;
}


void Buffer::writeLaTeXSource(odocstream & os,
			   string const & original_path,
			   OutputParams const & runparams_in,
			   bool const output_preamble, bool const output_body)
{
	OutputParams runparams = runparams_in;

	// validate the buffer.
	LYXERR(Debug::LATEX) << "  Validating buffer..." << endl;
	LaTeXFeatures features(*this, params(), runparams);
	validate(features);
	LYXERR(Debug::LATEX) << "  Buffer validation done." << endl;

	// The starting paragraph of the coming rows is the
	// first paragraph of the document. (Asger)
	if (output_preamble && runparams.nice) {
		os << "%% LyX " << lyx_version << " created this file.  "
			"For more info, see http://www.lyx.org/.\n"
			"%% Do not edit unless you really know what "
			"you are doing.\n";
		texrow().newline();
		texrow().newline();
	}
	LYXERR(Debug::INFO) << "lyx document header finished" << endl;
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
			// FIXME UNICODE
			// We don't know the encoding of inputpath
			docstring const inputpath = from_utf8(latex_path(original_path));
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

	texrow().start(paragraphs().begin()->id(), 0);
	
	LYXERR(Debug::INFO) << "preamble finished, now the body." << endl;

	if (!lyxrc.language_auto_begin &&
	    !params().language->babel().empty()) {
		// FIXME UNICODE
		os << from_utf8(subst(lyxrc.language_command_begin,
					   "$$lang",
					   params().language->babel()))
		   << '\n';
		texrow().newline();
	}

	Encoding const & encoding = params().encoding();
	if (encoding.package() == Encoding::CJK) {
		// Open a CJK environment, since in contrast to the encodings
		// handled by inputenc the document encoding is not set in
		// the preamble if it is handled by CJK.sty.
		os << "\\begin{CJK}{" << from_ascii(encoding.latexName())
		   << "}{}\n";
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

	loadChildDocuments(*this);

	// the real stuff
	latexParagraphs(*this, paragraphs(), os, texrow(), runparams);

	// Restore the parenthood if needed
	if (output_preamble)
		params().parentname = save_parentname;

	// add this just in case after all the paragraphs
	os << endl;
	texrow().newline();

	if (encoding.package() == Encoding::CJK) {
		// Close the open CJK environment.
		// latexParagraphs will have opened one even if the last text
		// was not CJK.
		os << "\\end{CJK}\n";
		texrow().newline();
	}

	if (!lyxrc.language_auto_end &&
	    !params().language->babel().empty()) {
		os << from_utf8(subst(lyxrc.language_command_end,
					   "$$lang",
					   params().language->babel()))
		   << '\n';
		texrow().newline();
	}

	if (output_preamble) {
		os << "\\end{document}\n";
		texrow().newline();

		LYXERR(Debug::LATEX) << "makeLaTeXFile...done" << endl;
	} else {
		LYXERR(Debug::LATEX) << "LaTeXFile for inclusion made."
				     << endl;
	}
	runparams_in.encoding = runparams.encoding;

	// Just to be sure. (Asger)
	texrow().newline();

	LYXERR(Debug::INFO) << "Finished making LaTeX file." << endl;
	LYXERR(Debug::INFO) << "Row count was " << texrow().rows() - 1
			    << '.' << endl;
}


bool Buffer::isLatex() const
{
	return params().getTextClass().outputType() == LATEX;
}


bool Buffer::isLiterate() const
{
	return params().getTextClass().outputType() == LITERATE;
}


bool Buffer::isDocBook() const
{
	return params().getTextClass().outputType() == DOCBOOK;
}


void Buffer::makeDocBookFile(FileName const & fname,
			      OutputParams const & runparams,
			      bool const body_only)
{
	LYXERR(Debug::LATEX) << "makeDocBookFile..." << endl;

	//ofstream ofs;
	odocfstream ofs;
	if (!openFileWrite(ofs, fname))
		return;

	writeDocBookSource(ofs, fname.absFilename(), runparams, body_only);

	ofs.close();
	if (ofs.fail())
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
}


void Buffer::writeDocBookSource(odocstream & os, string const & fname,
			     OutputParams const & runparams,
			     bool const only_body)
{
	LaTeXFeatures features(*this, params(), runparams);
	validate(features);

	texrow().reset();

	TextClass const & tclass = params().getTextClass();
	string const top_element = tclass.latexname();

	if (!only_body) {
		if (runparams.flavor == OutputParams::XML)
			os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

		// FIXME UNICODE
		os << "<!DOCTYPE " << from_ascii(top_element) << ' ';

		// FIXME UNICODE
		if (! tclass.class_header().empty())
			os << from_ascii(tclass.class_header());
		else if (runparams.flavor == OutputParams::XML)
			os << "PUBLIC \"-//OASIS//DTD DocBook XML//EN\" "
			    << "\"http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd\"";
		else
			os << " PUBLIC \"-//OASIS//DTD DocBook V4.2//EN\"";

		docstring preamble = from_utf8(params().preamble);
		if (runparams.flavor != OutputParams::XML ) {
			preamble += "<!ENTITY % output.print.png \"IGNORE\">\n";
			preamble += "<!ENTITY % output.print.pdf \"IGNORE\">\n";
			preamble += "<!ENTITY % output.print.eps \"IGNORE\">\n";
			preamble += "<!ENTITY % output.print.bmp \"IGNORE\">\n";
		}

		string const name = runparams.nice ? changeExtension(fileName(), ".sgml")
			 : fname;
		preamble += features.getIncludedFiles(name);
		preamble += features.getLyXSGMLEntities();

		if (!preamble.empty()) {
			os << "\n [ " << preamble << " ]";
		}
		os << ">\n\n";
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

	os << "<!-- " << ((runparams.flavor == OutputParams::XML)? "XML" : "SGML")
	    << " file was created by LyX " << lyx_version
	    << "\n  See http://www.lyx.org/ for more information -->\n";

	params().getTextClass().counters().reset();

	loadChildDocuments(*this);

	sgml::openTag(os, top);
	os << '\n';
	docbookParagraphs(paragraphs(), *this, os, runparams);
	sgml::closeTag(os, top_element);
}


// chktex should be run with these flags disabled: 3, 22, 25, 30, 38(?)
// Other flags: -wall -v0 -x
int Buffer::runChktex()
{
	busy(true);

	// get LaTeX-Filename
	FileName const path(temppath());
	string const name = addName(path.absFilename(), getLatexName());
	string const org_path = filePath();

	support::Path p(path); // path to LaTeX file
	message(_("Running chktex..."));

	// Generate the LaTeX file if neccessary
	OutputParams runparams(&params().encoding());
	runparams.flavor = OutputParams::LATEX;
	runparams.nice = false;
	makeLaTeXFile(FileName(name), org_path, runparams);

	TeXErrors terr;
	Chktex chktex(lyxrc.chktex_command, onlyFilename(name), filePath());
	int const res = chktex.run(terr); // run chktex

	if (res == -1) {
		Alert::error(_("chktex failure"),
			     _("Could not run chktex successfully."));
	} else if (res > 0) {
		ErrorList & errorList = pimpl_->errorLists["ChkTeX"];
		// Clear out old errors
		errorList.clear();
		// Fill-in the error list with the TeX errors
		bufferErrors(*this, terr, errorList);
	}

	busy(false);

	errors("ChkTeX");

	return res;
}


void Buffer::validate(LaTeXFeatures & features) const
{
	TextClass const & tclass = params().getTextClass();

	if (params().outputChanges) {
		bool dvipost    = LaTeXFeatures::isAvailable("dvipost");
		bool xcolorsoul = LaTeXFeatures::isAvailable("soul") &&
				  LaTeXFeatures::isAvailable("xcolor");

		if (features.runparams().flavor == OutputParams::LATEX) {
			if (dvipost) {
				features.require("ct-dvipost");
				features.require("dvipost");
			} else if (xcolorsoul) {
				features.require("ct-xcolor-soul");
				features.require("soul");
				features.require("xcolor");
			} else {
				features.require("ct-none");
			}
		} else if (features.runparams().flavor == OutputParams::PDFLATEX ) {
			if (xcolorsoul) {
				features.require("ct-xcolor-soul");
				features.require("soul");
				features.require("xcolor");
				features.require("pdfcolmk"); // improves color handling in PDF output
			} else {
				features.require("ct-none");
			}
		}
	}

	// AMS Style is at document level
	if (params().use_amsmath == BufferParams::package_on
	    || tclass.provides("amsmath"))
		features.require("amsmath");
	if (params().use_esint == BufferParams::package_on)
		features.require("esint");

	loadChildDocuments(*this);

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


void Buffer::getLabelList(vector<docstring> & list) const
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

	loadChildDocuments(*this);

	for (InsetIterator it = inset_iterator_begin(inset()); it; ++it)
		it.nextInset()->getLabelList(*this, list);
}


void Buffer::updateBibfilesCache()
{
	// if this is a child document and the parent is already loaded
	// update the parent's cache instead
	Buffer * tmp = getMasterBuffer();
	BOOST_ASSERT(tmp);
	if (tmp != this) {
		tmp->updateBibfilesCache();
		return;
	}

	bibfilesCache_.clear();
	for (InsetIterator it = inset_iterator_begin(inset()); it; ++it) {
		if (it->lyxCode() == Inset::BIBTEX_CODE) {
			InsetBibtex const & inset =
				static_cast<InsetBibtex const &>(*it);
			vector<FileName> const bibfiles = inset.getFiles(*this);
			bibfilesCache_.insert(bibfilesCache_.end(),
				bibfiles.begin(),
				bibfiles.end());
		} else if (it->lyxCode() == Inset::INCLUDE_CODE) {
			InsetInclude & inset =
				static_cast<InsetInclude &>(*it);
			inset.updateBibfilesCache(*this);
			vector<FileName> const & bibfiles =
					inset.getBibfilesCache(*this);
			bibfilesCache_.insert(bibfilesCache_.end(),
				bibfiles.begin(),
				bibfiles.end());
		}
	}
}


vector<FileName> const & Buffer::getBibfilesCache() const
{
	// if this is a child document and the parent is already loaded
	// use the parent's cache instead
	Buffer const * tmp = getMasterBuffer();
	BOOST_ASSERT(tmp);
	if (tmp != this)
		return tmp->getBibfilesCache();

	// We update the cache when first used instead of at loading time.
	if (bibfilesCache_.empty())
		const_cast<Buffer *>(this)->updateBibfilesCache();

	return bibfilesCache_;
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
		case LFUN_BUFFER_EXPORT: {
			bool const tmp = Exporter::Export(this, to_utf8(func.argument()), false);
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

	for_each(par_iterator_begin(),
		 par_iterator_end(),
		 bind(&Paragraph::changeLanguage, _1, params(), from, to));
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
	return lyx::par_iterator_begin(inset());
}


ParIterator Buffer::par_iterator_end()
{
	return lyx::par_iterator_end(inset());
}


ParConstIterator Buffer::par_iterator_begin() const
{
	return lyx::par_const_iterator_begin(inset());
}


ParConstIterator Buffer::par_iterator_end() const
{
	return lyx::par_const_iterator_end(inset());
}


Language const * Buffer::getLanguage() const
{
	return params().language;
}


docstring const Buffer::B_(string const & l10n) const
{
	return params().B_(l10n);
}


bool Buffer::isClean() const
{
	return pimpl_->lyx_clean;
}


bool Buffer::isBakClean() const
{
	return pimpl_->bak_clean;
}


bool Buffer::isExternallyModified(CheckMethod method) const
{
	BOOST_ASSERT(fs::exists(pimpl_->filename.toFilesystemEncoding()));
	// if method == timestamp, check timestamp before checksum
	return (method == checksum_method 
		|| pimpl_->timestamp_ != fs::last_write_time(pimpl_->filename.toFilesystemEncoding()))
		&& pimpl_->checksum_ != sum(pimpl_->filename);
}


void Buffer::saveCheckSum(string const & file) const
{
	if (fs::exists(file)) {
		pimpl_->timestamp_ = fs::last_write_time(file);
		pimpl_->checksum_ = sum(FileName(file));
	} else {
		// in the case of save to a new file.
		pimpl_->timestamp_ = 0;
		pimpl_->checksum_ = 0;
	}
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


// FIXME: this function should be moved to buffer_pimpl.C
void Buffer::markDirty()
{
	if (pimpl_->lyx_clean) {
		pimpl_->lyx_clean = false;
		updateTitles();
	}
	pimpl_->bak_clean = false;

	DepClean::iterator it = pimpl_->dep_clean.begin();
	DepClean::const_iterator const end = pimpl_->dep_clean.end();

	for (; it != end; ++it)
		it->second = false;
}


string const Buffer::fileName() const
{
	return pimpl_->filename.absFilename();
}


string const & Buffer::filePath() const
{
	return params().filepath;
}


bool Buffer::isReadonly() const
{
	return pimpl_->read_only;
}


void Buffer::setParentName(string const & name)
{
	if (name == pimpl_->filename.absFilename())
		// Avoids recursive include.
		params().parentname.clear();
	else
		params().parentname = name;
}


Buffer const * Buffer::getMasterBuffer() const
{
	if (!params().parentname.empty()
	    && theBufferList().exists(params().parentname)) {
		Buffer const * buf = theBufferList().getBuffer(params().parentname);
		//We need to check if the parent is us...
		//FIXME RECURSIVE INCLUDE
		//This is not sufficient, since recursive includes could be downstream.
		if (buf && buf != this)
			return buf->getMasterBuffer();
	}

	return this;
}


Buffer * Buffer::getMasterBuffer()
{
	if (!params().parentname.empty()
	    && theBufferList().exists(params().parentname)) {
		Buffer * buf = theBufferList().getBuffer(params().parentname);
		if (buf)
			return buf->getMasterBuffer();
	}

	return this;
}


MacroData const & Buffer::getMacro(docstring const & name) const
{
	return pimpl_->macros.get(name);
}


bool Buffer::hasMacro(docstring const & name) const
{
	return pimpl_->macros.has(name);
}


void Buffer::insertMacro(docstring const & name, MacroData const & data)
{
	MacroTable::globalMacros().insert(name, data);
	pimpl_->macros.insert(name, data);
}


void Buffer::buildMacros()
{
	// Start with global table.
	pimpl_->macros = MacroTable::globalMacros();

	// Now add our own.
	ParagraphList const & pars = text().paragraphs();
	for (size_t i = 0, n = pars.size(); i != n; ++i) {
		//lyxerr << "searching main par " << i
		//	<< " for macro definitions" << std::endl;
		InsetList const & insets = pars[i].insetlist;
		InsetList::const_iterator it = insets.begin();
		InsetList::const_iterator end = insets.end();
		for ( ; it != end; ++it) {
			//lyxerr << "found inset code " << it->inset->lyxCode() << std::endl;
			if (it->inset->lyxCode() == Inset::MATHMACRO_CODE) {
				MathMacroTemplate const & mac
					= static_cast<MathMacroTemplate const &>(*it->inset);
				insertMacro(mac.name(), mac.asMacroData());
			}
		}
	}
}


void Buffer::changeRefsIfUnique(docstring const & from, docstring const & to,
	Inset::Code code)
{
	//FIXME: This does not work for child documents yet.
	BOOST_ASSERT(code == Inset::CITE_CODE || code == Inset::REF_CODE);
	// Check if the label 'from' appears more than once
	vector<docstring> labels;

	if (code == Inset::CITE_CODE) {
		BiblioInfo keys;
		keys.fillWithBibKeys(this);
		BiblioInfo::const_iterator bit  = keys.begin();
		BiblioInfo::const_iterator bend = keys.end();

		for (; bit != bend; ++bit)
			// FIXME UNICODE
			labels.push_back(bit->first);
	} else
		getLabelList(labels);

	if (std::count(labels.begin(), labels.end(), from) > 1)
		return;

	for (InsetIterator it = inset_iterator_begin(inset()); it; ++it) {
		if (it->lyxCode() == code) {
			InsetCommand & inset = static_cast<InsetCommand &>(*it);
			inset.replaceContents(to_utf8(from), to_utf8(to));
		}
	}
}


void Buffer::getSourceCode(odocstream & os, pit_type par_begin,
	pit_type par_end, bool full_source)
{
	OutputParams runparams(&params().encoding());
	runparams.nice = true;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;

	texrow().reset();
	if (full_source) {
		os << "% " << _("Preview source code") << "\n\n";
		texrow().newline();
		texrow().newline();
		if (isLatex())
			writeLaTeXSource(os, filePath(), runparams, true, true);
		else {
			writeDocBookSource(os, fileName(), runparams, false);
		}
	} else {
		runparams.par_begin = par_begin;
		runparams.par_end = par_end;
		if (par_begin + 1 == par_end)
			os << "% "
			   << bformat(_("Preview source code for paragraph %1$d"), par_begin)
			   << "\n\n";
		else
			os << "% "
			   << bformat(_("Preview source code from paragraph %1$s to %2$s"),
					convert<docstring>(par_begin),
					convert<docstring>(par_end - 1))
			   << "\n\n";
		texrow().newline();
		texrow().newline();
		// output paragraphs
		if (isLatex()) {
			latexParagraphs(*this, paragraphs(), os, texrow(), runparams);
		} else {
			// DocBook
			docbookParagraphs(paragraphs(), *this, os, runparams);
		}
	}
}


ErrorList const & Buffer::errorList(string const & type) const
{
	static ErrorList const emptyErrorList;
	std::map<string, ErrorList>::const_iterator I = pimpl_->errorLists.find(type);
	if (I == pimpl_->errorLists.end())
		return emptyErrorList;

	return I->second;
}


ErrorList & Buffer::errorList(string const & type)
{
	return pimpl_->errorLists[type];
}


} // namespace lyx
