#ifndef _LUMIA_QMI_H_
#define _LUMIA_QMI_H_

#include <linux/qmi_encdec.h>

/* QMI service IDs */
#define QMI_LUMIA_RIL_SERVICE_ID    0x0F
#define QMI_LUMIA_NAS_SERVICE_ID    0x10
#define QMI_LUMIA_WMS_SERVICE_ID    0x11
#define QMI_LUMIA_PBM_SERVICE_ID    0x12

/* QMI message IDs */
#define QMI_LUMIA_NAS_GET_SIGNAL_STRENGTH    0x20
#define QMI_LUMIA_NAS_GET_SYSTEM_INFO       0x21
#define QMI_LUMIA_NAS_NETWORK_STATUS        0x22
#define QMI_LUMIA_RIL_GET_CARD_STATUS       0x23
#define QMI_LUMIA_RIL_GET_SLOT_STATUS       0x24

/* QMI message structures */
struct lumia_signal_strength {
    __u8 signal_strength;
    __u8 radio_interface;
} __packed;

struct lumia_system_info {
    __u8 registration_state;
    __u8 cs_attach_state;
    __u8 ps_attach_state;
    __u8 network_type;
    __u8 roaming_indicator;
} __packed;

struct lumia_card_status {
    __u8 card_state;
    __u8 upin_state;
    __u8 upuk_retries;
    __u8 upin_retries;
} __packed;

/* QMI message element info arrays */
extern struct qmi_elem_info lumia_signal_strength_ei[];
extern struct qmi_elem_info lumia_system_info_ei[];
extern struct qmi_elem_info lumia_card_status_ei[];

#endif /* _LUMIA_QMI_H_ */ 