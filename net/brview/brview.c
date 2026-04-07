
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include "brview.h"
#include "br_private.h"


/*
 * Add entries in sysfs onto the existing network class device
 * for the bridge.
 */
int brview_sysfs_addbr(struct net_device *dev)
{
	struct kobject *brobj = &dev->dev.kobj;
	struct net_bridge *br = netdev_priv(dev);
	int err;

	br->ifobj = kobject_create_and_add(SYSFS_BRVIEW_PORT_SUBDIR, brobj);
	if (!br->ifobj) {
		pr_info("%s: can't add kobject (directory) %s/%s\n",
			__func__, dev->name, SYSFS_BRVIEW_PORT_SUBDIR);
		err = -ENOMEM;
		goto out;
	}
	return 0;
 out:
	return err;

}

void brview_sysfs_delbr(struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);

	kobject_put(br->ifobj);
}

static int brview_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct net_bridge_port *p;
	struct net_bridge *br;
	int err;

	if (netif_is_bridge_master(dev)) {
		if (event == NETDEV_REGISTER) {
			/* register of bridge completed, add sysfs entries */
			err = brview_sysfs_addbr(dev);
			if (err)
				return notifier_from_errno(err);

			return NOTIFY_DONE;
		}
	}

	/* not a port of a bridge */
	p = br_port_get_rtnl(dev);
	if (!p)
		return NOTIFY_DONE;

	br = p->br;

	switch (event) {
	case NETDEV_CHANGEMTU:
		break;

	case NETDEV_PRE_CHANGEADDR:
		if (br->dev->addr_assign_type == NET_ADDR_SET)
			break;
		break;

	case NETDEV_CHANGEADDR:
		spin_lock_bh(&br->lock);
		spin_unlock_bh(&br->lock);

		break;

	case NETDEV_CHANGE:
		break;

	case NETDEV_FEAT_CHANGE:
		break;

	case NETDEV_DOWN:
		spin_lock_bh(&br->lock);
		spin_unlock_bh(&br->lock);
		break;

	case NETDEV_UP:
		break;

	case NETDEV_UNREGISTER:
		//br_del_if(br, dev);
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

/* Delete bridge device */
void brview_dev_delete(struct net_device *dev, struct list_head *head)
{
	struct net_bridge *br = netdev_priv(dev);

	brview_sysfs_delbr(br->dev);
	unregister_netdevice_queue(br->dev, head);
}

static int __init brview_init(void)
{
	int err;

	err = register_netdevice_notifier(&brview_device_notifier);
	if (err)
		return err;



	return 0;

}

static void __exit brview_deinit(void)
{
	unregister_netdevice_notifier(&brview_device_notifier);

}

module_init(brview_init)
module_exit(brview_deinit)
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("bridge probe module");
