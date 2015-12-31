#ifndef ARCHIVE_H
#define ARCHIVE_H

int archive_init(void);
int archive_shutdown(void);

/*
 * Start to access a directory,
 * if directly is 1 the call will return when the archive has finished extracting
 * if it is 0 the function will extract asynchrone
 */
const char* archive_access(const char* archive, int directly);

/*
 * Call this when you are done with this archive
 */
int archive_unref(const char *archive);

int archive_support(const char *fileending);
#endif