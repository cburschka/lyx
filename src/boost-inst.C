#include <config.h>

#include "BoostFormat.h"

#if USE_BOOST_FORMAT

namespace boost
{

using boost::io::out_of_range_bit;

template class basic_format<char>;

template
std::ostream &
operator<< <char, std::char_traits<char> >(std::ostream &,
					   boost::basic_format<char> const &);

namespace io
{

template
std::string
str<char, std::char_traits<char> >(boost::basic_format<char> const &);

namespace detail
{

template
bool parse_printf_directive<char, std::char_traits<char> >
(std::string const &, std::string::size_type*,
 format_item<char, std::char_traits<char> >*,
 std::ios &, unsigned char);

template
void distribute<char, std::char_traits<char>, std::string const &>
(boost::basic_format<char> &, std::string const&);

template
boost::basic_format<char> &
feed<char, std::char_traits<char>,
     std::string const &>(boost::basic_format<char> &,
			  std::string const &);

} // namespace detail

} // namespace io

} // namespace boost

#endif // USE_BOOST_FORMAT
