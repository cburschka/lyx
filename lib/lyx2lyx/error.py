# This file is part of lyx2lyx
# Copyright (C) 2002 José Matos <jamatos@lyx.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

class Error:
    invalid_file = "Invalid LyX file\n"
    invalid_format = "Invalid LyX format\n"
    format_not_supported = "Format not supported\n"
    same_format = "No convertion because start and ending formats are the same\n"
    newer_format = "Starting format is newer than end format\n"

class Warning:
    dont_match = "Proposed and input file formats do not match"

error = Error()
warning = Warning()
