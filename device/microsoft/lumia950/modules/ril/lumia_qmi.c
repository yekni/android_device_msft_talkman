#include <linux/module.h>
#include <linux/qmi_encdec.h>
#include "lumia_qmi.h"

/* QMI element info arrays for message encoding/decoding */
struct qmi_elem_info lumia_signal_strength_ei[] = {
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_signal_strength, signal_strength),
    },
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_signal_strength, radio_interface),
    },
    {}
};
EXPORT_SYMBOL(lumia_signal_strength_ei);

struct qmi_elem_info lumia_system_info_ei[] = {
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_system_info, registration_state),
    },
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_system_info, cs_attach_state),
    },
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_system_info, ps_attach_state),
    },
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_system_info, network_type),
    },
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_system_info, roaming_indicator),
    },
    {}
};
EXPORT_SYMBOL(lumia_system_info_ei);

struct qmi_elem_info lumia_card_status_ei[] = {
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_card_status, card_state),
    },
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_card_status, upin_state),
    },
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_card_status, upuk_retries),
    },
    {
        .data_type = QMI_UNSIGNED_1_BYTE,
        .elem_len  = 1,
        .elem_size = sizeof(__u8),
        .offset    = offsetof(struct lumia_card_status, upin_retries),
    },
    {}
};
EXPORT_SYMBOL(lumia_card_status_ei); 