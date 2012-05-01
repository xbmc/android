/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <dlfcn.h>
#include <sys/stat.h>
#include <linux/elf.h>
#include <fcntl.h>
#include <errno.h>

#include <android/log.h>

#include "xbmc_log.h"

static char *getXBFileName(const char * origName)
{
  if(strstr(origName,"lib") != origName)
    return NULL;
  char *str;
  str = (char *)malloc(sizeof(char*) * (strlen(origName)+3));
  strcpy (str,"libxb");
  strcat(str,origName + 3);
  return str;
}

static char *read_section(int fd, Elf32_Shdr *shdr)
{
    char *result;

    result = (char *)malloc(shdr->sh_size);
    if (lseek(fd, shdr->sh_offset, SEEK_SET) < 0) {
        close(fd);
        free(result);
        return NULL;
    }
    if (read(fd, result, shdr->sh_size) < (int) shdr->sh_size) {
        close(fd);
        free(result);
        return NULL;
    }

    return result;
}

static void free_ptrarray(void **pa)
{
    void **rover = pa;

    while (*rover != NULL)
        free(*rover++);

    free(pa);
}

char **lo_dlneeds(const char *library)
{
    int i, fd;
    int n_needed;
    char **result;
    char *shstrtab;
    char *dynstr = NULL;
    Elf32_Ehdr hdr;
    Elf32_Shdr shdr;
    Elf32_Dyn dyn;

    /* Open library and read ELF header */

    fd = open(library, O_RDONLY);

    if (fd == -1) {
        android_printf("lo_dlneeds: Could not open library %s: %s", library, strerror(errno));
        return NULL;
    }

    if (read(fd, &hdr, sizeof(hdr)) < (int) sizeof(hdr)) {
        android_printf("lo_dlneeds: Could not read ELF header of %s", library);
        close(fd);
        return NULL;
    }

    /* Read in .shstrtab */

    if (lseek(fd, hdr.e_shoff + hdr.e_shstrndx * sizeof(shdr), SEEK_SET) < 0) {
        android_printf("lo_dlneeds: Could not seek to .shstrtab section header of %s", library);
        close(fd);
        return NULL;
    }
    if (read(fd, &shdr, sizeof(shdr)) < (int) sizeof(shdr)) {
        android_printf("lo_dlneeds: Could not read section header of %s", library);
        close(fd);
        return NULL;
    }

    shstrtab = read_section(fd, &shdr);
    if (shstrtab == NULL)
        return NULL;

    /* Read section headers, looking for .dynstr section */

    if (lseek(fd, hdr.e_shoff, SEEK_SET) < 0) {
        android_printf("lo_dlneeds: Could not seek to section headers of %s", library);
        close(fd);
        return NULL;
    }
    for (i = 0; i < hdr.e_shnum; i++) {
        if (read(fd, &shdr, sizeof(shdr)) < (int) sizeof(shdr)) {
            android_printf("lo_dlneeds: Could not read section header of %s", library);
            close(fd);
            return NULL;
        }
        if (shdr.sh_type == SHT_STRTAB &&
            strcmp(shstrtab + shdr.sh_name, ".dynstr") == 0) {
            dynstr = read_section(fd, &shdr);
            if (dynstr == NULL) {
                free(shstrtab);
                return NULL;
            }
            break;
        }
    }

    if (i == hdr.e_shnum) {
        android_printf("lo_dlneeds: No .dynstr section in %s", library);
        close(fd);
        return NULL;
    }

    /* Read section headers, looking for .dynamic section */

    if (lseek(fd, hdr.e_shoff, SEEK_SET) < 0) {
        android_printf("lo_dlneeds: Could not seek to section headers of %s", library);
        close(fd);
        return NULL;
    }
    for (i = 0; i < hdr.e_shnum; i++) {
        if (read(fd, &shdr, sizeof(shdr)) < (int) sizeof(shdr)) {
            android_printf("lo_dlneeds: Could not read section header of %s", library);
            close(fd);
            return NULL;
        }
        if (shdr.sh_type == SHT_DYNAMIC) {
            size_t dynoff;

            /* Count number of DT_NEEDED entries */
            n_needed = 0;
            if (lseek(fd, shdr.sh_offset, SEEK_SET) < 0) {
                android_printf("lo_dlneeds: Could not seek to .dynamic section of %s", library);
                close(fd);
                return NULL;
            }
            for (dynoff = 0; dynoff < shdr.sh_size; dynoff += sizeof(dyn)) {
                if (read(fd, &dyn, sizeof(dyn)) < (int) sizeof(dyn)) {
                    android_printf("lo_dlneeds: Could not read .dynamic entry of %s", library);
                    close(fd);
                    return NULL;
                }
                if (dyn.d_tag == DT_NEEDED)
                    n_needed++;
            }

            /* LOGI("Found %d DT_NEEDED libs", n_needed); */

            result = (char **)malloc((n_needed+1) * sizeof(char *));

            n_needed = 0;
            if (lseek(fd, shdr.sh_offset, SEEK_SET) < 0) {
                android_printf("lo_dlneeds: Could not seek to .dynamic section of %s", library);
                close(fd);
                free(result);
                return NULL;
            }
            for (dynoff = 0; dynoff < shdr.sh_size; dynoff += sizeof(dyn)) {
                if (read(fd, &dyn, sizeof(dyn)) < (int) sizeof(dyn)) {
                    android_printf("lo_dlneeds: Could not read .dynamic entry in %s", library);
                    close(fd);
                    free(result);
                    return NULL;
                }
                if (dyn.d_tag == DT_NEEDED) {
                    /* LOGI("needs: %s\n", dynstr + dyn.d_un.d_val); */
                    result[n_needed] = strdup(dynstr + dyn.d_un.d_val);
                    n_needed++;
                }
            }

            close(fd);
            if (dynstr)
                free(dynstr);
            free(shstrtab);
            result[n_needed] = NULL;
            return result;
        }
    }

    android_printf("lo_dlneeds: Could not find .dynamic section in %s", library);
    close(fd);
    return NULL;
}

void *lo_dlopen(const char *library)
{
    /*
     * We should *not* try to just dlopen() the bare library name
     * first, as the stupid dynamic linker remembers for each library
     * basename if loading it has failed. Thus if you try loading it
     * once, and it fails because of missing needed libraries, and
     * your load those, and then try again, it fails with an
     * infuriating message "failed to load previously" in the log.
     *
     * We *must* first dlopen() all needed libraries, recursively. It
     * shouldn't matter if we dlopen() a library that already is
     * loaded, dlopen() just returns the same value then.
     */
    typedef struct {
        const char *name;
        void *handle;
        void *next;
    } *loadedLib;
    static loadedLib loaded_libraries;
    loadedLib rover;
    loadedLib new_loaded_lib;

    struct stat st;
    void *p;
    char *full_name;
    char *xb_name;
    char **needed;
    int i;
    int found;

    struct timeval tv0, tv1, tvdiff;

    static const char **library_locations;
    char libpath[] = "/data/data/org.xbmc/lib";
    char systempath[] = "/system/lib";
    char ffmpegpath[] = "/data/data/org.xbmc/cache/temp/assets/system/players/dvdplayer";
    library_locations = (const char **)malloc((4) * sizeof(char *));
    library_locations[0] = libpath;
    library_locations[1] = systempath;
    library_locations[2] = ffmpegpath;
    library_locations[3] = NULL;

    rover = loaded_libraries;
    while (rover && strcmp(rover->name, library) != 0)
        rover = (loadedLib)rover->next;

    if (rover != NULL)
        return rover->handle;

    android_printf("lo_dlopen(%s)", library);

    found = 0;
    if (library[0] == '/') {
        full_name = strdup(library);

        if (stat(full_name, &st) == 0 &&
            S_ISREG(st.st_mode))
            found = 1;
        else
            free(full_name);
    } else {
        for (i = 0; !found && library_locations[i] != NULL; i++) {
            full_name = (char*)malloc(strlen(library_locations[i]) + 1 + strlen(library) + 1);
            strcpy(full_name, library_locations[i]);
            strcat(full_name, "/");
            strcat(full_name, library);

            if (stat(full_name, &st) == 0 &&
                S_ISREG(st.st_mode))
                found = 1;
            else
                free(full_name);
        }
    }

    if (!found) {
        android_printf("lo_dlopen: Library %s not found", library);
        return NULL;
    }

    needed = lo_dlneeds(full_name);
    if (needed == NULL) {
        free(full_name);
        return NULL;
    }

    for (i = 0; needed[i] != NULL; i++)
    {
      xb_name = getXBFileName(needed[i]);
      if (xb_name == NULL || lo_dlopen(xb_name) == NULL)
      {
        free(xb_name);
        if (lo_dlopen(needed[i]) == NULL)
        {
              free_ptrarray((void **) needed);
              free(full_name);
              return NULL;
        }
      }
      else
        free(xb_name);
    }
    free_ptrarray((void **) needed);

    gettimeofday(&tv0, NULL);
    p = dlopen(full_name, RTLD_LOCAL);
    gettimeofday(&tv1, NULL);
    timersub(&tv1, &tv0, &tvdiff);
    android_printf("dlopen(%s) = %p, %ld.%03lds",
         full_name, p,
         (long) tvdiff.tv_sec, (long) tvdiff.tv_usec / 1000);
    free(full_name);
    if (p == NULL)
        android_printf("lo_dlopen: Error from dlopen(%s): %s", library, dlerror());

    new_loaded_lib = (loadedLib)malloc(sizeof(*new_loaded_lib));
    new_loaded_lib->name = strdup(library);
    new_loaded_lib->handle = p;

    new_loaded_lib->next = loaded_libraries;
    loaded_libraries = new_loaded_lib;

    return p;
}

