// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 *          This file Copyright 2000 Baruch Even
 * ================================================= */

#ifndef TRANSLATOR_H
#define TRANSLATOR_H


#include <vector>
#include <utility>
#include <algorithm>
#include <functional>

// Functors used in the template.
template<typename T1, typename T2>
class equal_1st_in_pair {
public:
    equal_1st_in_pair(T1 const & value) : value_(value) {}

    typedef std::pair<T1, T2> pair_type;
    bool operator() (pair_type p) const {
        return p.first == value_;
    }
private:
    T1 const & value_;
};

template<typename T1, typename T2>
class equal_2nd_in_pair {
public:
    equal_2nd_in_pair(T2 const & value) : value_(value) {}

    typedef std::pair<T1, T2> pair_type;
    bool operator() (pair_type p) const {
        return p.second == value_;
    }
private:
    T2 const & value_;
};

/** This class template is used to translate between two elements, specifically
 * it was worked out to translate between an enum and strings when reading
 * the lyx file.
 *
 * The two template arguments should be of different types.
 */

template<typename T1, typename T2>
class Translator {
public:
    typedef T1 first_argument_type;
    typedef T2 second_argument_type;

    /// c-tor.
    Translator(T1 const & t1, T2 const & t2) 
        : default_t1(t1), default_t2(t2) 
        {}
    /// d-tor. Not virtual since it's not going to be inherited.
    ~Translator() {}

    /// Add a mapping to the translator.
    void addPair(T1 const & first, T2 const & second) {
        map.push_back(MapPair(first, second));
    }

    /// Find the mapping for the first argument
    T2 const & find(T1 const & first) const {
#ifdef ENABLE_ASSERTIONS
        Assert( ! map.empty());
#endif

        // For explanation see the next find() function.
        Map::const_iterator it =
            std::find_if(map.begin(), map.end(),
                    equal_1st_in_pair<T1, T2>(first)
                        );

        if (it != map.end())
            return (*it).second;
        else {
            return default_t2;
        }
    }

    /// Find the mapping for the second argument
    T1 const & find(T2 const & second) const {
#ifdef ENABLE_ASSERTIONS
        Assert( ! map.empty());
#endif

        // The idea is as follows:
        // find_if() will try to compare the data in the vector with the value.
        // The vector is made of pairs and the value has the type of the
        // second part of the pair. 
        // We thus give find_if() an equal_to functor and assign to its second
        // post the value we want to compare. We now compose the equal_to 
        // functor with the select2nd functor to take only the second value
        // of the pair to be compared.
        //
        // We can depict it as follows:
        // equal_to( select2nd(pair) , second)
        Map::const_iterator it =
            std::find_if(map.begin(), map.end(),
                    equal_2nd_in_pair<T1, T2>(second)
                        );

        if (it != map.end())
            return (*it).first;
        else {
            return default_t1;
        }
    }

private:
    typedef std::pair<T1, T2> MapPair;
    typedef std::vector<MapPair> Map;

    Map map;

    const T1 default_t1;
    const T2 default_t2;

};

#endif
