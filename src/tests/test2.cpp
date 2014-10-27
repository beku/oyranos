/** @file test_oyranos.c
 *
 *  Oyranos is an open source Color Management System 
 *
 *  Copyright (C) 2004-2014  Kai-Uwe Behrmann
 *
 *  @brief    Oyranos test suite
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:\n
 *  new BSD <http://www.opensource.org/licenses/bsd-license.php>
 *  @since    2008/12/04
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "oyranos.h"
#include "oyranos_i18n.h"
#include "oyranos_string.h"
#include "oyranos_xml.h"
#include "oyranos_config_internal.h"

/* C++ includes and definitions */
#ifdef __cplusplus
void* myAllocFunc(size_t size) { return new char [size]; }
#include <fstream>
#include <iostream>
#else
void* myAllocFunc(size_t size) { return calloc(size,1); }
#endif

#include <cmath>


/* --- general test routines --- */

typedef enum {
  oyTESTRESULT_SUCCESS,
  oyTESTRESULT_FAIL,
  oyTESTRESULT_XFAIL,
  oyTESTRESULT_SYSERROR,
  oyTESTRESULT_UNKNOWN
} oyTESTRESULT_e;


const char * oyTestResultToString    ( oyTESTRESULT_e      error )
{
  const char * text = "";
  switch(error)
  {
    case oyTESTRESULT_SUCCESS: text = "SUCCESS"; break;
    case oyTESTRESULT_FAIL:    text = "FAIL"; break;
    case oyTESTRESULT_XFAIL:   text = "XFAIL"; break;
    case oyTESTRESULT_SYSERROR:text = "SYSERROR"; break;
    case oyTESTRESULT_UNKNOWN: text = "UNKNOWN"; break;
    default:                   text = "Huuch, whats that?"; break;
  }
  return text;
}

const char  *  oyIntToString         ( int                 integer )
{
  static char texts[3][255];
  static int a = 0;
  int i;

  if(a >= 3) a = 0;

  for(i = 0; i < 8-log10(integer); ++i)
    sprintf( &texts[a][i], " " );

  sprintf( &texts[a][i], "%d", integer );

  return texts[a++];
}

const char  *  oyProfilingToString   ( int                 integer,
                                       double              duration,
                                       const char        * term )
{
  static char texts[3][255];
  static int a = 0;
  int i, len;

  if(a >= 3) a = 0;

  if(integer/duration >= 1000000.0)
    sprintf( &texts[a][0], "%.02f M%s/s", integer/duration/1000000.0, term );
  else
    sprintf( &texts[a][0], "%.00f %s/s", integer/duration, term );

  len = strlen(&texts[a][0]);

  for(i = 0; i < 16-len; ++i)
    sprintf( &texts[a][i], " " );

  if(integer/duration >= 1000000.0)
    sprintf( &texts[a][i], "%.02f M%s/s", integer/duration/1000000.0, term );
  else
    sprintf( &texts[a][i], "%.00f %s/s", integer/duration, term );

  return texts[a++];
}

FILE * zout = stdout;  /* printed inbetween results */

#define PRINT_SUB( result_, ... ) { \
  if(result == oyTESTRESULT_XFAIL || \
     result == oyTESTRESULT_SUCCESS || \
     result == oyTESTRESULT_UNKNOWN ) \
    result = result_; \
  fprintf(stdout, ## __VA_ARGS__ ); \
  fprintf(stdout, " ..\t%s", oyTestResultToString(result_)); \
  if(result_ && result_ != oyTESTRESULT_XFAIL) \
    fprintf(stdout, " !!! ERROR !!!" ); \
  fprintf(stdout, "\n" ); \
}


/* --- actual tests --- */

oyTESTRESULT_e testVersion()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );
  fprintf(zout, "compiled version:     %d\n", OYRANOS_VERSION );
  fprintf(zout, " runtime version:     %d\n", oyVersion(0) );

  if(OYRANOS_VERSION == oyVersion(0))
    result = oyTESTRESULT_SUCCESS;
  else
    result = oyTESTRESULT_FAIL;

  return result;
}

#include <locale.h>
#include "oyranos_sentinel.h"

oyTESTRESULT_e testI18N()
{
  const char * lang = 0;
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  oyI18Nreset();

  lang = oyLanguage();
  if((lang && (strcmp(lang, "C") == 0)) || !lang)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyLanguage() uninitialised good %s                ", lang );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyLanguage() uninitialised failed                 " );
  }

  setlocale(LC_ALL,"");
  oyI18Nreset();

  lang = oyLanguage();
  if(lang && (strcmp(lang, "C") != 0))
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyLanguage() initialised good %s                  ", lang );
  } else
  { PRINT_SUB( oyTESTRESULT_XFAIL, 
    "oyLanguage() initialised failed %s                ", lang );
  }

  return result;
}

#define TEST_DOMAIN "sw/Oyranos/Tests"

#include "oyranos_elektra.h"
oyTESTRESULT_e testElektra()
{
  int error = 0;
  char * value = 0,
       * start = 0;
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  oyExportReset_(EXPORT_SETTING);

  fprintf(stdout, "\n" );

  error = oyAddKey_valueComment_(TEST_DOMAIN "/test_key",
                                 "NULLTestValue", "NULLTestComment" );
  start = oyGetKeyString_(TEST_DOMAIN "/test_key", 0);
  printf ("start is %s\n", start);
  if(!start)
  {
    oyExportStart_(EXPORT_CHECK_NO);
    oyExportEnd_();
    error = oyAddKey_valueComment_(TEST_DOMAIN "/test_key",
                                 "NULLTestValue", "NULLTestComment" );
    start = oyGetKeyString_(TEST_DOMAIN "/test_key", 0);
    printf ("start is %s\n", start);
    
    PRINT_SUB( start?oyTESTRESULT_SUCCESS:oyTESTRESULT_XFAIL,
    "Elektra not initialised? try oyExportStart_(EXPORT_CHECK_NO)" );
  }
  if(!start)
  {
    oyExportStart_(EXPORT_SETTING);
    oyExportEnd_();
    error = oyAddKey_valueComment_(TEST_DOMAIN "/test_key",
                                 "NULLTestValue", "NULLTestComment" );
    start = oyGetKeyString_(TEST_DOMAIN "/test_key", 0);
    PRINT_SUB( start?oyTESTRESULT_SUCCESS:oyTESTRESULT_XFAIL, 
    "Elektra not initialised? try oyExportStart_(EXPORT_SETTING)" );
  }
  if(start)
    fprintf(zout, "start key value: %s\n", start );
  else
    fprintf(zout, "could not initialise\n" );

  error = oyAddKey_valueComment_(TEST_DOMAIN "/test_key",
                                 "myTestValue", "myTestComment" );
  value = oyGetKeyString_(TEST_DOMAIN "/test_key", 0);
  if(value)
    fprintf(zout, "result key value: %s\n", value );

  if(error)
  {
    PRINT_SUB( oyTESTRESULT_SYSERROR, 
    "Elektra error: %d", error );
  } else
  /* we want "start" to be different from "value" */
  if(start && value && strcmp(start,value) == 0)
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "Elektra (start!=value) failed: %s|%s", start, value );
  } else
  if(!value)
  {
    if(!value)
      PRINT_SUB( oyTESTRESULT_FAIL, 
      "Elektra (value) failed" );
    if(!start)
      PRINT_SUB( oyTESTRESULT_FAIL, 
      "Elektra (init) failed" );
  } else
  if(value)
  {
    if(strcmp(value,"myTestValue") == 0)
    {
      PRINT_SUB( oyTESTRESULT_SUCCESS, 
      "Elektra (value): %s", value );
    } else
    {
      PRINT_SUB( oyTESTRESULT_FAIL, 
      "Elektra (value) wrong: %s", value );
    }
  } else
    result = oyTESTRESULT_SUCCESS;

  error = oyEraseKey_( TEST_DOMAIN "/test_key" );
  value = oyGetKeyString_(TEST_DOMAIN "/test_key", 0);
  if(value && strlen(value))
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "Elektra key not erased" );
  } else
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS,
    "Elektra key erased" );
  }

  return result;
}

#ifdef __cplusplus
extern "C" {
#endif
char *         oyStrnchr_            ( char              * text,
                                       char                delimiter,
                                       int                 len );
int    oyStringSegmentsN_            ( const char        * text,
                                       int                 len,
                                       char                delimiter );
int    oyStringSegments_             ( const char        * text,
                                       char                delimiter );
char *             oyStringSegment_  ( char              * text,
                                       char                delimiter,
                                       int                 segment,
                                       int               * end );
char *             oyStringSegmentN_ ( char              * text,
                                       int                 len,
                                       char                delimiter,
                                       int                 segment,
                                       int               * end );
char *         oyFilterRegistrationToSTextField (
                                       const char        * registration,
                                       oyFILTER_REG_e      field,
                                       int               * end );

#ifdef __cplusplus
}
#endif

oyTESTRESULT_e testStringRun ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int error = 0,
      i;

  const char * test = OY_INTERNAL "/display.oydi/display_name";
  
  int test_n = oyStringSegmentsN_( test, oyStrlen_(test), OY_SLASH_C );
  test_n = oyStringSegments_( test, OY_SLASH_C );
  char * test_out = (char*) malloc(strlen(test));
  char * test_sub = 0;
  int test_end;

  fprintf(zout, "\"%s\"\n", test );

  error = 0;
  if(test_n != 5) error = 1;
  for(i = 0; i < test_n; ++i)
  {
    int test_end = 0;
    test_sub = oyStringSegment_( (char*) test, OY_SLASH_C, i,
                                           &test_end );
    int test_sub_n = oyStringSegmentsN_( test_sub, test_end, '.' );

    switch(i) {
      case 0: if(test_sub_n != 1) error = 1; break;
      case 1: if(test_sub_n != 1) error = 1; break;
      case 2: if(test_sub_n != 1) error = 1; break;
      case 3: if(test_sub_n != 2) error = 1; break;
      case 4: if(test_sub_n != 1) error = 1; break;
      default: error = 1;
    }

    int j;
    for(j = 0; j < test_sub_n; ++j)
    {
      int test_end2 = 0;
      char * test_sub2 = oyStringSegmentN_( test_sub, test_end, '.', j,
                                               &test_end2 );
      memcpy( test_out, test_sub2, test_end2 );
      test_out[test_end2] = 0;
      fprintf(zout, "%d%c%d%c \"%s\"\n", i, j?' ':'/',j, j ? '.': ' ',
                      test_out);
    }
  }

  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyStringSegmentxxx()...                            " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyStringSegmentxxx()...                            " );
  }


  double clck = oyClock();
  for(i = 0; i < 1000000; ++i)
    test_sub = oyFilterRegistrationToSTextField( test, oyFILTER_REG_OPTION,
                                                 &test_end );
  clck = oyClock() - clck;

  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyFilterRegistrationToSTextField()          %d %.03f", i,
                                       (double)clck/(double)CLOCKS_PER_SEC );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyFilterRegistrationToSTextField()                 " );
  }


  clck = oyClock();
  for(i = 0; i < 1000000; ++i)
    test_sub = oyFilterRegistrationToText( test, oyFILTER_REG_OPTION, 0 );
  clck = oyClock() - clck;

  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyFilterRegistrationToText()                %d %.03f", i,
                                       (double)clck/(double)CLOCKS_PER_SEC );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyFilterRegistrationToText())                      " );
  }



  test = "//" OY_TYPE_STD "/display.oydi/";
  fprintf(zout, "\"%s\"\n", test );
  test_n = oyStringSegmentsN_( test, oyStrlen_(test), OY_SLASH_C );
  test_n = oyStringSegments_( test, OY_SLASH_C );

  error = 0;
  if(test_n != 5) error = 1;
  for(i = 0; i < test_n; ++i)
  {
    int test_end = 0;
    test_sub = oyStringSegment_( (char*) test, OY_SLASH_C, i,
                                           &test_end );
    int test_sub_n = oyStringSegmentsN_( test_sub, test_end, '.' );

    switch(i) {
      case 0: if(test_sub_n != 1) error = 1; break;
      case 1: if(test_sub_n != 1) error = 1; break;
      case 2: if(test_sub_n != 1) error = 1; break;
      case 3: if(test_sub_n != 2) error = 1; break;
      case 4: if(test_sub_n != 1) error = 1; break;
      default: error = 1;
    }

    int j;
    for(j = 0; j < test_sub_n; ++j)
    {
      int test_end2 = 0;
      char * test_sub2 = oyStringSegmentN_( test_sub, test_end, '.', j,
                                               &test_end2 );
      memcpy( test_out, test_sub2, test_end2 );
      test_out[test_end2] = 0;
      fprintf(zout, "%d%c%d%c \"%s\"\n", i, j?' ':'/', j, j ? '.': ' ',
                      test_out);
    }
  }
  free(test_out);

  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyStringSegmentxxx()...                            " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyStringSegmentxxx()...                            " );
  }

  return result;
}


oyTESTRESULT_e testOption ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  int error = 0;
  oyOption_s * o;
  const char * test_buffer = "test";
  size_t size = strlen(test_buffer);
  oyPointer ptr = oyAllocateFunc_( size );

  oyExportReset_(EXPORT_SETTING);

  fprintf(stdout, "\n" );

  o = oyOption_FromRegistration( 0, 0 );
  if(o)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_New() good                               " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_New() failed                             " );
  }

  error = oyOption_FromRegistration( "blabla", 0 ) != 0;
  if(o)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_New() with wrong registration rejected: ok");
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_New() with wrong registration not rejected");
  }

  memcpy( ptr, test_buffer, size );
  error = oyOption_SetFromData( o, ptr, size );
  if(!error)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_SetFromData() good                       " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_SetFromData() failed                     " );
  }

  oyDeAllocateFunc_( ptr ); ptr = 0;
  size = 0;

  ptr = oyOption_GetData( o, &size, oyAllocateFunc_ );
  if(ptr && size && memcmp( ptr, test_buffer, 4 ) == 0)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_GetData() good                           " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_GetData() failed                         " );
  }

  oyOption_Release( &o );

  return result;
}

#include "oyOption_s_.h"

oyTESTRESULT_e testOptionInt ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  int error = 0;
  oyOption_s * o = 0;
  oyOption_s_ * oi = 0;
  int32_t erg[4] = { -1,-1,-1,-1 };

  oyExportReset_(EXPORT_SETTING);

  fprintf(stdout, "\n" );

  o = oyOption_FromRegistration( "//" OY_TYPE_STD "/filter/x", 0 );
  oi = (oyOption_s_*) o;

  error = oyOption_SetFromInt( o, 0, 0, 0 );
  if(!error && oi->value &&
     oi->value->int32 == 0 &&
     oi->value_type == oyVAL_INT)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_SetFromInt() good                        " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_SetFromInt() failed                      " );
  }

  error = oyOption_SetFromInt( o, 58293, 0, 0 );
  if(!error && oi->value &&
     oi->value->int32 == 58293 &&
     oi->value_type == oyVAL_INT)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_SetFromInt() single int32_t good         " );
    erg[0] = oyOption_GetValueInt( o, 0 );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_SetFromInt() single int32_t failed       " );
  }

  error = oyOption_SetFromInt( o, 58293, 1, 0 );
  if(!error && oi->value &&
     oi->value->int32_list[0] == 2 &&
     oi->value->int32_list[1] == 58293 &&
     oi->value->int32_list[2] == 58293 &&
     oi->value_type == oyVAL_INT_LIST)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_SetFromInt() add int32_t list good     " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_SetFromInt() add int32_t list failed   " );
  }

  error = oyOption_SetFromInt( o, 58293, 2, 0 );
  if(!error && oi->value &&
     oi->value->int32_list[0] == 3 &&
     oi->value->int32_list[1] == 58293 &&
     oi->value->int32_list[2] == 58293 &&
     oi->value->int32_list[3] == 58293 &&
     oi->value_type == oyVAL_INT_LIST)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_SetFromInt() add int32_t list good     " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_SetFromInt() add int32_t list failed   " );
  }

  error = oyOption_SetFromInt( o, 58293, 1, 0 );
  if(!error && oi->value &&
     oi->value->int32_list[0] == 3 &&
     oi->value->int32_list[1] == 58293 &&
     oi->value->int32_list[2] == 58293 &&
     oi->value->int32_list[3] == 58293 &&
     oi->value_type == oyVAL_INT_LIST)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_SetFromInt() new int32_t list good     " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_SetFromInt() new int32_t list failed   " );
  }

  error = oyOption_SetFromInt( o, 293, 1, 0 );
  if(!error && oi->value &&
     oi->value->int32_list[0] == 3 &&
     oi->value->int32_list[1] == 58293 &&
     oi->value->int32_list[2] == 293 &&
     oi->value->int32_list[3] == 58293 &&
     oi->value_type == oyVAL_INT_LIST)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_SetFromInt() modify int32_t list good  " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_SetFromInt() modify int32_t list failed" );
  }

  error = oyOption_SetFromInt( o, 58293, 0, 0 );
  if(!error && oi->value &&
     oi->value->int32_list[0] == 3 &&
     oi->value->int32_list[1] == 58293 &&
     oi->value->int32_list[2] == 293 &&
     oi->value->int32_list[3] == 58293 &&
     oi->value_type == oyVAL_INT_LIST)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_SetFromInt() set int32_t list good     " );
    erg[0] = oyOption_GetValueInt( o, 0 );
    erg[1] = oyOption_GetValueInt( o, 1 );
    erg[2] = oyOption_GetValueInt( o, 2 );
    erg[3] = oyOption_GetValueInt( o, 3 );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_SetFromInt() set int32_t list failed   " );
  }

  if(!error && erg[0] == 58293 && erg[1] == 293 && erg[2] == 58293 &&
               erg[3] == 0)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_GetValueInt() good                     " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_GetValueInt() failed                   " );
  }

  oyOption_Release( &o );


  o = oyOption_FromRegistration( "//" OY_TYPE_STD "/filter/y", 0 );
  oi = (oyOption_s_*) o;
  /*o->value_type = oyVAL_INT_LIST;*/
  error = oyOption_SetFromInt( o, 58293, 2, 0 );
  if(!error && oi->value &&
     oi->value->int32_list[0] == 3 &&
     oi->value->int32_list[1] == 0 &&
     oi->value->int32_list[2] == 0 &&
     oi->value->int32_list[3] == 58293 &&
     oi->value_type == oyVAL_INT_LIST)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOption_SetFromInt() explicite int32_t list good" );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOption_SetFromInt() explicite int32_t list failed" );
  }

  oyOption_Release( &o );

  return result;
}

oyTESTRESULT_e testOptionsSet ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  int error = 0;
  oyOptions_s * setA = 0;
  const char * t = NULL;

  fprintf(stdout, "\n" );

  error = oyOptions_SetFromText( &setA,
                                 "org/test/" OY_TYPE_STD "/filter/gamma_A",
                                 "1", OY_CREATE_NEW );
  error = oyOptions_SetFromText( &setA,
                                 "org/test/" OY_TYPE_STD "/filter/gamma_A1",
                                 "1", OY_CREATE_NEW );
  error = oyOptions_SetFromText( &setA,
                                 "org/test/" OY_TYPE_STD "/filter/gamma_A12",
                                 "1", OY_CREATE_NEW );

  if(!error && oyOptions_Count( setA ) == 3)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOptions_SetFromText() similiar registration good  " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOptions_SetFromText() similiar registration failed" );
  }

  error = oyOptions_SetFromText( &setA,
                                 "org/oyranos/" OY_TYPE_STD "/filter/gamma_A2",
                                 "one\ntwo\nthree\nfour",
                                 OY_CREATE_NEW | OY_STRING_LIST);
  t = oyOptions_GetText( setA, oyNAME_NAME );
  if(t && t[0] && oyOptions_Count( setA ) == 4)
  {
    oyOption_s * opt;
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOptions_GetText()                             good" );
    fprintf( zout, "%s\n", t );
    opt = oyOptions_Get( setA, 3 );
    fprintf( zout, "fourth option\n" );
    fprintf( zout, "ValueText: %s\n", oyOption_GetValueText(opt, malloc ) );
    fprintf( zout, "NICK: %s\n", oyOption_GetText(opt, oyNAME_NICK) );
    fprintf( zout, "NAME: %s\n", oyOption_GetText(opt, oyNAME_NAME) );
    fprintf( zout, "DESCRIPTION: %s\n", oyOption_GetText(opt, oyNAME_DESCRIPTION) );
    
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOptions_GetText()                           failed" );
  }

  oyOptions_Release( &setA );

  return result;
}

oyTESTRESULT_e testOptionsCopy ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  int error = 0;
  oyOptions_s * setA = 0, * setB = 0, * setC = 0,
              * resultA = 0, * resultB = 0;
  int32_t count = 0;

  fprintf(stdout, "\n" );

  error = oyOptions_SetFromText( &setA,
                OY_INTERNAL "/lcm2.color.icc/rendering_bpc.advanced",
                                 "1", OY_CREATE_NEW );
  error = oyOptions_SetFromText( &setA,
                                 "//" OY_TYPE_STD "/image/A", "true",
                                 OY_CREATE_NEW );
  error = oyOptions_SetFromText( &setA,
                                 "//" OY_TYPE_STD "/image/A", "true",
                                 OY_CREATE_NEW );

  if(!error && oyOptions_Count( setA ) == 2)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOptions_SetFromText() good                    " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOptions_SetFromText() failed                  " );
  }

  error = oyOptions_SetFromText( &setB,
                                 "//" OY_TYPE_STD "/config/A", "true",
                                 OY_CREATE_NEW );
  error = oyOptions_SetFromText( &setB,
                                 "//" OY_TYPE_STD "/config/B", "true",
                                 OY_CREATE_NEW );
  error = oyOptions_SetFromText( &setB,
                                 "//" OY_TYPE_STD "/config/C", "true",
                                 OY_CREATE_NEW );

  error = oyOptions_SetFromText( &setC,
                                 "//" OY_TYPE_STD "/config/B", "true",
                                 OY_CREATE_NEW );
  error = oyOptions_SetFromText( &setC,
                                 "//" OY_TYPE_STD "/config/D", "true",
                                 OY_CREATE_NEW );
  error = oyOptions_SetFromText( &setC,
                                 "//" OY_TYPE_STD "/config/C", "true",
                                 OY_CREATE_NEW );

  error = oyOptions_CopyFrom( &resultA, setA, oyBOOLEAN_UNION,
                              oyFILTER_REG_NONE,0 );

  if(!error && oyOptions_Count( resultA ) == 2 &&
     oyOptions_FindString( resultA, "rendering_bpc", 0 ) &&
     oyOptions_FindString( resultA, "A", 0 ))
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOptions_CopyFrom() oyBOOLEAN_UNION good       " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOptions_CopyFrom() oyBOOLEAN_UNION failed     " );
  }

  error = oyOptions_CopyFrom( &resultA, setB, oyBOOLEAN_DIFFERENZ,
                              oyFILTER_REG_OPTION,0 );

  if(!error && oyOptions_Count( resultA ) == 3 &&
     oyOptions_FindString( resultA, "rendering_bpc", 0 ) &&
     oyOptions_FindString( resultA, "B", 0 ) &&
     oyOptions_FindString( resultA, "C", 0 ))
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOptions_CopyFrom() oyBOOLEAN_DIFFERENZ good   " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOptions_CopyFrom() oyBOOLEAN_DIFFERENZ failed " );
  }


  error = oyOptions_Filter( &resultB, &count, 0, oyBOOLEAN_INTERSECTION,
                  OY_INTERNAL "/image", setA  );

  if(!error && oyOptions_Count( resultB ) == 1 &&
     oyOptions_FindString( resultB, "A", 0 ))
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOptions_Filter() oyBOOLEAN_INTERSECTION good  " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOptions_Filter() oyBOOLEAN_INTERSECTION failed" );
  }

  oyOptions_Release( &setA );
  oyOptions_Release( &setB );
  oyOptions_Release( &setC );
  oyOptions_Release( &resultA );
  oyOptions_Release( &resultB );

  return result;
}

#include "oyBlob_s.h"

oyTESTRESULT_e testBlob ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  int error = 0;
  oyBlob_s * a = 0, * b = 0;
  oyPointer ptr = 0;
  const char static_ptr[16] = {0,1,0,1,0,1,0,1,  0,1,0,1,0,1,0,1};
  const char type[8] = "test";
  oyObject_s object = oyObject_New();

  fprintf(stdout, "\n" );

  a = oyBlob_New( 0 );
  ptr = (oyPointer) static_ptr;
  error = oyBlob_SetFromStatic( a, ptr, 16, type );

  if(!error)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyBlob_SetFromSatic() good                      " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyBlob_SetFromSatic() failed                    " );
  }

  b = oyBlob_Copy( a, object );

  if(!error && b && oyBlob_GetPointer(b) && oyBlob_GetSize(b) &&
     oyBlob_GetPointer( b ) == static_ptr)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyBlob_Copy( static ) good                      " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyBlob_Copy( static ) failed                    " );
  }

  error = oyBlob_Release( &b );

  error = oyBlob_SetFromStatic( a, ptr, 0, type );

  if(!error)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyBlob_SetFromSatic(0) good                     " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyBlob_SetFromSatic(0) failed                   " );
  }

  b = oyBlob_Copy( a, object );

  if(!error && b && oyBlob_GetPointer(b) && !oyBlob_GetSize(b) &&
     oyBlob_GetPointer(b) == static_ptr)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyBlob_Copy( static 0) good                     " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyBlob_Copy( static 0) failed                   " );
  }

  ptr = malloc(1024);
  error = oyBlob_SetFromData( a, ptr, 1024, type );

  if(!error && oyBlob_GetPointer(a) && oyBlob_GetSize(a) == 1024 &&
     oyBlob_GetPointer(a) != ptr)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyBlob_SetFromData() good                       " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyBlob_SetFromData() failed                     " );
  }
  
  error = oyBlob_Release( &b );

  if(!error && !b)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyBlob_Release() good                           " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyBlob_Release() failed                         " );
  }

  b = oyBlob_Copy( a, object );

  if(!error && b && a != b && oyBlob_GetPointer(b) &&
     oyBlob_GetSize(a) == oyBlob_GetSize(b) &&
     oyBlob_GetPointer(a) != oyBlob_GetPointer(b) )
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyBlob_Copy() good                              " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyBlob_Copy() failed                            " );
  }

  oyBlob_Release( &a );
  oyBlob_Release( &b );
  free(ptr);

  return result;
}

#include <libxml/parser.h>
#include <libxml/xmlsave.h>

oyTESTRESULT_e testSettings ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  int i;
  uint32_t size = 0;
  int count = 0,
      countB = 0;
  char * text = 0, * tmp = 0;
  int error = 0;
  oyOptions_s * opts = 0;
  oyOption_s * o;
  xmlDocPtr doc = 0;

  oyExportReset_(EXPORT_SETTING);

  fprintf(zout, "\n" );

  /* we check for out standard CMM */
  opts = oyOptions_ForFilter( "//" OY_TYPE_STD, "lcm2",
                                            oyOPTIONATTRIBUTE_ADVANCED /* |
                                            oyOPTIONATTRIBUTE_FRONT |
                                            OY_SELECT_COMMON */, 0 );

  oyOptions_SetFromText( &opts, "cmyk_cmyk_black_preservation", "1", 0 );

  count = oyOptions_Count( opts );
  if(!count)
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "No options found for lcm2" );
  } else
  {
    for(i = 0; i < count; ++i)
    {
      o = oyOptions_Get( opts, i );
      tmp = oyOption_GetValueText( o, 0 );
      fprintf(zout, "%s:", tmp );
      oyDeAllocateFunc_(tmp);

      tmp = oyFilterRegistrationToText( oyOption_GetText(o, oyNAME_DESCRIPTION),
                                        oyFILTER_REG_OPTION, 0 );
      if(strcmp( oyNoEmptyString_m_(tmp),"cmyk_cmyk_black_preservation") == 0)
        ++size;

      oyDeAllocateFunc_(tmp);
      oyOption_Release( &o );
    }

    if(!size)
    {
      PRINT_SUB( oyTESTRESULT_FAIL, 
      "oyOptions_FindString() could not find option" );
    } else
    if(size > 1)
    {
      PRINT_SUB( oyTESTRESULT_FAIL, 
      "oyOptions_FindString() returned doubled options %d",
                       size );
    } else
      PRINT_SUB( oyTESTRESULT_SUCCESS, 
      "oyOptions_FindString() returned one option" );
  }

  /*ptr = xmlSaveToBuffer( buf, 0, 0 );*/


  text = oyStringCopy_(oyOptions_GetText( opts, oyNAME_NAME ), oyAllocateFunc_);

  {
    if(!text || !strlen(text))
    {
      PRINT_SUB( oyTESTRESULT_FAIL, 
      "oyOptions_GetText() returned no text             " );
    } else
    {
      PRINT_SUB( oyTESTRESULT_SUCCESS, 
      "oyOptions_GetText() returned text               %d", (int)strlen(text) );
    }
  }
  

  oyOptions_Release( &opts );

  /* Roundtrip test */
  opts = oyOptions_FromText( text, 0, 0 );
  countB = oyOptions_Count( opts );

  {
    if(count == countB)
    {
      PRINT_SUB( oyTESTRESULT_SUCCESS, 
      "obtained same count from oyOptions_FromText %d|%d  ",
                      count, countB );
    } else
    {
      PRINT_SUB( oyTESTRESULT_FAIL, 
      "oyOptions_FromText() returned different count %d|%d",
                      count, countB );
    }
  }
  
  for( i = 0; i < countB; ++i)
  {
    char * t;
    o = oyOptions_Get( opts, i );
    t = oyStringCopy_(oyOption_GetText(o, oyNAME_DESCRIPTION), oyAllocateFunc_);
    fprintf(zout,"%d: \"%s\": \"%s\" %s %d\n", i, 
           t, oyOption_GetValueText( o, malloc ),
           oyFilterRegistrationToText( oyOption_GetText( o, oyNAME_DESCRIPTION),
                                       oyFILTER_REG_OPTION, 0 ),
           ((oyOption_s_*)o)->flags );

    oyOption_Release( &o );
  }

  text = oyStringAppend_( "<a>\n", text, 0 );
  oyStringAdd_( &text, "</a>", 0, 0 );

  doc = xmlParseMemory( text, oyStrlen_( text ) );
  error = !doc;
  {
    if(error)
    {
      PRINT_SUB( oyTESTRESULT_FAIL, 
      "libxml2::xmlParseMemory() returned could not parse the document" );
    } else
    {
      PRINT_SUB( oyTESTRESULT_SUCCESS, 
      "libxml2 returned document                        " );
    }
  }

  xmlDocDumpFormatMemory( doc, (xmlChar**)&text, &i, 1 );
  /*xmlSaveDoc( ptr, doc );*/

  oyOptions_Release( &opts );

  opts = oyOptions_ForFilter( "//" OY_TYPE_STD, "lcm2",
                                            oyOPTIONATTRIBUTE_ADVANCED  |
                                            oyOPTIONATTRIBUTE_FRONT |
                                            OY_SELECT_COMMON, 0 );
  fprintf(zout,"Show advanced common front end options:\n");
  countB = oyOptions_Count( opts );
  for( i = 0; i < countB; ++i)
  {
    char * t;
    o = oyOptions_Get( opts, i );
    t = oyStringCopy_(oyOption_GetText(o, oyNAME_DESCRIPTION), oyAllocateFunc_);
    fprintf(zout,"%d: \"%s\": \"%s\" %s %d\n", i, 
           t, oyOption_GetValueText( o, malloc ),
           oyFilterRegistrationToText( oyOption_GetText( o, oyNAME_DESCRIPTION),
                                       oyFILTER_REG_OPTION, 0 ),
           ((oyOption_s_*)o)->flags );

    oyOption_Release( &o );
  }
  oyOptions_Release( &opts );


  return result;
}

#include "oyranos_helper.h"
oyTESTRESULT_e testInterpolation ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  uint16_t ramp[4] = { 0, 21845, 43690, 65535 };
  float pol;

  fprintf(stdout, "\n" );

  pol = oyLinInterpolateRampU16( ramp, 4, 0.499999 );
  if(pol < ramp[3]/2.0)
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS,
    "Interpolation is fine %f                     ", pol );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "Interpolation failed: %f < %f                          ", pol, ramp[3]/2.0 );
  }

  pol = oyLinInterpolateRampU16( ramp, 4, 0.5 );
  if(pol == ramp[3]/2.0)
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS,
    "Interpolation is fine.                                 " );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "Interpolation failed: %f == %f                         ", pol, ramp[3]/2.0 );
  }

  pol = oyLinInterpolateRampU16( ramp, 4, 2.0/3.0 );
  if(pol == ramp[2])
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS,
    "Interpolation is fine.                                 " );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "Interpolation failed: %f == %u                         ", pol, ramp[2] );
  }

  pol = oyLinInterpolateRampU16( ramp, 4, -1 );
  if(pol == 0.0)
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS,
    "Interpolation is fine.                                 " );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "Interpolation failed: %f == 0.0                        ", pol );
  }

  pol = oyLinInterpolateRampU16( ramp, 4, 1.1 );
  if(pol == ramp[3])
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS,
    "Interpolation is fine.                                 " );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "Interpolation failed: %f == %u                         ", pol, ramp[3] );
  }
  return result;
}

#include "oyProfile_s.h"

oyTESTRESULT_e testProfile ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  size_t size = 0;
  oyPointer data;
  oyProfile_s * p_a,
              * p_b;

  oyExportReset_(EXPORT_SETTING);

  fprintf(stdout, "\n" );

  p_a = oyProfile_FromStd ( oyASSUMED_WEB, 0, NULL );
  if(!p_a)
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "No assumed WEB profile found                           " );
  } else
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "found oyASSUMED_WEB                                    " );
  }
  
  /* compare the usual conversion profiles with the total of profiles */
  data = oyProfile_GetMem( p_a, &size, 0, malloc );
  if(!data || ! size)
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "Could get memory from profile                          " );
  } else
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "Obtained memory block from oyASSUMED_WEB:        %d    ", (int)size );
  }

  p_b = oyProfile_FromMem( size, data, 0,0 );
  if(!p_b)
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "Could not load profile from memory.                    " );
  } else
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "Loaded profile from memory of oyASSUMED_WEB            " );
  }

  if(!oyProfile_Equal( p_a, p_b ))
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "Reload of oyASSUMED_WEB failed. Unexplained difference." );
  } else
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyASSUMED_WEB is equal to memory loaded oyProfile_s.   " );
  }

  oyOption_s * matrix = oyOption_FromRegistration("///color_matrix."
              "from_primaries."
              "redx_redy_greenx_greeny_bluex_bluey_whitex_whitey_gamma", NULL );
  {
    /* http://www.color.org/chardata/rgb/rommrgb.xalter
     * original gamma is 1.8, we adapt to typical cameraRAW gamma of 1.0 */
      oyOption_SetFromDouble( matrix, 0.7347, 0, 0);
      oyOption_SetFromDouble( matrix, 0.2653, 1, 0);
      oyOption_SetFromDouble( matrix, 0.1596, 2, 0);
      oyOption_SetFromDouble( matrix, 0.8404, 3, 0);
      oyOption_SetFromDouble( matrix, 0.0366, 4, 0);
      oyOption_SetFromDouble( matrix, 0.0001, 5, 0);
      oyOption_SetFromDouble( matrix, 0.3457, 6, 0);
      oyOption_SetFromDouble( matrix, 0.3585, 7, 0);
  }
  oyOption_SetFromDouble( matrix, 1.8, 8, 0);

  oyOptions_s * opts = oyOptions_New(0),
              * r = 0;

  //oyOptions_SetFromInt( &opts, "///icc_profile_flags", icc_profile_flags, 0, OY_CREATE_NEW );
  oyOptions_MoveIn( opts, &matrix, -1 );
  const char * reg = "//"OY_TYPE_STD"/create_profile.color_matrix.icc";
  oyOptions_Handle( reg, opts, "create_profile.icc_profile.color_matrix",
                    &r );

  oyProfile_s * p = NULL;
  p = (oyProfile_s*)oyOptions_GetType( r, -1, "icc_profile",
                                               oyOBJECT_PROFILE_S );
  oyOptions_Release( &r );

# define ICC_TEST_NAME "TEST ROMM gamma 1.0"
  oyProfile_AddTagText( p, icSigProfileDescriptionTag,
                           ICC_TEST_NAME );

  oyOptions_Release( &opts );

  if(!p )
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyOptions_Handle( \"create_profile.icc_profile.color_matrix\") failed" );
  } else
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyOptions_Handle( \"create_profile.icc_profile.color_matrix\")       " );
  }

  FILE * fp = fopen( ICC_TEST_NAME".icc", "r" );
  if(fp)
  {
    fclose(fp); fp = 0;
    remove( ICC_TEST_NAME".icc" );
  }
  
  data = oyProfile_GetMem( p, &size, 0, malloc );
  int error = oyWriteMemToFile_( ICC_TEST_NAME".icc", data, size );
  if(!error )
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyWriteMemToFile_( \"%s\")          ", ICC_TEST_NAME".icc" );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL,
    "oyWriteMemToFile_( \"%s\")  failed  ", ICC_TEST_NAME".icc" );
  }

  if(size >= 128 &&
     oyCheckProfileMem( data, 128, 0 ) == 0)
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyCheckProfileMem( \"%d\")          ", (int)size );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL,
    "oyCheckProfileMem( \"%d\")          ", (int)size );
  }

  oyProfile_Release( &p );

  p = oyProfile_FromFile( ICC_TEST_NAME".icc", OY_SKIP_NON_DEFAULT_PATH, NULL );
  if(!p )
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyProfile_FromFile( OY_SKIP_NON_DEFAULT_PATH )       " );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL,
    "oyProfile_FromFile( OY_SKIP_NON_DEFAULT_PATH ) failed" );
  }

  

  return result;
}

#include "oyProfiles_s.h"

oyTESTRESULT_e testProfiles ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  int i;
  uint32_t size = 0;
  int current = -1;
  int count = 0,
      countB = 0;
  char ** texts = 0;
  const char * tmp = 0;
  oyProfiles_s * profs = 0;
  oyProfile_s * p;

  oyExportReset_(EXPORT_SETTING);

  fprintf(stdout, "\n" );

  /* compare the usual conversion profiles with the total of profiles */
  profs = oyProfiles_ForStd( oyDEFAULT_PROFILE_START, 0, &current, 0 );
  count = oyProfiles_Count( profs );
  if(!count)
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "No profiles found for oyDEFAULT_PROFILE_START" );
  } else
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "profiles found for oyDEFAULT_PROFILE_START: %d (%d)", count, current );
  }
  oyProfiles_Release( &profs );

  texts = oyProfileListGet( 0, &size, 0 );
  if(!size)
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "No profiles found for oyProfileListGet()" );
  } else
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "profiles found for oyProfileListGet:        %d", size );
  }
  oyStringListRelease_( &texts, size, oyDeAllocateFunc_ );

  if((int)size < count)
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyProfileListGet() returned less than oyDEFAULT_PROFILE_START %d|%d", size, count );
  } else if(count)
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyProfileListGet and oyDEFAULT_PROFILE_START ok %d|%d", size, count );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL,
    "oyProfileListGet and/or oyDEFAULT_PROFILE_START are wrong %d|%d", size, count );
  }

  /* compare the default profile spaces with the total of profiles */
  countB = 0;
  /* thread testing needs initialisation through oyThreadLockingSet()
#pragma omp parallel for private(current,count,p,tmp,profs) */
  for(i = oyEDITING_XYZ; i <= oyEDITING_GRAY; ++i)
  {
    profs = oyProfiles_ForStd( (oyPROFILE_e)i, 0, &current, 0 );

    count = oyProfiles_Count( profs );
    countB += count;
    if(!count)
    {
      PRINT_SUB( oyTESTRESULT_FAIL, 
      "No profiles found for oyPROFILE_e %d             ", i );
    } else
    {
      p = oyProfiles_Get( profs, current );
      tmp = oyProfile_GetText( p, oyNAME_DESCRIPTION );
      PRINT_SUB( oyTESTRESULT_SUCCESS, 
      "profiles found for oyPROFILE_e %d: %d \"%s\"", i, count, tmp ? tmp :"");
    }

    oyProfiles_Release( &profs );
  }
  if((int)size < countB)
  {
    PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyProfileListGet() returned less than oyPROFILE_e %d|%d", size, count );
  } else
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyProfileListGet and oyPROFILE_e ok %d|%d", size, countB );
  }


  return result;
}


oyTESTRESULT_e testProfileLists ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  double start_time = DBG_UHR_;

#ifdef __cplusplus
  std::cout << "Start: " << start_time << std::endl;
#else
  fprintf(zout, "Start %.3f\n", start_time );
#endif

  uint32_t ref_count = 0;
  char ** reference = oyProfileListGet(0, &ref_count, myAllocFunc);

  if((int)ref_count)
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyProfileListGet() returned profiles %d", (int)ref_count );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL,
    "oyProfileListGet() zero" );
  }

  int i,j;
  for(i = 0; i < 1000; ++i)
  {
    uint32_t count = 0;
    char ** names = oyProfileListGet(0, &count, myAllocFunc);
    if(count != ref_count)
    {
      PRINT_SUB( oyTESTRESULT_FAIL, 
      ": wrong profile count: %d/%d", count, ref_count );
    }
    for(j = 0; j < (int)count; ++j)
    {
      if(!(names[j] && strlen(names[j])) ||
         strcmp( names[j], reference[j] ) != 0 )
      {
        PRINT_SUB( oyTESTRESULT_FAIL, 
        "\n no profile name found: run %d profile #%d", i , j );
      }
      if( names[j] )
#ifdef __cplusplus
        delete [] names[j];
#else
        free(names[j]);
#endif
    }
#ifdef __cplusplus
    if( names ) delete [] names;
    std::cout << "." << std::flush;
#else
    if( names ) free( names );
    fprintf(zout, "." ); fflush(zout);
#endif

  }

  double end = DBG_UHR_;

#ifdef __cplusplus
  std::cout << std::endl;
  std::cout << "1000 + 1 calls to oyProfileListGet() took: "<< end - start_time
            << " seconds" << std::endl;
#else
  fprintf(zout, "\n1000 + 1 calls to oyProfileListGet() took: %.03f seconds\n",
                  end - start_time );
#endif

  return result;
}

#include "oyProfile_s_.h"           /* oyProfile_ToFile_ */

oyTESTRESULT_e testProofingEffect ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;
  oyOptions_s * opts = oyOptions_New(0),
              * result_opts = 0;
  const char * text = 0;
  oyProfile_s * prof = oyProfile_FromStd( oyEDITING_CMYK, 0, NULL ),
              * abstract;
  int error;

  fprintf(stdout, "\n" );

  error = oyOptions_MoveInStruct( &opts, "//"OY_TYPE_STD"/icc_profile.proofing_profile",
                                  (oyStruct_s**) &prof, OY_CREATE_NEW );
  error = oyOptions_Handle( "//"OY_TYPE_STD"/create_profile.proofing_effect",
                            opts,"create_profile.proofing_effect",
                            &result_opts );
  abstract = (oyProfile_s*)oyOptions_GetType( result_opts, -1, "icc_profile",
                                              oyOBJECT_PROFILE_S );
  oyOptions_Release( &result_opts );
  oyOptions_Release( &opts );

  text = oyProfile_GetText( abstract, oyNAME_DESCRIPTION );

  if(abstract)
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyOptions_Handle(\"create_profile\"): %s", text );
  } else if(error == -1)
  {
    PRINT_SUB( oyTESTRESULT_FAIL,
    "oyOptions_Handle(\"create_profile\") no" );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL,
    "oyOptions_Handle(\"create_profile\") zero" );
  }

  oyProfile_ToFile_( (oyProfile_s_*)abstract, "test_proof_effect.icc" );
  oyProfile_Release( &abstract );

  return result;
}

#include "oyImage_s.h"
#include "oyConversion_s.h"

oyTESTRESULT_e testDeviceLinkProfile ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  double buf[24];
  uint32_t icc_profile_flags =oyICCProfileSelectionFlagsFromOptions( OY_CMM_STD,
                                       "//" OY_TYPE_STD "/icc_color", NULL, 0 );
  oyProfile_s * prof = oyProfile_FromStd( oyASSUMED_WEB, icc_profile_flags, 0 ), *dl = 0;
  oyImage_s * in = oyImage_Create( 2, 2, buf, OY_TYPE_123_DBL, prof, 0 );
  oyImage_s * out = oyImage_CreateForDisplay( 2, 2, buf, OY_TYPE_123_DBL, 0,
                                              0,0, 12,12,
                                              icc_profile_flags, 0 );
  oyOptions_s * options = NULL;
  oyOptions_SetFromText( &options, OY_CMM_STD"/context", "lcm2", OY_CREATE_NEW );
  oyConversion_s *cc = oyConversion_CreateBasicPixels( in, out, options, 0 );
  oyFilterGraph_s * graph = NULL;
  oyFilterNode_s * node = NULL;
  oyBlob_s * blob = NULL;
  int error = 0;
  const char * fn = NULL,
             * prof_fn = oyProfile_GetFileName( prof, -1 );
  int i,n=0, len;

  fprintf(stdout, "\n" );

  fprintf(stdout, "creating DL from sRGB to CIE*XYZ\n" );

  memset( buf, 0, sizeof(double)*24);

  /*oyConversion_RunPixels( cc, 0 );*/

  if(cc)
    graph = oyConversion_GetGraph( cc );
  if(graph)
    n = oyFilterGraph_CountEdges( graph );
  for(i = 0; i < n; ++i)
  {
    node = oyFilterGraph_GetNode( graph, i, NULL, NULL );
    blob = oyFilterNode_ToBlob( node, NULL );
    if(blob && oyBlob_GetSize( blob ))
    {
      char name[64];
      sprintf( name, "oy_dl_test_%d_", i );
      len = strlen(name);
      memcpy( &name[len], oyBlob_GetType( blob ), 4 );
      name[len+4] = 0;
      len = strlen(name);
      sprintf( &name[len], ".icc" );
      error = oyWriteMemToFile_( name, oyBlob_GetPointer( blob ),
                                 oyBlob_GetSize( blob) );
      if(!error)
        fprintf(zout,"wrote: %s\n", name );
      else
        fprintf(zout,"writing failed: %s\n", name );
      dl = oyProfile_FromMem( oyBlob_GetSize( blob ),
                              oyBlob_GetPointer( blob ), 0,0 );
    }

    oyBlob_Release( &blob );
    oyFilterNode_Release( &node );
  }

  fn = oyProfile_GetFileName( dl, 0 );
  if(strcmp(fn,prof_fn) == 0)
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyProfile_GetFileName(dl, 0): %s", fn );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL,
    "oyProfile_GetFileName(dl, 0): %s  %s", oyNoEmptyString_m_(fn), prof_fn );
  }

  fn = oyProfile_GetFileName( dl, 1 );
  if(fn)
  {
    PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyProfile_GetFileName(dl, 1): %s", fn );
  } else
  {
    PRINT_SUB( oyTESTRESULT_FAIL,
    "oyProfile_GetFileName(dl, 1): %s", oyNoEmptyString_m_(fn) );
  }

  error = oyConversion_Release( &cc );
  error = oyImage_Release( &in );
  error = oyImage_Release( &out );
  error = oyProfile_Release( &prof );
  error = oyProfile_Release( &dl );


  return result;
}

oyTESTRESULT_e testRegistrationMatch ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  if( oyFilterRegistrationMatch(OY_INTERNAL "/icc_color.lcms",
                                "//" OY_TYPE_STD "/icc_color",
                                oyOBJECT_CMM_API4_S ))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "simple CMM selection                  " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "simple CMM selection                  " );
  }

  if(!oyFilterRegistrationMatch(OY_INTERNAL "/icc_color.lcms",
                                "//" OY_TYPE_STD "/icc_color.octl",
                                oyOBJECT_CMM_API4_S ))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "simple CMM selection no match         " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "simple CMM selection no match         " );
  }

  if( oyFilterRegistrationMatch(OY_INTERNAL "/icc_color.lcms",
                                "//" OY_TYPE_STD "/icc_color.4+lcms",
                                oyOBJECT_CMM_API4_S ))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "special CMM selection                 " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "special CMM selection                 " );
  }

  if(!oyFilterRegistrationMatch(OY_INTERNAL "/icc_color.lcms",
                                "//" OY_TYPE_STD "/icc_color.4-lcms",
                                oyOBJECT_CMM_API4_S ))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "special CMM avoiding                  " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "special CMM avoiding                  " );
  }

  if( oyFilterRegistrationMatch(OY_INTERNAL "/icc_color.lcms",
                                "//" OY_TYPE_STD "/icc_color.7-lcms",
                                oyOBJECT_CMM_API4_S ))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "special CMM avoiding, other API       " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "special CMM avoiding, other API       " );
  }

  return result;
}

extern "C" {
int oyTextIccDictMatch( const char *, const char *, double delta ); }

oyTESTRESULT_e test_oyTextIccDictMatch ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  if( oyTextIccDictMatch("ABC",
                         "ABC", 0))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "simple text matching                  " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "simple text matching                  " );
  }

  if(!oyTextIccDictMatch("ABC",
                         "ABCD", 0))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "simple text mismatching               " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "simple text mismatching               " );
  }

  if( oyTextIccDictMatch("abcd,ABC,efgh",
                         "abcdef,12345,ABC", 0))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "multiple text matching                " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "multiple text matching                " );
  }

  if( oyTextIccDictMatch("abcd,ABC,efgh,12345",
                         "abcdef,12345,ABCD", 0.0005))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "multiple integer matching             " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "multiple integer matching             " );
  }

  if(!oyTextIccDictMatch("abcd,ABC,efgh,12345",
                         "abcdef,12345ABCD", 0.0005))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "multiple integer mismatching          " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "multiple integer mismatching          " );
  }

  if( oyTextIccDictMatch("abcd,ABC,efgh,123.45001",
                         "abcdef,123.45,ABCD", 0.0005))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "multiple float matching               " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "multiple float matching               " );
  }

  if(!oyTextIccDictMatch("abcd,ABC,efgh,123.45",
                         "abcdef,123", 0.0005))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "multiple float mismatching            " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "multiple float mismatching            " );
  }
  return result;
}

oyTESTRESULT_e testPolicy ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;
  char *data = 0;

  fprintf(stdout, "\n" );

#if 0
  if(argc > 1)
  {
    fprintf(zout,"%s\n", argv[1]);
    std::ifstream f( argv[1], std::ios::binary | std::ios::ate);
    if(f.good())
    {
      size_t size = f.tellg();
      f.seekg(0);
      if(size) {
        data = (char*) new char [size+1];
        f.read ((char*)data, size);
        f.close();
        std::cout << "Opened file: " << argv[1] << std::endl;
      }
    }
  }
#endif

  char *xml = data;
  if( !xml)
    xml = oyPolicyToXML( oyGROUP_ALL, 1, myAllocFunc );

  if( xml && xml[0] )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyPolicyToXML                         " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyPolicyToXML                         " );
  }

  if(xml) {
    oyReadXMLPolicy(oyGROUP_ALL, xml);
    fprintf(zout,"xml text: \n%s", xml);

    data = oyPolicyToXML( oyGROUP_ALL, 1, myAllocFunc );

    if( strcmp( data, xml ) == 0 )
    { PRINT_SUB( oyTESTRESULT_SUCCESS,
      "Policy rereading                      " );
    } else
    { PRINT_SUB( oyTESTRESULT_FAIL,
      "Policy rereading                      " );
    }

#ifdef __cplusplus
    delete [] xml;
    delete [] data;
#else
    free(xml);
    free(data);
#endif
  }


  return result;
}

/* forward declaration for oyranos_alpha.c */
#ifdef __cplusplus
extern "C" {
#endif
char ** oyCMMsGetLibNames_           ( uint32_t          * n,
                                       const char        * required_cmm );
#ifdef __cplusplus
}
#endif

#include "oyranos_devices.h"

oyTESTRESULT_e testCMMDevicesListing ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int i, j, k, j_n, k_n;
  uint32_t count = 0,
         * rank_list = 0;
  int error = 0;
  char ** texts = 0,
        * val = 0;

#ifdef USE_GETTEXT
  setlocale(LC_ALL,"");
#endif

  /* get all configuration filters */
  oyConfigDomainList( "//"OY_TYPE_STD"/config.device.icc_profile",
                      &texts, &count, &rank_list ,0 );

  if( count )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConfigDomainList Found CMM's %d     ", (int)count );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConfigDomainList Found CMM's %d     ", (int)count );
  }
  for( i = 0; i < (int)count; ++i)
  {
    fprintf( zout, "%d: %s\n", i, texts[i] );
  }
  fprintf(zout, "\n" );

  oyConfigs_s * configs = 0;
  oyConfig_s * config = 0;
  oyOptions_s * options_list = 0;
  oyOption_s * o = 0;
  oyOption_s_ * oi = 0;
  oyProfile_s * p = 0;

  /* send a empty query to one module to obtain instructions in a message */
  if(count)
  error = oyConfigs_FromDomain( texts[0], 0, &configs, 0 );
  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConfigs_FromDomain \"%s\" help text ", texts ? 
                                              oyNoEmptyString_m_(texts[0]) :"----");
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConfigs_FromDomain \"%s\" help text ", texts[0] );
  }
  fprintf( zout, "\n");


  /* add list call to module arguments */
  error = oyOptions_SetFromText( &options_list,
                                 "//" OY_TYPE_STD "/config/command", "list",
                                 OY_CREATE_NEW );
  error = oyOptions_SetFromText( &options_list,
                                 "//" OY_TYPE_STD "/config/icc_profile",
                                 "true", OY_CREATE_NEW );

  fprintf( zout, "oyConfigs_FromDomain() \"list\" call:\n" );
  for( i = 0; i < (int)count; ++i)
  {
    const char * registration_domain = texts[i];
    fprintf(zout,"%d[rank %d]: %s\n", i, rank_list[i], registration_domain);

    error = oyConfigs_FromDomain( registration_domain,
                                  options_list, &configs, 0 );
    j_n = oyConfigs_Count( configs );
    for( j = 0; j < j_n; ++j )
    {
      oyConfigs_s * dbs = 0,
                  * heap = 0;
      int precise_count = 0,
          serial_count = 0,
          mnft_count = 0,
          dev_name_count = 0;

      config = oyConfigs_Get( configs, j );

      fprintf(zout, "--------------------------------------------------------------------------------\n\"%s\":\n", oyConfig_FindString( config, "device_name", 0 ) );
      {
        oyOptions_s * options = 0;
        const char * t = 0;
        oyOptions_SetFromText( &options,
                   "//"OY_TYPE_STD"/config/icc_profile.x_color_region_target",
                         "yes", OY_CREATE_NEW );
        error = oyDeviceGetProfile( config, options, &p );
        oyOptions_Release( &options );
        t = oyProfile_GetText( p, oyNAME_DESCRIPTION);
        fprintf(zout, "oyDeviceGetProfile(): \"%s\"\n", t ? t : "----" );
        oyProfile_Release( &p );
      }

      error = oyConfigs_FromDB( oyConfig_GetRegistration( config ), &heap, 0 );

      error = oyDeviceSelectSimiliar( config, heap, 0, &dbs );
      precise_count = oyConfigs_Count( dbs );
      oyConfigs_Release( &dbs );

      error = oyDeviceSelectSimiliar( config, heap, 1, &dbs );
      serial_count = oyConfigs_Count( dbs );
      oyConfigs_Release( &dbs );

      error = oyDeviceSelectSimiliar( config, heap, 2, &dbs );
      mnft_count = oyConfigs_Count( dbs );
      oyConfigs_Release( &dbs );

      error = oyDeviceSelectSimiliar( config, heap, 4, &dbs );
      dev_name_count = oyConfigs_Count( dbs );
      oyConfigs_Release( &dbs );

      oyConfigs_Release( &heap );


      fprintf(zout, "\"%s\" has %d precise matches,\n"
              "\t%d manufacturer/model/serial, %d manufacturer/model and\n"
              "\t%d \"device_name\" entries in DB\n",
              oyConfig_FindString( config, "device_name", 0 ),
              precise_count,serial_count,mnft_count,dev_name_count );

      k_n = oyConfig_Count( config );
      for( k = 0; k < k_n; ++k )
      {
        char * r = 0;
        int mnft = -1, mn, pos;

        o = oyConfig_Get( config, k );
        oi = (oyOption_s_*) o;

        r = oyFilterRegistrationToText( oyOption_GetRegistration( o ),
                                        oyFILTER_REG_OPTION, 0 );
        if(r && strcmp(r,"supported_devices_info") == 0 &&
           oi->value_type == oyVAL_STRING_LIST)
        {
          /* print first line special */
          while(oi->value->string_list[++mnft])
          {
            mn = 0; pos = -1;
            while(oi->value->string_list[mnft][++pos])
            {
              if(oi->value->string_list[mnft][pos] == '\n')
              {
                if(mn && oi->value->string_list[mnft][pos+1])
                  putc(',', zout);
                else if(mn == 0)
                {
                  putc(':', zout);
                  putc('\n', zout);
                  putc(' ', zout);
                  putc(' ', zout);
                }
                ++mn;

              } else
                putc(oi->value->string_list[mnft][pos], zout);
            }
            putc('\n', zout);
          }
        } else
        {
          val = oyOption_GetValueText( o, oyAllocateFunc_ );
          fprintf(zout,"  %d::%d::%d \"%s\": \"%s\"\n", i,j,k,
                 oyOption_GetRegistration(o), val?val:"(nix)" );
        }

        if(r) oyDeAllocateFunc_(r); r = 0;
        if(val)
          oyDeAllocateFunc_( val ); val = 0;
        oyOption_Release( &o ); oi = 0;
      }

      oyConfig_Release( &config );
    }

    oyConfigs_Release( &configs );
  }
  fprintf( zout, "\n");
  oyOptions_Release( &options_list );

  fprintf( zout, "\n");

  return result;
}

oyTESTRESULT_e testCMMDevicesDetails ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int i, k, l, k_n;
  uint32_t count = 0,
         * rank_list = 0;
  int error = 0;
  char ** texts = 0,
        * val = 0;

  oyConfigs_s * configs = 0;
  oyConfig_s * config = 0;
  oyOptions_s * options = 0;
  oyOption_s * o = 0;
  int devices_n = 0;

#ifdef USE_GETTEXT
  setlocale(LC_ALL,"");
#endif

  /* get all configuration filters */
  oyConfigDomainList( "//"OY_TYPE_STD"/config.device.icc_profile",
                      &texts, &count, &rank_list ,0 );

  if( count )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConfigDomainList Found CMM's %d     ", (int)count );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConfigDomainList Found CMM's %d     ", (int)count );
  }


  fprintf( zout, "oyConfigs_FromDomain() \"properties\" call:\n" );
  for( i = 0; i < (int)count; ++i)
  {
    const char * registration_domain = texts[i];
    fprintf(zout,"%d[rank %d]: %s\n", i, rank_list[i], registration_domain);

    /* set a general request */
    error = oyOptions_SetFromText( &options,
                                     "//" OY_TYPE_STD "/config/command",
                                     "properties", OY_CREATE_NEW );
    /* send the query to a module */
    error = oyConfigs_FromDomain( registration_domain,
                                  options, &configs, 0 );
    devices_n = oyConfigs_Count( configs );
    for( l = 0; l < devices_n; ++l )
    {
      /* display results */
      fprintf(zout, "--------------------------------------------------------------------------------\n%s:\n", registration_domain );
      config = oyConfigs_Get( configs, l );

      k_n = oyConfig_Count( config );
      for( k = 0; k < k_n; ++k )
      {
        o = oyConfig_Get( config, k );

        val = oyOption_GetValueText( o, oyAllocateFunc_ );
        fprintf(zout, "  %d::%d %s: \"%s\"\n", l,k, 
                  oyStrrchr_(oyOption_GetRegistration(o),'/')+1, val );

        if(val) oyDeAllocateFunc_( val ); val = 0;
        oyOption_Release( &o );
      }

      o = oyConfig_Find( config, "icc_profile" );
      if(o)
      {
        val = oyOption_GetValueText( o, oyAllocateFunc_ );
        fprintf(zout, "  %d %s: \"%s\"\n", l, 
                oyStrrchr_(oyOption_GetRegistration(o),'/')+1, val );

        if(val) oyDeAllocateFunc_( val ); val = 0;
        oyOption_Release( &o );
      }

        //error = oyConfig_SaveToDB( config );
      oyConfig_Release( &config );
    }

    oyConfigs_Release( &configs );
    oyOptions_Release( &options );
  }

  fprintf( zout, "\n");


  if(texts && texts[0])
    config = oyConfig_FromRegistration( texts[0], 0 );
  error = oyConfig_AddDBData( config, "k1", "bla1", OY_CREATE_NEW );
  error = oyConfig_AddDBData( config, "k2", "bla2", OY_CREATE_NEW );
  error = oyConfig_AddDBData( config, "k3", "bla3", OY_CREATE_NEW );

  if( !error  && config && oyOptions_Count(*oyConfig_GetOptions( config,"db") ))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConfig_AddDBData                    " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConfig_AddDBData                    " );
  }

  char * registration = 0;
  if(config)
    registration = oyStringCopy_( oyConfig_GetRegistration( config ),
                                  oyAllocateFunc_ );
  error = oyConfig_SaveToDB( config );

  error = oyConfigs_FromDB( registration, &configs, 0 );
  count = oyConfigs_Count( configs );
  oyConfigs_Release( &configs );

  if( count > 0 )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConfigs_FromDB() %d                  ", (int)count );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConfigs_FromDB() failed %d           ", (int)count );
  }


  int32_t rank = 0;
  error = oyConfig_GetDB( config, &rank );
  const char * key_set_name = oyConfig_FindString( config, "key_set_name", 0 );
  char * key = 0;
  STRING_ADD( key, key_set_name );
  STRING_ADD( key, OY_SLASH );
  STRING_ADD( key, "k1" );
  o = oyConfig_Get( config, 0 );
  oyOption_SetRegistration( o, key );
  oyOption_Release( &o );
  if(key)
    oyDeAllocateFunc_( key ); key = 0;
  error = oyConfig_EraseFromDB( config );
  /* The following is equal to oyConfig_EraseFromDB() but more simple.
  error = oyRegistrationEraseFromDB( key_set_name );
   */
  oyConfig_Release( &config );

  error = oyConfigs_FromDB( registration, &configs, 0 );
  i = oyConfigs_Count( configs );
  oyConfigs_Release( &configs );

  if( count - i == 1 )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConfig_EraseFromDB() %d/%d            ", (int)count,i );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConfig_EraseFromDB() failed %d/%d     ", (int)count,i );
  }

  if(registration)
    oyDeAllocateFunc_( registration ); registration = 0;

  fprintf( zout, "\n");

  return result;
}

oyTESTRESULT_e testCMMRankMap ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

#ifdef USE_GETTEXT
  setlocale(LC_ALL,"");
#endif

  int error = 0;
  oyConfig_s * device;
  oyOptions_s * options = 0;
  oyConfigs_s * devices = 0;

  oyOptions_SetFromText( &options,
                   "//"OY_TYPE_STD"/config/icc_profile.x_color_region_target",
                         "yes", OY_CREATE_NEW );
  error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/command",
                                   "properties", OY_CREATE_NEW );  
  error = oyDevicesGet( OY_TYPE_STD, "monitor", options, &devices );
  oyOptions_Release( &options );

  int count = oyConfigs_Count( devices ),
      i;

  if( count )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "monitor(s) found               %d     ", (int)count );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "no monitor found               %d     ", (int)count );
  }

  for(i = 0; i < count; ++i)
  {
    char * json_text = 0;
    device = oyConfigs_Get( devices, i );
    oyDeviceToJSON( device, 0, &json_text, malloc );

    if( strlen(json_text) )
    { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "json from device [%d]         %d    ", i, (int)strlen(json_text) );
    } else
    { PRINT_SUB( oyTESTRESULT_FAIL,
    "json from device failed for [%d]      ", i );
    }

    const oyRankMap * map = oyConfig_GetRankMap( device );
    if( map )
    { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "Map from device  [%d]                 ", i );
    } else
    { PRINT_SUB( oyTESTRESULT_FAIL,
    "Map from device failed for [%d]       ", i );
    }

    char * rank_map_text = 0;
    error = oyRankMapToJSON( map, options, &rank_map_text, malloc );
    if( rank_map_text )
    { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "JSON from Map    [%d]          %lu    ", i, (long unsigned int)strlen(rank_map_text) );
    } else
    { PRINT_SUB( oyTESTRESULT_FAIL,
    "JSON from Map failed  [%d]            ", i );
    }

    oyRankMap * rank_map2 = 0;
    error = oyRankMapFromJSON( rank_map_text, options, &rank_map2, malloc );
    if( rank_map2 )
    { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "Map from JSON    [%d]                 ", i );
    } else
    { PRINT_SUB( oyTESTRESULT_FAIL,
    "Map from JSON failed [%d]             ", i );
    }

    oyConfig_Release( &device );


    oyDeviceFromJSON( json_text, 0, &device );
    if(json_text) free( json_text ); json_text = 0;
    oyDeviceToJSON( device, 0, &json_text, malloc );

    if( strlen(json_text) )
    { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "json from device [%d]         %d    ", i, (int)strlen(json_text) );
    } else
    { PRINT_SUB( oyTESTRESULT_FAIL,
    "json from device failed for %d        ", i );
    }

    oyConfig_Release( &device );
    fprintf( zout, "\n");
  }

  fprintf( zout, "\n");

  return result;
}

oyTESTRESULT_e testCMMMonitorJSON ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int i;
  int error = 0;
  double clck = 0;

#ifdef USE_GETTEXT
  setlocale(LC_ALL,"");
#endif

  oyConfigs_s * configs = 0;
  oyConfig_s * config = 0;
  int devices_n = 0;
  oyOptions_s * options = NULL;
  char * first_json = NULL;

  clck = oyClock();
  error = oyOptions_SetFromText( &options,
                                     "//" OY_TYPE_STD "/config/command",
                                     "properties", OY_CREATE_NEW );
  error = oyDevicesGet( 0, "monitor", options, &configs );
  if( error <= 0 && configs )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyDeviceGet() \"monitor\"          " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyDeviceGet() \"monitor\"          " );
  }

  clck = oyClock() - clck;
  devices_n = oyConfigs_Count( configs );
  for( i = 0; i < devices_n; ++i )
  {
    char * json_text = 0;
    config = oyConfigs_Get( configs, i );
    oyDeviceToJSON( config, 0, &json_text, malloc );
    fprintf(zout, "  %d oyDeviceToJSON():\n%s\n", i,
            json_text?json_text:"---" );

    oyConfig_Release( &config );
    if( json_text )
    { PRINT_SUB( oyTESTRESULT_SUCCESS,
      "oyDeviceToJSON() \"monitor\"       " );
      if(i == 0)
        first_json = strdup(json_text);
    } else
    { PRINT_SUB( oyTESTRESULT_FAIL,
      "oyDeviceToJSON() \"monitor\"       " );
    }

    if(i == 1)
    {
      if(strcmp(json_text,first_json) != 0)
      { PRINT_SUB( oyTESTRESULT_SUCCESS,
        "found second unique monitor        " );
      } else
      { PRINT_SUB( oyTESTRESULT_FAIL,
        "first and second monitor are equal " );
      }
    }

    oyDeviceFromJSON( json_text, 0, &config );
    if( config )
    { PRINT_SUB( oyTESTRESULT_SUCCESS,
      "oyDeviceFromJSON() %d             ", oyConfig_Count(config) );
    } else
    { PRINT_SUB( oyTESTRESULT_FAIL,
      "oyDeviceFromJSON() %d             ", oyConfig_Count(config) );
    }

    oyFree_m_( json_text );
  }

  oyConfigs_Release( &configs );
  fprintf( zout, "\n");

  return result;
}


oyTESTRESULT_e testCMMMonitorListing ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int i, k, k_n;
  int error = 0;
  double clck = 0;

#ifdef USE_GETTEXT
  setlocale(LC_ALL,"");
#endif

  oyConfigs_s * configs = 0;
  oyConfig_s * config = 0;
  oyOption_s * o = 0;
  int devices_n = 0;
  char * device_name = 0;
  char * text = 0,
       * val = 0;

  clck = oyClock();
  error = oyDevicesGet( 0, "monitor", 0, &configs );
  clck = oyClock() - clck;
  devices_n = oyConfigs_Count( configs );
  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyDevicesGet() \"monitor\": %d                      %s", devices_n,
                   oyProfilingToString(1,clck/(double)CLOCKS_PER_SEC,"Obj."));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyDevicesGet() \"monitor\": %d     ", devices_n );
  }
  for( i = 0; i < devices_n; ++i )
  {
    config = oyConfigs_Get( configs, i );
    fprintf(zout, "  %d oyConfig_FindString(..\"device_name\"..): %s\n", i,
            oyConfig_FindString( config, "device_name",0 ) );
    if(i==0)
      device_name = oyStringCopy_(oyConfig_FindString( config, "device_name",0),
                                  oyAllocateFunc_ );

    clck = oyClock();
    error = oyDeviceProfileFromDB( config, &text, myAllocFunc );
    clck = oyClock() - clck;
    if(text)
      fprintf( zout, "  %d oyDeviceProfileFromDB(): %s %s\n", i, text,
                   oyProfilingToString(1,clck/(double)CLOCKS_PER_SEC,"Obj."));
    else
      fprintf( zout, "  %d oyDeviceProfileFromDB(): ---\n", i );

    clck = oyClock();
    error = oyDeviceGetInfo( config, oyNAME_NICK, 0, &text, 0 );
    clck = oyClock() - clck;
    fprintf( zout, "  %d oyDeviceGetInfo)(..oyNAME_NICK..): \"%s\" %s\n",
             i, text? text:"???",
                   oyProfilingToString(1,clck/(double)CLOCKS_PER_SEC,"Obj."));
    clck = oyClock();
    error = oyDeviceGetInfo( config, oyNAME_NAME, 0, &text, 0 );
    clck = oyClock() - clck;
    fprintf( zout, "  %d oyDeviceGetInfo)(..oyNAME_NAME..): \"%s\" %s\n",
             i, text? text:"???",
                   oyProfilingToString(1,clck/(double)CLOCKS_PER_SEC,"Obj."));

    oyConfig_Release( &config );
  }
  oyConfigs_Release( &configs );
  fprintf( zout, "\n");

  error = oyDeviceGet( 0, "monitor", device_name, 0, &config );
  k_n = oyConfig_Count( config );
  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyDeviceGet(..\"monitor\" \"%s\"..) %d     ", device_name, k_n );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyDeviceGet(..\"monitor\" \"%s\"..) %d     ", device_name, k_n );
  }
    for( k = 0; k < k_n; ++k )
    {
      o = oyConfig_Get( config, k );
      val = oyOption_GetValueText( o, oyAllocateFunc_ );

      fprintf(zout, "  %d %s: \"%s\"\n", k, oyOption_GetRegistration(o), val );

      if(val) oyDeAllocateFunc_( val ); val = 0;
      oyOption_Release( &o );
    }
  oyConfig_Release( &config );
  oyConfigs_Release( &configs );
  fprintf( zout, "\n");


  return result;
}

oyTESTRESULT_e testCMMDBListing ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int j, k, j_n, k_n;
  int error = 0;
  oyConfigs_s * configs = 0;
  oyConfig_s * config = 0;
  oyOption_s * o = 0;
  char * val = 0;

  error = oyConfigs_FromDB( "//" OY_TYPE_STD "", &configs, 0 );
  j_n = oyConfigs_Count( configs );
  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConfigs_FromDB( \"//" OY_TYPE_STD "\" ) count: %d     ", j_n );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConfigs_FromDB( \"//" OY_TYPE_STD "\" ) count: %d     ", j_n );
  }
  for( j = 0; j < j_n; ++j )
  {
    config = oyConfigs_Get( configs, j );

    k_n = oyConfig_Count( config );
    for( k = 0; k < k_n; ++k )
    {
      o = oyConfig_Get( config, k );

      val = oyOption_GetValueText( o, oyAllocateFunc_ );
      fprintf(zout, "  %d::%d %s %s\n", j,k, oyOption_GetRegistration(o), val );

      if(val) oyDeAllocateFunc_( val ); val = 0;
      oyOption_Release( &o );
    }

    oyConfig_Release( &config );
  }

  return result;
}

oyTESTRESULT_e testCMMMonitorModule ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;
  oyConfigs_s * devices = 0;
  oyOptions_s * options = 0;
  const char * t = ":0.100";
  int error = 0;

  fprintf(stdout, "\n" );

  /* non existing display */
  error = oyOptions_SetFromText( &options,
                                 "//" OY_TYPE_STD "/config/device_name",
                                 t, OY_CREATE_NEW );
  /* clean up */
  error = oyOptions_SetFromText( &options,
                                 "//"OY_TYPE_STD"/config/command",
                                 "unset", OY_CREATE_NEW );
  error = oyDevicesGet( OY_TYPE_STD, "monitor", options, &devices );
  oyConfigs_Release( &devices );

  if( error == -1
#ifdef __APPLE__
      ||  error == 0
#endif
    )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyDevicesGet( \"//" OY_TYPE_STD "\", unset, ... ) = %d", error );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyDevicesGet( \"//" OY_TYPE_STD "\", unset, ... ) = %d", error );
  }

  return result;
}

oyTESTRESULT_e testCMMmonitorDBmatch ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int k, k_n;
  int32_t rank = 0;
  int error = 0;
  oyConfig_s * device = 0;
  oyOption_s * o = 0;
  char * val = 0;
  double clck = 0;
  const char * device_name = NULL; /* getenv("DISPLAY") + ".0"; */

  fprintf( zout, "load a device ...\n");
  clck = oyClock();
  error = oyDeviceGet( 0, "monitor", device_name, 0, &device );
  clck = oyClock() - clck;
  k_n = oyConfig_Count( device );
  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyDeviceGet(..\"monitor\" \"%s\".. &device ) %d     %s", device_name, k_n,
                   oyProfilingToString(1,clck/(double)CLOCKS_PER_SEC,"Obj."));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyDeviceGet(..\"monitor\" \"%s\".. &device) %d", device_name, k_n );
  }

  fprintf( zout, "... and search for the devices DB entry ...\n");
  clck = oyClock();
  error = oyConfig_GetDB( device, &rank );
  clck = oyClock() - clck;
  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConfig_GetDB( device )                         %s",
                   oyProfilingToString(1,clck/(double)CLOCKS_PER_SEC,"Obj."));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConfig_GetDB( device )                         %s",
                   oyProfilingToString(1,clck/(double)CLOCKS_PER_SEC,"Obj."));
  }
  if(device && rank > 0)
  {
    fprintf(zout,"rank: %d\n", rank);
    k_n = oyConfig_Count( device );
    for( k = 0; k < k_n; ++k )
    {
      o = oyConfig_Get( device, k );

      val = oyOption_GetValueText( o, oyAllocateFunc_ );
      fprintf(zout, "  d::%d %s: \"%s\"\n", k,
      strchr(strchr(strchr(strchr(oyOption_GetRegistration(o),'/')+1,'/')+1,'/')+1,'/')+1,
              val );

      if(val) oyDeAllocateFunc_( val ); val = 0;
      oyOption_Release( &o );
    }
    //error = oyConfig_EraseFromDB( config );
  }


  return result;
}


#include "oyranos_forms.h"

#define H(type,value) oyFormsAddHeadline( &t, type, value,\
                                           oyAllocateFunc_, oyDeAllocateFunc_ );
#define CHOICE(ref,label,help) oyFormsStartChoice( &t, ref, label, help,\
                                           oyAllocateFunc_, oyDeAllocateFunc_ );
#define ITEM(value,label) oyFormsAddItem( &t, value, label,\
                                           oyAllocateFunc_, oyDeAllocateFunc_ );
#define CHOICE_END STRING_ADD( t, "      </xf:choices>\n     </xf:select1>\n" );

#include "oyCMMapi4_s_.h"
#include "oyCMMapi6_s_.h"
#include "oyCMMapi7_s_.h"
#include "oyCMMapi8_s_.h"
#include "oyCMMapi9_s_.h"
#include "oyCMMapi10_s_.h"
#include "oyCMMapiFilter_s_.h"
#include "oyCMMinfo_s_.h"

#include "oyranos_module.h"
#include "oyranos_module_internal.h"

oyTESTRESULT_e testCMMsShow ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  int i, j, k, l;
  uint32_t count = 0;
  char ** texts = 0,
        * text = 0,
        * text_tmp = (char*)oyAllocateFunc_(65535),
        * t = 0,
        * rfile = 0;
  oyCMMinfo_s_ * cmm_info = 0;
  oyCMMapi4_s_ * cmm_api4 = 0;
  oyCMMapi6_s_ * cmm_api6 = 0;
  oyCMMapi7_s_ * cmm_api7 = 0;
  oyCMMapi8_s_ * cmm_api8 = 0;
  oyCMMapi9_s_ * cmm_api9 = 0;
  oyCMMapi10_s_ * cmm_api10 = 0;
  oyCMMapi_s_ * tmp = 0;
  oyCMMapiFilter_s_ * cmm_filter = 0;


  fprintf(zout, "\n" );

  texts = oyCMMsGetLibNames_( &count, 0 );

  /* Create a oforms style xhtml to present in a XFORMS viewer like
   * oyranos-xforms-fltk or FF with XFORMS plug-in.
   * Pretty large here.
   */
  STRING_ADD( t, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\
<html xmlns=\"http://www.w3.org/1999/xhtml\"\n\
      xmlns:xf=\"http://www.w3.org/2002/xforms\"\n\
      xmlns:oicc=\"http://www.oyranos.org/2009/oyranos_icc\">\n" );
  STRING_ADD( t, "<head>\n\
  <title>Filter options</title>\n\
  <xf:model>\n\
    <xf:instance xmlns=\"\">\n\
     <shared><dummy>0</dummy></shared>\n\
    </xf:instance>\n\
  </xf:model>\n");
  STRING_ADD( t, "  <style type=\"text/css\">\n\
  @namespace xf url(\"http://www.w3.org/2002/xforms\");\n\
  xf|label {\n\
   font-family: Helvetica, Geneva, Lucida, sans-serif;\n\
   width: 24ex;\n\
   text-align: right;\n\
   padding-right: 1em;\n\
  }\n\
  xf|select1 { display: table-row; }\n\
  xf|select1 xf|label, xf|choices xf|label  { display: table-cell; }\n\
  </style></head>\n\
<body>\n\
 <xf:group>\n" );
  H(3,"Oyranos Module Overview")

  for( i = 0; i < (int)count; ++i)
  {
    cmm_info = (oyCMMinfo_s_*)oyCMMinfoFromLibName_( texts[i] );
    if(cmm_info)
      tmp = (oyCMMapi_s_*)cmm_info->api;
    else
      tmp = 0;

    /* short skip for non compatible modules */
    if(oyCMMapi_Check_((oyCMMapi_s*)tmp) == oyOBJECT_NONE)
    {
      text = oyCMMinfoPrint_( (oyCMMinfo_s*)cmm_info, 1 );
      STRING_ADD( text, "    Not accepted by oyCMMapi_Check_() - Stop\n");
      fprintf(zout,"%d: \"%s\": %s\n\n", i, texts[i], text );
      continue;
    }

    text = oyCMMinfoPrint_( (oyCMMinfo_s*)cmm_info, 0 );

        while(tmp)
        {
          oyOBJECT_e type = oyOBJECT_NONE;
          char num[48],
               * api_reg = 0;
          const char * ctmp;

          /* oforms */
          CHOICE( "shared/dummy", oyStructTypeToText(tmp->type_), text )
          ITEM( "0", cmm_info->cmm )
          CHOICE_END

          type = oyCMMapi_Check_((oyCMMapi_s*)tmp);

          oySprintf_(num,"    %d:", type );
          STRING_ADD( text, num );
          ctmp = oyStruct_TypeToText((oyStruct_s*)tmp);
          STRING_ADD( text, ctmp );
          STRING_ADD( text, "\n" );

          if(type == oyOBJECT_CMM_API5_S)
          {
            cmm_filter = (oyCMMapiFilter_s_*) tmp;

            {
              oyCMMapiFilter_s_ * api = 0;
              oyCMMapiFilters_s * apis = 0;
              uint32_t * rank_list = 0;
              uint32_t apis_n = 0;
              char * classe = 0;
              const char * nick = cmm_info->cmm;

              classe = oyFilterRegistrationToText( cmm_filter->registration,
                                                   oyFILTER_REG_TYPE, 0 );
              api_reg = oyStringCopy_("//", oyAllocateFunc_ );
              STRING_ADD( api_reg, classe );
              if(classe)
              oyFree_m_( classe );


              H( 4, "API(s) load from Meta module" )
              STRING_ADD( text, "    API(s) load from Meta module:\n" );

              for(j = oyOBJECT_CMM_API4_S; j <= (int)oyOBJECT_CMM_API10_S; j++)
              {
                apis = oyCMMsGetFilterApis_( nick, 0, api_reg, (oyOBJECT_e)j,
                                             oyFILTER_REG_MODE_NONE,
                                             &rank_list, &apis_n );

                apis_n = oyCMMapiFilters_Count( apis );
                for(k = 0; k < (int)apis_n; ++k)
                {
                  api = (oyCMMapiFilter_s_*)oyCMMapiFilters_Get( apis, k );

                  if(api)
                  snprintf( text_tmp, 65535,
                            "      [%s]: \"%s\"  %d\n        %s\n",
                            oyStructTypeToText(api->type_),
                            api->registration,
                            (int)rank_list[k], api->id_ );
                  else
                    sprintf(text_tmp,"      no api obtained %d",k);
                  STRING_ADD( text, text_tmp );
                  /* oforms */
                  CHOICE( "shared/dummy", oyStructTypeToText(api->type_), text_tmp )
                  ITEM( "0", api->registration )
                  CHOICE_END

                  if(api->type_ == oyOBJECT_CMM_API4_S)
                  {
                    cmm_api4 = (oyCMMapi4_s_*) api;
                    oyStringAdd_( &text, "        category: ",
                                  oyAllocateFunc_, oyDeAllocateFunc_ );
                    if(cmm_api4->ui->category)
                    oyStringAdd_( &text, cmm_api4->ui->category,
                                  oyAllocateFunc_, oyDeAllocateFunc_ );
                    oyStringAdd_( &text, "\n        options: ",
                                  oyAllocateFunc_, oyDeAllocateFunc_ );
                    if(cmm_api4->ui->options)
                    oyStringAdd_( &text, cmm_api4->ui->options,
                                  oyAllocateFunc_, oyDeAllocateFunc_ );
                    oyStringAdd_( &text, oyXMLgetElement_(cmm_api4->ui->options,
                                  "freedesktop.org/default/profile",
                                  "editing_rgb" ),
                                  oyAllocateFunc_, oyDeAllocateFunc_ );
                    STRING_ADD( text, "\n" );
                  }

                  if(api->type_ == oyOBJECT_CMM_API6_S)
                  {
                    cmm_api6 = (oyCMMapi6_s_*) api;
                    snprintf( text_tmp, 65535,
                            "        \"%s\" -> \"%s\"\n",
                            cmm_api6->data_type_in,
                            cmm_api6->data_type_out );
                    STRING_ADD( text, text_tmp );
                  }

                  if(api->type_ == oyOBJECT_CMM_API7_S)
                  {
                    cmm_api7 = (oyCMMapi7_s_*) api;
                    snprintf( text_tmp, 65535,
                            "        context type \"%s\" plugs: %d  sockets: %d\n",
                            cmm_api7->context_type,
                            cmm_api7->plugs_n + cmm_api7->plugs_last_add,
                            cmm_api7->sockets_n + cmm_api7->sockets_last_add );
                    STRING_ADD( text, text_tmp );
                    for(l = 0; l < (int)cmm_api7->plugs_n; ++l)
                    {
                      snprintf( text_tmp, 65535,
                            "        plug[%d]: type:\"%s\" id:\"%s\" \"%s\" \"%s\"\n", l,
                            oyNoEmptyString_m_(oyConnector_GetReg( cmm_api7->plugs[l])),
                            oyNoEmptyString_m_(oyConnector_GetText(
                                cmm_api7->plugs[l],"name",oyNAME_NICK)),
                            oyNoEmptyString_m_(oyConnector_GetText(
                                cmm_api7->plugs[l],"name",oyNAME_NAME)),
                            oyNoEmptyString_m_(oyConnector_GetText(
                                cmm_api7->plugs[l],"name",oyNAME_DESCRIPTION))
                            );
                      STRING_ADD( text, text_tmp );
                    }
                    for(l = 0; l < (int)cmm_api7->sockets_n; ++l)
                    {
                      snprintf( text_tmp, 65535,
                            "        sock[%d]: type:\"%s\" id:\"%s\" \"%s\" \"%s\"\n", l,
                            oyNoEmptyString_m_(oyConnector_GetReg( cmm_api7->sockets[l])),
                            oyNoEmptyString_m_(oyConnector_GetText(
                                cmm_api7->sockets[l],"name",oyNAME_NICK)),
                            oyNoEmptyString_m_(oyConnector_GetText(
                                cmm_api7->sockets[l],"name",oyNAME_NAME)),
                            oyNoEmptyString_m_(oyConnector_GetText(
                                cmm_api7->sockets[l],"name",oyNAME_DESCRIPTION))
                            );
                      STRING_ADD( text, text_tmp );
                    }
                  }

                  if(api->type_ == oyOBJECT_CMM_API8_S)
                  {
                    l = 0;
                    cmm_api8 = (oyCMMapi8_s_*) api;
                    snprintf( text_tmp, 65535,
                              "        rank_map[#]:"
                                      " \"key\"  match,none_match,not_found\n" );
                    STRING_ADD( text, text_tmp );
                    while(cmm_api8->rank_map[l].key)
                    {
                      snprintf( text_tmp, 65535,
                              "        rank_map[%d]: \"%s\"  %d,%d,%d\n", l,
                              cmm_api8->rank_map[l].key,
                              cmm_api8->rank_map[l].match_value,
                              cmm_api8->rank_map[l].none_match_value,
                              cmm_api8->rank_map[l].not_found_value
                              );
                      STRING_ADD( text, text_tmp );
                      ++l;
                    }
                  }

                  if(api->type_ == oyOBJECT_CMM_API9_S)
                  {
                    cmm_api9 = (oyCMMapi9_s_*) api;
                    snprintf( text_tmp, 65535,
                            "        \"%s\"\n"
                            "        supported pattern: \"%s\"\n",
                            cmm_api9->options,
                            cmm_api9->pattern );
                    STRING_ADD( text, text_tmp );
                  }

                  if(api->type_ == oyOBJECT_CMM_API10_S)
                  {
                    cmm_api10 = (oyCMMapi10_s_*) api;
                    for(l = 0; l < 3; ++l)
                    {
                      if(cmm_api10->texts[l])
                      {
                        snprintf( text_tmp, 65535,
                            "        \"%s\":\n"
                            "        \"%s\"\n",
                            cmm_api10->texts[l],
                            cmm_api10->getText( cmm_api10->texts[l],
                                                oyNAME_DESCRIPTION,
                                                (oyStruct_s*)cmm_api10 ) );
                        STRING_ADD( text, text_tmp );
                      } else
                        break;
                    }
                  }

                  STRING_ADD( text, "\n" );
                  //oyCMMapiFilter_Release( &api );
                }
                oyCMMapiFilters_Release( &apis );
              }
              oyFree_m_(api_reg);
            }
          } else
          if(oyIsOfTypeCMMapiFilter( type ))
          {
            cmm_filter = (oyCMMapiFilter_s_*) tmp;

            snprintf( text_tmp, 65535, "%s: %s\n",
                      oyStructTypeToText( tmp->type_ ),
                      cmm_filter->registration );
            STRING_ADD( text, text_tmp );

          }
          tmp = (oyCMMapi_s_*)tmp->next;
        }

    fprintf(zout,"%d: \"%s\": %s\n\n", i, texts[i], text );

  }
  oyStringListRelease_( &texts, count, free );

  if( count )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyCMMsGetLibNames_( ) found %d                     ", count );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyCMMsGetLibNames_( ) found %d                     ", count );
  }

  oyDeAllocateFunc_( text_tmp );

  STRING_ADD( t, "     </xf:group>\n</body>\n</html>\n" );
  remove("test2_CMMs.xhtml");
  oyWriteMemToFile2_( "test2_CMMs.xhtml", t, strlen(t),0/*OY_FILE_NAME_SEARCH*/,
                      &rfile, malloc );
  fprintf(zout, "Wrote %s\n", rfile?rfile:"test2_CMMs.xhtml" );
  free( rfile );

  return result;
}

#include <kdb.h>
#ifndef KDB_VERSION_MAJOR
#define KDB_VERSION_MAJOR 0
#endif
#ifndef KDB_VERSION_MINOR
#define KDB_VERSION_MINOR 0
#endif
#define KDB_VERSION_NUM (KDB_VERSION_MAJOR*10000 + KDB_VERSION_MINOR*100)
#ifdef __cplusplus
#define ckdb ckdb::
#else
#define ckdb
#endif
extern ckdb KDB * oy_handle_;

double d[6] = {0.5,0.5,0.5,0,0,0};

#if KDB_VERSION_NUM >= 800
extern "C" {int oyGetKey(ckdb Key*);}
#define dbGetKey(a,b) oyGetKey(b)
#else
#define dbGetKey(a,b) ckdb kdbGetKey(a,b)
#endif

#include "oyFilterCore_s_.h"
#include "oyNamedColor_s.h"

oyTESTRESULT_e testCMMnmRun ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;
  oyNamedColor_s * c = 0;
  oyProfile_s * prof = oyProfile_FromStd( oyEDITING_XYZ, 0, NULL );
  int error = 0, l_error = 0,
      i,n = 10;

  fprintf(stdout, "\n" );
#if 1
  double clck = oyClock();
  for(i = 0; i < n*10000; ++i)
  {
    c = oyNamedColor_Create( NULL, NULL,0, prof, 0 );
    oyNamedColor_Release( &c );
  }
  clck = oyClock() - clck;

  c = oyNamedColor_Create( NULL, NULL,0, prof, 0 );
  if( c )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyNamedColor_Create( )             %s",
                   oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Obj."));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyNamedColor_Create( )                            " );
  }


  const char * key_name = OY_STD"/behaviour/rendering_bpc";
  oyAlloc_f allocate_func = oyAllocateFunc_;

  char* name = 0;
  char* full_key_name = 0;
  name = (char*) oyAllocateWrapFunc_( MAX_PATH, allocate_func );
  full_key_name = (char*) oyAllocateFunc_ (MAX_PATH);
  clck = oyClock();

  for(i = 0; i < n*3*17; ++i)
  {
  int rc = 0;
  ckdb Key * key = 0;
  int success = 0;

  sprintf( full_key_name, "%s%s", OY_USER, key_name );

  /** check if the key is a binary one */
  key = ckdb keyNew( full_key_name, KEY_END );
  rc= dbGetKey( oy_handle_, key );
  success = ckdb keyIsString(key);

  if(success)
    rc = ckdb keyGetString ( key, name, MAX_PATH );
  ckdb keyDel( key ); key = 0;

  if( rc || !strlen( name ))
  {
    sprintf( full_key_name, "%s%s", OY_SYS, key_name );
    key = ckdb keyNew( full_key_name, KEY_END );
    if(success)
      rc = ckdb keyGetString( key, name, MAX_PATH );
    ckdb keyDel( key ); key = 0;
  }

  }
  oyDeAllocateFunc_( full_key_name );
  oyDeAllocateFunc_( name );
  clck = oyClock() - clck;

  if( i )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyGetKeyString_()                   %s",
                  oyProfilingToString(n,clck/(double)CLOCKS_PER_SEC, "Filter"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyGetKeyString_()                                  " );
  }



  clck = oyClock();
  oyOption_s * option = oyOption_FromRegistration(OY_STD"/behaviour/rendering_bpc", 0);

  for(i = 0; i < n*3*17; ++i)
  {
  int error = !option || !oyOption_GetRegistration(option);
  char * text = 0,
       * ptr = 0;

  oyExportStart_(EXPORT_SETTING);

  if(error <= 0)
    text = oyGetKeyString_( oyOption_GetRegistration(option), oyAllocateFunc_ );

  if(error <= 0)
  {
    if(text)
      oyOption_SetFromText( option, text, 0 );
    else
    {
      ptr = oyGetKeyString_( oyOption_GetRegistration(option), oyAllocateFunc_ );
      if(ptr)
      {
        oyOption_SetFromData( option, ptr, strlen(ptr) );
        oyFree_m_( ptr );
      }
    }
  }

  if(text)
    oyFree_m_( text );

  oyExportEnd_();
  }
  clck = oyClock() - clck;

  if( i )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyOption_SetValueFromDB()           %s",
                  oyProfilingToString(n,clck/(double)CLOCKS_PER_SEC, "Filter"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyOption_SetValueFromDB()                          " );
  }

  clck = oyClock();
  for(i = 0; i < n*3; ++i)
  {
  oyFilterCore_s_ * filter = oyFilterCore_New_( 0 );
  oyCMMapi4_s_ * api4 = 0;
  oyObject_s object = 0;

  if(error <= 0)
  {
    api4 = (oyCMMapi4_s_*) oyCMMsGetFilterApi_( 0,
                                "//" OY_TYPE_STD "/root", oyOBJECT_CMM_API4_S );
    error = !api4;
  }

  if(error <= 0)
    error = oyFilterCore_SetCMMapi4_( filter, api4 );

  oyOptions_s * s = 0,
              * opts_tmp = 0,
              * opts_tmp2 = 0;
  oyOption_s * o = 0;
  char * type_txt = oyFilterRegistrationToText( filter->registration_,
                                                oyFILTER_REG_TYPE, 0 );
  oyCMMapi5_s_ * api5 = 0;
  int i,n, flags = 0;
  int error = !filter || !filter->api4_;

  /* by default we parse both sources */
  if(!(flags & OY_SELECT_FILTER) && !(flags & OY_SELECT_COMMON))
    flags |= OY_SELECT_FILTER | OY_SELECT_COMMON;

  if(!error)
  {
    /*
        Programm:
        1. get filter and its type
        2. get implementation for filter type
        3. parse static common options from meta module
        4. parse static options from filter 
        5. merge both
        6. get stored values from disk
     */

    /*  1. get filter */

    /*  2. get implementation for filter type */
    api5 = filter->api4_->api5_;

    /*  3. parse static common options from meta module */
    if(api5 && flags & OY_SELECT_COMMON)
    {
      oyCMMapiFilters_s * apis;
      int apis_n = 0;
      oyCMMapi9_s_ * cmm_api9 = 0;
      char * klass, * api_reg;

      klass = oyFilterRegistrationToText( filter->registration_,
                                          oyFILTER_REG_TYPE, 0 );
      api_reg = oyStringCopy_("//", oyAllocateFunc_ );
      STRING_ADD( api_reg, klass );
      oyFree_m_( klass );

      s = oyOptions_New( 0 );

      apis = oyCMMsGetFilterApis_( 0,0, api_reg,
                                   oyOBJECT_CMM_API9_S,
                                   oyFILTER_REG_MODE_STRIP_IMPLEMENTATION_ATTR,
                                   0,0);
      apis_n = oyCMMapiFilters_Count( apis );
      for(i = 0; i < apis_n; ++i)
      {
        cmm_api9 = (oyCMMapi9_s_*) oyCMMapiFilters_Get( apis, i );
        if(oyFilterRegistrationMatch( filter->registration_, cmm_api9->pattern,
                                      oyOBJECT_NONE ))
        {
          opts_tmp = oyOptions_FromText( cmm_api9->options, 0, object );
          oyOptions_AppendOpts( s, opts_tmp );
          oyOptions_Release( &opts_tmp );
        }
        if(cmm_api9->release)
          cmm_api9->release( (oyStruct_s**)&cmm_api9 );
      }
      oyCMMapiFilters_Release( &apis );
      oyFree_m_( api_reg );
      opts_tmp = s; s = 0;
    }
    /* requires step 2 */

    /*  4. parse static options from filter */
    if(flags & OY_SELECT_FILTER)
      opts_tmp2 = oyOptions_FromText( filter->api4_->ui->options, 0, object );

    /*  5. merge */
    s = oyOptions_FromBoolean( opts_tmp, opts_tmp2, oyBOOLEAN_UNION, object );

    oyOptions_Release( &opts_tmp );
    oyOptions_Release( &opts_tmp2 );

    /*  6. get stored values */
    n = oyOptions_Count( s );
    for(i = 0; i < n && error <= 0; ++i)
    {
      o = oyOptions_Get( s, i );
      oyOption_SetSource( o, oyOPTIONSOURCE_FILTER );
      /* ask Elektra */
      if(!(flags & oyOPTIONSOURCE_FILTER))
        error = oyOption_SetValueFromDB( o );
      oyOption_Release( &o );
    }
#if 0
    error = oyOptions_DoFilter ( s, flags, type_txt );
#else
    error = 1;
#endif
  }

  if(type_txt)
    oyDeAllocateFunc_( type_txt );

  }
  clck = oyClock() - clck;

  if( i )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyOptions_ForFilter_()              %s",
                  oyProfilingToString(n,clck/(double)CLOCKS_PER_SEC, "Filter"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyOptions_ForFilter_()                             " );
  }

  clck = oyClock();
  {
  oyFilterCore_s_ * s = oyFilterCore_New_( 0 );
  int error = !s;
  uint32_t ret = 0;
  oyOptions_s * opts_tmp = 0, * options = 0;
  oyCMMapi4_s_ * api4 = 0;

  if(error <= 0)
  {
    api4 = (oyCMMapi4_s_*) oyCMMsGetFilterApi_( 0,
                                "//" OY_TYPE_STD "/root", oyOBJECT_CMM_API4_S );
    error = !api4;
  }

  if(error <= 0)
    error = oyFilterCore_SetCMMapi4_( s, api4 );

  if(error <= 0)
  {
    for(i = 0; i < n*3; ++i)
      opts_tmp = oyOptions_ForFilter_( s, NULL, 0, s->oy_);
#if 0
    s->options_ = api4->oyCMMFilter_ValidateOptions( s, options, 0, &ret );
#endif
    error = ret;
    
    /* @todo test oyBOOLEAN_SUBSTRACTION for correctness */
    s->options_ = oyOptions_FromBoolean( opts_tmp, options,
                                         oyBOOLEAN_SUBSTRACTION, s->oy_ );
    oyOptions_Release( &opts_tmp );
  }

    oyFilterCore_Release( (oyFilterCore_s**)&s );
  }
  clck = oyClock() - clck;

  if( i )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyOptions_ForFilter_()              %s",
                  oyProfilingToString(n,clck/(double)CLOCKS_PER_SEC, "Filter"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyOptions_ForFilter_()                             " );
  }



  oyOptions_s * options = oyOptions_New(0);
  clck = oyClock();
  for(i = 0; i < n*3*10000; ++i)
  {
    oyFilterCore_s * core = oyFilterCore_NewWith( "//" OY_TYPE_STD "/root",
                                                  options,0 );
    if(!core) break;
    oyFilterCore_Release( &core );
  }
  clck = oyClock() - clck;

  if( i )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyFilterCore_New()                  %s",
                 oyProfilingToString(i/3,clck/(double)CLOCKS_PER_SEC, "Cores"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyFilterCore_New()                                 " );
  }


  clck = oyClock();
  const char * registration = "//" OY_TYPE_STD "/root";
  for(i = 0; i < n*3*10000; ++i)
  {
    oyCMMapi4_s_ * api4 = 0;
    api4 = (oyCMMapi4_s_*) oyCMMsGetFilterApi_( 0,
                                            registration, oyOBJECT_CMM_API4_S );
    error = !api4;
    if(!(i%30000)) fprintf(zout, "." ); fflush(zout);
  }
  fprintf(zout,"\n");
  clck = oyClock() - clck;

  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyFilterCore_New() oyCMMapi4_s      %s",
                 oyProfilingToString(i/3,clck/(double)CLOCKS_PER_SEC, "Cores"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyFilterCore_New() oyCMMapi4_s                     " );
  }


  clck = oyClock();
  oyConversion_s * s = 0;
  oyFilterNode_s * in = 0, * out = 0;
  oyImage_s * input  = NULL,
            * output = NULL;
  double * buf_in = &d[0],
         * buf_out = &d[3];
  oyDATATYPE_e buf_type_in = oyDOUBLE,
               buf_type_out = oyDOUBLE;
  uint32_t icc_profile_flags =oyICCProfileSelectionFlagsFromOptions( OY_CMM_STD,
                                       "//" OY_TYPE_STD "/icc_color", NULL, 0 );
  oyProfile_s * p_in = prof,
              * p_out = oyProfile_FromStd( oyASSUMED_WEB, icc_profile_flags, 0 );


  for(i = 0; i < n*100; ++i)
  if(error <= 0)
  {
    input =oyImage_Create( 1,1, 
                         buf_in ,
                         oyChannels_m(oyProfile_GetChannelsCount(p_in)) |
                          oyDataType_m(buf_type_in),
                         p_in,
                         0 );
    output=oyImage_Create( 1,1, 
                         buf_out ,
                         oyChannels_m(oyProfile_GetChannelsCount(p_out)) |
                          oyDataType_m(buf_type_out),
                         p_out,
                         0 );
    error = !input || !output;


    s = oyConversion_New ( 0 );
    error = !s;    

    if(error <= 0)
      in = oyFilterNode_NewWith( "//" OY_TYPE_STD "/root", options, 0 );
    if(error <= 0)
      error = oyConversion_Set( s, in, 0 );
    if(error <= 0)
      error = oyFilterNode_SetData( in, (oyStruct_s*)input, 0, 0 );

    if(error <= 0)
      out = oyFilterNode_FromOptions( OY_CMM_STD, "//" OY_TYPE_STD "/icc_color", options, NULL );
    if(error <= 0)
      error = oyFilterNode_SetData( out, (oyStruct_s*)output, 0, 0 );
    if(error <= 0)
      error = oyFilterNode_Connect( in, "//" OY_TYPE_STD "/data",
                                    out, "//" OY_TYPE_STD "/data", 0 );

    in = out; out = 0;

    if(error <= 0)
      out = oyFilterNode_NewWith( "//" OY_TYPE_STD "/output", options, 0 );
    if(error <= 0)
    {
      error = oyFilterNode_Connect( in, "//" OY_TYPE_STD "/data",
                                    out, "//" OY_TYPE_STD "/data", 0 );
      if(error)
        WARNc1_S( "could not add  filter: %s\n", "//" OY_TYPE_STD "/output" );
    }
    if(error <= 0)
      error = oyConversion_Set( s, 0, out );
    oyConversion_Release( &s );
    oyImage_Release( &input );
    oyImage_Release( &output );
    if(!(i%1000)) fprintf(zout, "." ); fflush(stdout);
  }
  fprintf(zout,"\n");
  clck = oyClock() - clck;

  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConversion_CreateBasicPixels()    %s",
                    oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Obj."));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConversion_CreateBasicPixels()                   " );
  }
#else
  oyConversion_s * s = 0;
  oyFilterNode_s * in = 0, * out = 0;
  oyImage_s * input  = NULL,
            * output = NULL;
  double * buf_in = &d[0],
         * buf_out = &d[3];
  oyDATATYPE_e buf_type_in = oyDOUBLE,
               buf_type_out = oyDOUBLE;
  oyProfile_s * p_in = prof,
              * p_out = oyProfile_FromStd( oyASSUMED_WEB, icc_profile_flags, 0 );
  double clck;
#endif

  input =oyImage_Create( 1,1, 
                         buf_in ,
                         oyChannels_m(oyProfile_GetChannelsCount(p_in)) |
                          oyDataType_m(buf_type_in),
                         p_in,
                         0 );
  output=oyImage_Create( 1,1, 
                         buf_out ,
                         oyChannels_m(oyProfile_GetChannelsCount(p_out)) |
                          oyDataType_m(buf_type_out),
                         p_out,
                         0 );

  #define OY_ERR if(l_error != 0) error = l_error;

  oyFilterPlug_s * plug = 0;
  oyPixelAccess_s * pixel_access = 0;
  s = oyConversion_CreateBasicPixels( input,output, 0, 0 );
  out = oyConversion_GetNode( s, OY_OUTPUT );
  if(s && out)
    plug = oyFilterNode_GetPlug( out, 0 );
  else
    error = 1;
  pixel_access = oyPixelAccess_Create( 0,0, plug,
                                           oyPIXEL_ACCESS_IMAGE, 0 );
  oyFilterPlug_Release( &plug );

  clck = oyClock();
  for(i = 0; i < n*1000; ++i)
  if(error <= 0)
  {
#if 1
    error  = oyConversion_RunPixels( s, pixel_access );
#else

  oyConversion_s * conversion = s;
  oyFilterPlug_s * plug = 0;
  oyFilterCore_s * filter = 0;
  oyImage_s * image = 0, * image_input = 0;
  int error = 0, result, l_error = 0, i,n, dirty = 0, tmp_ticket = 0;

  /* conversion->out_ has to be linear, so we access only the first plug */
  plug = oyFilterNode_GetPlug( out, 0 );
  if(!out || !plug)
  {
    WARNc1_S("graph incomplete [%d]", s ? oyObject_GetId( s->oy_ ) : -1)
    break;
  }                                    
                                       
  if(!pixel_access)
  {
    /* create a very simple pixel iterator as job ticket */
    if(plug)
      pixel_access = oyPixelAccess_Create( 0,0, plug,
                                           oyPIXEL_ACCESS_IMAGE, 0 );
    tmp_ticket = 1;
  } 

  /* should be the same as conversion->out_->filter */
  filter = oyFilterNode_GetCore( out );
  image = oyConversion_GetImage( conversion, OY_OUTPUT );
  oyFilterNode_Release (&out );

  if(pixel_access)
    result = oyImage_FillArray( image, pixel_access->output_image_roi, 0,
                                &pixel_access->array, 0, 0 );
  error = ( result != 0 );

  if(error <= 0)
    error = conversion->out_->api7_->oyCMMFilterPlug_Run( plug, pixel_access );

  if(error != 0)
  {
    dirty = oyOptions_FindString( pixel_access->graph->options, "dirty", "true")
            ? 1 : 0;

    /* refresh the graph representation */
    oyFilterGraph_SetFromNode( pixel_access->graph, conversion->input, 0, 0 );

    /* resolve missing data */
    image_input = oyFilterPlug_ResolveImage( plug, plug->remote_socket_,
                                             pixel_access );
    oyImage_Release( &image_input );

    n = oyFilterNodes_Count( pixel_access->graph->nodes );
    for(i = 0; i < n; ++i)
    {
      l_error = oyArray2d_Release( &pixel_access->array ); OY_ERR
      l_error = oyImage_FillArray( image, pixel_access->output_image_roi, 0,
                                   &pixel_access->array, 0, 0 ); OY_ERR

      if(error != 0 &&
         dirty)
      {
        if(pixel_access->start_xy[0] != pixel_access->start_xy_old[0] ||
           pixel_access->start_xy[1] != pixel_access->start_xy_old[1])
        {
          /* set back to previous values, at least for the simplest case */
          pixel_access->start_xy[0] = pixel_access->start_xy_old[0];
          pixel_access->start_xy[1] = pixel_access->start_xy_old[1];
        }

        oyFilterGraph_PrepareContexts( pixel_access->graph, 1 );
        error = conversion->out_->api7_->oyCMMFilterPlug_Run( plug,
                                                              pixel_access);
      }

      if(error == 0)
        break;
    }
  }

  if(tmp_ticket)
  {
    /* write the data to the output image */
    if(image != pixel_access->output_image)
      result = oyImage_ReadArray( image, pixel_access->output_image_roi,
                                         pixel_access->array, 0 );
    oyPixelAccess_Release( &pixel_access );
  }

  oyImage_Release( &image );

#endif
  }
  clck = oyClock() - clck;

  oyConversion_Release ( &s );
  oyPixelAccess_Release( &pixel_access );

  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConversion_RunPixels( oyPixelAcce.%s",
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Pixel"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConversion_RunPixels()                           " );
  }


  clck = oyClock();
  for(i = 0; i < 20*n; ++i)
  if(error <= 0)
  {
    s = oyConversion_CreateBasicPixels( input,output, options, 0 );
    error  = oyConversion_RunPixels( s, 0 );
    oyConversion_Release ( &s );
    if(!(i%100)) fprintf(zout, "." ); fflush(zout);
  }
  fprintf(zout, "\n" );

  clck = oyClock() - clck;
  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "+ oyConversion_RunPixels()          %s",
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Pixel"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "+ oyConversion_RunPixels()                         " );
  }



  clck = oyClock();

  for(i = 0; i < n*10 && error <= 0; ++i)
  {
    l_error = oyNamedColor_SetColorStd ( c, oyASSUMED_WEB,
                                           (oyPointer)d, oyDOUBLE, 0, options );
    if(error <= 0)
      error = l_error;
  }
  clck = oyClock() - clck;

  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyNamedColor_SetColorStd()        %s",
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Pixel"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyNamedColor_SetColorStd() oyASSUMED_WEB         " );
  }

  p_in = oyProfile_FromStd ( oyASSUMED_WEB, icc_profile_flags, NULL );
  p_out = oyProfile_FromStd ( oyEDITING_XYZ, icc_profile_flags, NULL );

  clck = oyClock();
  for(i = 0; i < n*20; ++i)
  {

  oyImage_s * in  = NULL,
            * out = NULL;
  oyConversion_s * conv = NULL;

  in    = oyImage_Create( 1,1, 
                         buf_in ,
                         oyChannels_m(oyProfile_GetChannelsCount(p_in)) |
                          oyDataType_m(buf_type_in),
                         p_in,
                         0 );
  out   = oyImage_Create( 1,1, 
                         buf_out ,
                         oyChannels_m(oyProfile_GetChannelsCount(p_out)) |
                          oyDataType_m(buf_type_out),
                         p_out,
                         0 );

  conv   = oyConversion_CreateBasicPixels( in,out, options, 0 );
  if(!error)
  error  = oyConversion_RunPixels( conv, 0 );

  oyConversion_Release( &conv );
  oyImage_Release( &in );
  oyImage_Release( &out );
  }
  clck = oyClock() - clck;


  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyColorConvert_()                  %s",
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Pixel"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyColorConvert_()                                 " );
  }


  input  = oyImage_Create( 1,1, 
                         buf_in ,
                         oyChannels_m(oyProfile_GetChannelsCount(p_in)) |
                          oyDataType_m(buf_type_in),
                         p_in,
                         0 );
  output = oyImage_Create( 1,1, 
                         buf_out ,
                         oyChannels_m(oyProfile_GetChannelsCount(p_out)) |
                          oyDataType_m(buf_type_out),
                         p_out,
                         0 );
  //oyFilterPlug_s * plug = 0;
  //oyPixelAccess_s   * pixel_access = 0;
  oyConversion_s * conv   = oyConversion_CreateBasicPixels( input,output, 0,0 );

  out = oyConversion_GetNode( conv, OY_OUTPUT );

  /* conversion->out_ has to be linear, so we access only the first plug */
  if(conv)
    plug = oyFilterNode_GetPlug( out, 0 );
  oyFilterNode_Release (&out );

  /* create a very simple pixel iterator as job ticket */
  if(plug)
    pixel_access = oyPixelAccess_Create( 0,0, plug,
                                           oyPIXEL_ACCESS_IMAGE, 0 );
  error  = oyConversion_RunPixels( conv, pixel_access );

  clck = oyClock();
  if(pixel_access)
  for(i = 0; i < n*1000; ++i)
  {
    if(!error)
      error  = oyConversion_RunPixels( conv, pixel_access );
  }
  clck = oyClock() - clck;



  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyColorConvert_() sans oyPixelAcce.%s",
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Pixel"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyColorConvert_() sans oyPixelAccess_Create()     " );
  }

  clck = oyClock();
  d[0] = d[1] = d[2] = 1.0;
  d[3] = d[4] = d[5] = 0.0;
  if(pixel_access)
  for(i = 0; i < n*1000; ++i)
  {
    if(!error)
      error = oyConversion_GetOnePixel( conv, 0,0, pixel_access );
  }
  clck = oyClock() - clck;
  oyConversion_Release( &conv );
  oyPixelAccess_Release( &pixel_access );

  if( !error  && d[3] != 0.0 )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConversion_GetOnePixel( oyPix. )  %s %.02g %.02g %.02g",
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Pixel"), d[3], d[4], d[5]);
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConversion_GetOnePixel( oyPix. )  %s %.02g %.02g %.02g" ,
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Pixel"), d[3], d[4], d[5]);
  }


  conv = oyConversion_New( 0 );
  oyFilterNode_s * in_node = oyFilterNode_NewWith( "//" OY_TYPE_STD "/root", 0, 0 );
  oyConversion_Set( conv, in_node, 0 );
  oyFilterNode_SetData( in_node, (oyStruct_s*)input, 0, 0 );
  oyFilterNode_s * out_node = oyFilterNode_NewWith( "//" OY_TYPE_STD "/output", 0, 0 );
  /*oyFilterNode_SetData( out_node, (oyStruct_s*)output, 0, 0 );*/
  error = oyFilterNode_Connect( in_node, "//" OY_TYPE_STD "/data",
                                out_node, "//" OY_TYPE_STD "/data", 0 );
  oyConversion_Set( conv, 0, out_node );
  oyConversion_GetNode( conv, OY_OUTPUT );
  plug = oyFilterNode_GetPlug( out, 0 );
  oyFilterNode_Release (&out );

  /* create a very simple pixel iterator as job ticket */
  if(plug)
    pixel_access = oyPixelAccess_Create( 0,0, plug,
                                           oyPIXEL_ACCESS_IMAGE, 0 );
  error  = oyConversion_RunPixels( conv, pixel_access );

  clck = oyClock();
  for(i = 0; i < n*1000; ++i)
  {
    if(!error)
      error = oyConversion_RunPixels( conv, pixel_access );
  }
  clck = oyClock() - clck;

  oyImage_Release( &input );
  oyImage_Release( &output );

  oyPixelAccess_Release( &pixel_access );
  oyConversion_Release( &conv );


  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConversion_RunPixels (2 nodes)    %s",
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Pixel"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConversion_RunPixels (2 nodes)                   " );
  }


  conv = oyConversion_CreateBasicPixelsFromBuffers(
                                        p_in, buf_in, oyDataType_m(buf_type_in),
                                        p_out, buf_out, oyDataType_m(buf_type_out),
                                        0, 1 );
  oyConversion_Release( &conv );

  return result;
}

#include "oyNamedColors_s.h"
oyTESTRESULT_e testNcl2()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;
  int i, error = 0;
  uint32_t icc_profile_flags =oyICCProfileSelectionFlagsFromOptions( OY_CMM_STD,
                                       "//" OY_TYPE_STD "/icc_color", NULL, 0 );
  oyProfile_s * p_cmyk = oyProfile_FromStd( oyEDITING_CMYK, icc_profile_flags, NULL );
  oyNamedColor_s * ncl = 0;
  oyNamedColors_s * colors = oyNamedColors_New(0);

  oyNamedColors_SetPrefix( colors, "test" );
  oyNamedColors_SetSuffix( colors, "color" );

  fprintf(stdout, "\n" );

  double clck = oyClock();
  for( i = 0; i < 10*100; ++i )
  {
    char name[12];
    double lab[3], device[4] = {0.2,0.2,0.1,0.5};

    lab[0] = i*0.001;
    lab[1] = lab[2] = 0.5;
    sprintf( name, "%d", i );

    ncl = oyNamedColor_CreateWithName( name, name, name, NULL, NULL, NULL, 0, p_cmyk, NULL );

    if(!error)
      error = oyNamedColor_SetColorStd( ncl, oyEDITING_LAB, lab, oyDOUBLE, 0, NULL );
    if(!error)
      oyNamedColor_SetChannels( ncl, device, 0 );

    oyNamedColors_MoveIn( colors, &ncl, i );
  }
  clck = oyClock() - clck;

  if( !error )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyNamedColor_CreateWithName()     %s",
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Ncl"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyNamedColor_CreateWithName()            " );
  }

  

  return result;
}

#include "oyRectangle_s_.h"

oyTESTRESULT_e testImagePixel()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;
  uint32_t icc_profile_flags =oyICCProfileSelectionFlagsFromOptions( OY_CMM_STD,
                                       "//" OY_TYPE_STD "/icc_color", NULL, 0 );
  oyProfile_s * p_lab = oyProfile_FromStd( oyEDITING_LAB, icc_profile_flags, NULL );
  oyProfile_s * p_web = oyProfile_FromStd( oyASSUMED_WEB, icc_profile_flags, NULL );
  oyProfile_s /** p_cmyk = oyProfile_FromStd( oyEDITING_CMYK, NULL ),*/
              * p_in, * p_out;
  int error = 0,
      i,n = 10;
  uint16_t buf_16in2x2[12] = {
  20000,20000,20000, 10000,10000,10000,
  0,0,0,             65535,65535,65535
  };
  uint16_t buf_16out2x2[12];
  oyDATATYPE_e buf_type_in = oyUINT16,
               buf_type_out = oyUINT16;
  oyImage_s *input, *output;

  fprintf(stdout, "\n" );

  double clck = oyClock();
  p_in = p_web;
  p_out = p_lab;
  input =oyImage_Create( 2,2, 
                         buf_16in2x2,
                         oyChannels_m(oyProfile_GetChannelsCount(p_in)) |
                          oyDataType_m(buf_type_in),
                         p_in,
                         0 );
  output=oyImage_Create( 2,2, 
                         buf_16out2x2,
                         oyChannels_m(oyProfile_GetChannelsCount(p_out)) |
                          oyDataType_m(buf_type_out),
                         p_out,
                         0 );

  oyFilterPlug_s * plug = 0;
  oyPixelAccess_s * pixel_access = 0;
  oyConversion_s * cc;
  memset( buf_16out2x2, 0, sizeof(uint16_t)*12 );
  cc = oyConversion_CreateBasicPixels( input,output, 0, 0 );
  oyFilterNode_s * out = oyConversion_GetNode( cc, OY_OUTPUT );
  if(cc && out)
    plug = oyFilterNode_GetPlug( oyConversion_GetNode( cc, OY_OUTPUT), 0 );
  else
    error = 1;
  pixel_access = oyPixelAccess_Create( 0,0, plug,
                                           oyPIXEL_ACCESS_IMAGE, 0 );
  oyFilterPlug_Release( &plug );

  error  = oyConversion_RunPixels( cc, pixel_access );
  clck = oyClock();
  for(i = 0; i < n*1000; ++i)
  if(error <= 0)
  {
    error  = oyConversion_RunPixels( cc, pixel_access );
  }
  clck = oyClock() - clck;

  if( !error &&
      /* check unchanged input buffer */
      buf_16in2x2[0]==20000 && buf_16in2x2[1]==20000 && buf_16in2x2[2]==20000 &&
      buf_16in2x2[3]==10000 && buf_16in2x2[4]==10000 && buf_16in2x2[5]==10000 &&
      buf_16in2x2[6]==0 && buf_16in2x2[7]==0 && buf_16in2x2[8]==0 &&
      buf_16in2x2[9]==65535 && buf_16in2x2[10]==65535 &&buf_16in2x2[11]==65535&&
      /* check black and white in lower row with typical *ab of 32896 */
      buf_16out2x2[6]<5000 && buf_16out2x2[7]>20000 && buf_16out2x2[7]<40000 &&
      buf_16out2x2[9]>65000 && buf_16out2x2[10]>20000 && buf_16out2x2[10]<40000
      )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "Plain Image                              %s",
                          oyProfilingToString(4*i,clck/(double)CLOCKS_PER_SEC, "Pixel"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "Plain Image                                        " );
  }

  fprintf(zout, "input:  %d,%d,%d %d,%d,%d\n        %d,%d,%d %d,%d,%d\n",
                  buf_16in2x2[0], buf_16in2x2[1], buf_16in2x2[2],
                  buf_16in2x2[3], buf_16in2x2[4], buf_16in2x2[5],
                  buf_16in2x2[6], buf_16in2x2[7], buf_16in2x2[8],
                  buf_16in2x2[9], buf_16in2x2[10], buf_16in2x2[11] );
  fprintf(zout, "output: %d,%d,%d %d,%d,%d\n        %d,%d,%d %d,%d,%d\n",
                  buf_16out2x2[0], buf_16out2x2[1], buf_16out2x2[2],
                  buf_16out2x2[3], buf_16out2x2[4], buf_16out2x2[5],
                  buf_16out2x2[6], buf_16out2x2[7], buf_16out2x2[8],
                  buf_16out2x2[9], buf_16out2x2[10], buf_16out2x2[11] );



  buf_16in2x2[0]=buf_16in2x2[1]=buf_16in2x2[2]=20000;
  buf_16in2x2[3]=buf_16in2x2[4]=buf_16in2x2[5]=10000;
  buf_16in2x2[6]=buf_16in2x2[7]=buf_16in2x2[8]=0;
  buf_16in2x2[9]=buf_16in2x2[10]=buf_16in2x2[11]=65535;
  memset( buf_16out2x2, 0, sizeof(uint16_t)*12 );
  /* use the lower left source pixel */
  if(pixel_access)
  {
    oyRectangle_s * r = oyPixelAccess_GetOutputROI( pixel_access );
    (*oyRectangle_SetGeo1(r,2)) *= 0.5;
    (*oyRectangle_SetGeo1(r,3)) *= 0.5;
    oyPixelAccess_ChangeRectangle( pixel_access, 0.5,0.5, r );
  }
  clck = oyClock();
  for(i = 0; i < n*1000; ++i)
  if(error <= 0)
  {
    error  = oyConversion_RunPixels( cc, pixel_access );
  }
  clck = oyClock() - clck;

  if( !error &&
      /* input should not change */
      buf_16in2x2[0]==20000 && buf_16in2x2[1]==20000 && buf_16in2x2[2]==20000 &&
      buf_16in2x2[3]==10000 && buf_16in2x2[4]==10000 && buf_16in2x2[5]==10000 &&
      buf_16in2x2[6]==0 && buf_16in2x2[7]==0 && buf_16in2x2[8]==0 &&
      buf_16in2x2[9]==65535 && buf_16in2x2[10]==65535 &&buf_16in2x2[11]==65535&&
      /* the result shall appear in the upper left corner / first pixel */
      buf_16out2x2[0]>65000 && buf_16out2x2[1]>20000 && buf_16out2x2[2]<40000&&
      /* all other buffer pixels shall remain untouched */
      buf_16out2x2[3]==0 && buf_16out2x2[4]==0 && buf_16out2x2[5]==0 &&
      buf_16out2x2[6]==0 && buf_16out2x2[7]==0 && buf_16out2x2[8]==0 &&
      buf_16out2x2[9]==0 && buf_16out2x2[10]==0 && buf_16out2x2[11]==0
      )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "lower right source pixel in 1 pixel RoI  %s",
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Pixel"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "lower right source pixel in 1 pixel RoI            " );
  }

  fprintf(zout, "input:  %d,%d,%d %d,%d,%d\n        %d,%d,%d %d,%d,%d\n",
                  buf_16in2x2[0], buf_16in2x2[1], buf_16in2x2[2],
                  buf_16in2x2[3], buf_16in2x2[4], buf_16in2x2[5],
                  buf_16in2x2[6], buf_16in2x2[7], buf_16in2x2[8],
                  buf_16in2x2[9], buf_16in2x2[10], buf_16in2x2[11] );
  fprintf(zout, "output: %d,%d,%d %d,%d,%d\n        %d,%d,%d %d,%d,%d\n",
                  buf_16out2x2[0], buf_16out2x2[1], buf_16out2x2[2],
                  buf_16out2x2[3], buf_16out2x2[4], buf_16out2x2[5],
                  buf_16out2x2[6], buf_16out2x2[7], buf_16out2x2[8],
                  buf_16out2x2[9], buf_16out2x2[10], buf_16out2x2[11] );


  buf_16in2x2[0]=buf_16in2x2[1]=buf_16in2x2[2]=20000;
  buf_16in2x2[3]=buf_16in2x2[4]=buf_16in2x2[5]=10000;
  buf_16in2x2[6]=buf_16in2x2[7]=buf_16in2x2[8]=0;
  buf_16in2x2[9]=buf_16in2x2[10]=buf_16in2x2[11]=65535;
  memset( buf_16out2x2, 0, sizeof(uint16_t)*12 );
  if(pixel_access)
  {
    oyRectangle_s * r = oyPixelAccess_GetOutputROI( pixel_access );
    oyRectangle_SetGeo(r, 0.5,0.5, 0.5,0.5);
    oyPixelAccess_ChangeRectangle( pixel_access, 0.5,0.5, r );
  }
  clck = oyClock();
  for(i = 0; i < n*1000; ++i)
  if(error <= 0)
  {
    error  = oyConversion_RunPixels( cc, pixel_access );
  }
  clck = oyClock() - clck;

  if( !error &&
      /* input should not change */
      buf_16in2x2[0]==20000 && buf_16in2x2[1]==20000 && buf_16in2x2[2]==20000 &&
      buf_16in2x2[3]==10000 && buf_16in2x2[4]==10000 && buf_16in2x2[5]==10000 &&
      buf_16in2x2[6]==0 && buf_16in2x2[7]==0 && buf_16in2x2[8]==0 &&
      buf_16in2x2[9]==65535 && buf_16in2x2[10]==65535 &&buf_16in2x2[11]==65535&&
      /* the result shall appear in the lower right corner / last pixel */
      buf_16out2x2[9]>65000 && buf_16out2x2[10]>20000&&buf_16out2x2[11]<40000&&
      /* all other buffer pixels shall remain untouched */
      buf_16out2x2[0]==0 && buf_16out2x2[1]==0 && buf_16out2x2[2]==0 &&
      buf_16out2x2[3]==0 && buf_16out2x2[4]==0 && buf_16out2x2[5]==0 &&
      buf_16out2x2[6]==0 && buf_16out2x2[7]==0 && buf_16out2x2[8]==0
      )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "lower right source in lower right output %s",
                          oyProfilingToString(i,clck/(double)CLOCKS_PER_SEC, "Pixel"));
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "lower right source in lower right output           " );
  }

  fprintf(zout, "input:  %d,%d,%d %d,%d,%d\n        %d,%d,%d %d,%d,%d\n",
                  buf_16in2x2[0], buf_16in2x2[1], buf_16in2x2[2],
                  buf_16in2x2[3], buf_16in2x2[4], buf_16in2x2[5],
                  buf_16in2x2[6], buf_16in2x2[7], buf_16in2x2[8],
                  buf_16in2x2[9], buf_16in2x2[10], buf_16in2x2[11] );
  fprintf(zout, "output: %d,%d,%d %d,%d,%d\n        %d,%d,%d %d,%d,%d\n",
                  buf_16out2x2[0], buf_16out2x2[1], buf_16out2x2[2],
                  buf_16out2x2[3], buf_16out2x2[4], buf_16out2x2[5],
                  buf_16out2x2[6], buf_16out2x2[7], buf_16out2x2[8],
                  buf_16out2x2[9], buf_16out2x2[10], buf_16out2x2[11] );


  oyConversion_Release ( &cc );
  oyPixelAccess_Release( &pixel_access );
  oyProfile_Release( &p_lab );
  oyProfile_Release( &p_web );

  oyPixel_t pixel_layout = OY_TYPE_123_16;
  oyPointer channels = 0;
  oyRectangle_s_ roi_ = {oyOBJECT_RECTANGLE_S, 0,0,0,0,0,0,0};
  oyRectangle_s_ a_roi_ = {oyOBJECT_RECTANGLE_S, 0,0,0,0,0,0,0};
  oyRectangle_s * roi = (oyRectangle_s*)&roi_,
                * a_roi = (oyRectangle_s*)&a_roi_;

  oyArray2d_s * a = oyArray2d_Create( channels,
                                      2 * oyToChannels_m(pixel_layout),
                                      2,
                                      oyToDataType_m(pixel_layout),
                                      0 );

  oyRectangle_SetGeo( roi, 0.5,0.5,0.5,0.5 );
  oyRectangle_SetGeo( a_roi, 0.5,0.5,0.5,0.5 );
  buf_16out2x2[9] = buf_16out2x2[10] = buf_16out2x2[11] = 65535;
  error = oyImage_FillArray( output, roi, 0,
                             &a,
                             a_roi, 0 );

  uint16_t ** rows_u16 = (uint16_t**)oyArray2d_GetData(a);
  uint16_t * output_u16 = rows_u16[0];

  if(!error &&
     rows_u16[0] != buf_16out2x2)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyImage_FillArray() keep allocation                 " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyImage_FillArray() keep alloc 0x%lo 0x%lo",
    (unsigned long)rows_u16[0], (unsigned long)buf_16out2x2 );
  }

  if(!error &&
     output_u16[0] == 65535 && output_u16[1] == 65535 && output_u16[2] == 65535)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyImage_FillArray() place array data                " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyImage_FillArray() place array data                " );
  }

  /* set lower right pixel */
  output_u16[0] = output_u16[1] = output_u16[2] = 2;

  fprintf(zout, "output: %d,%d,%d %d,%d,%d\n        %d,%d,%d %d,%d,%d\n",
                  buf_16out2x2[0], buf_16out2x2[1], buf_16out2x2[2],
                  buf_16out2x2[3], buf_16out2x2[4], buf_16out2x2[5],
                  buf_16out2x2[6], buf_16out2x2[7], buf_16out2x2[8],
                  buf_16out2x2[9], buf_16out2x2[10], buf_16out2x2[11] );

  oyRectangle_SetGeo( a_roi, 0.0,0.0,0.5,0.5 );
  error = oyImage_ReadArray( output, roi,
                             a, a_roi );


  rows_u16 = (uint16_t**)oyArray2d_GetData(a);
  if(!error &&
     rows_u16[0] != &buf_16out2x2[9] &&
     buf_16out2x2[9]==2&& buf_16out2x2[10]==2&& buf_16out2x2[11]==2)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyImage_ReadArray( array_roi )                      " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyImage_ReadArray( array_roi )                      " );
  }

  /* set lower right pixel */
  output_u16[0] = output_u16[1] = output_u16[2] = 3;
  error = oyImage_ReadArray( output, roi,
                             a, 0 );


  rows_u16 = (uint16_t**)oyArray2d_GetData(a);
  if(!error &&
     rows_u16[0] != &buf_16out2x2[9] &&
     buf_16out2x2[9]==3&& buf_16out2x2[10]==3&& buf_16out2x2[11]==3)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyImage_ReadArray()                                 " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyImage_ReadArray()                                 " );
  }

  fprintf(zout, "output: %d,%d,%d %d,%d,%d\n        %d,%d,%d %d,%d,%d\n",
                  buf_16out2x2[0], buf_16out2x2[1], buf_16out2x2[2],
                  buf_16out2x2[3], buf_16out2x2[4], buf_16out2x2[5],
                  buf_16out2x2[6], buf_16out2x2[7], buf_16out2x2[8],
                  buf_16out2x2[9], buf_16out2x2[10], buf_16out2x2[11] );

  /* move the focus to the top left */
  oyRectangle_s_ array_roi_pix = {oyOBJECT_RECTANGLE_S,0,0,0, 0,0,3,1};
  oyArray2d_SetFocus( a, (oyRectangle_s*)&array_roi_pix );


  rows_u16 = (uint16_t**)oyArray2d_GetData(a);
  if(!error &&
     rows_u16[0] != output_u16)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyArray2d_SetFocus() change array                   " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyArray2d_SetFocus() change 0x%lo 0x%lo",
    (unsigned long)rows_u16[0], (unsigned long)output_u16 );
  }


  output_u16 = (uint16_t*)rows_u16[0];

  /* set lower right pixel */
  output_u16[0] = output_u16[1] = output_u16[2] = 4;
  error = oyImage_ReadArray( output, roi,
                             a, 0 );
  rows_u16 = (uint16_t**)oyArray2d_GetData(a);
  if(!error &&
     rows_u16[0] != &buf_16out2x2[9] &&
     buf_16out2x2[9]==4&& buf_16out2x2[10]==4&& buf_16out2x2[11]==4)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyArray2d_SetFocus()                                " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyArray2d_SetFocus()                                " );
  }

  oyArray2d_Release( &a );

  oyRectangle_SetGeo( a_roi, 0.5,0.5,0.5,0.5 );
  error = oyImage_FillArray( output, roi, 0,
                             &a, a_roi, 0 );

  rows_u16 = (uint16_t**)oyArray2d_GetData(a);
  if(!error &&
     rows_u16[0] == &buf_16out2x2[9])
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyImage_FillArray() assigment                       " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyImage_FillArray() assigment 0x%lo 0x%lo",
    (unsigned long)rows_u16[0], (unsigned long)&buf_16out2x2[9] );
  }

  oyArray2d_Release( &a );
  oyImage_Release( &input );
  oyImage_Release( &output );

  return result;
}

oyTESTRESULT_e testConversion()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;
  uint32_t icc_profile_flags =oyICCProfileSelectionFlagsFromOptions( OY_CMM_STD,
                                       "//" OY_TYPE_STD "/icc_color", NULL, 0 );
  oyProfile_s * p_lab = oyProfile_FromFile( "compatibleWithAdobeRGB1998.icc", icc_profile_flags, NULL );
  oyProfile_s * p_web = oyProfile_FromStd( oyASSUMED_WEB, icc_profile_flags, NULL );
  oyProfile_s /** p_cmyk = oyProfile_FromStd( oyEDITING_CMYK, NULL ),*/
              * p_in, * p_out;
  uint16_t buf_16in2x2[12] = {
  20000,20000,20000, 10000,10000,10000,
  0,0,0,             65535,65535,65535
  };
  uint16_t buf_16out2x2[12];
  oyDATATYPE_e buf_type_in = oyUINT16,
               buf_type_out = oyUINT16;
  oyImage_s *input, *output;

  fprintf(stdout, "\n" );

  p_in = p_web;
  p_out = p_lab;
  input =oyImage_Create( 2,2, 
                         buf_16in2x2,
                         oyChannels_m(oyProfile_GetChannelsCount(p_in)) |
                          oyDataType_m(buf_type_in),
                         p_in,
                         0 );
  output=oyImage_Create( 2,2, 
                         buf_16out2x2,
                         oyChannels_m(oyProfile_GetChannelsCount(p_out)) |
                          oyDataType_m(buf_type_out),
                         p_out,
                         0 );

  oyOptions_s * options = NULL;

  oyOptions_SetFromText( &options, "////cached", "1", OY_CREATE_NEW );
  oyConversion_s * cc = oyConversion_CreateBasicPixels( input,output, options, 0 );
  oyFilterGraph_s * cc_graph = oyConversion_GetGraph( cc );
  oyFilterNode_s * icc = oyFilterGraph_GetNode( cc_graph, -1, "///icc", 0 );
  oyBlob_s * blob = oyFilterNode_ToBlob( icc, NULL );
  if(blob)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConversion_CreateBasicPixels( \"cached\"=\"1\" )" );
  } else
  { PRINT_SUB( oyTESTRESULT_XFAIL,
    "oyConversion_CreateBasicPixels( \"cached\"=\"1\" )" );
  }

  oyOptions_Release( &options );

  oyOptions_SetFromText( &options, "////context", "lcm2", OY_CREATE_NEW );
  cc = oyConversion_CreateBasicPixels( input,output, options, 0 );
  cc_graph = oyConversion_GetGraph( cc );
  icc = oyFilterGraph_GetNode( cc_graph, -1, "///icc_color", 0 );
  const char * reg = oyFilterNode_GetRegistration( icc );
  
  if(reg && strstr(reg, "lcm2"))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConversion_CreateBasicPixels( \"context\"=\"lcm2\" )" );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConversion_CreateBasicPixels( \"context\"=\"lcm2\" ) %s", oyNoEmptyString_m_(reg) );
  }

  blob = oyFilterNode_ToBlob( icc, NULL );
  if(blob)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyFilterNode_ToBlob( \"lcm2\" )                 " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyFilterNode_ToBlob( \"lcm2\" )                 " );
  }

  oyBlob_Release( &blob );
  oyOptions_Release( &options );

  oyOptions_SetFromText( &options, "////renderer", "lcms", OY_CREATE_NEW );
  oyOptions_SetFromText( &options, "////context", "lcm2", OY_CREATE_NEW );
  cc = oyConversion_CreateBasicPixels( input,output, options, 0 );
  cc_graph = oyConversion_GetGraph( cc );
  icc = oyFilterGraph_GetNode( cc_graph, -1, "///icc_color", 0 );
  reg = oyFilterNode_GetRendererRegistration( icc );

  if(reg && strstr(reg, "lcms"))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConversion_CreateBasicPixels( \"renderer\"=\"lcms\" )" );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConversion_CreateBasicPixels( \"renderer\"=\"lcms\" ) %s", oyNoEmptyString_m_(reg) );
    fprintf( zout, "\tnode reg = %s\n", oyFilterNode_GetRegistration( icc ));
  }

  oyOptions_Release( &options );

  options = oyFilterNode_GetOptions( icc, oyOPTIONATTRIBUTE_ADVANCED );
  oyOptions_SetFromText( &options, "////renderer", "lcm2", OY_CREATE_NEW );
  oyOptions_SetFromText( &options, "////context", "lcms", OY_CREATE_NEW );
  blob = oyFilterNode_ToBlob( icc, NULL );
  oyBlob_Release( &blob );

  reg = oyFilterNode_GetRegistration( icc );
  if(reg && strstr(reg, "lcms"))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyFilterNode_SetContext_( \"context\"=\"lcms\" )  " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyFilterNode_SetContext_( \"context\"=\"lcms\" ) %s  ", oyNoEmptyString_m_(reg) );
  }

  reg = oyFilterNode_GetRendererRegistration( icc );
  if(reg && strstr(reg, "lcm2"))
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyFilterNode_SetContext_( \"renderer\"=\"lcm2\" )" );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyFilterNode_SetContext_( \"renderer\"=\"lcm2\" ) %s", oyNoEmptyString_m_(reg) );
  }

  oyOptions_Release( &options );

  oyImage_Release( &input );
  oyImage_Release( &output );

  return result;
}

oyTESTRESULT_e testCMMlists()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  char ** list = oyGetCMMs( oyCMM_CONTEXT, oyNAME_NAME, 0, malloc );
  int i = 0;

  while(list && list[i])
  {
    fprintf(zout, "  %d: \"%s\" (%s)\n", i, list[i], oyCMMNameToRegistration( list[i], oyCMM_CONTEXT, oyNAME_NAME, 0, malloc ) );
    ++i;
  }

  if(list)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyGetCMMs(oyCMM_CONTEXT, oyNAME_NAME,) fine %d  ", i );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyGetCMMs(oyCMM_CONTEXT, oyNAME_NAME,) failed   " );
  }

  list = oyGetCMMs( oyCMM_CONTEXT, oyNAME_REGISTRATION, 0, malloc );
  i = 0;

  while(list && list[i])
  {
    fprintf(zout, "  %d: \"%s\" (%s)\n", i, list[i], oyCMMRegistrationToName(list[i], oyCMM_CONTEXT, oyNAME_NAME, 0, malloc) );
    ++i;
  }

  if(list)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyGetCMMs(oyCMM_CONTEXT, oyNAME_REGISTRATION,) fine %d", i );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyGetCMMs(oyCMM_CONTEXT, oyNAME_REGISTRATION,) failed " );
  }

  list = oyGetCMMs( oyCMM_CONTEXT, oyNAME_NICK, 0, malloc );
  i = 0;

  while(list && list[i])
  {
    fprintf(zout, "  %d: \"%s\" (%s)\n", i, list[i], oyCMMRegistrationToName(list[i], oyCMM_CONTEXT, oyNAME_PATTERN, 0, malloc) );
    ++i;
  }

  if(list)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyGetCMMs(oyCMM_CONTEXT, oyNAME_NICK,) fine %d        ", i );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyGetCMMs(oyCMM_CONTEXT, oyNAME_NICK,) failed         " );
  }

  int current = -1;
  oyOptionChoicesGet2( oyWIDGET_CMM_CONTEXT, 0, oyNAME_NAME, &i,
                       (const char***)&list, &current );

  if(current != -1)
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyOptionChoicesGet2( 0, current == %s [%d])      ", list[current],
                                                              current );
  } else
  { PRINT_SUB( oyTESTRESULT_XFAIL,
    "oyOptionChoicesGet2( current == ???? ) missed         " );
  }

  oyOptionChoicesFree( oyWIDGET_CMM_CONTEXT, &list, i );


  char * default_cmm = oyGetCMMPattern( oyCMM_CONTEXT, oySOURCE_DATA, malloc );

  if(default_cmm && default_cmm[0])
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyGetCMMPattern( oySOURCE_DATA ) == %s         ", default_cmm );
  } else
  { PRINT_SUB( oyTESTRESULT_XFAIL,
    "oyGetCMMPattern( oySOURCE_DATA ) not set              " );
  }
  if(default_cmm) free(default_cmm);


  default_cmm = oyGetCMMPattern( oyCMM_CONTEXT, oySOURCE_FILTER, malloc );

  if(default_cmm && default_cmm[0])
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyGetCMMPattern( oySOURCE_FILTER )==%s", default_cmm );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyGetCMMPattern( oySOURCE_FILTER ) failed             " );
  }
  if(default_cmm) free(default_cmm);


  default_cmm = oyGetCMMPattern( oyCMM_CONTEXT, 0, malloc );

  if(default_cmm && default_cmm[0])
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyGetCMMPattern( ) == %s             ", default_cmm );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyGetCMMPattern( ) failed                             " );
  }
  if(default_cmm) free(default_cmm);

  return result;
}

#include "oyranos_alpha.h"

oyTESTRESULT_e testConfDomain ()
{
  oyTESTRESULT_e result = oyTESTRESULT_UNKNOWN;

  int error = 0;
  oyConfDomain_s * a = 0, * b = 0;
  oyObject_s object = oyObject_New();
  const char ** texts = 0;
  char       ** domains = 0;
  int i,j,n;
  uint32_t count = 0,
         * rank_list = 0;

  fprintf(stdout, "\n" );

#ifdef __APPLE__
  a = oyConfDomain_FromReg( OY_STD"/config.device.icc_profile.monitor.qarz", 0 );
#else
  a = oyConfDomain_FromReg( OY_STD"/config.device.icc_profile.monitor.oyX1", 0 );
#endif
  error = !a;

  if(!error)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyConfDomain_FromReg() good                     " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyConfDomain_FromReg() failed                   " );
  }

  b = oyConfDomain_Copy( a, object );

  if(!error && b && b != a)
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyConfDomain_Copy good                          " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyConfDomain_Copy failed                        " );
  }

  error = oyConfDomain_Release( &b );

  b = oyConfDomain_Copy( a, 0 );

  if(!error && b && a == b )
  { PRINT_SUB( oyTESTRESULT_SUCCESS, 
    "oyConfDomain_Copy() good                        " );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL, 
    "oyConfDomain_Copy() failed                      " );
  }

  oyConfDomain_Release( &a );
  oyConfDomain_Release( &b );

  error = oyConfigDomainList( "//" OY_TYPE_STD, &domains, &count, &rank_list,
                              malloc );
  if( count )
  { PRINT_SUB( oyTESTRESULT_SUCCESS,
    "oyConfigDomainList \"%s\": %d               ", "//" OY_TYPE_STD "",
                                                    (int)count );
  } else
  { PRINT_SUB( oyTESTRESULT_FAIL,
    "oyConfigDomainList \"%s\": %d               ", "//" OY_TYPE_STD "",
                                                    (int)count );
  }
  for( i = 0; i < (int)count; ++i)
  {
    fprintf( zout, "%d: %s\n", i, domains[i] );
  }
  fprintf( zout, "\n");
  
  for(i = 0; i < (int)count; ++i)
  {
    int text_missed = 0;
    const char * t[3] = {0,0,0};
    const char * nick = domains[i];

    if(strchr(nick, '/'))
      nick = strrchr(nick, '/') + 1;

    a = oyConfDomain_FromReg( domains[i], 0 );
    texts = oyConfDomain_GetTexts( a );
    n = j = 0;
    if(texts)
      while(texts[j]) ++j;

    n = j;
    for(j = 0; j < n; ++j)
    {
      t[oyNAME_NICK] = oyConfDomain_GetText( a, texts[j], oyNAME_NICK );
      t[oyNAME_NAME] = oyConfDomain_GetText( a, texts[j], oyNAME_NAME );
      t[oyNAME_DESCRIPTION] = oyConfDomain_GetText( a, texts[j],
                                                          oyNAME_DESCRIPTION );

      if(!t[oyNAME_NICK])
      {
        fprintf(stdout, "  %d %s:\n    \"%s\" oyNAME_NICK is missed\n", j,
                        nick, texts[j]);
        text_missed = 1;
      }
      if(!t[oyNAME_NAME])
      {
        fprintf(stdout, "  %d %s:\n    \"%s\" oyNAME_NAME is missed\n", j,
                        nick, texts[j]);
        text_missed = 1;
      }
      if(!t[oyNAME_DESCRIPTION])
      {
        fprintf(stdout, "  %d %s:\n    \"%s\" oyNAME_DESCRIPTION is missed\n",j,
                        nick, texts[j]);
        text_missed = 1;
      }
      if(strcmp(texts[j], "name") == 0)
        fprintf(zout,"\"%s\" =\n  \"%s\" \"%s\" \"%s\"\n", texts[j],
                        t[oyNAME_NICK], t[oyNAME_NAME], t[oyNAME_DESCRIPTION]);
    }

    if(!error && n && !text_missed)
    { PRINT_SUB( oyTESTRESULT_SUCCESS, 
      "oyConfDomain_GetTexts() \"%s\" %d good  ", nick, n );
    } else
    { PRINT_SUB( oyTESTRESULT_FAIL, 
      "oyConfDomain_GetTexts() \"%s\" %d failed ", nick, n );
    }

    oyConfDomain_Release( &a );
    fprintf( zout, "----------\n");
  }
  oyStringListRelease_( &domains, count, free );


  return result;
}

static int test_number = 0;
#define TEST_RUN( prog, text ) { \
  if(argc > argpos) { \
      for(i = argpos; i < argc; ++i) \
        if(strstr(text, argv[i]) != 0 || \
           atoi(argv[i]) == test_number ) \
          oyTestRun( prog, text, test_number ); \
  } else if(list) \
    printf( "[%d] %s\n", test_number, text); \
  else \
    oyTestRun( prog, text, test_number ); \
  ++test_number; \
}

int results[oyTESTRESULT_UNKNOWN+1];
char * tests_failed[64];

oyTESTRESULT_e oyTestRun             ( oyTESTRESULT_e    (*test)(void),
                                       const char        * test_name,
                                       int                 number )
{
  oyTESTRESULT_e error = oyTESTRESULT_UNKNOWN;

  fprintf( stdout, "\n________________________________________________________________\n" );
  fprintf(stdout, "Test[%d]: %s ... ", test_number, test_name );

  error = test();

  fprintf(stdout, "\t%s", oyTestResultToString(error));

  if(error == oyTESTRESULT_FAIL)
    tests_failed[results[error]] = (char*)test_name;
  results[error] += 1;

  /* print */
  if(error && error != oyTESTRESULT_XFAIL)
    fprintf(stdout, " !!! ERROR !!!" );
  fprintf(stdout, "\n" );

  return error;
}

/*  main */
int main(int argc, char** argv)
{
  int i, error = 0,
      argpos = 1,
      list = 0;

  if(getenv("OY_DEBUG"))
  {
    int value = atoi(getenv("OY_DEBUG"));
    if(value > 0)
      oy_debug += value;
  }

  /* init */
  for(i = 0; i <= oyTESTRESULT_UNKNOWN; ++i)
    results[i] = 0;

  i = 1; while(i < argc) if( strcmp(argv[i++],"-l") == 0 )
  { ++argpos;
    zout = stderr;
    list = 1;
  }

  i = 1; while(i < argc) if( strcmp(argv[i++],"--silent") == 0 )
  { ++argpos;
    zout = stderr;
  }

  fprintf( zout, "\nOyranos Tests v" OYRANOS_VERSION_NAME
           "  developed: " OYRANOS_DATE
           "\n\n" );

  /* do tests */

  TEST_RUN( testVersion, "Version matching" );
  TEST_RUN( testI18N, "Internationalisation" );
  TEST_RUN( testElektra, "Elektra" );
  TEST_RUN( testStringRun, "String handling" );
  TEST_RUN( testOption, "basic oyOption_s" );
  TEST_RUN( testOptionInt,  "oyOption_s integers" );
  TEST_RUN( testOptionsSet,  "Set oyOptions_s" );
  TEST_RUN( testOptionsCopy,  "Copy oyOptions_s" );
  TEST_RUN( testBlob, "oyBlob_s" );
  TEST_RUN( testSettings, "default oyOptions_s settings" );
  TEST_RUN( testConfDomain, "oyConfDomain_s");
  TEST_RUN( testInterpolation, "Interpolation oyLinInterpolateRampU16" );
  TEST_RUN( testProfile, "Profile handling" );
  TEST_RUN( testProfiles, "Profiles reading" );
  TEST_RUN( testProfileLists, "Profile lists" );
  TEST_RUN( testProofingEffect, "proofing_effect" );
  TEST_RUN( testDeviceLinkProfile, "CMM deviceLink" );
  //TEST_RUN( testMonitor,  "Monitor profiles" );
  //TEST_RUN( testDevices,  "Devices listing" );
  TEST_RUN( testRegistrationMatch,  "Registration matching" );
  TEST_RUN( test_oyTextIccDictMatch,  "IccDict matching" );
  TEST_RUN( testPolicy, "Policy handling" );
  TEST_RUN( testCMMDevicesListing, "CMM devices listing" );
  TEST_RUN( testCMMDevicesDetails, "CMM devices details" );
  TEST_RUN( testCMMRankMap, "rank map handling" );
  TEST_RUN( testCMMMonitorJSON, "monitor JSON" );
  TEST_RUN( testCMMMonitorListing, "CMM monitor listing" );
  TEST_RUN( testCMMMonitorModule, "CMM monitor module" );
  TEST_RUN( testCMMDBListing, "CMM DB listing" );
  TEST_RUN( testCMMmonitorDBmatch, "CMM monitor DB match" );
  TEST_RUN( testCMMsShow, "CMMs show" );
  TEST_RUN( testCMMnmRun, "CMM named color run" );
  TEST_RUN( testNcl2, "named color serialisation" );
  TEST_RUN( testImagePixel, "CMM Image Pixel run" );
  TEST_RUN( testConversion, "CMM selection" );
  TEST_RUN( testCMMlists, "CMMs listing" );

  /* give a summary */
  if(!list)
  {

    fprintf( stdout, "\n################################################################\n" );
    fprintf( stdout, "#                                                              #\n" );
    fprintf( stdout, "#                     Results                                  #\n" );
    for(i = 0; i <= oyTESTRESULT_UNKNOWN; ++i)
      fprintf( stdout, "    Tests with status %s:\t%d\n",
                       oyTestResultToString( (oyTESTRESULT_e)i ), results[i] );

    error = (results[oyTESTRESULT_FAIL] ||
             results[oyTESTRESULT_SYSERROR] ||
             results[oyTESTRESULT_UNKNOWN]
            );

    for(i = 0; i < results[oyTESTRESULT_FAIL]; ++i)
      fprintf( stdout, "    %s: \"%s\"\n",
               oyTestResultToString( oyTESTRESULT_FAIL), tests_failed[i] );

    if(error)
      fprintf( stdout, "    Tests FAILED\n" );
    else
      fprintf( stdout, "    Tests SUCCEEDED\n" );

    fprintf( stdout, "\n    Hint: the '-l' option will list all test names\n" );

  }

  return error;
}

