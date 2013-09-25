/** @file oyList_s_.c

   [Template file inheritance graph]
   +-> oyList_s_.template.c
   |
   +-> BaseList_s_.c
   |
   +-- Base_s_.c

 *  Oyranos is an open source Color Management System
 *
 *  @par Copyright:
 *            2004-2013 (C) Kai-Uwe Behrmann
 *
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            new BSD - see: http://www.opensource.org/licenses/bsd-license.php
 *  @date     2013/09/25
 */



  
#include "oyList_s.h"
#include "oyList_s_.h"





#include "oyObject_s.h"
#include "oyranos_object_internal.h"

#include "oyranos_generic_internal.h"
  


/* Include "List.private_custom_definitions.c" { */
/** Function    oyList_Release__Members
 *  @memberof   oyList_s
 *  @brief      Custom List destructor
 *  @internal
 *
 *  This function will free up all memmory allocated by the
 *  input object. First all object members witch have their
 *  own release method are deallocated. Then the deallocateFunc_
 *  of the oy_ object is used to release the rest of the members
 *  that were allocated with oy_->allocateFunc_.
 *
 *  @param[in]  list  the List object
 *
 *  @version Oyranos: x.x.x
 *  @since   YYYY/MM/DD (Oyranos: x.x.x)
 *  @date    YYYY/MM/DD
 */
void oyList_Release__Members( oyList_s_ * list )
{
}

/** Function    oyList_Init__Members
 *  @memberof   oyList_s
 *  @brief      Custom List constructor
 *  @internal
 *
 *  This function will allocate all memmory for the input object.
 *  For the basic member types this is done using the allocateFunc_
 *  of the attatced (oyObject_s)oy_ object.
 *
 *  @param[in]  list  the List object
 *
 *  @version Oyranos: x.x.x
 *  @since   YYYY/MM/DD (Oyranos: x.x.x)
 *  @date    YYYY/MM/DD
 */
int oyList_Init__Members( oyList_s_ * list )
{
  return 0;
}

/** Function    oyList_Copy__Members
 *  @memberof   oyList_s
 *  @brief      Custom List copy constructor
 *  @internal
 *
 *  This function makes a copy of all values from the input
 *  to the output object. The destination object and all of its
 *  members should already be allocated.
 *
 *  @param[in]   src  the oyList_s_ input object
 *  @param[out]  dst  the output oyList_s_ object
 *
 *  @version Oyranos: x.x.x
 *  @since   YYYY/MM/DD (Oyranos: x.x.x)
 *  @date    YYYY/MM/DD
 */
int oyList_Copy__Members( oyList_s_ * dst, oyList_s_ * src)
{
  return 0;
}

/* } Include "List.private_custom_definitions.c" */



/** @internal
 *  Function oyList_New_
 *  @memberof oyList_s_
 *  @brief   allocate a new oyList_s_  object
 *
 *  @version Oyranos: 
 *  @since   2010/04/26 (Oyranos: 0.1.10)
 *  @date    2010/04/26
 */
oyList_s_ * oyList_New_ ( oyObject_s object )
{
  /* ---- start of common object constructor ----- */
  oyOBJECT_e type = oyOBJECT_LIST_S;
  int error = 0;
  oyObject_s    s_obj = oyObject_NewFrom( object );
  oyList_s_ * s = 0;

  if(s_obj)
    s = (oyList_s_*)s_obj->allocateFunc_(sizeof(oyList_s_));

  if(!s || !s_obj)
  {
    WARNc_S(_("MEM Error."));
    return NULL;
  }

  error = !memset( s, 0, sizeof(oyList_s_) );
  if(error)
    WARNc_S( "memset failed" );

  s->type_ = type;
  s->copy = (oyStruct_Copy_f) oyList_Copy;
  s->release = (oyStruct_Release_f) oyList_Release;

  s->oy_ = s_obj;

  
  /* ---- start of custom List constructor ----- */
  error += !oyObject_SetParent( s_obj, oyOBJECT_LIST_S, (oyPointer)s );
  /* ---- end of custom List constructor ------- */
  
  
  
  
  /* ---- end of common object constructor ------- */
  if(error)
    WARNc_S( "oyObject_SetParent failed" );


  
  s->list_ = oyStructList_Create( s->type_, 0, 0 );


  
  /* ---- start of custom List constructor ----- */
  error += oyList_Init__Members( s );
  /* ---- end of custom List constructor ------- */
  
  
  
  

  if(error)
    WARNc1_S("%d", error);

  return s;
}

/** @internal
 *  Function oyList_Copy__
 *  @memberof oyList_s_
 *  @brief   real copy a List object
 *
 *  @param[in]     list                 List struct object
 *  @param         object              the optional object
 *
 *  @version Oyranos: 
 *  @since   2010/04/26 (Oyranos: 0.1.10)
 *  @date    2010/04/26
 */
oyList_s_ * oyList_Copy__ ( oyList_s_ *list, oyObject_s object )
{
  oyList_s_ *s = 0;
  int error = 0;

  if(!list || !object)
    return s;

  s = (oyList_s_*) oyList_New( object );
  error = !s;

  if(!error) {
    
    /* ---- start of custom List copy constructor ----- */
    error = oyList_Copy__Members( s, list );
    /* ---- end of custom List copy constructor ------- */
    
    
    
    
    
    s->list_ = oyStructList_Copy( list->list_, s->oy_ );

  }

  if(error)
    oyList_Release_( &s );

  return s;
}

/** @internal
 *  Function oyList_Copy_
 *  @memberof oyList_s_
 *  @brief   copy or reference a List object
 *
 *  @param[in]     list                 List struct object
 *  @param         object              the optional object
 *
 *  @version Oyranos: 
 *  @since   2010/04/26 (Oyranos: 0.1.10)
 *  @date    2010/04/26
 */
oyList_s_ * oyList_Copy_ ( oyList_s_ *list, oyObject_s object )
{
  oyList_s_ *s = list;

  if(!list)
    return 0;

  if(list && !object)
  {
    s = list;
    
    oyObject_Copy( s->oy_ );
    return s;
  }

  s = oyList_Copy__( list, object );

  return s;
}
 
/** @internal
 *  Function oyList_Release_
 *  @memberof oyList_s_
 *  @brief   release and possibly deallocate a List list
 *
 *  @param[in,out] list                 List struct object
 *
 *  @version Oyranos: 
 *  @since   2010/04/26 (Oyranos: 0.1.10)
 *  @date    2010/04/26
 */
int oyList_Release_( oyList_s_ **list )
{
  /* ---- start of common object destructor ----- */
  oyList_s_ *s = 0;

  if(!list || !*list)
    return 0;

  s = *list;

  *list = 0;

  if(oyObject_UnRef(s->oy_))
    return 0;
  /* ---- end of common object destructor ------- */

  
  /* ---- start of custom List destructor ----- */
  oyList_Release__Members( s );
  /* ---- end of custom List destructor ------- */
  
  
  
  

  oyStructList_Release( &s->list_ );


  if(s->oy_->deallocateFunc_)
  {
    oyDeAlloc_f deallocateFunc = s->oy_->deallocateFunc_;

    oyObject_Release( &s->oy_ );

    deallocateFunc( s );
  }

  return 0;
}



/* Include "List.private_methods_definitions.c" { */

/* } Include "List.private_methods_definitions.c" */

