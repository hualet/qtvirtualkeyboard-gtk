#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>
#include <string.h>

#include "imcontext.h"

static const GtkIMContextInfo qvk_im_info = {
    "qtvirtualkeyboard", "Qt Virtual Keyboard", "qtvirtualkeyboard-gtk", "/usr/share/locale",
    "ja:ko:zh:*"};

static const GtkIMContextInfo *info_list[] = {&qvk_im_info};

G_MODULE_EXPORT const gchar *
g_module_check_init(GModule *module) {
    return glib_check_version(GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, 0);
}

G_MODULE_EXPORT void im_module_init(GTypeModule *type_module) {
    /* make module resident */
    g_type_module_use(type_module);
    qvk_im_context_register_type(type_module);
}

G_MODULE_EXPORT void im_module_exit(void) {}

G_MODULE_EXPORT GtkIMContext *im_module_create(const gchar *context_id) {
    if (context_id != NULL && strcmp(context_id, "qtvirtualkeyboard") == 0) {
        return (GtkIMContext*)qvk_im_context_new();
    }
    return NULL;
}

G_MODULE_EXPORT void im_module_list(const GtkIMContextInfo ***contexts,
                                    gint *n_contexts) {
    *contexts = info_list;
    *n_contexts = G_N_ELEMENTS(info_list);
}
