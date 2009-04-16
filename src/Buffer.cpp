/**
 * \file Buffer.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Buffer.h"

#include "Author.h"
#include "LayoutFile.h"
#include "BiblioInfo.h"
#include "BranchList.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "Bullet.h"
#include "Chktex.h"
#include "Converter.h"
#include "Counters.h"
#include "DispatchResult.h"
#include "DocIterator.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Exporter.h"
#include "Format.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "IndicesList.h"
#include "InsetIterator.h"
#include "InsetList.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "LaTeX.h"
#include "Layout.h"
#include "Lexer.h"
#include "LyXAction.h"
#include "LyX.h"
#include "LyXRC.h"
#include "LyXVC.h"
#include "output_docbook.h"
#include "output.h"
#include "output_latex.h"
#include "output_plaintext.h"
#include "paragraph_funcs.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "PDFOptions.h"
#include "SpellChecker.h"
#include "sgml.h"
#include "TexRow.h"
#include "TexStream.h"
#include "Text.h"
#include "TextClass.h"
#include "TocBackend.h"
#include "Undo.h"
#include "VCBackend.h"
#include "version.h"
#include "WordLangTuple.h"
#include "WordList.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetBibtex.h"
#include "insets/InsetInclude.h"
#include "insets/InsetText.h"

#include "mathed/MacroTable.h"
#include "mathed/MathMacroTemplate.h"
#include "mathed/MathSupport.h"

#include "frontends/alert.h"
#include "frontends/Delegates.h"
#include "frontends/WorkAreaManager.h"

#include "graphics/Previews.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/FileNameList.h"
#include "support/filetools.h"
#include "support/ForkedCalls.h"
#include "support/gettext.h"
#include "support/gzstream.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/Path.h"
#include "support/Systemcall.h"
#include "support/textutils.h"
#include "support/types.h"

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;
namespace os = support::os;

namespace {

// Do not remove the comment below, so we get merge conflict in
// independent branches. Instead add your own.
int const LYX_FORMAT = 352;  // jspitzm: splitindex support

typedef map<string, bool> DepClean;
typedef map<docstring, pair<InsetLabel const *, Buffer::References> > RefCache;

void showPrintError(string const & name)
{
	docstring str = bformat(_("Could not print the document %1$s.\n"
					    "Check that your printer is set up correctly."),
			     makeDisplayPath(name, 50));
	Alert::error(_("Print document failed"), str);
}

} // namespace anon

class BufferSet : public std::set<Buffer const *> {};

class Buffer::Impl
{
public:
	Impl(Buffer & parent, FileName const & file, bool readonly);

	~Impl()
	{
		if (wa_) {
			wa_->closeAll();
			delete wa_;
		}
		delete inset;
	}

	BufferParams params;
	LyXVC lyxvc;
	FileName temppath;
	mutable TexRow texrow;

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

	///
	mutable TocBackend toc_backend;

	/// macro tables
	typedef pair<DocIterator, MacroData> ScopeMacro;
	typedef map<DocIterator, ScopeMacro> PositionScopeMacroMap;
	typedef map<docstring, PositionScopeMacroMap> NamePositionScopeMacroMap;
	/// map from the macro name to the position map,
	/// which maps the macro definition position to the scope and the MacroData.
	NamePositionScopeMacroMap macros;
	bool macro_lock;

	/// positions of child buffers in the buffer
	typedef map<Buffer const * const, DocIterator> BufferPositionMap;
	typedef pair<DocIterator, Buffer const *> ScopeBuffer;
	typedef map<DocIterator, ScopeBuffer> PositionScopeBufferMap;
	/// position of children buffers in this buffer
	BufferPositionMap children_positions;
	/// map from children inclusion positions to their scope and their buffer
	PositionScopeBufferMap position_to_children;

	/// Container for all sort of Buffer dependant errors.
	map<string, ErrorList> errorLists;

	/// timestamp and checksum used to test if the file has been externally
	/// modified. (Used to properly enable 'File->Revert to saved', bug 4114).
	time_t timestamp_;
	unsigned long checksum_;

	///
	frontend::WorkAreaManager * wa_;

	///
	Undo undo_;

	/// A cache for the bibfiles (including bibfiles of loaded child
	/// documents), needed for appropriate update of natbib labels.
	mutable support::FileNameList bibfilesCache_;

	// FIXME The caching mechanism could be improved. At present, we have a
	// cache for each Buffer, that caches all the bibliography info for that
	// Buffer. A more efficient solution would be to have a global cache per
	// file, and then to construct the Buffer's bibinfo from that.
	/// A cache for bibliography info
	mutable BiblioInfo bibinfo_;
	/// whether the bibinfo cache is valid
	bool bibinfoCacheValid_;
	/// Cache of timestamps of .bib files
	map<FileName, time_t> bibfileStatus_;

	mutable RefCache ref_cache_;

	/// our Text that should be wrapped in an InsetText
	InsetText * inset;

	/// This is here to force the test to be done whenever parent_buffer
	/// is accessed.
	Buffer const * parent() const { 
		// if parent_buffer is not loaded, then it has been unloaded,
		// which means that parent_buffer is an invalid pointer. So we
		// set it to null in that case.
		if (!theBufferList().isLoaded(parent_buffer))
			parent_buffer = 0;
		return parent_buffer; 
	}
	///
	void setParent(Buffer const * pb) { parent_buffer = pb; }
private:
	/// So we can force access via the accessors.
	mutable Buffer const * parent_buffer;
};


/// Creates the per buffer temporary directory
static FileName createBufferTmpDir()
{
	static int count;
	// We are in our own directory.  Why bother to mangle name?
	// In fact I wrote this code to circumvent a problematic behaviour
	// (bug?) of EMX mkstemp().
	FileName tmpfl(package().temp_dir().absFilename() + "/lyx_tmpbuf" +
		convert<string>(count++));

	if (!tmpfl.createDirectory(0777)) {
		throw ExceptionMessage(WarningException, _("Disk Error: "), bformat(
			_("LyX could not create the temporary directory '%1$s' (Disk is full maybe?)"),
			from_utf8(tmpfl.absFilename())));
	}
	return tmpfl;
}


Buffer::Impl::Impl(Buffer & parent, FileName const & file, bool readonly_)
	: lyx_clean(true), bak_clean(true), unnamed(false),
	  read_only(readonly_), filename(file), file_fully_loaded(false),
	  toc_backend(&parent), macro_lock(false), timestamp_(0),
	  checksum_(0), wa_(0), undo_(parent), bibinfoCacheValid_(false),
	  parent_buffer(0)
{
	temppath = createBufferTmpDir();
	lyxvc.setBuffer(&parent);
	if (use_gui)
		wa_ = new frontend::WorkAreaManager;
}


Buffer::Buffer(string const & file, bool readonly)
	: d(new Impl(*this, FileName(file), readonly)), gui_(0)
{
	LYXERR(Debug::INFO, "Buffer::Buffer()");

	d->inset = new InsetText(*this);
	d->inset->setAutoBreakRows(true);
	d->inset->getText(0)->setMacrocontextPosition(par_iterator_begin());
}


Buffer::~Buffer()
{
	LYXERR(Debug::INFO, "Buffer::~Buffer()");
	// here the buffer should take care that it is
	// saved properly, before it goes into the void.

	// GuiView already destroyed
	gui_ = 0;

	if (d->unnamed && d->filename.extension() == "internal") {
		// No need to do additional cleanups for internal buffer.
		delete d;
		return;
	}

	// loop over children
	Impl::BufferPositionMap::iterator it = d->children_positions.begin();
	Impl::BufferPositionMap::iterator end = d->children_positions.end();
	for (; it != end; ++it) {
		Buffer * child = const_cast<Buffer *>(it->first);
		// The child buffer might have been closed already.
		if (theBufferList().isLoaded(child))
			theBufferList().releaseChild(this, child);
	}

	// clear references to children in macro tables
	d->children_positions.clear();
	d->position_to_children.clear();

	if (!d->temppath.destroyDirectory()) {
		Alert::warning(_("Could not remove temporary directory"),
			bformat(_("Could not remove the temporary directory %1$s"),
			from_utf8(d->temppath.absFilename())));
	}

	// Remove any previewed LaTeX snippets associated with this buffer.
	thePreviews().removeLoader(*this);

	delete d;
}


void Buffer::changed() const
{
	if (d->wa_)
		d->wa_->redrawAll();
}


frontend::WorkAreaManager & Buffer::workAreaManager() const
{
	LASSERT(d->wa_, /**/);
	return *d->wa_;
}


Text & Buffer::text() const
{
	return d->inset->text();
}


Inset & Buffer::inset() const
{
	return *d->inset;
}


BufferParams & Buffer::params()
{
	return d->params;
}


BufferParams const & Buffer::params() const
{
	return d->params;
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
	return d->lyxvc;
}


LyXVC const & Buffer::lyxvc() const
{
	return d->lyxvc;
}


string const Buffer::temppath() const
{
	return d->temppath.absFilename();
}


TexRow & Buffer::texrow()
{
	return d->texrow;
}


TexRow const & Buffer::texrow() const
{
	return d->texrow;
}


TocBackend & Buffer::tocBackend() const
{
	return d->toc_backend;
}


Undo & Buffer::undo()
{
	return d->undo_;
}


string Buffer::latexName(bool const no_path) const
{
	FileName latex_name = makeLatexName(d->filename);
	return no_path ? latex_name.onlyFileName()
		: latex_name.absFilename();
}


string Buffer::logName(LogType * type) const
{
	string const filename = latexName(false);

	if (filename.empty()) {
		if (type)
			*type = latexlog;
		return string();
	}

	string const path = temppath();

	FileName const fname(addName(temppath(),
				     onlyFilename(changeExtension(filename,
								  ".log"))));
	FileName const bname(
		addName(path, onlyFilename(
			changeExtension(filename,
					formats.extension("literate") + ".out"))));

	// If no Latex log or Build log is newer, show Build log

	if (bname.exists() &&
	    (!fname.exists() || fname.lastModified() < bname.lastModified())) {
		LYXERR(Debug::FILES, "Log name calculated as: " << bname);
		if (type)
			*type = buildlog;
		return bname.absFilename();
	}
	LYXERR(Debug::FILES, "Log name calculated as: " << fname);
	if (type)
			*type = latexlog;
	return fname.absFilename();
}


void Buffer::setReadonly(bool const flag)
{
	if (d->read_only != flag) {
		d->read_only = flag;
		setReadOnly(flag);
	}
}


void Buffer::setFileName(string const & newfile)
{
	d->filename = makeAbsPath(newfile);
	setReadonly(d->filename.isReadOnly());
	updateTitles();
}


int Buffer::readHeader(Lexer & lex)
{
	int unknown_tokens = 0;
	int line = -1;
	int begin_header_line = -1;

	// Initialize parameters that may be/go lacking in header:
	params().branchlist().clear();
	params().preamble.erase();
	params().options.erase();
	params().master.erase();
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
	params().columnsep.erase();
	params().fontsCJK.erase();
	params().listings_params.clear();
	params().clearLayoutModules();
	params().clearRemovedModules();
	params().pdfoptions().clear();
	params().indiceslist().clear();

	for (int i = 0; i < 4; ++i) {
		params().user_defined_bullet(i) = ITEMIZE_DEFAULTS[i];
		params().temp_bullet(i) = ITEMIZE_DEFAULTS[i];
	}

	ErrorList & errorList = d->errorLists["Parse"];

	while (lex.isOK()) {
		string token;
		lex >> token;

		if (token.empty())
			continue;

		if (token == "\\end_header")
			break;

		++line;
		if (token == "\\begin_header") {
			begin_header_line = line;
			continue;
		}

		LYXERR(Debug::PARSER, "Handling document header token: `"
				      << token << '\'');

		string unknown = params().readToken(lex, token, d->filename.onlyPath());
		if (!unknown.empty()) {
			if (unknown[0] != '\\' && token == "\\textclass") {
				Alert::warning(_("Unknown document class"),
		       bformat(_("Using the default document class, because the "
					      "class %1$s is unknown."), from_utf8(unknown)));
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

	params().makeDocumentClass();

	return unknown_tokens;
}


// Uwe C. Schroeder
// changed to be public and have one parameter
// Returns true if "\end_document" is not read (Asger)
bool Buffer::readDocument(Lexer & lex)
{
	ErrorList & errorList = d->errorLists["Parse"];
	errorList.clear();

	if (!lex.checkFor("\\begin_document")) {
		docstring const s = _("\\begin_document is missing");
		errorList.push_back(ErrorItem(_("Document header error"),
			s, -1, 0, 0));
	}

	// we are reading in a brand new document
	LASSERT(paragraphs().empty(), /**/);

	readHeader(lex);

	if (params().outputChanges) {
		bool dvipost    = LaTeXFeatures::isAvailable("dvipost");
		bool xcolorulem = LaTeXFeatures::isAvailable("ulem") &&
				  LaTeXFeatures::isAvailable("xcolor");

		if (!dvipost && !xcolorulem) {
			Alert::warning(_("Changes not shown in LaTeX output"),
				       _("Changes will not be highlighted in LaTeX output, "
					 "because neither dvipost nor xcolor/ulem are installed.\n"
					 "Please install these packages or redefine "
					 "\\lyxadded and \\lyxdeleted in the LaTeX preamble."));
		} else if (!xcolorulem) {
			Alert::warning(_("Changes not shown in LaTeX output"),
				       _("Changes will not be highlighted in LaTeX output "
					 "when using pdflatex, because xcolor and ulem are not installed.\n"
					 "Please install both packages or redefine "
					 "\\lyxadded and \\lyxdeleted in the LaTeX preamble."));
		}
	}

	if (!params().master.empty()) {
		FileName const master_file = makeAbsPath(params().master,
			   onlyPath(absFileName()));
		if (isLyXFilename(master_file.absFilename())) {
			Buffer * master = 
				checkAndLoadLyXFile(master_file, true);
			if (master) {
				// necessary e.g. after a reload
				// to re-register the child (bug 5873)
				// FIXME: clean up updateMacros (here, only
				// child registering is needed).
				master->updateMacros();
				// set master as master buffer, but only
				// if we are a real child
				if (master->isChild(this))
					setParent(master);
				// if the master is not fully loaded
				// it is probably just loading this
				// child. No warning needed then.
				else if (master->isFullyLoaded())
					LYXERR0("The master '"
						<< params().master
						<< "' assigned to this document ("
						<< absFileName()
						<< ") does not include "
						"this document. Ignoring the master assignment.");
			}
		}
	}

	// read main text
	bool const res = text().read(*this, lex, errorList, d->inset);

	updateMacros();
	updateMacroInstances();
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
					       par.layout().isEnvironment());
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
				par.insertChar(pos, *cit, font, params().trackChanges);
				++pos;
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


bool Buffer::readString(string const & s)
{
	params().compressed = false;

	// remove dummy empty par
	paragraphs().clear();
	Lexer lex;
	istringstream is(s);
	lex.setStream(is);
	FileName const name = FileName::tempName("Buffer_readString");
	switch (readFile(lex, name, true)) {
	case failure:
		return false;
	case wrongversion: {
		// We need to call lyx2lyx, so write the input to a file
		ofstream os(name.toFilesystemEncoding().c_str());
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

	params().compressed = fname.isZippedFile();

	// remove dummy empty par
	paragraphs().clear();
	Lexer lex;
	lex.setFile(fname);
	if (readFile(lex, fname) != success)
		return false;

	return true;
}


bool Buffer::isFullyLoaded() const
{
	return d->file_fully_loaded;
}


void Buffer::setFullyLoaded(bool value)
{
	d->file_fully_loaded = value;
}


Buffer::ReadStatus Buffer::readFile(Lexer & lex, FileName const & filename,
		bool fromstring)
{
	LASSERT(!filename.empty(), /**/);

	// the first (non-comment) token _must_ be...
	if (!lex.checkFor("\\lyxformat")) {
		Alert::error(_("Document format failure"),
			     bformat(_("%1$s is not a readable LyX document."),
				       from_utf8(filename.absFilename())));
		return failure;
	}

	string tmp_format;
	lex >> tmp_format;
	//lyxerr << "LyX Format: `" << tmp_format << '\'' << endl;
	// if present remove ".," from string.
	size_t dot = tmp_format.find_first_of(".,");
	//lyxerr << "           dot found at " << dot << endl;
	if (dot != string::npos)
			tmp_format.erase(dot, 1);
	int const file_format = convert<int>(tmp_format);
	//lyxerr << "format: " << file_format << endl;

	// save timestamp and checksum of the original disk file, making sure
	// to not overwrite them with those of the file created in the tempdir
	// when it has to be converted to the current format.
	if (!d->checksum_) {
		// Save the timestamp and checksum of disk file. If filename is an
		// emergency file, save the timestamp and checksum of the original lyx file
		// because isExternallyModified will check for this file. (BUG4193)
		string diskfile = filename.absFilename();
		if (suffixIs(diskfile, ".emergency"))
			diskfile = diskfile.substr(0, diskfile.size() - 10);
		saveCheckSum(FileName(diskfile));
	}

	if (file_format != LYX_FORMAT) {

		if (fromstring)
			// lyx2lyx would fail
			return wrongversion;

		FileName const tmpfile = FileName::tempName("Buffer_readFile");
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

		LYXERR(Debug::INFO, "Running '" << command_str << '\'');

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

	d->file_fully_loaded = true;
	return success;
}


// Should probably be moved to somewhere else: BufferView? LyXView?
bool Buffer::save() const
{
	// We don't need autosaves in the immediate future. (Asger)
	resetAutosaveTimers();

	string const encodedFilename = d->filename.toFilesystemEncoding();

	FileName backupName;
	bool madeBackup = false;

	// make a backup if the file already exists
	if (lyxrc.make_backup && fileName().exists()) {
		backupName = FileName(absFileName() + '~');
		if (!lyxrc.backupdir_path.empty()) {
			string const mangledName =
				subst(subst(backupName.absFilename(), '/', '!'), ':', '!');
			backupName = FileName(addName(lyxrc.backupdir_path,
						      mangledName));
		}
		if (fileName().copyTo(backupName)) {
			madeBackup = true;
		} else {
			Alert::error(_("Backup failure"),
				     bformat(_("Cannot create backup file %1$s.\n"
					       "Please check whether the directory exists and is writeable."),
					     from_utf8(backupName.absFilename())));
			//LYXERR(Debug::DEBUG, "Fs error: " << fe.what());
		}
	}

	// ask if the disk file has been externally modified (use checksum method)
	if (fileName().exists() && isExternallyModified(checksum_method)) {
		docstring const file = makeDisplayPath(absFileName(), 20);
		docstring text = bformat(_("Document %1$s has been externally modified. Are you sure "
							     "you want to overwrite this file?"), file);
		int const ret = Alert::prompt(_("Overwrite modified file?"),
			text, 1, 1, _("&Overwrite"), _("&Cancel"));
		if (ret == 1)
			return false;
	}

	if (writeFile(d->filename)) {
		markClean();
		return true;
	} else {
		// Saving failed, so backup is not backup
		if (madeBackup)
			backupName.moveTo(d->filename);
		return false;
	}
}


bool Buffer::writeFile(FileName const & fname) const
{
	if (d->read_only && fname == d->filename)
		return false;

	bool retval = false;

	docstring const str = bformat(_("Saving document %1$s..."),
		makeDisplayPath(fname.absFilename()));
	message(str);

	if (params().compressed) {
		gz::ogzstream ofs(fname.toFilesystemEncoding().c_str(), ios::out|ios::trunc);
		retval = ofs && write(ofs);
	} else {
		ofstream ofs(fname.toFilesystemEncoding().c_str(), ios::out|ios::trunc);
		retval = ofs && write(ofs);
	}

	if (!retval) {
		message(str + _(" could not write file!"));
		return false;
	}

	removeAutosaveFile();

	saveCheckSum(d->filename);
	message(str + _(" done."));

	return true;
}


bool Buffer::write(ostream & ofs) const
{
#ifdef HAVE_LOCALE
	// Use the standard "C" locale for file output.
	ofs.imbue(locale::classic());
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
		a_it->second.setUsed(false);

	ParIterator const end = const_cast<Buffer *>(this)->par_iterator_end();
	ParIterator it = const_cast<Buffer *>(this)->par_iterator_begin();
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
			   OutputParams const & runparams_in,
			   bool output_preamble, bool output_body) const
{
	OutputParams runparams = runparams_in;
	if (params().useXetex)
		runparams.flavor = OutputParams::XETEX;

	string const encoding = runparams.encoding->iconvName();
	LYXERR(Debug::LATEX, "makeLaTeXFile encoding: " << encoding << "...");

	ofdocstream ofs;
	try { ofs.reset(encoding); }
	catch (iconv_codecvt_facet_exception & e) {
		lyxerr << "Caught iconv exception: " << e.what() << endl;
		Alert::error(_("Iconv software exception Detected"), bformat(_("Please "
			"verify that the support software for your encoding (%1$s) is "
			"properly installed"), from_ascii(encoding)));
		return false;
	}
	if (!openFileWrite(ofs, fname))
		return false;

	//TexStream ts(ofs.rdbuf(), &texrow());
	ErrorList & errorList = d->errorLists["Export"];
	errorList.clear();
	bool failed_export = false;
	try {
		d->texrow.reset();
		writeLaTeXSource(ofs, original_path,
		      runparams, output_preamble, output_body);
	}
	catch (EncodingException & e) {
		odocstringstream ods;
		ods.put(e.failed_char);
		ostringstream oss;
		oss << "0x" << hex << e.failed_char << dec;
		docstring msg = bformat(_("Could not find LaTeX command for character '%1$s'"
					  " (code point %2$s)"),
					  ods.str(), from_utf8(oss.str()));
		errorList.push_back(ErrorItem(msg, _("Some characters of your document are probably not "
				"representable in the chosen encoding.\n"
				"Changing the document encoding to utf8 could help."),
				e.par_id, e.pos, e.pos + 1));
		failed_export = true;
	}
	catch (iconv_codecvt_facet_exception & e) {
		errorList.push_back(ErrorItem(_("iconv conversion failed"),
			_(e.what()), -1, 0, 0));
		failed_export = true;
	}
	catch (exception const & e) {
		errorList.push_back(ErrorItem(_("conversion failed"),
			_(e.what()), -1, 0, 0));
		failed_export = true;
	}
	catch (...) {
		lyxerr << "Caught some really weird exception..." << endl;
		lyx_exit(1);
	}

	ofs.close();
	if (ofs.fail()) {
		failed_export = true;
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
	}

	errors("Export");
	return !failed_export;
}


void Buffer::writeLaTeXSource(odocstream & os,
			   string const & original_path,
			   OutputParams const & runparams_in,
			   bool const output_preamble, bool const output_body) const
{
	// The child documents, if any, shall be already loaded at this point.

	OutputParams runparams = runparams_in;

	// Classify the unicode characters appearing in math insets
	Encodings::initUnicodeMath(*this);

	// validate the buffer.
	LYXERR(Debug::LATEX, "  Validating buffer...");
	LaTeXFeatures features(*this, params(), runparams);
	validate(features);
	LYXERR(Debug::LATEX, "  Buffer validation done.");

	// The starting paragraph of the coming rows is the
	// first paragraph of the document. (Asger)
	if (output_preamble && runparams.nice) {
		os << "%% LyX " << lyx_version << " created this file.  "
			"For more info, see http://www.lyx.org/.\n"
			"%% Do not edit unless you really know what "
			"you are doing.\n";
		d->texrow.newline();
		d->texrow.newline();
	}
	LYXERR(Debug::INFO, "lyx document header finished");

	// Don't move this behind the parent_buffer=0 code below,
	// because then the macros will not get the right "redefinition"
	// flag as they don't see the parent macros which are output before.
	updateMacros();

	// fold macros if possible, still with parent buffer as the
	// macros will be put in the prefix anyway.
	updateMacroInstances();

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
			d->texrow.newline();
		}
		if (!original_path.empty()) {
			// FIXME UNICODE
			// We don't know the encoding of inputpath
			docstring const inputpath = from_utf8(latex_path(original_path));
			os << "\\makeatletter\n"
			   << "\\def\\input@path{{"
			   << inputpath << "/}}\n"
			   << "\\makeatother\n";
			d->texrow.newline();
			d->texrow.newline();
			d->texrow.newline();
		}

		// get parent macros (if this buffer has a parent) which will be
		// written at the document begin further down.
		MacroSet parentMacros;
		listParentMacros(parentMacros, features);

		// Write the preamble
		runparams.use_babel = params().writeLaTeX(os, features, d->texrow);

		runparams.use_japanese = features.isRequired("japanese");

		if (!output_body)
			return;

		// make the body.
		os << "\\begin{document}\n";
		d->texrow.newline();

		// output the parent macros
		MacroSet::iterator it = parentMacros.begin();
		MacroSet::iterator end = parentMacros.end();
		for (; it != end; ++it)
			(*it)->write(os, true);
	} // output_preamble

	d->texrow.start(paragraphs().begin()->id(), 0);

	LYXERR(Debug::INFO, "preamble finished, now the body.");

	// if we are doing a real file with body, even if this is the
	// child of some other buffer, let's cut the link here.
	// This happens for example if only a child document is printed.
	Buffer const * save_parent = 0;
	if (output_preamble) {
		save_parent = d->parent();
		d->setParent(0);
	}

	// the real stuff
	latexParagraphs(*this, text(), os, d->texrow, runparams);

	// Restore the parenthood if needed
	if (output_preamble)
		d->setParent(save_parent);

	// add this just in case after all the paragraphs
	os << endl;
	d->texrow.newline();

	if (output_preamble) {
		os << "\\end{document}\n";
		d->texrow.newline();
		LYXERR(Debug::LATEX, "makeLaTeXFile...done");
	} else {
		LYXERR(Debug::LATEX, "LaTeXFile for inclusion made.");
	}
	runparams_in.encoding = runparams.encoding;

	// Just to be sure. (Asger)
	d->texrow.newline();

	LYXERR(Debug::INFO, "Finished making LaTeX file.");
	LYXERR(Debug::INFO, "Row count was " << d->texrow.rows() - 1 << '.');
}


bool Buffer::isLatex() const
{
	return params().documentClass().outputType() == LATEX;
}


bool Buffer::isLiterate() const
{
	return params().documentClass().outputType() == LITERATE;
}


bool Buffer::isDocBook() const
{
	return params().documentClass().outputType() == DOCBOOK;
}


void Buffer::makeDocBookFile(FileName const & fname,
			      OutputParams const & runparams,
			      bool const body_only) const
{
	LYXERR(Debug::LATEX, "makeDocBookFile...");

	ofdocstream ofs;
	if (!openFileWrite(ofs, fname))
		return;

	writeDocBookSource(ofs, fname.absFilename(), runparams, body_only);

	ofs.close();
	if (ofs.fail())
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
}


void Buffer::writeDocBookSource(odocstream & os, string const & fname,
			     OutputParams const & runparams,
			     bool const only_body) const
{
	LaTeXFeatures features(*this, params(), runparams);
	validate(features);

	d->texrow.reset();

	DocumentClass const & tclass = params().documentClass();
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

		string const name = runparams.nice
			? changeExtension(absFileName(), ".sgml") : fname;
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
		top += params().language->code().substr(0, 2);
	top += '"';

	if (!params().options.empty()) {
		top += ' ';
		top += params().options;
	}

	os << "<!-- " << ((runparams.flavor == OutputParams::XML)? "XML" : "SGML")
	    << " file was created by LyX " << lyx_version
	    << "\n  See http://www.lyx.org/ for more information -->\n";

	params().documentClass().counters().reset();

	updateMacros();

	sgml::openTag(os, top);
	os << '\n';
	docbookParagraphs(paragraphs(), *this, os, runparams);
	sgml::closeTag(os, top_element);
}


// chktex should be run with these flags disabled: 3, 22, 25, 30, 38(?)
// Other flags: -wall -v0 -x
int Buffer::runChktex()
{
	setBusy(true);

	// get LaTeX-Filename
	FileName const path(temppath());
	string const name = addName(path.absFilename(), latexName());
	string const org_path = filePath();

	PathChanger p(path); // path to LaTeX file
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
		ErrorList & errlist = d->errorLists["ChkTeX"];
		errlist.clear();
		bufferErrors(terr, errlist);
	}

	setBusy(false);

	errors("ChkTeX");

	return res;
}


void Buffer::validate(LaTeXFeatures & features) const
{
	params().validate(features);

	updateMacros();

	for_each(paragraphs().begin(), paragraphs().end(),
		 boost::bind(&Paragraph::validate, _1, boost::ref(features)));

	if (lyxerr.debugging(Debug::LATEX)) {
		features.showStruct();
	}
}


void Buffer::getLabelList(vector<docstring> & list) const
{
	// If this is a child document, use the parent's list instead.
	Buffer const * const pbuf = d->parent();
	if (pbuf) {
		pbuf->getLabelList(list);
		return;
	}

	list.clear();
	Toc & toc = d->toc_backend.toc("label");
	TocIterator toc_it = toc.begin();
	TocIterator end = toc.end();
	for (; toc_it != end; ++toc_it) {
		if (toc_it->depth() == 0)
			list.push_back(toc_it->str());
	}
}


void Buffer::updateBibfilesCache(UpdateScope scope) const
{
	// If this is a child document, use the parent's cache instead.
	Buffer const * const pbuf = d->parent();
	if (pbuf && scope != UpdateChildOnly) {
		pbuf->updateBibfilesCache();
		return;
	}

	d->bibfilesCache_.clear();
	for (InsetIterator it = inset_iterator_begin(inset()); it; ++it) {
		if (it->lyxCode() == BIBTEX_CODE) {
			InsetBibtex const & inset =
				static_cast<InsetBibtex const &>(*it);
			support::FileNameList const bibfiles = inset.getBibFiles();
			d->bibfilesCache_.insert(d->bibfilesCache_.end(),
				bibfiles.begin(),
				bibfiles.end());
		} else if (it->lyxCode() == INCLUDE_CODE) {
			InsetInclude & inset =
				static_cast<InsetInclude &>(*it);
			inset.updateBibfilesCache();
			support::FileNameList const & bibfiles =
					inset.getBibfilesCache();
			d->bibfilesCache_.insert(d->bibfilesCache_.end(),
				bibfiles.begin(),
				bibfiles.end());
		}
	}
	// the bibinfo cache is now invalid
	d->bibinfoCacheValid_ = false;
}


void Buffer::invalidateBibinfoCache()
{
	d->bibinfoCacheValid_ = false;
}


support::FileNameList const & Buffer::getBibfilesCache(UpdateScope scope) const
{
	// If this is a child document, use the parent's cache instead.
	Buffer const * const pbuf = d->parent();
	if (pbuf && scope != UpdateChildOnly)
		return pbuf->getBibfilesCache();

	// We update the cache when first used instead of at loading time.
	if (d->bibfilesCache_.empty())
		const_cast<Buffer *>(this)->updateBibfilesCache(scope);

	return d->bibfilesCache_;
}


BiblioInfo const & Buffer::masterBibInfo() const
{
	// if this is a child document and the parent is already loaded
	// use the parent's list instead  [ale990412]
	Buffer const * const tmp = masterBuffer();
	LASSERT(tmp, /**/);
	if (tmp != this)
		return tmp->masterBibInfo();
	return localBibInfo();
}


BiblioInfo const & Buffer::localBibInfo() const
{
	if (d->bibinfoCacheValid_) {
		support::FileNameList const & bibfilesCache = getBibfilesCache();
		// compare the cached timestamps with the actual ones.
		support::FileNameList::const_iterator ei = bibfilesCache.begin();
		support::FileNameList::const_iterator en = bibfilesCache.end();
		for (; ei != en; ++ ei) {
			time_t lastw = ei->lastModified();
			if (lastw != d->bibfileStatus_[*ei]) {
				d->bibinfoCacheValid_ = false;
				d->bibfileStatus_[*ei] = lastw;
				break;
			}
		}
	}

	if (!d->bibinfoCacheValid_) {
		d->bibinfo_.clear();
		for (InsetIterator it = inset_iterator_begin(inset()); it; ++it)
			it->fillWithBibKeys(d->bibinfo_, it);
		d->bibinfoCacheValid_ = true;
	}
	return d->bibinfo_;
}


bool Buffer::isDepClean(string const & name) const
{
	DepClean::const_iterator const it = d->dep_clean.find(name);
	if (it == d->dep_clean.end())
		return true;
	return it->second;
}


void Buffer::markDepClean(string const & name)
{
	d->dep_clean[name] = true;
}


bool Buffer::getStatus(FuncRequest const & cmd, FuncStatus & flag)
{
	switch (cmd.action) {
		case LFUN_BUFFER_EXPORT: {
			docstring const arg = cmd.argument();
			bool enable = arg == "custom" || isExportable(to_utf8(arg));
			if (!enable)
				flag.message(bformat(
					_("Don't know how to export to format: %1$s"), arg));
			flag.setEnabled(enable);
			break;
		}

		case LFUN_BRANCH_ACTIVATE: 
		case LFUN_BRANCH_DEACTIVATE: {
		BranchList const & branchList = params().branchlist();
		docstring const branchName = cmd.argument();
		flag.setEnabled(!branchName.empty()
				&& branchList.find(branchName));
			break;
		}

		case LFUN_BUFFER_PRINT:
			// if no Buffer is present, then of course we won't be called!
			flag.setEnabled(true);
			break;

		default:
			return false;
	}
	return true;
}


void Buffer::dispatch(string const & command, DispatchResult & result)
{
	return dispatch(lyxaction.lookupFunc(command), result);
}


// NOTE We can end up here even if we have no GUI, because we are called
// by LyX::exec to handled command-line requests. So we may need to check 
// whether we have a GUI or not. The boolean use_gui holds this information.
void Buffer::dispatch(FuncRequest const & func, DispatchResult & dr)
{
	// We'll set this back to false if need be.
	bool dispatched = true;

	switch (func.action) {
		case LFUN_BUFFER_EXPORT: {
			bool success = doExport(to_utf8(func.argument()), false);
			dr.setError(success);
			if (!success)
				dr.setMessage(bformat(_("Error exporting to format: %1$s."), 
				                      func.argument()));
			break;
		}

		case LFUN_BRANCH_ACTIVATE:
		case LFUN_BRANCH_DEACTIVATE: {
			BranchList & branchList = params().branchlist();
			docstring const branchName = func.argument();
			// the case without a branch name is handled elsewhere
			if (branchName.empty()) {
				dispatched = false;
				break;
			}
			Branch * branch = branchList.find(branchName);
			if (!branch) {
				LYXERR0("Branch " << branchName << " does not exist.");
				dr.setError(true);
				docstring const msg = 
					bformat(_("Branch \"%1$s\" does not exist."), branchName);
				dr.setMessage(msg);
			} else {
				branch->setSelected(func.action == LFUN_BRANCH_ACTIVATE);
				dr.setError(false);
				dr.update(Update::Force);
			}
			break;
		}

		case LFUN_BUFFER_PRINT: {
			// we'll assume there's a problem until we succeed
			dr.setError(true); 
			string target = func.getArg(0);
			string target_name = func.getArg(1);
			string command = func.getArg(2);

			if (target.empty()
			    || target_name.empty()
			    || command.empty()) {
				LYXERR0("Unable to parse " << func.argument());
				docstring const msg = 
					bformat(_("Unable to parse \"%1$s\""), func.argument());
				dr.setMessage(msg);
				break;
			}
			if (target != "printer" && target != "file") {
				LYXERR0("Unrecognized target \"" << target << '"');
				docstring const msg = 
					bformat(_("Unrecognized target \"%1$s\""), from_utf8(target));
				dr.setMessage(msg);
				break;
			}

			if (!doExport("dvi", true)) {
				showPrintError(absFileName());
				dr.setMessage(_("Error exporting to DVI."));
				break;
			}

			// Push directory path.
			string const path = temppath();
			// Prevent the compiler from optimizing away p
			FileName pp(path);
			PathChanger p(pp);

			// there are three cases here:
			// 1. we print to a file
			// 2. we print directly to a printer
			// 3. we print using a spool command (print to file first)
			Systemcall one;
			int res = 0;
			string const dviname = changeExtension(latexName(true), "dvi");

			if (target == "printer") {
				if (!lyxrc.print_spool_command.empty()) {
					// case 3: print using a spool
					string const psname = changeExtension(dviname,".ps");
					command += ' ' + lyxrc.print_to_file
						+ quoteName(psname)
						+ ' '
						+ quoteName(dviname);

					string command2 = lyxrc.print_spool_command + ' ';
					if (target_name != "default") {
						command2 += lyxrc.print_spool_printerprefix
							+ target_name
							+ ' ';
					}
					command2 += quoteName(psname);
					// First run dvips.
					// If successful, then spool command
					res = one.startscript(Systemcall::Wait, command);

					if (res == 0) {
						// If there's no GUI, we have to wait on this command. Otherwise,
						// LyX deletes the temporary directory, and with it the spooled
						// file, before it can be printed!!
						Systemcall::Starttype stype = use_gui ?
							Systemcall::DontWait : Systemcall::Wait;
						res = one.startscript(stype, command2);
					}
				} else {
					// case 2: print directly to a printer
					if (target_name != "default")
						command += ' ' + lyxrc.print_to_printer + target_name + ' ';
					// as above....
					Systemcall::Starttype stype = use_gui ?
						Systemcall::DontWait : Systemcall::Wait;
					res = one.startscript(stype, command + quoteName(dviname));
				}

			} else {
				// case 1: print to a file
				FileName const filename(makeAbsPath(target_name, filePath()));
				FileName const dvifile(makeAbsPath(dviname, path));
				if (filename.exists()) {
					docstring text = bformat(
						_("The file %1$s already exists.\n\n"
						  "Do you want to overwrite that file?"),
						makeDisplayPath(filename.absFilename()));
					if (Alert::prompt(_("Overwrite file?"),
					    text, 0, 1, _("&Overwrite"), _("&Cancel")) != 0)
						break;
				}
				command += ' ' + lyxrc.print_to_file
					+ quoteName(filename.toFilesystemEncoding())
					+ ' '
					+ quoteName(dvifile.toFilesystemEncoding());
				// as above....
				Systemcall::Starttype stype = use_gui ?
					Systemcall::DontWait : Systemcall::Wait;
				res = one.startscript(stype, command);
			}

			if (res == 0) 
				dr.setError(false);
			else {
				dr.setMessage(_("Error running external commands."));
				showPrintError(absFileName());
			}
			break;
		}

		default:
			dispatched = false;
			break;
	}
	dr.dispatched(dispatched);
}


void Buffer::changeLanguage(Language const * from, Language const * to)
{
	LASSERT(from, /**/);
	LASSERT(to, /**/);

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


DocIterator Buffer::getParFromID(int const id) const
{
	Buffer * buf = const_cast<Buffer *>(this);
	if (id < 0) {
		// John says this is called with id == -1 from undo
		lyxerr << "getParFromID(), id: " << id << endl;
		return doc_iterator_end(buf);
	}

	for (DocIterator it = doc_iterator_begin(buf); !it.atEnd(); it.forwardPar())
		if (it.paragraph().id() == id)
			return it;

	return doc_iterator_end(buf);
}


bool Buffer::hasParWithID(int const id) const
{
	return !getParFromID(id).atEnd();
}


ParIterator Buffer::par_iterator_begin()
{
	return ParIterator(doc_iterator_begin(this));
}


ParIterator Buffer::par_iterator_end()
{
	return ParIterator(doc_iterator_end(this));
}


ParConstIterator Buffer::par_iterator_begin() const
{
	return ParConstIterator(doc_iterator_begin(this));
}


ParConstIterator Buffer::par_iterator_end() const
{
	return ParConstIterator(doc_iterator_end(this));
}


Language const * Buffer::language() const
{
	return params().language;
}


docstring const Buffer::B_(string const & l10n) const
{
	return params().B_(l10n);
}


bool Buffer::isClean() const
{
	return d->lyx_clean;
}


bool Buffer::isBakClean() const
{
	return d->bak_clean;
}


bool Buffer::isExternallyModified(CheckMethod method) const
{
	LASSERT(d->filename.exists(), /**/);
	// if method == timestamp, check timestamp before checksum
	return (method == checksum_method
		|| d->timestamp_ != d->filename.lastModified())
		&& d->checksum_ != d->filename.checksum();
}


void Buffer::saveCheckSum(FileName const & file) const
{
	if (file.exists()) {
		d->timestamp_ = file.lastModified();
		d->checksum_ = file.checksum();
	} else {
		// in the case of save to a new file.
		d->timestamp_ = 0;
		d->checksum_ = 0;
	}
}


void Buffer::markClean() const
{
	if (!d->lyx_clean) {
		d->lyx_clean = true;
		updateTitles();
	}
	// if the .lyx file has been saved, we don't need an
	// autosave
	d->bak_clean = true;
}


void Buffer::markBakClean() const
{
	d->bak_clean = true;
}


void Buffer::setUnnamed(bool flag)
{
	d->unnamed = flag;
}


bool Buffer::isUnnamed() const
{
	return d->unnamed;
}


// FIXME: this function should be moved to buffer_pimpl.C
void Buffer::markDirty()
{
	if (d->lyx_clean) {
		d->lyx_clean = false;
		updateTitles();
	}
	d->bak_clean = false;

	DepClean::iterator it = d->dep_clean.begin();
	DepClean::const_iterator const end = d->dep_clean.end();

	for (; it != end; ++it)
		it->second = false;
}


FileName Buffer::fileName() const
{
	return d->filename;
}


string Buffer::absFileName() const
{
	return d->filename.absFilename();
}


string Buffer::filePath() const
{
	return d->filename.onlyPath().absFilename() + "/";
}


bool Buffer::isReadonly() const
{
	return d->read_only;
}


void Buffer::setParent(Buffer const * buffer)
{
	// Avoids recursive include.
	d->setParent(buffer == this ? 0 : buffer);
	updateMacros();
}


Buffer const * Buffer::parent() const
{
	return d->parent();
}


void Buffer::collectRelatives(BufferSet & bufs) const
{
	bufs.insert(this);
	if (parent())
		parent()->collectRelatives(bufs);

	// loop over children
	Impl::BufferPositionMap::iterator it = d->children_positions.begin();
	Impl::BufferPositionMap::iterator end = d->children_positions.end();
	for (; it != end; ++it)
		bufs.insert(const_cast<Buffer *>(it->first));
}


std::vector<Buffer const *> Buffer::allRelatives() const
{
	BufferSet bufs;
	collectRelatives(bufs);
	BufferSet::iterator it = bufs.begin();
	std::vector<Buffer const *> ret;
	for (; it != bufs.end(); ++it)
		ret.push_back(*it);
	return ret;
}


Buffer const * Buffer::masterBuffer() const
{
	Buffer const * const pbuf = d->parent();
	if (!pbuf)
		return this;

	return pbuf->masterBuffer();
}


bool Buffer::isChild(Buffer * child) const
{
	return d->children_positions.find(child) != d->children_positions.end();
}


DocIterator Buffer::firstChildPosition(Buffer const * child)
{
	Impl::BufferPositionMap::iterator it;
	it = d->children_positions.find(child);
	if (it == d->children_positions.end())
		return DocIterator(this);
	return it->second;
}


std::vector<Buffer *> Buffer::getChildren() const
{
	std::vector<Buffer *> clist;
	// loop over children
	Impl::BufferPositionMap::iterator it = d->children_positions.begin();
	Impl::BufferPositionMap::iterator end = d->children_positions.end();
	for (; it != end; ++it) {
		Buffer * child = const_cast<Buffer *>(it->first);
		clist.push_back(child);
		// there might be grandchildren
		std::vector<Buffer *> glist = child->getChildren();
		for (vector<Buffer *>::const_iterator git = glist.begin();
		     git != glist.end(); ++git)
			clist.push_back(*git);
	}
	return clist;
}


template<typename M>
typename M::iterator greatest_below(M & m, typename M::key_type const & x)
{
	if (m.empty())
		return m.end();

	typename M::iterator it = m.lower_bound(x);
	if (it == m.begin())
		return m.end();

	it--;
	return it;
}


MacroData const * Buffer::getBufferMacro(docstring const & name,
					 DocIterator const & pos) const
{
	LYXERR(Debug::MACROS, "Searching for " << to_ascii(name) << " at " << pos);

	// if paragraphs have no macro context set, pos will be empty
	if (pos.empty())
		return 0;

	// we haven't found anything yet
	DocIterator bestPos = par_iterator_begin();
	MacroData const * bestData = 0;

	// find macro definitions for name
	Impl::NamePositionScopeMacroMap::iterator nameIt
		= d->macros.find(name);
	if (nameIt != d->macros.end()) {
		// find last definition in front of pos or at pos itself
		Impl::PositionScopeMacroMap::const_iterator it
			= greatest_below(nameIt->second, pos);
		if (it != nameIt->second.end()) {
			while (true) {
				// scope ends behind pos?
				if (pos < it->second.first) {
					// Looks good, remember this. If there
					// is no external macro behind this,
					// we found the right one already.
					bestPos = it->first;
					bestData = &it->second.second;
					break;
				}

				// try previous macro if there is one
				if (it == nameIt->second.begin())
					break;
				it--;
			}
		}
	}

	// find macros in included files
	Impl::PositionScopeBufferMap::const_iterator it
		= greatest_below(d->position_to_children, pos);
	if (it == d->position_to_children.end())
		// no children before
		return bestData;

	while (true) {
		// do we know something better (i.e. later) already?
		if (it->first < bestPos )
			break;

		// scope ends behind pos?
		if (pos < it->second.first) {
			// look for macro in external file
			d->macro_lock = true;
			MacroData const * data
			= it->second.second->getMacro(name, false);
			d->macro_lock = false;
			if (data) {
				bestPos = it->first;
				bestData = data;
				break;
			}
		}

		// try previous file if there is one
		if (it == d->position_to_children.begin())
			break;
		--it;
	}

	// return the best macro we have found
	return bestData;
}


MacroData const * Buffer::getMacro(docstring const & name,
	DocIterator const & pos, bool global) const
{
	if (d->macro_lock)
		return 0;

	// query buffer macros
	MacroData const * data = getBufferMacro(name, pos);
	if (data != 0)
		return data;

	// If there is a master buffer, query that
	Buffer const * const pbuf = d->parent();
	if (pbuf) {
		d->macro_lock = true;
		MacroData const * macro	= pbuf->getMacro(
			name, *this, false);
		d->macro_lock = false;
		if (macro)
			return macro;
	}

	if (global) {
		data = MacroTable::globalMacros().get(name);
		if (data != 0)
			return data;
	}

	return 0;
}


MacroData const * Buffer::getMacro(docstring const & name, bool global) const
{
	// set scope end behind the last paragraph
	DocIterator scope = par_iterator_begin();
	scope.pit() = scope.lastpit() + 1;

	return getMacro(name, scope, global);
}


MacroData const * Buffer::getMacro(docstring const & name,
	Buffer const & child, bool global) const
{
	// look where the child buffer is included first
	Impl::BufferPositionMap::iterator it = d->children_positions.find(&child);
	if (it == d->children_positions.end())
		return 0;

	// check for macros at the inclusion position
	return getMacro(name, it->second, global);
}


void Buffer::updateMacros(DocIterator & it, DocIterator & scope) const
{
	pit_type lastpit = it.lastpit();

	// look for macros in each paragraph
	while (it.pit() <= lastpit) {
		Paragraph & par = it.paragraph();

		// iterate over the insets of the current paragraph
		InsetList const & insets = par.insetList();
		InsetList::const_iterator iit = insets.begin();
		InsetList::const_iterator end = insets.end();
		for (; iit != end; ++iit) {
			it.pos() = iit->pos;

			// is it a nested text inset?
			if (iit->inset->asInsetText()) {
				// Inset needs its own scope?
				InsetText const * itext = iit->inset->asInsetText();
				bool newScope = itext->isMacroScope();

				// scope which ends just behind the inset
				DocIterator insetScope = it;
				++insetScope.pos();

				// collect macros in inset
				it.push_back(CursorSlice(*iit->inset));
				updateMacros(it, newScope ? insetScope : scope);
				it.pop_back();
				continue;
			}

			// is it an external file?
			if (iit->inset->lyxCode() == INCLUDE_CODE) {
				// get buffer of external file
				InsetInclude const & inset =
					static_cast<InsetInclude const &>(*iit->inset);
				d->macro_lock = true;
				Buffer * child = inset.getChildBuffer();
				d->macro_lock = false;
				if (!child)
					continue;

				// register its position, but only when it is
				// included first in the buffer
				if (d->children_positions.find(child) ==
					d->children_positions.end())
						d->children_positions[child] = it;

				// register child with its scope
				d->position_to_children[it] = Impl::ScopeBuffer(scope, child);
				continue;
			}

			if (iit->inset->lyxCode() != MATHMACRO_CODE)
				continue;

			// get macro data
			MathMacroTemplate & macroTemplate =
				static_cast<MathMacroTemplate &>(*iit->inset);
			MacroContext mc(*this, it);
			macroTemplate.updateToContext(mc);

			// valid?
			bool valid = macroTemplate.validMacro();
			// FIXME: Should be fixNameAndCheckIfValid() in fact,
			// then the BufferView's cursor will be invalid in
			// some cases which leads to crashes.
			if (!valid)
				continue;

			// register macro
			d->macros[macroTemplate.name()][it] =
				Impl::ScopeMacro(scope, MacroData(*this, it));
		}

		// next paragraph
		it.pit()++;
		it.pos() = 0;
	}
}


void Buffer::updateMacros() const
{
	if (d->macro_lock)
		return;

	LYXERR(Debug::MACROS, "updateMacro of " << d->filename.onlyFileName());

	// start with empty table
	d->macros.clear();
	d->children_positions.clear();
	d->position_to_children.clear();

	// Iterate over buffer, starting with first paragraph
	// The scope must be bigger than any lookup DocIterator
	// later. For the global lookup, lastpit+1 is used, hence
	// we use lastpit+2 here.
	DocIterator it = par_iterator_begin();
	DocIterator outerScope = it;
	outerScope.pit() = outerScope.lastpit() + 2;
	updateMacros(it, outerScope);
}


void Buffer::updateMacroInstances() const
{
	LYXERR(Debug::MACROS, "updateMacroInstances for "
		<< d->filename.onlyFileName());
	DocIterator it = doc_iterator_begin(this);
	DocIterator end = doc_iterator_end(this);
	for (; it != end; it.forwardPos()) {
		// look for MathData cells in InsetMathNest insets
		Inset * inset = it.nextInset();
		if (!inset)
			continue;

		InsetMath * minset = inset->asInsetMath();
		if (!minset)
			continue;

		// update macro in all cells of the InsetMathNest
		DocIterator::idx_type n = minset->nargs();
		MacroContext mc = MacroContext(*this, it);
		for (DocIterator::idx_type i = 0; i < n; ++i) {
			MathData & data = minset->cell(i);
			data.updateMacros(0, mc);
		}
	}
}


void Buffer::listMacroNames(MacroNameSet & macros) const
{
	if (d->macro_lock)
		return;

	d->macro_lock = true;

	// loop over macro names
	Impl::NamePositionScopeMacroMap::iterator nameIt = d->macros.begin();
	Impl::NamePositionScopeMacroMap::iterator nameEnd = d->macros.end();
	for (; nameIt != nameEnd; ++nameIt)
		macros.insert(nameIt->first);

	// loop over children
	Impl::BufferPositionMap::iterator it = d->children_positions.begin();
	Impl::BufferPositionMap::iterator end = d->children_positions.end();
	for (; it != end; ++it)
		it->first->listMacroNames(macros);

	// call parent
	Buffer const * const pbuf = d->parent();
	if (pbuf)
		pbuf->listMacroNames(macros);

	d->macro_lock = false;
}


void Buffer::listParentMacros(MacroSet & macros, LaTeXFeatures & features) const
{
	Buffer const * const pbuf = d->parent();
	if (!pbuf)
		return;

	MacroNameSet names;
	pbuf->listMacroNames(names);

	// resolve macros
	MacroNameSet::iterator it = names.begin();
	MacroNameSet::iterator end = names.end();
	for (; it != end; ++it) {
		// defined?
		MacroData const * data =
		pbuf->getMacro(*it, *this, false);
		if (data) {
			macros.insert(data);

			// we cannot access the original MathMacroTemplate anymore
			// here to calls validate method. So we do its work here manually.
			// FIXME: somehow make the template accessible here.
			if (data->optionals() > 0)
				features.require("xargs");
		}
	}
}


Buffer::References & Buffer::references(docstring const & label)
{
	if (d->parent())
		return const_cast<Buffer *>(masterBuffer())->references(label);

	RefCache::iterator it = d->ref_cache_.find(label);
	if (it != d->ref_cache_.end())
		return it->second.second;

	static InsetLabel const * dummy_il = 0;
	static References const dummy_refs;
	it = d->ref_cache_.insert(
		make_pair(label, make_pair(dummy_il, dummy_refs))).first;
	return it->second.second;
}


Buffer::References const & Buffer::references(docstring const & label) const
{
	return const_cast<Buffer *>(this)->references(label);
}


void Buffer::setInsetLabel(docstring const & label, InsetLabel const * il)
{
	masterBuffer()->d->ref_cache_[label].first = il;
}


InsetLabel const * Buffer::insetLabel(docstring const & label) const
{
	return masterBuffer()->d->ref_cache_[label].first;
}


void Buffer::clearReferenceCache() const
{
	if (!d->parent())
		d->ref_cache_.clear();
}


void Buffer::changeRefsIfUnique(docstring const & from, docstring const & to,
	InsetCode code)
{
	//FIXME: This does not work for child documents yet.
	LASSERT(code == CITE_CODE, /**/);
	// Check if the label 'from' appears more than once
	vector<docstring> labels;
	string paramName;
	BiblioInfo const & keys = masterBibInfo();
	BiblioInfo::const_iterator bit  = keys.begin();
	BiblioInfo::const_iterator bend = keys.end();

	for (; bit != bend; ++bit)
		// FIXME UNICODE
		labels.push_back(bit->first);
	paramName = "key";

	if (count(labels.begin(), labels.end(), from) > 1)
		return;

	for (InsetIterator it = inset_iterator_begin(inset()); it; ++it) {
		if (it->lyxCode() == code) {
			InsetCommand & inset = static_cast<InsetCommand &>(*it);
			docstring const oldValue = inset.getParam(paramName);
			if (oldValue == from)
				inset.setParam(paramName, to);
		}
	}
}


void Buffer::getSourceCode(odocstream & os, pit_type par_begin,
	pit_type par_end, bool full_source) const
{
	OutputParams runparams(&params().encoding());
	runparams.nice = true;
	runparams.flavor = params().useXetex ? 
		OutputParams::XETEX : OutputParams::LATEX;
	runparams.linelen = lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;

	d->texrow.reset();
	if (full_source) {
		os << "% " << _("Preview source code") << "\n\n";
		d->texrow.newline();
		d->texrow.newline();
		if (isDocBook())
			writeDocBookSource(os, absFileName(), runparams, false);
		else
			// latex or literate
			writeLaTeXSource(os, string(), runparams, true, true);
	} else {
		runparams.par_begin = par_begin;
		runparams.par_end = par_end;
		if (par_begin + 1 == par_end) {
			os << "% "
			   << bformat(_("Preview source code for paragraph %1$d"), par_begin)
			   << "\n\n";
		} else {
			os << "% "
			   << bformat(_("Preview source code from paragraph %1$s to %2$s"),
					convert<docstring>(par_begin),
					convert<docstring>(par_end - 1))
			   << "\n\n";
		}
		d->texrow.newline();
		d->texrow.newline();
		// output paragraphs
		if (isDocBook())
			docbookParagraphs(paragraphs(), *this, os, runparams);
		else 
			// latex or literate
			latexParagraphs(*this, text(), os, d->texrow, runparams);
	}
}


ErrorList & Buffer::errorList(string const & type) const
{
	static ErrorList emptyErrorList;
	map<string, ErrorList>::iterator I = d->errorLists.find(type);
	if (I == d->errorLists.end())
		return emptyErrorList;

	return I->second;
}


void Buffer::updateTocItem(std::string const & type,
	DocIterator const & dit) const
{
	if (gui_)
		gui_->updateTocItem(type, dit);
}


void Buffer::structureChanged() const
{
	if (gui_)
		gui_->structureChanged();
}


void Buffer::errors(string const & err) const
{
	if (gui_)
		gui_->errors(err);
}


void Buffer::message(docstring const & msg) const
{
	if (gui_)
		gui_->message(msg);
}


void Buffer::setBusy(bool on) const
{
	if (gui_)
		gui_->setBusy(on);
}


void Buffer::setReadOnly(bool on) const
{
	if (d->wa_)
		d->wa_->setReadOnly(on);
}


void Buffer::updateTitles() const
{
	if (d->wa_)
		d->wa_->updateTitles();
}


void Buffer::resetAutosaveTimers() const
{
	if (gui_)
		gui_->resetAutosaveTimers();
}


bool Buffer::hasGuiDelegate() const
{
	return gui_;
}


void Buffer::setGuiDelegate(frontend::GuiBufferDelegate * gui)
{
	gui_ = gui;
}



namespace {

class AutoSaveBuffer : public ForkedProcess {
public:
	///
	AutoSaveBuffer(Buffer const & buffer, FileName const & fname)
		: buffer_(buffer), fname_(fname) {}
	///
	virtual boost::shared_ptr<ForkedProcess> clone() const
	{
		return boost::shared_ptr<ForkedProcess>(new AutoSaveBuffer(*this));
	}
	///
	int start()
	{
		command_ = to_utf8(bformat(_("Auto-saving %1$s"),
						 from_utf8(fname_.absFilename())));
		return run(DontWait);
	}
private:
	///
	virtual int generateChild();
	///
	Buffer const & buffer_;
	FileName fname_;
};


int AutoSaveBuffer::generateChild()
{
	// tmp_ret will be located (usually) in /tmp
	// will that be a problem?
	// Note that this calls ForkedCalls::fork(), so it's
	// ok cross-platform.
	pid_t const pid = fork();
	// If you want to debug the autosave
	// you should set pid to -1, and comment out the fork.
	if (pid != 0 && pid != -1)
		return pid;

	// pid = -1 signifies that lyx was unable
	// to fork. But we will do the save
	// anyway.
	bool failed = false;
	FileName const tmp_ret = FileName::tempName("lyxauto");
	if (!tmp_ret.empty()) {
		buffer_.writeFile(tmp_ret);
		// assume successful write of tmp_ret
		if (!tmp_ret.moveTo(fname_))
			failed = true;
	} else
		failed = true;

	if (failed) {
		// failed to write/rename tmp_ret so try writing direct
		if (!buffer_.writeFile(fname_)) {
			// It is dangerous to do this in the child,
			// but safe in the parent, so...
			if (pid == -1) // emit message signal.
				buffer_.message(_("Autosave failed!"));
		}
	}

	if (pid == 0) // we are the child so...
		_exit(0);

	return pid;
}

} // namespace anon


FileName Buffer::getAutosaveFilename() const
{
	string const fpath = isUnnamed() ? lyxrc.document_path : filePath();
	string const fname = "#" + d->filename.onlyFileName() + "#";
	return makeAbsPath(fname, fpath);
}


void Buffer::removeAutosaveFile() const
{
	FileName const f = getAutosaveFilename();
	if (f.exists())
		f.removeFile();
}


// Perfect target for a thread...
void Buffer::autoSave() const
{
	if (isBakClean() || isReadonly()) {
		// We don't save now, but we'll try again later
		resetAutosaveTimers();
		return;
	}

	// emit message signal.
	message(_("Autosaving current document..."));
	AutoSaveBuffer autosave(*this, getAutosaveFilename());
	autosave.start();

	markBakClean();
	resetAutosaveTimers();
}


string Buffer::bufferFormat() const
{
	if (isDocBook())
		return "docbook";
	if (isLiterate())
		return "literate";
	if (params().useXetex)
		return "xetex";
	if (params().encoding().package() == Encoding::japanese)
		return "platex";
	return "latex";
}


string Buffer::getDefaultOutputFormat() const
{
	if (!params().defaultOutputFormat.empty()
	    && params().defaultOutputFormat != "default")
		return params().defaultOutputFormat;
	typedef vector<Format const *> Formats;
	Formats formats = exportableFormats(true);
	if (isDocBook()
	    || isLiterate()
	    || params().useXetex
	    || params().encoding().package() == Encoding::japanese) {
		if (formats.empty())
			return string();
		// return the first we find
		return formats.front()->name();
	}
	return lyxrc.default_view_format;
}



bool Buffer::doExport(string const & format, bool put_in_tempdir,
	string & result_file) const
{
	string backend_format;
	OutputParams runparams(&params().encoding());
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = lyxrc.plaintext_linelen;
	vector<string> backs = backends();
	if (find(backs.begin(), backs.end(), format) == backs.end()) {
		// Get shortest path to format
		Graph::EdgePath path;
		for (vector<string>::const_iterator it = backs.begin();
		     it != backs.end(); ++it) {
			Graph::EdgePath p = theConverters().getPath(*it, format);
			if (!p.empty() && (path.empty() || p.size() < path.size())) {
				backend_format = *it;
				path = p;
			}
		}
		if (!path.empty())
			runparams.flavor = theConverters().getFlavor(path);
		else {
			Alert::error(_("Couldn't export file"),
				bformat(_("No information for exporting the format %1$s."),
				   formats.prettyName(format)));
			return false;
		}
	} else {
		backend_format = format;
		// FIXME: Don't hardcode format names here, but use a flag
		if (backend_format == "pdflatex")
			runparams.flavor = OutputParams::PDFLATEX;
	}

	string filename = latexName(false);
	filename = addName(temppath(), filename);
	filename = changeExtension(filename,
				   formats.extension(backend_format));

	// fix macros
	updateMacroInstances();

	// Plain text backend
	if (backend_format == "text")
		writePlaintextFile(*this, FileName(filename), runparams);
	// no backend
	else if (backend_format == "lyx")
		writeFile(FileName(filename));
	// Docbook backend
	else if (isDocBook()) {
		runparams.nice = !put_in_tempdir;
		makeDocBookFile(FileName(filename), runparams);
	}
	// LaTeX backend
	else if (backend_format == format) {
		runparams.nice = true;
		if (!makeLaTeXFile(FileName(filename), string(), runparams))
			return false;
	} else if (!lyxrc.tex_allows_spaces
		   && contains(filePath(), ' ')) {
		Alert::error(_("File name error"),
			   _("The directory path to the document cannot contain spaces."));
		return false;
	} else {
		runparams.nice = false;
		if (!makeLaTeXFile(FileName(filename), filePath(), runparams))
			return false;
	}

	string const error_type = (format == "program")
		? "Build" : bufferFormat();
	ErrorList & error_list = d->errorLists[error_type];
	string const ext = formats.extension(format);
	FileName const tmp_result_file(changeExtension(filename, ext));
	bool const success = theConverters().convert(this, FileName(filename),
		tmp_result_file, FileName(absFileName()), backend_format, format,
		error_list);
	// Emit the signal to show the error list.
	if (format != backend_format)
		errors(error_type);
	if (!success)
		return false;

	if (put_in_tempdir) {
		result_file = tmp_result_file.absFilename();
		return true;
	}

	result_file = changeExtension(absFileName(), ext);
	// We need to copy referenced files (e. g. included graphics
	// if format == "dvi") to the result dir.
	vector<ExportedFile> const files =
		runparams.exportdata->externalFiles(format);
	string const dest = onlyPath(result_file);
	CopyStatus status = SUCCESS;
	for (vector<ExportedFile>::const_iterator it = files.begin();
		it != files.end() && status != CANCEL; ++it) {
		string const fmt = formats.getFormatFromFile(it->sourceName);
		status = copyFile(fmt, it->sourceName,
			makeAbsPath(it->exportName, dest),
			it->exportName, status == FORCE);
	}
	if (status == CANCEL) {
		message(_("Document export cancelled."));
	} else if (tmp_result_file.exists()) {
		// Finally copy the main file
		status = copyFile(format, tmp_result_file,
			FileName(result_file), result_file,
			status == FORCE);
		message(bformat(_("Document exported as %1$s "
			"to file `%2$s'"),
			formats.prettyName(format),
			makeDisplayPath(result_file)));
	} else {
		// This must be a dummy converter like fax (bug 1888)
		message(bformat(_("Document exported as %1$s"),
			formats.prettyName(format)));
	}

	return true;
}


bool Buffer::doExport(string const & format, bool put_in_tempdir) const
{
	string result_file;
	return doExport(format, put_in_tempdir, result_file);
}


bool Buffer::preview(string const & format) const
{
	string result_file;
	if (!doExport(format, true, result_file))
		return false;
	return formats.view(*this, FileName(result_file), format);
}


bool Buffer::isExportable(string const & format) const
{
	vector<string> backs = backends();
	for (vector<string>::const_iterator it = backs.begin();
	     it != backs.end(); ++it)
		if (theConverters().isReachable(*it, format))
			return true;
	return false;
}


vector<Format const *> Buffer::exportableFormats(bool only_viewable) const
{
	vector<string> backs = backends();
	vector<Format const *> result =
		theConverters().getReachable(backs[0], only_viewable, true);
	for (vector<string>::const_iterator it = backs.begin() + 1;
	     it != backs.end(); ++it) {
		vector<Format const *>  r =
			theConverters().getReachable(*it, only_viewable, false);
		result.insert(result.end(), r.begin(), r.end());
	}
	return result;
}


vector<string> Buffer::backends() const
{
	vector<string> v;
	if (params().baseClass()->isTeXClassAvailable()) {
		v.push_back(bufferFormat());
		// FIXME: Don't hardcode format names here, but use a flag
		if (v.back() == "latex")
			v.push_back("pdflatex");
	}
	v.push_back("text");
	v.push_back("lyx");
	return v;
}


bool Buffer::readFileHelper(FileName const & s)
{
	// File information about normal file
	if (!s.exists()) {
		docstring const file = makeDisplayPath(s.absFilename(), 50);
		docstring text = bformat(_("The specified document\n%1$s"
						     "\ncould not be read."), file);
		Alert::error(_("Could not read document"), text);
		return false;
	}

	// Check if emergency save file exists and is newer.
	FileName const e(s.absFilename() + ".emergency");

	if (e.exists() && s.exists() && e.lastModified() > s.lastModified()) {
		docstring const file = makeDisplayPath(s.absFilename(), 20);
		docstring const text =
			bformat(_("An emergency save of the document "
				  "%1$s exists.\n\n"
					       "Recover emergency save?"), file);
		switch (Alert::prompt(_("Load emergency save?"), text, 0, 2,
				      _("&Recover"),  _("&Load Original"),
				      _("&Cancel")))
		{
		case 0:
			// the file is not saved if we load the emergency file.
			markDirty();
			return readFile(e);
		case 1:
			break;
		default:
			return false;
		}
	}

	// Now check if autosave file is newer.
	FileName const a(onlyPath(s.absFilename()) + '#' + onlyFilename(s.absFilename()) + '#');

	if (a.exists() && s.exists() && a.lastModified() > s.lastModified()) {
		docstring const file = makeDisplayPath(s.absFilename(), 20);
		docstring const text =
			bformat(_("The backup of the document "
				  "%1$s is newer.\n\nLoad the "
					       "backup instead?"), file);
		switch (Alert::prompt(_("Load backup?"), text, 0, 2,
				      _("&Load backup"), _("Load &original"),
				      _("&Cancel") ))
		{
		case 0:
			// the file is not saved if we load the autosave file.
			markDirty();
			return readFile(a);
		case 1:
			// Here we delete the autosave
			a.removeFile();
			break;
		default:
			return false;
		}
	}
	return readFile(s);
}


bool Buffer::loadLyXFile(FileName const & s)
{
	if (s.isReadableFile()) {
		if (readFileHelper(s)) {
			lyxvc().file_found_hook(s);
			if (!s.isWritable())
				setReadonly(true);
			return true;
		}
	} else {
		docstring const file = makeDisplayPath(s.absFilename(), 20);
		// Here we probably should run
		if (LyXVC::file_not_found_hook(s)) {
			docstring const text =
				bformat(_("Do you want to retrieve the document"
						       " %1$s from version control?"), file);
			int const ret = Alert::prompt(_("Retrieve from version control?"),
				text, 0, 1, _("&Retrieve"), _("&Cancel"));

			if (ret == 0) {
				// How can we know _how_ to do the checkout?
				// With the current VC support it has to be,
				// a RCS file since CVS do not have special ,v files.
				RCS::retrieve(s);
				return loadLyXFile(s);
			}
		}
	}
	return false;
}


void Buffer::bufferErrors(TeXErrors const & terr, ErrorList & errorList) const
{
	TeXErrors::Errors::const_iterator cit = terr.begin();
	TeXErrors::Errors::const_iterator end = terr.end();

	for (; cit != end; ++cit) {
		int id_start = -1;
		int pos_start = -1;
		int errorRow = cit->error_in_line;
		bool found = d->texrow.getIdFromRow(errorRow, id_start,
						       pos_start);
		int id_end = -1;
		int pos_end = -1;
		do {
			++errorRow;
			found = d->texrow.getIdFromRow(errorRow, id_end, pos_end);
		} while (found && id_start == id_end && pos_start == pos_end);

		errorList.push_back(ErrorItem(cit->error_desc,
			cit->error_text, id_start, pos_start, pos_end));
	}
}


void Buffer::setBuffersForInsets() const
{
	inset().setBuffer(const_cast<Buffer &>(*this)); 
}


void Buffer::updateLabels(UpdateScope scope) const
{
	// Use the master text class also for child documents
	Buffer const * const master = masterBuffer();
	DocumentClass const & textclass = master->params().documentClass();

	// keep the buffers to be children in this set. If the call from the
	// master comes back we can see which of them were actually seen (i.e.
	// via an InsetInclude). The remaining ones in the set need still be updated.
	static std::set<Buffer const *> bufToUpdate;
	if (scope == UpdateMaster) {
		// If this is a child document start with the master
		if (master != this) {
			bufToUpdate.insert(this);
			master->updateLabels();
			// Do this here in case the master has no gui associated with it. Then, 
			// the TocModel is not updated and TocModel::toc_ is invalid (bug 5699).
			if (!master->gui_)
				structureChanged();	

			// was buf referenced from the master (i.e. not in bufToUpdate anymore)?
			if (bufToUpdate.find(this) == bufToUpdate.end())
				return;
		}

		// start over the counters in the master
		textclass.counters().reset();
	}

	// update will be done below for this buffer
	bufToUpdate.erase(this);

	// update all caches
	clearReferenceCache();
	updateMacros();

	Buffer & cbuf = const_cast<Buffer &>(*this);

	LASSERT(!text().paragraphs().empty(), /**/);

	// do the real work
	ParIterator parit = cbuf.par_iterator_begin();
	updateLabels(parit);

	if (master != this)
		// TocBackend update will be done later.
		return;

	cbuf.tocBackend().update();
	if (scope == UpdateMaster)
		cbuf.structureChanged();
}


static depth_type getDepth(DocIterator const & it)
{
	depth_type depth = 0;
	for (size_t i = 0 ; i < it.depth() ; ++i)
		if (!it[i].inset().inMathed())
			depth += it[i].paragraph().getDepth() + 1;
	// remove 1 since the outer inset does not count
	return depth - 1;
}

static depth_type getItemDepth(ParIterator const & it)
{
	Paragraph const & par = *it;
	LabelType const labeltype = par.layout().labeltype;

	if (labeltype != LABEL_ENUMERATE && labeltype != LABEL_ITEMIZE)
		return 0;

	// this will hold the lowest depth encountered up to now.
	depth_type min_depth = getDepth(it);
	ParIterator prev_it = it;
	while (true) {
		if (prev_it.pit())
			--prev_it.top().pit();
		else {
			// start of nested inset: go to outer par
			prev_it.pop_back();
			if (prev_it.empty()) {
				// start of document: nothing to do
				return 0;
			}
		}

		// We search for the first paragraph with same label
		// that is not more deeply nested.
		Paragraph & prev_par = *prev_it;
		depth_type const prev_depth = getDepth(prev_it);
		if (labeltype == prev_par.layout().labeltype) {
			if (prev_depth < min_depth)
				return prev_par.itemdepth + 1;
			if (prev_depth == min_depth)
				return prev_par.itemdepth;
		}
		min_depth = min(min_depth, prev_depth);
		// small optimization: if we are at depth 0, we won't
		// find anything else
		if (prev_depth == 0)
			return 0;
	}
}


static bool needEnumCounterReset(ParIterator const & it)
{
	Paragraph const & par = *it;
	LASSERT(par.layout().labeltype == LABEL_ENUMERATE, /**/);
	depth_type const cur_depth = par.getDepth();
	ParIterator prev_it = it;
	while (prev_it.pit()) {
		--prev_it.top().pit();
		Paragraph const & prev_par = *prev_it;
		if (prev_par.getDepth() <= cur_depth)
			return  prev_par.layout().labeltype != LABEL_ENUMERATE;
	}
	// start of nested inset: reset
	return true;
}


// set the label of a paragraph. This includes the counters.
static void setLabel(Buffer const & buf, ParIterator & it)
{
	BufferParams const & bp = buf.masterBuffer()->params();
	DocumentClass const & textclass = bp.documentClass();
	Paragraph & par = it.paragraph();
	Layout const & layout = par.layout();
	Counters & counters = textclass.counters();

	if (par.params().startOfAppendix()) {
		// FIXME: only the counter corresponding to toplevel
		// sectionning should be reset
		counters.reset();
		counters.appendix(true);
	}
	par.params().appendix(counters.appendix());

	// Compute the item depth of the paragraph
	par.itemdepth = getItemDepth(it);

	if (layout.margintype == MARGIN_MANUAL) {
		if (par.params().labelWidthString().empty())
			par.params().labelWidthString(par.translateIfPossible(layout.labelstring(), bp));
	} else {
		par.params().labelWidthString(docstring());
	}

	switch(layout.labeltype) {
	case LABEL_COUNTER:
		if (layout.toclevel <= bp.secnumdepth
		    && (layout.latextype != LATEX_ENVIRONMENT
			|| isFirstInSequence(it.pit(), it.plist()))) {
			counters.step(layout.counter);
			par.params().labelString(
				par.expandLabel(layout, bp));
		} else
			par.params().labelString(docstring());
		break;

	case LABEL_ITEMIZE: {
		// At some point of time we should do something more
		// clever here, like:
		//   par.params().labelString(
		//    bp.user_defined_bullet(par.itemdepth).getText());
		// for now, use a simple hardcoded label
		docstring itemlabel;
		switch (par.itemdepth) {
		case 0:
			itemlabel = char_type(0x2022);
			break;
		case 1:
			itemlabel = char_type(0x2013);
			break;
		case 2:
			itemlabel = char_type(0x2217);
			break;
		case 3:
			itemlabel = char_type(0x2219); // or 0x00b7
			break;
		}
		par.params().labelString(itemlabel);
		break;
	}

	case LABEL_ENUMERATE: {
		// FIXME: Yes I know this is a really, really! bad solution
		// (Lgb)
		docstring enumcounter = from_ascii("enum");

		switch (par.itemdepth) {
		case 2:
			enumcounter += 'i';
		case 1:
			enumcounter += 'i';
		case 0:
			enumcounter += 'i';
			break;
		case 3:
			enumcounter += "iv";
			break;
		default:
			// not a valid enumdepth...
			break;
		}

		// Maybe we have to reset the enumeration counter.
		if (needEnumCounterReset(it))
			counters.reset(enumcounter);

		counters.step(enumcounter);

		string format;

		switch (par.itemdepth) {
		case 0:
			format = N_("\\arabic{enumi}.");
			break;
		case 1:
			format = N_("(\\alph{enumii})");
			break;
		case 2:
			format = N_("\\roman{enumiii}.");
			break;
		case 3:
			format = N_("\\Alph{enumiv}.");
			break;
		default:
			// not a valid enumdepth...
			break;
		}

		par.params().labelString(counters.counterLabel(
			par.translateIfPossible(from_ascii(format), bp)));

		break;
	}

	case LABEL_SENSITIVE: {
		string const & type = counters.current_float();
		docstring full_label;
		if (type.empty())
			full_label = buf.B_("Senseless!!! ");
		else {
			docstring name = buf.B_(textclass.floats().getType(type).name());
			if (counters.hasCounter(from_utf8(type))) {
				counters.step(from_utf8(type));
				full_label = bformat(from_ascii("%1$s %2$s:"), 
						     name, 
						     counters.theCounter(from_utf8(type)));
			} else
				full_label = bformat(from_ascii("%1$s #:"), name);	
		}
		par.params().labelString(full_label);	
		break;
	}

	case LABEL_NO_LABEL:
		par.params().labelString(docstring());
		break;

	case LABEL_MANUAL:
	case LABEL_TOP_ENVIRONMENT:
	case LABEL_CENTERED_TOP_ENVIRONMENT:
	case LABEL_STATIC:	
	case LABEL_BIBLIO:
		par.params().labelString(
			par.translateIfPossible(layout.labelstring(), bp));
		break;
	}
}


void Buffer::updateLabels(ParIterator & parit) const
{
	LASSERT(parit.pit() == 0, /**/);

	// set the position of the text in the buffer to be able
	// to resolve macros in it. This has nothing to do with
	// labels, but by putting it here we avoid implementing
	// a whole bunch of traversal routines just for this call.
	parit.text()->setMacrocontextPosition(parit);

	depth_type maxdepth = 0;
	pit_type const lastpit = parit.lastpit();
	for ( ; parit.pit() <= lastpit ; ++parit.pit()) {
		// reduce depth if necessary
		parit->params().depth(min(parit->params().depth(), maxdepth));
		maxdepth = parit->getMaxDepthAfter();

		// set the counter for this paragraph
		setLabel(*this, parit);

		// Now the insets
		InsetList::const_iterator iit = parit->insetList().begin();
		InsetList::const_iterator end = parit->insetList().end();
		for (; iit != end; ++iit) {
			parit.pos() = iit->pos;
			iit->inset->updateLabels(parit);
		}
	}
}


bool Buffer::nextWord(DocIterator & from, DocIterator & to,
	docstring & word) const
{
	bool inword = false;
	bool ignoreword = false;
	string lang_code;
	// Go backward a bit if needed in order to return the word currently
	// pointed by 'from'.
	while (from && from.pos() && isLetter(from))
		from.backwardPos();
	// OK, we start from here.
	to = from;
	while (to.depth()) {
		if (isLetter(to)) {
			if (!inword) {
				inword = true;
				ignoreword = false;
				from = to;
				word.clear();
				lang_code = to.paragraph().getFontSettings(params(),
					to.pos()).language()->code();
			}
			// Insets like optional hyphens and ligature
			// break are part of a word.
			if (!to.paragraph().isInset(to.pos())) {
				char_type const c = to.paragraph().getChar(to.pos());
				word += c;
				if (isDigit(c))
					ignoreword = true;
			}
		} else { // !isLetter(cur)
			if (inword && !word.empty() && !ignoreword)
				return true;
			inword = false;
		}
		to.forwardPos();
	}
	from = to;
	word.clear();
	return false;
}


int Buffer::spellCheck(DocIterator & from, DocIterator & to,
	WordLangTuple & word_lang, docstring_list & suggestions) const
{
	int progress = 0;
	SpellChecker::Result res = SpellChecker::OK;
	SpellChecker * speller = theSpellChecker();
	suggestions.clear();
	docstring word;
	while (nextWord(from, to, word)) {
		++progress;
		string lang_code = lyxrc.spellchecker_use_alt_lang
		      ? lyxrc.spellchecker_alt_lang
		      : from.paragraph().getFontSettings(params(), from.pos()).language()->code();
		WordLangTuple wl(word, lang_code);
		res = speller->check(wl);
		// ... just bail out if the spellchecker reports an error.
		if (!speller->error().empty()) {
			throw ExceptionMessage(WarningException,
				_("The spellchecker has failed."), speller->error());
		}
		if (res != SpellChecker::OK && res != SpellChecker::IGNORED_WORD) {
			word_lang = wl;
			break;
		}
		from = to;
	}
	while (!(word = speller->nextMiss()).empty())
		suggestions.push_back(word);
	return progress;
}

} // namespace lyx
