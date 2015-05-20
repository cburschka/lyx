# This file is part of lyx.
#
# The module comes from the enblend project, slightly modified.
#
# Check if compiler supports C++11 features
# and which compiler switches are necessary
# CXX11_FLAG : contains the necessary compiler flag

#
# Copyright (c) 2013 Thomas Modes <tmodes@@users.sourceforge.net>
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  1. Redistributions of source code must retain the copyright
#         notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the copyright
#         notice, this list of conditions and the following disclaimer in the
#         documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products
#         derived from this software without specific prior written permission.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with lyx; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

INCLUDE(CheckCXXSourceCompiles)
INCLUDE(FindPackageHandleStandardArgs)

if (CMAKE_CXX_COMPILER_ID MATCHES "^[cC]lang$")
  set(CXX11_FLAG_CANDIDATES "--std=c++11")
else()
  set(CXX11_FLAG_CANDIDATES
    "--std=gnu++11"
    "--std=c++11"
    "--std=gnu++0x"
  )
endif()

# sample openmp source code to test
SET(CXX11_TEST_SOURCE 
"
template <typename T>
struct check
{
    static_assert(sizeof(int) <= sizeof(T), \"not big enough\");
};

typedef check<check<bool>> right_angle_brackets;

class TestDeleted
{
public:
    TestDeleted() = delete;
};


int a;
decltype(a) b;

typedef check<int> check_type;
check_type c;
check_type&& cr = static_cast<check_type&&>(c);

auto d = a;

int main() {
  return 0;
};
")

# check c compiler
set(SAFE_CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET})
set(CMAKE_REQUIRED_QUIET ON)
FOREACH(FLAG ${CXX11_FLAG_CANDIDATES})
  SET(SAFE_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
  SET(CMAKE_REQUIRED_FLAGS "${FLAG}")
  UNSET(CXX11_FLAG_DETECTED CACHE)
  CHECK_CXX_SOURCE_COMPILES("${CXX11_TEST_SOURCE}" CXX11_FLAG_DETECTED)
  SET(CMAKE_REQUIRED_FLAGS "${SAFE_CMAKE_REQUIRED_FLAGS}")
  IF(CXX11_FLAG_DETECTED)
    SET(CXX11_FLAG "${FLAG}")
    message(STATUS "CXX11_FLAG_DETECTED = \"${FLAG}\"")
    set(LYX_USE_CXX11 1)
    BREAK()
  ENDIF()
ENDFOREACH()
set(CMAKE_REQUIRED_QUIET ${SAFE_CMAKE_REQUIRED_QUIET})

# handle the standard arguments for find_package
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CXX11Compiler DEFAULT_MSG CXX11_FLAG)

MARK_AS_ADVANCED(CXX11_FLAG)
