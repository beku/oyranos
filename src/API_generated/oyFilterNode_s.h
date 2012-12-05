/** @file oyFilterNode_s.h

   [Template file inheritance graph]
   +-> oyFilterNode_s.template.h
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



#ifndef OY_FILTER_NODE_S_H
#define OY_FILTER_NODE_S_H

#ifdef __cplusplus
/*namespace oyranos {*/
  extern "C" {
#endif /* __cplusplus */


  
#include <oyranos_object.h>

typedef struct oyFilterNode_s oyFilterNode_s;

#include "oyConnector_s.h"
#include "oyFilterCore_s.h"
#include "oyFilterPlug_s.h"
#include "oyFilterSocket_s.h"
#include "oyPixelAccess_s.h"
#include "oyPointer_s.h"


#include "oyStruct_s.h"



/* Include "FilterNode.public.h" { */

/* } Include "FilterNode.public.h" */


/* Include "FilterNode.dox" { */
/** @struct  oyFilterNode_s
 *  @ingroup objects_conversion
 *  @extends oyStruct_s
 *  @brief   A FilterNode object
 *  @internal
 *
 *  Filter nodes chain filters into a oyConversion_s graph. The filter nodes
 *  use plugs and sockets for creating connections. Each plug can only connect
 *  to one socket.
 \dot
digraph G {
  bgcolor="transparent";
  node[ shape=plaintext, fontname=Helvetica, fontsize=10 ];
  a [label=<
<table border="0" cellborder="1" cellspacing="4">
  <tr> <td>oyFilterCore_s A</td>
      <td bgcolor="red" width="10" port="s"> socket </td>
  </tr>
</table>>
  ]
  b [label=<
<table border="0" cellborder="1" cellspacing="4">
  <tr><td bgcolor="lightblue" width="10" port="p"> plug </td>
      <td>oyFilterCore_s B</td>
  </tr>
</table>>
  ]

  b:p->a:s [arrowtail=crow, arrowhead=box, constraint=false];

  subgraph cluster_0 {
    color=gray;
    label="FilterNode A";
    a;
  }
  subgraph cluster_1 {
    color=gray;
    label="FilterNode B";
    b;
  }
}
 \enddot
 *
 *  This object provides support for separation of options from chaining.
 *  So it will be possible to implement options changing, which can affect
 *  the same filter instance in different graphs.
 *
 *  A oyFilterNode_s can have various oyFilterPlug_s ' to obtain data from
 *  different sources. The required number is described in the oyCMMapi4_s 
 *  structure, which is part of oyFilterCore_s.
 \dot
digraph G {
  bgcolor="transparent";
  rankdir=LR
  node [shape=record, fontname=Helvetica, fontsize=10, style="rounded"];
  edge [fontname=Helvetica, fontsize=10];

  b [ label="{<plug> | Filter Node 2 |<socket>}"];
  c [ label="{<plug> | Filter Node 3 |<socket>}"];
  d [ label="{<plug> 2| Filter Node 4 |<socket>}"];

  b:socket -> d:plug [arrowtail=normal, arrowhead=none];
  c:socket -> d:plug [arrowtail=normal, arrowhead=none];
}
 \enddot
 *
 *  oyFilterSocket_s is designed to accept arbitrary numbers of connections 
 *  to allow for viewing on a filters data output or observe its state changes.
 \dot
digraph G {
  bgcolor="transparent";
  rankdir=LR
  node [shape=record, fontname=Helvetica, fontsize=10, style="rounded"];
  edge [fontname=Helvetica, fontsize=10];

  a [ label="{<plug> | Filter Node 1 |<socket>}"];
  b [ label="{<plug> 1| Filter Node 2 |<socket>}"];
  c [ label="{<plug> 1| Filter Node 3 |<socket>}"];
  d [ label="{<plug> 1| Filter Node 4 |<socket>}"];
  e [ label="{<plug> 1| Filter Node 5 |<socket>}"];

  a:socket -> b:plug [arrowtail=normal, arrowhead=none];
  a:socket -> c:plug [arrowtail=normal, arrowhead=none];
  a:socket -> d:plug [arrowtail=normal, arrowhead=none];
  a:socket -> e:plug [arrowtail=normal, arrowhead=none];
}
 \enddot
 *
 *  @version Oyranos: 0.1.9
 *  @since   2008/07/08 (Oyranos: 0.1.8)
 *  @date    2008/12/16
 */

/* } Include "FilterNode.dox" */

struct oyFilterNode_s {
/* Include "Struct.members.h" { */
oyOBJECT_e           type_;          /**< @private struct type */
oyStruct_Copy_f      copy;           /**< copy function */
oyStruct_Release_f   release;        /**< release function */
oyObject_s           oy_;            /**< @private features name and hash */

/* } Include "Struct.members.h" */
};


OYAPI oyFilterNode_s* OYEXPORT
  oyFilterNode_New( oyObject_s object );
OYAPI oyFilterNode_s* OYEXPORT
  oyFilterNode_Copy( oyFilterNode_s *filternode, oyObject_s obj );
OYAPI int OYEXPORT
  oyFilterNode_Release( oyFilterNode_s **filternode );



/* Include "FilterNode.public_methods_declarations.h" { */
OYAPI int  OYEXPORT
                 oyCMMptr_ConvertData (oyPointer_s       * cmm_ptr,
                                       oyPointer_s       * cmm_ptr_out,
                                       oyFilterNode_s    * node );
OYAPI int  OYEXPORT
                 oyFilterNode_Connect (oyFilterNode_s    * input,
                                       const char        * socket_nick,
                                       oyFilterNode_s    * output,
                                       const char        * plug_nick,
                                       int                 flags );
OYAPI int  OYEXPORT
               oyFilterNode_ConnectorMatch (
                                       oyFilterNode_s    * node_first,
                                       int                 pos_first,
                                       oyFilterPlug_s    * plug );
OYAPI oyFilterNode_s *  OYEXPORT
                oyFilterNode_Create   (oyFilterCore_s    * filter,
                                       oyObject_s          object );
OYAPI oyStruct_s *  OYEXPORT
                oyFilterNode_GetData  (oyFilterNode_s    * node,
                                       int                 socket_pos );
OYAPI int  OYEXPORT
                 oyFilterNode_SetData (oyFilterNode_s    * node,
                                       oyStruct_s        * data,
                                       int                 socket_pos,
                                       oyObject_s        * object );
OYAPI int  OYEXPORT
                 oyFilterNode_Disconnect (
                                       oyFilterNode_s    * node,
                                       int                 pos );
OYAPI int  OYEXPORT
                 oyFilterNode_EdgeCount (
                                       oyFilterNode_s    * node,
                                       int                 input,
                                       int                 flags );
OYAPI int  OYEXPORT
               oyFilterNode_GetConnectorPos (
                                       oyFilterNode_s    * node,
                                       int                 is_input,
                                       const char        * pattern,
                                       int                 nth_of_type,
                                       int                 flags );
OYAPI int  OYEXPORT
               oyFilterNode_GetId    ( oyFilterNode_s    * node );
OYAPI oyFilterPlug_s * OYEXPORT
               oyFilterNode_GetPlug  ( oyFilterNode_s    * node,
                                       int                 pos );
OYAPI int  OYEXPORT
                oyFilterNode_Run     ( oyFilterNode_s    * node,
                                       oyFilterPlug_s *    plug,
                                       oyPixelAccess_s *   ticket );
OYAPI oyFilterSocket_s * OYEXPORT
               oyFilterNode_GetSocket( oyFilterNode_s    * node,
                                       int                 pos );
OYAPI oyFilterNode_s * OYEXPORT
               oyFilterNode_GetSocketNode (
                                       oyFilterNode_s    * node,
                                       int                 pos,
                                       int                 plugs_pos );
OYAPI int OYEXPORT
               oyFilterNode_CountSocketNodes (
                                       oyFilterNode_s    * node,
                                       int                 pos,
                                       int                 flags );
OYAPI oyFilterNode_s * OYEXPORT
               oyFilterNode_GetPlugNode ( 
                                       oyFilterNode_s    * node,
                                       int                 pos );
OYAPI const char *  OYEXPORT
                oyFilterNode_GetText  ( oyFilterNode_s    * node,
                                       oyNAME_e            name_type );
OYAPI oyFilterNode_s *  OYEXPORT
                oyFilterNode_NewWith (
                                       const char        * registration,
                                       oyOptions_s       * options,
                                       oyObject_s          object );
OYAPI oyOptions_s *  OYEXPORT
                oyFilterNode_GetOptions (
                                       oyFilterNode_s    * node,
                                       int                 flags );
OYAPI oyConnector_s * OYEXPORT
               oyFilterNode_ShowConnector (
                                       oyFilterNode_s    * node,
                                       int                 as_pos,
                                       int                 plug );
OYAPI int  OYEXPORT
                 oyFilterNode_GetUi  ( oyFilterNode_s     * node,
                                       char              ** ui_text,
                                       char             *** namespaces,
                                       oyAlloc_f            allocateFunc );
OYAPI oyOptions_s *  OYEXPORT
                 oyFilterNode_GetTags( oyFilterNode_s     * node );
OYAPI oyFilterCore_s *  OYEXPORT
                 oyFilterNode_GetCore( oyFilterNode_s     * node );
OYAPI const char *  OYEXPORT
                 oyFilterNode_GetRegistration
                                     ( oyFilterNode_s     * node );
OYAPI const char *  OYEXPORT
                 oyFilterNode_GetRelatives
                                     ( oyFilterNode_s     * node );
OYAPI const char *  OYEXPORT
                 oyFilterNode_GetModuleName
                                     ( oyFilterNode_s     * node );
OYAPI oyPointer_s *  OYEXPORT
                 oyFilterNode_GetModuleData(
                                       oyFilterNode_s     * node );
OYAPI int  OYEXPORT
                 oyFilterNode_SetModuleData(
                                       oyFilterNode_s     * node,
                                       oyPointer_s        * data );

/* } Include "FilterNode.public_methods_declarations.h" */


#ifdef __cplusplus
} /* extern "C" */
/*}*/ /* namespace oyranos */
#endif /* __cplusplus */

#endif /* OY_FILTER_NODE_S_H */