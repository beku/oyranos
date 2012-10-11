/** @file oyConnector_s.h

   [Template file inheritance graph]
   +-> oyConnector_s.template.h
   |
   +-> Base_s.h
   |
   +-- oyStruct_s.template.h

 *  Oyranos is an open source Colour Management System
 *
 *  @par Copyright:
 *            2004-2012 (C) Kai-Uwe Behrmann
 *
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            new BSD - see: http://www.opensource.org/licenses/bsd-license.php
 *  @date     2012/10/08
 */



#ifndef OY_CONNECTOR_S_H
#define OY_CONNECTOR_S_H

#ifdef __cplusplus
/*namespace oyranos {*/
  extern "C" {
#endif /* __cplusplus */


  
#include <oyranos_object.h>

typedef struct oyConnector_s oyConnector_s;



#include "oyStruct_s.h"

#include "oyCMMapi5_s_.h"


/* Include "Connector.public.h" { */

/* } Include "Connector.public.h" */


/* Include "Connector.dox" { */
/** @struct  oyConnector_s
 *  @ingroup objects_conversion
 *  @extends oyStruct_s
 *  @brief   A filter connection description structure
 *
 *  This structure holds informations about the connection capabilities.
 *  It holds common structure members of oyFilterPlug_s and oyFilterSocket_s.
 *
 *  To signal a value is not initialised or does not apply, set the according
 *  integer value to -1.
 *
 *  @todo generalise the connector properties
 *
 *  @version Oyranos: 0.3.0
 *  @since   2008/07/26 (Oyranos: 0.1.8)
 *  @date    2011/01/31
 */

/* } Include "Connector.dox" */

struct oyConnector_s {
/* Include "Struct.members.h" { */
oyOBJECT_e           type_;          /**< @private struct type */
oyStruct_Copy_f      copy;           /**< copy function */
oyStruct_Release_f   release;        /**< release function */
oyObject_s           oy_;            /**< @private features name and hash */

/* } Include "Struct.members.h" */
};


OYAPI oyConnector_s* OYEXPORT
  oyConnector_New( oyObject_s object );
OYAPI oyConnector_s* OYEXPORT
  oyConnector_Copy( oyConnector_s *connector, oyObject_s obj );
OYAPI int OYEXPORT
  oyConnector_Release( oyConnector_s **connector );



/* Include "Connector.public_methods_declarations.h" { */
OYAPI int  OYEXPORT
                 oyConnector_SetName ( oyConnector_s     * obj,
                                       const char        * string,
                                       oyNAME_e            type );
OYAPI const char *  OYEXPORT
                oyConnector_GetName ( oyConnector_s     * obj,
                                       oyNAME_e            type );
OYAPI int  OYEXPORT
                 oyConnector_IsPlug  ( oyConnector_s     * obj );
OYAPI int  OYEXPORT
                 oyConnector_SetIsPlug(oyConnector_s     * obj,
                                       int                 is_plug );
OYAPI const char *  OYEXPORT
                oyConnector_GetReg  ( oyConnector_s     * obj );
OYAPI int  OYEXPORT
                 oyConnector_SetReg  ( oyConnector_s     * obj,
                                       const char        * type_registration );

#include "oyFilterPlug_s.h"
#include "oyFilterSocket_s.h"
/** typedef  oyCMMFilterSocket_MatchPlug_f
 *  @brief   verify connectors matching each other
 *  @ingroup module_api
 *  @memberof oyCMMapi5_s
 *
 *  A implementation for images is included in the core function
 *  oyFilterSocket_MatchImagePlug().
 *
 *  @param         socket              a filter socket
 *  @param         plug                a filter plug
 *  @return                            1 on success, otherwise 0
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/04/20 (Oyranos: 0.1.10)
 *  @date    2009/04/20
 */
typedef int          (*oyCMMFilterSocket_MatchPlug_f) (
                                       oyFilterSocket_s  * socket,
                                       oyFilterPlug_s    * plug );

OYAPI int  OYEXPORT
                 oyConnector_SetMatch( oyConnector_s     * obj,
                                       oyCMMFilterSocket_MatchPlug_f func );
OYAPI oyCMMFilterSocket_MatchPlug_f  OYEXPORT
                 oyConnector_GetMatch (
                                       oyConnector_s     * obj );

/* } Include "Connector.public_methods_declarations.h" */


#ifdef __cplusplus
} /* extern "C" */
/*}*/ /* namespace oyranos */
#endif /* __cplusplus */

#endif /* OY_CONNECTOR_S_H */