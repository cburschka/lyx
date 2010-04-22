// -*- C++ -*-
/**
 * \file numpunct_lyx_char_type.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_NUMPUNCT_LYX_CHAR_TYPE_H
#define LYX_NUMPUNCT_LYX_CHAR_TYPE_H


#include <locale>


namespace std
{

	template<>
	class numpunct<lyx::char_type> : public locale::facet
	{
	public:

		typedef lyx::char_type char_type;
		typedef basic_string<lyx::char_type> string_type;


		static locale::id			id;

		explicit numpunct(size_t __refs = 0) : chared(__refs)
		{}

		char_type decimal_point() const
		{ return chared.decimal_point(); }

		char_type thousands_sep() const
		{ return chared.thousands_sep(); }

		string grouping() const
		{ return chared.grouping(); }

		string_type truename() const
		{ return lyx::from_ascii(chared.truename()); }

		string_type falsename() const
		{ return lyx::from_ascii(chared.falsename()); }


	protected:
		virtual ~numpunct();


	private:
		numpunct<char> chared;

	};



}
#endif
