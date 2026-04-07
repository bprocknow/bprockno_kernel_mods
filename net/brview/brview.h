
#ifndef _BRVIEW_H
#define _BRVIEW_H


#define SYSFS_BRVIEW_PORT_SUBDIR "brview"

int brview_sysfs_addbr(struct net_device *dev);
void brview_dev_delete(struct net_device *dev, struct list_head *head);
void brview_sysfs_delbr(struct net_device *dev);

#endif /* _BRVIEW_H */
