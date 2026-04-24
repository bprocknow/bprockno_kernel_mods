
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include "brview.h"

static inline struct brview_bridge *to_brview_bridge(struct kobject *kobj)
{
	return container_of(kobj, struct brview_bridge, kobj);
}

static ssize_t name_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct brview_bridge *obj = to_brview_bridge(kobj);
	return sysfs_emit(buf, "%s\n", obj->ifname);
}

static ssize_t netdev_up_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	//struct brview_bridge *obj = to_brview_bridge(kobj);
	return sysfs_emit(buf, "%u\n", 1);
}


static struct kobj_attribute name_attr =
	__ATTR_RO(name);

static struct kobj_attribute netdev_up_attr =
	__ATTR_RO(netdev_up);


static struct attribute *brview_attrs[] = {
	&name_attr.attr,
	&netdev_up_attr.attr,
	NULL,
};

static const struct attribute_group brview_group = {
	.name = SYSFS_BRVIEW_ATTR,
	.attrs = brview_attrs,
};

static const struct attribute_group *brview_bridge_default_groups[] = {
	&brview_group,
	NULL,
};

static void brview_bridge_release(struct kobject *kobj)
{
	struct brview_bridge *obj = to_brview_bridge(kobj);
	kfree(obj);
}

static const struct kobj_type brview_bridge_ktype = {
	.release = brview_bridge_release,
	.sysfs_ops = &kobj_sysfs_ops,
	.default_groups = brview_bridge_default_groups,
};

/*
 * Add entries in sysfs onto the existing network class device
 * for the bridge.
 */
int brview_sysfs_addbr(struct kobject *parent, struct brview_bridge *obj)
{
	//struct net_device *dev = obj->dev;
	int err;

	err = kobject_init_and_add(&obj->kobj, &brview_bridge_ktype,
			parent, "%s", "brview");
	if (err) {
		kobject_put(&obj->kobj);
		return err;
	}

	/*
	 * Announce after sysfs file exists
	 */
	kobject_uevent(&obj->kobj, KOBJ_ADD);
	//obj->kobj = kobject_create_and_add(SYSFS_BRVIEW_PORT_SUBDIR, brview_root_kobj);
	//if (!obj->kobj) {
	//	pr_info("%s: can't add kobject (directory) %s/%s\n",
	//		__func__, dev->name, SYSFS_BRVIEW_PORT_SUBDIR);
	//	err = -ENOMEM;
	//	goto out1;
	//}

	//err = sysfs_create_group(brobj, &brview_group);
	//if (err) {
	//	pr_info("%s: can't create group %s/%s\n",
	//	    __func__, obj->ifname, brview_group.name);
	//	goto out2;
	//}

	return 0;
//out2:
//	kobject_put(obj->kobj);
//out1:
//	return err;
}

void brview_sysfs_delbr(struct kobject *kobj)
{

	if (!kobj)
		return;

	//sysfs_remove_group(kobj, &brview_group);
	kobject_uevent(kobj, KOBJ_REMOVE);
	kobject_put(kobj);
}

