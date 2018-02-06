/*
 * Copyright (c) 2017, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <inttypes.h>
#include <stdio.h>

#include "logging.h"

#include "tcti-factory.h"
#ifdef HAVE_TCTI_DEVICE
#include "tcti-device.h"
#endif
#ifdef HAVE_TCTI_SOCKET
#include "tcti-socket.h"
#endif
#include "tcti-dynamic.h"
#include "tcti-type-enum.h"

G_DEFINE_TYPE (TctiFactory, tcti_factory, G_TYPE_OBJECT);
G_DEFINE_QUARK ("tabrmd-tcti-factory-quark", tabrmd_tcti_factory)

enum
{
    PROP_0,
    PROP_TCTI_TYPE,
#ifdef HAVE_TCTI_DEVICE
    PROP_DEVICE_NAME,
#endif
#ifdef HAVE_TCTI_SOCKET
    PROP_SOCKET_ADDRESS,
    PROP_SOCKET_PORT,
#endif
    PROP_FILE_NAME,
    PROP_CONF_STR,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
tcti_factory_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    TctiFactory *self = TCTI_FACTORY (object);

    g_debug ("%s: for TctiFactory 0x%" PRIxPTR, __func__, (uintptr_t)self);
    switch (property_id) {
    case PROP_TCTI_TYPE:
        g_debug ("  PROP_TCTI_TYPE");
        self->tcti_type = g_value_get_enum (value);
        g_debug ("  value: 0x%x", self->tcti_type);
        break;
#ifdef HAVE_TCTI_DEVICE
    case PROP_DEVICE_NAME:
        g_free (self->device_name);
        self->device_name = g_value_dup_string (value);
        g_debug ("TctiFactory set device_name: %s", self->device_name);
        break;
#endif
#ifdef HAVE_TCTI_SOCKET
    case PROP_SOCKET_ADDRESS:
        self->socket_address = g_value_dup_string (value);
        g_debug ("TctiFactory set socket_address: %s", self->socket_address);
        break;
    case PROP_SOCKET_PORT:
        self->socket_port = g_value_get_uint (value);
        g_debug ("TctiFactory set socket_port: %d", self->socket_port);
        break;
#endif
    case PROP_FILE_NAME:
        self->file_name = g_value_dup_string (value);
        g_debug ("%s: PROP_FILE_NAME set to %s", __func__, self->file_name);
        break;
    case PROP_CONF_STR:
        self->conf_str = g_value_dup_string (value);
        g_debug ("%s: PROP_CONF_STR set to %s", __func__, self->conf_str);
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}
static void
tcti_factory_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    TctiFactory *self = TCTI_FACTORY (object);

    g_debug("tcti_factory_get_property");
    switch (property_id) {
    case PROP_TCTI_TYPE:
        g_debug ("  PROP_TCTI_TYPE: 0x%x", self->tcti_type);
        g_value_set_enum (value, self->tcti_type);
        break;
#ifdef HAVE_TCTI_DEVICE
    case PROP_DEVICE_NAME:
        g_value_set_string (value, self->device_name);
        break;
#endif
#ifdef HAVE_TCTI_SOCKET
    case PROP_SOCKET_ADDRESS:
        g_value_set_string (value, self->socket_address);
        break;
    case PROP_SOCKET_PORT:
        g_value_set_uint (value, self->socket_port);
        break;
#endif
    case PROP_FILE_NAME:
        g_debug ("%s: PROP_FILE_NAME set to %s", __func__, self->file_name);
        g_value_set_string (value, self->file_name);
        break;
    case PROP_CONF_STR:
        g_debug ("%s: PROP_CONF_STR set to %s", __func__, self->conf_str);
        g_value_set_string (value, self->conf_str);
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

/* override the parent finalize method so we can free the data associated with
 * the TctiFactory instance.
 */
static void
tcti_factory_finalize (GObject *obj)
{
    TctiFactory *self = TCTI_FACTORY (obj);

    g_clear_pointer (&self->device_name, g_free);
    g_clear_pointer (&self->socket_address, g_free);
    g_clear_pointer (&self->file_name, g_free);
    g_clear_pointer (&self->conf_str, g_free);
    G_OBJECT_CLASS (tcti_factory_parent_class)->finalize (obj);
}
static void
tcti_factory_init (TctiFactory *self)
{ /* noop */ }
/* When the class is initialized we set the pointer to our finalize function.
 */
static void
tcti_factory_class_init (TctiFactoryClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    if (tcti_factory_parent_class == NULL)
        tcti_factory_parent_class = g_type_class_peek_parent (klass);

    object_class->finalize     = tcti_factory_finalize;
    object_class->set_property = tcti_factory_set_property;
    object_class->get_property = tcti_factory_get_property;

    obj_properties[PROP_TCTI_TYPE] =
        g_param_spec_enum ("tcti",
                           "TCTI",
                           "TCTI used by tpm2-abrmd to communicate with the TPM",
                           TYPE_TCTI_TYPE_ENUM,
                           TCTI_TYPE_DEFAULT,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
#ifdef HAVE_TCTI_DEVICE
    obj_properties[PROP_DEVICE_NAME] =
        g_param_spec_string ("device-name",
                             "Device name",
                             "TPM2 device node",
                             TCTI_DEVICE_DEFAULT_FILE,
                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
#endif
#ifdef HAVE_TCTI_SOCKET
    obj_properties[PROP_SOCKET_ADDRESS] =
        g_param_spec_string ("socket-address",
                             "Socket address",
                             "Address for socket TCTI to connect",
                             TCTI_SOCKET_DEFAULT_HOST,
                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    obj_properties[PROP_SOCKET_PORT] =
        g_param_spec_uint ("socket-port",
                           "Port",
                           "Port for socket TCTI connection",
                           0,
                           65535,
                           TCTI_SOCKET_DEFAULT_PORT,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
#endif
    obj_properties[PROP_FILE_NAME] =
        g_param_spec_string ("tcti-file-name",
                             "Name of TCTI library file.",
                             "Name used to find TCTI library, See dlopen(3) for search rules.",
                             TCTI_DYNAMIC_DEFAULT_FILE_NAME,
                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    obj_properties[PROP_CONF_STR] =
        g_param_spec_string ("tcti-conf-str",
                             "TCTI configuration string.",
                             "Configuration string passed to TCTI at time of initialization.",
                             TCTI_DYNAMIC_DEFAULT_CONF_STR,
                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    g_object_class_install_properties (object_class,
                                       N_PROPERTIES,
                                       obj_properties);
}
/* A simple structure we use to map the string name for a TCTI to the
 * associated value from the TctiTypeEnum.
 */
typedef struct tcty_map_entry {
    char* name;
    TctiTypeEnum tcti_type;
} tcti_map_entry_t;

/* This array maps the string name for a TCTI to the enumeration value that
 * we use to identify it.
 */
tcti_map_entry_t tcti_map[] = {
    {
        .name      = "none",
        .tcti_type = TCTI_TYPE_NONE
    },
#ifdef HAVE_TCTI_DEVICE
    {
        .name      = "device",
        .tcti_type = TCTI_TYPE_DEVICE
    },
#endif
#ifdef HAVE_TCTI_SOCKET
    {
        .name      = "socket",
        .tcti_type = TCTI_TYPE_SOCKET
    },
#endif
    {
        .name = "dynamic",
        .tcti_type = TCTI_TYPE_DYNAMIC
    },
};
#define ARRAY_LENGTH(array, type) (sizeof (array) / sizeof (type))

TctiFactory*
tcti_factory_new (void)
{
    return TCTI_FACTORY (g_object_new (TYPE_TCTI_FACTORY, NULL));
}

/* callback used to handl the --tcti= option
 * This is how we determin which TCTI the tabrmd will use to communicate with
 * the TPM.
 */
gboolean
tcti_parse_opt_callback (const gchar   *option_name,
                         const gchar   *value,
                         gpointer       data,
                         GError       **error)
{
    int i;
    gboolean ret = FALSE;
    GValue gvalue = { 0, };
    TctiFactory *self = TCTI_FACTORY (data);
    tcti_map_entry_t *tcti_map_entry;

    g_debug ("tcti_parse_opt_callback");
    for (i = 0; i < ARRAY_LENGTH (tcti_map, tcti_map_entry_t); ++i) {
        tcti_map_entry = &tcti_map[i];
        if (!strcmp (tcti_map_entry->name, value)) {
            g_debug ("tcti name matched: %s", tcti_map_entry->name);
            g_value_init (&gvalue, tcti_type_enum_get_type ());
            g_value_set_enum (&gvalue, tcti_map_entry->tcti_type);
            g_object_set_property (G_OBJECT (self), "tcti", &gvalue);
            ret = TRUE;
        }
    }
    if (ret != TRUE) {
        g_set_error (error,
                     tabrmd_tcti_factory_quark (),
                     1,
                     "unknown TCTI: %s",
                     value);
    }

    return ret;
}

GOptionGroup*
tcti_factory_get_group (TctiFactory *self)
{
    GOptionGroup *group;

    GOptionEntry entries[] = {
        {
            .long_name       = "tcti",
            .short_name      = 't',
            .flags           = G_OPTION_FLAG_NONE,
            .arg             = G_OPTION_ARG_CALLBACK,
            .arg_data        = tcti_parse_opt_callback,
            .description     = "Downstream TCTI",
            .arg_description = "[ dynamic"
#ifdef HAVE_TCTI_DEVICE
            " | device"
#endif
#ifdef HAVE_TCTI_SOCKET
            " | socket"
#endif
            " ]"
        },
#ifdef HAVE_TCTI_DEVICE
        {
            .long_name       = "tcti-device",
            .short_name      = 'd',
            .flags           = G_OPTION_FLAG_NONE,
            .arg             = G_OPTION_ARG_FILENAME,
            .arg_data        = &self->device_name,
            .description     = "TPM2 device node, default is /dev/tpm0",
            .arg_description = "/dev/tpm0"
        },
#endif
#ifdef HAVE_TCTI_SOCKET
        {
            .long_name       = "tcti-socket-address",
            .short_name      = 'a',
            .flags           = G_OPTION_FLAG_NONE,
            .arg             = G_OPTION_ARG_STRING,
            .arg_data        = &self->socket_address,
            .description     = "address for socket TCTI",
            .arg_description = "127.0.0.1"
        },
        {
            .long_name       = "tcti-socket-port",
            .short_name      = 'p',
            .flags           = G_OPTION_FLAG_NONE,
            .arg             = G_OPTION_ARG_INT,
            .arg_data        = &self->socket_port,
            .description     = "port for socket TCTI",
            .arg_description = "0-65535",
        },
#endif
        {
            .long_name       = "tcti-file-name",
            .short_name      = 'i',
            .flags           = G_OPTION_FLAG_NONE,
            .arg             = G_OPTION_ARG_STRING,
            .arg_data        = &self->file_name,
            .description     = "Name of TCTI. See dlopen(3) for search rules.",
            .arg_description = "file-name",
        },
        {
            .long_name       = "tcti-conf-str",
            .short_name      = 'j',
            .flags           = G_OPTION_FLAG_NONE,
            .arg             = G_OPTION_ARG_STRING,
            .arg_data        = &self->conf_str,
            .description     = "Configuration string passed to TCTI at time of initialization.",
            .arg_description = "tcti-conf-str",
        },
        { NULL }
    };

    group = g_option_group_new ("tcti",
                                "TCTI Options",
                                "Show downstream TCTI options",
                                self,
                                NULL);
    if (group == NULL)
        return NULL;
    g_option_group_add_entries (group, entries);

    return group;
}

Tcti*
tcti_factory_get_tcti (TctiFactory *self)
{
    g_debug ("tcti_factory_get_tcti");
    switch (self->tcti_type) {
#ifdef HAVE_TCTI_DEVICE
    case TCTI_TYPE_DEVICE:
        g_assert (self->device_name);
        return TCTI (tcti_device_new (self->device_name));
#endif
#ifdef HAVE_TCTI_SOCKET
    case TCTI_TYPE_SOCKET:
        g_assert (self->socket_address);
        return TCTI (tcti_socket_new (self->socket_address, self->socket_port));
#endif
    case TCTI_TYPE_DYNAMIC:
        g_assert (self->file_name != NULL);
        return TCTI (tcti_dynamic_new (self->file_name, self->conf_str));
    default:
        tabrmd_critical ("unsupported TCTI type");
        break;
    }
}
