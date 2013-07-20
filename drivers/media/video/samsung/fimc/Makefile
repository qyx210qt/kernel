obj-$(CONFIG_VIDEO_FIMC)	+= fimc_dev.o fimc_v4l2.o fimc_capture.o fimc_output.o \
								fimc_overlay.o fimc_regs.o log_msg.o
obj-$(CONFIG_VIDEO_FIMC_MIPI)	+= csis.o

ifeq ($(CONFIG_CPU_S5PV210),y)
EXTRA_CFLAGS += -DCONFIG_MIPI_CSI_ADV_FEATURE
endif

EXTRA_CFLAGS += -Idrivers/media/video

ifeq ($(CONFIG_VIDEO_FIMC_DEBUG),y)
EXTRA_CFLAGS += -DDEBUG
endif

ifeq ($(wildcard drivers/media/video/samsung/fimc/ov9650.c),)
obj-$(CONFIG_VIDEO_FIMC)		+= tq_auto_detect.module
else
obj-$(CONFIG_VIDEO_FIMC)		+= tq_auto_detect.o
endif

ifeq ($(wildcard drivers/media/video/samsung/fimc/ov9650.c),)
obj-$(CONFIG_CAMERA_OV9650)		+= ov9650.module
else
obj-$(CONFIG_CAMERA_OV9650)		+= ov9650.o
endif

ifeq ($(wildcard drivers/media/video/samsung/fimc/ov3640.c),)
obj-$(CONFIG_CAMERA_OV9650)		+= ov3640.module
else
obj-$(CONFIG_CAMERA_OV3640)		+= ov3640.o
endif

ifeq ($(wildcard drivers/media/video/samsung/fimc/tvp5150.c),)
obj-$(CONFIG_CAMERA_TVP5150)	+= tvp5150.module
else
obj-$(CONFIG_CAMERA_TVP5150)	+= tvp5150.o
endif
ifeq ($(wildcard drivers/media/video/samsung/fimc/saa7113.c),)
obj-$(CONFIG_CAMERA_SAA7113)	+= saa7113.module
else
obj-$(CONFIG_CAMERA_SAA7113)	+= saa7113.o
endif
