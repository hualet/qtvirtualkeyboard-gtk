/*
 * Generated by gdbus-codegen 2.53.4. DO NOT EDIT.
 *
 * The license of this code is the same as for the source it was derived from.
 */

#ifndef __VIRTUALKEYBOARD_H__
#define __VIRTUALKEYBOARD_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for com.deepin.VirtualKeyboard */

#define TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD (com_deepin_virtual_keyboard_get_type ())
#define COM_DEEPIN_VIRTUAL_KEYBOARD(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD, ComDeepinVirtualKeyboard))
#define IS_COM_DEEPIN_VIRTUAL_KEYBOARD(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD))
#define COM_DEEPIN_VIRTUAL_KEYBOARD_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD, ComDeepinVirtualKeyboardIface))

struct _ComDeepinVirtualKeyboard;
typedef struct _ComDeepinVirtualKeyboard ComDeepinVirtualKeyboard;
typedef struct _ComDeepinVirtualKeyboardIface ComDeepinVirtualKeyboardIface;

struct _ComDeepinVirtualKeyboardIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_hide_keyboard) (
    ComDeepinVirtualKeyboard *object,
    GDBusMethodInvocation *invocation);

  gboolean (*handle_show_keyboard) (
    ComDeepinVirtualKeyboard *object,
    GDBusMethodInvocation *invocation);

  void (*commit) (
    ComDeepinVirtualKeyboard *object,
    const gchar *arg_String);

};

GType com_deepin_virtual_keyboard_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *com_deepin_virtual_keyboard_interface_info (void);
guint com_deepin_virtual_keyboard_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void com_deepin_virtual_keyboard_complete_show_keyboard (
    ComDeepinVirtualKeyboard *object,
    GDBusMethodInvocation *invocation);

void com_deepin_virtual_keyboard_complete_hide_keyboard (
    ComDeepinVirtualKeyboard *object,
    GDBusMethodInvocation *invocation);



/* D-Bus signal emissions functions: */
void com_deepin_virtual_keyboard_emit_commit (
    ComDeepinVirtualKeyboard *object,
    const gchar *arg_String);



/* D-Bus method calls: */
void com_deepin_virtual_keyboard_call_show_keyboard (
    ComDeepinVirtualKeyboard *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean com_deepin_virtual_keyboard_call_show_keyboard_finish (
    ComDeepinVirtualKeyboard *proxy,
    GAsyncResult *res,
    GError **error);

gboolean com_deepin_virtual_keyboard_call_show_keyboard_sync (
    ComDeepinVirtualKeyboard *proxy,
    GCancellable *cancellable,
    GError **error);

void com_deepin_virtual_keyboard_call_hide_keyboard (
    ComDeepinVirtualKeyboard *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean com_deepin_virtual_keyboard_call_hide_keyboard_finish (
    ComDeepinVirtualKeyboard *proxy,
    GAsyncResult *res,
    GError **error);

gboolean com_deepin_virtual_keyboard_call_hide_keyboard_sync (
    ComDeepinVirtualKeyboard *proxy,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY (com_deepin_virtual_keyboard_proxy_get_type ())
#define COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY, ComDeepinVirtualKeyboardProxy))
#define COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY, ComDeepinVirtualKeyboardProxyClass))
#define COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY, ComDeepinVirtualKeyboardProxyClass))
#define IS_COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY))
#define IS_COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_PROXY))

typedef struct _ComDeepinVirtualKeyboardProxy ComDeepinVirtualKeyboardProxy;
typedef struct _ComDeepinVirtualKeyboardProxyClass ComDeepinVirtualKeyboardProxyClass;
typedef struct _ComDeepinVirtualKeyboardProxyPrivate ComDeepinVirtualKeyboardProxyPrivate;

struct _ComDeepinVirtualKeyboardProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  ComDeepinVirtualKeyboardProxyPrivate *priv;
};

struct _ComDeepinVirtualKeyboardProxyClass
{
  GDBusProxyClass parent_class;
};

GType com_deepin_virtual_keyboard_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (ComDeepinVirtualKeyboardProxy, g_object_unref)
#endif

void com_deepin_virtual_keyboard_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
ComDeepinVirtualKeyboard *com_deepin_virtual_keyboard_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
ComDeepinVirtualKeyboard *com_deepin_virtual_keyboard_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void com_deepin_virtual_keyboard_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
ComDeepinVirtualKeyboard *com_deepin_virtual_keyboard_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
ComDeepinVirtualKeyboard *com_deepin_virtual_keyboard_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON (com_deepin_virtual_keyboard_skeleton_get_type ())
#define COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON, ComDeepinVirtualKeyboardSkeleton))
#define COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON, ComDeepinVirtualKeyboardSkeletonClass))
#define COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON, ComDeepinVirtualKeyboardSkeletonClass))
#define IS_COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON))
#define IS_COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_COM_DEEPIN_VIRTUAL_KEYBOARD_SKELETON))

typedef struct _ComDeepinVirtualKeyboardSkeleton ComDeepinVirtualKeyboardSkeleton;
typedef struct _ComDeepinVirtualKeyboardSkeletonClass ComDeepinVirtualKeyboardSkeletonClass;
typedef struct _ComDeepinVirtualKeyboardSkeletonPrivate ComDeepinVirtualKeyboardSkeletonPrivate;

struct _ComDeepinVirtualKeyboardSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  ComDeepinVirtualKeyboardSkeletonPrivate *priv;
};

struct _ComDeepinVirtualKeyboardSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType com_deepin_virtual_keyboard_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (ComDeepinVirtualKeyboardSkeleton, g_object_unref)
#endif

ComDeepinVirtualKeyboard *com_deepin_virtual_keyboard_skeleton_new (void);


G_END_DECLS

#endif /* __VIRTUALKEYBOARD_H__ */
