#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/qmi_encdec.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include "lumia_ril_core.h"
#include "lumia_qmi.h"

#define DEVICE_NAME "msril"
#define CLASS_NAME "lumia_ril"

struct msril_dev {
    struct qmi_handle qmi;
    struct work_struct qmi_work;
    struct workqueue_struct *qmi_wq;
    struct mutex lock;
    wait_queue_head_t wait;
    bool qmi_ready;
};

static struct msril_dev *msril_data;
static int major;
static struct class *ril_class;
static struct device *ril_device;
static struct cdev ril_cdev;

/* QMI message handlers */
static void msril_qmi_modem_ready_cb(struct qmi_handle *qmi, 
                                    struct sockaddr_qrtr *sq,
                                    struct qmi_txn *txn, const void *data)
{
    struct msril_dev *dev = container_of(qmi, struct msril_dev, qmi);
    
    mutex_lock(&dev->lock);
    dev->qmi_ready = true;
    mutex_unlock(&dev->lock);
    
    lumia_ril_send_event(LUMIA_RIL_EVENT_MODEM_READY, NULL);
    wake_up_interruptible(&dev->wait);
}

static void msril_qmi_sim_status_cb(struct qmi_handle *qmi,
                                   struct sockaddr_qrtr *sq,
                                   struct qmi_txn *txn, const void *data)
{
    struct lumia_card_status status;
    
    if (qmi_decode_message(data, &status, lumia_card_status_ei) == 0) {
        lumia_ril_send_event(LUMIA_RIL_EVENT_SIM_READY, &status);
    }
}

static struct qmi_msg_handler msril_qmi_handlers[] = {
    {
        .type = QMI_INDICATION,
        .msg_id = QMI_LUMIA_RIL_SERVICE_ID,
        .ei = NULL,
        .decoded_size = 0,
        .fn = msril_qmi_modem_ready_cb
    },
    {
        .type = QMI_RESPONSE,
        .msg_id = QMI_LUMIA_RIL_GET_CARD_STATUS,
        .ei = lumia_card_status_ei,
        .decoded_size = sizeof(struct lumia_card_status),
        .fn = msril_qmi_sim_status_cb
    },
    {}
};

/* RIL Operations */
static int msril_init(void)
{
    int ret;

    ret = qmi_handle_init(&msril_data->qmi, sizeof(struct qmi_elem_info),
                         NULL, msril_qmi_handlers);
    if (ret < 0) {
        pr_err("MSRIL: Failed to initialize QMI handle\n");
        return ret;
    }

    msril_data->qmi_wq = create_singlethread_workqueue("msril_qmi");
    if (!msril_data->qmi_wq) {
        qmi_handle_release(&msril_data->qmi);
        return -ENOMEM;
    }

    return 0;
}

static void msril_cleanup(void)
{
    if (msril_data->qmi_wq) {
        destroy_workqueue(msril_data->qmi_wq);
        msril_data->qmi_wq = NULL;
    }
    qmi_handle_release(&msril_data->qmi);
}

static int msril_send_qmi(u16 service_id, u16 msg_id, const void *data, size_t len)
{
    struct qmi_txn txn;
    int ret;

    if (!msril_data->qmi_ready)
        return -EAGAIN;

    ret = qmi_txn_init(&msril_data->qmi, &txn, NULL, NULL);
    if (ret < 0)
        return ret;

    ret = qmi_send_request(&msril_data->qmi, NULL, &txn,
                          msg_id, len, data, NULL, NULL);
    if (ret < 0) {
        qmi_txn_cancel(&txn);
        return ret;
    }

    return 0;
}

static int msril_get_sim_status(struct lumia_card_status *status)
{
    struct qmi_txn txn;
    int ret;

    ret = qmi_txn_init(&msril_data->qmi, &txn, lumia_card_status_ei, status);
    if (ret < 0)
        return ret;

    ret = qmi_send_request(&msril_data->qmi, NULL, &txn,
                          QMI_LUMIA_RIL_GET_CARD_STATUS, 0, NULL,
                          NULL, NULL);
    if (ret < 0) {
        qmi_txn_cancel(&txn);
        return ret;
    }

    ret = qmi_txn_wait(&txn, 5 * HZ);
    if (ret < 0) {
        qmi_txn_cancel(&txn);
        return ret;
    }

    return 0;
}

static int msril_get_signal_strength(struct lumia_signal_strength *signal)
{
    struct qmi_txn txn;
    int ret;

    ret = qmi_txn_init(&msril_data->qmi, &txn, lumia_signal_strength_ei, signal);
    if (ret < 0)
        return ret;

    ret = qmi_send_request(&msril_data->qmi, NULL, &txn,
                          QMI_LUMIA_NAS_GET_SIGNAL_STRENGTH, 0, NULL,
                          NULL, NULL);
    if (ret < 0) {
        qmi_txn_cancel(&txn);
        return ret;
    }

    ret = qmi_txn_wait(&txn, 5 * HZ);
    if (ret < 0) {
        qmi_txn_cancel(&txn);
        return ret;
    }

    return 0;
}

static int msril_get_system_info(struct lumia_system_info *info)
{
    struct qmi_txn txn;
    int ret;

    ret = qmi_txn_init(&msril_data->qmi, &txn, lumia_system_info_ei, info);
    if (ret < 0)
        return ret;

    ret = qmi_send_request(&msril_data->qmi, NULL, &txn,
                          QMI_LUMIA_NAS_GET_SYSTEM_INFO, 0, NULL,
                          NULL, NULL);
    if (ret < 0) {
        qmi_txn_cancel(&txn);
        return ret;
    }

    ret = qmi_txn_wait(&txn, 5 * HZ);
    if (ret < 0) {
        qmi_txn_cancel(&txn);
        return ret;
    }

    return 0;
}

static const struct lumia_ril_ops msril_ops = {
    .init = msril_init,
    .cleanup = msril_cleanup,
    .send_qmi = msril_send_qmi,
    .get_sim_status = msril_get_sim_status,
    .get_signal_strength = msril_get_signal_strength,
    .get_system_info = msril_get_system_info,
};

/* File operations */
static int msril_open(struct inode *inode, struct file *file)
{
    pr_info("MSRIL: Device opened\n");
    return 0;
}

static int msril_release(struct inode *inode, struct file *file)
{
    pr_info("MSRIL: Device closed\n");
    return 0;
}

static ssize_t msril_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    pr_debug("MSRIL: Read operation\n");
    return 0;
}

static ssize_t msril_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    pr_debug("MSRIL: Write operation\n");
    return count;
}

static const struct file_operations msril_fops = {
    .owner = THIS_MODULE,
    .open = msril_open,
    .release = msril_release,
    .read = msril_read,
    .write = msril_write,
};

/* Module init and exit */
static int __init msril_module_init(void)
{
    int ret;

    /* Allocate device data */
    msril_data = kzalloc(sizeof(*msril_data), GFP_KERNEL);
    if (!msril_data)
        return -ENOMEM;

    /* Initialize device data */
    mutex_init(&msril_data->lock);
    init_waitqueue_head(&msril_data->wait);
    msril_data->qmi_ready = false;

    /* Register character device */
    ret = alloc_chrdev_region(&major, 0, 1, DEVICE_NAME);
    if (ret < 0)
        goto err_free_data;

    ril_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ril_class)) {
        ret = PTR_ERR(ril_class);
        goto err_unreg_chrdev;
    }

    ril_device = device_create(ril_class, NULL, major, NULL, DEVICE_NAME);
    if (IS_ERR(ril_device)) {
        ret = PTR_ERR(ril_device);
        goto err_destroy_class;
    }

    cdev_init(&ril_cdev, &msril_fops);
    ret = cdev_add(&ril_cdev, major, 1);
    if (ret < 0)
        goto err_destroy_device;

    /* Register with RIL core */
    ret = lumia_ril_register(&msril_ops);
    if (ret < 0)
        goto err_del_cdev;

    pr_info("MSRIL: Module initialized successfully\n");
    return 0;

err_del_cdev:
    cdev_del(&ril_cdev);
err_destroy_device:
    device_destroy(ril_class, major);
err_destroy_class:
    class_destroy(ril_class);
err_unreg_chrdev:
    unregister_chrdev_region(major, 1);
err_free_data:
    kfree(msril_data);
    return ret;
}

static void __exit msril_module_exit(void)
{
    lumia_ril_unregister();
    cdev_del(&ril_cdev);
    device_destroy(ril_class, major);
    class_destroy(ril_class);
    unregister_chrdev_region(major, 1);
    kfree(msril_data);
    pr_info("MSRIL: Module removed\n");
}

module_init(msril_module_init);
module_exit(msril_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Android4Lumia");
MODULE_DESCRIPTION("Microsoft Lumia RIL Driver");
MODULE_VERSION("1.0"); 