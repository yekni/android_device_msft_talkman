obj-$(CONFIG_LUMIA_RIL) += msril.o
obj-$(CONFIG_LUMIA_CELLMUX) += cellmux.o
obj-$(CONFIG_LUMIA_CELL_USERMODE) += cellusermodeinterconnect.o
obj-$(CONFIG_LUMIA_CELL_NETMON) += cellnetmon.o
obj-$(CONFIG_LUMIA_RIL) += lumia_qmi.o
obj-$(CONFIG_LUMIA_RIL) += lumia_ril_core.o

ccflags-y += -I$(srctree)/drivers/net
ccflags-y += -I$(srctree)/drivers/misc/qcom/
ccflags-y += -I$(srctree)/drivers/misc/qcom/qmi_interface/

# Common headers for QMI
ccflags-y += -I$(srctree)/drivers/misc/qcom/qmi_interface/common/v01/
ccflags-y += -I$(srctree)/drivers/misc/qcom/qmi_interface/qmi/v01/

# Module specific flags
EXTRA_CFLAGS += -DCONFIG_MSM_RMNET_BAM 