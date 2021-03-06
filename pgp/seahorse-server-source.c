/*
 * Seahorse
 *
 * Copyright (C) 2004 Stefan Walter
 * Copyright (C) 2011 Collabora Ltd.
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
 * along with this program; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "seahorse-server-source.h"

#include "seahorse-hkp-source.h"
#include "seahorse-ldap-source.h"
#include "seahorse-pgp-key.h"

#include "seahorse-common.h"

#include "libseahorse/seahorse-util.h"

#include <glib/gi18n.h>

#include <stdlib.h>
#include <string.h>

/**
 * SECTION:seahorse-server-source
 * @short_description: Objects for handling of internet sources of keys (hkp/ldap)
 * @include:seahorse-server-source.h
 *
 **/

enum {
    PROP_0,
    PROP_LABEL,
    PROP_DESCRIPTION,
    PROP_ICON,
    PROP_CATEGORY,
    PROP_KEY_SERVER,
    PROP_URI,
    PROP_ACTIONS,
    PROP_ACTION_PREFIX,
    PROP_MENU_MODEL,
    PROP_SHOW_IF_EMPTY,
    N_PROPS
};

/* -----------------------------------------------------------------------------
 *  SERVER SOURCE
 */

typedef struct _SeahorseServerSourcePrivate {
    gchar *server;
    gchar *uri;
} SeahorseServerSourcePrivate;

static void      seahorse_server_source_collection_init    (GcrCollectionIface *iface);

static void      seahorse_server_source_place_iface        (SeahorsePlaceIface *iface);

G_DEFINE_TYPE_WITH_CODE (SeahorseServerSource, seahorse_server_source, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (SeahorseServerSource)
                         G_IMPLEMENT_INTERFACE (GCR_TYPE_COLLECTION, seahorse_server_source_collection_init);
                         G_IMPLEMENT_INTERFACE (SEAHORSE_TYPE_PLACE, seahorse_server_source_place_iface);
);

/* GObject handlers */
static void seahorse_server_source_finalize   (GObject *gobject);
static void seahorse_server_get_property      (GObject *object, guint prop_id,
                                               GValue *value, GParamSpec *pspec);
static void seahorse_server_set_property      (GObject *object, guint prop_id,
                                               const GValue *value, GParamSpec *pspec);

static void
seahorse_server_source_class_init (SeahorseServerSourceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = seahorse_server_source_finalize;
    gobject_class->set_property = seahorse_server_set_property;
    gobject_class->get_property = seahorse_server_get_property;

	g_object_class_override_property (gobject_class, PROP_LABEL, "label");
	g_object_class_override_property (gobject_class, PROP_DESCRIPTION, "description");
    g_object_class_override_property (gobject_class, PROP_ICON, "icon");
    g_object_class_override_property (gobject_class, PROP_CATEGORY, "category");
	g_object_class_override_property (gobject_class, PROP_ACTIONS, "actions");
	g_object_class_override_property (gobject_class, PROP_ACTION_PREFIX, "action-prefix");
	g_object_class_override_property (gobject_class, PROP_MENU_MODEL, "menu-model");
    g_object_class_override_property (gobject_class, PROP_SHOW_IF_EMPTY, "show-if-empty");

    g_object_class_install_property (gobject_class, PROP_KEY_SERVER,
            g_param_spec_string ("key-server", "Key Server",
                                 "Key Server to search on", "",
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class, PROP_URI,
            g_param_spec_string ("uri", "Key Server URI",
                                 "Key Server full URI", "",
                                 G_PARAM_READWRITE));
}

static void
seahorse_server_source_init (SeahorseServerSource *ssrc)
{
}

static void
seahorse_server_source_finalize (GObject *gobject)
{
    SeahorseServerSource *ssrc = SEAHORSE_SERVER_SOURCE (gobject);
    SeahorseServerSourcePrivate *priv =
        seahorse_server_source_get_instance_private (ssrc);

    g_free (priv->server);
    g_free (priv->uri);
 
    G_OBJECT_CLASS (seahorse_server_source_parent_class)->finalize (gobject);
}

static void
seahorse_server_source_load (SeahorsePlace *self,
                              GCancellable *cancellable,
                              GAsyncReadyCallback callback,
                              gpointer user_data)
{
	g_return_if_reached ();
}

static gboolean
seahorse_server_source_load_finish (SeahorsePlace *self,
                                     GAsyncResult *res,
                                     GError **error)
{
	g_return_val_if_reached (FALSE);
}

static gchar *
seahorse_server_source_get_label (SeahorsePlace* self)
{
    SeahorseServerSource *ssrc = SEAHORSE_SERVER_SOURCE (self);
    SeahorseServerSourcePrivate *priv =
        seahorse_server_source_get_instance_private (ssrc);

	return g_strdup (priv->server);
}

static void
seahorse_server_source_set_label (SeahorsePlace *self, const char *label)
{
}

static gchar *
seahorse_server_source_get_description (SeahorsePlace* self)
{
    SeahorseServerSource *ssrc = SEAHORSE_SERVER_SOURCE (self);
    SeahorseServerSourcePrivate *priv =
        seahorse_server_source_get_instance_private (ssrc);

	return g_strdup (priv->uri);
}

static gchar *
seahorse_server_source_get_uri (SeahorsePlace* self)
{
    SeahorseServerSource *ssrc = SEAHORSE_SERVER_SOURCE (self);
    SeahorseServerSourcePrivate *priv =
        seahorse_server_source_get_instance_private (ssrc);

	return g_strdup (priv->uri);
}

static GIcon *
seahorse_server_source_get_icon (SeahorsePlace* self)
{
	return g_themed_icon_new (NULL);
}

static SeahorsePlaceCategory
seahorse_server_source_get_category (SeahorsePlace *place)
{
    return SEAHORSE_PLACE_CATEGORY_KEYS;
}

static GActionGroup *
seahorse_server_source_get_actions (SeahorsePlace* self)
{
    return NULL;
}

static const gchar *
seahorse_server_source_get_action_prefix (SeahorsePlace* self)
{
    return NULL;
}

static GMenuModel *
seahorse_server_source_get_menu_model (SeahorsePlace* self)
{
    return NULL;
}

static gboolean
seahorse_server_source_get_show_if_empty (SeahorsePlace *place)
{
    return TRUE;
}

static void
seahorse_server_source_place_iface (SeahorsePlaceIface *iface)
{
    iface->load = seahorse_server_source_load;
    iface->load_finish = seahorse_server_source_load_finish;
    iface->get_actions = seahorse_server_source_get_actions;
    iface->get_action_prefix = seahorse_server_source_get_action_prefix;
    iface->get_menu_model = seahorse_server_source_get_menu_model;
    iface->get_description = seahorse_server_source_get_description;
    iface->get_icon = seahorse_server_source_get_icon;
    iface->get_category = seahorse_server_source_get_category;
    iface->get_label = seahorse_server_source_get_label;
    iface->set_label = seahorse_server_source_set_label;
    iface->get_uri = seahorse_server_source_get_uri;
    iface->get_show_if_empty = seahorse_server_source_get_show_if_empty;
}

/**
* object: A SeahorseServerSource object
* prop_id: The ID of the property to set
* value: The value to set
* pspec: ignored
*
* Properties that can be set:
* PROP_KEY_SERVER, PROP_URI
*
**/
static void 
seahorse_server_set_property (GObject *object, guint prop_id, 
                              const GValue *value, GParamSpec *pspec)
{
    SeahorseServerSource *ssrc = SEAHORSE_SERVER_SOURCE (object);
    SeahorseServerSourcePrivate *priv =
        seahorse_server_source_get_instance_private (ssrc);
 
    switch (prop_id) {
    case PROP_LABEL:
        seahorse_server_source_set_label (SEAHORSE_PLACE (ssrc),
                                          g_value_get_boxed (value));
        break;
    case PROP_KEY_SERVER:
        g_assert (priv->server == NULL);
        priv->server = g_strdup (g_value_get_string (value));
        g_return_if_fail (priv->server && priv->server[0]);
        break;
    case PROP_URI:
        g_free (priv->uri);
        priv->uri = g_strdup (g_value_get_string (value));
        g_return_if_fail (priv->uri && priv->uri[0]);
        break;
    default:
        break;
    }  
}

/**
* object: A #SeahorseServerSource object
* prop_id: The id of the property
* value: The value to get
* pspec: ignored
*
* The properties that can be read are:
* PROP_KEY_SERVER, PROP_URI
*
**/
static void 
seahorse_server_get_property (GObject *obj,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
	SeahorseServerSource *self = SEAHORSE_SERVER_SOURCE (obj);
	SeahorsePlace *place = SEAHORSE_PLACE (self);
    SeahorseServerSourcePrivate *priv =
        seahorse_server_source_get_instance_private (self);

	switch (prop_id) {
	case PROP_LABEL:
		g_value_take_string (value, seahorse_server_source_get_label (place));
		break;
	case PROP_KEY_SERVER:
		g_value_set_string (value, priv->server);
		break;
	case PROP_DESCRIPTION:
		g_value_take_string (value, seahorse_server_source_get_description (place));
		break;
	case PROP_URI:
		g_value_take_string (value, seahorse_server_source_get_uri (place));
		break;
    case PROP_ICON:
        g_value_take_object (value, seahorse_server_source_get_icon (place));
        break;
    case PROP_CATEGORY:
        g_value_set_enum (value, seahorse_server_source_get_category (place));
        break;
    case PROP_ACTIONS:
        g_value_set_object (value, seahorse_server_source_get_actions (place));
        break;
    case PROP_ACTION_PREFIX:
        g_value_set_string (value, seahorse_server_source_get_action_prefix (place));
        break;
    case PROP_MENU_MODEL:
        g_value_set_object (value, seahorse_server_source_get_menu_model (place));
        break;
    case PROP_SHOW_IF_EMPTY:
        g_value_set_boolean (value, TRUE);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
        break;
    }
}

static guint
seahorse_server_source_get_length (GcrCollection *collection)
{
	return 0;
}

static GList *
seahorse_server_source_get_objects (GcrCollection *collection)
{
	return NULL;
}

static gboolean
seahorse_server_source_contains (GcrCollection *collection,
                                 GObject *object)
{
	return FALSE;
}

static void
seahorse_server_source_collection_init (GcrCollectionIface *iface)
{
	/* This is implemented because SeahorseSource requires it */
	iface->get_length = seahorse_server_source_get_length;
	iface->get_objects = seahorse_server_source_get_objects;
	iface->contains = seahorse_server_source_contains;
}

/* --------------------------------------------------------------------------
 * METHODS
 */

/**
* uri: the uri to parse
* scheme: the scheme ("http") of this uri
* host: the host part of the uri
*
*
* Code adapted from GnuPG (file g10/keyserver.c)
*
* Returns FALSE if the separation failed
**/
static gboolean 
parse_keyserver_uri (char *uri, const char **scheme, const char **host)
{
    int assume_ldap = 0;

    g_assert (uri != NULL);
    g_assert (scheme != NULL && host != NULL);

    *scheme = NULL;
    *host = NULL;

    /* Get the scheme */

    *scheme = strsep(&uri, ":");
    if (uri == NULL) {
        /* Assume LDAP if there is no scheme */
        assume_ldap = 1;
        uri = (char*)*scheme;
        *scheme = "ldap";
    }

    if (assume_ldap || (uri[0] == '/' && uri[1] == '/')) {
        /* Two slashes means network path. */

        /* Skip over the "//", if any */
        if (!assume_ldap)
            uri += 2;

        /* Get the host */
        *host = strsep (&uri, "/");
        if (*host[0] == '\0')
            return FALSE;
    }

    if (*scheme[0] == '\0')
        return FALSE;

    return TRUE;
}

/**
 * seahorse_server_source_new:
 * @server: The server uri to create an object for
 *
 * Creates a #SeahorseServerSource object out of @server. Depending
 * on the defines at compilation time other sources are supported
 * (ldap, hkp)
 *
 * Returns: A new SeahorseServerSource or NULL
 */
SeahorseServerSource* 
seahorse_server_source_new (const gchar *server)
{
    SeahorseServerSource *ssrc = NULL;
    const gchar *scheme;
    const gchar *host;
    gchar *uri, *t;
    
    g_return_val_if_fail (server && server[0], NULL);
    
    uri = g_strdup (server);
        
    if (!parse_keyserver_uri (uri, &scheme, &host)) {
        g_warning ("invalid uri passed: %s", server);

        
    } else {
        
#ifdef WITH_LDAP       
        /* LDAP Uris */ 
        if (g_ascii_strcasecmp (scheme, "ldap") == 0) 
            ssrc = SEAHORSE_SERVER_SOURCE (seahorse_ldap_source_new (server, host));
        else
#endif /* WITH_LDAP */
        
#ifdef WITH_HKP
        /* HKP Uris */
        if (g_ascii_strcasecmp (scheme, "hkp") == 0) {
            
            ssrc = SEAHORSE_SERVER_SOURCE (seahorse_hkp_source_new (server, host));

        /* HTTP Uris */
        } else if (g_ascii_strcasecmp (scheme, "http") == 0 ||
                   g_ascii_strcasecmp (scheme, "https") == 0) {

            /* If already have a port */
            if (strchr (host, ':')) 
	            ssrc = SEAHORSE_SERVER_SOURCE (seahorse_hkp_source_new (server, host));

            /* No port make sure to use defaults */
            else {
                t = g_strdup_printf ("%s:%d", host, 
                                     (g_ascii_strcasecmp (scheme, "http") == 0) ? 80 : 443);
                ssrc = SEAHORSE_SERVER_SOURCE (seahorse_hkp_source_new (server, t));
                g_free (t);
            }

        } else
#endif /* WITH_HKP */
        
            g_message ("unsupported key server uri scheme: %s", scheme);
    }
    
    g_free (uri);
    return ssrc;
}

void
seahorse_server_source_search_async (SeahorseServerSource *self,
                                     const gchar *match,
                                     GcrSimpleCollection *results,
                                     GCancellable *cancellable,
                                     GAsyncReadyCallback callback,
                                     gpointer user_data)
{
	g_return_if_fail (SEAHORSE_IS_SERVER_SOURCE (self));
	g_return_if_fail (match != NULL);
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
	g_return_if_fail (SEAHORSE_SERVER_SOURCE_GET_CLASS (self)->search_async);
	SEAHORSE_SERVER_SOURCE_GET_CLASS (self)->search_async (self, match, results,
	                                                       cancellable, callback, user_data);
}

gboolean
seahorse_server_source_search_finish (SeahorseServerSource *self,
                                      GAsyncResult *result,
                                      GError **error)
{
	g_return_val_if_fail (SEAHORSE_IS_SERVER_SOURCE (self), FALSE);
	g_return_val_if_fail (G_IS_ASYNC_RESULT (result), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (SEAHORSE_SERVER_SOURCE_GET_CLASS (self)->search_finish, FALSE);
	return SEAHORSE_SERVER_SOURCE_GET_CLASS (self)->search_finish (self, result, error);
}

void
seahorse_server_source_export_async (SeahorseServerSource *self,
                                     const gchar **keyids,
                                     GCancellable *cancellable,
                                     GAsyncReadyCallback callback,
                                     gpointer user_data)
{
	SeahorseServerSourceClass *klass;

	g_return_if_fail (SEAHORSE_IS_SERVER_SOURCE (self));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	klass = SEAHORSE_SERVER_SOURCE_GET_CLASS (self);
	g_return_if_fail (klass->export_async);
	(klass->export_async) (self, keyids, cancellable, callback, user_data);
}

gpointer
seahorse_server_source_export_finish (SeahorseServerSource *self,
                                      GAsyncResult *result,
                                      gsize *size,
                                      GError **error)
{
	SeahorseServerSourceClass *klass;

	g_return_val_if_fail (SEAHORSE_IS_SERVER_SOURCE (self), NULL);
	g_return_val_if_fail (G_IS_ASYNC_RESULT (result), NULL);
	g_return_val_if_fail (size != NULL, NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	klass = SEAHORSE_SERVER_SOURCE_GET_CLASS (self);
	g_return_val_if_fail (klass->export_async != NULL, NULL);
	g_return_val_if_fail (klass->export_finish != NULL, NULL);
	return (klass->export_finish) (self, result, size, error);
}

void
seahorse_server_source_import_async (SeahorseServerSource *source,
                                     GInputStream *input,
                                     GCancellable *cancellable,
                                     GAsyncReadyCallback callback,
                                     gpointer user_data)
{
	g_return_if_fail (SEAHORSE_IS_SERVER_SOURCE (source));
	g_return_if_fail (G_IS_INPUT_STREAM (input));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
	g_return_if_fail (SEAHORSE_SERVER_SOURCE_GET_CLASS (source)->import_async);
	SEAHORSE_SERVER_SOURCE_GET_CLASS (source)->import_async (source, input, cancellable,
	                                                         callback, user_data);
}

GList *
seahorse_server_source_import_finish (SeahorseServerSource *source,
                                      GAsyncResult *result,
                                      GError **error)
{
	g_return_val_if_fail (SEAHORSE_IS_SERVER_SOURCE (source), NULL);
	g_return_val_if_fail (G_IS_ASYNC_RESULT (result), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);
	g_return_val_if_fail (SEAHORSE_SERVER_SOURCE_GET_CLASS (source)->import_finish, NULL);
	return SEAHORSE_SERVER_SOURCE_GET_CLASS (source)->import_finish (source, result, error);
}
