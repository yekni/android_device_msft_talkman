#ifndef _LUMIA_RIL_CORE_H_
#define _LUMIA_RIL_CORE_H_

#include <linux/types.h>
#include "lumia_qmi.h"

/* RIL states */
enum lumia_ril_state {
    LUMIA_RIL_STATE_UNKNOWN,
    LUMIA_RIL_STATE_OFF,
    LUMIA_RIL_STATE_INITIALIZING,
    LUMIA_RIL_STATE_READY,
    LUMIA_RIL_STATE_ERROR
};

/* RIL events */
enum lumia_ril_event {
    LUMIA_RIL_EVENT_NONE,
    LUMIA_RIL_EVENT_MODEM_READY,
    LUMIA_RIL_EVENT_SIM_READY,
    LUMIA_RIL_EVENT_NETWORK_STATUS,
    LUMIA_RIL_EVENT_SIGNAL_STRENGTH,
    LUMIA_RIL_EVENT_ERROR
};

/* RIL callback function type */
typedef void (*lumia_ril_callback)(enum lumia_ril_event event, void *data, void *context);

/* RIL operations structure */
struct lumia_ril_ops {
    int (*init)(void);
    void (*cleanup)(void);
    int (*register_callback)(lumia_ril_callback cb, void *context);
    int (*unregister_callback)(lumia_ril_callback cb);
    int (*send_qmi)(u16 service_id, u16 msg_id, const void *data, size_t len);
    int (*get_sim_status)(struct lumia_card_status *status);
    int (*get_signal_strength)(struct lumia_signal_strength *signal);
    int (*get_system_info)(struct lumia_system_info *info);
};

/* RIL registration/unregistration */
extern int lumia_ril_register(const struct lumia_ril_ops *ops);
extern void lumia_ril_unregister(void);

/* RIL core functions */
extern int lumia_ril_send_event(enum lumia_ril_event event, void *data);
extern int lumia_ril_get_state(void);

/* RIL module specific functions */
extern int msril_init(void);
extern void msril_exit(void);
extern int cellmux_init(void);
extern void cellmux_exit(void);
extern int cellusermode_init(void);
extern void cellusermode_exit(void);
extern int cellnetmon_init(void);
extern void cellnetmon_exit(void);

#endif /* _LUMIA_RIL_CORE_H_ */ 