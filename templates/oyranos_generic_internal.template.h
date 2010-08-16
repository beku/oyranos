{% load gsoc_extensions %}{% include "source_file_header.txt" %}

#ifndef {{ file_name|cut:".h"|upper }}_H
#define {{ file_name|cut:".h"|upper }}_H

#include "oyHash_s .h"
#include "oyStructList_s.h"

oyHash_s *   oyCacheListGetEntry_    ( oyStructList_s    * cache_list,
                                       uint32_t            flags,
                                       const char        * hash_text );


#endif /* {{ file_name|cut:".h"|upper }}_H */
