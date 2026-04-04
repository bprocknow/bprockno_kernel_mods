
#include <linux/module.h>


static int brview_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{

}

static struct notifier_block brview_device_notifier = {
	.notifier_call = brview_device_event
};

static int __init brview_init(void)
{
	int err;

	err = register_netdevice_notifier(&brview_device_event);
	if (err)
		return err;



	return 0;

}

static void __exit brview_deinit(void)
{

}

module_init(brview_init)
module_exit(brview_deinit)
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("bridge probe module");
