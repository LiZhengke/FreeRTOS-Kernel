#ifndef FS_H
#define FS_H
#define MAX_FD 32

typedef struct {
    const file_ops *fops;
    void *private;
    int flags;
    int pos;
} file;

typedef struct {
    int (*open)(file *f, const char *path, int flags);
    int (*read)(file *f, void *buf, int size);
    int (*write)(file *f, const void *buf, int size);
    int (*close)(file *f);
} file_ops;

extern file fd_table[];
extern file_ops lfs_ops;
int fd_alloc(const file_ops *fops, void *private, int flags);

#endif /*FS_H*/
