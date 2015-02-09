/** @file oyOptions_s_.h

   [Template file inheritance graph]
   +-> oyOptions_s_.template.h
   |
   +-> BaseList_s_.h
   |
   +-- Base_s_.h

 *  Oyranos is an open source Color Management System
 *
 *  @par Copyright:
 *            2004-2015 (C) Kai-Uwe Behrmann
 *
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            new BSD - see: http://www.opensource.org/licenses/bsd-license.php
 */


#ifndef OY_OPTIONS_S__H
#define OY_OPTIONS_S__H

#ifdef __cplusplus
/*namespace oyranos {*/
  extern "C" {
#endif /* __cplusplus */


#define oyOptionsPriv_m( var ) ((oyOptions_s_*) (var))

typedef struct oyOptions_s_ oyOptions_s_;



#include <libxml/parser.h>
  
#include <oyranos_object.h>

#include "oyStructList_s.h"
  

#include "oyOptions_s.h"

/* Include "Options.private.h" { */

/* } Include "Options.private.h" */

/** @internal
 *  @struct   oyOptions_s_
 *  @brief      generic Options
 *  @ingroup  objects_value
 *  @extends  oyStruct_s
 */
struct oyOptions_s_ {

/* Include "Struct.members.h" { */
const  oyOBJECT_e    type_;          /**< The struct type tells Oyranos how to interprete hidden fields. @brief Type of object */
oyStruct_Copy_f      copy;           /**< @brief Copy function */
oyStruct_Release_f   release;        /**< @brief Release function */
oyObject_s           oy_;            /**< Features name and hash. Do not change during object life time. @brief Oyranos internal object */

/* } Include "Struct.members.h" */


/* Include "BaseList_s_.h" { */
  oyStructList_s * list_;              /**< the list data */
/* } Include "BaseList_s_.h" */

};



oyOptions_s_*
  oyOptions_New_( oyObject_s object );
oyOptions_s_*
  oyOptions_Copy_( oyOptions_s_ *options, oyObject_s object);
oyOptions_s_*
  oyOptions_Copy__( oyOptions_s_ *options, oyObject_s object);
int
  oyOptions_Release_( oyOptions_s_ **options );

#if 0
/* FIXME Are these needed at all?? */
int
           oyOptions_MoveIn_         ( oyOptions_s_      * list,
                                       oyOption_s       ** ptr,
                                       int                 pos );
int
           oyOptions_ReleaseAt_      ( oyOptions_s_      * list,
                                       int                 pos );
oyOption_s *
           oyOptions_Get_            ( oyOptions_s_      * list,
                                       int                 pos );
int
           oyOptions_Count_          ( oyOptions_s_      * list );
#endif



/* Include "Options.private_methods_declarations.h" { */
void           oyOptions_ParseXML_   ( oyOptions_s_      * s,
                                       char            *** texts,
                                       int               * texts_n,
                                       xmlDocPtr           doc,
                                       xmlNodePtr          cur );

/* } Include "Options.private_methods_declarations.h" */




#ifdef __cplusplus
} /* extern "C" */
/*}*/ /* namespace oyranos */
#endif /* __cplusplus */

#endif /* OY_OPTIONS_S__H */
