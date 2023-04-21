#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

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
	{ 0x27d11f45, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xfe990052, __VMLINUX_SYMBOL_STR(gpio_free) },
	{ 0x8beca9f0, __VMLINUX_SYMBOL_STR(gpiod_unexport) },
	{ 0xc37f55d2, __VMLINUX_SYMBOL_STR(rtdm_dev_unregister) },
	{ 0x5386e4aa, __VMLINUX_SYMBOL_STR(rtdm_dev_register) },
	{ 0x30b8b192, __VMLINUX_SYMBOL_STR(gpiod_export) },
	{ 0x81e8def4, __VMLINUX_SYMBOL_STR(gpiod_direction_output_raw) },
	{ 0x47229b5c, __VMLINUX_SYMBOL_STR(gpio_request) },
	{ 0xd279365a, __VMLINUX_SYMBOL_STR(__rtdm_task_sleep) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0x28cc25db, __VMLINUX_SYMBOL_STR(arm_copy_from_user) },
	{ 0x7d0b070b, __VMLINUX_SYMBOL_STR(xnheap_free) },
	{ 0xf4fa543b, __VMLINUX_SYMBOL_STR(arm_copy_to_user) },
	{ 0x737bc8f5, __VMLINUX_SYMBOL_STR(xnheap_alloc) },
	{ 0x85c2f068, __VMLINUX_SYMBOL_STR(cobalt_heap) },
	{ 0x79c5a9f0, __VMLINUX_SYMBOL_STR(ioremap) },
	{ 0x9416e1d8, __VMLINUX_SYMBOL_STR(__request_region) },
	{ 0x2ab3cc9d, __VMLINUX_SYMBOL_STR(__release_region) },
	{ 0x85f74b00, __VMLINUX_SYMBOL_STR(iomem_resource) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0x822137e2, __VMLINUX_SYMBOL_STR(arm_heavy_mb) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0xc27ff155, __VMLINUX_SYMBOL_STR(gpiod_set_raw_value) },
	{ 0x4755c2a0, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cdpram");
MODULE_ALIAS("of:N*T*CdpramC*");
