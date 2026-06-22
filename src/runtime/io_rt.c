#include "../include/io_rt.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Security-oriented limits (tune via env only for debugging, not documented API). */
#define MC_MAX_PATH_LEN 1024
#define MC_MAX_LINE_LEN 8192
#define MC_MAX_READ_BYTES (8 * 1024 * 1024)
#define MC_MAX_WRITE_BYTES (8 * 1024 * 1024)
#define MC_MAX_POLL_MS 60000
#define MC_MAX_FD 256

static char *mc_strdup(const char *s)
{
    if (!s)
        return NULL;
    size_t n = strlen(s) + 1;
    char *d = (char *)malloc(n);
    if (d)
        memcpy(d, s, n);
    return d;
}

static char *mc_strdup_empty(void)
{
    return mc_strdup("");
}

static int mc_env_truthy(const char *name)
{
    const char *v = getenv(name);
    return v && (v[0] == '1' || v[0] == 'y' || v[0] == 'Y' || v[0] == 't' || v[0] == 'T');
}

/* Reject traversal, absolute paths, control chars, and oversize paths. */
static int mc_path_is_safe(const char *path)
{
    if (!path)
        return 0;

    size_t len = strnlen(path, MC_MAX_PATH_LEN + 1);
    if (len == 0 || len > MC_MAX_PATH_LEN)
        return 0;

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)path[i];
        if (c < 32 || c == 127)
            return 0;
    }

    if (path[0] == '/' && !mc_env_truthy("MC_IO_ALLOW_ABSOLUTE"))
        return 0;
    if (path[0] == '~')
        return 0;
    if (strstr(path, "..") != NULL)
        return 0;

    /* Block obvious sensitive targets even if absolute paths are allowed. */
    if (path[0] == '/') {
        if (strncmp(path, "/etc/", 5) == 0 ||
            strncmp(path, "/proc/", 6) == 0 ||
            strncmp(path, "/sys/", 5) == 0 ||
            strncmp(path, "/dev/", 5) == 0 ||
            strncmp(path, "/private/etc/", 13) == 0)
            return 0;
    }

    return 1;
}

static int mc_mode_is_safe(const char *mode)
{
    if (!mode || !mode[0] || mode[1] != '\0')
        return 0;
    return mode[0] == 'r' || mode[0] == 'w' || mode[0] == 'a';
}

static int mc_fd_is_valid(int fd)
{
    if (fd < 0 || fd >= MC_MAX_FD)
        return 0;
    return fcntl(fd, F_GETFL) != -1;
}

static void mc_sanitize_c_string(char *s, size_t cap)
{
    if (!s || cap == 0)
        return;
    for (size_t i = 0; i < cap && s[i] != '\0'; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c < 32 && c != '\t')
            s[i] = ' ';
    }
}

static char *mc_read_fd_bounded(int fd, size_t max_bytes)
{
    size_t cap = 4096;
    if (cap > max_bytes + 1)
        cap = max_bytes + 1;
    size_t len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf)
        return NULL;

    while (len < max_bytes) {
        size_t chunk = max_bytes - len;
        if (chunk > 4096)
            chunk = 4096;
        if (len + chunk + 1 > cap) {
            size_t new_cap = cap * 2;
            if (new_cap > max_bytes + 1)
                new_cap = max_bytes + 1;
            char *nb = (char *)realloc(buf, new_cap);
            if (!nb) {
                free(buf);
                return NULL;
            }
            buf = nb;
            cap = new_cap;
        }
        ssize_t got = read(fd, buf + len, chunk);
        if (got < 0) {
            if (errno == EINTR)
                continue;
            free(buf);
            return NULL;
        }
        if (got == 0)
            break;
        len += (size_t)got;
    }
    buf[len] = '\0';
    mc_sanitize_c_string(buf, len + 1);
    return buf;
}

const char *FileRead(int fd)
{
    if (!mc_fd_is_valid(fd))
        return mc_strdup_empty();

    char *data = mc_read_fd_bounded(fd, MC_MAX_READ_BYTES);
    return data ? data : mc_strdup_empty();
}

const char *ReadLine(void)
{
    char line[MC_MAX_LINE_LEN];
    if (!fgets(line, sizeof(line), stdin))
        return mc_strdup_empty();

    size_t n = strlen(line);
    while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
        line[n - 1] = '\0';
        n--;
    }
    mc_sanitize_c_string(line, sizeof(line));
    return mc_strdup(line);
}

int ReadChar(void)
{
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n <= 0)
        return -1;
    return (int)c;
}

int KeyAvailable(void)
{
    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    pfd.revents = 0;
    int r = poll(&pfd, 1, 0);
    if (r <= 0)
        return 0;
    return (pfd.revents & POLLIN) ? 1 : 0;
}

int PollKey(int timeout_ms)
{
    if (timeout_ms < 0)
        timeout_ms = 0;
    if (timeout_ms > MC_MAX_POLL_MS)
        timeout_ms = MC_MAX_POLL_MS;

    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    pfd.revents = 0;
    int r = poll(&pfd, 1, timeout_ms);
    if (r <= 0 || !(pfd.revents & POLLIN))
        return -1;
    return ReadChar();
}

int FileOpen(const char *path, const char *mode)
{
    if (!mc_path_is_safe(path) || !mc_mode_is_safe(mode))
        return -1;

    int flags = 0;
    if (mode[0] == 'r') {
        flags = O_RDONLY;
    } else if (mode[0] == 'w') {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    }

#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif

    /* Owner read/write only for newly created files. */
    return (int)open(path, flags, 0600);
}

int FileWrite(int fd, const char *data)
{
    if (!mc_fd_is_valid(fd) || !data)
        return -1;

    size_t len = strlen(data);
    if (len > MC_MAX_WRITE_BYTES)
        return -1;

    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, data + off, len - off);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (n == 0)
            break;
        off += (size_t)n;
    }
    return (int)off;
}

int FileClose(int fd)
{
    if (!mc_fd_is_valid(fd))
        return -1;
    return close(fd) == 0 ? 0 : -1;
}

int WriteFile(const char *path, const char *data)
{
    if (!mc_path_is_safe(path))
        return -1;

    int fd = FileOpen(path, "w");
    if (fd < 0)
        return -1;
    int n = FileWrite(fd, data ? data : "");
    int ok = (n >= 0);
    FileClose(fd);
    return ok ? n : -1;
}
