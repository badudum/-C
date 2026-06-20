#include "include/assembly_target.h"
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

#ifdef __linux__
#include <unistd.h>
#endif

static assembly_target_t current_target;
static int target_initialized = 0;
static assembly_os_t current_os = ASSEMBLY_OS_MACOS;
static int os_initialized = 0;

assembly_target_t assembly_target_get(void)
{
    if (!target_initialized) {
        current_target = assembly_target_detect_host();
        target_initialized = 1;
    }
    return current_target;
}

void assembly_target_set(assembly_target_t target)
{
    current_target = target;
    target_initialized = 1;
}

assembly_target_t assembly_target_parse(const char *name)
{
    if (!name)
        return assembly_target_detect_host();
    if (strcmp(name, "x86_64") == 0 || strcmp(name, "x86") == 0 ||
        strcmp(name, "amd64") == 0)
        return ASSEMBLY_TARGET_X86_64;
    if (strcmp(name, "arm64") == 0 || strcmp(name, "aarch64") == 0 ||
        strcmp(name, "arm") == 0)
        return ASSEMBLY_TARGET_AARCH64;
    fprintf(stderr, "Unknown target '%s', using host CPU\n", name);
    return assembly_target_detect_host();
}

assembly_target_t assembly_target_detect_host(void)
{
    struct utsname u;
    if (uname(&u) != 0)
        return ASSEMBLY_TARGET_AARCH64;

    if (strcmp(u.machine, "x86_64") == 0 || strcmp(u.machine, "amd64") == 0)
        return ASSEMBLY_TARGET_X86_64;
    if (strcmp(u.machine, "aarch64") == 0 || strcmp(u.machine, "arm64") == 0)
        return ASSEMBLY_TARGET_AARCH64;
    if (strcmp(u.machine, "i386") == 0 || strcmp(u.machine, "i686") == 0)
        return ASSEMBLY_TARGET_X86_64;

    return ASSEMBLY_TARGET_AARCH64;
}

const char *assembly_target_name(assembly_target_t target)
{
    return target == ASSEMBLY_TARGET_X86_64 ? "x86_64" : "arm64";
}

static assembly_os_t assembly_os_detect_host(void)
{
#ifdef __linux__
    return ASSEMBLY_OS_LINUX;
#else
    return ASSEMBLY_OS_MACOS;
#endif
}

assembly_os_t assembly_os_get(void)
{
    if (!os_initialized) {
        current_os = assembly_os_detect_host();
        os_initialized = 1;
    }
    return current_os;
}

const char *assembly_os_name(assembly_os_t os)
{
    return os == ASSEMBLY_OS_LINUX ? "linux" : "macos";
}
