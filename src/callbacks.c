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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(BSD) || defined(DARWIN) 
 #include <errno.h>
#endif

#include <gtk/gtk.h>
#include <plugin.h>

#include "findinfiles.h"
#include "callbacks.h"
#include "interface.h"

// Thread func
gpointer fif_search_proc (gpointer data);
// utility func
gboolean is_string_all_digits(gchar* string);

typedef struct _DocLineData {
	CssedPlugin *plugin;
	gint line;
} DocLineData;

#if defined(BSD) || defined(DARWIN)
int
getline(char** line, size_t* size, FILE* fp)
{
	size_t i;
	static const size_t grow_rate = 80; 
	int c;

	if (line == NULL || size == NULL || fp == NULL) { 
		errno = EINVAL;
		return -1;
	}
	if (*line == NULL ) { 
		*line = g_malloc0(sizeof(gchar) * grow_rate);
		*size = grow_rate;
	}

	i = 0;
	while (1) {
		c = fgetc(fp);
		if (c == EOF)
			break;
		if ((*size - i) <= 1) {
			*size += grow_rate;
			*line = (char*)g_realloc(*line, *size);
			if (*line == NULL) {
				errno = ENOMEM;
				return -1;
			}
		}
		*(*line + i++) = (char)c;
		if (c == '\n')
			break;
	}

	*(*line + i) = 0;

	if( ferror(fp) != 0 ){
		return -1;
	}else{
		return i;
	}
}
#endif


// for variable substitution, don't use it with large text as it uses a copy - so it doubles the
// memory needed for the string substitution.
// THIS IS FROM UTILS.C BUT WILL BE ON THE PLUGIN'S INTERFACE - FIXME
gchar* string_replace_all(gchar *string, gchar *replaced_str, gchar *replacement_str, gboolean free_string)
{
	gchar* string_copy; // we'll manipulate it so we need a copy
	gchar *iterator, *match, *endstr; // catch position pointers
	GString *newstring;

	g_return_val_if_fail(string != NULL, NULL);
	g_return_val_if_fail(replaced_str != NULL, NULL);
	g_return_val_if_fail(replacement_str != NULL, NULL);

	newstring = g_string_new("");
	string_copy = g_strdup(string);
	endstr = string_copy + strlen(string_copy);
	iterator = string_copy;

	while( (match = strstr (iterator, replaced_str)) != NULL ){
		*match = '\0';
		newstring = g_string_append(newstring, iterator);
		newstring = g_string_append(newstring, replacement_str);
		iterator = match + strlen(replaced_str);
	}
	
	// if no match it'll copy the whole string, if there's something
	// before last match will append the leading text.
	if( iterator != endstr )
		newstring = g_string_append(newstring, iterator);

	g_free(string_copy);
	string_copy = g_strdup(newstring->str);
	g_string_free(newstring, TRUE);
	
	if( free_string ) g_free(string);
	return string_copy;
} 

void
on_button_fif_search_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* basedir_entry;
	GtkWidget* searchterm_entry;
	GtkWidget* checkbox_depth;
	GtkWidget* spinbutton_depth;
	GError* error = NULL;
	gchar* basedir_str;
	gchar* searchterm_str;
	gchar* depth_str;
	gchar* cmd_line;
	gchar* old_ptr;
	GThread* thread;
	CssedFifPlugin* fif;

	fif = FIF_PLUGIN(user_data);
	basedir_entry = fif->basedir_entry;
	searchterm_entry = fif->searchterm_entry;
	checkbox_depth = fif->checkbox_depth;
	spinbutton_depth = fif->spinbutton_depth;

	basedir_str = gtk_editable_get_chars(GTK_EDITABLE(basedir_entry), 0, -1);
	if( strlen(basedir_str) <= 0 ){
		cssed_plugin_error_message(_("Base directory not set"), _("Please set the base directory where the search will start"));
		g_free( basedir_str );
		return;
	}

	searchterm_str = gtk_editable_get_chars(GTK_EDITABLE(searchterm_entry), 0, -1);
	if( strlen(searchterm_str) <= 0 ){
		cssed_plugin_error_message(_("Search term not set"), _("Please enter a search term to search for."));
		g_free( searchterm_str );
		g_free( basedir_str );
		return;
	}	
	cmd_line = g_strdup_printf ("find \"%s\" ",  basedir_str);
	
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox_depth)) ){
		depth_str = g_strdup_printf( "-maxdepth %d", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton_depth)) );
		old_ptr = cmd_line;
		cmd_line = g_strdup_printf("%s %s", old_ptr, depth_str );
		g_free(old_ptr);
		g_free(depth_str);
	}

#if defined(DARWIN) // was reported by Michelle that OsX doesn't work without the -maxdepth parameter.
	if( !(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox_depth))) ){
		depth_str = g_strdup_printf( "-maxdepth 1" );
		old_ptr = cmd_line;
		cmd_line = g_strdup_printf("%s %s", old_ptr, depth_str );
		g_free(old_ptr);
		g_free(depth_str);
	}
#endif

	searchterm_str = string_replace_all (searchterm_str, "\"", "\\\"", TRUE); // clean up quotes	
	searchterm_str = string_replace_all (searchterm_str, "`", "\\`", TRUE); // clean up backticks
	searchterm_str = string_replace_all (searchterm_str, "$", "\\$", TRUE); // clean up $
	searchterm_str = string_replace_all (searchterm_str, "\\", "\\\\", TRUE); // clean up \
	
	old_ptr = cmd_line;
	cmd_line = g_strdup_printf("%s -print0 -type f|xargs -0 grep -n \"%s\"", cmd_line, searchterm_str );
	g_free( old_ptr );	
	g_free( searchterm_str );
	
	if( fif->command ) g_free( fif->command );
	fif->command = cmd_line;
	thread = g_thread_create  (fif_search_proc,fif,FALSE ,&error);
	if( thread == NULL ){
		g_free( fif->command );
		fif->command = NULL;
		cssed_plugin_error_message(_("Search error"), _("Unable to launch the search proccess"));
	}
}


void
on_button_fif_set_basedir_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* basedir;
	gchar* dirname;
	gint response;
	gchar* basedir_str;
	gchar* full_path;
	CssedFifPlugin* fif;

#ifdef GTK_ATLEAST_2_4
	gchar* filename;
#else
	G_CONST_RETURN gchar* filename;
#endif
	fif = FIF_PLUGIN(user_data);
	basedir = fif->basedir_entry;

	basedir_str = gtk_editable_get_chars(GTK_EDITABLE(basedir), 0, -1);

	if( strlen(basedir_str) > 0 ){
		if( !g_str_has_suffix (basedir_str, G_DIR_SEPARATOR_S) ){
			full_path = g_strdup_printf("%s%s", basedir_str, G_DIR_SEPARATOR_S);	
			filename = cssed_plugin_prompt_for_directory_to_open(fif->plugin, _("Select a directory"), full_path);
			g_free( full_path );
		}else{
			filename = cssed_plugin_prompt_for_directory_to_open(fif->plugin, _("Select a directory"), basedir_str);
		}
	}else{
		filename = cssed_plugin_prompt_for_directory_to_open(fif->plugin, _("Select a directory"), NULL);
	}

	g_free( basedir_str );
	
	if( filename != NULL ){
		if( !g_file_test( filename, G_FILE_TEST_IS_DIR) ){			
			dirname = g_path_get_dirname ( filename );
			gtk_entry_set_text( GTK_ENTRY(basedir),  dirname);
			g_free( dirname );
		}else{
			gtk_entry_set_text( GTK_ENTRY(basedir),  filename);			
		}
		g_free(filename);
	}
}


void
on_button_fif_set_basedir_from_current_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *basedir;
	gchar *dirname, *filename;
	CssedFifPlugin *fif;

	fif = FIF_PLUGIN(user_data);
	basedir = fif->basedir_entry;
	filename = cssed_plugin_document_get_filename (fif->plugin);
	
	if( filename ){
		dirname = g_path_get_dirname(filename);
		gtk_entry_set_text( GTK_ENTRY(basedir),  dirname);		
		g_free(filename);
		g_free(dirname);
	}else{
		cssed_plugin_error_message(_("File is not saved"), _("The file is not saved on this, cannot use\nthis file's base directory"));
	}	
}

void
on_button_fif_cancel_search_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
	CssedFifPlugin* fif;
	guint16 flag;
	// No need for a mutex as there'll not be two threads at one time.
	// The running thread will "block" the search button from process
	// start to end.
	fif = FIF_PLUGIN(user_data);
	fif->flag = FIF_THREAD_STOP;
}

void
on_button_fif_clear_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	CssedFifPlugin* fif;
	GtkListStore* store;

	fif = FIF_PLUGIN(user_data);
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(fif->treeview)));
	gtk_list_store_clear(store);
}

// Thread func
gpointer
fif_search_proc               (gpointer data)
{
	CssedFifPlugin* fif;
	GtkWidget* search_button;
	GtkWidget* cancel_button;
	GtkWidget *popmenu;
	GtkListStore* store;
	GtkTreeIter iter;
	//GtkTreePath* path;
	//GMutex* mutex;
	gchar* outline;
	guint16 flag;
	gint c;
	FILE* fp;
	char * line = NULL;
#if defined(BSD) || defined(DARWIN)
	size_t len = 0;
	int read;
#else
	size_t len = 0;
	ssize_t read;
#endif

	fif = FIF_PLUGIN(data);	
	//mutex = fif->mutex;
	flag = fif->flag;
	search_button = fif->button_search;
	cancel_button = fif->button_cancel;
	popmenu = fif->popmenu_item;
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(fif->treeview)));

	gdk_threads_enter();
	gtk_widget_set_sensitive(search_button, FALSE); 
	gtk_widget_set_sensitive(cancel_button, TRUE);
	gtk_widget_set_sensitive(popmenu, FALSE);
#if defined(DARWIN)
	gtk_widget_set_sensitive(fif->treeview, FALSE);
#endif
	gdk_threads_leave();

	//g_mutex_lock( mutex );
	fif->flag = FIF_THREAD_CONTINUE;
	//g_mutex_unlock( mutex );

	fp = popen( fif->command, "r");
	if(fp == NULL){
		gdk_threads_enter();
		gtk_widget_set_sensitive(search_button, TRUE);
		gtk_widget_set_sensitive(cancel_button, FALSE); 
#if defined(DARWIN)
		gtk_widget_set_sensitive(fif->treeview, TRUE);
#endif		
		gdk_threads_leave();

		//g_mutex_lock( mutex );
		fif->flag = FIF_THREAD_INIT_STATE;
		//g_mutex_unlock( mutex );	

		if( fif->command ) g_free( fif->command );
		fif->command = NULL;

		gdk_threads_enter();
		cssed_plugin_error_message(_("Search error"), _("Cannot open pipe")); 
		gdk_threads_leave();
		return;
	}else{
		gdk_threads_enter();
		gtk_list_store_clear(store);
		cssed_plugin_select_page_with_widget_in_footer(CSSED_PLUGIN(fif), fif->user_interface);
		gtk_list_store_append(store, &iter);
		gtk_list_store_set( store, &iter, 0, _("** Starting search ..."), -1);
		gdk_threads_leave();
#if defined(BSD) || defined(DARWIN)
		while ((read = getline(&line, &len, fp)) > 0)
#else
		while ((read = getline(&line, &len, fp)) != -1)
#endif
		{
			if( fif->flag != FIF_THREAD_CONTINUE ){
				if(line) g_free(line);
				break;
			}else{
				g_strstrip(line);
				gdk_threads_enter();
				gtk_list_store_append(store, &iter);
				gtk_list_store_set(store, &iter, 0,line, -1);
				gdk_threads_leave();
				if(line) g_free(line);
			}
			line = NULL;
		}
		gdk_threads_enter();
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, _("** Search end."), -1);
		gdk_threads_leave();
		pclose(fp);
	}	

	fif->flag = FIF_THREAD_INIT_STATE;
	g_free(fif->command);
	fif->command = NULL;

	gdk_threads_enter();
	gtk_widget_set_sensitive(search_button, TRUE);
	gtk_widget_set_sensitive(cancel_button, FALSE);
	gtk_widget_set_sensitive(popmenu, TRUE);
#if defined(DARWIN)
	gtk_widget_set_sensitive(fif->treeview, TRUE);
#endif
	gdk_threads_leave();

	g_thread_exit(EXIT_SUCCESS);
}

gboolean    
fif_move_to_line_on_idle (gpointer user_data)
{
	DocLineData* data = (DocLineData*) user_data;
	cssed_plugin_set_arrow_marker_at_line (data->plugin, data->line);
	return FALSE;
}

void
on_treeview_fif_output_row_activated   (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{
	DocLineData* data;
	CssedFifPlugin* fif;
	GtkTreeIter iter;
	GtkListStore* store;
	gchar* outline;
	gchar** entries;
	gchar* file;
	gchar* number;
	gint pos;

	fif = FIF_PLUGIN(user_data);
	store = (GtkListStore*) gtk_tree_view_get_model( treeview );

	if (gtk_tree_model_get_iter (GTK_TREE_MODEL(store), &iter, path))
	{
		gtk_tree_model_get (GTK_TREE_MODEL(store), &iter, 0, &outline, -1);
		entries = g_strsplit  (outline,":",3);
		file = *entries;
		if( file == NULL ){
			g_strfreev( entries );
			return;
		}
		number = *(entries+1);
		if( number == NULL ){
			g_strfreev( entries );
			g_free( outline );
			return;		
		}		
		if( !is_string_all_digits(number) ){
			g_strfreev( entries );
			g_free( outline );
			return;	
		}
		if( !g_file_test(file, G_FILE_TEST_IS_REGULAR) ){
			g_strfreev( entries );
			g_free( outline );
			return;		
		}

		if( !cssed_plugin_is_file_opened(CSSED_PLUGIN(fif), file) ){
			cssed_plugin_open_file(CSSED_PLUGIN(fif), file);
			if( cssed_plugin_is_file_opened(CSSED_PLUGIN(fif), file) ){
				data = g_malloc0(sizeof(DocLineData));
				data->plugin = CSSED_PLUGIN(fif);
				data->line = atoi(number);				
				g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
                                 fif_move_to_line_on_idle,
                                 data, g_free);                                             

			}		
		}else{
			cssed_plugin_set_arrow_marker_at_line(CSSED_PLUGIN(fif), atoi(number));
		}

		g_strfreev( entries );
		g_free( outline );	
	}
}

gboolean
is_string_all_digits(gchar* string)
{
	gint len;
	gint i;

	len = strlen( string );
	
	for(i=0;i<len;i++){
		if( !g_ascii_isdigit(string[i]) ){
			return FALSE;
		}
	}
	return TRUE;
}

void
on_popmenu_in_current_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	CssedFifPlugin* fif;
	GtkWidget* searchterm_entry;
	gchar* selection;
	gint page;

	fif = FIF_PLUGIN(user_data);	
	searchterm_entry = fif->searchterm_entry;

	selection = cssed_plugin_get_selected_text( CSSED_PLUGIN(fif) );
	if( selection != NULL  ){
		gtk_entry_set_text( GTK_ENTRY(searchterm_entry), selection );
		on_button_fif_search_clicked (NULL, (gpointer) fif );
		g_free(selection);
	}	
}

void
on_popmenu_in_filedir_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{	
	CssedFifPlugin* fif;
	GtkWidget* basedir_entry;
	GtkWidget* searchterm_entry;
	gchar* selection;
	gchar* filename;
	G_CONST_RETURN gchar* file_basedir;
	gint page;

	fif = FIF_PLUGIN(user_data);	
	searchterm_entry = fif->searchterm_entry;
	basedir_entry = fif->basedir_entry,
	
	selection = cssed_plugin_get_selected_text(CSSED_PLUGIN(fif));

	if( selection != NULL  ){
		filename = cssed_plugin_document_get_filename(CSSED_PLUGIN(fif));
		if( filename ){
			file_basedir = g_dirname(filename);
			gtk_entry_set_text(GTK_ENTRY(basedir_entry), (gchar*) file_basedir);
			gtk_entry_set_text(GTK_ENTRY(searchterm_entry), selection);
			on_button_fif_search_clicked (NULL, (gpointer) fif);
			g_free(filename);			
		}else{
			cssed_plugin_error_message(_("File not saved"), _("This file is not saved, so there is no base directory for this file.\nPlease save the file and try again."));
		}
		g_free(selection);
	}	
}
