Utility to modify boot_info partition  
Can be run in linux(_x86) or on gateway itself (_rtkmipsel)

MGL03 boot config tool.

```
Usage: ./mgl03_boot_info [-f file] show
       ./mgl03_boot_info [-f file] gen_default
       ./mgl03_boot_info [-f file] priv_mode <on|off>
       ./mgl03_boot_info [-f file] root_sum <on|off>
       ./mgl03_boot_info [-f file] swap_slot
       ./mgl03_boot_info [-f file] reset_fail
       ./mgl03_boot_info [-f file] set_slot_newest <0|1> <0|1>
       ./mgl03_boot_info [-f file] set_slot_current <0|1> <0|1>
       ./mgl03_boot_info [-f file] set_kernel_0 <size> <chksum>
       ./mgl03_boot_info [-f file] set_rootfs_0 <size> <chksum>
       ./mgl03_boot_info [-f file] set_kernel_1 <size> <chksum>
       ./mgl03_boot_info [-f file] set_rootfs_1 <size> <chksum>
       ./mgl03_boot_info [-f file] set_vernum <value>
       ./mgl03_boot_info [-f file] set_wdog_time <value>
```
