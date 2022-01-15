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

#include <gtk/gtk.h>

void
on_button_fif_search_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_fif_set_basedir_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_fif_set_basedir_from_current_clicked (GtkButton       *button,
                                        gpointer         user_data);
void
on_button_fif_cancel_search_clicked    (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_fif_clear_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_treeview_fif_output_row_activated   (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_popmenu_in_current_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_popmenu_in_filedir_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


