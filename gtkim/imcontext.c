#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

    // dbus stuff
    GError *kb_error;
    ComDeepinVirtualKeyboard *kb_proxy;
    gulong signal_connection_commit;
    gulong signal_connection_backspace;


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

static GdkEventKey *_create_gdk_event(GdkWindow *window, guint keyval, guint state,
                                      GdkEventType type);
static gboolean _key_is_modifier(guint keyval);

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

    context->kb_proxy = com_deepin_virtual_keyboard_proxy_new_for_bus_sync(
                G_BUS_TYPE_SESSION,
                G_DBUS_PROXY_FLAGS_NONE,
                "com.deepin.VirtualKeyboard",
                "/com/deepin/VirtualKeyboard",
                NULL, &context->kb_error);

    if (context->kb_error != NULL) {
        QVK_WARN("failed to create virtual keyboard proxy instance %s", context->kb_error->message);
        context->kb_error = NULL;
    }

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

    if (context->kb_proxy)
        g_object_unref(context->kb_proxy);

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
    QVK_DEBUG("qvk_im_context_set_client_window: %lu",
              client_window ? GDK_WINDOW_XID(client_window) : 0);

    QVKIMContext *qvkcontext = QVK_IM_CONTEXT(context);
    set_ic_client_window(qvkcontext, client_window);
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
    QVKIMContext *qvkcontext = QVK_IM_CONTEXT(context);

    if (qvkcontext->kb_proxy) {
        com_deepin_virtual_keyboard_call_show_keyboard_sync(
                    qvkcontext->kb_proxy,
                    NULL, &qvkcontext->kb_error);

        if (qvkcontext->kb_error) {
            QVK_WARN("failed call to show keyboard: %s", qvkcontext->kb_error->message);
            qvkcontext->kb_error = NULL;
        }

        qvkcontext->signal_connection_commit =  g_signal_connect(
                    qvkcontext->kb_proxy, "commit",
                    G_CALLBACK(_qvk_im_context_commit_string_cb),
                    context);

        qvkcontext->signal_connection_backspace = g_signal_connect(
                    qvkcontext->kb_proxy, "backspace",
                    G_CALLBACK(_qvk_im_context_backspace_cb),
                    context);
    }

    // some application won't set client window, eg. google-chrome.
    if (!qvkcontext->client_window) {
        set_ic_client_window(qvkcontext,
                             gdk_screen_get_active_window(gdk_screen_get_default()));
    }

    return;
}

static void qvk_im_context_focus_out(GtkIMContext *context) {
    QVK_DEBUG("qvk_im_context_focus_out");
    QVKIMContext *qvkcontext = QVK_IM_CONTEXT(context);

    if (qvkcontext->kb_proxy) {
        g_signal_handler_disconnect(qvkcontext->kb_proxy, qvkcontext->signal_connection_commit);
        g_signal_handler_disconnect(qvkcontext->kb_proxy, qvkcontext->signal_connection_backspace);

        com_deepin_virtual_keyboard_call_hide_keyboard_sync(
                    qvkcontext->kb_proxy,
                    NULL, &qvkcontext->kb_error);

        if (qvkcontext->kb_error) {
            QVK_WARN("failed call to hide keyboard: %s", qvkcontext->kb_error->message);
        }
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
    QVKIMContext *qvkcontext = QVK_IM_CONTEXT(context);

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

    GdkEventKey *event = _create_gdk_event(context->client_window, GDK_KEY_BackSpace, 0, GDK_KEY_PRESS);
    gdk_event_put((GdkEvent *)event);
    gdk_event_free((GdkEvent *)event);

//    if (context->client_window) {
//        gchar cmd[128];
//        sprintf(cmd, "xdotool key --window %lu BackSpace",
//                GDK_WINDOW_XID(gdk_window_get_toplevel(context->client_window)));
//        QVK_WARN("%lu", GDK_WINDOW_XID(gdk_window_get_toplevel(context->client_window)));
//        system(cmd);
//    }
}

/* modification is based on fcitx version */
static GdkEventKey *_create_gdk_event(GdkWindow *window, guint keyval, guint state,
                                      GdkEventType type) {
    gunichar c = 0;
    gchar buf[8];

    GdkEventKey *event = (GdkEventKey *)gdk_event_new(type);

    event->window = g_object_ref(window);
    event->time = GDK_CURRENT_TIME;
    event->send_event = FALSE;
    event->state = state;
    event->keyval = keyval;
    event->string = NULL;
    event->length = 0;
    event->hardware_keycode = 0;
    if (event->window) {
#ifndef NEW_GDK_WINDOW_GET_DISPLAY
        GdkDisplay *display = gdk_display_get_default();
#else
        GdkDisplay *display = gdk_window_get_display(event->window);
#endif
        GdkKeymap *keymap = gdk_keymap_get_for_display(display);
        GdkKeymapKey *keys;
        gint n_keys = 0;

        if (gdk_keymap_get_entries_for_keyval(keymap, keyval, &keys, &n_keys)) {
            if (n_keys)
                event->hardware_keycode = keys[0].keycode;
            g_free(keys);
        }
    }

    event->group = 0;
    event->is_modifier = _key_is_modifier(keyval);

#ifdef DEPRECATED_GDK_KEYSYMS
    if (keyval != GDK_VoidSymbol)
#else
    if (keyval != GDK_KEY_VoidSymbol)
#endif
        c = gdk_keyval_to_unicode(keyval);

    if (c) {
        gsize bytes_written;
        gint len;

        /* Apply the control key - Taken from Xlib
        */
        if (event->state & GDK_CONTROL_MASK) {
            if ((c >= '@' && c < '\177') || c == ' ')
                c &= 0x1F;
            else if (c == '2') {
                event->string = g_memdup("\0\0", 2);
                event->length = 1;
                buf[0] = '\0';
                goto out;
            } else if (c >= '3' && c <= '7')
                c -= ('3' - '\033');
            else if (c == '8')
                c = '\177';
            else if (c == '/')
                c = '_' & 0x1F;
        }

        len = g_unichar_to_utf8(c, buf);
        buf[len] = '\0';

        event->string =
            g_locale_from_utf8(buf, len, NULL, &bytes_written, NULL);
        if (event->string)
            event->length = bytes_written;
#ifdef DEPRECATED_GDK_KEYSYMS
    } else if (keyval == GDK_Escape) {
#else
    } else if (keyval == GDK_KEY_Escape) {
#endif
        event->length = 1;
        event->string = g_strdup("\033");
    }
#ifdef DEPRECATED_GDK_KEYSYMS
    else if (keyval == GDK_Return || keyval == GDK_KP_Enter) {
#else
    else if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
#endif
        event->length = 1;
        event->string = g_strdup("\r");
    }

    if (!event->string) {
        event->length = 0;
        event->string = g_strdup("");
    }
out:
    return event;
}

gboolean _key_is_modifier(guint keyval) {
    /* See gdkkeys-x11.c:_gdk_keymap_key_is_modifier() for how this
    * really should be implemented */

    switch (keyval) {
#ifdef DEPRECATED_GDK_KEYSYMS
    case GDK_Shift_L:
    case GDK_Shift_R:
    case GDK_Control_L:
    case GDK_Control_R:
    case GDK_Caps_Lock:
    case GDK_Shift_Lock:
    case GDK_Meta_L:
    case GDK_Meta_R:
    case GDK_Alt_L:
    case GDK_Alt_R:
    case GDK_Super_L:
    case GDK_Super_R:
    case GDK_Hyper_L:
    case GDK_Hyper_R:
    case GDK_ISO_Lock:
    case GDK_ISO_Level2_Latch:
    case GDK_ISO_Level3_Shift:
    case GDK_ISO_Level3_Latch:
    case GDK_ISO_Level3_Lock:
    case GDK_ISO_Group_Shift:
    case GDK_ISO_Group_Latch:
    case GDK_ISO_Group_Lock:
        return TRUE;
#else
    case GDK_KEY_Shift_L:
    case GDK_KEY_Shift_R:
    case GDK_KEY_Control_L:
    case GDK_KEY_Control_R:
    case GDK_KEY_Caps_Lock:
    case GDK_KEY_Shift_Lock:
    case GDK_KEY_Meta_L:
    case GDK_KEY_Meta_R:
    case GDK_KEY_Alt_L:
    case GDK_KEY_Alt_R:
    case GDK_KEY_Super_L:
    case GDK_KEY_Super_R:
    case GDK_KEY_Hyper_L:
    case GDK_KEY_Hyper_R:
    case GDK_KEY_ISO_Lock:
    case GDK_KEY_ISO_Level2_Latch:
    case GDK_KEY_ISO_Level3_Shift:
    case GDK_KEY_ISO_Level3_Latch:
    case GDK_KEY_ISO_Level3_Lock:
    case GDK_KEY_ISO_Level5_Shift:
    case GDK_KEY_ISO_Level5_Latch:
    case GDK_KEY_ISO_Level5_Lock:
    case GDK_KEY_ISO_Group_Shift:
    case GDK_KEY_ISO_Group_Latch:
    case GDK_KEY_ISO_Group_Lock:
        return TRUE;
#endif
    default:
        return FALSE;
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
