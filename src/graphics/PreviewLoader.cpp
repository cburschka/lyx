/**
 * \file PreviewLoader.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "PreviewLoader.h"
#include "PreviewImage.h"
#include "GraphicsCache.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Converter.h"
#include "Encoding.h"
#include "Format.h"
#include "InsetIterator.h"
#include "LaTeXFeatures.h"
#include "LyXRC.h"
#include "output.h"
#include "OutputParams.h"
#include "TexRow.h"
#include "texstream.h"

#include "frontends/Application.h" // hexName

#include "insets/Inset.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/ForkedCalls.h"
#include "support/lstrings.h"

#include "support/bind.h"
#include "support/TempFile.h"

#include <atomic>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>

#include <QTimer>

using namespace std;
using namespace lyx::support;



namespace {

typedef pair<string, FileName> SnippetPair;

// A list of all snippets to be converted to previews
typedef list<string> PendingSnippets;

// Each item in the vector is a pair<snippet, image file name>.
typedef vector<SnippetPair> BitmapFile;


FileName const unique_tex_filename(FileName const & bufferpath)
{
	TempFile tempfile(bufferpath, "lyxpreviewXXXXXX.tex");
	tempfile.setAutoRemove(false);
	return tempfile.name();
}


lyx::Converter const * setConverter(string const & from)
{
	typedef vector<string> FmtList;
	typedef lyx::graphics::Cache GCache;
	FmtList const & loadableFormats = GCache::get().loadableFormats();
	FmtList::const_iterator it = loadableFormats.begin();
	FmtList::const_iterator const end = loadableFormats.end();

	for (; it != end; ++it) {
		string const to = *it;
		if (from == to)
			continue;

		lyx::Converter const * ptr = lyx::theConverters().getConverter(from, to);
		if (ptr)
			return ptr;
	}

	// Show the error only once
	static once_flag flag;
	call_once(flag, [&](){
			LYXERR0("PreviewLoader::startLoading()\n"
			        << "No converter from \"" << from
			        << "\" format has been defined.");
		});
	return 0;
}


void setAscentFractions(vector<double> & ascent_fractions,
			FileName const & metrics_file)
{
	// If all else fails, then the images will have equal ascents and
	// descents.
	vector<double>::iterator it  = ascent_fractions.begin();
	vector<double>::iterator end = ascent_fractions.end();
	fill(it, end, 0.5);

	ifstream in(metrics_file.toFilesystemEncoding().c_str());
	if (!in.good()) {
		LYXERR(lyx::Debug::GRAPHICS, "setAscentFractions(" << metrics_file << ")\n"
			<< "Unable to open file!");
		return;
	}

	bool error = false;

	int snippet_counter = 1;
	while (!in.eof() && it != end) {
		string snippet;
		int id;
		double ascent_fraction;

		in >> snippet >> id >> ascent_fraction;

		if (!in.good())
			// eof after all
			break;

		error = snippet != "Snippet";
		if (error)
			break;

		error = id != snippet_counter;
		if (error)
			break;

		*it = ascent_fraction;

		++snippet_counter;
		++it;
	}

	if (error) {
		LYXERR(lyx::Debug::GRAPHICS, "setAscentFractions(" << metrics_file << ")\n"
			<< "Error reading file!\n");
	}
}


class FindFirst
{
public:
	FindFirst(string const & comp) : comp_(comp) {}
	bool operator()(SnippetPair const & sp) const { return sp.first == comp_; }
private:
	string const comp_;
};


/// Store info on a currently executing, forked process.
class InProgress {
public:
	///
	InProgress() : pid(0) {}
	///
	InProgress(string const & filename_base,
		   PendingSnippets const & pending,
		   string const & to_format);
	/// Remove any files left lying around and kill the forked process.
	void stop() const;

	///
	pid_t pid;
	///
	string command;
	///
	FileName metrics_file;
	///
	BitmapFile snippets;
};

typedef map<pid_t, InProgress>  InProgressProcesses;

typedef InProgressProcesses::value_type InProgressProcess;

} // namespace anon



namespace lyx {
namespace graphics {

class PreviewLoader::Impl : public boost::signals2::trackable {
public:
	///
	Impl(PreviewLoader & p, Buffer const & b);
	/// Stop any InProgress items still executing.
	~Impl();
	///
	PreviewImage const * preview(string const & latex_snippet) const;
	///
	PreviewLoader::Status status(string const & latex_snippet) const;
	///
	void add(string const & latex_snippet);
	///
	void remove(string const & latex_snippet);
	/// \p wait whether to wait for the process to complete or, instead,
	/// to do it in the background.
	void startLoading(bool wait = false);
	///
	void refreshPreviews();

	/// Emit this signal when an image is ready for display.
	boost::signals2::signal<void(PreviewImage const &)> imageReady;

	Buffer const & buffer() const { return buffer_; }

private:
	/// Called by the ForkedCall process that generated the bitmap files.
	void finishedGenerating(pid_t, int);
	///
	void dumpPreamble(otexstream &, OutputParams::FLAVOR) const;
	///
	void dumpData(odocstream &, BitmapFile const &) const;

	/** cache_ allows easy retrieval of already-generated images
	 *  using the LaTeX snippet as the identifier.
	 */
	typedef std::shared_ptr<PreviewImage> PreviewImagePtr;
	///
	typedef map<string, PreviewImagePtr> Cache;
	///
	Cache cache_;

	/** pending_ stores the LaTeX snippets in anticipation of them being
	 *  sent to the converter.
	 */
	PendingSnippets pending_;

	/** in_progress_ stores all forked processes so that we can proceed
	 *  thereafter.
	    The map uses the conversion commands as its identifiers.
	 */
	InProgressProcesses in_progress_;

	///
	PreviewLoader & parent_;
	///
	Buffer const & buffer_;
	///
	mutable int font_scaling_factor_;
	///
	mutable int fg_color_;
	///
	mutable int bg_color_;
	///
	QTimer * delay_refresh_;
	///
	bool finished_generating_;

	/// We don't own this
	static lyx::Converter const * pconverter_;
};


lyx::Converter const * PreviewLoader::Impl::pconverter_;


//
// The public interface, defined in PreviewLoader.h
//

PreviewLoader::PreviewLoader(Buffer const & b)
	: pimpl_(new Impl(*this, b))
{}


PreviewLoader::~PreviewLoader()
{
	delete pimpl_;
}


PreviewImage const * PreviewLoader::preview(string const & latex_snippet) const
{
	return pimpl_->preview(latex_snippet);
}


PreviewLoader::Status PreviewLoader::status(string const & latex_snippet) const
{
	return pimpl_->status(latex_snippet);
}


void PreviewLoader::add(string const & latex_snippet) const
{
	pimpl_->add(latex_snippet);
}


void PreviewLoader::remove(string const & latex_snippet) const
{
	pimpl_->remove(latex_snippet);
}


void PreviewLoader::startLoading(bool wait) const
{
	pimpl_->startLoading(wait);
}


void PreviewLoader::refreshPreviews()
{
	pimpl_->refreshPreviews();
}


boost::signals2::connection PreviewLoader::connect(slot_type const & slot) const
{
	return pimpl_->imageReady.connect(slot);
}


void PreviewLoader::emitSignal(PreviewImage const & pimage) const
{
	pimpl_->imageReady(pimage);
}


Buffer const & PreviewLoader::buffer() const
{
	return pimpl_->buffer();
}

} // namespace graphics
} // namespace lyx


// The details of the Impl
// =======================

namespace {

class IncrementedFileName {
public:
	IncrementedFileName(string const & to_format,
			    string const & filename_base)
		: to_format_(to_format), base_(filename_base), counter_(1)
	{}

	SnippetPair const operator()(string const & snippet)
	{
		ostringstream os;
		os << base_ << counter_++ << '.' << to_format_;
		string const file = os.str();

		return make_pair(snippet, FileName(file));
	}

private:
	string const & to_format_;
	string const & base_;
	int counter_;
};


InProgress::InProgress(string const & filename_base,
		       PendingSnippets const & pending,
		       string const & to_format)
	: pid(0),
	  metrics_file(filename_base + ".metrics"),
	  snippets(pending.size())
{
	PendingSnippets::const_iterator pit  = pending.begin();
	PendingSnippets::const_iterator pend = pending.end();
	BitmapFile::iterator sit = snippets.begin();

	transform(pit, pend, sit,
		       IncrementedFileName(to_format, filename_base));
}


void InProgress::stop() const
{
	if (pid)
		ForkedCallsController::kill(pid, 0);

	if (!metrics_file.empty())
		metrics_file.removeFile();

	BitmapFile::const_iterator vit  = snippets.begin();
	BitmapFile::const_iterator vend = snippets.end();
	for (; vit != vend; ++vit) {
		if (!vit->second.empty())
			vit->second.removeFile();
	}
}

} // namespace anon


namespace lyx {
namespace graphics {

PreviewLoader::Impl::Impl(PreviewLoader & p, Buffer const & b)
	: parent_(p), buffer_(b), finished_generating_(true)
{
	font_scaling_factor_ = int(buffer_.fontScalingFactor());
	if (theApp()) {
		fg_color_ = strtol(theApp()->hexName(foregroundColor()).c_str(), 0, 16);
		bg_color_ = strtol(theApp()->hexName(backgroundColor()).c_str(), 0, 16);
	} else {
		fg_color_ = 0x0;
		bg_color_ = 0xffffff;
	}
	if (!pconverter_)
		pconverter_ = setConverter("lyxpreview");

	delay_refresh_ = new QTimer(&parent_);
	delay_refresh_->setSingleShot(true);
	QObject::connect(delay_refresh_, SIGNAL(timeout()),
	                 &parent_, SLOT(refreshPreviews()));
}


PreviewLoader::Impl::~Impl()
{
	delete delay_refresh_;

	InProgressProcesses::iterator ipit  = in_progress_.begin();
	InProgressProcesses::iterator ipend = in_progress_.end();

	for (; ipit != ipend; ++ipit)
		ipit->second.stop();
}


PreviewImage const *
PreviewLoader::Impl::preview(string const & latex_snippet) const
{
	int fs = int(buffer_.fontScalingFactor());
	int fg = 0x0;
	int bg = 0xffffff;
	if (theApp()) {
		fg = strtol(theApp()->hexName(foregroundColor()).c_str(), 0, 16);
		bg = strtol(theApp()->hexName(backgroundColor()).c_str(), 0, 16);
	}
	if (font_scaling_factor_ != fs || fg_color_ != fg || bg_color_ != bg) {
		// Schedule refresh of all previews on zoom or color changes.
		// The previews are regenerated only after the zoom factor
		// has not been changed for about 1 second.
		fg_color_ = fg;
		bg_color_ = bg;
		delay_refresh_->start(1000);
	}
	// Don't try to access the cache until we are done.
	if (delay_refresh_->isActive() || !finished_generating_)
		return 0;
	Cache::const_iterator it = cache_.find(latex_snippet);
	return (it == cache_.end()) ? 0 : it->second.get();
}


void PreviewLoader::Impl::refreshPreviews()
{
	font_scaling_factor_ = int(buffer_.fontScalingFactor());
	// Reschedule refresh until the previous process completed.
	if (!finished_generating_) {
		delay_refresh_->start(1000);
		return;
	}
	Cache::const_iterator cit = cache_.begin();
	Cache::const_iterator cend = cache_.end();
	while (cit != cend)
		parent_.remove((cit++)->first);
	finished_generating_ = false;
	buffer_.updatePreviews();
}


namespace {

class FindSnippet {
public:
	FindSnippet(string const & s) : snippet_(s) {}
	bool operator()(InProgressProcess const & process) const
	{
		BitmapFile const & snippets = process.second.snippets;
		BitmapFile::const_iterator beg  = snippets.begin();
		BitmapFile::const_iterator end = snippets.end();
		return find_if(beg, end, FindFirst(snippet_)) != end;
	}

private:
	string const snippet_;
};

} // namespace anon

PreviewLoader::Status
PreviewLoader::Impl::status(string const & latex_snippet) const
{
	Cache::const_iterator cit = cache_.find(latex_snippet);
	if (cit != cache_.end())
		return Ready;

	PendingSnippets::const_iterator pit  = pending_.begin();
	PendingSnippets::const_iterator pend = pending_.end();

	pit = find(pit, pend, latex_snippet);
	if (pit != pend)
		return InQueue;

	InProgressProcesses::const_iterator ipit  = in_progress_.begin();
	InProgressProcesses::const_iterator ipend = in_progress_.end();

	ipit = find_if(ipit, ipend, FindSnippet(latex_snippet));
	if (ipit != ipend)
		return Processing;

	return NotFound;
}


void PreviewLoader::Impl::add(string const & latex_snippet)
{
	if (!pconverter_ || status(latex_snippet) != NotFound)
		return;

	string const snippet = trim(latex_snippet);
	if (snippet.empty())
		return;

	LYXERR(Debug::GRAPHICS, "adding snippet:\n" << snippet);

	pending_.push_back(snippet);
}


namespace {

class EraseSnippet {
public:
	EraseSnippet(string const & s) : snippet_(s) {}
	void operator()(InProgressProcess & process)
	{
		BitmapFile & snippets = process.second.snippets;
		BitmapFile::iterator it  = snippets.begin();
		BitmapFile::iterator end = snippets.end();

		it = find_if(it, end, FindFirst(snippet_));
		if (it != end)
			snippets.erase(it, it+1);
	}

private:
	string const & snippet_;
};

} // namespace anon


void PreviewLoader::Impl::remove(string const & latex_snippet)
{
	Cache::iterator cit = cache_.find(latex_snippet);
	if (cit != cache_.end())
		cache_.erase(cit);

	PendingSnippets::iterator pit  = pending_.begin();
	PendingSnippets::iterator pend = pending_.end();

	pending_.erase(std::remove(pit, pend, latex_snippet), pend);

	InProgressProcesses::iterator ipit  = in_progress_.begin();
	InProgressProcesses::iterator ipend = in_progress_.end();

	for_each(ipit, ipend, EraseSnippet(latex_snippet));

	while (ipit != ipend) {
		InProgressProcesses::iterator curr = ipit++;
		if (curr->second.snippets.empty())
			in_progress_.erase(curr);
	}
}


void PreviewLoader::Impl::startLoading(bool wait)
{
	if (pending_.empty() || !pconverter_)
		return;

	// Only start the process off after the buffer is loaded from file.
	if (!buffer_.isFullyLoaded())
		return;

	LYXERR(Debug::GRAPHICS, "PreviewLoader::startLoading()");

	// As used by the LaTeX file and by the resulting image files
	FileName const directory(buffer_.temppath());

	FileName const latexfile = unique_tex_filename(directory);
	string const filename_base = removeExtension(latexfile.absFileName());

	// Create an InProgress instance to place in the map of all
	// such processes if it starts correctly.
	InProgress inprogress(filename_base, pending_, pconverter_->to());

	// clear pending_, so we're ready to start afresh.
	pending_.clear();

	// Output the LaTeX file.
	// we use the encoding of the buffer
	Encoding const & enc = buffer_.params().encoding();
	ofdocstream of;
	try { of.reset(enc.iconvName()); }
	catch (iconv_codecvt_facet_exception const & e) {
		LYXERR0("Caught iconv exception: " << e.what()
			<< "\nUnable to create LaTeX file: " << latexfile);
		return;
	}

	otexstream os(of);
	OutputParams runparams(&enc);
	LaTeXFeatures features(buffer_, buffer_.params(), runparams);

	if (!openFileWrite(of, latexfile))
		return;

	if (!of) {
		LYXERR(Debug::GRAPHICS, "PreviewLoader::startLoading()\n"
					<< "Unable to create LaTeX file\n" << latexfile);
		return;
	}
	of << "\\batchmode\n";

	// Set \jobname of previews to the document name (see bug 9627)
	of << "\\def\\jobname{"
	   << from_utf8(changeExtension(buffer_.latexName(true), ""))
	   << "}\n";

	LYXERR(Debug::LATEX, "Format = " << buffer_.params().getDefaultOutputFormat());
	string latexparam = "";
	bool docformat = !buffer_.params().default_output_format.empty()
			&& buffer_.params().default_output_format != "default";
	// Use LATEX flavor if the document does not specify a specific
	// output format (see bug 9371).
	OutputParams::FLAVOR flavor = docformat
					? buffer_.params().getOutputFlavor()
					: OutputParams::LATEX;
	if (buffer_.params().encoding().package() == Encoding::japanese) {
		latexparam = " --latex=platex";
		flavor = OutputParams::LATEX;
	}
	else if (buffer_.params().useNonTeXFonts) {
		if (flavor == OutputParams::LUATEX)
			latexparam = " --latex=lualatex";
		else {
			flavor = OutputParams::XETEX;
			latexparam = " --latex=xelatex";
		}
	}
	else {
		switch (flavor) {
			case OutputParams::PDFLATEX:
				latexparam = " --latex=pdflatex";
				break;
			case OutputParams::XETEX:
				latexparam = " --latex=xelatex";
				break;
			case OutputParams::LUATEX:
				latexparam = " --latex=lualatex";
				break;
			case OutputParams::DVILUATEX:
				latexparam = " --latex=dvilualatex";
				break;
			default:
				flavor = OutputParams::LATEX;
		}
	}
	dumpPreamble(os, flavor);
	// handle inputenc etc.
	// I think, this is already hadled by dumpPreamble(): Kornel
	// buffer_.params().writeEncodingPreamble(os, features);
	of << "\n\\begin{document}\n";
	dumpData(of, inprogress.snippets);
	of << "\n\\end{document}\n";
	of.close();
	if (of.fail()) {
		LYXERR(Debug::GRAPHICS, "PreviewLoader::startLoading()\n"
					 << "File was not closed properly.");
		return;
	}

	// The conversion command.
	ostringstream cs;
	cs << pconverter_->command()
	   << " " << quoteName(latexfile.toFilesystemEncoding())
	   << " --dpi " << font_scaling_factor_;

	// FIXME XHTML 
	// The colors should be customizable.
	if (!buffer_.isExporting()) {
		ColorCode const fg = PreviewLoader::foregroundColor();
		ColorCode const bg = PreviewLoader::backgroundColor();
		cs << " --fg " << theApp()->hexName(fg) 
		   << " --bg " << theApp()->hexName(bg);
	}

	cs << latexparam;
	if (buffer_.params().bibtex_command != "default")
		cs << " --bibtex=" << quoteName(buffer_.params().bibtex_command);
	else if (buffer_.params().encoding().package() == Encoding::japanese)
		cs << " --bibtex=" << quoteName(lyxrc.jbibtex_command);
	else
		cs << " --bibtex=" << quoteName(lyxrc.bibtex_command);
	if (buffer_.params().bufferFormat() == "lilypond-book")
		cs << " --lilypond";

	string const command = cs.str();

	if (wait) {
		ForkedCall call(buffer_.filePath(), buffer_.layoutPos());
		int ret = call.startScript(ForkedProcess::Wait, command);
		static atomic_int fake((2^20) + 1);
		int pid = fake++;
		inprogress.pid = pid;
		inprogress.command = command;
		in_progress_[pid] = inprogress;
		finishedGenerating(pid, ret);
		return;
	}

	// Initiate the conversion from LaTeX to bitmap images files.
	ForkedCall::SignalTypePtr
		convert_ptr(new ForkedCall::SignalType);
	convert_ptr->connect(bind(&Impl::finishedGenerating, this, _1, _2));

	ForkedCall call(buffer_.filePath());
	int ret = call.startScript(command, convert_ptr);

	if (ret != 0) {
		LYXERR(Debug::GRAPHICS, "PreviewLoader::startLoading()\n"
					<< "Unable to start process\n" << command);
		return;
	}

	// Store the generation process in a list of all such processes
	inprogress.pid = call.pid();
	inprogress.command = command;
	in_progress_[inprogress.pid] = inprogress;
}


double PreviewLoader::displayPixelRatio() const
{
	return buffer().params().display_pixel_ratio;
}

void PreviewLoader::Impl::finishedGenerating(pid_t pid, int retval)
{
	// Paranoia check!
	InProgressProcesses::iterator git = in_progress_.find(pid);
	if (git == in_progress_.end()) {
		lyxerr << "PreviewLoader::finishedGenerating(): unable to find "
			"data for PID " << pid << endl;
		finished_generating_ = true;
		return;
	}

	string const command = git->second.command;
	string const status = retval > 0 ? "failed" : "succeeded";
	LYXERR(Debug::GRAPHICS, "PreviewLoader::finishedInProgress("
				<< retval << "): processing " << status
				<< " for " << command);
	if (retval > 0) {
		in_progress_.erase(git);
		finished_generating_ = true;
		return;
	}

	// Read the metrics file, if it exists
	vector<double> ascent_fractions(git->second.snippets.size());
	setAscentFractions(ascent_fractions, git->second.metrics_file);

	// Add these newly generated bitmap files to the cache and
	// start loading them into LyX.
	BitmapFile::const_iterator it  = git->second.snippets.begin();
	BitmapFile::const_iterator end = git->second.snippets.end();

	list<PreviewImagePtr> newimages;

	int metrics_counter = 0;
	for (; it != end; ++it, ++metrics_counter) {
		string const & snip = it->first;
		FileName const & file = it->second;
		double af = ascent_fractions[metrics_counter];

		// Add the image to the cache only if it's actually present
		// and not empty (an empty image is signaled by af < 0)
		if (af >= 0 && file.isReadableFile()) {
			PreviewImagePtr ptr(new PreviewImage(parent_, snip, file, af));
			cache_[snip] = ptr;

			newimages.push_back(ptr);
		}

	}

	// Remove the item from the list of still-executing processes.
	in_progress_.erase(git);

	// Tell the outside world
	list<PreviewImagePtr>::const_reverse_iterator
		nit  = newimages.rbegin();
	list<PreviewImagePtr>::const_reverse_iterator
		nend = newimages.rend();
	for (; nit != nend; ++nit) {
		imageReady(*nit->get());
	}
	finished_generating_ = true;
}


void PreviewLoader::Impl::dumpPreamble(otexstream & os, OutputParams::FLAVOR flavor) const
{
	// Dump the preamble only.
	LYXERR(Debug::LATEX, "dumpPreamble, flavor == " << flavor);
	OutputParams runparams(&buffer_.params().encoding());
	runparams.flavor = flavor;
	runparams.nice = true;
	runparams.moving_arg = true;
	runparams.free_spacing = true;
	runparams.is_child = buffer_.parent();
	buffer_.writeLaTeXSource(os, buffer_.filePath(), runparams, Buffer::OnlyPreamble);

	// FIXME! This is a HACK! The proper fix is to control the 'true'
	// passed to WriteStream below:
	// int InsetMathNest::latex(Buffer const &, odocstream & os,
	//                          OutputParams const & runparams) const
	// {
	//	WriteStream wi(os, runparams.moving_arg, true);
	//	par_->write(wi);
	//	return wi.line();
	// }
	os << "\n"
	   << "\\def\\lyxlock{}\n"
	   << "\n";

	// All equation labels appear as "(#)" + preview.sty's rendering of
	// the label name
	if (lyxrc.preview_hashed_labels)
		os << "\\renewcommand{\\theequation}{\\#}\n";

	// Use the preview style file to ensure that each snippet appears on a
	// fresh page.
	// Also support PDF output (automatically generated e.g. when
	// \usepackage[pdftex]{hyperref} is used and XeTeX.
	os << "\n"
	   << "\\usepackage[active,delayed,showlabels,lyx]{preview}\n"
	   << "\n";
}


void PreviewLoader::Impl::dumpData(odocstream & os,
				   BitmapFile const & vec) const
{
	if (vec.empty())
		return;

	BitmapFile::const_iterator it  = vec.begin();
	BitmapFile::const_iterator end = vec.end();

	for (; it != end; ++it) {
		// FIXME UNICODE
		os << "\\begin{preview}\n"
		   << from_utf8(it->first)
		   << "\n\\end{preview}\n\n";
	}
}

} // namespace graphics
} // namespace lyx

#include "moc_PreviewLoader.cpp"
