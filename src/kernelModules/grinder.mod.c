#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xa5b87d97, "module_layout" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x944db494, "misc_deregister" },
	{ 0x2aabb777, "gpio_free_array" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0x4ff1b6d1, "gpio_request_array" },
	{ 0x3a3e7139, "misc_register" },
	{ 0x25336af3, "__ipipe_root_status" },
	{ 0x65d37637, "__ipipe_restore_root" },
	{ 0xa8a19a1a, "ipipe_trace_end" },
	{ 0x749410ee, "ipipe_trace_begin" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x37a0cba, "kfree" },
	{ 0xb742fd7, "simple_strtol" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xea147363, "printk" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "8F962FF3DC30B45886FD46D");
