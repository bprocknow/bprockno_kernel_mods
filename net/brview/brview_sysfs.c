
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include "brview.h"

static const struct attribute_group brview_group = {
	.name = SYSFS_BRVIEW_ATTR,
	.attrs = brview_attrs,
};

/*
 * Add entries in sysfs onto the existing network class device
 * for the bridge.
 */
int brview_sysfs_addbr(struct brview_bridge *brvb)
{
	struct kobject *brobj;
	struct net_device *dev = brvb->dev;
	int err;

	brvb->kobj = kobject_create_and_add(SYSFS_BRVIEW_PORT_SUBDIR, brview_root_kobj);
	if (!brvb->kobj) {
		pr_info("%s: can't add kobject (directory) %s/%s\n",
			__func__, dev->name, SYSFS_BRVIEW_PORT_SUBDIR);
		err = -ENOMEM;
		goto out1;
	}

	err = sysfs_create_group(brobj, &brview_group);
	if (err) {
		pr_info("%s: can't create group %s/%s\n",
		    __func__, brvb->ifname, brview_group.name);
		goto out2;
	}

	return 0;

out2:
	kobject_put(brvb->kobj);
out1:
	return err;

}

void brview_sysfs_delbr(struct brview_bridge *brvb)
{
	struct kobject *kobj = brvb->kobj;

	sysfs_remove_group(kobj, &brview_group);
	kobject_put(kobj);
}

