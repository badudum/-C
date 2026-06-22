#ifndef IO_RT_H
#define IO_RT_H

/*
 * minusC I/O runtime (linked as io_rt.o).
 * String-returning functions allocate with malloc; caller owns the pointer.
 *
 * Security defaults (see io_rt.c):
 * - Relative paths only (set MC_IO_ALLOW_ABSOLUTE=1 to allow absolute paths)
 * - No ".." or "~" in paths; blocklist for /etc, /proc, /dev, /sys
 * - Bounded reads/writes and line length; O_NOFOLLOW on open when available
 */

const char *ReadLine(void);
int ReadChar(void);
int KeyAvailable(void);
int PollKey(int timeout_ms);

int FileOpen(const char *path, const char *mode);
const char *FileRead(int fd);
int FileWrite(int fd, const char *data);
int FileClose(int fd);
int WriteFile(const char *path, const char *data);

#endif
