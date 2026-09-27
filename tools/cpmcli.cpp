/*----------------------------------------------------------------------------
  cpmcli - small headless harness around the portable CP/M engine.

  This exists so the engine can be driven and verified from a terminal (or CI)
  without a GUI. It is a thin wrapper over cpm_test.c, the same entry points the
  Qt front end uses.

  Note: this is built as C++ because cpm_test.h declares its prototypes inside
  a bare `extern "C" { ... }` block (it was written for C++Builder).

  Usage:
    cpmcli <diskdefs> fmt
    cpmcli <diskdefs> mkfs <image> <format> [boot1..boot4]
    cpmcli <diskdefs> ls   <image> <format> [pattern]
    cpmcli <diskdefs> get  <image> <format> <name> <dest>
    cpmcli <diskdefs> put  <image> <format> <src>  <name>
    cpmcli <diskdefs> rm   <image> <format> <name>
----------------------------------------------------------------------------*/
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "cpm_test.h"

extern "C" {
/*! Defined by the engine; sized [261] in cpmfs.c. */
extern char defpath[261];
extern char cur_defpath[261];

/*! Frees the (char**, count) pair produced by cpm_ls_test(). */
void cpmglobfree(char **dirent, int entries);
}

#define MAX_BOOT 4

static void usage()
{
    std::fprintf(stderr,
                 "usage: cpmcli <diskdefs> fmt\n"
                 "       cpmcli <diskdefs> mkfs <image> <format> [boot1..boot4]\n"
                 "       cpmcli <diskdefs> ls   <image> <format> [pattern]\n"
                 "       cpmcli <diskdefs> get  <image> <format> <name> <dest>\n"
                 "       cpmcli <diskdefs> put  <image> <format> <src>  <name>\n"
                 "       cpmcli <diskdefs> rm   <image> <format> <name>\n");
}

/*! Report an engine error (NULL means success) and return the exit code. */
static int check(const char *what, const char *err)
{
    if (err != nullptr) {
        std::fprintf(stderr, "cpmcli: %s: %s\n", what, err);
        return 1;
    }
    return 0;
}

static int cmd_fmt()
{
    int    count = 0;
    char **names = nullptr;

    if (check("list formats", cpm_fmt_test(&count, &names)) != 0)
        return 1;

    for (int i = 0; i < count; ++i)
        std::printf("%s\n", names[i]);

    for (int i = 0; i < count; ++i)
        std::free(names[i]);
    std::free(names);

    std::printf("(%d formats)\n", count);
    return 0;
}

static int cmd_mkfs(int argc, char *argv[])
{
    char *image  = argv[0];
    char *format = argv[1];
    char *boot[MAX_BOOT];
    int   nboot = argc - 2;

    if (nboot > MAX_BOOT)
        nboot = MAX_BOOT;

    /* creat_cpm() walks the array until it hits NULL, so every slot must be
       initialised even when fewer than four boot files are supplied. */
    for (int i = 0; i < MAX_BOOT; ++i)
        boot[i] = nullptr;
    for (int i = 0; i < nboot; ++i)
        boot[i] = argv[2 + i];

    return check("mkfs", creat_cpm_test(image, format, boot));
}

static int cmd_ls(int argc, char *argv[])
{
    char *image   = argv[0];
    char *format  = argv[1];
    char *pattern = (argc > 2) ? argv[2] : const_cast<char *>("*.*");
    int   count   = 0;
    char **names  = nullptr;

    if (check("ls", cpm_ls_test(image, format, pattern, &count, &names)) != 0)
        return 1;

    for (int i = 0; i < count; ++i)
        std::printf("%s\n", names[i]);

    if (count > 0)
        cpmglobfree(names, count);

    std::printf("(%d entries)\n", count);
    return 0;
}

static int cmd_get(int argc, char *argv[])
{
    if (argc != 4) { usage(); return 2; }
    return check("get", cpm_to_win_test(argv[0], argv[1], argv[2], argv[3]));
}

static int cmd_put(int argc, char *argv[])
{
    if (argc != 4) { usage(); return 2; }
    return check("put", win_to_cpm_test(argv[0], argv[1], argv[2], argv[3]));
}

static int cmd_rm(int argc, char *argv[])
{
    if (argc != 3) { usage(); return 2; }
    return check("rm", cpm_rm_test(argv[0], argv[1], argv[2]));
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        usage();
        return 2;
    }

    /* argv[1] is the diskdefs file the engine should read. */
    std::strncpy(defpath, argv[1], 260);
    defpath[260] = '\0';
    cur_defpath[0] = '\0';

    const char *command = argv[2];
    int rc = 2;

    if (std::strcmp(command, "fmt") == 0)
        rc = cmd_fmt();
    else if (std::strcmp(command, "mkfs") == 0 && argc >= 5)
        rc = cmd_mkfs(argc - 3, argv + 3);
    else if (std::strcmp(command, "ls") == 0 && argc >= 5)
        rc = cmd_ls(argc - 3, argv + 3);
    else if (std::strcmp(command, "get") == 0)
        rc = cmd_get(argc - 3, argv + 3);
    else if (std::strcmp(command, "put") == 0)
        rc = cmd_put(argc - 3, argv + 3);
    else if (std::strcmp(command, "rm") == 0)
        rc = cmd_rm(argc - 3, argv + 3);
    else
        usage();

    return rc;
}
