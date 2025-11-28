#ifndef INCLUDE_FS_H
#define INCLUDE_FS_H

#define FS_MAGIC    0x4C4F4653
#define FS_NAME_MAX 32

#define FS_TYPE_FILE 1
#define FS_TYPE_DIR  2

/* Superblock: header for entire filesystem image */
struct fs_super {
    unsigned int magic;
    unsigned int version;
    unsigned int root_index; /* index into object table */
    unsigned int obj_count;  /* how many objects follow */
} __attribute__((packed));

/* Object header: describes a file or directory */
struct fs_obj {
    char         name[FS_NAME_MAX]; /* zero-terminated if shorter */
    unsigned int type;              /* FS_TYPE_FILE / FS_TYPE_DIR */
    unsigned int size;              /* bytes in body */
    unsigned int body_offset;       /* byte offset from image start */
    unsigned int entry_count;       /* for dirs: number of children */
    unsigned int reserved[3];       /* padding / future flags */
} __attribute__((packed));

void fs_init(void);

/* Lookup "/path" and return object or 0 on failure */
struct fs_obj *fs_lookup(const char *path);

/* Read up to len bytes from file into buf starting at off.
 * Returns >0 = bytes read, 0 = EOF, <0 = error.
 */
int fs_read(const struct fs_obj *obj, void *buf,
            unsigned int off, unsigned int len);

#endif
