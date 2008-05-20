/** @file oyranos_io.h
 *
 *  Oyranos is an open source Colour Management System 
 *
 *  Copyright (C) 2004-2008  Kai-Uwe Behrmann
 *
 */

/**
 *  @brief    input / output  methods
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @license: new BSD <http://www.opensource.org/licenses/bsd-license.php>
 *  @since    2004/11/25
 */

/**
 *  This file contains functions for handling and writing files and directories.
 */

#ifndef OYRANOS_IO_H
#define OYRANOS_IO_H

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "oyranos.h"
#include "oyranos_internal.h"
#include "oyranos_helper.h"
#include "oyranos_debug.h"

/* --- Helpers  --- */

/* --- static variables   --- */

#define OY_WEB_RGB "sRGB.icc"
extern int oy_warn_;


int     oySetProfile_Block                (const char      * name,
                                           void            * mem,
                                           size_t            size,
                                           oyPROFILE_e       type,
                                           const char      * comnt);

/* --- internal API definition --- */

/* separate from the external functions */
char* oyGetPathFromProfileName_           (const char*   profilename,
                                           oyAllocFunc_t allocate_func);


char **  oyProfileListGet_           ( const char        * coloursig,
                                       uint32_t          * size );

size_t	oyGetProfileSize_                 (const char*   profilename);
void*	oyGetProfileBlock_                (const char*   profilename,
                                           size_t       *size,
                                           oyAllocFunc_t allocate_func);

char**  oyPolicyListGet_                  (int         * size);
char**  oyFileListGet_                    (const char * subpath,
                                           int        * size,
                                           int          data,
                                           int          owner);
char**  oyLibListGet_                     (const char * subpath,
                                           int        * size,
                                           int          owner);


/* oyranos part */
/* check for the global and the users directory */
void oyCheckDefaultDirectories_ ();

/* search in profile path and in current path */
char* oyFindProfile_ (const char* name);


#endif /* OYRANOS_IO_H */
