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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <plugin.h>

#include "findinfiles.h"
#include "callbacks.h"
#include "interface.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_findinfiles_ui (CssedFifPlugin* fif)
{
	GtkWidget *findinfiles_ui;
	GtkWidget *vbox_fif;
	GtkWidget *hbox_controls;
	GtkWidget *hbox_controls_2;
	GtkWidget *label_searchterm;
	GtkWidget *entry_search_term;
	GtkWidget *label_basedir;
	GtkWidget *entry_base_dir;
	GtkWidget *combo_base_dir;
	GtkWidget *button_fif_search;
	GtkWidget *checkbutton_depth;
	GtkObject *spinbutton_depth_adj;
	GtkWidget *spinbutton_depth;
	GtkWidget *button_fif_set_basedir;
	GtkWidget *button_fif_set_basedir_from_current;	
	GtkWidget *button_fif_cancel_search;
	GtkWidget *button_fif_clear;
	GtkWidget *scrolledwindow_output;
	GtkWidget *treeview_fif_output;
	GtkTooltips *tooltips;
	GtkCellRenderer *renderer;
	GtkListStore *store;
	GtkTreeViewColumn *column;
	
	tooltips = gtk_tooltips_new ();
	
	findinfiles_ui = gtk_frame_new( NULL );

	// first row
	vbox_fif = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox_fif);
	gtk_container_add (GTK_CONTAINER (findinfiles_ui), vbox_fif);
	
	hbox_controls = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox_controls);
	gtk_box_pack_start (GTK_BOX (vbox_fif), hbox_controls, FALSE, TRUE, 0);
	
	label_searchterm = gtk_label_new (_("Search term"));
	gtk_widget_show (label_searchterm);
	gtk_box_pack_start (GTK_BOX (hbox_controls), label_searchterm, FALSE, FALSE, 5);
	
	entry_search_term = gtk_entry_new ();
	gtk_widget_show (entry_search_term);
	gtk_box_pack_start (GTK_BOX (hbox_controls), entry_search_term, TRUE, TRUE, 5);
	
	checkbutton_depth = gtk_check_button_new_with_mnemonic (_("Depth"));
	gtk_widget_show (checkbutton_depth);
	gtk_box_pack_start (GTK_BOX (hbox_controls), checkbutton_depth, FALSE, FALSE, 5);

	spinbutton_depth_adj = gtk_adjustment_new (1, 1, 100, 1, 10, 10);
	spinbutton_depth = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_depth_adj), 1, 0);
	gtk_widget_show (spinbutton_depth);
	gtk_box_pack_start (GTK_BOX (hbox_controls), spinbutton_depth, FALSE, FALSE, 10);

	button_fif_search = gtk_button_new_from_stock ("gtk-find");
	gtk_widget_show (button_fif_search);
	gtk_box_pack_start (GTK_BOX (hbox_controls), button_fif_search, FALSE, FALSE, 0);
	gtk_tooltips_set_tip (tooltips, button_fif_search, _("Click here to start a search"), NULL);
	
	button_fif_cancel_search = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (button_fif_cancel_search);
	gtk_box_pack_start (GTK_BOX (hbox_controls), button_fif_cancel_search, FALSE, FALSE, 5);
	gtk_widget_set_sensitive (button_fif_cancel_search, FALSE);	

	// second row
	hbox_controls_2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox_controls_2);
	gtk_box_pack_start (GTK_BOX (vbox_fif), hbox_controls_2, FALSE, TRUE, 0);

	label_basedir = gtk_label_new (_("Base directory"));
	gtk_widget_show (label_basedir);
	gtk_box_pack_start (GTK_BOX (hbox_controls_2), label_basedir, FALSE, FALSE, 5);

	button_fif_set_basedir_from_current = gtk_button_new_with_label (_("From document"));
	gtk_widget_show (button_fif_set_basedir_from_current);
	gtk_box_pack_start (GTK_BOX (hbox_controls_2), button_fif_set_basedir_from_current, FALSE, FALSE, 0);
	gtk_tooltips_set_tip (tooltips, button_fif_set_basedir_from_current, _("Click here to set the base directory from current's document one."), NULL);
	
	button_fif_set_basedir = gtk_button_new_from_stock ("gtk-open");
	gtk_widget_show (button_fif_set_basedir);
	gtk_box_pack_start (GTK_BOX (hbox_controls_2), button_fif_set_basedir, FALSE, FALSE, 0);
	gtk_tooltips_set_tip (tooltips, button_fif_set_basedir, _("Click here to set the base directory"), NULL);

	entry_base_dir = gtk_entry_new ();
	gtk_widget_show (entry_base_dir);
	gtk_box_pack_start (GTK_BOX (hbox_controls_2), entry_base_dir, TRUE, TRUE, 5);
	gtk_tooltips_set_tip (tooltips, entry_base_dir, _("Enter a base directory to search"), NULL);
	
	button_fif_clear = gtk_button_new_from_stock ("gtk-clear");
	gtk_widget_show (button_fif_clear);
	gtk_box_pack_start (GTK_BOX (hbox_controls_2), button_fif_clear, FALSE, FALSE, 5);

	// /////////
	scrolledwindow_output = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow_output);
	gtk_box_pack_start (GTK_BOX (vbox_fif), scrolledwindow_output, TRUE, TRUE, 5);
	
	treeview_fif_output = gtk_tree_view_new ();
	gtk_widget_show (treeview_fif_output);
	gtk_container_add (GTK_CONTAINER (scrolledwindow_output), treeview_fif_output);
	
	renderer = gtk_cell_renderer_text_new ();	
	column =	gtk_tree_view_column_new_with_attributes ("",  renderer,  "text", 0, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(column),   TRUE);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (treeview_fif_output),	column, 0);
	store =	gtk_list_store_new (1, G_TYPE_STRING);	
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview_fif_output),	 GTK_TREE_MODEL (store));
	g_object_unref(store);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview_fif_output), FALSE);
	
	fif->basedir_entry = entry_base_dir;
	fif->searchterm_entry = entry_search_term;
	fif->treeview = treeview_fif_output;
	fif->checkbox_depth = checkbutton_depth;
	fif->spinbutton_depth = spinbutton_depth;
	fif->button_search = button_fif_search;
	fif-> button_cancel = button_fif_cancel_search;
	
	g_signal_connect ((gpointer) button_fif_search, "clicked",
					G_CALLBACK (on_button_fif_search_clicked),
					fif);
	g_signal_connect ((gpointer) button_fif_set_basedir, "clicked",
					G_CALLBACK (on_button_fif_set_basedir_clicked),
					fif);
	g_signal_connect ((gpointer) button_fif_set_basedir_from_current, "clicked",
					G_CALLBACK (on_button_fif_set_basedir_from_current_clicked),
					fif);
	g_signal_connect ((gpointer) button_fif_cancel_search, "clicked",
					G_CALLBACK (on_button_fif_cancel_search_clicked),
					fif);
	g_signal_connect ((gpointer) button_fif_clear, "clicked",
					G_CALLBACK (on_button_fif_clear_clicked),
					fif);
	g_signal_connect ((gpointer) treeview_fif_output, "row_activated",
					G_CALLBACK (on_treeview_fif_output_row_activated), fif);

	return findinfiles_ui;
}


void
create_pop_menu_entry (CssedFifPlugin* fif)
{

	GtkWidget* cssed_popmenu;
	GtkWidget* image;
	GtkWidget* topmenu_item;
	GtkWidget* topmenu_menu;
	GtkWidget* in_current;
	GtkWidget* in_this_files_dir;

	cssed_popmenu = cssed_plugin_get_pop_menu( CSSED_PLUGIN(fif) );

	image = gtk_image_new_from_stock ("gtk-find", GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);

	topmenu_item = gtk_image_menu_item_new_with_label (_("Find in files"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (topmenu_item), image);
	gtk_widget_show (topmenu_item);
	gtk_container_add (GTK_CONTAINER (cssed_popmenu), topmenu_item);
	
	topmenu_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (topmenu_item), topmenu_menu);
	                                           
	in_current = gtk_image_menu_item_new_with_label (_("In current base directory"));
	gtk_widget_show (in_current);
	gtk_container_add (GTK_CONTAINER (topmenu_menu), in_current);
	
	in_this_files_dir = gtk_image_menu_item_new_with_label (_("In file's base directory"));
	gtk_widget_show (in_this_files_dir);
	gtk_container_add (GTK_CONTAINER (topmenu_menu), in_this_files_dir);

	fif->popmenu_item = topmenu_item;	

	g_signal_connect ((gpointer) in_current, "activate", G_CALLBACK (on_popmenu_in_current_activate), fif);
	g_signal_connect ((gpointer) in_this_files_dir, "activate", G_CALLBACK (on_popmenu_in_filedir_activate), fif);
}
