// -*- C++ -*-
#ifndef LYX_BOOST_FORMAT_H
#define LYX_BOOST_FORMAT_H

// Only include this if it is possible to use
// Boost.Format at all.
#if USE_BOOST_FORMAT

#include <boost/format.hpp>

// Add explicit instantion for g++ compilers
#ifdef __GNUG__

namespace boost
{

extern
template basic_format<char>;

extern template
std::ostream &
operator<< <char, std::char_traits<char> >(std::ostream &,
					   boost::basic_format<char> const &);

namespace io
{

extern template
std::string
str<char, std::char_traits<char> >(boost::basic_format<char> const &);

namespace detail
{

extern template
bool parse_printf_directive<char, std::char_traits<char> >
(std::string const &, std::string::size_type*,
 format_item<char, std::char_traits<char> >*,
 std::ios &, unsigned char);

extern template
void distribute<char, std::char_traits<char>, std::string const &>
(boost::basic_format<char>&, std::string const &);

extern template
boost::basic_format<char> &
feed<char, std::char_traits<char>,
     std::string const &>(boost::basic_format<char> &,
			  std::string const &);

} // namespace detail

} // namespace io

} // namespace boost

#endif // __GNUG__

#endif // USE_BOOST_FORMAT

#endif // LYX_BOOST_FORMAT_H
