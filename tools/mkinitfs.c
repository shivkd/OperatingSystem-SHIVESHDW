/* tools/mkinitfs.c – build initfs.img from a directory tree
 *
 * Usage: mkinitfs <root_dir> <output.img>
 *
 * Layout written matches fs.h:
 *  [ fs_super ]
 *  [ fs_obj[obj_count] ]
 *  [ bodies for each object in order ]
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "../fs.h"   /* reuse struct defs and constants */

struct build_obj {
    struct fs_obj obj;
    unsigned char *body;    /* malloc'ed; for dirs: array of uint32_t child idx */
};

static struct build_obj *g_objs = NULL;
static unsigned int g_count = 0;
static unsigned int g_cap   = 0;

static void die(const char *msg)
{
    perror(msg);
    exit(1);
}

static void ensure_capacity(void)
{
    if (g_count >= g_cap) {
        unsigned int new_cap = (g_cap == 0) ? 16 : g_cap * 2;
        struct build_obj *new_arr =
            (struct build_obj *)realloc(g_objs, new_cap * sizeof(struct build_obj));
        if (!new_arr) die("realloc");
        g_objs = new_arr;
        g_cap  = new_cap;
    }
}

static unsigned int add_object(const char *name, unsigned int type)
{
    ensure_capacity();
    memset(&g_objs[g_count], 0, sizeof(struct build_obj));

    strncpy(g_objs[g_count].obj.name, name, FS_NAME_MAX - 1);
    g_objs[g_count].obj.name[FS_NAME_MAX - 1] = '\0';
    g_objs[g_count].obj.type = type;
    g_objs[g_count].obj.size = 0;
    g_objs[g_count].obj.body_offset = 0;
    g_objs[g_count].obj.entry_count = 0;

    return g_count++;
}

static int is_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
}

static int is_regular(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode);
}

/* Forward declaration */
static unsigned int build_dir(const char *host_path, const char *name);

/* Build file object: read whole file into memory */
static unsigned int build_file(const char *host_path, const char *name)
{
    unsigned int idx = add_object(name, FS_TYPE_FILE);
    FILE *f = fopen(host_path, "rb");
    long sz;
    size_t nread;

    if (!f) die("fopen file");

    if (fseek(f, 0, SEEK_END) != 0) die("fseek end");
    sz = ftell(f);
    if (sz < 0) die("ftell");
    if (fseek(f, 0, SEEK_SET) != 0) die("fseek set");

    g_objs[idx].obj.size = (unsigned int)sz;
    if (sz > 0) {
        g_objs[idx].body = (unsigned char *)malloc((size_t)sz);
        if (!g_objs[idx].body) die("malloc body");
        nread = fread(g_objs[idx].body, 1, (size_t)sz, f);
        if (nread != (size_t)sz) die("fread");
    }

    fclose(f);
    return idx;
}

/* Recursively build directory object */
static unsigned int build_dir(const char *host_path, const char *name)
{
    unsigned int idx = add_object(name, FS_TYPE_DIR);
    DIR *d = opendir(host_path);
    struct dirent *de;
    uint32_t *children = NULL;
    unsigned int child_count = 0;
    unsigned int child_cap   = 0;

    if (!d) die("opendir");

    while ((de = readdir(d)) != NULL) {
        char child_path[1024];
        unsigned int child_idx;

        if (strcmp(de->d_name, ".") == 0)  continue;
        if (strcmp(de->d_name, "..") == 0) continue;

        snprintf(child_path, sizeof(child_path),
                 "%s/%s", host_path, de->d_name);

        if (is_dir(child_path)) {
            child_idx = build_dir(child_path, de->d_name);
        } else if (is_regular(child_path)) {
            child_idx = build_file(child_path, de->d_name);
        } else {
            /* skip other types */
            continue;
        }

        if (child_count >= child_cap) {
            unsigned int new_cap = (child_cap == 0) ? 8 : child_cap * 2;
            uint32_t *new_arr =
                (uint32_t *)realloc(children, new_cap * sizeof(uint32_t));
            if (!new_arr) die("realloc children");
            children   = new_arr;
            child_cap  = new_cap;
        }
        children[child_count++] = child_idx;
    }

    closedir(d);

    /* Store children array as directory body */
    g_objs[idx].obj.entry_count = child_count;
    if (child_count > 0) {
        size_t bytes = child_count * sizeof(uint32_t);
        g_objs[idx].body = (unsigned char *)malloc(bytes);
        if (!g_objs[idx].body) die("malloc dir body");
        memcpy(g_objs[idx].body, children, bytes);
        g_objs[idx].obj.size = (unsigned int)bytes;
    }

    free(children);
    return idx;
}

int main(int argc, char **argv)
{
    const char *root_dir;
    const char *out_path;
    struct fs_super super;
    FILE *out;
    unsigned int i;
    unsigned int offset;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <root_dir> <output.img>\n", argv[0]);
        return 1;
    }

    root_dir = argv[1];
    out_path = argv[2];

    /* Build object tree: root directory at index root_idx */
    unsigned int root_idx = build_dir(root_dir, ""); /* root gets empty name */

    /* Fill super */
    memset(&super, 0, sizeof(super));
    super.magic      = FS_MAGIC;
    super.version    = 1;
    super.root_index = root_idx;
    super.obj_count  = g_count;

    /* Compute body offsets.
     * Layout:
     *   [super][obj array][bodies...]
     */
    offset = sizeof(struct fs_super) + g_count * sizeof(struct fs_obj);
    for (i = 0; i < g_count; i++) {
        g_objs[i].obj.body_offset = offset;
        offset += g_objs[i].obj.size;
    }

    out = fopen(out_path, "wb");
    if (!out) die("fopen output");

    /* Write superblock */
    if (fwrite(&super, 1, sizeof(super), out) != sizeof(super))
        die("fwrite super");

    /* Write object headers */
    for (i = 0; i < g_count; i++) {
        if (fwrite(&g_objs[i].obj, 1, sizeof(struct fs_obj), out)
            != sizeof(struct fs_obj))
            die("fwrite obj");
    }

    /* Write bodies */
    for (i = 0; i < g_count; i++) {
        if (g_objs[i].obj.size > 0) {
            if (fwrite(g_objs[i].body, 1, g_objs[i].obj.size, out)
                != g_objs[i].obj.size)
                die("fwrite body");
        }
    }

    fclose(out);
    return 0;
}
