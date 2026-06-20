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
	{ 0xf46d5bf3, "mutex_lock" },
	{ 0x67628f51, "msleep" },
	{ 0xf46d5bf3, "mutex_unlock" },
	{ 0xc1e6c71e, "__mutex_init" },
	{ 0x77ef2dc7, "gpiod_get_index_optional" },
	{ 0xfeeac846, "gpiod_to_irq" },
	{ 0x9126ce86, "request_threaded_irq" },
	{ 0x102ec640, "platform_device_unregister" },
	{ 0xea97e6f1, "platform_driver_unregister" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0xacbfeb0e, "__platform_driver_register" },
	{ 0x2f9b0293, "platform_device_register_full" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xd272d446, "__fentry__" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x9dd4105e, "free_irq" },
	{ 0x7acf5086, "gpiod_put" },
	{ 0xe8213e80, "_printk" },
	{ 0xd94efd11, "const_current_task" },
	{ 0x814e12e5, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xf46d5bf3,
	0x67628f51,
	0xf46d5bf3,
	0xc1e6c71e,
	0x77ef2dc7,
	0xfeeac846,
	0x9126ce86,
	0x102ec640,
	0xea97e6f1,
	0xbd03ed67,
	0xacbfeb0e,
	0x2f9b0293,
	0xd272d446,
	0xd272d446,
	0xd272d446,
	0x9dd4105e,
	0x7acf5086,
	0xe8213e80,
	0xd94efd11,
	0x814e12e5,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"mutex_lock\0"
	"msleep\0"
	"mutex_unlock\0"
	"__mutex_init\0"
	"gpiod_get_index_optional\0"
	"gpiod_to_irq\0"
	"request_threaded_irq\0"
	"platform_device_unregister\0"
	"platform_driver_unregister\0"
	"__ref_stack_chk_guard\0"
	"__platform_driver_register\0"
	"platform_device_register_full\0"
	"__stack_chk_fail\0"
	"__fentry__\0"
	"__x86_return_thunk\0"
	"free_irq\0"
	"gpiod_put\0"
	"_printk\0"
	"const_current_task\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "8F7EB75707702A589472758");
