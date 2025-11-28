//fs.c simple read only filesytem over embedded initfs 
#include "fs.h"
#include "framebuffer.h" // fb_write
//symbols produced by objcopy -i binary -0 elf32-i386
extern unsigned char _binary_initfs_img_start[];
extern unsigned char _binary_initfs_img_end[];
static unsigned char *fs_image_base = 0;
static struct fs_super *fs_superblock = 0;
static struct fs_obj *fs_objects = 0;
//local helper - no libc so implement minimal 


/* Compare a segment (not null-terminated) to an object name */
static int name_matches(const char *obj_name,
                        const char *seg, unsigned int seg_len)
{
    unsigned int i;
    for (i = 0; i < seg_len; i++) {
        if (obj_name[i] != seg[i]) return 0;
        if (obj_name[i] == '\0')   return 0; /* obj name shorter */
    }
    /* segment ended; obj_name must either end here or continue.
     * We accept both "init" vs "init\0...".
     */
    return (obj_name[seg_len] == '\0' || obj_name[seg_len] != '/');
}

static void mem_copy(void *dst, const void *src, unsigned int n)
{
    unsigned char       *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    unsigned int i;
    for (i = 0; i < n; i++) d[i] = s[i];
}

void fs_init(void){
    fs_image_base = _binary_initfs_img_start;
    fs_superblock = (struct fs_super *)fs_image_base;
    fs_objects = (struct fs_obj *)(fs_superblock + 1);

    if (fs_superblock->magic != FS_MAGIC){
        fb_write("Bad magic\n", 10);
        for(;;);

    }
    if (fs_superblock->obj_count == 0){
        fb_write("Empty FS\n", 9);
        for(;;);
    }
}
// get object pinter by index, with bound check
static struct fs_obj *fs_get_obj(unsigned int idx)
{
    if(idx >= fs_superblock->obj_count) return 0;
    return &fs_objects[idx];
}
//path traversal starting from root index, follow abc seg
struct fs_obj *fs_lookup(const char *path)
{
    unsigned int cur_idx;
    struct fs_obj *cur;
    const char *p = path;

    if (!fs_superblock) return 0;

    /* Absolute paths only. Allow "/" or "/name/..." */
    if (p[0] == '/') p++;

    cur_idx = fs_superblock->root_index;
    cur = fs_get_obj(cur_idx);
    if (!cur) return 0;

    /* Empty path or "/" → return root */
    if (p[0] == '\0')
        return cur;

    while (*p != '\0') {
        /* Extract next component up to '/' or '\0' */
        char seg[FS_NAME_MAX];
        unsigned int seg_len = 0;

        /* Skip extra slashes */
        while (*p == '/') p++;
        if (*p == '\0') break;

        while (*p != '\0' && *p != '/' && seg_len < FS_NAME_MAX - 1) {
            seg[seg_len++] = *p++;
        }
        seg[seg_len] = '\0';

        /* Current must be a directory */
        if (cur->type != FS_TYPE_DIR)
            return 0;

        /* Directory body is array of unsigned int child indices */
        unsigned int *child_idx =
            (unsigned int *)(fs_image_base + cur->body_offset);
        unsigned int i;
        struct fs_obj *found = 0;

        for (i = 0; i < cur->entry_count; i++) {
            struct fs_obj *child = fs_get_obj(child_idx[i]);
            if (!child) continue;
            if (name_matches(child->name, seg, seg_len)) {
                found = child;
                break;
            }
        }

        if (!found)
            return 0;

        cur = found;

        /* If current char is '/', loop to handle next segment */
        while (*p == '/') p++;
    }

    return cur;
}
int fs_read(const struct fs_obj *obj, void *buf,
            unsigned int off, unsigned int len)
{
    unsigned int avail;
    unsigned int to_copy;
    unsigned char *src;

    if (!obj) return -1;
    if (obj->type != FS_TYPE_FILE) return -1;

    if (off >= obj->size)
        return 0;  /* EOF */

    avail = obj->size - off;
    if (len > avail)
        to_copy = avail;
    else
        to_copy = len;

    src = fs_image_base + obj->body_offset + off;
    mem_copy(buf, src, to_copy);

    return (int)to_copy;
}
    

  
