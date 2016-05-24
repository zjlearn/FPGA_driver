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
	{ 0x14522340, "module_layout" },
	{ 0x1fedf0f4, "__request_region" },
	{ 0x42e80c19, "cdev_del" },
	{ 0x4f1939c7, "per_cpu__current_task" },
	{ 0xc45a9f63, "cdev_init" },
	{ 0x69a358a6, "iomem_resource" },
	{ 0xd2037915, "dev_set_drvdata" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xd691cba2, "malloc_sizes" },
	{ 0xa30682, "pci_disable_device" },
	{ 0xf417ff07, "pci_disable_msix" },
	{ 0xfc4f55f3, "down_interruptible" },
	{ 0x7edc1537, "device_destroy" },
	{ 0xd3364703, "x86_dma_fallback_dev" },
	{ 0x1985ed3c, "prepare_to_wait_exclusive" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x7d11c268, "jiffies" },
	{ 0x343a1a8, "__list_add" },
	{ 0xffc7c184, "__init_waitqueue_head" },
	{ 0xaf559063, "pci_set_master" },
	{ 0x9f1019bd, "pci_set_dma_mask" },
	{ 0x7b3d21a1, "pci_enable_msix" },
	{ 0x150853cf, "down_trylock" },
	{ 0xea147363, "printk" },
	{ 0x85f8a266, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x6dcaeb88, "per_cpu__kernel_stack" },
	{ 0x521445b, "list_del" },
	{ 0x2d2cf7d, "device_create" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0xa6d1bdca, "cdev_add" },
	{ 0x78764f4e, "pv_irq_ops" },
	{ 0x42c8de35, "ioremap_nocache" },
	{ 0x1000e51, "schedule" },
	{ 0x7c61340c, "__release_region" },
	{ 0x68f7c535, "pci_unregister_driver" },
	{ 0x2044fa9e, "kmem_cache_alloc_trace" },
	{ 0x642e54ac, "__wake_up" },
	{ 0x37a0cba, "kfree" },
	{ 0x33d92f9a, "prepare_to_wait" },
	{ 0xedc03953, "iounmap" },
	{ 0x3f1899f1, "up" },
	{ 0x5f07b9f3, "__pci_register_driver" },
	{ 0xe06bb002, "class_destroy" },
	{ 0x9ccb2622, "finish_wait" },
	{ 0x436c2179, "iowrite32" },
	{ 0xa12add91, "pci_enable_device" },
	{ 0xb02504d8, "pci_set_consistent_dma_mask" },
	{ 0xa2654165, "__class_create" },
	{ 0x3302b500, "copy_from_user" },
	{ 0xa92a43c, "dev_get_drvdata" },
	{ 0x6e9681d2, "dma_ops" },
	{ 0xe484e35f, "ioread32" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000019E5d00000164sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000019E5d00000165sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "14A7E439E7A19D3C5ECF809");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 5,
};
