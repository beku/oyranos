/** @file oyranos_cmm_oyra_image.c
 *
 *  Oyranos is an open source Colour Management System 
 *
 *  @par Copyright:
 *            2008-2009 (C) Kai-Uwe Behrmann
 *
 *  @brief    backends for Oyranos
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            new BSD <http://www.opensource.org/licenses/bsd-license.php>
 *  @since    2008/01/02
 */

#include "config.h"
#include "oyranos_alpha.h"
#include "oyranos_cmm.h"
#include "oyranos_cmm_oyra.h"
#include "oyranos_helper.h"
#include "oyranos_icc.h"
#include "oyranos_i18n.h"
#include "oyranos_io.h"
#include "oyranos_definitions.h"
#include "oyranos_texts.h"
#include <iconv.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>  /* UINT32_MAX */

/* OY_IMAGE_ROOT_REGISTRATION */
/* OY_IMAGE_OUTPUT_REGISTRATION */
/* OY_IMAGE_REGIONS_REGISTRATION */

/* ---------------------------------------------------------------------------*/


/** @func    oyraFilter_ImageRegionsCanHandle
 *  @brief   inform about image handling capabilities
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/23 (Oyranos: 0.1.10)
 *  @date    2009/02/23
 */
int    oyraFilter_ImageRegionsCanHandle(oyCMMQUERY_e     type,
                                       uint32_t          value )
{
  int ret = -1;

  switch(type)
  {
    case oyQUERY_OYRANOS_COMPATIBILITY:
         ret = OYRANOS_VERSION; break;
    case oyQUERY_PIXELLAYOUT_CHANNELCOUNT:
         ret = 65535;
         break;
    case oyQUERY_PIXELLAYOUT_DATATYPE:
         switch(value) {
         case oyUINT8:
         case oyUINT16:
         case oyUINT32:
         case oyHALF:
         case oyFLOAT:
         case oyDOUBLE:
              ret = 1; break;
         default:
              ret = 0; break;
         }
         break;
    case oyQUERY_PIXELLAYOUT_SWAP_COLOURCHANNELS:
         ret = 1;
         break;
    case oyQUERY_PIXELLAYOUT_PLANAR:
         ret = 1;
         break;
    case oyQUERY_HDR:
         ret = 1;
         break;
    default: break;
  }

  return ret;
}

oyOptions_s* oyraFilter_ImageRegionsValidateOptions
                                     ( oyFilterCore_s    * filter,
                                       oyOptions_s       * validate,
                                       int                 statical,
                                       uint32_t          * result )
{
  uint32_t error = !filter;

  if(!error)
    error = filter->type_ != oyOBJECT_FILTER_CORE_S;

  *result = error;

  return 0;
}

/** @func    oyraFilterNode_ImageRegionsContextToMem
 *  @brief   implement oyCMMFilter_ContextToMem_f()
 *
 *  Serialise into a Oyranos specific ICC profile containers "Info" tag.
 *  We do not have any binary context to include.
 *  Thus oyFilterNode_TextToInfo_() is fine.
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/23 (Oyranos: 0.1.10)
 *  @date    2009/02/23
 */
oyPointer  oyraFilterNode_ImageRegionsContextToMem (
                                       oyFilterNode_s    * node,
                                       size_t            * size,
                                       oyAlloc_f           allocateFunc )
{
  return oyFilterNode_TextToInfo_( node, size, allocateFunc );
}

/** @func    oyraFilterPlug_ImageRegionsRun
 *  @brief   implement oyCMMFilter_GetNext_f()
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/23 (Oyranos: 0.1.10)
 *  @date    2009/02/23
 */
int      oyraFilterPlug_ImageRegionsRun(oyFilterPlug_s   * requestor_plug,
                                       oyPixelAccess_s   * ticket )
{
  int x = 0, y = 0, n = 0, i;
  int result = 0, error = 0;
  int is_allocated = 0;
  oyPointer * ptr = 0;
  oyFilterSocket_s * socket = requestor_plug->remote_socket_;
  oyFilterNode_s * input_node = 0,
                 * node = socket->node;
  oyImage_s * image = (oyImage_s*)socket->data;
  char key[12];
  oyOption_s * o = 0;
  oyRegion_s * r = 0;
  oyPixelAccess_s * new_ticket = 0;


  x = ticket->start_xy[0];
  y = ticket->start_xy[1];

  result = oyPixelAccess_CalculateNextStartPixel( ticket, requestor_plug);

  if(result != 0)
    return result;

  if(x < image->width &&
     y < image->height &&
     ticket->pixels_n)
  {
    n = ticket->pixels_n;
    if(n == 1)
      ptr = image->getPoint( image, x, y, 0, &is_allocated );

    result = !ptr;

  } else {

    /* Map each matching plug to a new ticket with a corrected region. */
    oyOptions_s * regions = node->filter->options_;

    n = oyFilterNode_EdgeCount( node, 1, OY_FILTEREDGE_CONNECTED );
    if(n != oyOptions_Count( regions ))
      WARNc2_S("plugs!=regions: %d/%d", n,oyOptions_Count( regions ));

    for(i = 0; i < n; ++i)
    {
      sprintf( key, "%d", i );
      o = oyOptions_Find( regions, key );

      if(o && o->value_type == oyVAL_STRUCT)
        r = (oyRegion_s *) o->value->oy_struct;

      new_ticket = oyPixelAccess_Copy( ticket, ticket->oy_ );

      if(r)
        oyRegion_SetByRegion( new_ticket->output_image_roi, r );

      if(oyRegion_CountPoints(  new_ticket->output_image_roi ) > 0)
      {
        /* select node */
        input_node = node->plugs[i]->remote_socket_->node;
        /* adapt the region of interesst to the new image dimensions */
        oyRegion_Trim( new_ticket->output_image_roi, ticket->output_image_roi );
        /* start new call */
        result = input_node->api7_->oyCMMFilterPlug_Run( node->plugs[i],
                                                         new_ticket );
      }
      oyRegion_Release( &r );
      oyOption_Release( &o );
    }
  }

  return result;
}


oyDATATYPE_e oyra_image_data_types[7] = {oyUINT8, oyUINT16, oyUINT32,
                                         oyHALF, oyFLOAT, oyDOUBLE, 0};

oyConnector_s oyra_imageRegions_plug = {
  oyOBJECT_CONNECTOR_S,0,0,0,
  {oyOBJECT_NAME_S, 0,0,0, "Img", "Image", "Image Regions Plug"},
  oyCONNECTOR_SPLITTER, /* connector_type */
  1, /* is_plug == oyFilterPlug_s */
  oyra_image_data_types, /* data_types */
  6, /* data_types_n; elements in data_types array */
  -1, /* max_colour_offset */
  1, /* min_channels_count; */
  255, /* max_channels_count; */
  1, /* min_colour_count; */
  255, /* max_colour_count; */
  0, /* can_planar; can read separated channels */
  1, /* can_interwoven; can read continuous channels */
  1, /* can_swap; can swap colour channels (BGR)*/
  1, /* can_swap_bytes; non host byte order */
  1, /* can_revert; revert 1 -> 0 and 0 -> 1 */
  1, /* can_premultiplied_alpha; */
  1, /* can_nonpremultiplied_alpha; */
  1, /* can_subpixel; understand subpixel order */
  0, /* oyCHANNELTYPE_e    * channel_types; */
  0, /* count in channel_types */
  1, /* id; relative to oyFilter_s, e.g. 1 */
  0  /* is_mandatory; mandatory flag */
};
oyConnector_s *oyra_imageRegions_plugs[2] = {&oyra_imageRegions_plug,0};

oyConnector_s oyra_imageRegions_socket = {
  oyOBJECT_CONNECTOR_S,0,0,0,
  {oyOBJECT_NAME_S, 0,0,0, "Img", "Image", "Image Regions Plug"},
  oyCONNECTOR_IMAGE, /* connector_type */
  0, /* is_plug == oyFilterPlug_s */
  oyra_image_data_types, /* data_types */
  6, /* data_types_n; elements in data_types array */
  -1, /* max_colour_offset */
  1, /* min_channels_count; */
  255, /* max_channels_count; */
  1, /* min_colour_count; */
  255, /* max_colour_count; */
  0, /* can_planar; can read separated channels */
  1, /* can_interwoven; can read continuous channels */
  1, /* can_swap; can swap colour channels (BGR)*/
  1, /* can_swap_bytes; non host byte order */
  1, /* can_revert; revert 1 -> 0 and 0 -> 1 */
  1, /* can_premultiplied_alpha; */
  1, /* can_nonpremultiplied_alpha; */
  1, /* can_subpixel; understand subpixel order */
  0, /* oyCHANNELTYPE_e    * channel_types; */
  0, /* count in channel_types */
  2, /* id; relative to oyFilter_s, e.g. 1 */
  0  /* is_mandatory; mandatory flag */
};
oyConnector_s *oyra_imageRegions_sockets[2] = {&oyra_imageRegions_socket,0};


#define OY_IMAGE_REGIONS_REGISTRATION OY_TOP_INTERNAL OY_SLASH OY_DOMAIN_INTERNAL OY_SLASH "image/regions"
/** @instance oyra_api7
 *  @brief    oyra oyCMMapi7_s implementation
 *
 *  a filter routing the graph to several regions
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/24 (Oyranos: 0.1.10)
 *  @date    2009/02/24
 */
oyCMMapi7_s   oyra_api7_image_regions = {

  oyOBJECT_CMM_API7_S, /* oyStruct_s::type oyOBJECT_CMM_API7_S */
  0,0,0, /* unused oyStruct_s fileds; keep to zero */
  (oyCMMapi_s*) & oyra_api4_image_output_ppm, /* oyCMMapi_s * next */
  
  oyraCMMInit, /* oyCMMInit_f */
  oyraCMMMessageFuncSet, /* oyCMMMessageFuncSet_f */
  oyraFilter_ImageRegionsCanHandle, /* oyCMMCanHandle_f */

  /* registration */
  OY_IMAGE_REGIONS_REGISTRATION,

  CMM_VERSION, /* int32_t version[3] */
  0,   /* id_; keep empty */
  0,   /* api5_; keep empty */

  oyraFilterPlug_ImageRegionsRun, /* oyCMMFilterPlug_Run_f */
  {0}, /* char data_type[8] */

  oyra_imageRegions_plugs,   /* plugs */
  1,   /* plugs_n */
  UINT16_MAX,   /* plugs_last_add */
  oyra_imageRegions_sockets,   /* sockets */
  1,   /* sockets_n */
  0    /* sockets_last_add */
};

/** @instance oyra_api4
 *  @brief    oyra oyCMMapi4_s implementation
 *
 *  a filter routing the graph to several regions
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/24 (Oyranos: 0.1.10)
 *  @date    2009/02/24
 */
oyCMMapi4_s   oyra_api4_image_regions = {

  oyOBJECT_CMM_API4_S, /* oyStruct_s::type oyOBJECT_CMM_API4_S */
  0,0,0, /* unused oyStruct_s fileds; keep to zero */
  (oyCMMapi_s*) & oyra_api7_image_regions, /* oyCMMapi_s * next */
  
  oyraCMMInit, /* oyCMMInit_f */
  oyraCMMMessageFuncSet, /* oyCMMMessageFuncSet_f */
  oyraFilter_ImageRegionsCanHandle, /* oyCMMCanHandle_f */

  /* registration */
  OY_IMAGE_REGIONS_REGISTRATION,

  CMM_VERSION, /* int32_t version[3] */
  0,   /* id_; keep empty */
  0,   /* api5_; keep empty */

  oyraFilter_ImageRootValidateOptions, /* oyCMMFilter_ValidateOptions_f */
  oyraWidgetEvent, /* oyWidgetEvent_f */

  oyraFilterNode_ImageRootContextToMem, /* oyCMMFilterNode_ContextToMem_f */
  0, /* oyCMMFilterNode_ContextToMem_f oyCMMFilterNode_ContextToMem */
  {0}, /* char context_type[8] */

  {oyOBJECT_NAME_S, 0,0,0, "regions", "Regions", "Regions Splitter Object"}, /* name; translatable, eg "scale" "image scaling" "..." */
  "Graph/Regions[out]", /* category */
  0,   /* options */
  0    /* opts_ui_ */
};


/* ---------------------------------------------------------------------------*/


/** @func    oyraFilter_ImageRootCanHandle
 *  @brief   inform about image handling capabilities
 *
 *  @version Oyranos: 0.1.8
 *  @since   2008/07/10 (Oyranos: 0.1.8)
 *  @date    2008/07/10
 */
int    oyraFilter_ImageRootCanHandle ( oyCMMQUERY_e      type,
                                       uint32_t          value )
{
  int ret = -1;

  switch(type)
  {
    case oyQUERY_OYRANOS_COMPATIBILITY:
         ret = OYRANOS_VERSION; break;
    case oyQUERY_PIXELLAYOUT_CHANNELCOUNT:
         ret = 65535;
         break;
    case oyQUERY_PIXELLAYOUT_DATATYPE:
         switch(value) {
         case oyUINT8:
         case oyUINT16:
         case oyUINT32:
         case oyFLOAT:
         case oyDOUBLE:
              ret = 1; break;
         case oyHALF:
         default:
              ret = 0; break;
         }
         break;
    case oyQUERY_PIXELLAYOUT_SWAP_COLOURCHANNELS:
         ret = 1;
         break;
    case oyQUERY_PIXELLAYOUT_PLANAR:
         ret = 1;
         break;
    case oyQUERY_HDR:
         ret = 1;
         break;
    default: break;
  }

  return ret;
}

oyOptions_s* oyraFilter_ImageRootValidateOptions
                                     ( oyFilterCore_s    * filter,
                                       oyOptions_s       * validate,
                                       int                 statical,
                                       uint32_t          * result )
{
  uint32_t error = !filter;

  if(!error)
    error = filter->type_ != oyOBJECT_FILTER_CORE_S;

#if 0
  if(!error)
    if(filter->image_ && filter->image_->layout_)
    {
      oyDATATYPE_e data_type = oyToDataType_m( filter->image_->layout_[0] );
      if(!(data_type == oyUINT8 ||
           data_type == oyUINT16 ||
           data_type == oyUINT32 ||
           data_type == oyHALF ||
           data_type == oyFLOAT ||
           data_type == oyDOUBLE
                                   ))
        error = 1;
    }
#endif

  *result = error;

  return 0;
}

/** @func    oyraFilterNode_ImageRootContextToMem
 *  @brief   implement oyCMMFilter_ContextToMem_f()
 *
 *  Serialise into a Oyranos specific ICC profile containers "Info" tag.
 *  We do not have any binary context to include.
 *  Thus oyFilterNode_TextToInfo_() is fine.
 *
 *  @version Oyranos: 0.1.8
 *  @since   2008/07/17 (Oyranos: 0.1.8)
 *  @date    2008/07/18
 */
oyPointer  oyraFilterNode_ImageRootContextToMem (
                                       oyFilterNode_s    * node,
                                       size_t            * size,
                                       oyAlloc_f           allocateFunc )
{
  return oyFilterNode_TextToInfo_( node, size, allocateFunc );
}

/** @func    oyraFilterPlug_ImageRootRun
 *  @brief   implement oyCMMFilter_GetNext_f()
 *
 *  @version Oyranos: 0.1.8
 *  @since   2008/07/10 (Oyranos: 0.1.8)
 *  @date    2008/10/05
 */
int      oyraFilterPlug_ImageRootRun ( oyFilterPlug_s    * requestor_plug,
                                       oyPixelAccess_s   * ticket )
{
  int x = 0, y = 0, n = 0;
  int result = 0, error = 0;
  int is_allocated = 0;
  oyPointer * ptr = 0;
  oyFilterSocket_s * socket = requestor_plug->remote_socket_;
  oyImage_s * image = (oyImage_s*)socket->data;

  x = ticket->start_xy[0];
  y = ticket->start_xy[1];

  result = oyPixelAccess_CalculateNextStartPixel( ticket, requestor_plug);

  if(result != 0)
    return result;

  if(x < image->width &&
     y < image->height &&
     ticket->pixels_n)
  {
    n = ticket->pixels_n;
    if(n == 1)
      ptr = image->getPoint( image, x, y, 0, &is_allocated );

    result = !ptr;

  } else {

    /* adapt the region of interesst to the new image dimensions */
    oyRegion_s * new_roi = oyRegion_NewFrom( ticket->output_image_roi, 0 );
    double correct = ticket->output_image->width / (double) image->width;
    new_roi->width *= correct;
    new_roi->height *= correct;
    error = oyImage_FillArray( image, new_roi, 1, &ticket->array, 0 );
  }

  return result;
}


oyConnector_s oyra_imageRoot_connector = {
  oyOBJECT_CONNECTOR_S,0,0,0,
  {oyOBJECT_NAME_S, 0,0,0, "Img", "Image", "Image Socket"},
  oyCONNECTOR_IMAGE, /* connector_type */
  0, /* is_plug == oyFilterPlug_s */
  oyra_image_data_types, /* data_types */
  6, /* data_types_n; elements in data_types array */
  -1, /* max_colour_offset */
  1, /* min_channels_count; */
  255, /* max_channels_count; */
  1, /* min_colour_count; */
  255, /* max_colour_count; */
  0, /* can_planar; can read separated channels */
  1, /* can_interwoven; can read continuous channels */
  0, /* can_swap; can swap colour channels (BGR)*/
  0, /* can_swap_bytes; non host byte order */
  0, /* can_revert; revert 1 -> 0 and 0 -> 1 */
  1, /* can_premultiplied_alpha; */
  1, /* can_nonpremultiplied_alpha; */
  0, /* can_subpixel; understand subpixel order */
  0, /* oyCHANNELTYPE_e    * channel_types; */
  0, /* count in channel_types */
  1, /* id; relative to oyFilter_s, e.g. 1 */
  0  /* is_mandatory; mandatory flag */
};
oyConnector_s * oyra_imageRoot_connectors[2] = {&oyra_imageRoot_connector,0};


#define OY_IMAGE_ROOT_REGISTRATION OY_TOP_INTERNAL OY_SLASH OY_DOMAIN_INTERNAL OY_SLASH "image/root"
/** @instance oyra_api7
 *  @brief    oyra oyCMMapi7_s implementation
 *
 *  a filter providing a source image
 *
 *  @version Oyranos: 0.1.10
 *  @since   2008/12/27 (Oyranos: 0.1.10)
 *  @date    2008/12/27
 */
oyCMMapi7_s   oyra_api7_image_root = {

  oyOBJECT_CMM_API7_S, /* oyStruct_s::type oyOBJECT_CMM_API7_S */
  0,0,0, /* unused oyStruct_s fileds; keep to zero */
  (oyCMMapi_s*) & oyra_api4_image_output, /* oyCMMapi_s * next */
  
  oyraCMMInit, /* oyCMMInit_f */
  oyraCMMMessageFuncSet, /* oyCMMMessageFuncSet_f */
  oyraFilter_ImageRootCanHandle, /* oyCMMCanHandle_f */

  /* registration */
  OY_IMAGE_ROOT_REGISTRATION,

  CMM_VERSION, /* int32_t version[3] */
  0,   /* id_; keep empty */
  0,   /* api5_; keep empty */

  oyraFilterPlug_ImageRootRun, /* oyCMMFilterPlug_Run_f */
  {0}, /* char data_type[8] */

  0,   /* plugs */
  0,   /* plugs_n */
  0,   /* plugs_last_add */
  oyra_imageRoot_connectors,   /* sockets */
  1,   /* sockets_n */
  0    /* sockets_last_add */
};

/** @instance oyra_api4
 *  @brief    oyra oyCMMapi4_s implementation
 *
 *  a filter providing a source image
 *
 *  @version Oyranos: 0.1.8
 *  @since   2008/02/08 (Oyranos: 0.1.8)
 *  @date    2008/06/26
 */
oyCMMapi4_s   oyra_api4_image_root = {

  oyOBJECT_CMM_API4_S, /* oyStruct_s::type oyOBJECT_CMM_API4_S */
  0,0,0, /* unused oyStruct_s fileds; keep to zero */
  (oyCMMapi_s*) & oyra_api7_image_root, /* oyCMMapi_s * next */
  
  oyraCMMInit, /* oyCMMInit_f */
  oyraCMMMessageFuncSet, /* oyCMMMessageFuncSet_f */
  oyraFilter_ImageRootCanHandle, /* oyCMMCanHandle_f */

  /* registration */
  OY_IMAGE_ROOT_REGISTRATION,

  CMM_VERSION, /* int32_t version[3] */
  0,   /* id_; keep empty */
  0,   /* api5_; keep empty */

  oyraFilter_ImageRootValidateOptions, /* oyCMMFilter_ValidateOptions_f */
  oyraWidgetEvent, /* oyWidgetEvent_f */

  oyraFilterNode_ImageRootContextToMem, /* oyCMMFilterNode_ContextToMem_f */
  0, /* oyCMMFilterNode_ContextToMem_f oyCMMFilterNode_ContextToMem */
  {0}, /* char context_type[8] */

  {oyOBJECT_NAME_S, 0,0,0, "image", "Image", "Image Filter Object"}, /* name; translatable, eg "scale" "image scaling" "..." */
  "Image/Simple Image[in]", /* category */
  0,   /* options */
  0    /* opts_ui_ */
};



/* ---------------------------------------------------------------------------*/

oyConnector_s oyra_imageOutput_connector = {
  oyOBJECT_CONNECTOR_S,0,0,0,
  {oyOBJECT_NAME_S, 0,0,0, "Img", "Image", "Image Plug"},
  oyCONNECTOR_IMAGE, /* connector_type */
  1, /* is_plug == oyFilterPlug_s */
  oyra_image_data_types,
  6, /* data_types_n; elements in data_types array */
  -1, /* max_colour_offset */
  1, /* min_channels_count; */
  255, /* max_channels_count; */
  1, /* min_colour_count; */
  255, /* max_colour_count; */
  0, /* can_planar; can read separated channels */
  1, /* can_interwoven; can read continuous channels */
  0, /* can_swap; can swap colour channels (BGR)*/
  0, /* can_swap_bytes; non host byte order */
  0, /* can_revert; revert 1 -> 0 and 0 -> 1 */
  1, /* can_premultiplied_alpha; */
  1, /* can_nonpremultiplied_alpha; */
  0, /* can_subpixel; understand subpixel order */
  0, /* oyCHANNELTYPE_e    * channel_types; */
  0, /* count in channel_types */
  1, /* id; relative to oyFilter_s, e.g. 1 */
  0  /* is_mandatory; mandatory flag */
};
oyConnector_s* oyra_imageOutput_connectors[2] = {&oyra_imageOutput_connector,0};


/** @func    oyraFilter_ImageOutputRun
 *  @brief   implement oyCMMFilter_GetNext_f()
 *
 *  @version Oyranos: 0.1.8
 *  @since   2008/07/19 (Oyranos: 0.1.8)
 *  @date    2008/10/03
 */
int      oyraFilterPlug_ImageOutputRun(oyFilterPlug_s    * requestor_plug,
                                       oyPixelAccess_s   * ticket )
{
  oyFilterSocket_s * socket = requestor_plug->remote_socket_;
  oyFilterNode_s * node = 0;
  int result = 0;

  node = socket->node;

  /* to reuse the requestor_plug is a exception for the starting request */
  result = node->api7_->oyCMMFilterPlug_Run( requestor_plug, ticket );

  return result;
}


#define OY_IMAGE_OUTPUT_REGISTRATION OY_TOP_INTERNAL OY_SLASH OY_DOMAIN_INTERNAL OY_SLASH "image/output"
/** @instance oyra_api7
 *  @brief    oyra oyCMMapi7_s implementation
 *
 *  a filter providing a target image
 *
 *  @version Oyranos: 0.1.10
 *  @since   2008/12/27 (Oyranos: 0.1.10)
 *  @date    2008/12/27
 */
oyCMMapi7_s   oyra_api7_image_output = {

  oyOBJECT_CMM_API7_S, /* oyStruct_s::type oyOBJECT_CMM_API7_S */
  0,0,0, /* unused oyStruct_s fileds; keep to zero */
  (oyCMMapi_s*) & oyra_api4_image_regions, /* oyCMMapi_s * next */
  
  oyraCMMInit, /* oyCMMInit_f */
  oyraCMMMessageFuncSet, /* oyCMMMessageFuncSet_f */
  oyraFilter_ImageRootCanHandle, /* oyCMMCanHandle_f */

  /* registration */
  OY_IMAGE_OUTPUT_REGISTRATION,

  CMM_VERSION, /* int32_t version[3] */
  0,   /* id_; keep empty */
  0,   /* api5_; keep empty */

  oyraFilterPlug_ImageOutputRun, /* oyCMMFilterPlug_Run_f */
  {0}, /* char data_type[8] */

  oyra_imageOutput_connectors,   /* plugs */
  1,   /* plugs_n */
  0,   /* plugs_last_add */
  0,   /* sockets */
  0,   /* sockets_n */
  0    /* sockets_last_add */
};

/** @instance oyra_api4
 *  @brief    oyra oyCMMapi4_s implementation
 *
 *  a filter providing a target image
 *
 *  @version Oyranos: 0.1.8
 *  @since   2008/07/19 (Oyranos: 0.1.8)
 *  @date    2008/07/19
 */
oyCMMapi4_s   oyra_api4_image_output = {

  oyOBJECT_CMM_API4_S, /* oyStruct_s::type oyOBJECT_CMM_API4_S */
  0,0,0, /* unused oyStruct_s fileds; keep to zero */
  (oyCMMapi_s*) & oyra_api7_image_output, /* oyCMMapi_s * next */
  
  oyraCMMInit, /* oyCMMInit_f */
  oyraCMMMessageFuncSet, /* oyCMMMessageFuncSet_f */
  oyraFilter_ImageRootCanHandle, /* oyCMMCanHandle_f */

  /* registration */
  OY_IMAGE_OUTPUT_REGISTRATION,

  CMM_VERSION, /* int32_t version[3] */
  0,   /* id_; keep empty */
  0,   /* api5_; keep empty */

  oyraFilter_ImageRootValidateOptions, /* oyCMMFilter_ValidateOptions_f */
  oyraWidgetEvent, /* oyWidgetEvent_f */

  oyraFilterNode_ImageRootContextToMem, /* oyCMMFilterNode_ContextToMem_f */
  0, /* oyCMMFilterNode_ContextToMem_f oyCMMFilterNode_ContextToMem */
  {0}, /* char context_type[8] */

  {oyOBJECT_NAME_S, 0,0,0, "image_out", "Image[out]", "Output Image Filter Object"}, /* name; translatable, eg "scale" "image scaling" "..." */
  "Image/Simple Image[out]", /* category */
  0,   /* options */
  0   /* opts_ui_ */
};

/* ---------------------------------------------------------------------------*/



