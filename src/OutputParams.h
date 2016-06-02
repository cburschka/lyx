// -*- C++ -*-
/**
 * \file OutputParams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef OUTPUTPARAMS_H
#define OUTPUTPARAMS_H


#include "Changes.h"

#include <memory>


namespace lyx {


class Encoding;
class ExportData;
class Font;
class Language;

class OutputParams {
public:
	enum FLAVOR {
		DVILUATEX,
		LATEX,
		LUATEX,
		PDFLATEX,
		XETEX,
		XML,
		HTML,
		TEXT,
		LYX
	};

	enum MathFlavor {
		NotApplicable,
		MathAsMathML,
		MathAsHTML,
		MathAsImages,
		MathAsLaTeX
	};

	enum TableCell {
		NO,
		PLAIN,
		ALIGNED
	};

	enum Float {
		NONFLOAT,
		MAINFLOAT,
		SUBFLOAT
	};

	OutputParams(Encoding const *);
	~OutputParams();

	/** The file that we export depends occasionally on what is to
	    compile the file.
	*/
	FLAVOR flavor;
	/// is it some flavor of LaTeX?
	bool isLaTeX() const;
	/// does this flavour support full unicode?
	bool isFullUnicode() const;

	/// Same, but for math output, which only matter is XHTML output.
	MathFlavor math_flavor;

	/** Are we to write a 'nice' LaTeX file or not.
	    This esentially seems to mean whether InsetInclude, InsetGraphics
	    and InsetExternal should add the absolute path to any external
	    files or not.
	*/
	bool nice;

	/** Is this a real child (i.e., compiled as a child)?
	    This depends on wherefrom we export the buffer. Even children
	    that have a master can be compiled standalone.
	*/
	mutable bool is_child;

	/** moving_arg == true means that the environment in which the inset
	    is typeset is a moving argument. The inset should take care about
	    fragile commands by preceding the latex with \\protect.
	*/
	bool moving_arg;

	/** intitle == true means that the environment in which the
	    inset is typeset is part of a title (before a \\maketitle).
	    Footnotes in such environments have moving arguments.
	*/
	bool intitle;

	/** inulemcmd > 0 means that the environment in which the
	    inset is typeset is part of a ulem command (\uline, \uuline,
	    \uwave, or \sout). Insets that output latex commands relying
	    on local assignments (such as \cite) should enclose such
	    commands in \mbox{} in order to avoid breakage.
	*/
	mutable int inulemcmd;

	/** the font at the point where the inset is
	 */
	Font const * local_font;

	/** Document language babel name
	 */
	std::string document_language;

	/** The master language. Non-null only for child documents.
	    Note that this is not the language of the top level master, but
	    of the direct parent for nested includes.
	 */
	mutable Language const * master_language;

	/** Current stream encoding. Only used for LaTeX.
	    This must be set to the document encoding (via the constructor)
	    before output starts. Afterwards it must be kept up to date for
	    each single character (\sa Paragraph::latex).
	    This does also mean that you need to set it back if you use a
	    copy (e.g. in insets): \code
	    int InsetFoo::latex(..., OutputParams const & runparams_in) const
	    {
		OutputParams runparams(runparams_in);
		runparams.inComment = true;
		...
		InsetBla::latex(..., runparams);
		...
		runparams_in.encoding = runparams.encoding;
	    }
	    \endcode
	 */
	mutable Encoding const * encoding;

	/** free_spacing == true means that the inset is in a free-spacing
	    paragraph.
	*/
	bool free_spacing;

	/** This var is set by the return value from BufferParams::writeLaTeX
	*/
	bool use_babel;

	/** Do we use polyglossia (instead of babel)?
	*/
	bool use_polyglossia;

	/** Are we generating multiple indices?
	*/
	bool use_indices;

	/** Are we using japanese (pLaTeX)?
	*/
	bool use_japanese;

	/** Customized bibtex_command
	*/
	std::string bibtex_command;

	/** Customized index_command
	*/
	std::string index_command;

	/** Line length to use with plaintext or LaTeX export.
	*/
	size_type linelen;

	/** The depth of the current paragraph, set for plaintext
	 *  export and used by InsetTabular
	 */
	int depth;

	/** Export data filled in by the latex(), docbook() etc methods.
	    This is a hack: Make it possible to add stuff to constant
	    OutputParams instances.
	*/
	std::shared_ptr<ExportData> exportdata;

	/** Whether we are inside a comment inset. Insets that are including
	 *  external files like InsetGraphics, InsetInclude and InsetExternal
	 *  may only write the usual output and must not attempt to do
	 *  something with the included files (e.g. copying, converting)
	 *  if this flag is true, since they may not exist.
	 */
	bool inComment;

	/** Whether we are in a table cell.
	 *  For newline, it matters whether its content is aligned or not.
         */
	TableCell inTableCell;

	/** Whether we are inside a float or subfloat.
	 *  Needed for subfloat detection on the command line.
	 */
	Float inFloat;

	/** Whether we are inside an index inset.
	 *  ERT needs to know this, due to the active chars.
	 */
	bool inIndexEntry;

	/** Whether we are inside an IPA inset.
	 *  Needed for proper IPA output.
	 */
	bool inIPA;

	/** Whether we are inside an inset that is logically deleted.
	 *  A value > 0 indicates a deleted inset.
         */
	int inDeletedInset;

	/** The change information of the outermost logically deleted inset.
	 *  changeOfDeletedInset shall only be evaluated if inDeletedInset > 0.
         */
	Change changeOfDeletedInset;

	/** allow output of only part of the top-level paragraphs
	 *  par_begin: beginning paragraph
	 */
	mutable pit_type par_begin;

	/** allow output of only part of the top-level paragraphs
	 *  par_end: par_end-1 is the ending paragraph
	 *  if par_begin=par_end, output all paragraphs
	 */
	mutable pit_type par_end;

	/// Id of the last paragraph before an inset
	mutable int lastid;

	/// Last position in the last paragraph before an inset
	mutable int lastpos;

	/// is this the last paragraph in the current buffer/inset?
	bool isLastPar;


	/** whether or not do actual file copying and image conversion
	 *  This mode will be used to preview the source code
	 */
	bool dryrun;

	/// whether to display output errors or not
	bool silent;

	/// Should we output verbatim or escape LaTeX's special chars?
	bool pass_thru;

	/// Should we output verbatim specific chars?
	docstring pass_thru_chars;

	/// Should we output captions?
	bool html_disable_captions;

	/// Are we already in a paragraph?
	bool html_in_par;

	/// Does the present context even permit paragraphs?
	bool html_make_pars;

	/// Are we generating this material for inclusion in a TOC-like entity?
	bool for_toc;

	/// Are we generating this material for inclusion in a tooltip?
	bool for_tooltip;

	/// Are we generating this material for use by advanced search?
	bool for_search;

	/// Are we generating this material for instant preview?
	bool for_preview;

	/// Include all children notwithstanding the use of \includeonly
	bool includeall;

	/// Explicit output folder, if any is desired
	std::string export_folder;
};


} // namespace lyx

#endif // NOT OUTPUTPARAMS_H
