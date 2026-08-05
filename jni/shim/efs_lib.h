/*
 * efs_lib.h — replacement for Noda's Embedded File System on the Android
 * port: EFS files are plain files under the app data directory.
 */
#ifndef _EFS_LIB_SHIM_H
#define _EFS_LIB_SHIM_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EFS_MAXPATHLEN      256
#define EFS_READBUFFERSIZE  4096

typedef struct EFS_FILE_ EFS_FILE;

int       EFS_Init(void);
EFS_FILE *EFS_fopen(const char *path);
size_t    EFS_fread(void *ptr, size_t size, size_t nmemb, EFS_FILE *f);
size_t    EFS_fwrite(const void *ptr, size_t size, size_t nmemb, EFS_FILE *f);
int       EFS_fseek(EFS_FILE *f, long offset, int whence);
long      EFS_ftell(EFS_FILE *f);
int       EFS_fclose(EFS_FILE *f);
long      EFS_GetFileSize(EFS_FILE *f);
void      EFS_Flush(EFS_FILE *f);

#ifdef __cplusplus
}
#endif

#endif
