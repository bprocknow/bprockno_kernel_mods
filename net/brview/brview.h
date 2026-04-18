
#ifndef _BRVIEW_H
#define _BRVIEW_H


#define SYSFS_BRVIEW_PORT_SUBDIR "brview"
#define SYSFS_BRVIEW_ATTR "brview_attrs"

extern struct kobject *brview_root_kobj;

struct brview_bridge {
	struct kobject *kobj;
	struct list_head next;
	char ifname[IFNAMSIZ];

	/* Tracked bridge object associated */
	struct net_device *dev;
};


void brview_dev_delete(struct net_device *dev, struct list_head *head);

#ifdef CONFIG_SYSFS
int brview_sysfs_addbr(struct brview_bridge *dev);
void brview_sysfs_delbr(struct brview_bridge *dev);
#else
int brview_sysfs_addbr(struct brview_bridge *dev) { return 0; }
void brview_sysfs_delbr(struct brview_bridge *dev) { return; }
#endif /* CONFIG_SYSFS */

#endif /* _BRVIEW_H */
