/*
 * Seahorse
 *
 * Copyright (C) 2003 Jacob Perkins
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gnome.h>
#include <eel/eel.h>

#include "seahorse-key-manager-store.h"
#include "seahorse-preferences.h"
#include "seahorse-validity.h"
#include "seahorse-util.h"

#define KEY_MANAGER_SORT_KEY "/apps/seahorse/listing/sort_by"

enum {
    KEY_STORE_BASE_COLUMNS,
	VALIDITY_STR,
	EXPIRES_STR,
	TRUST_STR,
	LENGTH,
	TYPE,
    VALIDITY,
    EXPIRES,
    TRUST,
	COLS
};

static const gchar* col_ids[] = {
    KEY_STORE_BASE_IDS,
    "validity",
    "expires",
    "trust",
    "length",
    "type",
    "validity",
    "expires",
    "trust"
};

static GType col_types[] = {
    KEY_STORE_BASE_TYPES, 
    G_TYPE_STRING,
    G_TYPE_STRING, 
    G_TYPE_STRING, 
    G_TYPE_INT, 
    G_TYPE_STRING,
    G_TYPE_INT, 
    G_TYPE_LONG, 
    G_TYPE_INT
};

static void	seahorse_key_manager_store_class_init	(SeahorseKeyManagerStoreClass	*klass);

static void	seahorse_key_manager_store_append	(SeahorseKeyStore		*skstore,
                                                 SeahorseKey			*skey,
                                                 GtkTreeIter			*iter);
static void	seahorse_key_manager_store_set		(SeahorseKeyStore		*store,
                                                 SeahorseKey            *skey,
                                                 GtkTreeIter			*iter);
static void	seahorse_key_manager_store_remove	(SeahorseKeyStore		*skstore,
                                                 GtkTreeIter			*iter);
static void	seahorse_key_manager_store_changed	(SeahorseKeyStore       *skstore,
                                                 SeahorseKey			*skey,
                                                 GtkTreeIter			*iter,
                                                 SeahorseKeyChange      change);

static SeahorseKeyStoreClass	*parent_class	= NULL;

GType
seahorse_key_manager_store_get_type (void)
{
	static GType key_manager_store_type = 0;
	
	if (!key_manager_store_type) {
		static const GTypeInfo key_manager_store_info =
		{
			sizeof (SeahorseKeyManagerStoreClass),
			NULL, NULL,
			(GClassInitFunc) seahorse_key_manager_store_class_init,
			NULL, NULL,
			sizeof (SeahorseKeyManagerStore),
			0, NULL
		};
		
		key_manager_store_type = g_type_register_static (SEAHORSE_TYPE_KEY_STORE,
			"SeahorseKeyManagerStore", &key_manager_store_info, 0);
	}
	
	return key_manager_store_type;
}

static void
seahorse_key_manager_store_class_init (SeahorseKeyManagerStoreClass *klass)
{
	SeahorseKeyStoreClass *skstore_class;
	
	parent_class = g_type_class_peek_parent (klass);
	skstore_class = SEAHORSE_KEY_STORE_CLASS (klass);
	
	skstore_class->append = seahorse_key_manager_store_append;
	skstore_class->set = seahorse_key_manager_store_set;
	skstore_class->remove = seahorse_key_manager_store_remove;
	skstore_class->changed = seahorse_key_manager_store_changed;
  
  	/* Base class behavior and columns */
    skstore_class->use_check = FALSE;
    skstore_class->n_columns = COLS;
    skstore_class->col_ids = col_ids;
    skstore_class->col_types = col_types;
    skstore_class->gconf_sort_key = KEY_MANAGER_SORT_KEY;
}

static const char* 
get_algo_string (gpgme_pubkey_algo_t algo) {
    /* TODO: Some of these strings need to be fixed */
    switch (algo) {
        case GPGME_PK_RSA:
            return _("RSA");
        case GPGME_PK_RSA_E:
            return _("RSA-E");
        case GPGME_PK_RSA_S:
            return _("RSA-S");
        case GPGME_PK_ELG_E:
            return _("ELG-E");
        case GPGME_PK_DSA:
            return _("DSA");
        case GPGME_PK_ELG:
            return _("ELG");
        default:
            return _("Unknown");
    }
}

/* Appends subkeys for @skey with @iter as parent */
static void
append_uids (GtkTreeStore *store, GtkTreeIter *iter, SeahorseKey *skey)
{
	gint index = 1, max;
	GtkTreeIter child;

	max = seahorse_key_get_num_uids (skey);
	
	while (index < max) {
		gtk_tree_store_append (store, &child, iter);
		index++;
	}
}

/* Remove subkeys where @iter is the parent */
static void
remove_uids (GtkTreeStore *store, GtkTreeIter *iter)
{
	GtkTreeIter child;

	while (gtk_tree_model_iter_children (GTK_TREE_MODEL (store), &child, iter))
		gtk_tree_store_remove (store, &child);
}

/* Do append for @skey & it's subkeys */
static void
seahorse_key_manager_store_append (SeahorseKeyStore *skstore, SeahorseKey *skey, GtkTreeIter *iter)
{
	gtk_tree_store_append (GTK_TREE_STORE (skstore), iter, NULL);
	append_uids (GTK_TREE_STORE (skstore), iter, skey);
	parent_class->append (skstore, skey, iter);
}

/* Sets attributes for @skey at @iter and @skey's subkeys at @iter's children */
static void
seahorse_key_manager_store_set (SeahorseKeyStore *store, SeahorseKey *skey, GtkTreeIter *iter)
{
	GtkTreeIter child;
	gint index = 1, max;
	SeahorseValidity validity, trust;
	gulong expires_date;
	const gchar *expires;
	
	validity = seahorse_key_get_validity (skey);
	trust = seahorse_key_get_trust (skey);
	
	if (skey->key->expired) {
		expires = _("Expired");
		expires_date = -1;
	}
	else {
		expires_date = skey->key->subkeys->expires;
		
		if (expires_date == 0) {
			expires = _("Never Expires");
			expires_date = G_MAXLONG;
		}
		else
			expires = seahorse_util_get_date_string (expires_date);
	}
	
	gtk_tree_store_set (GTK_TREE_STORE (store), iter,
		VALIDITY_STR, seahorse_validity_get_string (validity),
		VALIDITY, validity,
		EXPIRES_STR, expires,
		EXPIRES, expires_date,
		TRUST_STR, seahorse_validity_get_string (trust),
		TRUST, trust,
		LENGTH, skey->key->subkeys->length,
		TYPE, get_algo_string (skey->key->subkeys->pubkey_algo),
		-1);
	
	max = seahorse_key_get_num_uids (skey);
	
	while (index < max && 
        gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (store), &child, iter, index-1)) {
        gchar *userid = seahorse_key_get_userid (skey, index);
		gtk_tree_store_set (GTK_TREE_STORE (store), &child,
			KEY_STORE_NAME, userid,
			TYPE, "UID", -1);
        g_free (userid);
		index++;
	}
	
	parent_class->set (store, skey, iter);
}

/* Removes subkeys, then does remove */
static void
seahorse_key_manager_store_remove (SeahorseKeyStore *skstore, GtkTreeIter *iter)
{
	remove_uids (GTK_TREE_STORE (skstore), iter);
	parent_class->remove (skstore, iter);
}

/* Refreshed @skey if trust has changed */
static void
seahorse_key_manager_store_changed (SeahorseKeyStore *skstore, SeahorseKey *skey, 
				               GtkTreeIter *iter, SeahorseKeyChange change)
{
	switch (change) {
		case SKEY_CHANGE_TRUST: case SKEY_CHANGE_EXPIRES:
		case SKEY_CHANGE_DISABLED:
			SEAHORSE_KEY_STORE_GET_CLASS (skstore)->set (skstore, skey, iter);
			break;
		/* Refresh uid iters, then let parent call set */
		case SKEY_CHANGE_UIDS:
			remove_uids (GTK_TREE_STORE (skstore), iter);
			append_uids (GTK_TREE_STORE (skstore), iter, skey);
		default:
			parent_class->changed (skstore, skey, iter, change);
			break;
	}
}

static void
gconf_notification (GConfClient *gclient, guint id, GConfEntry *entry, GtkTreeView *view)
{
	const gchar *key;
	GConfValue *value;
	GtkTreeViewColumn *col;
	
	key = gconf_entry_get_key (entry);
	value = gconf_entry_get_value (entry);
	
	if (g_str_equal (key, SHOW_VALIDITY_KEY))
		col = gtk_tree_view_get_column (view, VALIDITY_STR - 2);
	else if (g_str_equal (key, SHOW_EXPIRES_KEY))
		col = gtk_tree_view_get_column (view, EXPIRES_STR - 2);
	else if (g_str_equal (key, SHOW_TRUST_KEY))
		col = gtk_tree_view_get_column (view, TRUST_STR - 2);
	else if (g_str_equal (key, SHOW_LENGTH_KEY))
		col = gtk_tree_view_get_column (view, LENGTH - 2);
	else if (g_str_equal (key, SHOW_TYPE_KEY))
		col = gtk_tree_view_get_column (view, TYPE - 2);
	else
		return;
	
	gtk_tree_view_column_set_visible (col, gconf_value_get_bool (value));
}

/**
 * seahorse_key_manager_store_new:
 * @sctx: Current #SeahorseContext
 * @view: #GtkTreeView to show the new #SeahorseKeyManagerStore
 *
 * Creates a new #SeahorseKeyManagerStore.
 * Shown attributes are Name, KeyID, Trust, Type, and Length.
 *
 * Returns: The new #SeahorseKeyStore
 **/
SeahorseKeyStore*
seahorse_key_manager_store_new (SeahorseContext *sctx, GtkTreeView *view)
{
	SeahorseKeyStore *skstore;
	GtkTreeViewColumn *col;

	skstore = g_object_new (SEAHORSE_TYPE_KEY_MANAGER_STORE, "ctx", sctx, NULL);
    seahorse_key_store_init (skstore, view);
	
	col = seahorse_key_store_append_column (view, _("Validity"), VALIDITY_STR);
	gtk_tree_view_column_set_visible (col, eel_gconf_get_boolean (SHOW_VALIDITY_KEY));
	gtk_tree_view_column_set_sort_column_id (col, VALIDITY);
	
	col = seahorse_key_store_append_column (view, _("Expiration Date"), EXPIRES_STR);
	gtk_tree_view_column_set_visible (col, eel_gconf_get_boolean (SHOW_EXPIRES_KEY));
	gtk_tree_view_column_set_sort_column_id (col, EXPIRES);

	col = seahorse_key_store_append_column (view, _("Trust"), TRUST_STR);
	gtk_tree_view_column_set_visible (col, eel_gconf_get_boolean (SHOW_TRUST_KEY));
	gtk_tree_view_column_set_sort_column_id (col, TRUST);
	
	col = seahorse_key_store_append_column (view, _("Length"), LENGTH);
	gtk_tree_view_column_set_visible (col, eel_gconf_get_boolean (SHOW_LENGTH_KEY));
	gtk_tree_view_column_set_sort_column_id (col, LENGTH);
	
	col = seahorse_key_store_append_column (view, _("Type"), TYPE);
	gtk_tree_view_column_set_visible (col, eel_gconf_get_boolean (SHOW_TYPE_KEY));
	gtk_tree_view_column_set_sort_column_id (col, TYPE);
	
	eel_gconf_notification_add (LISTING_SCHEMAS, (GConfClientNotifyFunc)gconf_notification, view);
	eel_gconf_monitor_add (LISTING_SCHEMAS);
	
	return skstore;
}
