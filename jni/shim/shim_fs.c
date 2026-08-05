/*
 * shim_fs.c — virtual filesystem: game paths ("/maps/...", "/settings",
 * "/highscores", "/last_game") are remapped under a root directory
 * (Android: the app's internal files dir; host tests: a work dir).
 * Also provides the devkitPro dirent API and the EFS replacement.
 */
#include <PA9.h>
#undef fopen
#undef main
#include <dirent.h>
#include <sys/stat.h>
#include "efs_lib.h"
#include "sys/dir.h"

static char fs_root[512] = ".";

void shim_SetFsRoot(const char *root)
{
    snprintf(fs_root, sizeof(fs_root), "%s", root);
}

const char *shim_GetFsRoot(void) { return fs_root; }

static void map_path(const char *in, char *out, size_t outsz)
{
    if (in[0] == '/')
        snprintf(out, outsz, "%s%s", fs_root, in);
    else
        snprintf(out, outsz, "%s/%s", fs_root, in);
}

FILE *shim_fopen(const char *path, const char *mode)
{
    char full[1024];
    map_path(path, full, sizeof(full));
    return fopen(full, mode);
}

/* ---------------- devkitPro-style directory iteration ---------------- */

struct DIR_ITER_ {
    DIR *d;
    char path[1024];
};

DIR_ITER *diropen(const char *path)
{
    char full[1024];
    map_path(path, full, sizeof(full));
    DIR *d = opendir(full);
    if (!d) return NULL;
    DIR_ITER *it = (DIR_ITER *)malloc(sizeof(DIR_ITER));
    it->d = d;
    snprintf(it->path, sizeof(it->path), "%s", full);
    return it;
}

int dirreset(DIR_ITER *it)
{
    if (!it) return -1;
    rewinddir(it->d);
    return 0;
}

int dirnext(DIR_ITER *it, char *filename, void *st)
{
    (void)st;
    if (!it) return -1;
    struct dirent *e = readdir(it->d);
    if (!e) return -1;
    /* game expects at most 256 bytes */
    strncpy(filename, e->d_name, 255);
    filename[255] = '\0';
    return 0;
}

int dirclose(DIR_ITER *it)
{
    if (!it) return -1;
    closedir(it->d);
    free(it);
    return 0;
}

/* ---------------- EFS replacement ---------------- */

struct EFS_FILE_ { FILE *f; };

int EFS_Init(void) { return 1; }

EFS_FILE *EFS_fopen(const char *path)
{
    char full[1024];
    map_path(path, full, sizeof(full));
    FILE *f = fopen(full, "r+b");
    if (!f)
        f = fopen(full, "w+b");
    if (!f)
        return NULL;
    EFS_FILE *e = (EFS_FILE *)malloc(sizeof(EFS_FILE));
    e->f = f;
    return e;
}

size_t EFS_fread(void *ptr, size_t size, size_t nmemb, EFS_FILE *f)
{
    if (!f) { memset(ptr, 0, size * nmemb); return 0; }
    size_t got = fread(ptr, 1, size * nmemb, f->f);
    if (got < size * nmemb)
        memset((u8 *)ptr + got, 0, size * nmemb - got);
    return got / (size ? size : 1);
}

size_t EFS_fwrite(const void *ptr, size_t size, size_t nmemb, EFS_FILE *f)
{
    if (!f) return 0;
    return fwrite(ptr, 1, size * nmemb, f->f) / (size ? size : 1);
}

int EFS_fseek(EFS_FILE *f, long offset, int whence)
{
    return f ? fseek(f->f, offset, whence) : -1;
}

long EFS_ftell(EFS_FILE *f) { return f ? ftell(f->f) : -1; }

int EFS_fclose(EFS_FILE *f)
{
    if (!f) return -1;
    fclose(f->f);
    free(f);
    return 0;
}

long EFS_GetFileSize(EFS_FILE *f)
{
    if (!f) return 0;
    long cur = ftell(f->f), sz;
    fseek(f->f, 0, SEEK_END);
    sz = ftell(f->f);
    fseek(f->f, cur, SEEK_SET);
    return sz;
}

void EFS_Flush(EFS_FILE *f) { if (f) fflush(f->f); }
