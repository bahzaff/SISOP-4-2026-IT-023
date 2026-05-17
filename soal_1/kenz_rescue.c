#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>



// ======================================
// GANTI SESUAI PATH pwd LU
// ======================================

static const char *source_dir =
"/home/bahzaf/Documents/SISOP-4-2026-IT-023/soal_1/amba_files";



// ======================================
// GETATTR
// ======================================

static int x_getattr(const char *path,
                     struct stat *stbuf,
                     struct fuse_file_info *fi)
{
    (void) fi;

    memset(stbuf, 0, sizeof(struct stat));

    // root directory
    if (strcmp(path, "/") == 0)
    {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    // virtual file
    if (strcmp(path, "/tujuan.txt") == 0)
    {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = 66;
        return 0;
    }

    // file asli
    char fullpath[1024];

    snprintf(fullpath,
             sizeof(fullpath),
             "%s%s",
             source_dir,
             path);

    if (lstat(fullpath, stbuf) == -1)
        return -errno;

    return 0;
}



// ======================================
// READDIR
// ======================================

static int x_readdir(const char *path,
                     void *buf,
                     fuse_fill_dir_t filler,
                     off_t offset,
                     struct fuse_file_info *fi,
                     enum fuse_readdir_flags flags)
{
    (void) offset;
    (void) fi;
    (void) flags;

    DIR *dp;
    struct dirent *de;

    char fullpath[1024];

    snprintf(fullpath,
             sizeof(fullpath),
             "%s%s",
             source_dir,
             path);

    dp = opendir(fullpath);

    if (dp == NULL)
        return -errno;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    while ((de = readdir(dp)) != NULL)
    {
        filler(buf, de->d_name, NULL, 0, 0);
    }

    closedir(dp);

    // virtual file
    filler(buf, "tujuan.txt", NULL, 0, 0);

    return 0;
}



// ======================================
// OPEN
// ======================================

static int x_open(const char *path,
                  struct fuse_file_info *fi)
{
    (void) fi;

    // virtual file
    if (strcmp(path, "/tujuan.txt") == 0)
        return 0;

    char fullpath[1024];

    snprintf(fullpath,
             sizeof(fullpath),
             "%s%s",
             source_dir,
             path);

    int fd = open(fullpath, O_RDONLY);

    if (fd == -1)
        return -errno;

    close(fd);

    return 0;
}



// ======================================
// READ
// ======================================

static int x_read(const char *path,
                  char *buf,
                  size_t size,
                  off_t offset,
                  struct fuse_file_info *fi)
{
    (void) fi;

    // ==================================
    // FILE VIRTUAL tujuan.txt
    // ==================================

    if (strcmp(path, "/tujuan.txt") == 0)
    {
        static char result[4096];

        memset(result, 0, sizeof(result));

        strcpy(result, "Tujuan Mas Amba: ");

        char line[1024];

        for (int i = 1; i <= 7; i++)
        {
            char filepath[1024];

            snprintf(filepath,
                     sizeof(filepath),
                     "%s/%d.txt",
                     source_dir,
                     i);

            FILE *fp = fopen(filepath, "r");

            if (fp == NULL)
                continue;

            while (fgets(line, sizeof(line), fp))
            {
                char *found = strstr(line, "KOORD:");

                if (found != NULL)
                {
                    char temp[1024];

                    char *start = found + 6;

                    // buang spasi depan
                    while (*start == ' ')
                    {
                        start++;
                    }

                    strcpy(temp, start);

                    // buang newline
                    temp[strcspn(temp, "\n")] = 0;

                    strcat(result, temp);
                }
            }

            fclose(fp);
        }

        strcat(result, "\n");

        size_t len = strlen(result);

        if (offset >= len)
            return 0;

        if (offset + size > len)
            size = len - offset;

        memcpy(buf, result + offset, size);

        return size;
    }

    // ==================================
    // FILE ASLI PASSTHROUGH
    // ==================================

    char fullpath[1024];

    snprintf(fullpath,
             sizeof(fullpath),
             "%s%s",
             source_dir,
             path);

    int fd = open(fullpath, O_RDONLY);

    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);

    if (res == -1)
        res = -errno;

    close(fd);

    return res;
}



// ======================================
// OPERATIONS
// ======================================

static struct fuse_operations x_oper = {
    .getattr = x_getattr,
    .readdir = x_readdir,
    .open = x_open,
    .read = x_read,
};



// ======================================
// MAIN
// ======================================

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &x_oper, NULL);
}
