// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlCitation.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef HELPERFUNCS_H
#define HELPERFUNCS_H

#ifdef __GNUG__
#pragma interface
#endif

/** Functions to convert a string to/from a vector. */

///
string const
getStringFromVector(std::vector<string> const & vec, string const & delim=",");

///
std::vector<string> const
getVectorFromString(string const & str, string const & delim=",");

/** Functions to extract vectors of the first and second elems from a
    vector<pair<A,B> >
*/

///
template <class A, class B>
std::vector<A> const getFirst(std::vector<std::pair<A,B> > const & pairVec)
{
	typedef std::vector<std::pair<A,B> > PV;

	std::vector<A> first(pairVec.size());

	for (PV::size_type i = 0; i < pairVec.size(); ++i) {
		first[i] = pairVec[i].first;
	}

	return first;
}
///
template <class A, class B>
std::vector<B> const getSecond(std::vector<std::pair<A,B> > const & pairVec)
{
	typedef std::vector<std::pair<A,B> > PV;

	std::vector<B> second(pairVec.size());

	for (PV::size_type i = 0; i < pairVec.size(); ++i) {
		second[i] = pairVec[i].second;
	}

	return second;
}


template<class Pair>
struct firster {
	typedef typename Pair::first_type first_type;
	first_type const & operator()(Pair const & p) { return p.first; }
};

template<class Pair>
struct seconder {
	typedef typename Pair::second_type second_type;
	second_type const & operator()(Pair const & p) { return p.second; }
};

template<class Pair>
typename Pair::first_type const getFirst(std::vector<Pair> const & pr)
{
	std::vector<typename Pair::first_type> tmp(pr.size);
	std::transform(pr.begin(), pr.end(), tmp.begin(), firster<Pair>());
	return tmp;
}

template<class Pair>
typename Pair::second_type const getSecond(std::vector<Pair> const & pr)
{
	std::vector<typename Pair::second_type> tmp(pr.size);
	std::transform(pr.begin(), pr.end(), tmp.begin(), seconder<Pair>());
	return tmp;
}



#endif // HELPERFUNCS_H

