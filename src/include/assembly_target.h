#ifndef ASSEMBLY_TARGET_H
#define ASSEMBLY_TARGET_H

typedef enum
{
    ASSEMBLY_TARGET_AARCH64 = 0,
    ASSEMBLY_TARGET_X86_64 = 1,
} assembly_target_t;

assembly_target_t assembly_target_get(void);
void assembly_target_set(assembly_target_t target);
assembly_target_t assembly_target_parse(const char *name);
assembly_target_t assembly_target_detect_host(void);
const char *assembly_target_name(assembly_target_t target);

typedef enum
{
    ASSEMBLY_OS_MACOS = 0,
    ASSEMBLY_OS_LINUX = 1,
} assembly_os_t;

assembly_os_t assembly_os_get(void);
const char *assembly_os_name(assembly_os_t os);

#endif
