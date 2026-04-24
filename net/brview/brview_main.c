
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/netdevice.h>

#include "brview.h"
#include "br_private.h"

static LIST_HEAD(brview_bridge_list);
static DEFINE_MUTEX(brview_bridge_lock);

struct kobject *brview_root_kobj;
EXPORT_SYMBOL_GPL(brview_root_kobj);


static int brview_create(struct net_device *dev)
{
	struct brview_bridge *obj;
	int err;

	mutex_lock(&brview_bridge_lock);

	list_for_each_entry(obj, &brview_bridge_list, next) {
		if (obj->dev == dev) {
			err = -EEXIST;
			WARN_ONCE(1, "bridge device already exists in the list\n");
			goto out_unlock;
		}
	}

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj) {
		err = -ENOMEM;
		goto out_unlock;
	}

	obj->dev = dev;
	snprintf(obj->ifname, sizeof(obj->ifname), "brview_%s", dev->name);

	err = brview_sysfs_addbr(brview_root_kobj, obj);
	if (err) {
		goto out_free;
	}

	list_add_tail(&obj->next, &brview_bridge_list);

	mutex_unlock(&brview_bridge_lock);
	return 0;

out_free:
	kfree(obj);
out_unlock:
	mutex_unlock(&brview_bridge_lock);
	return err;
}

static void brview_release(struct net_device *dev)
{
	struct brview_bridge *obj, *tmp;
	pr_info("BlahRemoving net_device: %s\n", dev->name);

	mutex_lock(&brview_bridge_lock);

	list_for_each_entry_safe(obj, tmp, &brview_bridge_list, next) {
		if (obj->dev == dev) {
			pr_info("Removing net_device: %s\n", dev->name);

			list_del_init(&obj->next);

			brview_sysfs_delbr(&obj->kobj);
			mutex_unlock(&brview_bridge_lock);
			return;
		}
	}

	mutex_unlock(&brview_bridge_lock);
	WARN_ONCE(1, "brview does not exist in list\n");
}

static int brview_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	//struct net_bridge_port *p;
	//struct net_bridge *br;
	int err;

	if (netif_is_bridge_master(dev)) {
		if (event == NETDEV_REGISTER) {
			/* register of bridge completed, add sysfs entries */
			err = brview_create(dev);
			if (err)
				return notifier_from_errno(err);

			return NOTIFY_DONE;
		}
	}

	/* not a port of a bridge */
	//p = br_port_get_rtnl(dev);
	//if (!p)
	//	return NOTIFY_DONE;

	//br = p->br;

	switch (event) {
	case NETDEV_CHANGEMTU:
		break;

	case NETDEV_PRE_CHANGEADDR:
		break;

	case NETDEV_CHANGEADDR:
		break;

	case NETDEV_CHANGE:
		break;

	case NETDEV_FEAT_CHANGE:
		break;

	case NETDEV_DOWN:
		break;

	case NETDEV_UP:
		break;

	case NETDEV_UNREGISTER:
		if (netif_is_bridge_master(dev))
			brview_release(dev);
		break;

	case NETDEV_CHANGENAME:
		break;

	case NETDEV_PRE_TYPE_CHANGE:
		/* Forbid underlying device to change its type. */
		return NOTIFY_BAD;

	case NETDEV_RESEND_IGMP:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block brview_device_notifier = {
	.notifier_call = brview_device_event
};

static int __init brview_init(void)
{
	int err;

	brview_root_kobj = kobject_create_and_add("brview_monitor", kernel_kobj);
	if (!brview_root_kobj) {
		pr_info("%s: can't add kobject (directory)\n", __func__);
		err = -ENOMEM;
		goto out_ret;
	}

	err = register_netdevice_notifier(&brview_device_notifier);
	if (err) {
		pr_err("brview: register_netdevice_notifier failed %d\n", err);
		goto out_free;
	}


	return 0;
out_free:
	kobject_put(brview_root_kobj);
out_ret:
	return err;
}

static void __exit brview_deinit(void)
{
	struct brview_bridge *obj;
	unregister_netdevice_notifier(&brview_device_notifier);

	mutex_lock(&brview_bridge_lock);

	list_for_each_entry(obj, &brview_bridge_list, next) {
		brview_sysfs_delbr(&obj->kobj);
	}

	mutex_unlock(&brview_bridge_lock);

	kobject_put(brview_root_kobj);
}

module_init(brview_init);
module_exit(brview_deinit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("bridge probe module");

