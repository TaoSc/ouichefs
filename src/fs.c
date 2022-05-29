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

#include "ioctl.h"
#include "ouichefs.h"
#include "bitmap.h"

struct dentry *debug_file;
struct dentry *mount_point;
int major;

char temp_c[4096];


ssize_t debugfs_read(struct file * file, char *buf, size_t count, loff_t *pos)
{
	struct inode *inode = NULL;
	struct super_block *sb = mount_point->d_sb;
	struct ouichefs_sb_info *sbi = OUICHEFS_SB(sb);
	struct ouichefs_inode *cinode = NULL;
	struct buffer_head *bh_inode = NULL;
	struct buffer_head *bh_tmp = NULL;
	struct ouichefs_file_index_block *tmp_index = NULL;
	uint32_t inode_block;
	ssize_t len = 0;

	len += sprintf(temp_c, "%d inode(s)\n", 
				sbi->nr_inodes - sbi->nr_free_inodes);
	len += sprintf(temp_c + len, "inode\t\t\tversion\t\tblock history\n");

	list_for_each_entry(inode, &sb->s_inodes, i_sb_list) {
		if (!S_ISREG(inode->i_mode) || !(inode->i_ino)) continue;

		len += sprintf(temp_c + len, "%ld", inode->i_ino);

		inode_block = (inode->i_ino / OUICHEFS_INODES_PER_BLOCK) + 1;
		bh_inode = sb_bread(sb, inode_block);
		if (!bh_inode) return -EIO;
		cinode = ((struct ouichefs_inode *)(bh_inode->b_data) + 
			(inode->i_ino % OUICHEFS_INODES_PER_BLOCK) - 1);

		bh_tmp = sb_bread(sb, cinode->index_block);
		if (!bh_tmp) return -EIO;
		tmp_index = (struct ouichefs_file_index_block *)bh_tmp->b_data;
		len += sprintf(temp_c + len, "\t\t\t%d (%lld blks)", 
			tmp_index->blocks[OUICHEFS_INDEX_COUNT], 
			inode->i_blocks);

		len += sprintf(temp_c + len, "\t\t%d", cinode->index_block);
		while (tmp_index->blocks[OUICHEFS_PREV_INDEX] > 0) {
			len += sprintf(temp_c + len, ", %d", 
				tmp_index->blocks[OUICHEFS_PREV_INDEX]);
			bh_tmp = sb_bread(sb, 
				tmp_index->blocks[OUICHEFS_PREV_INDEX]);
			if (!bh_tmp) return -EIO;
			tmp_index = 
			(struct ouichefs_file_index_block *)bh_tmp->b_data;
		}
		len += sprintf(temp_c + len, "\n");
	}

	return simple_read_from_buffer(buf, len, pos, temp_c, 4096);
}

const struct file_operations debugfs_ops = {
	.owner = THIS_MODULE,
	.read  = debugfs_read,
};

long ouichefs_unlocked_ioctl(struct file *file, unsigned int cmd, 
						unsigned long arg)
{
	struct super_block *sb = mount_point->d_sb;
	struct ouichefs_sb_info *sbi = OUICHEFS_SB(sb);
	struct inode *inode = NULL;
	struct ouichefs_inode *cinode = NULL;
	struct buffer_head *bh_inode = NULL, *bh_tmp = NULL, *bh2 = NULL;
	struct ouichefs_file_index_block *tmp_index = NULL;
	struct ioctl_request req;
	uint32_t inode_block;
	uint32_t i = 0;

	if (copy_from_user(&req, (struct ioctl_request *)arg, sizeof(arg))) {
		pr_err("ioctl: Couldn't copy arguments. Error!");
		return EINVAL;
	}
	pr_info("ino: %d\n", req.ino);
	inode = ouichefs_iget(sb, req.ino);
	inode_block = (inode->i_ino / OUICHEFS_INODES_PER_BLOCK) + 1;

	bh_inode = sb_bread(sb, inode_block);
	if (!bh_inode) return -EIO;
	cinode = ((struct ouichefs_inode *)(bh_inode->b_data) + 
			(inode->i_ino % OUICHEFS_INODES_PER_BLOCK) - 1);

	if (!S_ISREG(inode->i_mode)) {
		pr_err("ioctl: File not regular. Error !");
		return EINVAL;
	}


	switch (cmd)
	{
	case CHANGE_VER:
		bh_tmp = sb_bread(sb, cinode->index_block);
		if (!bh_tmp) return -EIO;
		tmp_index = (struct ouichefs_file_index_block *)bh_tmp->b_data;

		pr_info("old ib: %d\n", cinode->index_block);

		for (i = req.nb_version; i != 0 && 
			tmp_index->blocks[OUICHEFS_PREV_INDEX] > 0; i--) {
			bh_tmp = sb_bread(sb, tmp_index->blocks[OUICHEFS_PREV_INDEX]);
			if (!bh_tmp) return -EIO;
			tmp_index = (struct ouichefs_file_index_block *)bh_tmp->b_data;
		}
		cinode->index_block = bh_tmp->b_blocknr;

		pr_info("new ib: %d\n", cinode->index_block);
		break;

	case NEW_LATEST:
		if (!cinode->last_index_block || cinode->last_index_block == cinode->index_block)
			break;

cleanup_bi:
		bh_tmp = sb_bread(sb, cinode->last_index_block);
		if (!bh_tmp) return -EIO;
		tmp_index = (struct ouichefs_file_index_block *)bh_tmp->b_data;

		for (i = 0; i < OUICHEFS_INDEX_COUNT; i++) {
			char *block;

			if(!tmp_index->blocks[i])
				continue;

			put_block(sbi, tmp_index->blocks[i]);
			bh2 = sb_bread(sb, tmp_index->blocks[i]);
			if (!bh2)
				continue;
			block = (char *)bh2->b_data;
			memset(block, 0, OUICHEFS_BLOCK_SIZE);
			mark_buffer_dirty(bh2);
			brelse(bh2);
		}

		// delete blocks referenced by newer versions of the file
		if (tmp_index->blocks[OUICHEFS_PREV_INDEX] && tmp_index->blocks[OUICHEFS_PREV_INDEX] != cinode->index_block) {
			bh_tmp = sb_bread(sb, tmp_index->blocks[OUICHEFS_PREV_INDEX]);
			if (!bh_tmp) return -EIO;
			tmp_index = (struct ouichefs_file_index_block *)bh_tmp->b_data;
			goto cleanup_bi;
		}

		cinode->last_index_block = cinode->index_block;
		break;

	default:
		return -ENOTTY;
		break;
	}

	mark_inode_dirty(inode);
	mark_buffer_dirty(bh_inode);
	brelse(bh_inode);
	brelse(bh_tmp);

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

	debug_file = debugfs_create_file("ouichefs", 0444, NULL, NULL, 
							&debugfs_ops);
	pr_info("module loaded\n");

	major = register_chrdev(0, IOCTL_NAME, &ioctl_ops);
	pr_info("created device with no: %d\n", major);
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
