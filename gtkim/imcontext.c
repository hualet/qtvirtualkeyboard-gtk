#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <xkbcommon/xkbcommon-compose.h>

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

#include <gdk/gdkx.h>

#include "imcontext.h"
#include "virtualkeyboard.h"

#if !GTK_CHECK_VERSION(2, 91, 0)
#define DEPRECATED_GDK_KEYSYMS 1
#endif

#if GTK_CHECK_VERSION(2, 24, 0)
#define NEW_GDK_WINDOW_GET_DISPLAY
#endif

#define QVK_DEBUG(...) g_debug(__VA_ARGS__)
#define QVK_WARN(...) g_warning(__VA_ARGS__)

struct _QVKIMContext {
    GtkIMContext parent;

    GdkWindow *client_window;
};

struct _QVKIMContextClass {
    GtkIMContextClass parent;
    /* klass members */
};

/* functions prototype */
static void qvk_im_context_class_init(QVKIMContextClass *klass);
static void qvk_im_context_class_fini(QVKIMContextClass *klass);
static void qvk_im_context_init(QVKIMContext *im_context);
static void qvk_im_context_finalize(GObject *obj);
static void qvk_im_context_set_client_window(GtkIMContext *context,
                                             GdkWindow *client_window);
static gboolean qvk_im_context_filter_keypress(GtkIMContext *context,
                                               GdkEventKey *key);
static void qvk_im_context_reset(GtkIMContext *context);
static void qvk_im_context_focus_in(GtkIMContext *context);
static void qvk_im_context_focus_out(GtkIMContext *context);
static void qvk_im_context_set_cursor_location(GtkIMContext *context,
                                               GdkRectangle *area);
static void qvk_im_context_set_use_preedit(GtkIMContext *context,
                                           gboolean use_preedit);
static void qvk_im_context_set_surrounding(GtkIMContext *context,
                                           const gchar *text, gint len,
                                           gint cursor_index);
static void qvk_im_context_get_preedit_string(GtkIMContext *context,
                                              gchar **str,
                                              PangoAttrList **attrs,
                                              gint *cursor_pos);

static void _qvk_im_context_commit_string_cb(GObject *gobject,
                                             const gchar *string,
                                             void* user_data);

static void _qvk_im_context_backspace_cb(GObject *gobject,
                                         void* user_data);

#if GTK_CHECK_VERSION(3, 6, 0)

static void _qvk_im_context_input_hints_changed_cb(GObject *gobject,
                                                   GParamSpec *pspec,
                                                   gpointer user_data);
static void _qvk_im_context_input_purpose_changed_cb(GObject *gobject,
                                                     GParamSpec *pspec,
                                                     gpointer user_data);
#endif

static GType _qvk_type_im_context = 0;
static GtkIMContextClass *parent_class = NULL;

static guint _signal_commit_id = 0;
static guint _signal_preedit_changed_id = 0;
static guint _signal_preedit_start_id = 0;
static guint _signal_preedit_end_id = 0;
static guint _signal_delete_surrounding_id = 0;
static guint _signal_retrieve_surrounding_id = 0;
static gboolean _use_sync_mode = 0;

static GtkIMContext *_focus_im_context = NULL;

// dbus stuff
static ComDeepinVirtualKeyboard *kb_proxy = NULL;
static GError *kb_error = NULL;

void qvk_im_context_register_type(GTypeModule *type_module) {
    static const GTypeInfo qvk_im_context_info = {
        sizeof(QVKIMContextClass),
        (GBaseInitFunc)NULL,
        (GBaseFinalizeFunc)NULL,
        (GClassInitFunc)qvk_im_context_class_init,
        (GClassFinalizeFunc)qvk_im_context_class_fini,
        NULL, /* klass data */
        sizeof(QVKIMContext),
        0,
        (GInstanceInitFunc)qvk_im_context_init,
        0};

    if (!_qvk_type_im_context) {
        if (type_module) {
            _qvk_type_im_context = g_type_module_register_type(
                        type_module, GTK_TYPE_IM_CONTEXT, "QVKIMContext",
                        &qvk_im_context_info, (GTypeFlags)0);
        } else {
            _qvk_type_im_context =
                    g_type_register_static(GTK_TYPE_IM_CONTEXT, "QVKIMContext",
                                           &qvk_im_context_info, (GTypeFlags)0);
        }
    }
}

GType qvk_im_context_get_type(void) {
    if (_qvk_type_im_context == 0) {
        qvk_im_context_register_type(NULL);
    }

    g_assert(_qvk_type_im_context != 0);
    return _qvk_type_im_context;
}

QVKIMContext *qvk_im_context_new(void) {
    GObject *obj = g_object_new(QVK_TYPE_IM_CONTEXT, NULL);
    return QVK_IM_CONTEXT(obj);
}

///
static void qvk_im_context_class_init(QVKIMContextClass *klass) {
    GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS(klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    parent_class = (GtkIMContextClass *)g_type_class_peek_parent(klass);

    im_context_class->set_client_window = qvk_im_context_set_client_window;
    im_context_class->filter_keypress = qvk_im_context_filter_keypress;
    im_context_class->reset = qvk_im_context_reset;
    im_context_class->get_preedit_string = qvk_im_context_get_preedit_string;
    im_context_class->focus_in = qvk_im_context_focus_in;
    im_context_class->focus_out = qvk_im_context_focus_out;
    im_context_class->set_cursor_location =
            qvk_im_context_set_cursor_location;
    im_context_class->set_use_preedit = qvk_im_context_set_use_preedit;
    im_context_class->set_surrounding = qvk_im_context_set_surrounding;
    gobject_class->finalize = qvk_im_context_finalize;

    _signal_commit_id = g_signal_lookup("commit", G_TYPE_FROM_CLASS(klass));
    g_assert(_signal_commit_id != 0);

    _signal_preedit_changed_id =
            g_signal_lookup("preedit-changed", G_TYPE_FROM_CLASS(klass));
    g_assert(_signal_preedit_changed_id != 0);

    _signal_preedit_start_id =
            g_signal_lookup("preedit-start", G_TYPE_FROM_CLASS(klass));
    g_assert(_signal_preedit_start_id != 0);

    _signal_preedit_end_id =
            g_signal_lookup("preedit-end", G_TYPE_FROM_CLASS(klass));
    g_assert(_signal_preedit_end_id != 0);

    _signal_delete_surrounding_id =
            g_signal_lookup("delete-surrounding", G_TYPE_FROM_CLASS(klass));
    g_assert(_signal_delete_surrounding_id != 0);

    _signal_retrieve_surrounding_id =
            g_signal_lookup("retrieve-surrounding", G_TYPE_FROM_CLASS(klass));
    g_assert(_signal_retrieve_surrounding_id != 0);
}

static void qvk_im_context_class_fini(QVKIMContextClass *klass) {

}

static void qvk_im_context_init(QVKIMContext *context) {
    QVK_DEBUG("qvk_im_context_init");

    kb_proxy = com_deepin_virtual_keyboard_proxy_new_for_bus_sync(
                G_BUS_TYPE_SESSION,
                G_DBUS_PROXY_FLAGS_NONE,
                "com.deepin.VirtualKeyboard",
                "/com/deepin/VirtualKeyboard",
                NULL, &kb_error);

    if (kb_error != NULL) {
        QVK_WARN("failed to create virtual keyboard proxy instance %s", kb_error->message);
        kb_error = NULL;
    }

    g_signal_connect(kb_proxy, "commit",
                     G_CALLBACK(_qvk_im_context_commit_string_cb),
                     context);

    g_signal_connect(kb_proxy, "backspace",
                     G_CALLBACK(_qvk_im_context_backspace_cb),
                     context);

#if GTK_CHECK_VERSION(3, 6, 0)
    g_signal_connect(context, "notify::input-hints",
                     G_CALLBACK(_qvk_im_context_input_hints_changed_cb),
                     NULL);
    g_signal_connect(context, "notify::input-purpose",
                     G_CALLBACK(_qvk_im_context_input_purpose_changed_cb),
                     NULL);
#endif
}

static void qvk_im_context_finalize(GObject *obj) {
    QVK_DEBUG("qvk_im_context_finalize");
    QVKIMContext *context = QVK_IM_CONTEXT(obj);

    qvk_im_context_set_client_window(GTK_IM_CONTEXT(context), NULL);

    g_object_unref(kb_proxy);

    G_OBJECT_CLASS(parent_class)->finalize(obj);
}

static void set_ic_client_window(QVKIMContext *context,
                                 GdkWindow *client_window) {
    if (!client_window)
        return;

    if (context->client_window) {
        g_object_unref(context->client_window);
        context->client_window = NULL;
    }

    if (client_window != NULL)
        context->client_window = g_object_ref(client_window);
}

///
static void qvk_im_context_set_client_window(GtkIMContext *context,
                                             GdkWindow *client_window) {
    QVK_DEBUG("qvk_im_context_set_client_window");
    QVKIMContext *fcitxcontext = QVK_IM_CONTEXT(context);
    set_ic_client_window(fcitxcontext, client_window);
}

///
static gboolean qvk_im_context_filter_keypress(GtkIMContext *context,
                                               GdkEventKey *event) {
    QVK_DEBUG("qvk_im_context_filter_keypress");

    return FALSE;
}

///
static void qvk_im_context_focus_in(GtkIMContext *context) {
    QVK_DEBUG("qvk_im_context_focus_in");

    com_deepin_virtual_keyboard_call_show_keyboard_sync(
                kb_proxy,
                NULL, &kb_error);

    if (kb_error) {
        QVK_WARN("failed call to show keyboard: %s", kb_error->message);
    }

    return;
}

static void qvk_im_context_focus_out(GtkIMContext *context) {
    QVK_DEBUG("qvk_im_context_focus_out");

    com_deepin_virtual_keyboard_call_hide_keyboard_sync(
                kb_proxy,
                NULL, &kb_error);

    if (kb_error) {
        QVK_WARN("failed call to hide keyboard: %s", kb_error->message);
    }

    return;
}

///
static void qvk_im_context_set_cursor_location(GtkIMContext *context,
                                               GdkRectangle *area) {
    QVK_DEBUG("qvk_im_context_set_cursor_location %d %d %d %d", area->x,
              area->y, area->height, area->width);

    return;
}

///
static void qvk_im_context_set_use_preedit(GtkIMContext *context,
                                           gboolean use_preedit) {
    QVK_DEBUG("qvk_im_context_set_use_preedit");

    return;
}

static void qvk_im_context_set_surrounding(GtkIMContext *context,
                                           const gchar *text, gint l,
                                           gint cursor_index) {
    g_return_if_fail(context != NULL);
    g_return_if_fail(QVK_IS_IM_CONTEXT(context));
    g_return_if_fail(text != NULL);

    gint len = l;
    if (len < 0) {
        len = strlen(text);
    }

    g_return_if_fail(0 <= cursor_index && cursor_index <= len);

    return;
}

///
static void qvk_im_context_reset(GtkIMContext *context) {
    QVK_DEBUG("qvk_im_context_reset");

    return;
}

static void qvk_im_context_get_preedit_string(GtkIMContext *context,
                                              gchar **str,
                                              PangoAttrList **attrs,
                                              gint *cursor_pos) {
    QVK_DEBUG("qvk_im_context_get_preedit_string");
    QVKIMContext *fcitxcontext = QVK_IM_CONTEXT(context);

    if (str) {
        // TODO
        *str = strdup("");
    }

    if (attrs) {
        *attrs = pango_attr_list_new();
    }

    if (cursor_pos)
        *cursor_pos = 0;

    return;
}

void _qvk_im_context_commit_string_cb(GObject *gobject,
                                      const gchar *string,
                                      void *user_data)
{
    QVK_DEBUG("callback commit string: %s", string);

    QVKIMContext *context = QVK_IM_CONTEXT(user_data);
    if (context) {
        g_signal_emit(context, _signal_commit_id, 0, string);
    }
}

void _qvk_im_context_backspace_cb(GObject *gobject,
                                  void *user_data)
{
    QVK_DEBUG("callback backspace.");

    QVKIMContext *context = QVK_IM_CONTEXT(user_data);
    if (context) {
        gboolean return_value;
        g_signal_emit(context, _signal_delete_surrounding_id, 0, -1, 1, &return_value);
    }
}


#if GTK_CHECK_VERSION(3, 6, 0)

void _qvk_im_context_input_purpose_changed_cb(GObject *gobject,
                                              GParamSpec *pspec,
                                              gpointer user_data) {
}

void _qvk_im_context_input_hints_changed_cb(GObject *gobject,
                                            GParamSpec *pspec,
                                            gpointer user_data) {
}

#endif

// kate: indent-mode cstyle; replace-tabs on;
