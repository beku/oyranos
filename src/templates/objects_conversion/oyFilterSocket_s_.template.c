{% extends "Base_s_.c" %}

{% block refCount %}
  {
  uint32_t n = 0;
  int r OY_UNUSED = oyObject_UnRef(s->oy_);

  if(s->node)
    ++n;

  if(s->requesting_plugs_)
    ++n;

  if( r+1 < (int)n )
    WARNcc2_S( s, "reference count below internal references to other object(s): %s %s",
               s->node?"node":"", s->requesting_plugs_?"requesting_plugs_":"" );

  /* referenences from members has to be substracted
   * from this objects ref count */
  if(oyObject_GetRefCount( s->oy_ ) > (int)(n + observer_refs))
    return 0;

  /* ref before oyXXX_Release__Members(), so the
   * oyXXX_Release() is not called twice */
  oyObject_Ref(s->oy_);
  }
{% endblock %}

{% block customDestructor %}
  /* unref after oyXXX_Release__Members() */
  oyObject_UnRef(s->oy_);
{% endblock customDestructor %}

{% block customStaticMessage %}
{{ block.super }}
  if(type == oyNAME_DESCRIPTION)
    sprintf( &text[strlen(text)], "%s", s->relatives_?s->relatives_:"" );
{% endblock %}
