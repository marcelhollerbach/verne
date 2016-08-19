#ifndef JESUS_ARCHIVE_H
#define JESUS_ARCHIVE_H

typedef enum {
    ARCHIVE_TYPE_ZIP,
    ARCHIVE_TYPE_XZ,
    ARCHIVE_TYPE_TAR_XZ,
    ARCHIVE_TYPE_TAR_GZ,
    ARCHIVE_TYPE_TAR_BZIP2
} Archive_Type;

void archive_extract(const char *path);
void archive_create(const char *path, Archive_Type type);

#endif