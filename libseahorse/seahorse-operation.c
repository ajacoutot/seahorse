/*
 * Seahorse
 *
 * Copyright (C) 2004 Nate Nielsen
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

#include "seahorse-operation.h"

enum {
    DONE,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };
static GObjectClass *parent_class = NULL;

/* GObject handlers */
static void seahorse_operation_class_init (SeahorseOperationClass *klass);
static void seahorse_operation_init       (SeahorseOperation *operation);
static void seahorse_operation_dispose    (GObject *gobject);
static void seahorse_operation_finalize   (GObject *gobject);

GType
seahorse_operation_get_type (void)
{
    static GType operation_type = 0;
 
    if (!operation_type) {
        
        static const GTypeInfo operation_info = {
            sizeof (SeahorseOperationClass), NULL, NULL,
            (GClassInitFunc) seahorse_operation_class_init, NULL, NULL,
            sizeof (SeahorseOperation), 0, (GInstanceInitFunc) seahorse_operation_init
        };
        
        operation_type = g_type_register_static (G_TYPE_OBJECT, 
                                "SeahorseOperation", &operation_info, 0);
    }
  
    return operation_type;
}

static void
seahorse_operation_class_init (SeahorseOperationClass *klass)
{
    GObjectClass *gobject_class;
   
    parent_class = g_type_class_peek_parent (klass);
    gobject_class = G_OBJECT_CLASS (klass);
    
    gobject_class->dispose = seahorse_operation_dispose;
    gobject_class->finalize = seahorse_operation_finalize;

    signals[DONE] = g_signal_new ("done", SEAHORSE_TYPE_OPERATION, 
                G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (SeahorseOperationClass, done),
                NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

/* Initialize the object */
static void
seahorse_operation_init (SeahorseOperation *operation)
{
    operation->done = FALSE;
    operation->donefunc = NULL;
    operation->userdata = NULL;
}

/* dispose of all our internal references */
static void
seahorse_operation_dispose (GObject *gobject)
{
    SeahorseOperation *operation;
    operation = SEAHORSE_OPERATION (gobject);
    
    if (!operation->done) 
        seahorse_operation_cancel (operation);
        
    G_OBJECT_CLASS (parent_class)->dispose (gobject);
}

/* free private vars */
static void
seahorse_operation_finalize (GObject *gobject)
{
    SeahorseOperation *operation;
    operation = SEAHORSE_OPERATION (gobject);
    g_assert (operation->done);
        
    G_OBJECT_CLASS (parent_class)->finalize (gobject);
}

SeahorseOperation*  
seahorse_operation_new (SeahorseOperationDone donefunc, gpointer userdata)
{
    SeahorseOperation *operation;
    operation = g_object_new (SEAHORSE_TYPE_OPERATION, NULL);
    operation->donefunc = donefunc;
    operation->userdata = userdata;
    return operation;
}

void                
seahorse_operation_cancel (SeahorseOperation *operation)
{
    g_return_if_fail (SEAHORSE_IS_OPERATION (operation));
    g_return_if_fail (operation->done == FALSE);

	g_object_ref (operation);
    operation->done = TRUE;   
    
    if (operation->donefunc) 
        (operation->donefunc) (operation, TRUE, operation->userdata);

    operation->userdata = NULL;
	g_object_unref (operation);
}

void                
seahorse_operation_done (SeahorseOperation *operation)
{ 
    g_return_if_fail (SEAHORSE_IS_OPERATION (operation));
    g_return_if_fail (operation->done == FALSE);

	g_object_ref (operation);
    operation->done = TRUE;
    
    if (operation->donefunc) 
        (operation->donefunc) (operation, FALSE, operation->userdata);

    operation->userdata = NULL;
    g_object_unref (operation);
}

GSList*             
seahorse_operation_list_add (GSList *list, SeahorseOperation *operation)
{
    /* We assume ownership of the operation, so no need to ref */
    return g_slist_prepend (list, operation);
}

GSList*             
seahorse_operation_list_remove (GSList *list, SeahorseOperation *operation)
{
   GSList *element;
   
   element = g_slist_find (list, operation);
   if (element) {
        g_object_unref (operation);
        list = g_slist_remove_link (list, element);
        g_slist_free (element);
   } 
   
   return list;
}

void             
seahorse_operation_list_cancel (GSList *list)
{
    SeahorseOperation *operation;
    
    while (list) {
        operation = SEAHORSE_OPERATION (list->data);
        list = g_slist_next (list);
        
        if (!seahorse_operation_is_done (operation))
            seahorse_operation_cancel (operation);
    }
}

GSList*             
seahorse_operation_list_purge (GSList *list)
{
    GSList *l, *p;
    
    p = list;
    
    while (p != NULL) {

        l = p;
        p = g_slist_next (p);
        
        if (seahorse_operation_is_done (SEAHORSE_OPERATION (l->data))) {
            g_object_unref (G_OBJECT (l->data));

            list = g_slist_remove_link (list, l);
            g_slist_free (l);
        }         
    }
    
    return list;
}
                                                    

                                                    
GSList*             
seahorse_operation_list_free (GSList *list)
{
    GSList *l;
    
    for (l = list; l; l = g_slist_next (l)) {
        g_assert (SEAHORSE_IS_OPERATION (l->data));
        g_object_unref (G_OBJECT (l->data));
    }
    
    g_slist_free (list);
    return NULL;
}
