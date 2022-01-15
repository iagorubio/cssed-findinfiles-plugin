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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <gmodule.h>
#include <plugin.h>

#include "findinfiles.h"
#include "interface.h"


G_MODULE_EXPORT CssedPlugin* init_plugin(void);
gboolean load_findinfiles_plugin ( CssedPlugin* );
void clean_findinfiles_plugin ( CssedPlugin* );

static CssedPlugin findinfiles_plugin;
// this will return the plugin to the caller
G_MODULE_EXPORT CssedPlugin* init_plugin()
{
	findinfiles_plugin.name = _("Findinfiles Plugin"); 					// the plugin name	
	findinfiles_plugin.description  = _("Find matches in various files");// the plugin description
	findinfiles_plugin.load_plugin = &load_findinfiles_plugin; 			// load plugin function, will build the UI
	findinfiles_plugin.clean_plugin = &clean_findinfiles_plugin;		// clean plugin function, will destroy the UI
	findinfiles_plugin.user_data = NULL;								// User data
	findinfiles_plugin.priv =  NULL;									// Private data, this is opaque and should be ignored
	
	return &findinfiles_plugin;
}

gboolean
load_findinfiles_plugin (CssedPlugin* plugin)
{
	CssedFifPlugin* fif;
	GtkWidget* user_interface;

	if (!g_thread_supported ()) g_thread_init (NULL);
 
	gtk_set_locale ();
#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif
	
	fif = g_malloc(sizeof(CssedFifPlugin));
	fif->plugin = &findinfiles_plugin;
	fif->user_interface = create_findinfiles_ui(fif);
	fif->flag = FIF_THREAD_INIT_STATE;
	fif->command = NULL;
	//fif->mutex = g_mutex_new ();
	
	gtk_widget_show( fif->user_interface );
	findinfiles_plugin.user_data = fif;
	cssed_plugin_add_page_with_widget_to_footer( &findinfiles_plugin, fif->user_interface,	_("Find in files") );	
	create_pop_menu_entry (fif);
	
	return TRUE;
}

/* could be used to post UI destroy
void g_module_unload (GModule *module)
{
	g_print(_("** Find in files plugin unloaded\n"));	
}
*/

// to destroy UI and stuff.
void clean_findinfiles_plugin ( CssedPlugin* p )
{
	CssedFifPlugin* fif;
	GtkWidget* ui;
	
	fif =  FIF_PLUGIN( findinfiles_plugin.user_data );
	ui = fif->user_interface;
	gtk_widget_destroy(	ui );
	gtk_widget_destroy(	fif->popmenu_item );
	g_free(fif);

	return;
}







