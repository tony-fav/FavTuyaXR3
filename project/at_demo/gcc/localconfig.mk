#
# project local config options, override the common config options
#

# ----------------------------------------------------------------------------
# board definition
# ----------------------------------------------------------------------------
__PRJ_CONFIG_BOARD := xr809_module

# ----------------------------------------------------------------------------
# override global config options
# ----------------------------------------------------------------------------
# set chip type: xr871 or xr32
export __CONFIG_CHIP_TYPE := xr809

# ----------------------------------------------------------------------------
# override global config options
# ----------------------------------------------------------------------------
# set y to enable bootloader and disable some features, for bootloader only
# export __CONFIG_BOOTLOADER := y

# set n to disable dual core features, for bootloader only
# export __CONFIG_ARCH_DUAL_CORE := n

# set n to use lwIP 2.x.x, support dual IPv4/IPv6 stack
# export __CONFIG_LWIP_V1 := n

# ----------------------------------------------------------------------------
# override project common config options
# ----------------------------------------------------------------------------
# support both sta and ap, default to n
__PRJ_CONFIG_WLAN_STA_AP := y

# support xplayer, default to n
# __PRJ_CONFIG_XPLAYER := y

# enable XIP, default to n
__PRJ_CONFIG_XIP := y

# set y to link function level's text/rodata/data to ".xip" section
ifeq ($(__PRJ_CONFIG_XIP), y)
export __CONFIG_XIP_SECTION_FUNC_LEVEL := y
endif

# enable OTA, default to n
__PRJ_CONFIG_OTA := y
