// SPDX-License-Identifier: GPL-2.0
/*
 * ouiche_fs - a simple educational filesystem for Linux
 *
 * Copyright (C) 2018  Redha Gouicem <redha.gouicem@lip6.fr>
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/ioctl.h>
#include <linux/buffer_head.h>

#define change_version _IOW('A',1,char*)

#include "ouichefs.h"

struct dentry *debug_file;
struct dentry *mount_point;
int major;

char temp_c[512];


ssize_t debugfs_read(struct file * file, char *buf, size_t count, loff_t *pos)
 {
	// struct inode *inode = mount_point->d_inode;
	// struct ouichefs_inode *cinode = NULL;
	// struct ouichefs_inode_info *ci = OUICHEFS_INODE(inode);
	struct inode *inode;
	struct super_block *sb = mount_point->d_sb;
	struct ouichefs_sb_info *sbi = OUICHEFS_SB(sb);
	struct ouichefs_inode *cinode = NULL;
	struct buffer_head *bh_old_index;
	struct buffer_head *bh_inode;
	struct ouichefs_file_index_block *old_index;
	uint32_t block_old_index;

	ssize_t len = 0;
	uint32_t i = 0;

	// stat->f_blocks = sbi->nr_blocks;
	// stat->f_bfree = sbi->nr_free_blocks;
	// stat->f_bavail = sbi->nr_free_blocks;
	// stat->f_files = sbi->nr_inodes - sbi->nr_free_inodes;
	// stat->f_ffree = sbi->nr_free_inodes;
	// stat->f_namelen = OUICHEFS_FILENAME_LEN;
	// ouichefs_statfs(mount_point, stat);
	len += sprintf(temp_c, "%d files\n", sbi->nr_inodes - sbi->nr_free_inodes);
	len += sprintf(temp_c+len,"%s dir\n",mount_point->d_name.name);
	len += sprintf(temp_c+len,"inodes : ");
	list_for_each_entry(inode,&sb->s_inodes,i_sb_list){
		len += sprintf(temp_c+len, "%d ", inode->i_ino);
		uint32_t inode_block = (inode->i_ino / OUICHEFS_INODES_PER_BLOCK) + 1;

		bh_inode = sb_bread(sb, inode_block);
		if (!bh_inode) return -EIO;
		cinode = ((struct ouichefs_inode *)(bh_inode->b_data) + (inode->i_ino % OUICHEFS_INODES_PER_BLOCK) - 1);

		bh_old_index = sb_bread(sb, cinode->index_block);
		if (!bh_old_index) return -EIO;
		old_index = (struct ouichefs_file_index_block *)bh_old_index->b_data;

		len += sprintf(temp_c+len, "-> version %d\n", old_index->blocks[(OUICHEFS_BLOCK_SIZE >> 2) - 3]);
	}

	return simple_read_from_buffer(buf, len, pos, temp_c, 512);
}

const struct file_operations debugfs_ops = {
	.owner = THIS_MODULE,
	.read  = debugfs_read,
};

static long ouichefs_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct inode *inode = file->f_inode;
	uint32_t inode_block = (inode->i_ino / OUICHEFS_INODES_PER_BLOCK) + 1;
	struct super_block *sb = inode->i_sb;
	struct ouichefs_inode *cinode = NULL;
	struct buffer_head *bh_inode, *bh_tmp;
	switch (cmd)
	{
	case change_version:
		bh_inode = sb_bread(sb, inode_block);
		if (!bh_inode) return -EIO;
		cinode = ((struct ouichefs_inode *)(bh_inode->b_data) + (inode->i_ino % OUICHEFS_INODES_PER_BLOCK) - 1);

		bh_tmp = sb_bread(sb, cinode->index_block);
		if (!bh_inode) return -EIO;
		int i;
		for (i = arg; i != 0; i--) {
			if (bh_tmp->b_data[(OUICHEFS_BLOCK_SIZE >> 2) - 1] <= 0) break;
			bh_tmp = sb_bread(sb, bh_tmp->b_data[(OUICHEFS_BLOCK_SIZE >> 2) - 1]);
		}
		cinode->index_block = bh_tmp->b_blocknr;
		break;
	default:
		pr_info("wrong command\n");
		break;
	}
	return 0;
}

const struct file_operations ioctl_ops = {
	.unlocked_ioctl = ouichefs_unlocked_ioctl
};
/*
 * Mount a ouiche_fs partition
 */
struct dentry *ouichefs_mount(struct file_system_type *fs_type, int flags,
			      const char *dev_name, void *data)
{
	struct dentry *dentry = NULL;

	dentry = mount_bdev(fs_type, flags, dev_name, data,
			    ouichefs_fill_super);
	if (IS_ERR(dentry))
		pr_err("'%s' mount failure\n", dev_name);
	else {
		pr_info("'%s' mount success\n", dev_name);
		mount_point = dentry;
	}

	return dentry;
}

/*
 * Unmount a ouiche_fs partition
 */
void ouichefs_kill_sb(struct super_block *sb)
{
	kill_block_super(sb);

	pr_info("unmounted disk\n");
}

static struct file_system_type ouichefs_file_system_type = {
	.owner = THIS_MODULE,
	.name = "ouichefs",
	.mount = ouichefs_mount,
	.kill_sb = ouichefs_kill_sb,
	.fs_flags = FS_REQUIRES_DEV,
	.next = NULL,
};

static int __init ouichefs_init(void)
{
	int ret;

	ret = ouichefs_init_inode_cache();
	if (ret) {
		pr_err("inode cache creation failed\n");
		goto end;
	}

	ret = register_filesystem(&ouichefs_file_system_type);
	if (ret) {
		pr_err("register_filesystem() failed\n");
		goto end;
	}

	debug_file = debugfs_create_file("ouichefs", 0444, NULL, NULL, &debugfs_ops);
	pr_info("module loaded\n");

	major = register_chrdev(0, "ouichefs_ioctl", &ioctl_ops);
	pr_info("created device with no : %d\n", major);
end:
	return ret;
}

static void __exit ouichefs_exit(void)
{
	int ret;

	ret = unregister_filesystem(&ouichefs_file_system_type);
	if (ret)
		pr_err("unregister_filesystem() failed\n");

	ouichefs_destroy_inode_cache();

	debugfs_remove(debug_file);

	unregister_chrdev(major, "ouichefs_ioctl");
	pr_info("module unloaded\n");
}

module_init(ouichefs_init);
module_exit(ouichefs_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Redha Gouicem, <redha.gouicem@lip6.fr>");
MODULE_DESCRIPTION("ouichefs, a simple educational filesystem for Linux");
