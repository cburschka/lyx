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


#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "RadioButtonGroup.h"

#include "debug.h" // for lyxerr

#include <functional>
#include <algorithm>
#include <iterator>
using std::find_if;
using std::endl;

void RadioButtonGroup::registerRadioButton(FL_OBJECT *button, int value)
{
#if 0
    bvec.push_back(button);
    vvec.push_back(value);
#endif
    map.push_back( ButtonValuePair(button, value) );
}


void RadioButtonGroup::reset()
{
#if 0
    bvec.clear();
    vvec.clear();
#endif
    map.clear(); 
}

    // Functor to help us in our work, we should try to find how to achieve
    // this with only STL predicates, but its easier to write this than to
    // dig. If you can find the equivalent STL predicate combination, let me 
    // know.
    //
    // The idea is to take a pair and a value and return true when the second
    // element in the pair equals the value.
    template <typename T>
    struct equal_to_second_in_pair {
        typedef bool                    result_type;
        typedef T              first_argument_type;
        typedef typename T::second_type second_argument_type;

        bool operator() (
           pair<typename T::first_type, typename T::second_type> const & left,
           typename T::second_type const & right) const
        {
            return left.second == right;
        }
    };

void RadioButtonGroup::setButton(int value)
{
#if 0
    ValueVector::iterator vit =
        find_if(vvec.begin(), vvec.end(),
            bind2nd(equal_to<int>(), value));
    
    if (vit == vvec.end()) {
        lyxerr << "BUG: Requested value in RadioButtonGroup doesn't exists"
            << endl;
        return;
    }

    unsigned n = std::distance(vvec.begin(), vit);

    fl_set_button(bvec[n], 1);
#endif


    ButtonValueMap::const_iterator it = 
#if 0
        find_if(map.begin(), map.end(),
                bind2nd(equal_to_second_in_pair<ButtonValuePair>(),
                    value));
#else
        std::find_if(map.begin(), map.end(),
                std::compose1(
                    std::bind2nd(std::equal_to<int>(), value)
                    ,
                    std::select2nd<ButtonValuePair>()
                )
        );
#endif
    // If we found nothing, report it and return
    if (it == map.end()) {
        lyxerr << "BUG: Requested value in RadioButtonGroup doesn't exists"
            << endl;
    } else {
        fl_set_button((*it).first, 1);
    }
    
}

template<typename T>
struct is_set_button {
    bool operator() (T const & item) const
    {
        return fl_get_button( (item).first );
    }
};

int  RadioButtonGroup::getButton()
{
#if 0
    ButtonVector::const_iterator bit = bvec.begin();
    ValueVector::const_iterator vit = vvec.begin();

    while (bit != bvec.end()) {
        if (fl_get_button(*bit))
            return *vit;

        bit++;
        vit++;
    }

    return 0;
#endif

    // Find the first button that is active
    ButtonValueMap::iterator it =
        find_if(map.begin(), map.end(),
                is_set_button<ButtonValuePair>() );

    // If such a button was found, return its value.
    if (it != map.end()) {
        return (*it).second;
    }

    lyxerr << "BUG: No radio button found to be active." << endl;

    // Else return 0.
    return 0;
}

