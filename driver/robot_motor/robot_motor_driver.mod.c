#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

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



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xc1e6c71e, "__mutex_init" },
	{ 0x9f222e1e, "alloc_chrdev_region" },
	{ 0x4c075f7d, "cdev_init" },
	{ 0x6459621a, "cdev_add" },
	{ 0xb6c08e4c, "class_create" },
	{ 0xf350d701, "device_create" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0x546c19d9, "validate_usercopy_range" },
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x40a621c5, "snprintf" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0x102ec640, "platform_device_unregister" },
	{ 0xea97e6f1, "platform_driver_unregister" },
	{ 0x357aaab3, "mutex_trylock" },
	{ 0xacbfeb0e, "__platform_driver_register" },
	{ 0x2f9b0293, "platform_device_register_full" },
	{ 0xd272d446, "__fentry__" },
	{ 0x30a11079, "device_destroy" },
	{ 0xfd4b4a36, "class_destroy" },
	{ 0x0c92f06e, "cdev_del" },
	{ 0x0bc5fb0d, "unregister_chrdev_region" },
	{ 0xe8213e80, "_printk" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xf46d5bf3, "mutex_unlock" },
	{ 0x814e12e5, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xc1e6c71e,
	0x9f222e1e,
	0x4c075f7d,
	0x6459621a,
	0xb6c08e4c,
	0xf350d701,
	0xbd03ed67,
	0x546c19d9,
	0xa61fd7aa,
	0x092a35a2,
	0x90a48d82,
	0xd272d446,
	0x40a621c5,
	0x092a35a2,
	0x102ec640,
	0xea97e6f1,
	0x357aaab3,
	0xacbfeb0e,
	0x2f9b0293,
	0xd272d446,
	0x30a11079,
	0xfd4b4a36,
	0x0c92f06e,
	0x0bc5fb0d,
	0xe8213e80,
	0xd272d446,
	0xf46d5bf3,
	0x814e12e5,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__mutex_init\0"
	"alloc_chrdev_region\0"
	"cdev_init\0"
	"cdev_add\0"
	"class_create\0"
	"device_create\0"
	"__ref_stack_chk_guard\0"
	"validate_usercopy_range\0"
	"__check_object_size\0"
	"_copy_from_user\0"
	"__ubsan_handle_out_of_bounds\0"
	"__stack_chk_fail\0"
	"snprintf\0"
	"_copy_to_user\0"
	"platform_device_unregister\0"
	"platform_driver_unregister\0"
	"mutex_trylock\0"
	"__platform_driver_register\0"
	"platform_device_register_full\0"
	"__fentry__\0"
	"device_destroy\0"
	"class_destroy\0"
	"cdev_del\0"
	"unregister_chrdev_region\0"
	"_printk\0"
	"__x86_return_thunk\0"
	"mutex_unlock\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Crayyuan,robot-motor-1.0");
MODULE_ALIAS("of:N*T*Crayyuan,robot-motor-1.0C*");

MODULE_INFO(srcversion, "05AB2D8EA3DF5597897F40E");
