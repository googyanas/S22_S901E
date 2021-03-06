// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
 *      http://www.samsung.com
 *
 * Samsung TN debugging code
 */

#define pr_fmt(fmt)     KBUILD_MODNAME ":%s() " fmt, __func__

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fiemap.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <linux/samsung/bsp/sec_class.h>
#include <linux/samsung/bsp/sec_param.h>

#define SEC_EDTBO_FILENAME		"/spu/edtbo/edtbo.img"
#define EDTBO_FIEMAP_MAGIC		0x7763
#define EDTBO_FIEMAP_COUNT		128

static unsigned long edtbo_ver __ro_after_init;
module_param_named(edtbo, edtbo_ver, ulong, 0440);

static int fiemap_chk_ranges(struct super_block *sb,
		u64 start, u64 len, u64 *new_len)
{
	u64 maxbytes = (u64)sb->s_maxbytes;

	*new_len = len;

	if (len == 0)
		return -EINVAL;

	if (start > maxbytes)
		return -EFBIG;

	/*
	 * Shrink request scope to what the fs can actually handle.
	 */
	if (len > maxbytes || (maxbytes - len) < start)
		*new_len = maxbytes - start;

	return 0;
}

static ssize_t sec_edtbo_update_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct fiemap *pfiemap;
	struct fiemap_extent_info fieinfo = { 0, };
	struct file *fp;
	mm_segment_t old_fs;
	struct inode *inode;
	u64 len;
	int update, error;
	u32 allocsize = 0;
	u32 fiemap_magic = EDTBO_FIEMAP_MAGIC;

	error = kstrtoint(buf, 10, &update);
	if (error < 0 || update != 1)
		return -EINVAL;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(SEC_EDTBO_FILENAME, O_RDONLY, 0400);
	if (IS_ERR(fp)) {
		pr_err("file open error\n");
		error = -ENOENT;
		goto fp_err;
	}

	inode = file_inode(fp);
	if (!inode->i_op->fiemap) {
		error = -EOPNOTSUPP;
		goto open_err;
	}

	allocsize = EDTBO_FIEMAP_COUNT * sizeof(struct fiemap_extent)
			+ offsetof(struct fiemap, fm_extents);
	pfiemap = kmalloc(allocsize, GFP_KERNEL | __GFP_ZERO);
	if (!pfiemap) {
		error = -ENOMEM;
		goto open_err;
	}

	pfiemap->fm_length = ULONG_MAX;
	pfiemap->fm_extent_count = EDTBO_FIEMAP_COUNT;

	error = fiemap_chk_ranges(inode->i_sb,
			pfiemap->fm_start, pfiemap->fm_length, &len);
	if (error)
		goto alloc_err;

	fieinfo.fi_flags = pfiemap->fm_flags;
	fieinfo.fi_extents_max = pfiemap->fm_extent_count;
	fieinfo.fi_extents_start = pfiemap->fm_extents;

	error = inode->i_op->fiemap(inode, &fieinfo, pfiemap->fm_start, len);
	if (error)
		goto alloc_err;

	pfiemap->fm_flags = fieinfo.fi_flags;
	pfiemap->fm_mapped_extents = fieinfo.fi_extents_mapped;

	if (!pfiemap->fm_mapped_extents) {
		error = -EFAULT;
		goto alloc_err;
	}

	sec_param_set(param_index_fiemap_result, pfiemap);
	sec_param_set(param_index_fiemap_update, &fiemap_magic);

	error = size;

alloc_err:
	kfree(pfiemap);
open_err:
	filp_close(fp, NULL);
fp_err:
	set_fs(old_fs);

	return error;
}
static DEVICE_ATTR_WO(sec_edtbo_update);

static ssize_t sec_edtbo_version_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%lu\n", edtbo_ver);
}
static DEVICE_ATTR_RO(sec_edtbo_version);

static struct attribute *sec_sysup_attributes[] = {
	&dev_attr_sec_edtbo_update.attr,
	&dev_attr_sec_edtbo_version.attr,
	NULL,
};

static struct attribute_group sec_sysup_attr_group = {
	.attrs = sec_sysup_attributes,
};

static int __init sec_sysup_init(void)
{
	int ret;
	struct device *dev;

	pr_info("start\n");

	dev = sec_device_create(NULL, "sec_sysup");
	if (!dev)
		pr_err("sec device create failed!\n");

	ret = sysfs_create_group(&dev->kobj, &sec_sysup_attr_group);
	if (ret)
		pr_err("could not create sysfs node\n");

	return 0;
}

static void __exit sec_sysup_exit(void)
{
	pr_info("exit\n");
}

module_init(sec_sysup_init);
module_exit(sec_sysup_exit);

MODULE_AUTHOR("Samsung Electronics");
MODULE_DESCRIPTION("Samsung System Update");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);	/* NOTE: OK. I know it. */
