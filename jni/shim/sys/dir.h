/* devkitPro <sys/dir.h> compatibility for the Android/Linux port */
#ifndef _SHIM_SYS_DIR_H
#define _SHIM_SYS_DIR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DIR_ITER_ DIR_ITER;

DIR_ITER *diropen(const char *path);
int  dirreset(DIR_ITER *it);
int  dirnext(DIR_ITER *it, char *filename, void *st);
int  dirclose(DIR_ITER *it);

#ifdef __cplusplus
}
#endif

#endif
