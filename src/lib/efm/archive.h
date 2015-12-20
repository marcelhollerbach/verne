#ifndef ARCHIVE_H
#define ARCHIVE_H

int archive_init(void);
int archive_shutdown(void);

/*
 * Start to access a directory,
 * if directly is 1 the call will return when the archive has finished  extracting
 *
 */
const char* archive_access(const char* archive, int directly);

//call if you dont need the archive anymore
int archive_unref(const char *archive);


#endif