// SPDX-License-Identifier: GPL-2.0
/*
 * ouiche_fs - a simple educational filesystem for Linux
 *
 * Copyright (C) 2018 Redha Gouicem <redha.gouicem@lip6.fr>
 */

#define pr_fmt(fmt) "ouichefs: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>

#include "ouichefs.h"
#include "bitmap.h"

/*
 * Map the buffer_head passed in argument with the iblock-th block of the file
 * represented by inode. If the requested block is not allocated and create is
 * true,  allocate a new block on disk and map it.
 */
static int ouichefs_file_get_block(struct inode *inode, sector_t iblock,
				   struct buffer_head *bh_result, int create)
{
	struct super_block *sb = inode->i_sb;
	struct ouichefs_sb_info *sbi = OUICHEFS_SB(sb);
	struct ouichefs_inode_info *ci = OUICHEFS_INODE(inode);
	struct ouichefs_file_index_block *index;
	struct buffer_head *bh_index;
	bool alloc = false;
	int ret = 0, bno;

	/* If block number exceeds filesize, fail */

	if (iblock >= OUICHEFS_INDEX_COUNT && iblock < OUICHEFS_BLOCK_SIZE >> 2)
		pr_info("on tente de lire un des trois derniers blocks.\n");

	if (iblock >= OUICHEFS_INDEX_COUNT)
		return -EFBIG;

	/* Read index block from disk */
	bh_index = sb_bread(sb, ci->index_block);
	if (!bh_index)
		return -EIO;
	index = (struct ouichefs_file_index_block *)bh_index->b_data;

	/*
	 * Check if iblock is already allocated. If not and create is true,
	 * allocate it. Else, get the physical block number.
	 */
	if (index->blocks[iblock] == 0) {
		if (!create)
			return 0;
		bno = get_free_block(sbi);
		if (!bno) {
			ret = -ENOSPC;
			goto brelse_index;
		}
		index->blocks[iblock] = bno;
		alloc = true;
	} else {
		bno = index->blocks[iblock];
	}

	/* Map the physical block to to the given buffer_head */
	map_bh(bh_result, sb, bno);

brelse_index:
	brelse(bh_index);

	return ret;
}

/*
 * Called by the page cache to read a page from the physical disk and map it in
 * memory.
 */
static int ouichefs_readpage(struct file *file, struct page *page)
{
	return mpage_readpage(page, ouichefs_file_get_block);
}

/*
 * Called by the page cache to write a dirty page to the physical disk (when
 * sync is called or when memory is needed).
 */
static int ouichefs_writepage(struct page *page, struct writeback_control *wbc)
{
	return block_write_full_page(page, ouichefs_file_get_block, wbc);
}

/*
 * Called by the VFS when a write() syscall occurs on file before writing the
 * data in the page cache. This functions checks if the write will be able to
 * complete and allocates the necessary blocks through block_write_begin().
 */
static int ouichefs_write_begin(struct file *file,
				struct address_space *mapping, loff_t pos,
				unsigned int len, unsigned int flags,
				struct page **pagep, void **fsdata)
{
	struct ouichefs_sb_info *sbi = OUICHEFS_SB(file->f_inode->i_sb);
	int err;
	uint32_t nr_allocs = 0;

	struct inode *inode = file->f_inode;
	struct ouichefs_inode *cinode = NULL;
	struct super_block *sb = inode->i_sb;
	uint32_t inode_block = (inode->i_ino / OUICHEFS_INODES_PER_BLOCK) + 1;

	struct ouichefs_file_index_block *old_index = NULL;
	struct ouichefs_file_index_block *new_index = NULL;

	struct buffer_head *bh_new_index = NULL;
	struct buffer_head *bh_old_index = NULL;
	struct buffer_head *bh_inode = NULL;


	// On alloue un nouveau bloc dans lequel on stocke l'index
	uint32_t block_new_index = get_free_block(OUICHEFS_SB(sb));
	// On r??cup??re le num??ro de block de l'ancien index
	uint32_t block_old_index;

	// Step 3 checks
	// if (!cinode->last_index_block || 
	// 		cinode->index_block != cinode->last_index_block) {
	// 	pr_err("Trying to edit an older revision. \
	// 		Please switch file to latest revision.");
	// 	return -EIO;
	// }

	bh_inode = sb_bread(sb, inode_block);
	if (!bh_inode) 
		return -EIO;
	cinode = ((struct ouichefs_inode *)(bh_inode->b_data) + 
			(inode->i_ino % OUICHEFS_INODES_PER_BLOCK) - 1);

	block_old_index = cinode->index_block;

	bh_old_index = sb_bread(sb, block_old_index);
	if (!bh_old_index) 
		return -EIO;
	old_index = (struct ouichefs_file_index_block *)bh_old_index->b_data;

	// On lit ce bloc
	bh_new_index = sb_bread(sb, block_new_index);
	if (!bh_new_index) return -EIO;
	new_index = (struct ouichefs_file_index_block *)bh_new_index->b_data;

	// On ins??re le nouveau bloc dans la liste
	old_index->blocks[OUICHEFS_NEXT_INDEX] = block_new_index;
	new_index->blocks[OUICHEFS_PREV_INDEX] = block_old_index;
	new_index->blocks[OUICHEFS_NEXT_INDEX] = -1;

	// On incr??mente le compteur de versions
	new_index->blocks[OUICHEFS_INDEX_COUNT] =
		old_index->blocks[OUICHEFS_INDEX_COUNT] + 1;

	// On met ?? jour le num??ro de bloc correspondant ?? la nouvelle version
	cinode->index_block = block_new_index;
	cinode->last_index_block = block_new_index;

	pr_info("ino bl: %d, ino: %d, cpt: %d, new: %d, old: %d.\n",
		inode_block, (int)inode->i_ino, 
				new_index->blocks[OUICHEFS_INDEX_COUNT],
		cinode->index_block, block_old_index);

	mark_inode_dirty(inode);
	mark_buffer_dirty(bh_inode);
	mark_buffer_dirty(bh_new_index);
	mark_buffer_dirty(bh_old_index);
	brelse(bh_inode);
	brelse(bh_new_index);
	brelse(bh_old_index);

	/* Check if the write can be completed (enough space?) */
	if (pos + len > OUICHEFS_MAX_FILESIZE)
		return -ENOSPC;
	nr_allocs = max(pos + len, 
			file->f_inode->i_size) / OUICHEFS_BLOCK_SIZE;
	if (nr_allocs > file->f_inode->i_blocks - 1)
		nr_allocs -= file->f_inode->i_blocks - 1;
	else
		nr_allocs = 0;
	if (nr_allocs > sbi->nr_free_blocks)
		return -ENOSPC;

	/* prepare the write */
	err = block_write_begin(mapping, 0, len, flags, pagep,
				ouichefs_file_get_block);
	/* if this failed, reclaim newly allocated blocks */
	if (err < 0) {
		pr_err("%s:%d: newly allocated blocks reclaim not \
						implemented yet\n",
		       __func__, __LINE__);
	}
	return err;
}

/*
 * Called by the VFS after writing data from a write() syscall to the page
 * cache. This functions updates inode metadata and truncates the file if
 * necessary.
 */
static int ouichefs_write_end(struct file *file, struct address_space *mapping,
			      loff_t pos, unsigned int len, 
			      unsigned int copied, struct page *page, 
				void *fsdata)
{
	int ret;
	struct inode *inode = file->f_inode;

	/* Complete the write() */
	ret = generic_write_end(file, mapping, pos, len, copied, page, fsdata);
	if (ret < len) {
		pr_err("%s:%d: wrote less than asked... what do I do? \
						nothing for now...\n",
		       __func__, __LINE__);
	} else {
		/* Update inode metadata */
		inode->i_blocks = inode->i_size / OUICHEFS_BLOCK_SIZE + 2;
		inode->i_mtime = inode->i_ctime = current_time(inode);
		mark_inode_dirty(inode);
	}

	return ret;
}

const struct address_space_operations ouichefs_aops = {
	.readpage    = ouichefs_readpage,
	.writepage   = ouichefs_writepage,
	.write_begin = ouichefs_write_begin,
	.write_end   = ouichefs_write_end
};

const struct file_operations ouichefs_file_ops = {
	.owner      = THIS_MODULE,
	.llseek     = generic_file_llseek,
	.read_iter  = generic_file_read_iter,
	.write_iter = generic_file_write_iter
};
