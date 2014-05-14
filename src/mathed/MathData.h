// -*- C++ -*-
/**
 * \file MathData.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 * \author Lars Gullik Bjønnes
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_DATA_H
#define MATH_DATA_H

#include "Dimension.h"
#include "MathAtom.h"

#include "OutputEnums.h"

#include "support/strfwd.h"

#include <cstddef>
#include <vector>


namespace lyx {

class Buffer;
class BufferView;
class Cursor;
class DocIterator;
class LaTeXFeatures;
class ReplaceData;
class MacroContext;
class MathMacro;
class MetricsInfo;
class PainterInfo;
class ParIterator;
class TextMetricsInfo;
class TextPainter;


class MathData : private std::vector<MathAtom> {
public:
	/// re-use inhertited stuff
	typedef std::vector<MathAtom> base_type;
	using base_type::const_iterator;
	using base_type::iterator;
	using base_type::size_type;
	using base_type::difference_type;
	using base_type::size;
	using base_type::empty;
	using base_type::clear;
	using base_type::begin;
	using base_type::end;
	using base_type::push_back;
	using base_type::pop_back;
	using base_type::back;
	using base_type::front;
	///
	typedef size_type idx_type;
	typedef size_type pos_type;

public:
	///
	MathData(Buffer * buf = 0) : buffer_(buf) {}
	///
	MathData(Buffer * buf, const_iterator from, const_iterator to);
	///
	Buffer * buffer() { return buffer_; }
	///
	Buffer const * buffer() const { return buffer_; }
	///
	void append(MathData const & ar);

	/// inserts single atom at position pos
	void insert(size_type pos, MathAtom const & at);
	/// inserts multiple atoms at position pos
	void insert(size_type pos, MathData const & ar);

	/// erase range from pos1 to pos2
	void erase(iterator pos1, iterator pos2);
	/// erase single atom
	void erase(iterator pos);
	/// erase range from pos1 to pos2
	void erase(size_type pos1, size_type pos2);
	/// erase single atom
	void erase(size_type pos);

	///
	void dump() const;
	///
	void dump2() const;
	///
	void replace(ReplaceData &);
	///
	void substitute(MathData const & m);

	/// looks for exact match
	bool match(MathData const & ar) const;
	/// looks for inclusion match starting at pos
	bool matchpart(MathData const & ar, pos_type pos) const;
	/// looks for containment, return == size mean not found
	size_type find(MathData const & ar) const;
	/// looks for containment, return == size mean not found
	size_type find_last(MathData const & ar) const;
	///
	bool contains(MathData const & ar) const;
	///
	void validate(LaTeXFeatures &) const;

	/// checked write access
	MathAtom & operator[](pos_type);
	/// checked read access
	MathAtom const & operator[](pos_type) const;
	/// rebuild cached metrics information
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	Dimension const & dimension(BufferView const &) const;

	/// redraw cell using cache metrics information
	void draw(PainterInfo & pi, int x, int y) const;
	/// rebuild cached metrics information
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	/// redraw cell using cache metrics information
	void drawT(TextPainter & pi, int x, int y) const;
	/// mark cell for re-drawing
	void touch() const;

	/// access to cached x coordinate of last drawing
	int xo(BufferView const & bv) const;
	/// access to cached y coordinate of last drawing
	int yo(BufferView const & bv) const;
	/// access to cached x coordinate of mid point of last drawing
	int xm(BufferView const & bv) const;
	/// access to cached y coordinate of mid point of last drawing
	int ym(BufferView const & bv) const;
	/// write access to coordinate;
	void setXY(BufferView & bv, int x, int y) const;
	/// returns x coordinate of given position in the array
	int pos2x(BufferView const * bv, size_type pos) const;
	/// returns position of given x coordinate
	int pos2x(BufferView const * bv, size_type pos, int glue) const;
	/// returns position of given x coordinate
	size_type x2pos(BufferView const * bv, int targetx) const;
	/// returns position of given x coordinate starting from a certain pos
	size_type x2pos(BufferView const * bv, int targetx, int glue) const;
	/// returns distance of this cell to the point given by x and y
	// assumes valid position and size cache
	int dist(BufferView const & bv, int x, int y) const;

	/// minimum ascent offset for superscript
	int minasc() const { return minasc_; }
	/// minimum descent offset for subscript
	int mindes() const { return mindes_; }
	/// level above/below which super/subscript should extend
	int slevel() const { return slevel_; }
	/// additional super/subscript shift
	int sshift() const { return sshift_; }
	/// superscript kerning
	int kerning(BufferView const *) const { return kerning_; }
	///
	void swap(MathData & ar) { base_type::swap(ar); }

	/// attach/detach arguments to macros, updating the cur to
	/// stay visually at the same position (cur==0 is allowed)
	void updateMacros(Cursor * cur, MacroContext const & mc, UpdateType);
	///
	void updateBuffer(ParIterator const &, UpdateType);

protected:
	/// cached values for super/subscript placement
	mutable int minasc_;
	mutable int mindes_;
	mutable int slevel_;
	mutable int sshift_;
	mutable int kerning_;
	Buffer * buffer_;

private:
	/// is this an exact match at this position?
	bool find1(MathData const & ar, size_type pos) const;

	///
	void detachMacroParameters(DocIterator * dit, const size_type macroPos);
	///
	void attachMacroParameters(Cursor * cur, const size_type macroPos, 
		const size_type macroNumArgs, const int macroOptionals,
		const bool fromInitToNormalMode, const bool interactiveInit,
		const size_t appetite);
	///
	void collectOptionalParameters(Cursor * cur, 
		const size_type numOptionalParams, std::vector<MathData> & params, 
		size_t & pos, MathAtom & scriptToPutAround,
		const pos_type macroPos, const int thisPos, const int thisSlice);
	///
	void collectParameters(Cursor * cur, 
		const size_type numParams, std::vector<MathData> & params, 
		size_t & pos, MathAtom & scriptToPutAround,
		const pos_type macroPos, const int thisPos, const int thisSlice,
		const size_t appetite);
};

///
std::ostream & operator<<(std::ostream & os, MathData const & ar);
///
odocstream & operator<<(odocstream & os, MathData const & ar);


} // namespace lyx

#endif
