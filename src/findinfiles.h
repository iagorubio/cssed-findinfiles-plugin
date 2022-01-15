 /*  find in files plugin for cssed (c) Iago Rubio 2004
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#define FIF_PLUGIN(n) (CssedFifPlugin*) n
#define CSSED_PLUGIN(obj) (obj)->plugin

enum {
	FIF_THREAD_INIT_STATE,
	FIF_THREAD_CONTINUE,
	FIF_THREAD_STOP
};

typedef struct _CssedFifPlugin {
	CssedPlugin* plugin;
	GtkWidget* user_interface;
	GtkWidget* popmenu_item;
	GtkWidget* basedir_entry;
	GtkWidget* searchterm_entry;
	GtkWidget* checkbox_depth;
	GtkWidget* spinbutton_depth;
	GtkWidget* treeview;
	GtkWidget* button_search;
	GtkWidget* button_cancel;
	//GMutex* mutex;
	guint16 flag;
	gchar* command;
} CssedFifPlugin;





