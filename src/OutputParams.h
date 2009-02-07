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

#include <string>

#include "support/types.h"
#include <boost/shared_ptr.hpp>
#include "Changes.h"


namespace lyx {


class Encoding;
class ExportData;
class Font;


class OutputParams {
public:
	enum FLAVOR {
		LATEX,
		PDFLATEX,
		XML
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

	/** The latex that we export depends occasionally on what is to
	    compile the file.
	*/
	FLAVOR flavor;

	/** Are we to write a 'nice' LaTeX file or not.
	    This esentially seems to mean whether InsetInclude, InsetGraphics
	    and InsetExternal should add the absolute path to any external
	    files or not.
	*/
	bool nice;

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

	/** the font at the point where the inset is
	 */
	Font const * local_font;

	/** Document language babel name
	 */
	mutable std::string document_language;

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

	/** Are we using japanese (pLaTeX)?
	*/
	bool use_japanese;

	/** Line length to use with plaintext export.
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
	boost::shared_ptr<ExportData> exportdata;

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
	pit_type par_begin;

	/** allow output of only part of the top-level paragraphs
	 *  par_end: par_end-1 is the ending paragraph
	 *  if par_begin=par_end, output all paragraphs
	 */
	pit_type par_end;

	/// is this the last paragraph in the current buffer/inset?
	bool isLastPar;

	/** whether or not do actual file copying and image conversion
	 *  This mode will be used to preview the source code
	 */
	bool dryrun;
	/// Should we output verbatim or escape LaTeX's special chars?
	bool verbatim;
};


} // namespace lyx

#endif // NOT OUTPUTPARAMS_H
