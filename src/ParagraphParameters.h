// -*- C++ -*-
/**
 * \file ParagraphParameters.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARAGRAPHPARAMETERS_H
#define PARAGRAPHPARAMETERS_H

#include "layout.h"
#include "ShareContainer.h"

#include "support/types.h"

#include <iosfwd>
#include <string>

class BufferView;
class LyXLength;
class LyXLex;
class Paragraph;
class ParameterStruct;
class Spacing;

///
class ParagraphParameters {
public:
	///
	ParagraphParameters();
	///
	void clear();
	///
	bool sameLayout(ParagraphParameters const &) const;
	///
	Spacing const & spacing() const;
	///
	void spacing(Spacing const &);
	///
	bool noindent() const;
	///
	void noindent(bool);
	///
	LyXAlignment align() const;
	///
	void align(LyXAlignment);
	///
	typedef lyx::depth_type depth_type;
	///
	depth_type depth() const;
	///
	void depth(depth_type);
	///
	bool startOfAppendix() const;
	///
	void startOfAppendix(bool);
	///
	bool appendix() const;
	///
	void appendix(bool);
	///
	std::string const & labelString() const;
	///
	void labelString(std::string const &);
	///
	std::string const & labelWidthString() const;
	///
	void labelWidthString(std::string const &);
	///
	LyXLength const & leftIndent() const;
	///
	void leftIndent(LyXLength const &);

	/// read the parameters from a lex
	void read(LyXLex & lex);

	/// write out the parameters to a stream
	void write(std::ostream & os) const;

private:
	///
	void set_from_struct(ParameterStruct const &);
	///
	boost::shared_ptr<ParameterStruct> param;
	///
	static ShareContainer<ParameterStruct> container;
};


/** Generate a string \param data from \param par's ParagraphParameters.
    The function also generates some additional info needed by the
    Paragraph dialog.
 */
void params2string(Paragraph const & par, std::string & data);

/** Given \param data, an encoding of the ParagraphParameters generated
    in the Paragraph dialog, this function sets the current paragraph
    appropriately.
 */
void setParagraphParams(BufferView & bv, std::string const & data);

#endif
