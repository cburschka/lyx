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
#include "BiblioInfo.h"
#include "BranchList.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "Bullet.h"
#include "Chktex.h"
#include "ColorSet.h"
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
#include "output.h"
#include "output_latex.h"
#include "output_docbook.h"
#include "output_plaintext.h"
#include "output_xhtml.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "PDFOptions.h"
#include "Session.h"
#include "SpellChecker.h"
#include "xml.h"
#include "texstream.h"
#include "TexRow.h"
#include "Text.h"
#include "TextClass.h"
#include "TocBackend.h"
#include "Undo.h"
#include "VCBackend.h"
#include "version.h"
#include "WordLangTuple.h"

#include "insets/InsetBranch.h"
#include "insets/InsetInclude.h"
#include "insets/InsetText.h"

#include "mathed/InsetMathHull.h"
#include "mathed/MacroTable.h"
#include "mathed/InsetMathMacroTemplate.h"
#include "mathed/MathSupport.h"

#include "graphics/PreviewLoader.h"

#include "frontends/Application.h"
#include "frontends/alert.h"
#include "frontends/Delegates.h"
#include "frontends/WorkAreaManager.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/ExceptionMessage.h"
#include "support/FileMonitor.h"
#include "support/FileName.h"
#include "support/FileNameList.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/gzstream.h"
#include "support/lstrings.h"
#include "support/mutex.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/PathChanger.h"
#include "support/Systemcall.h"
#include "support/TempFile.h"
#include "support/textutils.h"
#include "support/types.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
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

// Information about labels and their associated refs
struct LabelInfo {
	/// label string
	docstring label;
	/// label inset
	InsetLabel const * inset;
	/// associated references cache
	Buffer::References references;
	/// whether this label is active (i.e., not deleted)
	bool active;
};

typedef vector<LabelInfo> LabelCache;

typedef map<docstring, Buffer::References> RefCache;

// A storehouse for the cloned buffers.
typedef list<CloneList_ptr> CloneStore;
CloneStore cloned_buffers;

} // namespace



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
	FileName exportFileName() const;

	Buffer * owner_;

	BufferParams params;
	LyXVC lyxvc;
	FileName temppath;
	mutable TexRow texrow;

	/// need to regenerate .tex?
	DepClean dep_clean;

	/// name of the file the buffer is associated with.
	FileName filename;

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

	/// positions of child buffers in the buffer
	typedef map<Buffer const * const, DocIterator> BufferPositionMap;
	struct ScopeBuffer {
		ScopeBuffer() : buffer(nullptr) {}
		ScopeBuffer(DocIterator const & s, Buffer const * b)
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

	/// checksum used to test if the file has been externally modified.  Used to
	/// double check whether the file had been externally modified when saving.
	unsigned long checksum_;

	///
	frontend::WorkAreaManager * wa_;
	///
	frontend::GuiBufferDelegate * gui_;

	///
	Undo undo_;

	/// A cache for the bibfiles (including bibfiles of loaded child
	/// documents), needed for appropriate update of natbib labels.
	mutable docstring_list bibfiles_cache_;

	// FIXME The caching mechanism could be improved. At present, we have a
	// cache for each Buffer, that caches all the bibliography info for that
	// Buffer. A more efficient solution would be to have a global cache per
	// file, and then to construct the Buffer's bibinfo from that.
	/// A cache for bibliography info
	mutable BiblioInfo bibinfo_;
	/// Cache of timestamps of .bib files
	map<FileName, time_t> bibfile_status_;

	/// These two hold the file name and format, written to by
	/// Buffer::preview and read from by LFUN_BUFFER_VIEW_CACHE.
	FileName preview_file_;
	string preview_format_;

	/// Cache the references associated to a label and their positions
	/// in the buffer.
	mutable RefCache ref_cache_;
	/// Cache the label insets and their activity status.
	mutable LabelCache label_cache_;

	/// our Text that should be wrapped in an InsetText
	InsetText * inset;

	///
	PreviewLoader * preview_loader_;

	/// If non zero, this buffer is a clone of existing buffer \p cloned_buffer_
	/// This one is useful for preview detached in a thread.
	Buffer const * cloned_buffer_;
	///
	CloneList_ptr clone_list_;

	///
	std::list<Buffer const *> include_list_;
private:
	/// So we can force access via the accessors.
	mutable Buffer const * parent_buffer;

	FileMonitorPtr file_monitor_;

/// ints and bools are all listed last so as to avoid alignment issues
public:
	/// original format of loaded file
	int file_format;

	/// are we in the process of exporting this buffer?
	mutable bool doing_export;

	/// If there was an error when previewing, on the next preview we do
	/// a fresh compile (e.g. in case the user installed a package that
	/// was missing).
	bool require_fresh_start_;

	/// Indicates whether the bibinfo has changed since the last time
	/// we ran updateBuffer(), i.e., whether citation labels may need
	/// to be updated.
	mutable bool cite_labels_valid_;
	/// Do we have a bibliography environment?
	mutable bool have_bibitems_;

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

	/** Set to true only when the file is fully loaded.
	 *  Used to prevent the premature generation of previews
	 *  and by the citation inset.
	 */
	bool file_fully_loaded;

	/// if the file was originally loaded from an older format, do
	/// we need to back it up still?
	bool need_format_backup;

	/// Ignore the parent (e.g. when exporting a child standalone)?
	bool ignore_parent;

	/// This seem to change the way Buffer::getMacro() works
	mutable bool macro_lock;

	/// has been externally modified? Can be reset by the user.
	mutable bool externally_modified_;

	/// whether the bibinfo cache is valid
	mutable bool bibinfo_cache_valid_;

	///
	mutable bool need_update;

private:
	int word_count_;
	int char_count_;
	int blank_count_;

public:
	/// This is here to force the test to be done whenever parent_buffer
	/// is accessed.
	Buffer const * parent() const
	{
		// ignore_parent temporarily "orphans" a buffer
		// (e.g. if a child is compiled standalone)
		if (ignore_parent)
			return nullptr;
		// if parent_buffer is not loaded, then it has been unloaded,
		// which means that parent_buffer is an invalid pointer. So we
		// set it to null in that case.
		// however, the BufferList doesn't know about cloned buffers, so
		// they will always be regarded as unloaded. in that case, we hope
		// for the best.
		if (!cloned_buffer_ && !theBufferList().isLoaded(parent_buffer))
			parent_buffer = nullptr;
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
		if (!cloned_buffer_ && parent_buffer)
			parent_buffer->invalidateBibinfoCache();
	}

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

	// Make sure the file monitor monitors the good file.
	void refreshFileMonitor();

	/// Notify or clear of external modification
	void fileExternallyModified(bool exists);

	///Binding LaTeX lines with buffer positions.
	//Common routine for LaTeX and Reference errors listing.
        void traverseErrors(TeXErrors::Errors::const_iterator err,
		TeXErrors::Errors::const_iterator end,
		ErrorList & errorList) const;
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
	: owner_(owner), filename(file), toc_backend(owner), checksum_(0),
	  wa_(nullptr),  gui_(nullptr), undo_(*owner), inset(nullptr),
	  preview_loader_(nullptr), cloned_buffer_(cloned_buffer),
	  clone_list_(nullptr), parent_buffer(nullptr), file_format(LYX_FORMAT),
	  doing_export(false), require_fresh_start_(false), cite_labels_valid_(false),
	  have_bibitems_(false), lyx_clean(true), bak_clean(true), unnamed(false),
	  internal_buffer(false), read_only(readonly_), file_fully_loaded(false),
	  need_format_backup(false), ignore_parent(false), macro_lock(false),
	  externally_modified_(false), bibinfo_cache_valid_(false),
	  need_update(false), word_count_(0), char_count_(0), blank_count_(0)
{
	refreshFileMonitor();
	if (!cloned_buffer_) {
		temppath = createBufferTmpDir();
		lyxvc.setBuffer(owner_);
		Language const * inplang = theApp() ?
					languages.getFromCode(theApp()->inputLanguageCode())
				      : nullptr;
		if (inplang)
			params.language = inplang;
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
	bibfile_status_ = cloned_buffer_->d->bibfile_status_;
	cite_labels_valid_ = cloned_buffer_->d->cite_labels_valid_;
	have_bibitems_ = cloned_buffer_->d->have_bibitems_;
	unnamed = cloned_buffer_->d->unnamed;
	internal_buffer = cloned_buffer_->d->internal_buffer;
	layout_position = cloned_buffer_->d->layout_position;
	preview_file_ = cloned_buffer_->d->preview_file_;
	preview_format_ = cloned_buffer_->d->preview_format_;
	require_fresh_start_ = cloned_buffer_->d->require_fresh_start_;
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
	d->gui_ = nullptr;

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
		for (auto const & p : d->children_positions) {
			Buffer * child = const_cast<Buffer *>(p.first);
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
			// The clone list itself is empty, but it's still referenced in our list
			// of clones. So let's find it and remove it.
			CloneStore::iterator it =
				find(cloned_buffers.begin(), cloned_buffers.end(), d->clone_list_);
			if (it == cloned_buffers.end()) {
				// We will leak in this case, but it is safe to continue.
				LATTEST(false);
			} else
				cloned_buffers.erase(it);
		}
		// FIXME Do we really need to do this right before we delete d?
		// clear references to children in macro tables
		d->children_positions.clear();
		d->position_to_children.clear();
	} else {
		// loop over children
		for (auto const & p : d->children_positions) {
			Buffer * child = const_cast<Buffer *>(p.first);
			if (theBufferList().isLoaded(child)) {
				if (theBufferList().isOthersChild(this, child))
					child->setParent(nullptr);
				else
					theBufferList().release(child);
			}
		}

		if (!isClean()) {
			docstring msg = _("LyX attempted to close a document that had unsaved changes!\n");
			try {
				msg += emergencyWrite();
			} catch (...) {
				msg += "  " + _("Save failed! Document is lost.");
			}
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


Buffer * Buffer::cloneWithChildren() const
{
	BufferMap bufmap;
	cloned_buffers.emplace_back(new CloneList);
	CloneList_ptr clones = cloned_buffers.back();

	cloneWithChildren(bufmap, clones);

	// make sure we got cloned
	BufferMap::const_iterator bit = bufmap.find(this);
	LASSERT(bit != bufmap.end(), return nullptr);
	Buffer * cloned_buffer = bit->second;

	return cloned_buffer;
}


void Buffer::cloneWithChildren(BufferMap & bufmap, CloneList_ptr clones) const
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
	for (auto const & p : d->position_to_children) {
		DocIterator dit = p.first.clone(buffer_clone);
		dit.setBuffer(buffer_clone);
		Buffer * child = const_cast<Buffer *>(p.second.buffer);

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
}


Buffer * Buffer::cloneBufferOnly() const {
	cloned_buffers.emplace_back(new CloneList);
	CloneList_ptr clones = cloned_buffers.back();
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
	for (Author const & a : params().authors())
		mparams.authors().record(a);
	return mparams;
}


double Buffer::fontScalingFactor() const
{
	return isExporting() ? 75.0 * params().html_math_img_scale
		: 0.01 * lyxrc.dpi * lyxrc.currentZoom * lyxrc.preview_scale_factor * params().display_pixel_ratio;
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

	string const name = addExtension(filename.onlyFileNameWithoutExt()
			+ to_utf8(branch_suffix), filename.extension());
	FileName res(filename.onlyPath().absFileName() + "/" + name);

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
					theFormats().extension(params().bufferFormat()) + ".out"))));

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
	d->refreshFileMonitor();
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
	params().float_alignment.erase();
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
	params().font_roman_opts.erase();
	params().font_sans_opts.erase();
	params().font_typewriter_opts.erase();
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
	params().isnotefontcolor = false;
	params().boxbgcolor = RGBColor(0xFF, 0, 0);
	params().isboxbgcolor = false;
	params().html_latex_start.clear();
	params().html_latex_end.clear();
	params().html_math_img_scale = 1.0;
	params().output_sync_macro.erase();
	params().setLocalLayout(docstring(), false);
	params().setLocalLayout(docstring(), true);
	params().biblio_opts.erase();
	params().biblatex_bibstyle.erase();
	params().biblatex_citestyle.erase();
	params().multibib.erase();
	params().lineno_opts.clear();

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
			params().readToken(lex, token, d->filename);
		if (!result.empty()) {
			if (token == "\\textclass") {
				d->layout_position = result;
			} else {
				++unknown_tokens;
				docstring const s = bformat(_("Unknown token: "
									"%1$s %2$s\n"),
							 from_utf8(token),
							 lex.getDocString());
				errorList.push_back(ErrorItem(_("Document header error"), s));
			}
		}
	}
	if (begin_header_line) {
		docstring const s = _("\\begin_header is missing");
		errorList.push_back(ErrorItem(_("Document header error"), s));
	}

	params().shell_escape = theSession().shellescapeFiles().find(absFileName());

	params().makeDocumentClass(isClone(), isInternal());

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
		errorList.push_back(ErrorItem(_("Document header error"), s));
	}

	readHeader(lex);

	if (params().output_changes) {
		bool xcolorulem = LaTeXFeatures::isAvailable("ulem") &&
				  LaTeXFeatures::isAvailable("xcolor");

		if (!xcolorulem) {
			Alert::warning(_("Changes not shown in LaTeX output"),
				       _("Changes will not be highlighted in LaTeX output, "
					 "because xcolor and ulem are not installed.\n"
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
				// If the master has just been created, un-hide it (#11162)
				if (!master->fileName().exists())
					lyx::dispatch(FuncRequest(LFUN_BUFFER_SWITCH,
								  master->absFileName()));
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

	if (!parent())
		clearIncludeList();

	bool const res = text().read(lex, errorList, d->inset);
	d->old_position.clear();

	// inform parent buffer about local macros
	if (parent()) {
		Buffer const * pbuf = parent();
		for (auto const & m : usermacros)
			pbuf->usermacros.insert(m);
	}
	usermacros.clear();
	updateMacros();
	updateMacroInstances(InternalUpdate);
	return res;
}


bool Buffer::importString(string const & format, docstring const & contents, ErrorList & errorList)
{
	Format const * fmt = theFormats().getFormat(format);
	if (!fmt)
		return false;
	// It is important to use the correct extension here, since some
	// converters create a wrong output file otherwise (e.g. html2latex)
	FileName const name = tempFileName("Buffer_importStringXXXXXX." + fmt->extension());
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

	removeTempFile(name);
	return converted;
}


bool Buffer::importFile(string const & format, FileName const & name, ErrorList & errorList)
{
	if (!theConverters().isReachable(format, "lyx"))
		return false;

	FileName const lyx = tempFileName("Buffer_importFileXXXXXX.lyx");
	Converters::RetVal const retval =
		theConverters().convert(nullptr, name, lyx, name, format, "lyx", errorList);
	if (retval == Converters::SUCCESS) {
		bool const success = readFile(lyx) == ReadSuccess;
		removeTempFile(lyx);
		return success;
	}

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
	Lexer lex;
	if (!lex.setFile(fn)) {
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
	params().compressed = theFormats().isZippedFile(d->filename);
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


bool Buffer::freshStartRequired() const
{
	return d->require_fresh_start_;
}

void Buffer::requireFreshStart(bool const b) const
{
	d->require_fresh_start_ = b;
}


PreviewLoader * Buffer::loader() const
{
	if (!isExporting() && lyxrc.preview == LyXRC::PREVIEW_OFF)
		return nullptr;
	if (!d->preview_loader_)
		d->preview_loader_ = new PreviewLoader(*this);
	return d->preview_loader_;
}


void Buffer::removePreviews() const
{
	delete d->preview_loader_;
	d->preview_loader_ = nullptr;
}


void Buffer::updatePreviews() const
{
	PreviewLoader * ploader = loader();
	if (!ploader)
		return;

	InsetIterator it = begin(*d->inset);
	InsetIterator const itend = end(*d->inset);
	for (; it != itend; ++it)
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
		<< " -o " << quoteName(tmpfile.toSafeFilesystemEncoding())
		<< ' ' << quoteName(fn.toSafeFilesystemEncoding());
	string const command_str = command.str();

	LYXERR(Debug::INFO, "Running '" << command_str << '\'');

	cmd_ret const ret = runCommand(command_str);
	if (!ret.valid) {
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
	map<int, string> const file_formats = {
	  {544, "23"},
	  {508, "22"},
	  {474, "21"},
	  {413, "20"},
	  {345, "16"},
	  {276, "15"},
	  {245, "14"},
	  {221, "13"},
	  {220, "12"},
	  {218, "1163"},
	  {217, "116"},
	  {216, "115"},
	  {215, "11"},
	  {210, "010"},
	  {200, "006"}
	};
	FileName const & fn = fileName();
	string const fname = fn.onlyFileNameWithoutExt();
	string const fext  = fn.extension() + "~";
	string const fpath = lyxrc.backupdir_path.empty() ?
		fn.onlyPath().absFileName() :
		lyxrc.backupdir_path;
	string backup_suffix;
	// If file format is from a stable series use version instead of file format
	auto const it = file_formats.find(d->file_format);
	if (it != file_formats.end())
		backup_suffix = "-lyx" + it->second;
	else
		backup_suffix = "-lyxformat-" + convert<string>(d->file_format);
	string const backname = fname + backup_suffix;
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
	if (fileName().exists() && isChecksumModified()) {
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
	auto tempfile = lyx::make_unique<TempFile>(fileName().onlyPath(),
	                                      justname + "-XXXXXX.lyx");
	bool const symlink = fileName().isSymLink();
	if (!symlink)
		tempfile->setAutoRemove(false);

	FileName savefile(tempfile->name());
	LYXERR(Debug::FILES, "Saving to " << savefile.absFileName());
	if (!savefile.clonePermissions(fileName()))
		LYXERR0("Failed to clone the permission from " << fileName().absFileName() << " to " << savefile.absFileName());

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


docstring Buffer::emergencyWrite() const
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

	user_message += "  " + _("Save failed! Document is lost.");
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
	    << " created this file. For more info see https://www.lyx.org/\n"
	    << "\\lyxformat " << LYX_FORMAT << "\n"
	    << "\\begin_document\n";

	/// For each author, set 'used' to true if there is a change
	/// by this author in the document; otherwise set it to 'false'.
	for (Author const & a : params().authors())
		a.setUsed(false);

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


Buffer::ExportStatus Buffer::makeLaTeXFile(FileName const & fname,
			   string const & original_path,
			   OutputParams const & runparams_in,
			   OutputWhat output) const
{
	OutputParams runparams = runparams_in;

	string const encoding = runparams.encoding->iconvName();
	LYXERR(Debug::LATEX, "makeLaTeXFile encoding: " << encoding << ", fname=" << fname.realPath());

	ofdocstream ofs;
	try { ofs.reset(encoding); }
	catch (iconv_codecvt_facet_exception const & e) {
		lyxerr << "Caught iconv exception: " << e.what() << endl;
		Alert::error(_("Iconv software exception Detected"),
			bformat(_("Please verify that the `iconv' support software is"
					  " properly installed and supports the selected encoding"
					  " (%1$s), or change the encoding in"
					  " Document>Settings>Language."), from_ascii(encoding)));
		return ExportError;
	}
	if (!openFileWrite(ofs, fname))
		return ExportError;

	ErrorList & errorList = d->errorLists["Export"];
	errorList.clear();
	ExportStatus status = ExportSuccess;
	otexstream os(ofs);

	// make sure we are ready to export
	// this needs to be done before we validate
	// FIXME Do we need to do this all the time? I.e., in children
	// of a master we are exporting?
	updateBuffer();
	updateMacroInstances(OutputUpdate);

	ExportStatus retval;
	try {
		retval = writeLaTeXSource(os, original_path, runparams, output);
		if (retval == ExportKilled)
			return ExportKilled;
	}
	catch (EncodingException const & e) {
		docstring const failed(1, e.failed_char);
		ostringstream oss;
		oss << "0x" << hex << static_cast<uint32_t>(e.failed_char) << dec;
		if (getParFromID(e.par_id).paragraph().layout().pass_thru) {
			docstring msg = bformat(_("Uncodable character '%1$s'"
						  " (code point %2$s)"),
						  failed, from_utf8(oss.str()));
			errorList.push_back(ErrorItem(msg, _("Some characters of your document are not "
					"representable in specific verbatim contexts.\n"
					"Changing the document encoding to utf8 could help."),
						      {e.par_id, e.pos}, {e.par_id, e.pos + 1}));
		} else {
			docstring msg = bformat(_("Could not find LaTeX command for character '%1$s'"
						  " (code point %2$s)"),
						  failed, from_utf8(oss.str()));
			errorList.push_back(ErrorItem(msg, _("Some characters of your document are probably not "
					"representable in the chosen encoding.\n"
					"Changing the document encoding to utf8 could help."),
						      {e.par_id, e.pos}, {e.par_id, e.pos + 1}));
		}
		status = ExportError;
	}
	catch (iconv_codecvt_facet_exception const & e) {
		errorList.push_back(ErrorItem(_("iconv conversion failed"),
		                              _(e.what())));
		status = ExportError;
	}
	catch (exception const & e) {
		errorList.push_back(ErrorItem(_("conversion failed"),
		                              _(e.what())));
		lyxerr << e.what() << endl;
		status = ExportError;
	}
	catch (...) {
		lyxerr << "Caught some really weird exception..." << endl;
		lyx_exit(1);
	}

	d->texrow = move(os.texrow());

	ofs.close();
	if (ofs.fail()) {
		status = ExportError;
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
	}

	if (runparams_in.silent)
		errorList.clear();
	else
		errors("Export");
	return status;
}


Buffer::ExportStatus Buffer::writeLaTeXSource(otexstream & os,
			   string const & original_path,
			   OutputParams const & runparams_in,
			   OutputWhat output) const
{
	// The child documents, if any, shall be already loaded at this point.

	OutputParams runparams = runparams_in;

	// Some macros rely on font encoding
	runparams.main_fontenc = params().main_font_encoding();

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
	if (!runparams.is_child) {
		runparams.use_polyglossia = features.usePolyglossia();
		runparams.use_hyperref = features.isRequired("hyperref");
		runparams.use_CJK = features.mustProvide("CJK");
	}
	LYXERR(Debug::LATEX, "  Buffer validation done.");

	bool const output_preamble =
		output == FullSource || output == OnlyPreamble;
	bool const output_body =
		output == FullSource || output == OnlyBody;

	// The starting paragraph of the coming rows is the
	// first paragraph of the document. (Asger)
	if (output_preamble && runparams.nice) {
		os << "%% LyX " << lyx_version << " created this file.  "
			"For more info, see https://www.lyx.org/.\n"
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
				for (char_type n : inputpath) {
					if (!enc->encodable(n)) {
						docstring const glyph(1, n);
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
					bformat(
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
				string docdir = os::latex_path(original_path);
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
				if (contains(docdir, '~'))
					docdir = subst(docdir, "~", "\\string~");
				bool const nonascii = !isAscii(from_utf8(docdir));
				// LaTeX 2019/10/01 handles non-ascii path without detokenize
				bool const utfpathlatex = features.isAvailable("LaTeX-2019/10/01");
				bool const detokenize = !utfpathlatex && nonascii;
				bool const quote = contains(docdir, ' ');
				if (utfpathlatex && nonascii)
					os << "\\UseRawInputEncoding\n";
				os << "\\makeatletter\n"
				   << "\\def\\input@path{{";
				if (detokenize)
					os << "\\detokenize{";
				if (quote)
					os << "\"";
				os << docdir;
				if (quote)
					os << "\"";
				if (detokenize)
					os << "}";
				os << "}}\n"
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

		// Active characters
		runparams.active_chars = features.getActiveChars();

		// Biblatex bibliographies are loaded here
		if (params().useBiblatex()) {
			vector<pair<docstring, string>> const bibfiles =
				prepareBibFilePaths(runparams, getBibfiles(), true);
			for (pair<docstring, string> const & file: bibfiles) {
				os << "\\addbibresource";
				if (!file.second.empty())
					os << "[bibencoding=" << file.second << "]";
				os << "{" << file.first << "}\n";
			}
		}

		if (!runparams.dryrun && features.hasPolyglossiaExclusiveLanguages()
		    && !features.hasOnlyPolyglossiaLanguages()) {
			docstring blangs;
			docstring plangs;
			vector<string> bll = features.getBabelExclusiveLanguages();
			vector<string> pll = features.getPolyglossiaExclusiveLanguages();
			if (!bll.empty()) {
				docstring langs;
				for (string const & sit : bll) {
					if (!langs.empty())
						langs += ", ";
					langs += _(sit);
				}
				blangs = bll.size() > 1 ?
					    bformat(_("The languages %1$s are only supported by Babel."), langs)
					  : bformat(_("The language %1$s is only supported by Babel."), langs);
			}
			if (!pll.empty()) {
				docstring langs;
				for (string const & pit : pll) {
					if (!langs.empty())
						langs += ", ";
					langs += _(pit);
				}
				plangs = pll.size() > 1 ?
					    bformat(_("The languages %1$s are only supported by Polyglossia."), langs)
					  : bformat(_("The language %1$s is only supported by Polyglossia."), langs);
				if (!blangs.empty())
					plangs += "\n";
			}

			frontend::Alert::warning(
				_("Incompatible Languages!"),
				bformat(
				  _("You cannot use the following languages "
				    "together in one LaTeX document because "
				    "they require conflicting language packages:\n"
				    "%1$s%2$s"),
				  plangs, blangs));
		}

		// Japanese might be required only in some children of a document,
		// but once required, we must keep use_japanese true.
		runparams.use_japanese |= features.isRequired("japanese");

		if (!output_body) {
			// Restore the parenthood if needed
			if (!runparams.is_child)
				d->ignore_parent = false;
			return ExportSuccess;
		}

		// make the body.
		// mark the beginning of the body to separate it from InPreamble insets
		os.texrow().start(TexRow::beginDocument());
		os << "\\begin{document}\n";

		// mark the start of a new paragraph by simulating a newline,
		// so that os.afterParbreak() returns true at document start
		os.lastChar('\n');

		// output the parent macros
		for (auto const & mac : parentMacros) {
			int num_lines = mac->write(os.os(), true);
			os.texrow().newlines(num_lines);
		}

	} // output_preamble

	LYXERR(Debug::INFO, "preamble finished, now the body.");

	// the real stuff
	try {
		latexParagraphs(*this, text(), os, runparams);
	}
	catch (ConversionException const &) { return ExportKilled; }

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

	LYXERR(Debug::INFO, "Finished making LaTeX file.");
	LYXERR(Debug::INFO, "Row count was " << os.texrow().rows() - 1 << '.');
	return ExportSuccess;
}


Buffer::ExportStatus Buffer::makeDocBookFile(FileName const & fname,
			      OutputParams const & runparams,
			      OutputWhat output) const
{
	LYXERR(Debug::LATEX, "makeDocBookFile...");

	ofdocstream ofs;
	if (!openFileWrite(ofs, fname))
		return ExportError;

	// make sure we are ready to export
	// this needs to be done before we validate
	updateBuffer();
	updateMacroInstances(OutputUpdate);

	ExportStatus const retval =
		writeDocBookSource(ofs, runparams, output);
	if (retval == ExportKilled)
		return ExportKilled;

	ofs.close();
	if (ofs.fail())
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
	return ExportSuccess;
}


Buffer::ExportStatus Buffer::writeDocBookSource(odocstream & os,
			     OutputParams const & runparams,
			     OutputWhat output) const
{
	LaTeXFeatures features(*this, params(), runparams);
	validate(features);
	d->bibinfo_.makeCitationLabels(*this);

	d->texrow.reset();

	DocumentClass const & tclass = params().documentClass();

	bool const output_preamble =
		output == FullSource || output == OnlyPreamble;
	bool const output_body =
	  output == FullSource || output == OnlyBody || output == IncludedFile;

	if (output_preamble) {
		// XML preamble, no doctype needed.
		// Not using XMLStream for this, as the root tag would be in the tag stack and make troubles with the error
		// detection mechanisms (these are called before the end tag is output, and thus interact with the canary
		// parsep in output_docbook.cpp).
		os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		   << "<!-- This DocBook file was created by LyX " << lyx_version
		   << "\n  See https://www.lyx.org/ for more information -->\n";

		// Directly output the root tag, based on the current type of document.
		string languageCode = params().language->code();
		string params = "xml:lang=\"" + languageCode + '"'
						+ " xmlns=\"http://docbook.org/ns/docbook\""
						+ " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
						+ " xmlns:m=\"http://www.w3.org/1998/Math/MathML\""
						+ " xmlns:xi=\"http://www.w3.org/2001/XInclude\""
						+ " version=\"5.2\"";

		os << "<" << from_ascii(tclass.docbookroot()) << " " << from_ascii(params) << ">\n";
	}

	if (output_body) {
		// Start to output the document.
		XMLStream xs(os);
		docbookParagraphs(text(), *this, xs, runparams);
	}

	if (output_preamble) {
		// Close the root element. No need for a line break, as free text is never allowed
		// in a root element, it must always be wrapped in some container.
		os << "</" << from_ascii(tclass.docbookroot()) << ">";
	}

	return ExportSuccess;
}


Buffer::ExportStatus Buffer::makeLyXHTMLFile(FileName const & fname,
			      OutputParams const & runparams) const
{
	LYXERR(Debug::LATEX, "makeLyXHTMLFile...");

	ofdocstream ofs;
	if (!openFileWrite(ofs, fname))
		return ExportError;

	// make sure we are ready to export
	// this has to be done before we validate
	updateBuffer(UpdateMaster, OutputUpdate);
	updateMacroInstances(OutputUpdate);

	ExportStatus const retval = writeLyXHTMLSource(ofs, runparams, FullSource);
	if (retval == ExportKilled)
		return retval;

	ofs.close();
	if (ofs.fail())
		lyxerr << "File '" << fname << "' was not closed properly." << endl;
	return retval;
}


Buffer::ExportStatus Buffer::writeLyXHTMLSource(odocstream & os,
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
		         xml::escapeString(doctitle, XMLStream::ESCAPE_ALL))
		   << "</title>\n";

		docstring styles = features.getTClassHTMLPreamble();
		if (!styles.empty())
			os << "\n<!-- Text Class Preamble -->\n" << styles << '\n';

		// we will collect CSS information in a stream, and then output it
		// either here, as part of the header, or else in a separate file.
		odocstringstream css;
		styles = features.getCSSSnippets();
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
			os << "<body dir=\"auto\">\n";
		XMLStream xs(os);
		if (output != IncludedFile)
			// if we're an included file, the counters are in the master.
			params().documentClass().counters().reset();
		try {
			xhtmlParagraphs(text(), *this, xs, runparams);
		}
		catch (ConversionException const &) { return ExportKilled; }
		if (output_body_tag)
			os << "</body>\n";
	}

	if (output_preamble)
		os << "</html>\n";

	return ExportSuccess;
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
	runparams.flavor = Flavor::LaTeX;
	runparams.nice = false;
	runparams.linelen = lyxrc.plaintext_linelen;
	ExportStatus const retval =
		makeLaTeXFile(FileName(name), org_path, runparams);
	if (retval != ExportSuccess) {
		// error code on failure
		return -1;
	}

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

	if (!parent())
		clearIncludeList();

	for (Paragraph const & p : paragraphs())
		p.validate(features);

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
	for (auto const & tocit : *toc) {
		if (tocit.depth() == 0)
			list.push_back(tocit.str());
	}
}


void Buffer::invalidateBibinfoCache() const
{
	d->bibinfo_cache_valid_ = false;
	d->cite_labels_valid_ = false;
	removeBiblioTempFiles();
	// also invalidate the cache for the parent buffer
	Buffer const * const pbuf = d->parent();
	if (pbuf)
		pbuf->invalidateBibinfoCache();
}


docstring_list const & Buffer::getBibfiles(UpdateScope scope) const
{
	// FIXME This is probably unnecessary, given where we call this.
	// If this is a child document, use the master instead.
	Buffer const * const pbuf = masterBuffer();
	if (pbuf != this && scope != UpdateChildOnly)
		return pbuf->getBibfiles();

	// In 2.3.x, we have:
	//if (!d->bibfile_cache_valid_)
	//	this->updateBibfilesCache(scope);
	// I think that is a leftover, but there have been so many back-
	// and-forths with this, due to Windows issues, that I am not sure.

	return d->bibfiles_cache_;
}


BiblioInfo const & Buffer::masterBibInfo() const
{
	Buffer const * const tmp = masterBuffer();
	if (tmp != this)
		return tmp->masterBibInfo();
	return d->bibinfo_;
}


BiblioInfo const & Buffer::bibInfo() const
{
	return d->bibinfo_;
}


void Buffer::registerBibfiles(const docstring_list & bf) const
{
	// We register the bib files in the master buffer,
	// if there is one, but also in every single buffer,
	// in case a child is compiled alone.
	Buffer const * const tmp = masterBuffer();
	if (tmp != this)
		tmp->registerBibfiles(bf);

	for (auto const & p : bf) {
		docstring_list::const_iterator temp =
			find(d->bibfiles_cache_.begin(), d->bibfiles_cache_.end(), p);
		if (temp == d->bibfiles_cache_.end())
			d->bibfiles_cache_.push_back(p);
	}
}


static map<docstring, FileName> bibfileCache;

FileName Buffer::getBibfilePath(docstring const & bibid) const
{
	map<docstring, FileName>::const_iterator it =
		bibfileCache.find(bibid);
	if (it != bibfileCache.end()) {
		// i.e., return bibfileCache[bibid];
		return it->second;
	}

	LYXERR(Debug::FILES, "Reading file location for " << bibid);
	string const texfile = changeExtension(to_utf8(bibid), "bib");
	// we need to check first if this file exists where it's said to be.
	// there's a weird bug that occurs otherwise: if the file is in the
	// Buffer's directory but has the same name as some file that would be
	// found by kpsewhich, then we find the latter, not the former.
	FileName const local_file = makeAbsPath(texfile, filePath());
	FileName file = local_file;
	if (!file.exists()) {
		// there's no need now to check whether the file can be found
		// locally
		file = findtexfile(texfile, "bib", true);
		if (file.empty())
			file = local_file;
	}
	LYXERR(Debug::FILES, "Found at: " << file);

	bibfileCache[bibid] = file;
	return bibfileCache[bibid];
}


void Buffer::checkIfBibInfoCacheIsValid() const
{
	// use the master's cache
	Buffer const * const tmp = masterBuffer();
	if (tmp != this) {
		tmp->checkIfBibInfoCacheIsValid();
		return;
	}

	// If we already know the cache is invalid, stop here.
	// This is important in the case when the bibliography
	// environment (rather than Bib[la]TeX) is used.
	// In that case, the timestamp check below gives no
	// sensible result. Rather than that, the cache will
	// be invalidated explicitly via invalidateBibInfoCache()
	// by the Bibitem inset.
	// Same applies for bib encoding changes, which trigger
	// invalidateBibInfoCache() by InsetBibtex.
	if (!d->bibinfo_cache_valid_)
		return;

	if (d->have_bibitems_) {
		// We have a bibliography environment.
		// Invalidate the bibinfo cache unconditionally.
		// Cite labels will get invalidated by the inset if needed.
		d->bibinfo_cache_valid_ = false;
		return;
	}

	// OK. This is with Bib(la)tex. We'll assume the cache
	// is valid and change this if we find changes in the bibs.
	d->bibinfo_cache_valid_ = true;
	d->cite_labels_valid_ = true;

	// compare the cached timestamps with the actual ones.
	docstring_list const & bibfiles_cache = getBibfiles();
	for (auto const & bf : bibfiles_cache) {
		FileName const fn = getBibfilePath(bf);
		time_t lastw = fn.lastModified();
		time_t prevw = d->bibfile_status_[fn];
		if (lastw != prevw) {
			d->bibinfo_cache_valid_ = false;
			d->cite_labels_valid_ = false;
			d->bibfile_status_[fn] = lastw;
		}
	}
}


void Buffer::clearBibFileCache() const
{
	bibfileCache.clear();
}


void Buffer::reloadBibInfoCache(bool const force) const
{
	// we should not need to do this for internal buffers
	if (isInternal())
		return;

	// use the master's cache
	Buffer const * const tmp = masterBuffer();
	if (tmp != this) {
		tmp->reloadBibInfoCache(force);
		return;
	}

	if (!force) {
		checkIfBibInfoCacheIsValid();
		if (d->bibinfo_cache_valid_)
			return;
	}

	LYXERR(Debug::FILES, "Bibinfo cache was invalid.");
	// re-read file locations when this info changes
	// FIXME Is this sufficient? Or should we also force that
	// in some other cases? If so, then it is easy enough to
	// add the following line in some other places.
	clearBibFileCache();
	d->bibinfo_.clear();
	FileNameList checkedFiles;
	d->have_bibitems_ = false;
	collectBibKeys(checkedFiles);
	d->bibinfo_cache_valid_ = true;
}


void Buffer::collectBibKeys(FileNameList & checkedFiles) const
{
	if (!parent())
		clearIncludeList();

	for (InsetIterator it = begin(inset()); it; ++it) {
		it->collectBibKeys(it, checkedFiles);
		if (it->lyxCode() == BIBITEM_CODE) {
			if (parent() != nullptr)
				parent()->d->have_bibitems_ = true;
			else
				d->have_bibitems_ = true;
		}
	}
}


void Buffer::addBiblioInfo(BiblioInfo const & bi_in) const
{
	// We add the biblio info to the parent buffer,
	// if there is one, but also to this buffer, in case
	// it is compiled alone.
	BiblioInfo & our_bi = d->bibinfo_;
	our_bi.mergeBiblioInfo(bi_in);

	if (parent())
		parent()->addBiblioInfo(bi_in);
}


void Buffer::addBibTeXInfo(docstring const & key, BibTeXInfo const & bin) const
{
	// We add the bibtex info to the master buffer,
	// if there is one, but also to every single buffer,
	// in case a child is compiled alone.
	BiblioInfo & bi = d->bibinfo_;
	bi[key] = bin;

	if (parent() != nullptr) {
		BiblioInfo & masterbi = masterBuffer()->d->bibinfo_;
		masterbi[key] = bin;
	}
}


void Buffer::makeCitationLabels() const
{
	Buffer const * const master = masterBuffer();
	return d->bibinfo_.makeCitationLabels(*master);
}


void Buffer::invalidateCiteLabels() const
{
	masterBuffer()->d->cite_labels_valid_ = false;
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
		flag.setOnOff(hasReadonlyFlag());
		break;

		// FIXME: There is need for a command-line import.
		//case LFUN_BUFFER_IMPORT:

	case LFUN_BUFFER_AUTO_SAVE:
		break;

	case LFUN_BUFFER_EXPORT_CUSTOM:
		// FIXME: Nothing to check here?
		break;

	case LFUN_BUFFER_EXPORT: {
		docstring const & arg = cmd.argument();
		if (arg == "custom") {
			enable = true;
			break;
		}
		string format = (arg.empty() || arg == "default") ?
			params().getDefaultOutputFormat() : to_utf8(arg);
		size_t pos = format.find(' ');
		if (pos != string::npos)
			format = format.substr(0, pos);
		enable = params().isExportable(format, false);
		if (!enable)
			flag.message(bformat(
					     _("Don't know how to export to format: %1$s"), arg));
		break;
	}

	case LFUN_BUILD_PROGRAM:
		enable = params().isExportable("program", false);
		break;

	case LFUN_BRANCH_ACTIVATE:
	case LFUN_BRANCH_DEACTIVATE:
	case LFUN_BRANCH_MASTER_ACTIVATE:
	case LFUN_BRANCH_MASTER_DEACTIVATE: {
		bool const master = (cmd.action() == LFUN_BRANCH_MASTER_ACTIVATE
				     || cmd.action() == LFUN_BRANCH_MASTER_DEACTIVATE);
		BranchList const & branchList = master ? masterBuffer()->params().branchlist()
			: params().branchlist();
		docstring const & branchName = cmd.argument();
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

	case LFUN_BUFFER_TOGGLE_COMPRESSION:
		flag.setOnOff(params().compressed);
		break;

	case LFUN_BUFFER_TOGGLE_OUTPUT_SYNC:
		flag.setOnOff(params().output_sync);
		break;

	case LFUN_BUFFER_ANONYMIZE:
		break;

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
	// This handles undo groups automagically
	UndoGroupHelper ugh(this);

	switch (func.action()) {
	case LFUN_BUFFER_TOGGLE_READ_ONLY:
		if (lyxvc().inUse()) {
			string log = lyxvc().toggleReadOnly();
			if (!log.empty())
				dr.setMessage(log);
		}
		else
			setReadonly(!hasReadonlyFlag());
		break;

	case LFUN_BUFFER_EXPORT: {
		string const format = (argument.empty() || argument == "default") ?
			params().getDefaultOutputFormat() : argument;
		ExportStatus const status = doExport(format, false);
		dr.setError(status != ExportSuccess);
		if (status != ExportSuccess)
			dr.setMessage(bformat(_("Error exporting to format: %1$s."),
			                      from_utf8(format)));
		break;
	}

	case LFUN_BUILD_PROGRAM: {
		ExportStatus const status = doExport("program", true);
		dr.setError(status != ExportSuccess);
		if (status != ExportSuccess)
			dr.setMessage(_("Error generating literate programming code."));
		break;
	}

	case LFUN_BUFFER_EXPORT_CUSTOM: {
		string format_name;
		string command = split(argument, format_name, ' ');
		Format const * format = theFormats().getFormat(format_name);
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

		docstring const & branch_name = func.argument();
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
		docstring const & branchnames = func.argument();
		if (branchnames.empty()) {
			dispatched = false;
			break;
		}
		BranchList & branch_list = params().branchlist();
		vector<docstring> const branches =
			getVectorFromString(branchnames, branch_list.separator());
		docstring msg;
		for (docstring const & branch_name : branches) {
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
		InsetIterator it  = begin(inset());
		InsetIterator const itend = end(inset());
		bool success = false;
		for (; it != itend; ++it) {
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
				Buffer * child = ins.loadIfNeeded();
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
		if (!theFormats().view(*this, d->preview_file_,
				  d->preview_format_))
			dr.setMessage(_("Error viewing the output file."));
		break;

	case LFUN_CHANGES_TRACK:
		if (params().save_transient_properties)
			undo().recordUndoBufferParams(CursorData());
		params().track_changes = !params().track_changes;
		break;

	case LFUN_CHANGES_OUTPUT:
		if (params().save_transient_properties)
			undo().recordUndoBufferParams(CursorData());
		params().output_changes = !params().output_changes;
		if (params().output_changes) {
			bool xcolorulem = LaTeXFeatures::isAvailable("ulem") &&
					  LaTeXFeatures::isAvailable("xcolor");

			if (!xcolorulem) {
				Alert::warning(_("Changes not shown in LaTeX output"),
					       _("Changes will not be highlighted in LaTeX output, "
						 "because xcolor and ulem are not installed.\n"
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

	case LFUN_BUFFER_ANONYMIZE: {
		undo().recordUndoFullBuffer(CursorData());
		CursorData cur(doc_iterator_begin(this));
		for ( ; cur ; cur.forwardPar())
			cur.paragraph().anonymize();
		dr.forceBufferUpdate();
		dr.screenUpdate(Update::Force);
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
	LASSERT(from, return);
	LASSERT(to, return);

	ParIterator it = par_iterator_begin();
	ParIterator eit = par_iterator_end();
	for (; it != eit; ++it)
		it->changeLanguage(params(), from, to);
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
	std::set<Language const *> langs;
	getLanguages(langs);
	return langs;
}


void Buffer::getLanguages(std::set<Language const *> & langs) const
{
	ParConstIterator end = par_iterator_end();
	// add the buffer language, even if it's not actively used
	langs.insert(language());
	// iterate over the paragraphs
	for (ParConstIterator it = par_iterator_begin(); it != end; ++it)
		it->getLanguages(langs);
	// also children
	ListOfBuffers clist = getDescendants();
	for (auto const & cit : clist)
		cit->getLanguages(langs);
}


DocIterator Buffer::getParFromID(int const id) const
{
	Buffer * buf = const_cast<Buffer *>(this);
	if (id < 0)
		// This means non-existent
		return doc_iterator_end(buf);

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

bool Buffer::empty() const
{
	return paragraphs().size() == 1 && paragraphs().front().empty();
}


Language const * Buffer::language() const
{
	return params().language;
}


docstring Buffer::B_(string const & l10n) const
{
	return params().B_(l10n);
}


bool Buffer::isClean() const
{
	return d->lyx_clean;
}


bool Buffer::isChecksumModified() const
{
	LASSERT(d->filename.exists(), return false);
	return d->checksum_ != d->filename.checksum();
}


void Buffer::saveCheckSum() const
{
	FileName const & file = d->filename;
	file.refresh();
	d->checksum_ = file.exists() ? file.checksum()
		: 0; // in the case of save to a new file.
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
	clearExternalModification();
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

	for (auto & depit : d->dep_clean)
		depit.second = false;
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


string const Buffer::prepareFileNameForLaTeX(string const & name,
					     string const & ext, bool nice) const
{
	string const fname = makeAbsPath(name, filePath()).absFileName();
	if (FileName::isAbsolute(name) || !FileName(fname + ext).isReadableFile())
		return name;
	if (!nice)
		return fname;

	// FIXME UNICODE
	return to_utf8(makeRelPath(from_utf8(fname),
		from_utf8(masterBuffer()->filePath())));
}


vector<pair<docstring, string>> const Buffer::prepareBibFilePaths(OutputParams const & runparams,
						docstring_list const & bibfilelist,
						bool const add_extension) const
{
	// If we are processing the LaTeX file in a temp directory then
	// copy the .bib databases to this temp directory, mangling their
	// names in the process. Store this mangled name in the list of
	// all databases.
	// (We need to do all this because BibTeX *really*, *really*
	// can't handle "files with spaces" and Windows users tend to
	// use such filenames.)
	// Otherwise, store the (maybe absolute) path to the original,
	// unmangled database name.

	vector<pair<docstring, string>> res;

	// determine the export format
	string const tex_format = flavor2format(runparams.flavor);

	// check for spaces in paths
	bool found_space = false;

	for (auto const & bit : bibfilelist) {
		string utf8input = to_utf8(bit);
		string database =
			prepareFileNameForLaTeX(utf8input, ".bib", runparams.nice);
		FileName try_in_file =
			makeAbsPath(database + ".bib", filePath());
		bool not_from_texmf = try_in_file.isReadableFile();
		// If the file has not been found, try with the real file name
		// (it might come from a child in a sub-directory)
		if (!not_from_texmf) {
			try_in_file = getBibfilePath(bit);
			if (try_in_file.isReadableFile()) {
				// Check if the file is in texmf
				FileName kpsefile(findtexfile(changeExtension(utf8input, "bib"), "bib", true));
				not_from_texmf = kpsefile.empty()
						|| kpsefile.absFileName() != try_in_file.absFileName();
				if (not_from_texmf)
					// If this exists, make path relative to the master
					// FIXME Unicode
					database =
						removeExtension(prepareFileNameForLaTeX(
											to_utf8(makeRelPath(from_utf8(try_in_file.absFileName()),
																from_utf8(filePath()))),
											".bib", runparams.nice));
			}
		}

		if (!runparams.inComment && !runparams.dryrun && !runparams.nice &&
		    not_from_texmf) {
			// mangledFileName() needs the extension
			DocFileName const in_file = DocFileName(try_in_file);
			database = removeExtension(in_file.mangledFileName());
			FileName const out_file = makeAbsPath(database + ".bib",
					masterBuffer()->temppath());
			bool const success = in_file.copyTo(out_file);
			if (!success) {
				LYXERR0("Failed to copy '" << in_file
				       << "' to '" << out_file << "'");
			}
		} else if (!runparams.inComment && runparams.nice && not_from_texmf) {
			runparams.exportdata->addExternalFile(tex_format, try_in_file, database + ".bib");
			if (!isValidLaTeXFileName(database)) {
				frontend::Alert::warning(_("Invalid filename"),
					 _("The following filename will cause troubles "
					       "when running the exported file through LaTeX: ") +
					     from_utf8(database));
			}
			if (!isValidDVIFileName(database)) {
				frontend::Alert::warning(_("Problematic filename for DVI"),
					 _("The following filename can cause troubles "
					       "when running the exported file through LaTeX "
						   "and opening the resulting DVI: ") +
					     from_utf8(database), true);
			}
		}

		if (add_extension)
			database += ".bib";

		// FIXME UNICODE
		docstring const path = from_utf8(latex_path(database));

		if (contains(path, ' '))
			found_space = true;
		string enc;
		if (params().useBiblatex() && !params().bibFileEncoding(utf8input).empty())
			enc = params().bibFileEncoding(utf8input);

		bool recorded = false;
		for (auto const & pe : res) {
			if (pe.first == path) {
				recorded = true;
				break;
			}

		}
		if (!recorded)
			res.push_back(make_pair(path, enc));
	}

	// Check if there are spaces in the path and warn BibTeX users, if so.
	// (biber can cope with such paths)
	if (!prefixIs(runparams.bibtex_command, "biber")) {
		// Post this warning only once.
		static bool warned_about_spaces = false;
		if (!warned_about_spaces &&
		    runparams.nice && found_space) {
			warned_about_spaces = true;
			Alert::warning(_("Export Warning!"),
				       _("There are spaces in the paths to your BibTeX databases.\n"
						      "BibTeX will be unable to find them."));
		}
	}

	return res;
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


bool Buffer::hasReadonlyFlag() const
{
	return d->read_only;
}


bool Buffer::isReadonly() const
{
	return hasReadonlyFlag() || notifiesExternalModification();
}


void Buffer::setParent(Buffer const * buffer)
{
	// We need to do some work here to avoid recursive parent structures.
	// This is the easy case.
	if (buffer == this) {
		LYXERR0("Ignoring attempt to set self as parent in\n" << fileName());
		return;
	}
	// Now we check parents going upward, to make sure that IF we set the
	// parent as requested, we would not generate a recursive include.
	set<Buffer const *> sb;
	Buffer const * b = buffer;
	bool found_recursion = false;
	while (b) {
		if (sb.find(b) != sb.end()) {
			found_recursion = true;
			break;
		}
		sb.insert(b);
		b = b->parent();
	}

	if (found_recursion) {
		LYXERR0("Ignoring attempt to set parent of\n" <<
			fileName() <<
			"\nto " <<
			buffer->fileName() <<
			"\nbecause that would create a recursive inclusion.");
		return;
	}

	// We should be safe now.
	d->setParent(buffer);
	updateMacros();
}


Buffer const * Buffer::parent() const
{
	return d->parent();
}


ListOfBuffers Buffer::allRelatives() const
{
	ListOfBuffers lb = masterBuffer()->getDescendants();
	lb.push_front(const_cast<Buffer *>(masterBuffer()));
	return lb;
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


bool Buffer::hasChildren() const
{
	return !d->children_positions.empty();
}


void Buffer::collectChildren(ListOfBuffers & children, bool grand_children) const
{
	// loop over children
	for (auto const & p : d->children_positions) {
		Buffer * child = const_cast<Buffer *>(p.first);
		// No duplicates
		ListOfBuffers::const_iterator bit = find(children.begin(), children.end(), child);
		if (bit != children.end())
			continue;
		children.push_back(child);
		if (grand_children)
			// there might be grandchildren
			child->collectChildren(children, true);
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


ListOfBuffers Buffer::getDescendants() const
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

	--it;
	return it;
}


MacroData const * Buffer::Impl::getBufferMacro(docstring const & name,
					 DocIterator const & pos) const
{
	LYXERR(Debug::MACROS, "Searching for " << to_ascii(name) << " at " << pos);

	// if paragraphs have no macro context set, pos will be empty
	if (pos.empty())
		return nullptr;

	// we haven't found anything yet
	DocIterator bestPos = owner_->par_iterator_begin();
	MacroData const * bestData = nullptr;

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
		return nullptr;

	// query buffer macros
	MacroData const * data = d->getBufferMacro(name, pos);
	if (data != nullptr)
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
		if (data != nullptr)
			return data;
	}

	return nullptr;
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
		return nullptr;

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
		for (auto const & insit : par.insetList()) {
			it.pos() = insit.pos;

			// is it a nested text inset?
			if (insit.inset->asInsetText()) {
				// Inset needs its own scope?
				InsetText const * itext = insit.inset->asInsetText();
				bool newScope = itext->isMacroScope();

				// scope which ends just behind the inset
				DocIterator insetScope = it;
				++insetScope.pos();

				// collect macros in inset
				it.push_back(CursorSlice(*insit.inset));
				updateMacros(it, newScope ? insetScope : scope);
				it.pop_back();
				continue;
			}

			if (insit.inset->asInsetTabular()) {
				CursorSlice slice(*insit.inset);
				size_t const numcells = slice.nargs();
				for (; slice.idx() < numcells; slice.forwardIdx()) {
					it.push_back(slice);
					updateMacros(it, scope);
					it.pop_back();
				}
				continue;
			}

			// is it an external file?
			if (insit.inset->lyxCode() == INCLUDE_CODE) {
				// get buffer of external file
				InsetInclude const & incinset =
					static_cast<InsetInclude const &>(*insit.inset);
				macro_lock = true;
				Buffer * child = incinset.loadIfNeeded();
				macro_lock = false;
				if (!child)
					continue;

				// register its position, but only when it is
				// included first in the buffer
				children_positions.insert({child, it});

				// register child with its scope
				position_to_children[it] = Impl::ScopeBuffer(scope, child);
				continue;
			}

			InsetMath * im = insit.inset->asInsetMath();
			if (doing_export && im)  {
				InsetMathHull * hull = im->asHullInset();
				if (hull)
					hull->recordLocation(it);
			}

			if (insit.inset->lyxCode() != MATHMACRO_CODE)
				continue;

			// get macro data
			InsetMathMacroTemplate & macroTemplate =
				*insit.inset->asInsetMath()->asMacroTemplate();
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
	for (Inset const & it : inset()) {
		if (it.lyxCode() == BRANCH_CODE) {
			InsetBranch const & br = static_cast<InsetBranch const &>(it);
			docstring const name = br.branch();
			if (!from_master && !params().branchlist().find(name))
				result.push_back(name);
			else if (from_master && !masterBuffer()->params().branchlist().find(name))
				result.push_back(name);
			continue;
		}
		if (it.lyxCode() == INCLUDE_CODE) {
			// get buffer of external file
			InsetInclude const & ins =
				static_cast<InsetInclude const &>(it);
			Buffer * child = ins.loadIfNeeded();
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
		idx_type n = minset->nargs();
		MacroContext mc = MacroContext(this, it);
		for (idx_type i = 0; i < n; ++i) {
			MathData & data = minset->cell(i);
			data.updateMacros(nullptr, mc, utype, 0);
		}
	}
}


void Buffer::listMacroNames(MacroNameSet & macros) const
{
	if (d->macro_lock)
		return;

	d->macro_lock = true;

	// loop over macro names
	for (auto const & nameit : d->macros)
		macros.insert(nameit.first);

	// loop over children
	for (auto const & p : d->children_positions) {
		Buffer * child = const_cast<Buffer *>(p.first);
		// The buffer might have been closed (see #10766).
		if (theBufferList().isLoaded(child))
			child->listMacroNames(macros);
	}

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
	for (auto const & mit : names) {
		// defined?
		MacroData const * data = pbuf->getMacro(mit, *this, false);
		if (data) {
			macros.insert(data);

			// we cannot access the original InsetMathMacroTemplate anymore
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
		return it->second;

	static References const dummy_refs = References();
	it = d->ref_cache_.insert(
		make_pair(label, dummy_refs)).first;
	return it->second;
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


void Buffer::setInsetLabel(docstring const & label, InsetLabel const * il,
			   bool const active)
{
	LabelInfo linfo;
	linfo.label = label;
	linfo.inset = il;
	linfo.active = active;
	masterBuffer()->d->label_cache_.push_back(linfo);
}


InsetLabel const * Buffer::insetLabel(docstring const & label,
				      bool const active) const
{
	for (auto const & rc : masterBuffer()->d->label_cache_) {
		if (rc.label == label && (rc.active || !active))
			return rc.inset;
	}
	return nullptr;
}


bool Buffer::activeLabel(docstring const & label) const
{
	return insetLabel(label, true) != nullptr;
}


void Buffer::clearReferenceCache() const
{
	if (!d->parent()) {
		d->ref_cache_.clear();
		d->label_cache_.clear();
	}
}


void Buffer::changeRefsIfUnique(docstring const & from, docstring const & to)
{
	//FIXME: This does not work for child documents yet.
	reloadBibInfoCache();

	// Check if the label 'from' appears more than once
	vector<docstring> labels;
	for (auto const & bibit : masterBibInfo())
		labels.push_back(bibit.first);

	if (count(labels.begin(), labels.end(), from) > 1)
		return;

	string const paramName = "key";
	UndoGroupHelper ugh(this);
	InsetIterator it = begin(inset());
	for (; it; ++it) {
		if (it->lyxCode() != CITE_CODE)
			continue;
		InsetCommand * inset = it->asInsetCommand();
		docstring const oldValue = inset->getParam(paramName);
		if (oldValue == from) {
			undo().recordUndo(CursorData(it));
			inset->setParam(paramName, to);
		}
	}
}

// returns NULL if id-to-row conversion is unsupported
unique_ptr<TexRow> Buffer::getSourceCode(odocstream & os, string const & format,
                                         pit_type par_begin, pit_type par_end,
                                         OutputWhat output, bool master) const
{
	unique_ptr<TexRow> texrow;
	OutputParams runparams(&params().encoding());
	runparams.nice = true;
	runparams.flavor = params().getOutputFlavor(format);
	runparams.linelen = lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;

	// Some macros rely on font encoding
	runparams.main_fontenc = params().main_font_encoding();

	// Use the right wrapping for the comment at the beginning of the generated
	// snippet, so that it is either valid LaTeX or valid XML (including HTML and DocBook).
	docstring comment_start = from_ascii("% ");
	docstring comment_end = from_ascii("");
	if (runparams.flavor == Flavor::Html || runparams.flavor == Flavor::DocBook5) {
		comment_start = from_ascii("<!-- ");
		comment_end = from_ascii(" -->");
	}

	if (output == CurrentParagraph) {
		runparams.par_begin = par_begin;
		runparams.par_end = par_end;
		if (par_begin + 1 == par_end) {
			os << comment_start
			   << bformat(_("Preview source code for paragraph %1$d"), par_begin)
			   << comment_end
			   << "\n\n";
		} else {
			os << comment_start
			   << bformat(_("Preview source code from paragraph %1$s to %2$s"),
					convert<docstring>(par_begin),
					convert<docstring>(par_end - 1))
			   << comment_end
			   << "\n\n";
		}
		// output paragraphs
		if (runparams.flavor == Flavor::LyX) {
			Paragraph const & par = text().paragraphs()[par_begin];
			ostringstream ods;
			depth_type dt = par.getDepth();
			par.write(ods, params(), dt);
			os << from_utf8(ods.str());
		} else if (runparams.flavor == Flavor::Html) {
			XMLStream xs(os);
			setMathFlavor(runparams);
			xhtmlParagraphs(text(), *this, xs, runparams);
		} else if (runparams.flavor == Flavor::Text) {
			bool dummy = false;
			// FIXME Handles only one paragraph, unlike the others.
			// Probably should have some routine with a signature like them.
			writePlaintextParagraph(*this,
				text().paragraphs()[par_begin], os, runparams, dummy);
		} else if (runparams.flavor == Flavor::DocBook5) {
			XMLStream xs{os};
			docbookParagraphs(text(), *this, xs, runparams);
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
			validate(features);
			runparams.use_polyglossia = features.usePolyglossia();
			runparams.use_hyperref = features.isRequired("hyperref");
			// latex or literate
			otexstream ots(os);
			// output above
			ots.texrow().newlines(2);
			// the real stuff
			latexParagraphs(*this, text(), ots, runparams);
			texrow = ots.releaseTexRow();

			// Restore the parenthood
			if (!master)
				d->ignore_parent = false;
		}
	} else {
		os << comment_start;
		if (output == FullSource)
			os << _("Preview source code");
		else if (output == OnlyPreamble)
			os << _("Preview preamble");
		else if (output == OnlyBody)
			os << _("Preview body");
		os << comment_end;
		os << "\n\n";
		if (runparams.flavor == Flavor::LyX) {
			ostringstream ods;
			if (output == FullSource)
				write(ods);
			else if (output == OnlyPreamble)
				params().writeFile(ods, this);
			else if (output == OnlyBody)
				text().write(ods);
			os << from_utf8(ods.str());
		} else if (runparams.flavor == Flavor::Html) {
			writeLyXHTMLSource(os, runparams, output);
		} else if (runparams.flavor == Flavor::Text) {
			if (output == OnlyPreamble)
				os << "% "<< _("Plain text does not have a preamble.");
			else
				writePlaintextFile(*this, os, runparams);
		} else if (runparams.flavor == Flavor::DocBook5) {
			writeDocBookSource(os, runparams, output);
		} else {
			// latex or literate
			otexstream ots(os);
			// output above
			ots.texrow().newlines(2);
			if (master)
				runparams.is_child = true;
			updateBuffer();
			writeLaTeXSource(ots, string(), runparams, output);
			texrow = ots.releaseTexRow();
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


void Buffer::moveAutosaveFile(FileName const & oldauto) const
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
	if (buf->d->bak_clean || hasReadonlyFlag())
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
	ListOfBuffers clist = getDescendants();
	for (auto const & bit : clist)
		bit->d->doing_export = e;
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
			params().maintain_unincluded_children != BufferParams::CM_None
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
		if (format == "default")
			format = params().getDefaultOutputFormat();
		runparams.export_folder = FileName(dest_filename).onlyPath().realPath();
		FileName(dest_filename).onlyPath().createPath();
		LYXERR(Debug::FILES, "format=" << format << ", dest_filename=" << dest_filename << ", export_folder=" << runparams.export_folder);
	}
	MarkAsExporting exporting(this);
	string backend_format;
	runparams.flavor = Flavor::LaTeX;
	runparams.linelen = lyxrc.plaintext_linelen;
	runparams.includeall = includeall;
	vector<string> backs = params().backends();
	Converters converters = theConverters();
	bool need_nice_file = false;
	if (find(backs.begin(), backs.end(), format) == backs.end()) {
		// Get shortest path to format
		converters.buildGraph();
		Graph::EdgePath path;
		for (string const & sit : backs) {
			Graph::EdgePath p = converters.getPath(sit, format);
			if (!p.empty() && (path.empty() || p.size() < path.size())) {
				backend_format = sit;
				path = p;
			}
		}
		if (path.empty()) {
			if (!put_in_tempdir) {
				// Only show this alert if this is an export to a non-temporary
				// file (not for previewing).
				docstring s = bformat(_("No information for exporting the format %1$s."),
						      translateIfPossible(theFormats().prettyName(format)));
				if (format == "pdf4")
					s += "\n"
					  + bformat(_("Hint: use non-TeX fonts or set input encoding "
						  " to '%1$s'"), from_utf8(encodings.fromLyXName("ascii")->guiName()));
				Alert::error(_("Couldn't export file"), s);
			}
			return ExportNoPathToFormat;
		}
		runparams.flavor = converters.getFlavor(path, this);
		runparams.hyperref_driver = converters.getHyperrefDriver(path);
		for (auto const & edge : path)
			if (theConverters().get(edge).nice()) {
				need_nice_file = true;
				break;
			}

	} else {
		backend_format = format;
		LYXERR(Debug::FILES, "backend_format=" << backend_format);
		// FIXME: Don't hardcode format names here, but use a flag
		if (backend_format == "pdflatex")
			runparams.flavor = Flavor::PdfLaTeX;
		else if (backend_format == "luatex")
			runparams.flavor = Flavor::LuaTeX;
		else if (backend_format == "dviluatex")
			runparams.flavor = Flavor::DviLuaTeX;
		else if (backend_format == "xetex")
			runparams.flavor = Flavor::XeTeX;
	}

	string filename = latexName(false);
	filename = addName(temppath(), filename);
	filename = changeExtension(filename,
				   theFormats().extension(backend_format));
	LYXERR(Debug::FILES, "filename=" << filename);

	// Plain text backend
	if (backend_format == "text") {
		runparams.flavor = Flavor::Text;
		try {
			writePlaintextFile(*this, FileName(filename), runparams);
		}
		catch (ConversionException const &) { return ExportCancel; }
	}
	// HTML backend
	else if (backend_format == "xhtml") {
		runparams.flavor = Flavor::Html;
		setMathFlavor(runparams);
		if (makeLyXHTMLFile(FileName(filename), runparams) == ExportKilled)
			return ExportKilled;
	} else if (backend_format == "lyx")
		writeFile(FileName(filename));
	// DocBook backend
	else if (backend_format == "docbook5") {
		runparams.flavor = Flavor::DocBook5;
		runparams.nice = false;
		if (makeDocBookFile(FileName(filename), runparams) == ExportKilled)
			return ExportKilled;
	}
	// LaTeX backend
	else if (backend_format == format || need_nice_file) {
		runparams.nice = true;
		ExportStatus const retval =
			makeLaTeXFile(FileName(filename), string(), runparams);
		if (retval == ExportKilled)
			return ExportKilled;
		if (d->cloned_buffer_)
			d->cloned_buffer_->d->errorLists["Export"] = d->errorLists["Export"];
		if (retval != ExportSuccess)
			return retval;
	} else if (!lyxrc.tex_allows_spaces
		   && contains(filePath(), ' ')) {
		Alert::error(_("File name error"),
			bformat(_("The directory path to the document\n%1$s\n"
			    "contains spaces, but your TeX installation does "
			    "not allow them. You should save the file to a directory "
					"whose name does not contain spaces."), from_utf8(filePath())));
		return ExportTexPathHasSpaces;
	} else {
		runparams.nice = false;
		ExportStatus const retval =
			makeLaTeXFile(FileName(filename), filePath(), runparams);
		if (retval == ExportKilled)
			return ExportKilled;
		if (d->cloned_buffer_)
			d->cloned_buffer_->d->errorLists["Export"] = d->errorLists["Export"];
		if (retval != ExportSuccess)
			return ExportError;
	}

	string const error_type = (format == "program")
		? "Build" : params().bufferFormat();
	ErrorList & error_list = d->errorLists[error_type];
	string const ext = theFormats().extension(format);
	FileName const tmp_result_file(changeExtension(filename, ext));
	Converters::RetVal const retval = 
		converters.convert(this, FileName(filename), tmp_result_file,
				   FileName(absFileName()), backend_format, format,
				   error_list, Converters::none, includeall);
	if (retval == Converters::KILLED)
		return ExportCancel;
	bool success = (retval == Converters::SUCCESS);

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
		ListOfBuffers clist = getDescendants();
		for (auto const & bit : clist) {
			if (runparams.silent)
				bit->d->errorLists[error_type].clear();
			else if (d->cloned_buffer_) {
				// Enable reverse search by copying back the
				// texrow object to the cloned buffer.
				// FIXME: this is not thread safe.
				bit->d->cloned_buffer_->d->texrow = bit->d->texrow;
				bit->d->cloned_buffer_->d->errorLists[error_type] =
					bit->d->errorLists[error_type];
			} else
				bit->errors(error_type, true);
		}
	}

	if (d->cloned_buffer_) {
		// Enable reverse dvi or pdf to work by copying back the texrow
		// object to the cloned buffer.
		// FIXME: There is a possibility of concurrent access to texrow
		// here from the main GUI thread that should be securized.
		d->cloned_buffer_->d->texrow = d->texrow;
		string const err_type = params().bufferFormat();
		d->cloned_buffer_->d->errorLists[error_type] = d->errorLists[err_type];
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
	vector<ExportedFile> const extfiles =
		runparams.exportdata->externalFiles(format);
	string const dest = runparams.export_folder.empty() ?
		onlyPath(result_file) : runparams.export_folder;
	bool use_force = use_gui ? lyxrc.export_overwrite == ALL_FILES
				 : force_overwrite == ALL_FILES;
	CopyStatus status = use_force ? FORCE : SUCCESS;

	for (ExportedFile const & exp : extfiles) {
		if (status == CANCEL) {
			message(_("Document export cancelled."));
			return ExportCancel;
		}
		string const fmt = theFormats().getFormatFromFile(exp.sourceName);
		string fixedName = exp.exportName;
		if (!runparams.export_folder.empty()) {
			// Relative pathnames starting with ../ will be sanitized
			// if exporting to a different folder
			while (fixedName.substr(0, 3) == "../")
				fixedName = fixedName.substr(3, fixedName.length() - 3);
		}
		FileName fixedFileName = makeAbsPath(fixedName, dest);
		fixedFileName.onlyPath().createPath();
		status = copyFile(fmt, exp.sourceName,
			fixedFileName,
			exp.exportName, status == FORCE,
			runparams.export_folder.empty());
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
				translateIfPossible(theFormats().prettyName(format)),
				makeDisplayPath(result_file)));
		}
	} else {
		// This must be a dummy converter like fax (bug 1888)
		message(bformat(_("Document exported as %1$s"),
			translateIfPossible(theFormats().prettyName(format))));
	}

	return success ? ExportSuccess : ExportConverterError;
}


Buffer::ExportStatus Buffer::preview(string const & format) const
{
	bool const update_unincluded =
			params().maintain_unincluded_children != BufferParams::CM_None
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
	theimpl->require_fresh_start_ = (status != ExportSuccess);

	if (status != ExportSuccess)
		return status;

	if (previewFile.exists())
		return theFormats().view(*this, previewFile, format) ?
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
			if (hasReadonlyFlag()) {
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
		else {
			// See bug #11464
			FileName newname;
			string const ename = emergencyFile.absFileName();
			bool noname = true;
			// Surely we can find one in 100 tries?
			for (int i = 1; i < 100; ++i) {
				newname.set(ename + to_string(i) + ".lyx");
				if (!newname.exists()) {
					noname = false;
					break;
				}
			}
			if (!noname) {
				// renameTo returns true on success. So inverting that
				// will give us true if we fail.
				noname = !emergencyFile.renameTo(newname);
			}
			if (noname) {
				Alert::warning(_("Can't rename emergency file!"),
					_("LyX was unable to rename the emergency file. "
					  "You should do so manually. Otherwise, you will be "
					  "asked about it again the next time you try to load "
					  "this file, and may over-write your own work."));
			} else {
				Alert::warning(_("Emergency File Renames"),
					bformat(_("Emergency file renamed as:\n %1$s"),
					from_utf8(newname.onlyFileName())));
			}
		}
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
			if (hasReadonlyFlag()) {
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


void Buffer::Impl::traverseErrors(TeXErrors::Errors::const_iterator err, TeXErrors::Errors::const_iterator end, ErrorList & errorList) const
{
	for (; err != end; ++err) {
		TexRow::TextEntry start = TexRow::text_none, end = TexRow::text_none;
		int errorRow = err->error_in_line;
		Buffer const * buf = nullptr;
		Impl const * p = this;
		if (err->child_name.empty())
			tie(start, end) = p->texrow.getEntriesFromRow(errorRow);
		else {
			// The error occurred in a child
			for (Buffer const * child : owner_->getDescendants()) {
				string const child_name =
					DocFileName(changeExtension(child->absFileName(), "tex")).
					mangledFileName();
				if (err->child_name != child_name)
					continue;
				tie(start, end) = child->d->texrow.getEntriesFromRow(errorRow);
				if (!TexRow::isNone(start)) {
					buf = this->cloned_buffer_
						? child->d->cloned_buffer_->d->owner_
						: child->d->owner_;
					p = child->d;
					break;
				}
			}
		}
		errorList.push_back(ErrorItem(err->error_desc, err->error_text,
		                              start, end, buf));
	}
}


void Buffer::bufferErrors(TeXErrors const & terr, ErrorList & errorList) const
{
	TeXErrors::Errors::const_iterator err = terr.begin();
	TeXErrors::Errors::const_iterator end = terr.end();

	d->traverseErrors(err, end, errorList);
}


void Buffer::bufferRefs(TeXErrors const & terr, ErrorList & errorList) const
{
	TeXErrors::Errors::const_iterator err = terr.begin_ref();
	TeXErrors::Errors::const_iterator end = terr.end_ref();

	d->traverseErrors(err, end, errorList);
}


void Buffer::updateBuffer(UpdateScope scope, UpdateType utype) const
{
	LBUFERR(!text().paragraphs().empty());

	// Use the master text class also for child documents
	Buffer const * const master = masterBuffer();
	DocumentClass const & textclass = master->params().documentClass();

	docstring_list old_bibfiles;
	// Do this only if we are the top-level Buffer. We also need to account
	// for the case of a previewed child with ignored parent here.
	if (master == this && !d->ignore_parent) {
		textclass.counters().reset(from_ascii("bibitem"));
		reloadBibInfoCache();
		// we will re-read this cache as we go through, but we need
		// to know whether it's changed to know whether we need to
		// update the bibinfo cache.
		old_bibfiles = d->bibfiles_cache_;
		d->bibfiles_cache_.clear();
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
			// See: https://marc.info/?l=lyx-devel&m=138590578911716&w=2
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

	Buffer & cbuf = const_cast<Buffer &>(*this);
	// if we are reloading, then we could have a dangling TOC,
	// in effect. so we need to go ahead and reset, even though
	// we will do so again when we rebuild the TOC later.
	cbuf.tocBackend().reset();

	// do the real work
	ParIterator parit = cbuf.par_iterator_begin();
	if (scope == UpdateMaster)
		clearIncludeList();
	updateBuffer(parit, utype);

	// If this document has siblings, then update the TocBackend later. The
	// reason is to ensure that later siblings are up to date when e.g. the
	// broken or not status of references is computed. The update is called
	// in InsetInclude::addToToc.
	if (master != this)
		return;

	// if the bibfiles changed, the cache of bibinfo is invalid
	docstring_list new_bibfiles = d->bibfiles_cache_;
	// this is a trick to determine whether the two vectors have
	// the same elements.
	sort(new_bibfiles.begin(), new_bibfiles.end());
	sort(old_bibfiles.begin(), old_bibfiles.end());
	if (old_bibfiles != new_bibfiles) {
		LYXERR(Debug::FILES, "Reloading bibinfo cache.");
		invalidateBibinfoCache();
		reloadBibInfoCache();
		// We relied upon the bibinfo cache when recalculating labels. But that
		// cache was invalid, although we didn't find that out until now. So we
		// have to do it all again.
		// That said, the only thing we really need to do is update the citation
		// labels. Nothing else will have changed. So we could create a new 
		// UpdateType that would signal that fact, if we needed to do so.
		parit = cbuf.par_iterator_begin();
		// we will be re-doing the counters and references and such.
		textclass.counters().reset();
		clearReferenceCache();
		// we should not need to do this again?
		// updateMacros();
		updateBuffer(parit, utype);
		// this will already have been done by reloadBibInfoCache();
		// d->bibinfo_cache_valid_ = true;
	}
	else {
		LYXERR(Debug::FILES, "Bibfiles unchanged.");
		// this is also set to true on the other path, by reloadBibInfoCache.
		d->bibinfo_cache_valid_ = true;
	}
	d->cite_labels_valid_ = true;
	/// FIXME: Perf
	clearIncludeList();
	cbuf.tocBackend().update(true, utype);
	if (scope == UpdateMaster)
		cbuf.structureChanged();

	d->need_update = false;
}


static depth_type getDepth(DocIterator const & it)
{
	depth_type depth = 0;
	for (size_t i = 0 ; i < it.depth() ; ++i)
		if (!it[i].inset().inMathed())
			depth += it[i].paragraph().getDepth() + 1;
	// remove 1 since the outer inset does not count
	// we should have at least one non-math inset, so
	// depth should nevery be 0. but maybe it is worth
	// marking this, just in case.
	LATTEST(depth > 0);
	// coverity[INTEGER_OVERFLOW]
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
			return prev_par.layout().name() != par.layout().name();
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
			// • U+2022 BULLET
			itemlabel = char_type(0x2022);
			break;
		case 1:
			// – U+2013 EN DASH
			itemlabel = char_type(0x2013);
			break;
		case 2:
			// ∗ U+2217 ASTERISK OPERATOR
			itemlabel = char_type(0x2217);
			break;
		case 3:
			// · U+00B7 MIDDLE DOT
			itemlabel = char_type(0x00b7);
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
			// fall through
		case 1:
			enumcounter += 'i';
			// fall through
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

		if (needEnumCounterReset(it)) {
			// Increase the parent counter?
			if (layout.stepparentcounter)
				counters.stepParent(enumcounter, utype);
			// Maybe we have to reset the enumeration counter.
			if (!layout.resumecounter)
				counters.reset(enumcounter);
		}
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


void Buffer::updateBuffer(ParIterator & parit, UpdateType utype, bool const deleted) const
{
	pushIncludedBuffer(this);
	// LASSERT: Is it safe to continue here, or should we just return?
	LASSERT(parit.pit() == 0, /**/);

	// Set the position of the text in the buffer to be able
	// to resolve macros in it.
	parit.text()->setMacrocontextPosition(parit);

	depth_type maxdepth = 0;
	pit_type const lastpit = parit.lastpit();
	bool changed = false;
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
			CursorData(parit).recordUndo();
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

		// now the insets
		for (auto const & insit : parit->insetList()) {
			parit.pos() = insit.pos;
			insit.inset->updateBuffer(parit, utype, deleted || parit->isDeleted(insit.pos));
			changed |= insit.inset->isChanged();
		}

		// are there changes in this paragraph?
		changed |= parit->isChanged();
	}

	// set change indicator for the inset (or the cell that the iterator
	// points to, if applicable).
	parit.text()->inset().isChanged(changed);
	popIncludedBuffer();
}


void Buffer::forceUpdate() const
{
	d->need_update = true;
}


bool Buffer::needUpdate() const
{
	return d->need_update;
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
				if (ins && ins->isLetter()) {
					odocstringstream os;
					ins->toString(os);
					char_count_ += os.str().length();
				}
				else if (ins && ins->isSpace())
					++blank_count_;
				else if (ins) {
					pair<int, int> words = ins->isWords();
					char_count_ += words.first;
					word_count_ += words.second;
					inword = false;
				}
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


bool Buffer::areChangesPresent() const
{
	return inset().isChanged();
}


Buffer::ReadStatus Buffer::reload()
{
	setBusy(true);
	// c.f. bug https://www.lyx.org/trac/ticket/6587
	removeAutosaveFile();
	// e.g., read-only status could have changed due to version control
	d->filename.refresh();
	docstring const disp_fn = makeDisplayPath(d->filename.absFileName());

	// clear parent. this will get reset if need be.
	d->setParent(nullptr);
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
	for (auto const & bit : d->children_positions) {
		DocIterator dit = bit.second;
		Buffer * cbuf = const_cast<Buffer *>(bit.first);
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
		cbuf->setParent(nullptr);
		inset_inc->setChildBuffer(nullptr);
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
		setParent(nullptr);
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


void Buffer::Impl::refreshFileMonitor()
{
	if (file_monitor_ && file_monitor_->filename() == filename.absFileName()) {
		file_monitor_->refresh();
		return;
	}

	// The previous file monitor is invalid
	// This also destroys the previous file monitor and all its connections
	file_monitor_ = FileSystemWatcher::monitor(filename);
	// file_monitor_ will be destroyed with *this, so it is not going to call a
	// destroyed object method.
	file_monitor_->connect([this](bool exists) {
			fileExternallyModified(exists);
		});
}


void Buffer::Impl::fileExternallyModified(bool const exists)
{
	// ignore notifications after our own saving operations
	if (checksum_ == filename.checksum()) {
		LYXERR(Debug::FILES, "External modification but "
		       "checksum unchanged: " << filename);
		return;
	}
	// If the file has been deleted, only mark the file as dirty since it is
	// pointless to prompt for reloading. If later a file is moved into this
	// location, then the externally modified warning will appear then.
	if (exists)
			externally_modified_ = true;
	// Update external modification notification.
	// Dirty buffers must be visible at all times.
	if (wa_ && wa_->unhide(owner_))
		wa_->updateTitles();
	else
		// Unable to unhide the buffer (e.g. no GUI or not current View)
		lyx_clean = true;
}


bool Buffer::notifiesExternalModification() const
{
	return d->externally_modified_;
}


void Buffer::clearExternalModification() const
{
	d->externally_modified_ = false;
	if (d->wa_)
		d->wa_->updateTitles();
}


void Buffer::pushIncludedBuffer(Buffer const * buf) const
{
	masterBuffer()->d->include_list_.push_back(buf);
	if (lyxerr.debugging(Debug::FILES)) {
		LYXERR0("Pushed. Stack now:");
		if (masterBuffer()->d->include_list_.empty())
			LYXERR0("EMPTY!");
		else
			for (auto const & b : masterBuffer()->d->include_list_)
				LYXERR0(b->fileName());
	}
}


void Buffer::popIncludedBuffer() const
{
	masterBuffer()->d->include_list_.pop_back();
	if (lyxerr.debugging(Debug::FILES)) {
		LYXERR0("Popped. Stack now:");
		if (masterBuffer()->d->include_list_.empty())
			LYXERR0("EMPTY!");
		else
			for (auto const & b : masterBuffer()->d->include_list_)
				LYXERR0(b->fileName());
	}
}


bool Buffer::isBufferIncluded(Buffer const * buf) const
{
	if (!buf)
		return false;
	if (lyxerr.debugging(Debug::FILES)) {
		LYXERR0("Checking for " << buf->fileName() << ". Stack now:");
		if (masterBuffer()->d->include_list_.empty())
			LYXERR0("EMPTY!");
		else
			for (auto const & b : masterBuffer()->d->include_list_)
				LYXERR0(b->fileName());
	}
	list<Buffer const *> const & blist = masterBuffer()->d->include_list_;
	return find(blist.begin(), blist.end(), buf) != blist.end();
}


void Buffer::clearIncludeList() const
{
	LYXERR(Debug::FILES, "Clearing include list for " << fileName());
	d->include_list_.clear();
}

} // namespace lyx
