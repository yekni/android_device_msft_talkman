#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/slab.h>
#include "lumia_ril_core.h"

#define MAX_CALLBACKS 32

struct ril_callback_info {
    struct list_head list;
    lumia_ril_callback callback;
    void *context;
};

static struct {
    struct mutex lock;
    enum lumia_ril_state state;
    const struct lumia_ril_ops *ops;
    struct list_head callbacks;
    int num_callbacks;
} ril_core;

/* RIL registration/unregistration */
int lumia_ril_register(const struct lumia_ril_ops *ops)
{
    if (!ops)
        return -EINVAL;

    mutex_lock(&ril_core.lock);
    if (ril_core.ops) {
        mutex_unlock(&ril_core.lock);
        return -EBUSY;
    }

    ril_core.ops = ops;
    ril_core.state = LUMIA_RIL_STATE_INITIALIZING;
    mutex_unlock(&ril_core.lock);

    if (ops->init)
        return ops->init();

    return 0;
}
EXPORT_SYMBOL(lumia_ril_register);

void lumia_ril_unregister(void)
{
    struct ril_callback_info *cb, *tmp;

    mutex_lock(&ril_core.lock);
    if (ril_core.ops && ril_core.ops->cleanup)
        ril_core.ops->cleanup();

    ril_core.ops = NULL;
    ril_core.state = LUMIA_RIL_STATE_OFF;

    /* Clean up callbacks */
    list_for_each_entry_safe(cb, tmp, &ril_core.callbacks, list) {
        list_del(&cb->list);
        kfree(cb);
    }
    ril_core.num_callbacks = 0;

    mutex_unlock(&ril_core.lock);
}
EXPORT_SYMBOL(lumia_ril_unregister);

/* RIL core functions */
int lumia_ril_send_event(enum lumia_ril_event event, void *data)
{
    struct ril_callback_info *cb;

    mutex_lock(&ril_core.lock);
    list_for_each_entry(cb, &ril_core.callbacks, list) {
        cb->callback(event, data, cb->context);
    }
    mutex_unlock(&ril_core.lock);

    return 0;
}
EXPORT_SYMBOL(lumia_ril_send_event);

int lumia_ril_get_state(void)
{
    return ril_core.state;
}
EXPORT_SYMBOL(lumia_ril_get_state);

/* Callback registration */
static int ril_register_callback(lumia_ril_callback cb, void *context)
{
    struct ril_callback_info *info;

    if (!cb)
        return -EINVAL;

    mutex_lock(&ril_core.lock);
    if (ril_core.num_callbacks >= MAX_CALLBACKS) {
        mutex_unlock(&ril_core.lock);
        return -ENOSPC;
    }

    info = kzalloc(sizeof(*info), GFP_KERNEL);
    if (!info) {
        mutex_unlock(&ril_core.lock);
        return -ENOMEM;
    }

    info->callback = cb;
    info->context = context;
    list_add_tail(&info->list, &ril_core.callbacks);
    ril_core.num_callbacks++;

    mutex_unlock(&ril_core.lock);
    return 0;
}

static int ril_unregister_callback(lumia_ril_callback cb)
{
    struct ril_callback_info *info, *tmp;
    int found = 0;

    mutex_lock(&ril_core.lock);
    list_for_each_entry_safe(info, tmp, &ril_core.callbacks, list) {
        if (info->callback == cb) {
            list_del(&info->list);
            kfree(info);
            ril_core.num_callbacks--;
            found = 1;
            break;
        }
    }
    mutex_unlock(&ril_core.lock);

    return found ? 0 : -ENOENT;
}

/* Module init/exit */
static int __init lumia_ril_core_init(void)
{
    mutex_init(&ril_core.lock);
    INIT_LIST_HEAD(&ril_core.callbacks);
    ril_core.state = LUMIA_RIL_STATE_OFF;
    ril_core.num_callbacks = 0;

    pr_info("Lumia RIL Core initialized\n");
    return 0;
}

static void __exit lumia_ril_core_exit(void)
{
    lumia_ril_unregister();
    pr_info("Lumia RIL Core removed\n");
}

module_init(lumia_ril_core_init);
module_exit(lumia_ril_core_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Android4Lumia");
MODULE_DESCRIPTION("Microsoft Lumia RIL Core Driver");
MODULE_VERSION("1.0"); 