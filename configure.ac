dnl configure.ac for ccze
dnl Copyright (C) 2002, 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
dnl
dnl Process this file with autoconf to produce a configure script.
dnl
dnl This file is part of ccze.
dnl
dnl ccze is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl ccze is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

dnl **********************
dnl * Boilerplate
dnl **********************
AC_PREREQ(2.54)
AC_INIT([ccze], [0.2-stable], [algernon@bonehunter.rulez.org])
AC_CONFIG_SRCDIR(src/ccze.c)

AC_CONFIG_HEADERS(system.h)

AC_CANONICAL_HOST
AC_SUBST(host_os)

VERSION="$PACKAGE_VERSION"

dnl **********************
dnl * Programs
dnl **********************
AC_CHECK_TOOL(CC, cc)
AC_PROG_CC

WFLAGS=""
WFLAGS_3X=""
if test "x$ac_compiler_gnu" = "xyes"; then
	WFLAGS='${WFLAGS_GCC}'
	AC_MSG_CHECKING(whether we are using GCC 3)
	GCCVER=$(${CC} -dumpversion 2>/dev/null)
	case ${GCCVER} in
		3.*)
			WFLAGS_3X='${WFLAGS_3X}'
			AC_MSG_RESULT(yes)
			;;
		*)
			AC_MSG_RESULT(no)
			;;
	esac
else
	AC_MSG_CHECKING(whether we are using the Intel C compiler)
	if ${CC} -V 2>&1 | head -n 1 | grep -q "Intel(R)"; then
		AC_MSG_RESULT(yes)
		WFLAGS='${WFLAGS_ICC}'
	else
		AC_MSG_RESULT(no)
	fi
fi
AC_SUBST(WFLAGS)
AC_SUBST(WFLAGS_3X)

AC_PROG_CPP
AC_PROG_INSTALL

dnl **********************
dnl * Headers
dnl **********************
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([argp.h getopt.h netdb.h fcntl.h stddef.h])

dnl **********************
dnl * Typedefs & co
dnl **********************
AC_C_CONST
AC_C_VOLATILE
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL

dnl **********************
dnl * Library functions
dnl **********************
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRFTIME
AC_FUNC_STAT
AC_FUNC_MEMCMP
AC_FUNC_VPRINTF
AC_FUNC_CLOSEDIR_VOID
AC_CHECK_FUNCS([alphasort argp_parse asprintf getdelim getline getopt_long \
		getsubopt memchr memset scandir strcasecmp strchr strdup \
		strndup strstr])

AC_CHECK_LIB(ncurses, initscr, [], [AC_ERROR(NCurses library not found)])
AC_CHECK_FUNC(dlopen, [], [AC_CHECK_LIB(dl, dlopen)])
AC_CHECK_FUNC(getopt_long, [], [AC_CHECK_LIB(gnugetopt, getopt_long)])

if test -z "${PCRE_CONFIG}"; then
	AC_PATH_PROG(PCRE_CONFIG, pcre-config, no)
fi
AC_MSG_CHECKING(for PCRE)
if test "${PCRE_CONFIG}" = "no"; then
	AC_ERROR(PCRE not found)
fi
PCRE_CFLAGS=$($PCRE_CONFIG --cflags)
PCRE_LIBS=$($PCRE_CONFIG --libs)
AC_SUBST(PCRE_CFLAGS)
AC_SUBST(PCRE_LIBS)
AC_MSG_RESULT(found)

AC_MSG_CHECKING(for suboptarg)
AC_RUN_IFELSE(AC_LANG_PROGRAM(
[[#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char **empty_subopts[[]] = { NULL };
extern char *suboptarg;
]],
[[  char *subopts = "something";
  char *value;

  getsubopt (&subopts, empty_subopts, &value);
  exit (!suboptarg);]]), [have_suboptarg="yes"],
[have_suboptarg="no"], [have_suboptarg="no"])
AC_MSG_RESULT(${have_suboptarg})
case ${have_suboptarg} in
	yes)
		AC_DEFINE_UNQUOTED(HAVE_SUBOPTARG, 1,
			Define this if you have the suboptarg variable)
		;;
esac

dnl **********************
dnl * Plugins
dnl **********************
ALL_PLUGINS=$(tr '\n' ' ' < ${srcdir}/ccze.plugins)
AC_SUBST(ALL_PLUGINS)
AC_ARG_WITH(builtins,
	   [  --with-builtins=LIST    Build the listed plugins into the binary],
	   [case $withval in
		all)
			S_PLUGINS=$ALL_PLUGINS;;
		*)
			S_PLUGINS="$(echo $withval | sed 's/,/ /g')";;
	   esac], [S_PLUGINS=""])
AC_SUBST(S_PLUGINS)

dnl **********************
dnl * Output
dnl **********************
AC_CONFIG_FILES([
Rules.mk
Makefile
doc/Makefile
src/Makefile
testsuite/Makefile
])

AC_OUTPUT
