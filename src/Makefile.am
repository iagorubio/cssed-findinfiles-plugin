## Process this file with automake to produce Makefile.in
plugindir=$(prefix)/lib/cssed/plugins
plugin_PROGRAMS=findinfiles.so

SRCS = \
	findinfiles.c findinfiles.h \
	interface.c interface.h \
	callbacks.c callbacks.h

if ISDARWIN
findinfiles_so_SOURCES = $(SRCS)
findinfiles_so_LDFLAGS =  -bundle -flat_namespace -undefined suppress
findinfiles_so_LDADD = @PACKAGE_LIBS@
findinfiles_so_CFLAGS = -fno-common -DDARWIN
else
findinfiles_so_SOURCES= $(SRCS)
findinfiles_so_LDFLAGS = -shared -fPICS
findinfiles_so_LDADD = @PACKAGE_LIBS@
endif

if ISBSD
INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/share/locale"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/findinfiles-plugin"\" \
	-DPACKAGE_LIB_DIR=\""$(prefix)/lib/cssed/"\" \
	-I$(top_srcdir) -I$(top_srcdir)/src \
	@PACKAGE_CFLAGS@ \
	-DBSD
else !ISBSD
if ISDARWIN
INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/share/locale"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/findinfiles-plugin"\" \
	-DPACKAGE_LIB_DIR=\""$(prefix)/lib/cssed/"\" \
	-I$(top_srcdir) -I$(top_srcdir)/src \
	@PACKAGE_CFLAGS@ \
	-DDARWIN
else
INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/share/locale"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/findinfiles-plugin"\" \
	-DPACKAGE_LIB_DIR=\""$(prefix)/lib/cssed/"\" \
	-I$(top_srcdir) -I$(top_srcdir)/src \
	@PACKAGE_CFLAGS@
endif
endif
