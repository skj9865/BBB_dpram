#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xf392aa65, "module_layout" },
	{ 0xfe990052, "gpio_free" },
	{ 0x8eb47517, "gpiod_unexport" },
	{ 0x1d302d1c, "rtdm_dev_unregister" },
	{ 0xbb6ced9e, "rtdm_dev_register" },
	{ 0xe9c17489, "gpiod_export" },
	{ 0x92b846d7, "gpiod_direction_output_raw" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xd279365a, "__rtdm_task_sleep" },
	{ 0x5f754e5a, "memset" },
	{ 0x28cc25db, "arm_copy_from_user" },
	{ 0x5905aae5, "xnheap_free" },
	{ 0xf4fa543b, "arm_copy_to_user" },
	{ 0x5fcac5f2, "xnheap_alloc" },
	{ 0x2244b0e1, "cobalt_heap" },
	{ 0xedc03953, "iounmap" },
	{ 0x4384eb42, "__release_region" },
	{ 0xe97c4103, "ioremap" },
	{ 0xae9849dd, "__request_region" },
	{ 0xc94d8e3b, "iomem_resource" },
	{ 0x7c32d0f0, "printk" },
	{ 0x822137e2, "arm_heavy_mb" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xcdc4a1bf, "gpiod_set_raw_value" },
	{ 0xfcbecb1d, "gpio_to_desc" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cdpram");
MODULE_ALIAS("of:N*T*CdpramC*");
