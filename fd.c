#include <fcntl.h>
#include <stdlib.h>

#include "fd.h"
#include "lfs.h"

file fd_table[MAX_FD];
static lfs_t lfs;
int fd_alloc(const file_ops *fops, void *private, int flags) {
    for (int i = 0; i < MAX_FD; i++) {
        if (fd_table[i].fops == NULL) {
            fd_table[i].fops = fops;
            fd_table[i].private = private;
            fd_table[i].flags = flags;
            fd_table[i].pos = 0;
            return i;
        }
    }
    return -1;
}

/*int alloc_fd() {
    for (int i = 0; i < MAX_FD; i++) {
        if (fd_table[i].fops == NULL)
            return i;
    }
    return -1;
}*/

typedef struct {
    lfs_file_t lfs_file;
} lfs_file_priv_t;

int lfs_open(file *f, const char *path, int flags) {
    lfs_file_priv_t *priv = malloc(sizeof(*priv));

    int lfs_flags = 0;
    if (flags & O_CREAT) lfs_flags |= LFS_O_CREAT;
    if (flags & O_RDWR)  lfs_flags |= LFS_O_RDWR;

    int ret = lfs_file_open(&lfs, &priv->lfs_file, path, lfs_flags);
    if (ret < 0) return ret;

    f->private = priv;
    return 0;
}

int lfs_read(file *f, void *buf, int size) {
    lfs_file_priv_t *priv = f->private;
    return lfs_file_read(&lfs, &priv->lfs_file, buf, size);
}

int lfs_write(file *f, const void *buf, int size) {
    lfs_file_priv_t *priv = f->private;
    return lfs_file_write(&lfs, &priv->lfs_file, buf, size);
}

int lfs_close(file *f) {
    lfs_file_priv_t *priv = f->private;
    int ret = lfs_file_close(&lfs, &priv->lfs_file);
    free(priv);
    return ret;
}

file_ops lfs_ops = {
    .open  = lfs_open,
    .read  = lfs_read,
    .write = lfs_write,
    .close = lfs_close,
};
