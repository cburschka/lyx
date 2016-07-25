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
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "DocIterator.h"
#include "BufferEncodings.h"
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
#include "output_xhtml.h"
#include "output_plaintext.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "PDFOptions.h"
#include "SpellChecker.h"
#include "sgml.h"
#include "TexRow.h"
#include "Text.h"
#include "TextClass.h"
#include "TocBackend.h"
#include "Undo.h"
#include "VCBackend.h"
#include "version.h"
#include "WordLangTuple.h"
#include "WordList.h"

#include "insets/InsetBibtex.h"
#include "insets/InsetBranch.h"
#include "insets/InsetInclude.h"
#include "insets/InsetTabular.h"
#include "insets/InsetText.h"

#include "mathed/InsetMathHull.h"
#include "mathed/MacroTable.h"
#include "mathed/MathMacroTemplate.h"
#include "mathed/MathSupport.h"

#include "graphics/PreviewLoader.h"

#include "frontends/alert.h"
#include "frontends/Delegates.h"
#include "frontends/WorkAreaManager.h"

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
#include "support/mutex.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/PathChanger.h"
#include "support/Systemcall.h"
#include "support/TempFile.h"
#include "support/textutils.h"
#include "support/types.h"

#include "support/bind.h"
#include "support/shared_ptr.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <vector>

using namespace std;
using namespace lyx::support;
using namespace lyx::graphics;

namespace lyx {

namespace Alert = frontend::Alert;
namespace os = support::os;

namespace {

int const LYX_FORMAT = LYX_FORMAT_LYX;

typedef map<string, bool> DepClean;
typedef map<docstring, pair<InsetLabel const *, Buffer::References> > RefCache;

} // namespace anon


// A storehouse for the cloned buffers.
list<CloneList *> cloned_buffers;


class Buffer::Impl
{
public:
	Impl(Buffer * owner, FileName const & file, bool readonly, Buffer const * cloned_buffer);

	~Impl()
	{
		delete preview_loader_;
		if (wa_) {
			wa_->closeAll();
			delete wa_;
		}
		delete inset;
	}

	/// search for macro in local (buffer) table or in children
	MacroData const * getBufferMacro(docstring const & name,
		DocIterator const & pos) const;

	/// Update macro table starting with position of it \param it in some
	/// text inset.
	void updateMacros(DocIterator & it, DocIterator & scope);
	///
	void setLabel(ParIterator & it, UpdateType utype) const;

	/** If we have branches that use the file suffix
	    feature, return the file name with suffix appended.
	*/
	support::FileName exportFileName() const;

	Buffer * owner_;

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

	/// is this an unnamed file (New...)?
	bool unnamed;

	/// is this an internal bufffer?
	bool internal_buffer;

	/// buffer is r/o
	bool read_only;

	/// name of the file the buffer is associated with.
	FileName filename;

	/** Set to true only when the file is fully loaded.
	 *  Used to prevent the premature generation of previews
	 *  and by the citation inset.
	 */
	bool file_fully_loaded;

	/// original format of loaded file
	int file_format;

	/// if the file was originally loaded from an older format, do
	/// we need to back it up still?
	bool need_format_backup;

	/// Ignore the parent (e.g. when exporting a child standalone)?
	bool ignore_parent;

	///
	mutable TocBackend toc_backend;

	/// macro tables
	struct ScopeMacro {
		ScopeMacro() {}
		ScopeMacro(DocIterator const & s, MacroData const & m)
			: scope(s), macro(m) {}
		DocIterator scope;
		MacroData macro;
	};
	typedef map<DocIterator, ScopeMacro> PositionScopeMacroMap;
	typedef map<docstring, PositionScopeMacroMap> NamePositionScopeMacroMap;
	/// map from the macro name to the position map,
	/// which maps the macro definition position to the scope and the MacroData.
	NamePositionScopeMacroMap macros;
	/// This seem to change the way Buffer::getMacro() works
	mutable bool macro_lock;

	/// positions of child buffers in the buffer
	typedef map<Buffer const * const, DocIterator> BufferPositionMap;
	struct ScopeBuffer {
		ScopeBuffer() {}
		ScopeBuffer(DocIterator const & s,Buffer const * b)
			: scope(s), buffer(b) {}
		DocIterator scope;
		Buffer const * buffer;
	};
	typedef map<DocIterator, ScopeBuffer> PositionScopeBufferMap;
	/// position of children buffers in this buffer
	BufferPositionMap children_positions;
	/// map from children inclusion positions to their scope and their buffer
	PositionScopeBufferMap position_to_children;

	/// Contains the old buffer filePath() while saving-as, or the
	/// directory where the document was last saved while loading.
	string old_position;

	/** Keeps track of the path of local layout files.
	 *  If possible, it is always relative to the buffer path.
	 *  Empty for layouts in system or user directory.
	 */
	string layout_position;

	/// Container for all sort of Buffer dependant errors.
	map<string, ErrorList> errorLists;

	/// timestamp and checksum used to test if the file has been externally
	/// modified. (Used to properly enable 'File->Revert to saved', bug 4114).
	time_t timestamp_;
	unsigned long checksum_;

	///
	frontend::WorkAreaManager * wa_;
	///
	frontend::GuiBufferDelegate * gui_;

	///
	Undo undo_;

	/// A cache for the bibfiles (including bibfiles of loaded child
	/// documents), needed for appropriate update of natbib labels.
	mutable support::FileNameList bibfiles_cache_;

	// FIXME The caching mechanism could be improved. At present, we have a
	// cache for each Buffer, that caches all the bibliography info for that
	// Buffer. A more efficient solution would be to have a global cache per
	// file, and then to construct the Buffer's bibinfo from that.
	/// A cache for bibliography info
	mutable BiblioInfo bibinfo_;
	/// whether the bibinfo cache is valid
	mutable bool bibinfo_cache_valid_;
	/// whether the bibfile cache is valid
	mutable bool bibfile_cache_valid_;
	/// Cache of timestamps of .bib files
	map<FileName, time_t> bibfile_status_;
	/// Indicates whether the bibinfo has changed since the last time
	/// we ran updateBuffer(), i.e., whether citation labels may need
	/// to be updated.
	mutable bool cite_labels_valid_;

	/// These two hold the file name and format, written to by
	/// Buffer::preview and read from by LFUN_BUFFER_VIEW_CACHE.
	FileName preview_file_;
	string preview_format_;
	/// If there was an error when previewing, on the next preview we do
	/// a fresh compile (e.g. in case the user installed a package that
	/// was missing).
	bool preview_error_;

	mutable RefCache ref_cache_;

	/// our Text that should be wrapped in an InsetText
	InsetText * inset;

	///
	PreviewLoader * preview_loader_;

	/// This is here to force the test to be done whenever parent_buffer
	/// is accessed.
	Buffer const * parent() const
	{
		// ignore_parent temporarily "orphans" a buffer
		// (e.g. if a child is compiled standalone)
		if (ignore_parent)
			return 0;
		// if parent_buffer is not loaded, then it has been unloaded,
		// which means that parent_buffer is an invalid pointer. So we
		// set it to null in that case.
		// however, the BufferList doesn't know about cloned buffers, so
		// they will always be regarded as unloaded. in that case, we hope
		// for the best.
		if (!cloned_buffer_ && !theBufferList().isLoaded(parent_buffer))
			parent_buffer = 0;
		return parent_buffer;
	}

	///
	void setParent(Buffer const * pb)
	{
		if (parent_buffer == pb)
			// nothing to do
			return;
		if (!cloned_buffer_ && parent_buffer && pb)
			LYXERR0("Warning: a buffer should not have two parents!");
		parent_buffer = pb;
		if (!cloned_buffer_ && parent_buffer) {
			parent_buffer->invalidateBibfileCache();
			parent_buffer->invalidateBibinfoCache();
		}
	}

	/// If non zero, this buffer is a clone of existing buffer \p cloned_buffer_
	/// This one is useful for preview detached in a thread.
	Buffer const * cloned_buffer_;
	///
	CloneList * clone_list_;
	/// are we in the process of exporting this buffer?
	mutable bool doing_export;

	/// compute statistics
	/// \p from initial position
	/// \p to points to the end position
	void updateStatistics(DocIterator & from, DocIterator & to,
			      bool skipNoOutput = true);
	/// statistics accessor functions
	int wordCount() const
	{
		return word_count_;
	}
	int charCount(bool with_blanks) const
	{
		return char_count_
		+ (with_blanks ? blank_count_ : 0);
	}

	// does the buffer contains tracked changes? (if so, we automatically
	// display the review toolbar)
	mutable bool tracked_changes_present_;

private:
	/// So we can force access via the accessors.
	mutable Buffer const * parent_buffer;

	int word_count_;
	int char_count_;
	int blank_count_;

};


/// Creates the per buffer temporary directory
static FileName createBufferTmpDir()
{
	// FIXME This would be the ideal application for a TempDir class (like
	//       TempFile but for directories)
	string counter;
	{
		static int count;
		static Mutex mutex;
		Mutex::Locker locker(&mutex);
		counter = convert<string>(count++);
	}
	// We are in our own directory.  Why bother to mangle name?
	// In fact I wrote this code to circumvent a problematic behaviour
	// (bug?) of EMX mkstemp().
	FileName tmpfl(package().temp_dir().absFileName() + "/lyx_tmpbuf" +
		counter);

	if (!tmpfl.createDirectory(0777)) {
		throw ExceptionMessage(WarningException, _("Disk Error: "), bformat(
			_("LyX could not create the temporary directory '%1$s' (Disk is full maybe?)"),
			from_utf8(tmpfl.absFileName())));
	}
	return tmpfl;
}


Buffer::Impl::Impl(Buffer * owner, FileName const & file, bool readonly_,
	Buffer const * cloned_buffer)
	: owner_(owner), lyx_clean(true), bak_clean(true), unnamed(false),
	  internal_buffer(false), read_only(readonly_), filename(file),
	  file_fully_loaded(false), file_format(LYX_FORMAT), need_format_backup(false),
	  ignore_parent(false),  toc_backend(owner), macro_lock(false), timestamp_(0),
	  checksum_(0), wa_(0),  gui_(0), undo_(*owner), bibinfo_cache_valid_(false),
	  bibfile_cache_valid_(false), cite_labels_valid_(false), inset(0),
	  preview_loader_(0), cloned_buffer_(cloned_buffer), clone_list_(0),
	  doing_export(false), parent_buffer(0),
	  word_count_(0), char_count_(0), blank_count_(0)
{
	if (!cloned_buffer_) {
		temppath = createBufferTmpDir();
		lyxvc.setBuffer(owner_);
		if (use_gui)
			wa_ = new frontend::WorkAreaManager;
		return;
	}
	temppath = cloned_buffer_->d->temppath;
	file_fully_loaded = true;
	params = cloned_buffer_->d->params;
	bibfiles_cache_ = cloned_buffer_->d->bibfiles_cache_;
	bibinfo_ = cloned_buffer_->d->bibinfo_;
	bibinfo_cache_valid_ = cloned_buffer_->d->bibinfo_cache_valid_;
	bibfile_cache_valid_ = cloned_buffer_->d->bibfile_cache_valid_;
	bibfile_status_ = cloned_buffer_->d->bibfile_status_;
	cite_labels_valid_ = cloned_buffer_->d->cite_labels_valid_;
	unnamed = cloned_buffer_->d->unnamed;
	internal_buffer = cloned_buffer_->d->internal_buffer;
	layout_position = cloned_buffer_->d->layout_position;
	preview_file_ = cloned_buffer_->d->preview_file_;
	preview_format_ = cloned_buffer_->d->preview_format_;
	preview_error_ = cloned_buffer_->d->preview_error_;
	tracked_changes_present_ = cloned_buffer_->d->tracked_changes_present_;
}


Buffer::Buffer(string const & file, bool readonly, Buffer const * cloned_buffer)
	: d(new Impl(this, FileName(file), readonly, cloned_buffer))
{
	LYXERR(Debug::INFO, "Buffer::Buffer()");
	if (cloned_buffer) {
		d->inset = new InsetText(*cloned_buffer->d->inset);
		d->inset->setBuffer(*this);
		// FIXME: optimize this loop somewhat, maybe by creating a new
		// general recursive Inset::setId().
		DocIterator it = doc_iterator_begin(this);
		DocIterator cloned_it = doc_iterator_begin(cloned_buffer);
		for (; !it.atEnd(); it.forwardPar(), cloned_it.forwardPar())
			it.paragraph().setId(cloned_it.paragraph().id());
	} else
		d->inset = new InsetText(this);
	d->inset->getText(0)->setMacrocontextPosition(par_iterator_begin());
}


Buffer::~Buffer()
{
	LYXERR(Debug::INFO, "Buffer::~Buffer()");
	// here the buffer should take care that it is
	// saved properly, before it goes into the void.

	// GuiView already destroyed
	d->gui_ = 0;

	if (isInternal()) {
		// No need to do additional cleanups for internal buffer.
		delete d;
		return;
	}

	if (isClone()) {
		// this is in case of recursive includes: we won't try to delete
		// ourselves as a child.
		d->clone_list_->erase(this);
		// loop over children
		Impl::BufferPositionMap::iterator it = d->children_positions.begin();
		Impl::BufferPositionMap::iterator end = d->children_positions.end();
		for (; it != end; ++it) {
			Buffer * child = const_cast<Buffer *>(it->first);
				if (d->clone_list_->erase(child))
					delete child;
		}
		// if we're the master buffer, then we should get rid of the list
		// of clones
		if (!parent()) {
			// If this is not empty, we have leaked something. Worse, one of the
			// children still has a reference to this list. But we will try to
			// continue, rather than shut down.
			LATTEST(d->clone_list_->empty());
			list<CloneList *>::iterator it =
				find(cloned_buffers.begin(), cloned_buffers.end(), d->clone_list_);
			if (it == cloned_buffers.end()) {
				// We will leak in this case, but it is safe to continue.
				LATTEST(false);
			} else
				cloned_buffers.erase(it);
			delete d->clone_list_;
		}
		// FIXME Do we really need to do this right before we delete d?
		// clear references to children in macro tables
		d->children_positions.clear();
		d->position_to_children.clear();
	} else {
		// loop over children
		Impl::BufferPositionMap::iterator it = d->children_positions.begin();
		Impl::BufferPositionMap::iterator end = d->children_positions.end();
		for (; it != end; ++it) {
			Buffer * child = const_cast<Buffer *>(it->first);
			if (theBufferList().isLoaded(child)) { 
			 if (theBufferList().isOthersChild(this, child))
				 child->setParent(0);
			 else
				theBufferList().release(child);
			}
		}

		if (!isClean()) {
			docstring msg = _("LyX attempted to close a document that had unsaved changes!\n");
			msg += emergencyWrite();
			Alert::warning(_("Attempting to close changed document!"), msg);
		}

		// FIXME Do we really need to do this right before we delete d?
		// clear references to children in macro tables
		d->children_positions.clear();
		d->position_to_children.clear();

		if (!d->temppath.destroyDirectory()) {
			LYXERR0(bformat(_("Could not remove the temporary directory %1$s"),
				from_utf8(d->temppath.absFileName())));
		}
		removePreviews();
	}

	delete d;
}


Buffer * Buffer::cloneFromMaster() const
{
	BufferMap bufmap;
	cloned_buffers.push_back(new CloneList);
	CloneList * clones = cloned_buffers.back();

	masterBuffer()->cloneWithChildren(bufmap, clones);

	// make sure we got cloned
	BufferMap::const_iterator bit = bufmap.find(this);
	LASSERT(bit != bufmap.end(), return 0);
	Buffer * cloned_buffer = bit->second;

	return cloned_buffer;
}


void Buffer::cloneWithChildren(BufferMap & bufmap, CloneList * clones) const
{
	// have we already been cloned?
	if (bufmap.find(this) != bufmap.end())
		return;

	Buffer * buffer_clone = new Buffer(fileName().absFileName(), false, this);

	// The clone needs its own DocumentClass, since running updateBuffer() will
	// modify it, and we would otherwise be sharing it with the original Buffer.
	buffer_clone->params().makeDocumentClass(true);
	ErrorList el;
	cap::switchBetweenClasses(
			params().documentClassPtr(), buffer_clone->params().documentClassPtr(),
			static_cast<InsetText &>(buffer_clone->inset()), el);

	bufmap[this] = buffer_clone;
	clones->insert(buffer_clone);
	buffer_clone->d->clone_list_ = clones;
	buffer_clone->d->macro_lock = true;
	buffer_clone->d->children_positions.clear();

	// FIXME (Abdel 09/01/2010): this is too complicated. The whole children_positions and
	// math macro caches need to be rethought and simplified.
	// I am not sure wether we should handle Buffer cloning here or in BufferList.
	// Right now BufferList knows nothing about buffer clones.
	Impl::PositionScopeBufferMap::iterator it = d->position_to_children.begin();
	Impl::PositionScopeBufferMap::iterator end = d->position_to_children.end();
	for (; it != end; ++it) {
		DocIterator dit = it->first.clone(buffer_clone);
		dit.setBuffer(buffer_clone);
		Buffer * child = const_cast<Buffer *>(it->second.buffer);

		child->cloneWithChildren(bufmap, clones);
		BufferMap::iterator const bit = bufmap.find(child);
		LASSERT(bit != bufmap.end(), continue);
		Buffer * child_clone = bit->second;

		Inset * inset = dit.nextInset();
		LASSERT(inset && inset->lyxCode() == INCLUDE_CODE, continue);
		InsetInclude * inset_inc = static_cast<InsetInclude *>(inset);
		inset_inc->setChildBuffer(child_clone);
		child_clone->d->setParent(buffer_clone);
		// FIXME Do we need to do this now, or can we wait until we run updateMacros()?
		buffer_clone->setChild(dit, child_clone);
	}
	buffer_clone->d->macro_lock = false;
	return;
}


Buffer * Buffer::cloneBufferOnly() const {
	cloned_buffers.push_back(new CloneList);
	CloneList * clones = cloned_buffers.back();
	Buffer * buffer_clone = new Buffer(fileName().absFileName(), false, this);

	// The clone needs its own DocumentClass, since running updateBuffer() will
	// modify it, and we would otherwise be sharing it with the original Buffer.
	buffer_clone->params().makeDocumentClass(true);
	ErrorList el;
	cap::switchBetweenClasses(
			params().documentClassPtr(), buffer_clone->params().documentClassPtr(),
			static_cast<InsetText &>(buffer_clone->inset()), el);

	clones->insert(buffer_clone);
	buffer_clone->d->clone_list_ = clones;

	// we won't be cloning the children
	buffer_clone->d->children_positions.clear();
	return buffer_clone;
}


bool Buffer::isClone() const
{
	return d->cloned_buffer_;
}


void Buffer::changed(bool update_metrics) const
{
	if (d->wa_)
		d->wa_->redrawAll(update_metrics);
}


frontend::WorkAreaManager & Buffer::workAreaManager() const
{
	LBUFERR(d->wa_);
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


BufferParams const & Buffer::masterParams() const
{
	if (masterBuffer() == this)
		return params();

	BufferParams & mparams = const_cast<Buffer *>(masterBuffer())->params();
	// Copy child authors to the params. We need those pointers.
	AuthorList const & child_authors = params().authors();
	AuthorList::Authors::const_iterator it = child_authors.begin();
	for (; it != child_authors.end(); ++it)
		mparams.authors().record(*it);
	return mparams;
}


double Buffer::fontScalingFactor() const
{
	return isExporting() ? 75.0 * params().html_math_img_scale
		: 0.01 * lyxrc.dpi * lyxrc.zoom * lyxrc.preview_scale_factor * params().display_pixel_ratio;
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
	return d->temppath.absFileName();
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


void Buffer::setChild(DocIterator const & dit, Buffer * child)
{
	d->children_positions[child] = dit;
}


string Buffer::latexName(bool const no_path) const
{
	FileName latex_name =
		makeLatexName(d->exportFileName());
	return no_path ? latex_name.onlyFileName()
		: latex_name.absFileName();
}


FileName Buffer::Impl::exportFileName() const
{
	docstring const branch_suffix =
		params.branchlist().getFileNameSuffix();
	if (branch_suffix.empty())
		return filename;

	string const name = filename.onlyFileNameWithoutExt()
		+ to_utf8(branch_suffix);
	FileName res(filename.onlyPath().absFileName() + "/" + name);
	res.changeExtension(filename.extension());

	return res;
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
				     onlyFileName(changeExtension(filename,
								  ".log"))));

	// FIXME: how do we know this is the name of the build log?
	FileName const bname(
		addName(path, onlyFileName(
			changeExtension(filename,
					formats.extension(params().bufferFormat()) + ".out"))));

	// Also consider the master buffer log file
	FileName masterfname = fname;
	LogType mtype = latexlog;
	if (masterBuffer() != this) {
		string const mlogfile = masterBuffer()->logName(&mtype);
		masterfname = FileName(mlogfile);
	}

	// If no Latex log or Build log is newer, show Build log
	if (bname.exists() &&
	    ((!fname.exists() && !masterfname.exists())
	     || (fname.lastModified() < bname.lastModified()
	         && masterfname.lastModified() < bname.lastModified()))) {
		LYXERR(Debug::FILES, "Log name calculated as: " << bname);
		if (type)
			*type = buildlog;
		return bname.absFileName();
	// If we have a newer master file log or only a master log, show this
	} else if (fname != masterfname
		   && (!fname.exists() && (masterfname.exists()
		   || fname.lastModified() < masterfname.lastModified()))) {
		LYXERR(Debug::FILES, "Log name calculated as: " << masterfname);
		if (type)
			*type = mtype;
		return masterfname.absFileName();
	}
	LYXERR(Debug::FILES, "Log name calculated as: " << fname);
	if (type)
			*type = latexlog;
	return fname.absFileName();
}


void Buffer::setReadonly(bool const flag)
{
	if (d->read_only != flag) {
		d->read_only = flag;
		changed(false);
	}
}


void Buffer::setFileName(FileName const & fname)
{
	bool const changed = fname != d->filename;
	d->filename = fname;
	if (changed)
		lyxvc().file_found_hook(fname);
	setReadonly(d->filename.isReadOnly());
	saveCheckSum();
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
	params().fonts_cjk.erase();
	params().listings_params.clear();
	params().clearLayoutModules();
	params().clearRemovedModules();
	params().clearIncludedChildren();
	params().pdfoptions().clear();
	params().indiceslist().clear();
	params().backgroundcolor = lyx::rgbFromHexName("#ffffff");
	params().isbackgroundcolor = false;
	params().fontcolor = RGBColor(0, 0, 0);
	params().isfontcolor = false;
	params().notefontcolor = RGBColor(0xCC, 0xCC, 0xCC);
	params().boxbgcolor = RGBColor(0xFF, 0, 0);
	params().html_latex_start.clear();
	params().html_latex_end.clear();
	params().html_math_img_scale = 1.0;
	params().output_sync_macro.erase();
	params().setLocalLayout(string(), false);
	params().setLocalLayout(string(), true);

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

		string const result =
			params().readToken(lex, token, d->filename.onlyPath());
		if (!result.empty()) {
			if (token == "\\textclass") {
				d->layout_position = result;
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

	// remove dummy empty par
	paragraphs().clear();

	if (!lex.checkFor("\\begin_document")) {
		docstring const s = _("\\begin_document is missing");
		errorList.push_back(ErrorItem(_("Document header error"),
			s, -1, 0, 0));
	}

	readHeader(lex);

	if (params().output_changes) {
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

	if (!parent() && !params().master.empty()) {
		FileName const master_file = makeAbsPath(params().master,
			   onlyPath(absFileName()));
		if (isLyXFileName(master_file.absFileName())) {
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

	// assure we have a default index
	params().indiceslist().addDefault(B_("Index"));

	// read main text
	if (FileName::isAbsolute(params().origin))
		d->old_position = params().origin;
	else
		d->old_position = filePath();
	bool const res = text().read(lex, errorList, d->inset);
	d->old_position.clear();

	// inform parent buffer about local macros
	if (parent()) {
		Buffer const * pbuf = parent();
		UserMacroSet::const_iterator cit = usermacros.begin();
		UserMacroSet::const_iterator end = usermacros.end();
		for (; cit != end; ++cit)
			pbuf->usermacros.insert(*cit);
	}
	usermacros.clear();
	updateMacros();
	updateMacroInstances(InternalUpdate);
	return res;
}


bool Buffer::importString(string const & format, docstring const & contents, ErrorList & errorList)
{
	Format const * fmt = formats.getFormat(format);
	if (!fmt)
		return false;
	// It is important to use the correct extension here, since some
	// converters create a wrong output file otherwise (e.g. html2latex)
	TempFile const tempfile("Buffer_importStringXXXXXX." + fmt->extension());
	FileName const name(tempfile.name());
	ofdocstream os(name.toFilesystemEncoding().c_str());
	// Do not convert os implicitly to bool, since that is forbidden in C++11.
	bool const success = !(os << contents).fail();
	os.close();

	bool converted = false;
	if (success) {
		params().compressed = false;

		// remove dummy empty par
		paragraphs().clear();

		converted = importFile(format, name, errorList);
	}

	if (name.exists())
		name.removeFile();
	return converted;
}


bool Buffer::importFile(string const & format, FileName const & name, ErrorList & errorList)
{
	if (!theConverters().isReachable(format, "lyx"))
		return false;

	TempFile const tempfile("Buffer_importFileXXXXXX.lyx");
	FileName const lyx(tempfile.name());
	if (theConverters().convert(0, name, lyx, name, format, "lyx", errorList))
		return readFile(lyx) == ReadSuccess;

	return false;
}


bool Buffer::readString(string const & s)
{
	params().compressed = false;

	Lexer lex;
	istringstream is(s);
	lex.setStream(is);
	TempFile tempfile("Buffer_readStringXXXXXX.lyx");
	FileName const fn = tempfile.name();

	int file_format;
	bool success = parseLyXFormat(lex, fn, file_format) == ReadSuccess;

	if (success && file_format != LYX_FORMAT) {
		// We need to call lyx2lyx, so write the input to a file
		ofstream os(fn.toFilesystemEncoding().c_str());
		os << s;
		os.close();
		// lyxvc in readFile
		if (readFile(fn) != ReadSuccess)
			success = false;
	}
	else if (success)
		if (readDocument(lex))
			success = false;
	return success;
}


Buffer::ReadStatus Buffer::readFile(FileName const & fn)
{
	FileName fname(fn);
	Lexer lex;
	if (!lex.setFile(fname)) {
		Alert::error(_("File Not Found"),
			bformat(_("Unable to open file `%1$s'."),
			        from_utf8(fn.absFileName())));
		return ReadFileNotFound;
	}

	int file_format;
	ReadStatus const ret_plf = parseLyXFormat(lex, fn, file_format);
	if (ret_plf != ReadSuccess)
		return ret_plf;

	if (file_format != LYX_FORMAT) {
		FileName tmpFile;
		ReadStatus ret_clf = convertLyXFormat(fn, tmpFile, file_format);
		if (ret_clf != ReadSuccess)
			return ret_clf;
		ret_clf = readFile(tmpFile);
		if (ret_clf == ReadSuccess) {
			d->file_format = file_format;
			d->need_format_backup = true;
		}
		return ret_clf;
	}

	// FIXME: InsetInfo needs to know whether the file is under VCS
	// during the parse process, so this has to be done before.
	lyxvc().file_found_hook(d->filename);

	if (readDocument(lex)) {
		Alert::error(_("Document format failure"),
			bformat(_("%1$s ended unexpectedly, which means"
				" that it is probably corrupted."),
					from_utf8(fn.absFileName())));
		return ReadDocumentFailure;
	}

	d->file_fully_loaded = true;
	d->read_only = !d->filename.isWritable();
	params().compressed = formats.isZippedFile(d->filename);
	saveCheckSum();
	return ReadSuccess;
}


bool Buffer::isFullyLoaded() const
{
	return d->file_fully_loaded;
}


void Buffer::setFullyLoaded(bool value)
{
	d->file_fully_loaded = value;
}


bool Buffer::lastPreviewError() const
{
	return d->preview_error_;
}


PreviewLoader * Buffer::loader() const
{
	if (!isExporting() && lyxrc.preview == LyXRC::PREVIEW_OFF)
		return 0;
	if (!d->preview_loader_)
		d->preview_loader_ = new PreviewLoader(*this);
	return d->preview_loader_;
}


void Buffer::removePreviews() const
{
	delete d->preview_loader_;
	d->preview_loader_ = 0;
}


void Buffer::updatePreviews() const
{
	PreviewLoader * ploader = loader();
	if (!ploader)
		return;

	InsetIterator it = inset_iterator_begin(*d->inset);
	InsetIterator const end = inset_iterator_end(*d->inset);
	for (; it != end; ++it)
		it->addPreview(it, *ploader);

	ploader->startLoading();
}


Buffer::ReadStatus Buffer::parseLyXFormat(Lexer & lex,
	FileName const & fn, int & file_format) const
{
	if(!lex.checkFor("\\lyxformat")) {
		Alert::error(_("Document format failure"),
			bformat(_("%1$s is not a readable LyX document."),
				from_utf8(fn.absFileName())));
		return ReadNoLyXFormat;
	}

	string tmp_format;
	lex >> tmp_format;

	// LyX formats 217 and earlier were written as 2.17. This corresponds
	// to files from LyX versions < 1.1.6.3. We just remove the dot in
	// these cases. See also: www.lyx.org/trac/changeset/1313.
	size_t dot = tmp_format.find_first_of(".,");
	if (dot != string::npos)
		tmp_format.erase(dot, 1);

	file_format = convert<int>(tmp_format);
	return ReadSuccess;
}


Buffer::ReadStatus Buffer::convertLyXFormat(FileName const & fn,
	FileName & tmpfile, int from_format)
{
	TempFile tempfile("Buffer_convertLyXFormatXXXXXX.lyx");
	tempfile.setAutoRemove(false);
	tmpfile = tempfile.name();
	if(tmpfile.empty()) {
		Alert::error(_("Conversion failed"),
			bformat(_("%1$s is from a different"
				" version of LyX, but a temporary"
				" file for converting it could"
				" not be created."),
				from_utf8(fn.absFileName())));
		return LyX2LyXNoTempFile;
	}

	FileName const lyx2lyx = libFileSearch("lyx2lyx", "lyx2lyx");
	if (lyx2lyx.empty()) {
		Alert::error(_("Conversion script not found"),
		     bformat(_("%1$s is from a different"
			       " version of LyX, but the"
			       " conversion script lyx2lyx"
			       " could not be found."),
			       from_utf8(fn.absFileName())));
		return LyX2LyXNotFound;
	}

	// Run lyx2lyx:
	//   $python$ "$lyx2lyx$" -t $LYX_FORMAT$ -o "$tempfile$" "$filetoread$"
	ostringstream command;
	command << os::python()
		<< ' ' << quoteName(lyx2lyx.toFilesystemEncoding())
		<< " -t " << convert<string>(LYX_FORMAT)
		<< " -o " << quoteName(tmpfile.toFilesystemEncoding())
		<< ' ' << quoteName(fn.toSafeFilesystemEncoding());
	string const command_str = command.str();

	LYXERR(Debug::INFO, "Running '" << command_str << '\'');

	cmd_ret const ret = runCommand(command_str);
	if (ret.first != 0) {
		if (from_format < LYX_FORMAT) {
			Alert::error(_("Conversion script failed"),
				bformat(_("%1$s is from an older version"
					" of LyX and the lyx2lyx script"
					" failed to convert it."),
					from_utf8(fn.absFileName())));
			return LyX2LyXOlderFormat;
		} else {
			Alert::error(_("Conversion script failed"),
				bformat(_("%1$s is from a newer version"
					" of LyX and the lyx2lyx script"
					" failed to convert it."),
					from_utf8(fn.absFileName())));
			return LyX2LyXNewerFormat;
		}
	}
	return ReadSuccess;
}


FileName Buffer::getBackupName() const {
	FileName const & fn = fileName();
	string const fname = fn.onlyFileNameWithoutExt();
	string const fext  = fn.extension();
	string const fpath = lyxrc.backupdir_path.empty() ?
		fn.onlyPath().absFileName() :
		lyxrc.backupdir_path;
	string const fform = convert<string>(d->file_format);
	string const backname = fname + "-lyxformat-" + fform;
	FileName backup(addName(fpath, addExtension(backname, fext)));

	// limit recursion, just in case
	int v = 1;
	unsigned long orig_checksum = 0;
	while (backup.exists() && v < 100) {
		if (orig_checksum == 0)
			orig_checksum = fn.checksum();
		unsigned long new_checksum = backup.checksum();
		if (orig_checksum == new_checksum) {
			LYXERR(Debug::FILES, "Not backing up " << fn <<
			       "since " << backup << "has the same checksum.");
			// a bit of a hack, but we have to check this anyway
			// below, and setting this is simpler than introducing
			// a special boolean for this purpose.
			v = 1000;
			break;
		}
		string const newbackname = backname + "-" + convert<string>(v);
		backup.set(addName(fpath, addExtension(newbackname, fext)));
		v++;
	}
	return v < 100 ? backup : FileName();
}


// Should probably be moved to somewhere else: BufferView? GuiView?
bool Buffer::save() const
{
	docstring const file = makeDisplayPath(absFileName(), 20);
	d->filename.refresh();

	// check the read-only status before moving the file as a backup
	if (d->filename.exists()) {
		bool const read_only = !d->filename.isWritable();
		if (read_only) {
			Alert::warning(_("File is read-only"),
				bformat(_("The file %1$s cannot be written because it "
				"is marked as read-only."), file));
			return false;
		}
	}

	// ask if the disk file has been externally modified (use checksum method)
	if (fileName().exists() && isExternallyModified(checksum_method)) {
		docstring text =
			bformat(_("Document %1$s has been externally modified. "
				"Are you sure you want to overwrite this file?"), file);
		int const ret = Alert::prompt(_("Overwrite modified file?"),
			text, 1, 1, _("&Overwrite"), _("&Cancel"));
		if (ret == 1)
			return false;
	}

	// We don't need autosaves in the immediate future. (Asger)
	resetAutosaveTimers();

	// if the file does not yet exist, none of the backup activity
	// that follows is necessary
  if (!fileName().exists()) {
		if (!writeFile(fileName()))
      return false;
    markClean();
    return true;
  }

	// we first write the file to a new name, then move it to its
	// proper location once that has been done successfully. that
	// way we preserve the original file if something goes wrong.
	string const justname = fileName().onlyFileNameWithoutExt();
	boost::scoped_ptr<TempFile>
		tempfile(new TempFile(fileName().onlyPath(),
	           justname + "-XXXXXX.lyx"));
	bool const symlink = fileName().isSymLink();
	if (!symlink)
		tempfile->setAutoRemove(false);

	FileName savefile(tempfile->name());
	LYXERR(Debug::FILES, "Saving to " << savefile.absFileName());
	if (!writeFile(savefile))
		return false;

	// we will set this to false if we fail
	bool made_backup = true;

	FileName backupName;
	bool const needBackup = lyxrc.make_backup || d->need_format_backup;
	if (needBackup) {
		if (d->need_format_backup)
			backupName = getBackupName();

		// If we for some reason failed to find a backup name in case of
		// a format change, this will still set one. It's the best we can
		// do in this case.
		if (backupName.empty()) {
			backupName.set(fileName().absFileName() + "~");
			if (!lyxrc.backupdir_path.empty()) {
				string const mangledName =
					subst(subst(backupName.absFileName(), '/', '!'), ':', '!');
				backupName.set(addName(lyxrc.backupdir_path, mangledName));
			}
		}

		LYXERR(Debug::FILES, "Backing up original file to " <<
				backupName.absFileName());
		// Except file is symlink do not copy because of #6587.
		// Hard links have bad luck.
		made_backup = symlink ?
			fileName().copyTo(backupName):
			fileName().moveTo(backupName);

		if (!made_backup) {
			Alert::error(_("Backup failure"),
				     bformat(_("Cannot create backup file %1$s.\n"
					       "Please check whether the directory exists and is writable."),
					     from_utf8(backupName.absFileName())));
			//LYXERR(Debug::DEBUG, "Fs error: " << fe.what());
		} else if (d->need_format_backup) {
			// the original file has been backed up successfully, so we
			// will not need to do that again
			d->need_format_backup = false;
		}
	}

	// Destroy tempfile since it keeps the file locked on windows (bug 9234)
	// Only do this if tempfile is not in autoremove mode
	if (!symlink)
		tempfile.reset();
	// If we have no symlink, we can simply rename the temp file.
	// Otherwise, we need to copy it so the symlink stays intact.
	if (made_backup && symlink ? savefile.copyTo(fileName(), true) :
		                           savefile.moveTo(fileName()))
	{
		// saveCheckSum() was already called by writeFile(), but the
		// time stamp is invalidated by copying/moving
		saveCheckSum();
		markClean();
		if (d->file_format != LYX_FORMAT)
			// the file associated with this buffer is now in the current format
			d->file_format = LYX_FORMAT;
		return true;
	}
	// else we saved the file, but failed to move it to the right location.

	if (needBackup && made_backup && !symlink) {
		// the original file was moved to some new location, so it will look
		// to the user as if it was deleted. (see bug #9234.) we could try
		// to restore it, but that would basically mean trying to do again
		// what we just failed to do. better to leave things as they are.
		Alert::error(_("Write failure"),
		             bformat(_("The file has successfully been saved as:\n  %1$s.\n"
		                       "But LyX could not move it to:\n  %2$s.\n"
		                       "Your original file has been backed up to:\n  %3$s"),
		                     from_utf8(savefile.absFileName()),
		                     from_utf8(fileName().absFileName()),
		                     from_utf8(backupName.absFileName())));
	} else {
		// either we did not try to make a backup, or else we tried and failed,
		// or else the original file was a symlink, in which case it was copied,
		// not moved. so the original file is intact.
		Alert::error(_("Write failure"),
			     bformat(_("Cannot move saved file to:\n  %1$s.\n"
				       "But the file has successfully been saved as:\n  %2$s."),
				     from_utf8(fileName().absFileName()),
		         from_utf8(savefile.absFileName())));
	}
	return false;
}


bool Buffer::writeFile(FileName const & fname) const
{
	if (d->read_only && fname == d->filename)
		return false;

	bool retval = false;

	docstring const str = bformat(_("Saving document %1$s..."),
		makeDisplayPath(fname.absFileName()));
	message(str);

	string const encoded_fname = fname.toSafeFilesystemEncoding(os::CREATE);

	if (params().compressed) {
		gz::ogzstream ofs(encoded_fname.c_str(), ios::out|ios::trunc);
		retval = ofs && write(ofs);
	} else {
		ofstream ofs(encoded_fname.c_str(), ios::out|ios::trunc);
		retval = ofs && write(ofs);
	}

	if (!retval) {
		message(str + _(" could not write file!"));
		return false;
	}

	// see bug 6587
	// removeAutosaveFile();

	saveCheckSum();
	message(str + _(" done."));

	return true;
}


docstring Buffer::emergencyWrite()
{
	// No need to save if the buffer has not changed.
	if (isClean())
		return docstring();

	string const doc = isUnnamed() ? onlyFileName(absFileName()) : absFileName();

	docstring user_message = bformat(
		_("LyX: Attempting to save document %1$s\n"), from_utf8(doc));

	// We try to save three places:
	// 1) Same place as document. Unless it is an unnamed doc.
	if (!isUnnamed()) {
		string s = absFileName();
		s += ".emergency";
		LYXERR0("  " << s);
		if (writeFile(FileName(s))) {
			markClean();
			user_message += "  " + bformat(_("Saved to %1$s. Phew.\n"), from_utf8(s));
			return user_message;
		} else {
			user_message += "  " + _("Save failed! Trying again...\n");
		}
	}

	// 2) In HOME directory.
	string s = addName(Package::get_home_dir().absFileName(), absFileName());
	s += ".emergency";
	lyxerr << ' ' << s << endl;
	if (writeFile(FileName(s))) {
		markClean();
		user_message += "  " + bformat(_("Saved to %1$s. Phew.\n"), from_utf8(s));
		return user_message;
	}

	user_message += "  " + _("Save failed! Trying yet again...\n");

	// 3) In "/tmp" directory.
	// MakeAbsPath to prepend the current
	// drive letter on OS/2
	s = addName(package().temp_dir().absFileName(), absFileName());
	s += ".emergency";
	lyxerr << ' ' << s << endl;
	if (writeFile(FileName(s))) {
		markClean();
		user_message += "  " + bformat(_("Saved to %1$s. Phew.\n"), from_utf8(s));
		return user_message;
	}

	user_message += "  " + _("Save failed! Bummer. Document is lost.");
	// Don't try again.
	markClean();
	return user_message;
}


bool Buffer::write(ostream & ofs) const
{
#ifdef HAVE_LOCALE
	// Use the standard "C" locale for file output.
	ofs.imbue(locale::classic());
#endif

	// The top of the file should not be written by params().

	// write out a comment in the top of the file
	// Important: Keep the version formatting in sync with lyx2lyx and
	//            tex2lyx (bug 7951)
	ofs << "#LyX " << lyx_version_major << "." << lyx_version_minor
	    << " created this file. For more info see http://www.lyx.org/\n"
	    << "\\lyxformat " << LYX_FORMAT << "\n"
	    << "\\begin_document\n";

	/// For each author, set 'used' to true if there is a change
	/// by this author in the document; otherwise set it to 'false'.
	AuthorList::Authors::const_iterator a_it = params().authors().begin();
	AuthorList::Authors::const_iterator a_end = params().authors().end();
	for (; a_it != a_end; ++a_it)
		a_it->setUsed(false);

	ParIterator const end = const_cast<Buffer *>(this)->par_iterator_end();
	ParIterator it = const_cast<Buffer *>(this)->par_iterator_begin();
	for ( ; it != end; ++it)
		it->checkAuthors(params().authors());

	// now write out the buffer parameters.
	ofs << "\\begin_header\n";
	params().writeFile(ofs, this);
	ofs << "\\end_header\n";

	// write the text
	ofs << "\n\\begin_body\n";
	text().write(ofs);
	ofs << "\n\\end_body\n";

	// Write marker that shows file is complete
	ofs << "\\end_document" << endl;

	// Shouldn't really be needed....
	//ofs.close();

	// how to check if close went ok?
	// Following is an attempt... (BE 20001011)

	// good() returns false if any error occurred, including some
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
			   OutputWhat output) const
{
	OutputParams runparams = runparams_in;

	// XeTeX with TeX fonts is only safe with ASCII encoding (see also #9740),
	// Check here, because the "flavor" is not known in BufferParams::encoding()
	// (power users can override this safety measure selecting "utf8-plain").
	if (!params().useNonTeXFonts && (runparams.flavor == OutputParams::XETEX)
	    && (runparams.encoding->name() != "utf8-plain"))
		runparams.encoding = encodings.fromLyXName("ascii");

	string const encoding = runparams.encoding->iconvName();
	LYXERR(Debug::LATEX, "makeLaTeXFile encoding: " << encoding << ", fname=" << fname.realPath());

	ofdocstream ofs;
	try { ofs.reset(encoding); }
	catch (iconv_codecvt_facet_exception const & e) {
		lyxerr << "Caught iconv exception: " << e.what() << endl;
		Alert::error(_("Iconv software exception Detected"), bformat(_("Please "
			"verify that the support software for your encoding (%1$s) is "
			"properly installed"), from_ascii(encoding)));
		return false;
	}
	if (!openFileWrite(ofs, fname))
		return false;

	ErrorList & errorList = d->errorLists["Export"];
	errorList.clear();
	bool failed_export = false;
	otexstream os(ofs, d->texrow);

	// make sure we are ready to export
	// this needs to be done before we validate
	// FIXME Do we need to do this all the time? I.e., in children
	// of a master we are exporting?
	updateBuffer();
	updateMacroInstances(OutputUpdate);

	try {
		os.texrow().reset();
		writeLaTeXSource(os, original_path, runparams, output);
	}
	catch (EncodingException const & e) {
		docstring const failed(1, e.failed_char);
		ostringstream oss;
		oss << "0x" << hex << e.failed_char << dec;
		docstring msg = bformat(_("Could not find LaTeX command for character '%1$s'"
					  " (code point %2$s)"),
					  failed, from_utf8(oss.str()));
		errorList.push_back(ErrorItem(msg, _("Some characters of your document are probably not "
				"representable in the chosen encoding.\n"
				"Changing the document encoding to utf8 could help."),
				e.par_id, e.pos, e.pos + 1));
		failed_export = true;
	}
	catch (iconv_codecvt_facet_exception const & e) {
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

	if (runparams_in.silent)
		errorList.clear();
	else
		errors("Export");
	return !failed_export;
}


void Buffer::writeLaTeXSource(otexstream & os,
			   string const & original_path,
			   OutputParams const & runparams_in,
			   OutputWhat output) const
{
	// The child documents, if any, shall be already loaded at this point.

	OutputParams runparams = runparams_in;

	// XeTeX with TeX fonts is only safe with ASCII encoding,
	// Check here, because the "flavor" is not known in BufferParams::encoding()
	// (power users can override this safety measure selecting "utf8-plain").
	if (!params().useNonTeXFonts && (runparams.flavor == OutputParams::XETEX)
	    && (runparams.encoding->name() != "utf8-plain"))
		runparams.encoding = encodings.fromLyXName("ascii");
	// FIXME: when only the current paragraph is shown, this is ignored
	//        (or not reached) and characters encodable in the current
	//        encoding are not converted to ASCII-representation.

	// If we are compiling a file standalone, even if this is the
	// child of some other buffer, let's cut the link here, so the
	// file is really independent and no concurring settings from
	// the master (e.g. branch state) interfere (see #8100).
	if (!runparams.is_child)
		d->ignore_parent = true;

	// Classify the unicode characters appearing in math insets
	BufferEncodings::initUnicodeMath(*this);

	// validate the buffer.
	LYXERR(Debug::LATEX, "  Validating buffer...");
	LaTeXFeatures features(*this, params(), runparams);
	validate(features);
	// This is only set once per document (in master)
	if (!runparams.is_child)
		runparams.use_polyglossia = features.usePolyglossia();
	LYXERR(Debug::LATEX, "  Buffer validation done.");

	bool const output_preamble =
		output == FullSource || output == OnlyPreamble;
	bool const output_body =
		output == FullSource || output == OnlyBody;

	// The starting paragraph of the coming rows is the
	// first paragraph of the document. (Asger)
	if (output_preamble && runparams.nice) {
		os << "%% LyX " << lyx_version << " created this file.  "
			"For more info, see http://www.lyx.org/.\n"
			"%% Do not edit unless you really know what "
			"you are doing.\n";
	}
	LYXERR(Debug::INFO, "lyx document header finished");

	// There are a few differences between nice LaTeX and usual files:
	// usual files have \batchmode and special input@path to allow
	// inclusion of figures specified by an explicitly relative path
	// (i.e., a path starting with './' or '../') with either \input or
	// \includegraphics, as the TEXINPUTS method doesn't work in this case.
	// input@path is set when the actual parameter original_path is set.
	// This is done for usual tex-file, but not for nice-latex-file.
	// (Matthias 250696)
	// Note that input@path is only needed for something the user does
	// in the preamble, included .tex files or ERT, files included by
	// LyX work without it.
	if (output_preamble) {
		if (!runparams.nice) {
			// code for usual, NOT nice-latex-file
			os << "\\batchmode\n"; // changed from \nonstopmode
		}
		if (!original_path.empty()) {
			// FIXME UNICODE
			// We don't know the encoding of inputpath
			docstring const inputpath = from_utf8(original_path);
			docstring uncodable_glyphs;
			Encoding const * const enc = runparams.encoding;
			if (enc) {
				for (size_t n = 0; n < inputpath.size(); ++n) {
					if (!enc->encodable(inputpath[n])) {
						docstring const glyph(1, inputpath[n]);
						LYXERR0("Uncodable character '"
							<< glyph
							<< "' in input path!");
						uncodable_glyphs += glyph;
					}
				}
			}

			// warn user if we found uncodable glyphs.
			if (!uncodable_glyphs.empty()) {
				frontend::Alert::warning(
					_("Uncodable character in file path"),
					support::bformat(
					  _("The path of your document\n"
					    "(%1$s)\n"
					    "contains glyphs that are unknown "
					    "in the current document encoding "
					    "(namely %2$s). This may result in "
					    "incomplete output, unless "
					    "TEXINPUTS contains the document "
					    "directory and you don't use "
					    "explicitly relative paths (i.e., "
					    "paths starting with './' or "
					    "'../') in the preamble or in ERT."
					    "\n\nIn case of problems, choose "
					    "an appropriate document encoding\n"
					    "(such as utf8) or change the "
					    "file path name."),
					  inputpath, uncodable_glyphs));
			} else {
				string docdir =
					support::latex_path(original_path);
				if (contains(docdir, '#')) {
					docdir = subst(docdir, "#", "\\#");
					os << "\\catcode`\\#=11"
					      "\\def\\#{#}\\catcode`\\#=6\n";
				}
				if (contains(docdir, '%')) {
					docdir = subst(docdir, "%", "\\%");
					os << "\\catcode`\\%=11"
					      "\\def\\%{%}\\catcode`\\%=14\n";
				}
				os << "\\makeatletter\n"
				   << "\\def\\input@path{{"
				   << docdir << "}}\n"
				   << "\\makeatother\n";
			}
		}

		// get parent macros (if this buffer has a parent) which will be
		// written at the document begin further down.
		MacroSet parentMacros;
		listParentMacros(parentMacros, features);

		// Write the preamble
		runparams.use_babel = params().writeLaTeX(os, features,
							  d->filename.onlyPath());

		// Japanese might be required only in some children of a document,
		// but once required, we must keep use_japanese true.
		runparams.use_japanese |= features.isRequired("japanese");

		if (!output_body) {
			// Restore the parenthood if needed
			if (!runparams.is_child)
				d->ignore_parent = false;
			return;
		}

		// make the body.
		os << "\\begin{document}\n";

		// output the parent macros
		MacroSet::iterator it = parentMacros.begin();
		MacroSet::iterator end = parentMacros.end();
		for (; it != end; ++it) {
			int num_lines = (*it)->write(os.os(), true);
			os.texrow().newlines(num_lines);
		}

	} // output_preamble

	os.texrow().start(paragraphs().begin()->id(), 0);

	LYXERR(Debug::INFO, "preamble finished, now the body.");

	// the real stuff
	latexParagraphs(*this, text(), os, runparams);

	// Restore the parenthood if needed
	if (!runparams.is_child)
		d->ignore_parent = false;

	// add this just in case after all the paragraphs
	os << endl;

	if (output_preamble) {
		os << "\\end{document}\n";
		LYXERR(Debug::LATEX, "makeLaTeXFile...done");
	} else {
		LYXERR(Debug::LATEX, "LaTeXFile for inclusion made.");
	}
	runparams_in.encoding = runparams.encoding;

	os.texrow().finalize();

	LYXERR(Debug::INFO, "Finished making LaTeX file.");
	LYXERR(Debug::INFO, "Row count was " << os.texrow().rows() - 1 << '.');
}


void Buffer::makeDocBookFile(FileName const & fname,
			      OutputParams const & runparams,
			      OutputWhat output) const
{
	LYXERR(Debug::LATEX, "makeDocBookFile...");

	ofdocstream ofs;
	if (!openFileWrite(ofs, fname))
		return;

	// make sure we are ready to export
	// this needs to be done before we validate
	updateBuffer();
	updateMacroInstances(OutputUpdate);

	writeDocBookSource(ofs, fname.absFileName(), runparams, output);

	ofs.close();
	if (ofs.fail())
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
}


void Buffer::writeDocBookSource(odocstream & os, string const & fname,
			     OutputParams const & runparams,
			     OutputWhat output) const
{
	LaTeXFeatures features(*this, params(), runparams);
	validate(features);

	d->texrow.reset(false);

	DocumentClass const & tclass = params().documentClass();
	string const & top_element = tclass.latexname();

	bool const output_preamble =
		output == FullSource || output == OnlyPreamble;
	bool const output_body =
	  output == FullSource || output == OnlyBody;

	if (output_preamble) {
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

	if (output_body) {
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

		sgml::openTag(os, top);
		os << '\n';
		docbookParagraphs(text(), *this, os, runparams);
		sgml::closeTag(os, top_element);
	}
}


void Buffer::makeLyXHTMLFile(FileName const & fname,
			      OutputParams const & runparams) const
{
	LYXERR(Debug::LATEX, "makeLyXHTMLFile...");

	ofdocstream ofs;
	if (!openFileWrite(ofs, fname))
		return;

	// make sure we are ready to export
	// this has to be done before we validate
	updateBuffer(UpdateMaster, OutputUpdate);
	updateMacroInstances(OutputUpdate);

	writeLyXHTMLSource(ofs, runparams, FullSource);

	ofs.close();
	if (ofs.fail())
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
}


void Buffer::writeLyXHTMLSource(odocstream & os,
			     OutputParams const & runparams,
			     OutputWhat output) const
{
	LaTeXFeatures features(*this, params(), runparams);
	validate(features);
	d->bibinfo_.makeCitationLabels(*this);

	bool const output_preamble =
		output == FullSource || output == OnlyPreamble;
	bool const output_body =
	  output == FullSource || output == OnlyBody || output == IncludedFile;

	if (output_preamble) {
		os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		   << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN\" \"http://www.w3.org/Math/DTD/mathml2/xhtml-math11-f.dtd\">\n"
		   // FIXME Language should be set properly.
		   << "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
		   << "<head>\n"
		   << "<meta name=\"GENERATOR\" content=\"" << PACKAGE_STRING << "\" />\n"
		   // FIXME Presumably need to set this right
		   << "<meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\" />\n";

		docstring const & doctitle = features.htmlTitle();
		os << "<title>"
		   << (doctitle.empty() ?
		         from_ascii("LyX Document") :
		         html::htmlize(doctitle, XHTMLStream::ESCAPE_ALL))
		   << "</title>\n";

		docstring styles = features.getTClassHTMLPreamble();
		if (!styles.empty())
			os << "\n<!-- Text Class Preamble -->\n" << styles << '\n';

		styles = from_utf8(features.getPreambleSnippets());
		if (!styles.empty())
			os << "\n<!-- Preamble Snippets -->\n" << styles << '\n';

		// we will collect CSS information in a stream, and then output it
		// either here, as part of the header, or else in a separate file.
		odocstringstream css;
		styles = from_utf8(features.getCSSSnippets());
		if (!styles.empty())
			css << "/* LyX Provided Styles */\n" << styles << '\n';

		styles = features.getTClassHTMLStyles();
		if (!styles.empty())
			css << "/* Layout-provided Styles */\n" << styles << '\n';

		bool const needfg = params().fontcolor != RGBColor(0, 0, 0);
		bool const needbg = params().backgroundcolor != RGBColor(0xFF, 0xFF, 0xFF);
		if (needfg || needbg) {
				css << "\nbody {\n";
				if (needfg)
				   css << "  color: "
					    << from_ascii(X11hexname(params().fontcolor))
					    << ";\n";
				if (needbg)
				   css << "  background-color: "
					    << from_ascii(X11hexname(params().backgroundcolor))
					    << ";\n";
				css << "}\n";
		}

		docstring const dstyles = css.str();
		if (!dstyles.empty()) {
			bool written = false;
			if (params().html_css_as_file) {
				// open a file for CSS info
				ofdocstream ocss;
				string const fcssname = addName(temppath(), "docstyle.css");
				FileName const fcssfile = FileName(fcssname);
				if (openFileWrite(ocss, fcssfile)) {
					ocss << dstyles;
					ocss.close();
					written = true;
					// write link to header
					os << "<link rel='stylesheet' href='docstyle.css' type='text/css' />\n";
					// register file
					runparams.exportdata->addExternalFile("xhtml", fcssfile);
				}
			}
			// we are here if the CSS is supposed to be written to the header
			// or if we failed to write it to an external file.
			if (!written) {
				os << "<style type='text/css'>\n"
					 << dstyles
					 << "\n</style>\n";
			}
		}
		os << "</head>\n";
	}

	if (output_body) {
		bool const output_body_tag = (output != IncludedFile);
		if (output_body_tag)
			os << "<body>\n";
		XHTMLStream xs(os);
		if (output != IncludedFile)
			// if we're an included file, the counters are in the master.
			params().documentClass().counters().reset();
		xhtmlParagraphs(text(), *this, xs, runparams);
		if (output_body_tag)
			os << "</body>\n";
	}

	if (output_preamble)
		os << "</html>\n";
}


// chktex should be run with these flags disabled: 3, 22, 25, 30, 38(?)
// Other flags: -wall -v0 -x
int Buffer::runChktex()
{
	setBusy(true);

	// get LaTeX-Filename
	FileName const path(temppath());
	string const name = addName(path.absFileName(), latexName());
	string const org_path = filePath();

	PathChanger p(path); // path to LaTeX file
	message(_("Running chktex..."));

	// Generate the LaTeX file if neccessary
	OutputParams runparams(&params().encoding());
	runparams.flavor = OutputParams::LATEX;
	runparams.nice = false;
	runparams.linelen = lyxrc.plaintext_linelen;
	makeLaTeXFile(FileName(name), org_path, runparams);

	TeXErrors terr;
	Chktex chktex(lyxrc.chktex_command, onlyFileName(name), filePath());
	int const res = chktex.run(terr); // run chktex

	if (res == -1) {
		Alert::error(_("chktex failure"),
			     _("Could not run chktex successfully."));
	} else {
		ErrorList & errlist = d->errorLists["ChkTeX"];
		errlist.clear();
		bufferErrors(terr, errlist);
	}

	setBusy(false);

	if (runparams.silent)
		d->errorLists["ChkTeX"].clear();
	else
		errors("ChkTeX");

	return res;
}


void Buffer::validate(LaTeXFeatures & features) const
{
	// Validate the buffer params, but not for included
	// files, since they also use the parent buffer's
	// params (# 5941)
	if (!features.runparams().is_child)
		params().validate(features);

	for_each(paragraphs().begin(), paragraphs().end(),
		 bind(&Paragraph::validate, _1, ref(features)));

	if (lyxerr.debugging(Debug::LATEX)) {
		features.showStruct();
	}
}


void Buffer::getLabelList(vector<docstring> & list) const
{
	// If this is a child document, use the master's list instead.
	if (parent()) {
		masterBuffer()->getLabelList(list);
		return;
	}

	list.clear();
	shared_ptr<Toc> toc = d->toc_backend.toc("label");
	TocIterator toc_it = toc->begin();
	TocIterator end = toc->end();
	for (; toc_it != end; ++toc_it) {
		if (toc_it->depth() == 0)
			list.push_back(toc_it->str());
	}
}


void Buffer::updateBibfilesCache(UpdateScope scope) const
{
	// FIXME This is probably unnecssary, given where we call this.
	// If this is a child document, use the parent's cache instead.
	if (parent() && scope != UpdateChildOnly) {
		masterBuffer()->updateBibfilesCache();
		return;
	}

	d->bibfiles_cache_.clear();
	for (InsetIterator it = inset_iterator_begin(inset()); it; ++it) {
		if (it->lyxCode() == BIBTEX_CODE) {
			InsetBibtex const & inset = static_cast<InsetBibtex const &>(*it);
			support::FileNameList const bibfiles = inset.getBibFiles();
			d->bibfiles_cache_.insert(d->bibfiles_cache_.end(),
				bibfiles.begin(),
				bibfiles.end());
		} else if (it->lyxCode() == INCLUDE_CODE) {
			InsetInclude & inset = static_cast<InsetInclude &>(*it);
			Buffer const * const incbuf = inset.getChildBuffer();
			if (!incbuf)
				continue;
			support::FileNameList const & bibfiles =
					incbuf->getBibfilesCache(UpdateChildOnly);
			if (!bibfiles.empty()) {
				d->bibfiles_cache_.insert(d->bibfiles_cache_.end(),
					bibfiles.begin(),
					bibfiles.end());
			}
		}
	}
	d->bibfile_cache_valid_ = true;
	d->bibinfo_cache_valid_ = false;
	d->cite_labels_valid_ = false;
}


void Buffer::invalidateBibinfoCache() const
{
	d->bibinfo_cache_valid_ = false;
	d->cite_labels_valid_ = false;
	// also invalidate the cache for the parent buffer
	Buffer const * const pbuf = d->parent();
	if (pbuf)
		pbuf->invalidateBibinfoCache();
}


void Buffer::invalidateBibfileCache() const
{
	d->bibfile_cache_valid_ = false;
	d->bibinfo_cache_valid_ = false;
	d->cite_labels_valid_ = false;
	// also invalidate the cache for the parent buffer
	Buffer const * const pbuf = d->parent();
	if (pbuf)
		pbuf->invalidateBibfileCache();
}


support::FileNameList const & Buffer::getBibfilesCache(UpdateScope scope) const
{
	// FIXME This is probably unnecessary, given where we call this.
	// If this is a child document, use the master's cache instead.
	Buffer const * const pbuf = masterBuffer();
	if (pbuf != this && scope != UpdateChildOnly)
		return pbuf->getBibfilesCache();

	if (!d->bibfile_cache_valid_)
		this->updateBibfilesCache(scope);

	return d->bibfiles_cache_;
}


BiblioInfo const & Buffer::masterBibInfo() const
{
	Buffer const * const tmp = masterBuffer();
	if (tmp != this)
		return tmp->masterBibInfo();
	return d->bibinfo_;
}


void Buffer::checkIfBibInfoCacheIsValid() const
{
	// use the master's cache
	Buffer const * const tmp = masterBuffer();
	if (tmp != this) {
		tmp->checkIfBibInfoCacheIsValid();
		return;
	}

	// compare the cached timestamps with the actual ones.
	FileNameList const & bibfiles_cache = getBibfilesCache();
	FileNameList::const_iterator ei = bibfiles_cache.begin();
	FileNameList::const_iterator en = bibfiles_cache.end();
	for (; ei != en; ++ ei) {
		time_t lastw = ei->lastModified();
		time_t prevw = d->bibfile_status_[*ei];
		if (lastw != prevw) {
			d->bibinfo_cache_valid_ = false;
			d->cite_labels_valid_ = false;
			d->bibfile_status_[*ei] = lastw;
		}
	}
}


void Buffer::reloadBibInfoCache() const
{
	// use the master's cache
	Buffer const * const tmp = masterBuffer();
	if (tmp != this) {
		tmp->reloadBibInfoCache();
		return;
	}

	checkIfBibInfoCacheIsValid();
	if (d->bibinfo_cache_valid_)
		return;

	d->bibinfo_.clear();
	collectBibKeys();
	d->bibinfo_cache_valid_ = true;
}


void Buffer::collectBibKeys() const
{
	for (InsetIterator it = inset_iterator_begin(inset()); it; ++it)
		it->collectBibKeys(it);
}


void Buffer::addBiblioInfo(BiblioInfo const & bi) const
{
	Buffer const * tmp = masterBuffer();
	BiblioInfo & masterbi = (tmp == this) ?
		d->bibinfo_ : tmp->d->bibinfo_;
	masterbi.mergeBiblioInfo(bi);
}


void Buffer::addBibTeXInfo(docstring const & key, BibTeXInfo const & bi) const
{
	Buffer const * tmp = masterBuffer();
	BiblioInfo & masterbi = (tmp == this) ?
		d->bibinfo_ : tmp->d->bibinfo_;
	masterbi[key] = bi;
}


void Buffer::makeCitationLabels() const
{
	Buffer const * const master = masterBuffer();
	return d->bibinfo_.makeCitationLabels(*master);
}


bool Buffer::citeLabelsValid() const
{
	return masterBuffer()->d->cite_labels_valid_;
}


void Buffer::removeBiblioTempFiles() const
{
	// We remove files that contain LaTeX commands specific to the
	// particular bibliographic style being used, in order to avoid
	// LaTeX errors when we switch style.
	FileName const aux_file(addName(temppath(), changeExtension(latexName(),".aux")));
	FileName const bbl_file(addName(temppath(), changeExtension(latexName(),".bbl")));
	LYXERR(Debug::FILES, "Removing the .aux file " << aux_file);
	aux_file.removeFile();
	LYXERR(Debug::FILES, "Removing the .bbl file " << bbl_file);
	bbl_file.removeFile();
	// Also for the parent buffer
	Buffer const * const pbuf = parent();
	if (pbuf)
		pbuf->removeBiblioTempFiles();
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
	if (isInternal()) {
		// FIXME? if there is an Buffer LFUN that can be dispatched even
		// if internal, put a switch '(cmd.action)' here.
		return false;
	}

	bool enable = true;

	switch (cmd.action()) {

	case LFUN_BUFFER_TOGGLE_READ_ONLY:
		flag.setOnOff(isReadonly());
		break;

		// FIXME: There is need for a command-line import.
		//case LFUN_BUFFER_IMPORT:

	case LFUN_BUFFER_AUTO_SAVE:
		break;

	case LFUN_BUFFER_EXPORT_CUSTOM:
		// FIXME: Nothing to check here?
		break;

	case LFUN_BUFFER_EXPORT: {
		docstring const arg = cmd.argument();
		if (arg == "custom") {
			enable = true;
			break;
		}
		string format = to_utf8(arg);
		size_t pos = format.find(' ');
		if (pos != string::npos)
			format = format.substr(0, pos);
		enable = params().isExportable(format);
		if (!enable)
			flag.message(bformat(
					     _("Don't know how to export to format: %1$s"), arg));
		break;
	}

	case LFUN_BUFFER_CHKTEX:
		enable = params().isLatex() && !lyxrc.chktex_command.empty();
		break;

	case LFUN_BUILD_PROGRAM:
		enable = params().isExportable("program");
		break;

	case LFUN_BRANCH_ACTIVATE:
	case LFUN_BRANCH_DEACTIVATE:
	case LFUN_BRANCH_MASTER_ACTIVATE:
	case LFUN_BRANCH_MASTER_DEACTIVATE: {
		bool const master = (cmd.action() == LFUN_BRANCH_MASTER_ACTIVATE
				     || cmd.action() == LFUN_BRANCH_MASTER_DEACTIVATE);
		BranchList const & branchList = master ? masterBuffer()->params().branchlist()
			: params().branchlist();
		docstring const branchName = cmd.argument();
		flag.setEnabled(!branchName.empty() && branchList.find(branchName));
		break;
	}

	case LFUN_BRANCH_ADD:
	case LFUN_BRANCHES_RENAME:
		// if no Buffer is present, then of course we won't be called!
		break;

	case LFUN_BUFFER_LANGUAGE:
		enable = !isReadonly();
		break;

	case LFUN_BUFFER_VIEW_CACHE:
		(d->preview_file_).refresh();
		enable = (d->preview_file_).exists() && !(d->preview_file_).isFileEmpty();
		break;

	case LFUN_CHANGES_TRACK:
		flag.setEnabled(true);
		flag.setOnOff(params().track_changes);
		break;

	case LFUN_CHANGES_OUTPUT:
		flag.setEnabled(true);
		flag.setOnOff(params().output_changes);
		break;

	case LFUN_BUFFER_TOGGLE_COMPRESSION: {
		flag.setOnOff(params().compressed);
		break;
	}

	case LFUN_BUFFER_TOGGLE_OUTPUT_SYNC: {
		flag.setOnOff(params().output_sync);
		break;
	}

	default:
		return false;
	}
	flag.setEnabled(enable);
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
	if (isInternal()) {
		// FIXME? if there is an Buffer LFUN that can be dispatched even
		// if internal, put a switch '(cmd.action())' here.
		dr.dispatched(false);
		return;
	}
	string const argument = to_utf8(func.argument());
	// We'll set this back to false if need be.
	bool dispatched = true;
	undo().beginUndoGroup();

	switch (func.action()) {
	case LFUN_BUFFER_TOGGLE_READ_ONLY:
		if (lyxvc().inUse()) {
			string log = lyxvc().toggleReadOnly();
			if (!log.empty())
				dr.setMessage(log);
		}
		else
			setReadonly(!isReadonly());
		break;

	case LFUN_BUFFER_EXPORT: {
		ExportStatus const status = doExport(argument, false);
		dr.setError(status != ExportSuccess);
		if (status != ExportSuccess)
			dr.setMessage(bformat(_("Error exporting to format: %1$s."),
					      func.argument()));
		break;
	}

	case LFUN_BUILD_PROGRAM: {
		ExportStatus const status = doExport("program", true);
		dr.setError(status != ExportSuccess);
		if (status != ExportSuccess)
			dr.setMessage(_("Error generating literate programming code."));
		break;
	}

	case LFUN_BUFFER_CHKTEX:
		runChktex();
		break;

	case LFUN_BUFFER_EXPORT_CUSTOM: {
		string format_name;
		string command = split(argument, format_name, ' ');
		Format const * format = formats.getFormat(format_name);
		if (!format) {
			lyxerr << "Format \"" << format_name
				<< "\" not recognized!"
				<< endl;
			break;
		}

		// The name of the file created by the conversion process
		string filename;

		// Output to filename
		if (format->name() == "lyx") {
			string const latexname = latexName(false);
			filename = changeExtension(latexname,
				format->extension());
			filename = addName(temppath(), filename);

			if (!writeFile(FileName(filename)))
				break;

		} else {
			doExport(format_name, true, filename);
		}

		// Substitute $$FName for filename
		if (!contains(command, "$$FName"))
			command = "( " + command + " ) < $$FName";
		command = subst(command, "$$FName", filename);

		// Execute the command in the background
		Systemcall call;
		call.startscript(Systemcall::DontWait, command,
		                 filePath(), layoutPos());
		break;
	}

	// FIXME: There is need for a command-line import.
	/*
	case LFUN_BUFFER_IMPORT:
		doImport(argument);
		break;
	*/

	case LFUN_BUFFER_AUTO_SAVE:
		autoSave();
		resetAutosaveTimers();
		break;

	case LFUN_BRANCH_ACTIVATE:
	case LFUN_BRANCH_DEACTIVATE:
	case LFUN_BRANCH_MASTER_ACTIVATE:
	case LFUN_BRANCH_MASTER_DEACTIVATE: {
		bool const master = (func.action() == LFUN_BRANCH_MASTER_ACTIVATE
				     || func.action() == LFUN_BRANCH_MASTER_DEACTIVATE);
		Buffer * buf = master ? const_cast<Buffer *>(masterBuffer())
				      : this;

		docstring const branch_name = func.argument();
		// the case without a branch name is handled elsewhere
		if (branch_name.empty()) {
			dispatched = false;
			break;
		}
		Branch * branch = buf->params().branchlist().find(branch_name);
		if (!branch) {
			LYXERR0("Branch " << branch_name << " does not exist.");
			dr.setError(true);
			docstring const msg =
				bformat(_("Branch \"%1$s\" does not exist."), branch_name);
			dr.setMessage(msg);
			break;
		}
		bool const activate = (func.action() == LFUN_BRANCH_ACTIVATE
				       || func.action() == LFUN_BRANCH_MASTER_ACTIVATE);
		if (branch->isSelected() != activate) {
			buf->undo().recordUndoBufferParams(CursorData());
			branch->setSelected(activate);
			dr.setError(false);
			dr.screenUpdate(Update::Force);
			dr.forceBufferUpdate();
		}
		break;
	}

	case LFUN_BRANCH_ADD: {
		docstring branch_name = func.argument();
		if (branch_name.empty()) {
			dispatched = false;
			break;
		}
		BranchList & branch_list = params().branchlist();
		vector<docstring> const branches =
			getVectorFromString(branch_name, branch_list.separator());
		docstring msg;
		for (vector<docstring>::const_iterator it = branches.begin();
		     it != branches.end(); ++it) {
			branch_name = *it;
			Branch * branch = branch_list.find(branch_name);
			if (branch) {
				LYXERR0("Branch " << branch_name << " already exists.");
				dr.setError(true);
				if (!msg.empty())
					msg += ("\n");
				msg += bformat(_("Branch \"%1$s\" already exists."), branch_name);
			} else {
				undo().recordUndoBufferParams(CursorData());
				branch_list.add(branch_name);
				branch = branch_list.find(branch_name);
				string const x11hexname = X11hexname(branch->color());
				docstring const str = branch_name + ' ' + from_ascii(x11hexname);
				lyx::dispatch(FuncRequest(LFUN_SET_COLOR, str));
				dr.setError(false);
				dr.screenUpdate(Update::Force);
			}
		}
		if (!msg.empty())
			dr.setMessage(msg);
		break;
	}

	case LFUN_BRANCHES_RENAME: {
		if (func.argument().empty())
			break;

		docstring const oldname = from_utf8(func.getArg(0));
		docstring const newname = from_utf8(func.getArg(1));
		InsetIterator it  = inset_iterator_begin(inset());
		InsetIterator const end = inset_iterator_end(inset());
		bool success = false;
		for (; it != end; ++it) {
			if (it->lyxCode() == BRANCH_CODE) {
				InsetBranch & ins = static_cast<InsetBranch &>(*it);
				if (ins.branch() == oldname) {
					undo().recordUndo(CursorData(it));
					ins.rename(newname);
					success = true;
					continue;
				}
			}
			if (it->lyxCode() == INCLUDE_CODE) {
				// get buffer of external file
				InsetInclude const & ins =
					static_cast<InsetInclude const &>(*it);
				Buffer * child = ins.getChildBuffer();
				if (!child)
					continue;
				child->dispatch(func, dr);
			}
		}

		if (success) {
			dr.screenUpdate(Update::Force);
			dr.forceBufferUpdate();
		}
		break;
	}

	case LFUN_BUFFER_VIEW_CACHE:
		if (!formats.view(*this, d->preview_file_,
				  d->preview_format_))
			dr.setMessage(_("Error viewing the output file."));
		break;

	case LFUN_CHANGES_TRACK:
		if (params().save_transient_properties)
			undo().recordUndoBufferParams(CursorData());
		params().track_changes = !params().track_changes;
		if (!params().track_changes)
			dr.forceChangesUpdate();
		break;

	case LFUN_CHANGES_OUTPUT:
		if (params().save_transient_properties)
			undo().recordUndoBufferParams(CursorData());
		params().output_changes = !params().output_changes;
		if (params().output_changes) {
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
		break;

	case LFUN_BUFFER_TOGGLE_COMPRESSION:
		// turn compression on/off
		undo().recordUndoBufferParams(CursorData());
		params().compressed = !params().compressed;
		break;

	case LFUN_BUFFER_TOGGLE_OUTPUT_SYNC:
		undo().recordUndoBufferParams(CursorData());
		params().output_sync = !params().output_sync;
		break;

	default:
		dispatched = false;
		break;
	}
	dr.dispatched(dispatched);
	undo().endUndoGroup();
}


void Buffer::changeLanguage(Language const * from, Language const * to)
{
	LASSERT(from, return);
	LASSERT(to, return);

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


std::set<Language const *> Buffer::getLanguages() const
{
	std::set<Language const *> languages;
	getLanguages(languages);
	return languages;
}


void Buffer::getLanguages(std::set<Language const *> & languages) const
{
	ParConstIterator end = par_iterator_end();
	// add the buffer language, even if it's not actively used
	languages.insert(language());
	// iterate over the paragraphs
	for (ParConstIterator it = par_iterator_begin(); it != end; ++it)
		it->getLanguages(languages);
	// also children
	ListOfBuffers clist = getDescendents();
	ListOfBuffers::const_iterator cit = clist.begin();
	ListOfBuffers::const_iterator const cen = clist.end();
	for (; cit != cen; ++cit)
		(*cit)->getLanguages(languages);
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


bool Buffer::isExternallyModified(CheckMethod method) const
{
	LASSERT(d->filename.exists(), return false);
	// if method == timestamp, check timestamp before checksum
	return (method == checksum_method
		|| d->timestamp_ != d->filename.lastModified())
		&& d->checksum_ != d->filename.checksum();
}


void Buffer::saveCheckSum() const
{
	FileName const & file = d->filename;

	file.refresh();
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
	d->undo_.markDirty();
}


void Buffer::setUnnamed(bool flag)
{
	d->unnamed = flag;
}


bool Buffer::isUnnamed() const
{
	return d->unnamed;
}


/// \note
/// Don't check unnamed, here: isInternal() is used in
/// newBuffer(), where the unnamed flag has not been set by anyone
/// yet. Also, for an internal buffer, there should be no need for
/// retrieving fileName() nor for checking if it is unnamed or not.
bool Buffer::isInternal() const
{
	return d->internal_buffer;
}


void Buffer::setInternal(bool flag)
{
	d->internal_buffer = flag;
}


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
	return d->filename.absFileName();
}


string Buffer::filePath() const
{
	string const abs = d->filename.onlyPath().absFileName();
	if (abs.empty())
		return abs;
	int last = abs.length() - 1;

	return abs[last] == '/' ? abs : abs + '/';
}


DocFileName Buffer::getReferencedFileName(string const & fn) const
{
	DocFileName result;
	if (FileName::isAbsolute(fn) || !FileName::isAbsolute(params().origin))
		result.set(fn, filePath());
	else {
		// filePath() ends with a path separator
		FileName const test(filePath() + fn);
		if (test.exists())
			result.set(fn, filePath());
		else
			result.set(fn, params().origin);
	}

	return result;
}


string Buffer::layoutPos() const
{
	return d->layout_position;
}


void Buffer::setLayoutPos(string const & path)
{
	if (path.empty()) {
		d->layout_position.clear();
		return;
	}

	LATTEST(FileName::isAbsolute(path));

	d->layout_position =
		to_utf8(makeRelPath(from_utf8(path), from_utf8(filePath())));

	if (d->layout_position.empty())
		d->layout_position = ".";
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


ListOfBuffers Buffer::allRelatives() const
{
	ListOfBuffers lb = masterBuffer()->getDescendents();
	lb.push_front(const_cast<Buffer *>(masterBuffer()));
	return lb;
}


Buffer const * Buffer::masterBuffer() const
{
	// FIXME Should be make sure we are not in some kind
	// of recursive include? A -> B -> A will crash this.
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


bool Buffer::hasChildren() const
{
	return !d->children_positions.empty();
}


void Buffer::collectChildren(ListOfBuffers & clist, bool grand_children) const
{
	// loop over children
	Impl::BufferPositionMap::iterator it = d->children_positions.begin();
	Impl::BufferPositionMap::iterator end = d->children_positions.end();
	for (; it != end; ++it) {
		Buffer * child = const_cast<Buffer *>(it->first);
		// No duplicates
		ListOfBuffers::const_iterator bit = find(clist.begin(), clist.end(), child);
		if (bit != clist.end())
			continue;
		clist.push_back(child);
		if (grand_children)
			// there might be grandchildren
			child->collectChildren(clist, true);
	}
}


ListOfBuffers Buffer::getChildren() const
{
	ListOfBuffers v;
	collectChildren(v, false);
	// Make sure we have not included ourselves.
	ListOfBuffers::iterator bit = find(v.begin(), v.end(), this);
	if (bit != v.end()) {
		LYXERR0("Recursive include detected in `" << fileName() << "'.");
		v.erase(bit);
	}
	return v;
}


ListOfBuffers Buffer::getDescendents() const
{
	ListOfBuffers v;
	collectChildren(v, true);
	// Make sure we have not included ourselves.
	ListOfBuffers::iterator bit = find(v.begin(), v.end(), this);
	if (bit != v.end()) {
		LYXERR0("Recursive include detected in `" << fileName() << "'.");
		v.erase(bit);
	}
	return v;
}


template<typename M>
typename M::const_iterator greatest_below(M & m, typename M::key_type const & x)
{
	if (m.empty())
		return m.end();

	typename M::const_iterator it = m.lower_bound(x);
	if (it == m.begin())
		return m.end();

	it--;
	return it;
}


MacroData const * Buffer::Impl::getBufferMacro(docstring const & name,
					 DocIterator const & pos) const
{
	LYXERR(Debug::MACROS, "Searching for " << to_ascii(name) << " at " << pos);

	// if paragraphs have no macro context set, pos will be empty
	if (pos.empty())
		return 0;

	// we haven't found anything yet
	DocIterator bestPos = owner_->par_iterator_begin();
	MacroData const * bestData = 0;

	// find macro definitions for name
	NamePositionScopeMacroMap::const_iterator nameIt = macros.find(name);
	if (nameIt != macros.end()) {
		// find last definition in front of pos or at pos itself
		PositionScopeMacroMap::const_iterator it
			= greatest_below(nameIt->second, pos);
		if (it != nameIt->second.end()) {
			while (true) {
				// scope ends behind pos?
				if (pos < it->second.scope) {
					// Looks good, remember this. If there
					// is no external macro behind this,
					// we found the right one already.
					bestPos = it->first;
					bestData = &it->second.macro;
					break;
				}

				// try previous macro if there is one
				if (it == nameIt->second.begin())
					break;
				--it;
			}
		}
	}

	// find macros in included files
	PositionScopeBufferMap::const_iterator it
		= greatest_below(position_to_children, pos);
	if (it == position_to_children.end())
		// no children before
		return bestData;

	while (true) {
		// do we know something better (i.e. later) already?
		if (it->first < bestPos )
			break;

		// scope ends behind pos?
		if (pos < it->second.scope
			&& (cloned_buffer_ ||
			    theBufferList().isLoaded(it->second.buffer))) {
			// look for macro in external file
			macro_lock = true;
			MacroData const * data
				= it->second.buffer->getMacro(name, false);
			macro_lock = false;
			if (data) {
				bestPos = it->first;
				bestData = data;
				break;
			}
		}

		// try previous file if there is one
		if (it == position_to_children.begin())
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
	MacroData const * data = d->getBufferMacro(name, pos);
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


void Buffer::Impl::updateMacros(DocIterator & it, DocIterator & scope)
{
	pit_type const lastpit = it.lastpit();

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

			if (iit->inset->asInsetTabular()) {
				CursorSlice slice(*iit->inset);
				size_t const numcells = slice.nargs();
				for (; slice.idx() < numcells; slice.forwardIdx()) {
					it.push_back(slice);
					updateMacros(it, scope);
					it.pop_back();
				}
				continue;
			}

			// is it an external file?
			if (iit->inset->lyxCode() == INCLUDE_CODE) {
				// get buffer of external file
				InsetInclude const & inset =
					static_cast<InsetInclude const &>(*iit->inset);
				macro_lock = true;
				Buffer * child = inset.getChildBuffer();
				macro_lock = false;
				if (!child)
					continue;

				// register its position, but only when it is
				// included first in the buffer
				if (children_positions.find(child) ==
					children_positions.end())
						children_positions[child] = it;

				// register child with its scope
				position_to_children[it] = Impl::ScopeBuffer(scope, child);
				continue;
			}

			InsetMath * im = iit->inset->asInsetMath();
			if (doing_export && im)  {
				InsetMathHull * hull = im->asHullInset();
				if (hull)
					hull->recordLocation(it);
			}

			if (iit->inset->lyxCode() != MATHMACRO_CODE)
				continue;

			// get macro data
			MathMacroTemplate & macroTemplate =
				*iit->inset->asInsetMath()->asMacroTemplate();
			MacroContext mc(owner_, it);
			macroTemplate.updateToContext(mc);

			// valid?
			bool valid = macroTemplate.validMacro();
			// FIXME: Should be fixNameAndCheckIfValid() in fact,
			// then the BufferView's cursor will be invalid in
			// some cases which leads to crashes.
			if (!valid)
				continue;

			// register macro
			// FIXME (Abdel), I don't understand why we pass 'it' here
			// instead of 'macroTemplate' defined above... is this correct?
			macros[macroTemplate.name()][it] =
				Impl::ScopeMacro(scope, MacroData(const_cast<Buffer *>(owner_), it));
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
	d->updateMacros(it, outerScope);
}


void Buffer::getUsedBranches(std::list<docstring> & result, bool const from_master) const
{
	InsetIterator it  = inset_iterator_begin(inset());
	InsetIterator const end = inset_iterator_end(inset());
	for (; it != end; ++it) {
		if (it->lyxCode() == BRANCH_CODE) {
			InsetBranch & br = static_cast<InsetBranch &>(*it);
			docstring const name = br.branch();
			if (!from_master && !params().branchlist().find(name))
				result.push_back(name);
			else if (from_master && !masterBuffer()->params().branchlist().find(name))
				result.push_back(name);
			continue;
		}
		if (it->lyxCode() == INCLUDE_CODE) {
			// get buffer of external file
			InsetInclude const & ins =
				static_cast<InsetInclude const &>(*it);
			Buffer * child = ins.getChildBuffer();
			if (!child)
				continue;
			child->getUsedBranches(result, true);
		}
	}
	// remove duplicates
	result.unique();
}


void Buffer::updateMacroInstances(UpdateType utype) const
{
	LYXERR(Debug::MACROS, "updateMacroInstances for "
		<< d->filename.onlyFileName());
	DocIterator it = doc_iterator_begin(this);
	it.forwardInset();
	DocIterator const end = doc_iterator_end(this);
	for (; it != end; it.forwardInset()) {
 		// look for MathData cells in InsetMathNest insets
		InsetMath * minset = it.nextInset()->asInsetMath();
		if (!minset)
			continue;

		// update macro in all cells of the InsetMathNest
		DocIterator::idx_type n = minset->nargs();
		MacroContext mc = MacroContext(this, it);
		for (DocIterator::idx_type i = 0; i < n; ++i) {
			MathData & data = minset->cell(i);
			data.updateMacros(0, mc, utype);
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


Buffer::References & Buffer::getReferenceCache(docstring const & label)
{
	if (d->parent())
		return const_cast<Buffer *>(masterBuffer())->getReferenceCache(label);

	RefCache::iterator it = d->ref_cache_.find(label);
	if (it != d->ref_cache_.end())
		return it->second.second;

	static InsetLabel const * dummy_il = 0;
	static References const dummy_refs = References();
	it = d->ref_cache_.insert(
		make_pair(label, make_pair(dummy_il, dummy_refs))).first;
	return it->second.second;
}


Buffer::References const & Buffer::references(docstring const & label) const
{
	return const_cast<Buffer *>(this)->getReferenceCache(label);
}


void Buffer::addReference(docstring const & label, Inset * inset, ParIterator it)
{
	References & refs = getReferenceCache(label);
	refs.push_back(make_pair(inset, it));
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


void Buffer::changeRefsIfUnique(docstring const & from, docstring const & to)
{
	//FIXME: This does not work for child documents yet.
	reloadBibInfoCache();

	// Check if the label 'from' appears more than once
	BiblioInfo const & keys = masterBibInfo();
	BiblioInfo::const_iterator bit  = keys.begin();
	BiblioInfo::const_iterator bend = keys.end();
	vector<docstring> labels;

	for (; bit != bend; ++bit)
		// FIXME UNICODE
		labels.push_back(bit->first);

	if (count(labels.begin(), labels.end(), from) > 1)
		return;

	string const paramName = "key";
	for (InsetIterator it = inset_iterator_begin(inset()); it; ++it) {
		if (it->lyxCode() != CITE_CODE)
			continue;
		InsetCommand * inset = it->asInsetCommand();
		docstring const oldValue = inset->getParam(paramName);
		if (oldValue == from)
			inset->setParam(paramName, to);
	}
}

// returns NULL if id-to-row conversion is unsupported
auto_ptr<TexRow> Buffer::getSourceCode(odocstream & os, string const & format,
			   pit_type par_begin, pit_type par_end,
			   OutputWhat output, bool master) const
{
	auto_ptr<TexRow> texrow(NULL);
	OutputParams runparams(&params().encoding());
	runparams.nice = true;
	runparams.flavor = params().getOutputFlavor(format);
	runparams.linelen = lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;

	if (output == CurrentParagraph) {
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
		// output paragraphs
		if (runparams.flavor == OutputParams::LYX) {
			Paragraph const & par = text().paragraphs()[par_begin];
			ostringstream ods;
			depth_type dt = par.getDepth();
			par.write(ods, params(), dt);
			os << from_utf8(ods.str());
		} else if (runparams.flavor == OutputParams::HTML) {
			XHTMLStream xs(os);
			setMathFlavor(runparams);
			xhtmlParagraphs(text(), *this, xs, runparams);
		} else if (runparams.flavor == OutputParams::TEXT) {
			bool dummy = false;
			// FIXME Handles only one paragraph, unlike the others.
			// Probably should have some routine with a signature like them.
			writePlaintextParagraph(*this,
				text().paragraphs()[par_begin], os, runparams, dummy);
		} else if (params().isDocBook()) {
			docbookParagraphs(text(), *this, os, runparams);
		} else {
			// If we are previewing a paragraph, even if this is the
			// child of some other buffer, let's cut the link here,
			// so that no concurring settings from the master
			// (e.g. branch state) interfere (see #8101).
			if (!master)
				d->ignore_parent = true;
			// We need to validate the Buffer params' features here
			// in order to know if we should output polyglossia
			// macros (instead of babel macros)
			LaTeXFeatures features(*this, params(), runparams);
			params().validate(features);
			runparams.use_polyglossia = features.usePolyglossia();
			texrow.reset(new TexRow());
			texrow->newline();
			texrow->newline();
			// latex or literate
			otexstream ots(os, *texrow);

			// the real stuff
			latexParagraphs(*this, text(), ots, runparams);
			texrow->finalize();

			// Restore the parenthood
			if (!master)
				d->ignore_parent = false;
		}
	} else {
		os << "% ";
		if (output == FullSource)
			os << _("Preview source code");
		else if (output == OnlyPreamble)
			os << _("Preview preamble");
		else if (output == OnlyBody)
			os << _("Preview body");
		os << "\n\n";
		if (runparams.flavor == OutputParams::LYX) {
			ostringstream ods;
			if (output == FullSource)
				write(ods);
			else if (output == OnlyPreamble)
				params().writeFile(ods, this);
			else if (output == OnlyBody)
				text().write(ods);
			os << from_utf8(ods.str());
		} else if (runparams.flavor == OutputParams::HTML) {
			writeLyXHTMLSource(os, runparams, output);
		} else if (runparams.flavor == OutputParams::TEXT) {
			if (output == OnlyPreamble) {
				os << "% "<< _("Plain text does not have a preamble.");
			} else
				writePlaintextFile(*this, os, runparams);
		} else if (params().isDocBook()) {
				writeDocBookSource(os, absFileName(), runparams, output);
		} else {
			// latex or literate
			texrow.reset(new TexRow());
			texrow->newline();
			texrow->newline();
			otexstream ots(os, *texrow);
			if (master)
				runparams.is_child = true;
			writeLaTeXSource(ots, string(), runparams, output);
			texrow->finalize();
		}
	}
	return texrow;
}


ErrorList & Buffer::errorList(string const & type) const
{
	return d->errorLists[type];
}


void Buffer::updateTocItem(std::string const & type,
	DocIterator const & dit) const
{
	if (d->gui_)
		d->gui_->updateTocItem(type, dit);
}


void Buffer::structureChanged() const
{
	if (d->gui_)
		d->gui_->structureChanged();
}


void Buffer::errors(string const & err, bool from_master) const
{
	if (d->gui_)
		d->gui_->errors(err, from_master);
}


void Buffer::message(docstring const & msg) const
{
	if (d->gui_)
		d->gui_->message(msg);
}


void Buffer::setBusy(bool on) const
{
	if (d->gui_)
		d->gui_->setBusy(on);
}


void Buffer::updateTitles() const
{
	if (d->wa_)
		d->wa_->updateTitles();
}


void Buffer::resetAutosaveTimers() const
{
	if (d->gui_)
		d->gui_->resetAutosaveTimers();
}


bool Buffer::hasGuiDelegate() const
{
	return d->gui_;
}


void Buffer::setGuiDelegate(frontend::GuiBufferDelegate * gui)
{
	d->gui_ = gui;
}



namespace {

class AutoSaveBuffer : public ForkedProcess {
public:
	///
	AutoSaveBuffer(Buffer const & buffer, FileName const & fname)
		: buffer_(buffer), fname_(fname) {}
	///
	virtual shared_ptr<ForkedProcess> clone() const
	{
		return shared_ptr<ForkedProcess>(new AutoSaveBuffer(*this));
	}
	///
	int start()
	{
		command_ = to_utf8(bformat(_("Auto-saving %1$s"),
						 from_utf8(fname_.absFileName())));
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
#if defined(__APPLE__)
	/* FIXME fork() is not usable for autosave on Mac OS X 10.6 (snow leopard)
	 *   We should use something else like threads.
	 *
	 * Since I do not know how to determine at run time what is the OS X
	 * version, I just disable forking altogether for now (JMarc)
	 */
	pid_t const pid = -1;
#else
	// tmp_ret will be located (usually) in /tmp
	// will that be a problem?
	// Note that this calls ForkedCalls::fork(), so it's
	// ok cross-platform.
	pid_t const pid = fork();
	// If you want to debug the autosave
	// you should set pid to -1, and comment out the fork.
	if (pid != 0 && pid != -1)
		return pid;
#endif

	// pid = -1 signifies that lyx was unable
	// to fork. But we will do the save
	// anyway.
	bool failed = false;
	TempFile tempfile("lyxautoXXXXXX.lyx");
	tempfile.setAutoRemove(false);
	FileName const tmp_ret = tempfile.name();
	if (!tmp_ret.empty()) {
		if (!buffer_.writeFile(tmp_ret))
			failed = true;
		else if (!tmp_ret.moveTo(fname_))
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


FileName Buffer::getEmergencyFileName() const
{
	return FileName(d->filename.absFileName() + ".emergency");
}


FileName Buffer::getAutosaveFileName() const
{
	// if the document is unnamed try to save in the backup dir, else
	// in the default document path, and as a last try in the filePath,
	// which will most often be the temporary directory
	string fpath;
	if (isUnnamed())
		fpath = lyxrc.backupdir_path.empty() ? lyxrc.document_path
			: lyxrc.backupdir_path;
	if (!isUnnamed() || fpath.empty() || !FileName(fpath).exists())
		fpath = filePath();

	string const fname = "#" + d->filename.onlyFileName() + "#";

	return makeAbsPath(fname, fpath);
}


void Buffer::removeAutosaveFile() const
{
	FileName const f = getAutosaveFileName();
	if (f.exists())
		f.removeFile();
}


void Buffer::moveAutosaveFile(support::FileName const & oldauto) const
{
	FileName const newauto = getAutosaveFileName();
	oldauto.refresh();
	if (newauto != oldauto && oldauto.exists())
		if (!oldauto.moveTo(newauto))
			LYXERR0("Unable to move autosave file `" << oldauto << "'!");
}


bool Buffer::autoSave() const
{
	Buffer const * buf = d->cloned_buffer_ ? d->cloned_buffer_ : this;
	if (buf->d->bak_clean || isReadonly())
		return true;

	message(_("Autosaving current document..."));
	buf->d->bak_clean = true;

	FileName const fname = getAutosaveFileName();
	LASSERT(d->cloned_buffer_, return false);

	// If this buffer is cloned, we assume that
	// we are running in a separate thread already.
	TempFile tempfile("lyxautoXXXXXX.lyx");
	tempfile.setAutoRemove(false);
	FileName const tmp_ret = tempfile.name();
	if (!tmp_ret.empty()) {
		writeFile(tmp_ret);
		// assume successful write of tmp_ret
		if (tmp_ret.moveTo(fname))
			return true;
	}
	// failed to write/rename tmp_ret so try writing direct
	return writeFile(fname);
}


void Buffer::setExportStatus(bool e) const
{
	d->doing_export = e;
	ListOfBuffers clist = getDescendents();
	ListOfBuffers::const_iterator cit = clist.begin();
	ListOfBuffers::const_iterator const cen = clist.end();
	for (; cit != cen; ++cit)
		(*cit)->d->doing_export = e;
}


bool Buffer::isExporting() const
{
	return d->doing_export;
}


Buffer::ExportStatus Buffer::doExport(string const & target, bool put_in_tempdir)
	const
{
	string result_file;
	return doExport(target, put_in_tempdir, result_file);
}

Buffer::ExportStatus Buffer::doExport(string const & target, bool put_in_tempdir,
	string & result_file) const
{
	bool const update_unincluded =
			params().maintain_unincluded_children
			&& !params().getIncludedChildren().empty();

	// (1) export with all included children (omit \includeonly)
	if (update_unincluded) {
		ExportStatus const status =
			doExport(target, put_in_tempdir, true, result_file);
		if (status != ExportSuccess)
			return status;
	}
	// (2) export with included children only
	return doExport(target, put_in_tempdir, false, result_file);
}


void Buffer::setMathFlavor(OutputParams & op) const
{
	switch (params().html_math_output) {
	case BufferParams::MathML:
		op.math_flavor = OutputParams::MathAsMathML;
		break;
	case BufferParams::HTML:
		op.math_flavor = OutputParams::MathAsHTML;
		break;
	case BufferParams::Images:
		op.math_flavor = OutputParams::MathAsImages;
		break;
	case BufferParams::LaTeX:
		op.math_flavor = OutputParams::MathAsLaTeX;
		break;
	}
}


Buffer::ExportStatus Buffer::doExport(string const & target, bool put_in_tempdir,
	bool includeall, string & result_file) const
{
	LYXERR(Debug::FILES, "target=" << target);
	OutputParams runparams(&params().encoding());
	string format = target;
	string dest_filename;
	size_t pos = target.find(' ');
	if (pos != string::npos) {
		dest_filename = target.substr(pos + 1, target.length() - pos - 1);
		format = target.substr(0, pos);
		runparams.export_folder = FileName(dest_filename).onlyPath().realPath();
		FileName(dest_filename).onlyPath().createPath();
		LYXERR(Debug::FILES, "format=" << format << ", dest_filename=" << dest_filename << ", export_folder=" << runparams.export_folder);
	}
	MarkAsExporting exporting(this);
	string backend_format;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = lyxrc.plaintext_linelen;
	runparams.includeall = includeall;
	vector<string> backs = params().backends();
	Converters converters = theConverters();
	bool need_nice_file = false;
	if (find(backs.begin(), backs.end(), format) == backs.end()) {
		// Get shortest path to format
		converters.buildGraph();
		Graph::EdgePath path;
		for (vector<string>::const_iterator it = backs.begin();
		     it != backs.end(); ++it) {
			Graph::EdgePath p = converters.getPath(*it, format);
			if (!p.empty() && (path.empty() || p.size() < path.size())) {
				backend_format = *it;
				path = p;
			}
		}
		if (path.empty()) {
			if (!put_in_tempdir) {
				// Only show this alert if this is an export to a non-temporary
				// file (not for previewing).
				Alert::error(_("Couldn't export file"), bformat(
					_("No information for exporting the format %1$s."),
					formats.prettyName(format)));
			}
			return ExportNoPathToFormat;
		}
		runparams.flavor = converters.getFlavor(path, this);
		Graph::EdgePath::const_iterator it = path.begin();
		Graph::EdgePath::const_iterator en = path.end();
		for (; it != en; ++it)
			if (theConverters().get(*it).nice()) {
				need_nice_file = true;
				break;
			}

	} else {
		backend_format = format;
		LYXERR(Debug::FILES, "backend_format=" << backend_format);
		// FIXME: Don't hardcode format names here, but use a flag
		if (backend_format == "pdflatex")
			runparams.flavor = OutputParams::PDFLATEX;
		else if (backend_format == "luatex")
			runparams.flavor = OutputParams::LUATEX;
		else if (backend_format == "dviluatex")
			runparams.flavor = OutputParams::DVILUATEX;
		else if (backend_format == "xetex")
			runparams.flavor = OutputParams::XETEX;
	}

	string filename = latexName(false);
	filename = addName(temppath(), filename);
	filename = changeExtension(filename,
				   formats.extension(backend_format));
	LYXERR(Debug::FILES, "filename=" << filename);

	// Plain text backend
	if (backend_format == "text") {
		runparams.flavor = OutputParams::TEXT;
		writePlaintextFile(*this, FileName(filename), runparams);
	}
	// HTML backend
	else if (backend_format == "xhtml") {
		runparams.flavor = OutputParams::HTML;
		setMathFlavor(runparams);
		makeLyXHTMLFile(FileName(filename), runparams);
	} else if (backend_format == "lyx")
		writeFile(FileName(filename));
	// Docbook backend
	else if (params().isDocBook()) {
		runparams.nice = !put_in_tempdir;
		makeDocBookFile(FileName(filename), runparams);
	}
	// LaTeX backend
	else if (backend_format == format || need_nice_file) {
		runparams.nice = true;
		bool const success = makeLaTeXFile(FileName(filename), string(), runparams);
		if (d->cloned_buffer_)
			d->cloned_buffer_->d->errorLists["Export"] = d->errorLists["Export"];
		if (!success)
			return ExportError;
	} else if (!lyxrc.tex_allows_spaces
		   && contains(filePath(), ' ')) {
		Alert::error(_("File name error"),
			   _("The directory path to the document cannot contain spaces."));
		return ExportTexPathHasSpaces;
	} else {
		runparams.nice = false;
		bool const success = makeLaTeXFile(
			FileName(filename), filePath(), runparams);
		if (d->cloned_buffer_)
			d->cloned_buffer_->d->errorLists["Export"] = d->errorLists["Export"];
		if (!success)
			return ExportError;
	}

	string const error_type = (format == "program")
		? "Build" : params().bufferFormat();
	ErrorList & error_list = d->errorLists[error_type];
	string const ext = formats.extension(format);
	FileName const tmp_result_file(changeExtension(filename, ext));
	bool const success = converters.convert(this, FileName(filename),
		tmp_result_file, FileName(absFileName()), backend_format, format,
		error_list);

	// Emit the signal to show the error list or copy it back to the
	// cloned Buffer so that it can be emitted afterwards.
	if (format != backend_format) {
		if (runparams.silent)
			error_list.clear();
		else if (d->cloned_buffer_)
			d->cloned_buffer_->d->errorLists[error_type] =
				d->errorLists[error_type];
		else
			errors(error_type);
		// also to the children, in case of master-buffer-view
		ListOfBuffers clist = getDescendents();
		ListOfBuffers::const_iterator cit = clist.begin();
		ListOfBuffers::const_iterator const cen = clist.end();
		for (; cit != cen; ++cit) {
			if (runparams.silent)
				(*cit)->d->errorLists[error_type].clear();
			else if (d->cloned_buffer_) {
				// Enable reverse search by copying back the
				// texrow object to the cloned buffer.
				// FIXME: this is not thread safe.
				(*cit)->d->cloned_buffer_->d->texrow = (*cit)->d->texrow;
				(*cit)->d->cloned_buffer_->d->errorLists[error_type] =
					(*cit)->d->errorLists[error_type];
			} else
				(*cit)->errors(error_type, true);
		}
	}

	if (d->cloned_buffer_) {
		// Enable reverse dvi or pdf to work by copying back the texrow
		// object to the cloned buffer.
		// FIXME: There is a possibility of concurrent access to texrow
		// here from the main GUI thread that should be securized.
		d->cloned_buffer_->d->texrow = d->texrow;
		string const error_type = params().bufferFormat();
		d->cloned_buffer_->d->errorLists[error_type] = d->errorLists[error_type];
	}


	if (put_in_tempdir) {
		result_file = tmp_result_file.absFileName();
		return success ? ExportSuccess : ExportConverterError;
	}

	if (dest_filename.empty())
		result_file = changeExtension(d->exportFileName().absFileName(), ext);
	else
		result_file = dest_filename;
	// We need to copy referenced files (e. g. included graphics
	// if format == "dvi") to the result dir.
	vector<ExportedFile> const files =
		runparams.exportdata->externalFiles(format);
	string const dest = runparams.export_folder.empty() ?
		onlyPath(result_file) : runparams.export_folder;
	bool use_force = use_gui ? lyxrc.export_overwrite == ALL_FILES
				 : force_overwrite == ALL_FILES;
	CopyStatus status = use_force ? FORCE : SUCCESS;

	vector<ExportedFile>::const_iterator it = files.begin();
	vector<ExportedFile>::const_iterator const en = files.end();
	for (; it != en && status != CANCEL; ++it) {
		string const fmt = formats.getFormatFromFile(it->sourceName);
		string fixedName = it->exportName;
		if (!runparams.export_folder.empty()) {
			// Relative pathnames starting with ../ will be sanitized
			// if exporting to a different folder
			while (fixedName.substr(0, 3) == "../")
				fixedName = fixedName.substr(3, fixedName.length() - 3);
		}
		FileName fixedFileName = makeAbsPath(fixedName, dest);
		fixedFileName.onlyPath().createPath();
		status = copyFile(fmt, it->sourceName,
			fixedFileName,
			it->exportName, status == FORCE,
			runparams.export_folder.empty());
	}

	if (status == CANCEL) {
		message(_("Document export cancelled."));
		return ExportCancel;
	}

	if (tmp_result_file.exists()) {
		// Finally copy the main file
		use_force = use_gui ? lyxrc.export_overwrite != NO_FILES
				    : force_overwrite != NO_FILES;
		if (status == SUCCESS && use_force)
			status = FORCE;
		status = copyFile(format, tmp_result_file,
			FileName(result_file), result_file,
			status == FORCE);
		if (status == CANCEL) {
			message(_("Document export cancelled."));
			return ExportCancel;
		} else {
			message(bformat(_("Document exported as %1$s "
				"to file `%2$s'"),
				formats.prettyName(format),
				makeDisplayPath(result_file)));
		}
	} else {
		// This must be a dummy converter like fax (bug 1888)
		message(bformat(_("Document exported as %1$s"),
			formats.prettyName(format)));
	}

	return success ? ExportSuccess : ExportConverterError;
}


Buffer::ExportStatus Buffer::preview(string const & format) const
{
	bool const update_unincluded =
			params().maintain_unincluded_children
			&& !params().getIncludedChildren().empty();
	return preview(format, update_unincluded);
}


Buffer::ExportStatus Buffer::preview(string const & format, bool includeall) const
{
	MarkAsExporting exporting(this);
	string result_file;
	// (1) export with all included children (omit \includeonly)
	if (includeall) {
		ExportStatus const status = doExport(format, true, true, result_file);
		if (status != ExportSuccess)
			return status;
	}
	// (2) export with included children only
	ExportStatus const status = doExport(format, true, false, result_file);
	FileName const previewFile(result_file);

	Impl * theimpl = isClone() ? d->cloned_buffer_->d : d;
	theimpl->preview_file_ = previewFile;
	theimpl->preview_format_ = format;
	theimpl->preview_error_ = (status != ExportSuccess);

	if (status != ExportSuccess)
		return status;

	if (previewFile.exists())
		return formats.view(*this, previewFile, format) ?
			PreviewSuccess : PreviewError;

	// Successful export but no output file?
	// Probably a bug in error detection.
	LATTEST(status != ExportSuccess);
	return status;
}


Buffer::ReadStatus Buffer::extractFromVC()
{
	bool const found = LyXVC::file_not_found_hook(d->filename);
	if (!found)
		return ReadFileNotFound;
	if (!d->filename.isReadableFile())
		return ReadVCError;
	return ReadSuccess;
}


Buffer::ReadStatus Buffer::loadEmergency()
{
	FileName const emergencyFile = getEmergencyFileName();
	if (!emergencyFile.exists()
		  || emergencyFile.lastModified() <= d->filename.lastModified())
		return ReadFileNotFound;

	docstring const file = makeDisplayPath(d->filename.absFileName(), 20);
	docstring const text = bformat(_("An emergency save of the document "
		"%1$s exists.\n\nRecover emergency save?"), file);

	int const load_emerg = Alert::prompt(_("Load emergency save?"), text,
		0, 2, _("&Recover"), _("&Load Original"), _("&Cancel"));

	switch (load_emerg)
	{
	case 0: {
		docstring str;
		ReadStatus const ret_llf = loadThisLyXFile(emergencyFile);
		bool const success = (ret_llf == ReadSuccess);
		if (success) {
			if (isReadonly()) {
				Alert::warning(_("File is read-only"),
					bformat(_("An emergency file is successfully loaded, "
					"but the original file %1$s is marked read-only. "
					"Please make sure to save the document as a different "
					"file."), from_utf8(d->filename.absFileName())));
			}
			markDirty();
			lyxvc().file_found_hook(d->filename);
			str = _("Document was successfully recovered.");
		} else
			str = _("Document was NOT successfully recovered.");
		str += "\n\n" + bformat(_("Remove emergency file now?\n(%1$s)"),
			makeDisplayPath(emergencyFile.absFileName()));

		int const del_emerg =
			Alert::prompt(_("Delete emergency file?"), str, 1, 1,
				_("&Remove"), _("&Keep"));
		if (del_emerg == 0) {
			emergencyFile.removeFile();
			if (success)
				Alert::warning(_("Emergency file deleted"),
					_("Do not forget to save your file now!"), true);
			}
		return success ? ReadSuccess : ReadEmergencyFailure;
	}
	case 1: {
		int const del_emerg =
			Alert::prompt(_("Delete emergency file?"),
				_("Remove emergency file now?"), 1, 1,
				_("&Remove"), _("&Keep"));
		if (del_emerg == 0)
			emergencyFile.removeFile();
		return ReadOriginal;
	}

	default:
		break;
	}
	return ReadCancel;
}


Buffer::ReadStatus Buffer::loadAutosave()
{
	// Now check if autosave file is newer.
	FileName const autosaveFile = getAutosaveFileName();
	if (!autosaveFile.exists()
		  || autosaveFile.lastModified() <= d->filename.lastModified())
		return ReadFileNotFound;

	docstring const file = makeDisplayPath(d->filename.absFileName(), 20);
	docstring const text = bformat(_("The backup of the document %1$s "
		"is newer.\n\nLoad the backup instead?"), file);
	int const ret = Alert::prompt(_("Load backup?"), text, 0, 2,
		_("&Load backup"), _("Load &original"),	_("&Cancel"));

	switch (ret)
	{
	case 0: {
		ReadStatus const ret_llf = loadThisLyXFile(autosaveFile);
		// the file is not saved if we load the autosave file.
		if (ret_llf == ReadSuccess) {
			if (isReadonly()) {
				Alert::warning(_("File is read-only"),
					bformat(_("A backup file is successfully loaded, "
					"but the original file %1$s is marked read-only. "
					"Please make sure to save the document as a "
					"different file."),
					from_utf8(d->filename.absFileName())));
			}
			markDirty();
			lyxvc().file_found_hook(d->filename);
			return ReadSuccess;
		}
		return ReadAutosaveFailure;
	}
	case 1:
		// Here we delete the autosave
		autosaveFile.removeFile();
		return ReadOriginal;
	default:
		break;
	}
	return ReadCancel;
}


Buffer::ReadStatus Buffer::loadLyXFile()
{
	if (!d->filename.isReadableFile()) {
		ReadStatus const ret_rvc = extractFromVC();
		if (ret_rvc != ReadSuccess)
			return ret_rvc;
	}

	ReadStatus const ret_re = loadEmergency();
	if (ret_re == ReadSuccess || ret_re == ReadCancel)
		return ret_re;

	ReadStatus const ret_ra = loadAutosave();
	if (ret_ra == ReadSuccess || ret_ra == ReadCancel)
		return ret_ra;

	return loadThisLyXFile(d->filename);
}


Buffer::ReadStatus Buffer::loadThisLyXFile(FileName const & fn)
{
	return readFile(fn);
}


void Buffer::bufferErrors(TeXErrors const & terr, ErrorList & errorList) const
{
	TeXErrors::Errors::const_iterator it = terr.begin();
	TeXErrors::Errors::const_iterator end = terr.end();
	ListOfBuffers clist = getDescendents();
	ListOfBuffers::const_iterator cen = clist.end();

	for (; it != end; ++it) {
		int id_start = -1;
		int pos_start = -1;
		int errorRow = it->error_in_line;
		Buffer const * buf = 0;
		Impl const * p = d;
		if (it->child_name.empty())
		    p->texrow.getIdFromRow(errorRow, id_start, pos_start);
		else {
			// The error occurred in a child
			ListOfBuffers::const_iterator cit = clist.begin();
			for (; cit != cen; ++cit) {
				string const child_name =
					DocFileName(changeExtension(
						(*cit)->absFileName(), "tex")).
							mangledFileName();
				if (it->child_name != child_name)
					continue;
				(*cit)->d->texrow.getIdFromRow(errorRow,
							id_start, pos_start);
				if (id_start != -1) {
					buf = d->cloned_buffer_
						? (*cit)->d->cloned_buffer_->d->owner_
						: (*cit)->d->owner_;
					p = (*cit)->d;
					break;
				}
			}
		}
		int id_end = -1;
		int pos_end = -1;
		bool found;
		do {
			++errorRow;
			found = p->texrow.getIdFromRow(errorRow, id_end, pos_end);
		} while (found && id_start == id_end && pos_start == pos_end);

		if (id_start != id_end) {
			// Next registered position is outside the inset where
			// the error occurred, so signal end-of-paragraph
			pos_end = 0;
		}

		errorList.push_back(ErrorItem(it->error_desc,
			it->error_text, id_start, pos_start, pos_end, buf));
	}
}


void Buffer::setBuffersForInsets() const
{
	inset().setBuffer(const_cast<Buffer &>(*this));
}


void Buffer::updateBuffer(UpdateScope scope, UpdateType utype) const
{
	LBUFERR(!text().paragraphs().empty());

	// Use the master text class also for child documents
	Buffer const * const master = masterBuffer();
	DocumentClass const & textclass = master->params().documentClass();

	// do this only if we are the top-level Buffer
	if (master == this) {
		textclass.counters().reset(from_ascii("bibitem"));
		reloadBibInfoCache();
	}

	// keep the buffers to be children in this set. If the call from the
	// master comes back we can see which of them were actually seen (i.e.
	// via an InsetInclude). The remaining ones in the set need still be updated.
	static std::set<Buffer const *> bufToUpdate;
	if (scope == UpdateMaster) {
		// If this is a child document start with the master
		if (master != this) {
			bufToUpdate.insert(this);
			master->updateBuffer(UpdateMaster, utype);
			// If the master buffer has no gui associated with it, then the TocModel is
			// not updated during the updateBuffer call and TocModel::toc_ is invalid
			// (bug 5699). The same happens if the master buffer is open in a different
			// window. This test catches both possibilities.
			// See: http://marc.info/?l=lyx-devel&m=138590578911716&w=2
			// There remains a problem here: If there is another child open in yet a third
			// window, that TOC is not updated. So some more general solution is needed at
			// some point.
			if (master->d->gui_ != d->gui_)
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
	setChangesPresent(false);

	Buffer & cbuf = const_cast<Buffer &>(*this);

	// do the real work
	ParIterator parit = cbuf.par_iterator_begin();
	updateBuffer(parit, utype);

	if (master != this)
		// TocBackend update will be done later.
		return;

	d->bibinfo_cache_valid_ = true;
	d->cite_labels_valid_ = true;
	/// FIXME: Perf
	cbuf.tocBackend().update(true, utype);
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
	LASSERT(par.layout().labeltype == LABEL_ENUMERATE, return false);
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
void Buffer::Impl::setLabel(ParIterator & it, UpdateType utype) const
{
	BufferParams const & bp = owner_->masterBuffer()->params();
	DocumentClass const & textclass = bp.documentClass();
	Paragraph & par = it.paragraph();
	Layout const & layout = par.layout();
	Counters & counters = textclass.counters();

	if (par.params().startOfAppendix()) {
		// We want to reset the counter corresponding to toplevel sectioning
		Layout const & lay = textclass.getTOCLayout();
		docstring const cnt = lay.counter;
		if (!cnt.empty())
			counters.reset(cnt);
		counters.appendix(true);
	}
	par.params().appendix(counters.appendix());

	// Compute the item depth of the paragraph
	par.itemdepth = getItemDepth(it);

	if (layout.margintype == MARGIN_MANUAL) {
		if (par.params().labelWidthString().empty())
			par.params().labelWidthString(par.expandLabel(layout, bp));
	} else if (layout.latextype == LATEX_BIB_ENVIRONMENT) {
		// we do not need to do anything here, since the empty case is
		// handled during export.
	} else {
		par.params().labelWidthString(docstring());
	}

	switch(layout.labeltype) {
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
		docstring enumcounter = layout.counter.empty() ? from_ascii("enum") : layout.counter;

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
		counters.step(enumcounter, utype);

		string const & lang = par.getParLanguage(bp)->code();
		par.params().labelString(counters.theCounter(enumcounter, lang));

		break;
	}

	case LABEL_SENSITIVE: {
		string const & type = counters.current_float();
		docstring full_label;
		if (type.empty())
			full_label = owner_->B_("Senseless!!! ");
		else {
			docstring name = owner_->B_(textclass.floats().getType(type).name());
			if (counters.hasCounter(from_utf8(type))) {
				string const & lang = par.getParLanguage(bp)->code();
				counters.step(from_utf8(type), utype);
				full_label = bformat(from_ascii("%1$s %2$s:"),
						     name,
						     counters.theCounter(from_utf8(type), lang));
			} else
				full_label = bformat(from_ascii("%1$s #:"), name);
		}
		par.params().labelString(full_label);
		break;
	}

	case LABEL_NO_LABEL:
		par.params().labelString(docstring());
		break;

	case LABEL_ABOVE:
	case LABEL_CENTERED:
	case LABEL_STATIC: {
		docstring const & lcounter = layout.counter;
		if (!lcounter.empty()) {
			if (layout.toclevel <= bp.secnumdepth
						&& (layout.latextype != LATEX_ENVIRONMENT
					|| it.text()->isFirstInSequence(it.pit()))) {
				if (counters.hasCounter(lcounter))
					counters.step(lcounter, utype);
				par.params().labelString(par.expandLabel(layout, bp));
			} else
				par.params().labelString(docstring());
		} else
			par.params().labelString(par.expandLabel(layout, bp));
		break;
	}

	case LABEL_MANUAL:
	case LABEL_BIBLIO:
		par.params().labelString(par.expandLabel(layout, bp));
	}
}


void Buffer::updateBuffer(ParIterator & parit, UpdateType utype) const
{
	// LASSERT: Is it safe to continue here, or should we just return?
	LASSERT(parit.pit() == 0, /**/);

	// Set the position of the text in the buffer to be able
	// to resolve macros in it.
	parit.text()->setMacrocontextPosition(parit);

	depth_type maxdepth = 0;
	pit_type const lastpit = parit.lastpit();
	for ( ; parit.pit() <= lastpit ; ++parit.pit()) {
		// reduce depth if necessary
		if (parit->params().depth() > maxdepth) {
			/** FIXME: this function is const, but
			 * nevertheless it modifies the buffer. To be
			 * cleaner, one should modify the buffer in
			 * another function, which is actually
			 * non-const. This would however be costly in
			 * terms of code duplication.
			 */
			const_cast<Buffer *>(this)->undo().recordUndo(CursorData(parit));
			parit->params().depth(maxdepth);
		}
		maxdepth = parit->getMaxDepthAfter();

		if (utype == OutputUpdate) {
			// track the active counters
			// we have to do this for the master buffer, since the local
			// buffer isn't tracking anything.
			masterBuffer()->params().documentClass().counters().
					setActiveLayout(parit->layout());
		}

		// set the counter for this paragraph
		d->setLabel(parit, utype);

		// update change-tracking flag 
		parit->addChangesToBuffer(*this);

		// now the insets
		InsetList::const_iterator iit = parit->insetList().begin();
		InsetList::const_iterator end = parit->insetList().end();
		for (; iit != end; ++iit) {
			parit.pos() = iit->pos;
			iit->inset->updateBuffer(parit, utype);
		}
	}
}


int Buffer::spellCheck(DocIterator & from, DocIterator & to,
	WordLangTuple & word_lang, docstring_list & suggestions) const
{
	int progress = 0;
	WordLangTuple wl;
	suggestions.clear();
	word_lang = WordLangTuple();
	bool const to_end = to.empty();
	DocIterator const end = to_end ? doc_iterator_end(this) : to;
	// OK, we start from here.
	for (; from != end; from.forwardPos()) {
		// This skips all insets with spell check disabled.
		while (!from.allowSpellCheck()) {
			from.pop_back();
			from.pos()++;
		}
		// If from is at the end of the document (which is possible
		// when "from" was changed above) LyX will crash later otherwise.
		if (from.atEnd() || (!to_end && from >= end))
			break;
		to = from;
		from.paragraph().spellCheck();
		SpellChecker::Result res = from.paragraph().spellCheck(from.pos(), to.pos(), wl, suggestions);
		if (SpellChecker::misspelled(res)) {
			word_lang = wl;
			break;
		}
		// Do not increase progress when from == to, otherwise the word
		// count will be wrong.
		if (from != to) {
			from = to;
			++progress;
		}
	}
	return progress;
}


void Buffer::Impl::updateStatistics(DocIterator & from, DocIterator & to, bool skipNoOutput)
{
	bool inword = false;
	word_count_ = 0;
	char_count_ = 0;
	blank_count_ = 0;

	for (DocIterator dit = from ; dit != to && !dit.atEnd(); ) {
		if (!dit.inTexted()) {
			dit.forwardPos();
			continue;
		}

		Paragraph const & par = dit.paragraph();
		pos_type const pos = dit.pos();

		// Copied and adapted from isWordSeparator() in Paragraph
		if (pos == dit.lastpos()) {
			inword = false;
		} else {
			Inset const * ins = par.getInset(pos);
			if (ins && skipNoOutput && !ins->producesOutput()) {
				// skip this inset
				++dit.top().pos();
				// stop if end of range was skipped
				if (!to.atEnd() && dit >= to)
					break;
				continue;
			} else if (!par.isDeleted(pos)) {
				if (par.isWordSeparator(pos))
					inword = false;
				else if (!inword) {
					++word_count_;
					inword = true;
				}
				if (ins && ins->isLetter())
					++char_count_;
				else if (ins && ins->isSpace())
					++blank_count_;
				else {
					char_type const c = par.getChar(pos);
					if (isPrintableNonspace(c))
						++char_count_;
					else if (isSpace(c))
						++blank_count_;
				}
			}
		}
		dit.forwardPos();
	}
}


void Buffer::updateStatistics(DocIterator & from, DocIterator & to, bool skipNoOutput) const
{
	d->updateStatistics(from, to, skipNoOutput);
}


int Buffer::wordCount() const
{
	return d->wordCount();
}


int Buffer::charCount(bool with_blanks) const
{
	return d->charCount(with_blanks);
}


Buffer::ReadStatus Buffer::reload()
{
	setBusy(true);
	// c.f. bug http://www.lyx.org/trac/ticket/6587
	removeAutosaveFile();
	// e.g., read-only status could have changed due to version control
	d->filename.refresh();
	docstring const disp_fn = makeDisplayPath(d->filename.absFileName());

	// clear parent. this will get reset if need be.
	d->setParent(0);
	ReadStatus const status = loadLyXFile();
	if (status == ReadSuccess) {
		updateBuffer();
		changed(true);
		updateTitles();
		markClean();
		message(bformat(_("Document %1$s reloaded."), disp_fn));
		d->undo_.clear();
	} else {
		message(bformat(_("Could not reload document %1$s."), disp_fn));
	}
	setBusy(false);
	removePreviews();
	updatePreviews();
	errors("Parse");
	return status;
}


bool Buffer::saveAs(FileName const & fn)
{
	FileName const old_name = fileName();
	FileName const old_auto = getAutosaveFileName();
	bool const old_unnamed = isUnnamed();
	bool success = true;
	d->old_position = filePath();

	setFileName(fn);
	markDirty();
	setUnnamed(false);

	if (save()) {
		// bring the autosave file with us, just in case.
		moveAutosaveFile(old_auto);
		// validate version control data and
		// correct buffer title
		lyxvc().file_found_hook(fileName());
		updateTitles();
		// the file has now been saved to the new location.
		// we need to check that the locations of child buffers
		// are still valid.
		checkChildBuffers();
		checkMasterBuffer();
	} else {
		// save failed
		// reset the old filename and unnamed state
		setFileName(old_name);
		setUnnamed(old_unnamed);
		success = false;
	}

	d->old_position.clear();
	return success;
}


void Buffer::checkChildBuffers()
{
	Impl::BufferPositionMap::iterator it = d->children_positions.begin();
	Impl::BufferPositionMap::iterator const en = d->children_positions.end();
	for (; it != en; ++it) {
		DocIterator dit = it->second;
		Buffer * cbuf = const_cast<Buffer *>(it->first);
		if (!cbuf || !theBufferList().isLoaded(cbuf))
			continue;
		Inset * inset = dit.nextInset();
		LASSERT(inset && inset->lyxCode() == INCLUDE_CODE, continue);
		InsetInclude * inset_inc = static_cast<InsetInclude *>(inset);
		docstring const & incfile = inset_inc->getParam("filename");
		string oldloc = cbuf->absFileName();
		string newloc = makeAbsPath(to_utf8(incfile),
				onlyPath(absFileName())).absFileName();
		if (oldloc == newloc)
			continue;
		// the location of the child file is incorrect.
		cbuf->setParent(0);
		inset_inc->setChildBuffer(0);
	}
	// invalidate cache of children
	d->children_positions.clear();
	d->position_to_children.clear();
}


// If a child has been saved under a different name/path, it might have been
// orphaned. Therefore the master needs to be reset (bug 8161).
void Buffer::checkMasterBuffer()
{
	Buffer const * const master = masterBuffer();
	if (master == this)
		return;

	// necessary to re-register the child (bug 5873)
	// FIXME: clean up updateMacros (here, only
	// child registering is needed).
	master->updateMacros();
	// (re)set master as master buffer, but only
	// if we are a real child
	if (master->isChild(this))
		setParent(master);
	else
		setParent(0);
}


string Buffer::includedFilePath(string const & name, string const & ext) const
{
	if (d->old_position.empty() ||
	    equivalent(FileName(d->old_position), FileName(filePath())))
		return name;

	bool isabsolute = FileName::isAbsolute(name);
	// both old_position and filePath() end with a path separator
	string absname = isabsolute ? name : d->old_position + name;

	// if old_position is set to origin, we need to do the equivalent of
	// getReferencedFileName() (see readDocument())
	if (!isabsolute && d->old_position == params().origin) {
		FileName const test(addExtension(filePath() + name, ext));
		if (test.exists())
			absname = filePath() + name;
	}

	if (!FileName(addExtension(absname, ext)).exists())
		return name;

	if (isabsolute)
		return to_utf8(makeRelPath(from_utf8(name), from_utf8(filePath())));

	return to_utf8(makeRelPath(from_utf8(FileName(absname).realPath()),
	                           from_utf8(filePath())));
}


void Buffer::setChangesPresent(bool b) const
{
	d->tracked_changes_present_ = b;
}


bool Buffer::areChangesPresent() const
{
	return d->tracked_changes_present_;
}


void Buffer::updateChangesPresent() const
{
	LYXERR(Debug::CHANGES, "Buffer::updateChangesPresent");
	setChangesPresent(false);
	ParConstIterator it = par_iterator_begin();
	ParConstIterator const end = par_iterator_end();
	for (; !areChangesPresent() && it != end; ++it)
		it->addChangesToBuffer(*this);
}



} // namespace lyx
