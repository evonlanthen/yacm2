#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xa5b87d97, "module_layout" },
	{ 0xadf42bd5, "__request_region" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x788fe103, "iomem_resource" },
	{ 0x97255bdf, "strlen" },
	{ 0x6746cc8c, "no_llseek" },
	{ 0x20000329, "simple_strtoul" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x3a3e7139, "misc_register" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xea147363, "printk" },
	{ 0x9bce482f, "__release_region" },
	{ 0x45a55ec8, "__iounmap" },
	{ 0x40a6f522, "__arm_ioremap" },
	{ 0x944db494, "misc_deregister" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "CCE35C1FC2372C0CCA86402");
