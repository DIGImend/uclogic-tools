#
# Copyright (C) 2013 Nikolai Kondrashov
#
# This file is part of uclogic-tools.
#
# Usbhid-dump is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Usbhid-dump is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with usbhid-dump; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([uclogic-tools], [3])
AM_INIT_AUTOMAKE([1.9 -Wall foreign])
AM_MAINTAINER_MODE
AC_CONFIG_HEADER([config.h])

# Check for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Check for libraries.
PKG_CHECK_MODULES(LIBUSB, libusb-1.0 >= 1.0.0)
CFLAGS="$CFLAGS $LIBUSB_CFLAGS"
LIBS="$LIBS $LIBUSB_LIBS"

# Check for features
AC_ARG_ENABLE(
    debug,
    AS_HELP_STRING([--enable-debug], [enable debugging features]),
    [], [enable_debug="no"])

# Output features to preprocessor and compiler
if test "$enable_debug" = "yes"; then
    CPPFLAGS="$CPPFLAGS -UNDEBUG"
    CFLAGS="$CFLAGS -Wall -Wextra -Werror -g -O0"
fi

# Check for library functions.
AC_CHECK_FUNCS(libusb_strerror)

# Output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
