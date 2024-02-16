#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0xbd7b29d9, "module_layout" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x37a0cba, "kfree" },
	{ 0x44ff345, "filp_close" },
	{ 0x3501e90a, "blk_mq_free_request" },
	{ 0x187bbd8a, "blk_rq_unmap_user" },
	{ 0xd7c41c9a, "nvme_execute_passthru_rq" },
	{ 0xea244762, "blk_rq_map_kern" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x94223a84, "nvme_alloc_request" },
	{ 0xc934e37d, "I_BDEV" },
	{ 0x92997ed8, "_printk" },
	{ 0x99af41bb, "filp_open" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "8CEEF119CB2B7341A03D80F");
