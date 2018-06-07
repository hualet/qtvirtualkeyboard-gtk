#ifndef IM_CONTEXT_H_
#define IM_CONTEXT_H_

#include <gtk/gtk.h>

#define QVK_TYPE_IM_CONTEXT (qvk_im_context_get_type())
#define QVK_IM_CONTEXT(obj)                                                  \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), QVK_TYPE_IM_CONTEXT, QVKIMContext))
#define QVK_IM_CONTEXT_CLASS(klass)                                          \
    (G_TYPE_CHECK_CLASS_CAST((klass), QVK_TYPE_IM_CONTEXT,                   \
    QVKIMContextClass))
#define QVK_IS_IM_CONTEXT(obj)                                               \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), QVK_TYPE_IM_CONTEXT))
#define QVK_IS_IM_CONTEXT_CLASS(klass)                                       \
    (G_TYPE_CHECK_CLASS_TYPE((klass), QVK_TYPE_IM_CONTEXT))
#define QVK_IM_CONTEXT_GET_CLASS(obj)                                        \
    (G_TYPE_CHECK_GET_CLASS((obj), QVK_TYPE_IM_CONTEXT, QVKIMContextClass))

G_BEGIN_DECLS

typedef struct _QVKIMContext QVKIMContext;
typedef struct _QVKIMContextClass QVKIMContextClass;

GType qvk_im_context_get_type(void);
QVKIMContext *qvk_im_context_new(void);

void im_context_register_type(GTypeModule *type_module);

G_END_DECLS
#endif
