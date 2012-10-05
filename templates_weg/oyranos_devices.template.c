{% include "source_file_header.txt" %}

#include <string.h>

#include <oyranos_helper.h>
#include <oyranos_icc.h>

#include "oyranos_alpha.h"
#include "oyranos_devices.h"
#include "oyranos_devices_internal.h"
#include "oyranos_object_internal.h"
#include "oyConfig_s_.h"
#include "oyOption_s_.h"
#include "oyOptions_s_.h"
#include "oyProfiles_s.h"

#include "oyjl/oyjl_tree.h"


/** \addtogroup devices_handling Device API
 *
 *  Devices are a special form of configurations. Their access is grouped
 *  for effective performance. Known devices are queried with
 *  oyDevicesGet(). oyConfigDomainList() provides a list of known device
 *  modules.
 *  A single device can be obtained by oyDeviceGet(). The \a
 *  device_type argument defaults to OY_TYPE_STD and can be omitted for this
 *  group. The \a device_class argument specifies a subgroup, e.g. 
 *  "monitor".
 *
 *  All other functions return a handle to the device. With this handle it is
 *  possible to get informations (oyDeviceGetInfo()), query it's current,
 *  possibly remote profile (oyDeviceAskProfile2()) or typical used get a 
 *  profile with fallbacks including the DB through (oyDeviceGetProfile()), 
 *  set the profile persistent (oyDeviceSetProfile()) or query the persistent
 *  stored profile (oyDeviceProfileFromDB()).
 *
 *  @{
 */



/** Function oyDevicesGet
 *  @brief   get all devices matching to a device class and type
 *
 *  @verbatim
    // "list" all monitors
    oyConfig_s * monitors = 0;
    int error = oyDevicesGet( 0, "monitor", 0, &monitors );
    // see how many are included
    int n = oyConfigs_Count( monitors );
    // release them
    oyConfigs_Release( &monitors );
    @endverbatim
 *
 *  For obtaining expensive "properties" informations at once, add the according
 *  option.
 *  @verbatim
    // get all monitors the expensive way
    oyConfig_s * monitors = 0;
    oyOptions_s * options = oyOptions_New( 0 );
    int error = 0;

    error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/command",
                                   "properties", OY_CREATE_NEW );
    error = oyDevicesGet( 0, "monitor", 0, &monitors );
    oyOptions_Release( &options );

    // see how many are included
    int n = oyConfigs_Count( monitors );

    // release them
    oyConfigs_Release( &monitors );
    @endverbatim
 *
 *  @param[in]     device_type         the device type ::oyFILTER_REG_TYPE,
 *                                     defaults to OY_TYPE_STD (optional)
 *  @param[in]     device_class        the device class, e.g. "monitor",
 *                                     ::oyFILTER_REG_APPLICATION
 *  @param[in]     options             options for the device
 *  @param[out]    devices             the found devices
 *  @return                            0 - good, >= 1 - error, <= -1 unknown
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/02 (Oyranos: 0.1.10)
 *  @date    2009/02/02
 */
OYAPI int  OYEXPORT
           oyDevicesGet              ( const char        * device_type,
                                       const char        * device_class,
                                       oyOptions_s       * options,
                                       oyConfigs_s      ** devices )
{
  int error = !device_class || !device_class[0];
  static char * num = 0;

  if(error > 0)
  {
    WARNc_S( "Argument(s) incorrect. Giving up" );
    return error;
  }

  if(!num)
    oyAllocHelper_m_( num, char, 80, 0, error = 1; return error );

  /** 1. obtain detailed and expensive device informations */

  if(!options)
  {
    options = oyOptions_New( 0 );
    /** 1.1 add "list" call to module arguments */
    error = oyOptions_SetDeviceTextKey_( (oyOptions_s_*)options, device_type,
                                             device_class,
                                             "command", "list" );
  }

  /** 1.2 ask each module */
  if(error <= 0)
    error = oyConfigs_FromDeviceClass( device_type, device_class,
                                           options, devices, 0 );


  return error;
}

/** Function oyDeviceGet
 *  @brief   ask a module for device informations or other direct calls
 *
 *  @verbatim
    oyConfig_s * device = 0;
    int error = oyDeviceGet( 0, "monitor", ":0.0", 0, &device );
    oyConfig_Release( &device );
    @endverbatim
 *
 *  @verbatim
    // pass empty options to the module to get a usage message
    oyOptions_s * options = oyOptions_New( 0 );
    oyDeviceGet( OY_TYPE_STD, "monitor", ":0.0", options, 0 );
    @endverbatim
 *
 *  @param[in]     device_type         the device type, e.g. OY_TYPE_STD,
 *                                     defaults to OY_TYPE_STD (optional)
 *  @param[in]     device_class        registration ::oyFILTER_REG_APPLICATION
 *                                     part, e.g. "monitor", mandatory
 *  @param[in]     device_name         the device name as returned by
 *                                     oyConfigs_FromPattern_f, mandatory,
                                       ::oyFILTER_REG_OPTION
 *  @param[in]     options             options to pass to the module, for zero
 *                                     the verbose and expensive "properties"
 *                                     call is assumed
 *  @param[out]    device              the returned device
 *  @return                            error
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/01/28 (Oyranos: 0.1.10)
 *  @date    2009/02/09
 */
OYAPI int  OYEXPORT
           oyDeviceGet               ( const char        * device_type,
                                       const char        * device_class,
                                       const char        * device_name,
                                       oyOptions_s       * options,
                                       oyConfig_s       ** device )
{
  int error = !device_name || !device_name[0] ||
              !device_class || !device_class[0];
  oyConfigs_s * devices = 0;
  oyConfig_s * s = 0;

  if(error > 0)
  {
    WARNc2_S( "No device_name/device_class argument provided. Give up: %s/%s",
              oyNoEmptyString_m_(device_name),
              oyNoEmptyString_m_(device_class) );
    return 0;
  }

  /** 1. obtain basic device informations */

  if(!options)
  {
    options = oyOptions_New( 0 );
    error = !options;
    /** 1.1 add "list" call to module arguments */
    if(error <= 0)
    error = oyOptions_SetDeviceTextKey_( (oyOptions_s_*)options, device_type,
                                             device_class,
                                             "command", "list" );
  }

  /** 1.1.2 set device filter */
  if(error <= 0)
    error = oyOptions_SetDeviceTextKey_( (oyOptions_s_*)options, device_type,
                                             device_class,
                                             "device_name",device_name);

  /** 2. get the device */
  error = oyConfigs_FromDeviceClass( device_type, device_class,
                                         options, &devices, 0 );

  s = oyConfigs_Get( devices, 0 );

  oyConfigs_Release( &devices );


  /** 3. check for success of device detection */
  error = !s;
  if(error)
    DBG_NUM2_S( "%s: \"%s\"", _("Could not open device"), device_name );

  if(device)
    *device = s;
  else
    oyConfig_Release( &s );

  return error;
}

/** Function oyDeviceBackendCall
 *  @brief   get device answere from options
 *
 *  @param[in]     device          the device
 *  @param[in]     options             options for the device
 *  @return                            error
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/02 (Oyranos: 0.1.10)
 *  @date    2009/08/25
 */
OYAPI int  OYEXPORT
               oyDeviceBackendCall   ( oyConfig_s        * device,
                                       oyOptions_s       * options )
{
  int error = !device,
      l_error = 0;
  oyConfigs_s * devices = 0;
  oyConfig_s * s = device;
  int new_options = 0;

  oyCheckType__m( oyOBJECT_CONFIG_S, return 1 )

  if(!options)
  {
    /** 1. obtain detailed and expensive device informations for a
     *     zero options argument through the "properties" command. */
    options = oyOptions_New( 0 );
    l_error = !options; OY_ERR
    /** 1.1 add "properties" call to module arguments */
    if(error <= 0)
    l_error = oyOptions_SetRegistrationTextKey_( oyOptionsPriv_m(options),
                                                 oyConfigPriv_m(device)->registration,
                                                 "command", "properties" ); OY_ERR
    new_options = 1;
  }

  devices = oyConfigs_New( 0 );
  error = !devices;
  
  if(error <= 0)
  {
    /* Keep a reference to config as devices will later be released. */
    s = oyConfig_Copy( device, 0 );

    oyConfigs_MoveIn( devices, &device, -1 );
  }

  /** 3. talk to the module */
  l_error = oyConfigs_Modify( devices, options ); OY_ERR

  oyConfigs_Release( &devices );
  if(new_options)
    oyOptions_Release( &options );

  return error;
}

icProfileClassSignature oyDeviceSigGet(oyConfig_s        * device )
{
  icProfileClassSignature deviceSignature = 0;
  if(oyFilterRegistrationMatch( oyConfigPriv_m(device)->registration, "monitor", 0 ))
    deviceSignature = icSigDisplayClass;
  else if(oyFilterRegistrationMatch( oyConfigPriv_m(device)->registration, "scanner", 0 ))
    deviceSignature = icSigInputClass;
  else if(oyFilterRegistrationMatch( oyConfigPriv_m(device)->registration, "raw-image", 0 ))
    deviceSignature = icSigInputClass;
  else if(oyFilterRegistrationMatch( oyConfigPriv_m(device)->registration, "printer", 0 ))
    deviceSignature = icSigOutputClass;

  return deviceSignature;
}

/** Function oyDeviceSetup
 *  @brief   activate the device using the stored configuration
 *
 *  @param[in]     device              the device
 *  @return                            error
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/01/30 (Oyranos: 0.1.10)
 *  @date    2009/02/07
 */
OYAPI int  OYEXPORT
         oyDeviceSetup               ( oyConfig_s        * device )
{
  int error = !device;
  oyOptions_s * options = 0;
  oyProfile_s * p = 0;
  char * profile_name = 0,
       * profile_name_temp = 0;
  const char * device_name = 0;
  oyConfig_s * s = device;
  oyOption_s * o;

  oyCheckType__m( oyOBJECT_CONFIG_S, return 1 )

  {
    /* 1. ask for the profile the device is setup with */
    error = oyDeviceAskProfile2( device, 0, &p );
    if(p)
    {
      oyProfile_Release( &p );
      /** We ignore a device, which already has its profile setup. */
      return error;
    }

    /* 2. query the full device information */
    error = oyDeviceProfileFromDB( device, &profile_name, 0 );

    /* 2.1 select best match to device from installed profiles */
    if(!profile_name)
    {
      int size;
      oyProfile_s * profile = 0;
      oyProfiles_s * patterns = 0, * iccs = 0;
      icProfileClassSignature device_signature = oyDeviceSigGet(device);
      int32_t * rank_list = 0;
      double clck;

      profile = oyProfile_FromSignature( device_signature, oySIGNATURE_CLASS, 0 );
      oyProfiles_MoveIn( patterns, &profile, -1 );

      clck = oyClock();
      iccs = oyProfiles_Create( patterns, 0 );
      clck = oyClock() - clck;
      DBG_NUM1_S("oyProfiles_Create(): %g", clck/1000000.0 );
      oyProfiles_Release( &patterns );

      size = oyProfiles_Count(iccs);
      oyAllocHelper_m_( rank_list, int32_t, oyProfiles_Count(iccs), 0, error = 1; return error );
      if(error <= 0)
      {
        clck = oyClock();
        oyProfiles_DeviceRank( iccs, device, rank_list );
        clck = oyClock() - clck;
        DBG_NUM1_S("oyProfiles_DeviceRank(): %g", clck/1000000.0 );
      }
      if(error <= 0 && size && rank_list[0] > 0)
      {
        p = oyProfiles_Get( iccs, 0 );
        profile_name = oyStringCopy_( oyProfile_GetFileName(p, -1),
                                      oyAllocateFunc_ );
        WARNc1_S( "implicitely selected %s", oyNoEmptyString_m_(profile_name) );
        if(oy_debug > 1)
        {
          int i, n = oyProfiles_Count( iccs );
          const char * fn;
          oyProfile_Release( &p );
          for(i = 0; i < n; ++i)
          {
            p = oyProfiles_Get( iccs, i );
            fn = oyProfile_GetFileName(p, -1);
            if(rank_list[i])
              DBG_NUM2_S("%d: %s", rank_list[i], fn);
          }
        }
        oyFree_m_( rank_list );
      }

      oyProfile_Release( &p );
      oyProfiles_Release( &iccs );
    }


    if(!profile_name)
    {
      oyOptions_s * fallback = oyOptions_New( 0 );
      error = oyOptions_SetRegistrationTextKey_( oyOptionsPriv_m(fallback),
                                                 oyConfigPriv_m(device)->registration,
                                                 "icc_profile.fallback","true");
      /* 2.2.1 try fallback for rescue */
      error = oyDeviceAskProfile2( device, fallback, &p );
      oyOptions_Release( &fallback );
      if(p)
      {
        profile_name = oyStringCopy_( oyProfile_GetFileName(p, -1),
                                      oyAllocateFunc_ );
        if(!profile_name)
        {
          char * data = 0;
          size_t size = 0;
          data = oyProfile_GetMem( p, &size, 0, oyAllocateFunc_ );
          if(data && size)
          {
            char * fn = 0;
            const char * t = 0;
            STRING_ADD( fn, OY_USERCOLORDATA OY_SLASH OY_ICCDIRNAME OY_SLASH );
            STRING_ADD( fn, "devices" OY_SLASH );
            STRING_ADD( fn, oyICCDeviceClassDescription(
                             oyProfile_GetSignature( p, oySIGNATURE_CLASS ) ) );
            STRING_ADD( fn, OY_SLASH );
            if((t = oyProfile_GetText( p, oyNAME_DESCRIPTION )) != 0)
              STRING_ADD( fn, t );
            STRING_ADD( fn, ".icc" );

            error = oyWriteMemToFile_ ( fn, data, size );
            if(!error)
              profile_name = fn;
            else
              error = oyWriteMemToFile2_( "oyranos_tmp.icc", data, size,
                                        OY_FILE_NAME_SEARCH | OY_FILE_TEMP_DIR,
                                        &profile_name_temp, oyAllocateFunc_ );
          } else
          {
            error = 1;
            WARNc1_S( "%s",_("Could not open profile") );
          }

          if(profile_name_temp)
            profile_name = profile_name_temp;
          else if( !profile_name )
          {
            error = 1;
            WARNc2_S("%s: \"%s\"(oyranos_tmp.icc)",_("Could not write to file"),
                      oyNoEmptyString_m_(profile_name_temp) );
          }
        }
      }

      if(!profile_name)
        return error;
    }

    /* 2.3 get device_name */
    device_name = oyConfig_FindString( device, "device_name", 0);

    /* 3. setup the device through the module */
    error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/command",
                                   "setup", OY_CREATE_NEW );
    error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/device_name",
                                   device_name, OY_CREATE_NEW );
    error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/profile_name",
                                   profile_name, OY_CREATE_NEW );
    /* 3.1 send the query to a module */
    error = oyDeviceBackendCall( device, options );

    /* 3.2 check if the module has used that profile and complete do that if needed */
    if(!oyConfig_Has( device, "icc_profile" ))
    {
      int has = 0;
#define OY_DOMAIN OY_TOP_SHARED OY_SLASH OY_DOMAIN_INTERNAL OY_SLASH OY_TYPE_STD
      o = oyOption_FromRegistration( OY_DOMAIN OY_SLASH "icc_profile", 0 );

      p = oyProfile_FromFile( profile_name, 0,0 );

      if(p)
      {
        has = 1;
        error = oyOption_StructMoveIn( o, (oyStruct_s**) &p );
      }
      else
      /** Warn on not found profile. */
      {
        oyMessageFunc_p( oyMSG_ERROR,(oyStruct_s*)device,
                       OY_DBG_FORMAT_"\n\t%s: \"%s\"\n\t%s\n", OY_DBG_ARGS_,
                _("Could not open ICC profile"), profile_name,
                _("install in the OpenIccDirectory icc path") );
      }

      if(has)
        oyOptions_Set( oyConfigPriv_m(device)->data, o, -1, 0 );
      oyOption_Release( &o );
      oyProfile_Release( &p );
    }

    if(profile_name_temp)
      oyRemoveFile_( profile_name_temp );
    profile_name_temp = 0;
    oyOptions_Release( &options );
    if(profile_name)
      oyFree_m_( profile_name );
  }

  return error;
}

/** Function oyDeviceUnset
 *  @brief   unset the device profile
 *
 *  The function solely calls \a unset in the module, e.g. unset graphic card
 *  luts and server stored profile. So pretty all device/server side 
 *  informatin should go away. \n
 *
 *  @param         device          the device
 *  @return                            error
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/12 (Oyranos: 0.1.10)
 *  @date    2009/02/12
 */
int      oyDeviceUnset               ( oyConfig_s        * device )
{
  int error = !device;
  oyOptions_s * options = 0;
  char * profile_name = 0;
  const char * device_name = 0;
  oyConfig_s * s = device;

  oyCheckType__m( oyOBJECT_CONFIG_S, return 1 )

  {
    /* 1. query the full device information */
    error = oyDeviceProfileFromDB( device, &profile_name, 0 );

    /* 1.1 get device_name */
    device_name = oyConfig_FindString( device, "device_name", 0);

    /* 2. unset the device through the module */
    /** 2.1 set a general request */
    error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/command",
                                   "unset", OY_CREATE_NEW );
    error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/device_name",
                                   device_name, OY_CREATE_NEW );

    /** 2.2 send the query to a module */
#ifdef UNHIDE_CMM
    error = oyConfigs_FromDomain( oyConfigPriv_m(device)->registration, options, 0, 0 );
#else
    error = 1;
#endif

    oyOptions_Release( &options );
    /* 3.1 send the query to a module */
    error = oyDeviceBackendCall( device, options );

    oyOptions_Release( &options );
    if(profile_name)
      oyFree_m_( profile_name );
  }

  return error;
}

/** Function oyDeviceGetInfo
 *  @brief   get all devices matching to a device class and type
 *
 *  @verbatim
    // print all properties
    int error = oyDeviceGetInfo( device, oyNAME_DESCRIPTION, 0, &text,
                                     malloc );
    char * list = text, * tmp = 0, * line = malloc(128);
    int even = 1;

        tmp = list;
        while(list && list[0])
        {
          snprintf( line, 128, "%s", list );
          if(strchr( line, '\n' ))
          {
            tmp = strchr( line, '\n' );
            tmp[0] = 0;
          }
          if(even)
            printf( "%s\n", line );
          else
            printf( "  %s\n", line );
          list = strchr( list, '\n' );
          if(list) ++list;
          even = !even;
        }

    if(line) free(line);
    if(text) free(text);
    @endverbatim
 *
 *  To obtain a certain single pice of information you do not need 
 *  oyDeviceGetInfo. See the following example:
 *  @verbatim
    char * device_name = ":0.0"; // a typical device
    char * text = 0;
    oyConfig_s * device = 0;
    oyOptions_s * options = 0;
    int error = 0;

    // tell the module with the "properties" call to add all informations
    error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD
                                   "/config/command",
                                   "properties", OY_CREATE_NEW );

    oyDeviceGet( OY_TYPE_STD, "monitor", device_name, options, &device );
    text = oyConfig_FindString( device, "manufacturer", 0 );
    @endverbatim
 *
 *  @param[in]     device          the device
 *  @param[in]     type                influences the info_text output
 *                                     - oyNAME_NAME - a short one line text,
 *                                     - oyNAME_NICK - one word,
 *                                     - oyNAME_DESCRIPTION - expensive text,
 *                                     even lines contain the property key name,
 *                                     odd lines contain the value,
 *                                     lines are separated by newline '\\n'
 *  @param[in]     options             defaults to command=properties
 *  @param[out]    info_text           the text
 *  @param[in]     allocateFunc        the user allocator for info_text
 *  @return                            0 - good, 1 >= error, -1 <= issue(s)
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/02 (Oyranos: 0.1.10)
 *  @date    2009/03/27
 */
OYAPI int  OYEXPORT
           oyDeviceGetInfo           ( oyConfig_s        * device,
                                       oyNAME_e            type,
                                       oyOptions_s       * options,
                                       char             ** info_text,
                                       oyAlloc_f           allocateFunc )
{
  int error = !device || !info_text;
  oyConfig_s_ * device_ = (oyConfig_s_*)device;
  oyOption_s * o = 0;
  oyConfig_s * config = 0;
  const char * tmp = 0;
  static char * num = 0;
  char * text = 0, * t = 0;
  int i, n,
      own_options = 0;
  oyConfig_s * s = device;

  oyCheckType__m( oyOBJECT_CONFIG_S, return 1 )

  if(error > 0)
  {
    WARNc_S( "Argument(s) incorrect. Giving up" );
    return error;
  }

  if(!allocateFunc)
    allocateFunc = oyAllocateFunc_;

  if(!options)
    own_options = 1;

  if(type == oyNAME_NICK)
  {
    tmp = oyOptions_FindString( device_->backend_core,"device_name", 0 );
    *info_text = oyStringCopy_( tmp, allocateFunc );
    return error;
  }

  if(type == oyNAME_DESCRIPTION)
  {
    /* get expensive infos */
    if(oyOptions_Count( device_->backend_core ) < 2)
    {
      error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/command",
                                     "properties", OY_CREATE_NEW );

      if(error <= 0)
        error = oyDeviceBackendCall( device, options );
    }

    if(error <= 0)
    {
      n = oyOptions_Count( device_->backend_core );
      for( i = 0; i < n; ++i )
      {
        o = oyOptions_Get( device_->backend_core, i );
        
        STRING_ADD( text, oyStrrchr_( oyOption_GetRegistration(o),
                          OY_SLASH_C ) + 1 );
        STRING_ADD( text, ":\n" );
        t = oyOption_GetValueText(o,oyAllocateFunc_);
        if(t)
        {
          STRING_ADD( text, t );
          oyDeAllocateFunc_(t); t = 0;
        }
        STRING_ADD( text, "\n" );

        oyOption_Release( &o );
      }
    }
    *info_text = oyStringCopy_( text, allocateFunc );
    oyFree_m_(text);
    return error;
  }


  if(!num)
    oyAllocHelper_m_( num, char, 80, 0, error = 1; return error );

  if(!options)
  {
    options = oyOptions_New( 0 );

    error = !options;
  }

  if(error <= 0)
  {
    /* add "list" call to module arguments */
    error = oyOptions_SetRegistrationTextKey_( (oyOptions_s_*)options,
                                               device_->registration,
                                               "command", "list" );
  }

  if(error <= 0)
  {
    if(type == oyNAME_NAME)
    error = oyOptions_SetRegistrationTextKey_( (oyOptions_s_*)options,
                                               device_->registration,
                                               "oyNAME_NAME", "true" );
  }


  /** 1.2 ask each module */
  if(error <= 0)
    error = oyDeviceBackendCall( device, options );

  if(error <= 0 && device_->backend_core)
  {
    /** 1.2.1 add device_name to the string list */
    if(type == oyNAME_NICK)
      tmp = oyOptions_FindString( device_->backend_core,"device_name",0);
    else if(type == oyNAME_NAME)
      tmp = oyOptions_FindString( device_->data, "oyNAME_NAME", 0 );
    else if(type == oyNAME_DESCRIPTION)
      tmp = oyOptions_FindString( device_->data, "oyNAME_DESCRIPTION", 0 );
  }

  *info_text = oyStringCopy_( tmp, allocateFunc );

  if(own_options)
    oyOptions_Release( &options );
  oyConfig_Release( &config );

  return error;
}

/** Function oyDeviceGetProfile
 *  @brief   order a device profile
 *
 *  This function is designed to satisfy most users as it tries to deliver
 *  a profile all the time. 
 *  Following code can almost allways expect some profile to go with.
 *  It tries hard to get a current profile or set the system up and retry or
 *  get at least one basic profile.
 *
 *  For a basic and thus weaker call to the device use
 *  oyDeviceAskProfile2() instead.
 *
 *  @param         device              the device
 *  @param         options             options passed to the backend
 *  @param         profile             the device's ICC profile
 *  @return                            error
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/08 (Oyranos: 0.1.10)
 *  @date    2009/02/09
 */
OYAPI int  OYEXPORT
           oyDeviceGetProfile        ( oyConfig_s        * device,
                                       oyOptions_s       * options,
                                       oyProfile_s      ** profile )
{
  int error = !device,
      l_error = 0;
  oyConfig_s * s = device;

  oyCheckType__m( oyOBJECT_CONFIG_S, return 1 )


  l_error = oyDeviceAskProfile2( device, options, profile ); OY_ERR

  /** This function does a device setup in case no profile is delivered
   *  by the according module. */
  if(error != 0 && !*profile)
    error = oyDeviceSetup( device );

  if(error <= 0) 
    l_error = oyDeviceAskProfile2( device, options, profile ); OY_ERR

  /** As a last means oyASSUMED_WEB is delivered. */
  if(!*profile)
  {
    *profile = oyProfile_FromStd( oyASSUMED_WEB, 0 );
    if(error == 0)
      error = -1;
  }

  return error;
}

/** Function oyDeviceAskProfile2
 *  @brief   ask for the device profile
 *
 *  Ask for a profile associated with the device. A device capable to
 *  hold a profile. Only the held profile will be checked and returned.
 *  In case this profile is not found a "icc_profile" of oyVAL_STRUCT should be
 *  included.
 *
 *  The device might not be able to hold a profile, then just the DB profile
 *  will be returned from here without an issue. For interessted users, the
 *  source of the profile keeps transparent, as it can be checked if the
 *  device contains a "icc_profile" option which contains a oyProfile_s object.
 *
 *  @param[in]     device              the device
 *  @param[in]     options             additional options
 *  @param[out]    profile             the device's ICC profile
 *  @return                            0 - good, 1 >= error, -1 <= issue(s)
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/10 (Oyranos: 0.1.10)
 *  @date    2009/12/10
 */
OYAPI int  OYEXPORT
           oyDeviceAskProfile2       ( oyConfig_s        * device,
                                       oyOptions_s       * options,
                                       oyProfile_s      ** profile )
{
  int error = !device;
  oyOption_s * o = 0;
  oyConfig_s * s = device;
  int own_options = 0;
  oyProfile_s * p = 0;

  oyCheckType__m( oyOBJECT_CONFIG_S, return 1 )


  if(!options)
  {
    options = oyOptions_New( 0 );
    own_options = 1;
    error = !options;
  }

  if(error <= 0)
  {
    /* add "list" call to module arguments */
    error = oyOptions_SetRegistrationTextKey_( (oyOptions_s_*)options,
                                               oyConfigPriv_m(device)->registration,
                                               "command", "list" );
  }

  if(error <= 0)
  {
    error = oyOptions_SetRegistrationTextKey_( (oyOptions_s_*)options,
                                                 oyConfigPriv_m(device)->registration,
                                                 "icc_profile", "true" );
  }

  if(error <= 0)
    error = oyDeviceBackendCall( device, options );

  /* The backend shows with the existence of the "icc_profile" response that it
   * can handle device profiles through the driver. */
  if(error <= 0)
    o = oyConfig_Find( device, "icc_profile" );

  p = (oyProfile_s*) oyOption_GetStruct( o, oyOBJECT_PROFILE_S );
  if(oyProfile_GetSignature( p, oySIGNATURE_MAGIC ) == icMagicNumber)
    *profile = p;
  else if(!error)
    error = -1;
  p = 0;

  /* The backend can not handle device driver profiles. Switch back to DB. */
  if(error <= 0 && !(*profile) && !o)
  {
    char * profile_name = 0;
    oyDeviceProfileFromDB( device, &profile_name, 0 );
    if(profile_name)
    {
      *profile = oyProfile_FromFile( profile_name, 0,0 );
      oyDeAllocateFunc_( profile_name );
    }
  }

  if(own_options)
    oyOptions_Release( &options );
  oyOption_Release( &o );

  return error;
}

/** Function oyDeviceSetProfile
 *  @brief   set the device profile
 *
 *  The function will lookup the monitor in the Oyranos device database
 *  and stores the given profile there.
 *
 *  To set a new profile und update the device please call the following
 *  sequence:
 *  @verbatim
    // store new settings in the Oyranos data base
    oyDeviceSetProfile( device, profile );
    // remove any device entries
    oyDeviceUnset( device );
    // update the device from the newly added Oyranos data base settings
    oyDeviceSetup( device );
    @endverbatim
 *
 *  @param         device              the device
 *  @param         profile_name        the device's ICC profile or zero to
 *                                     unset
 *  @return                            error
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/07 (Oyranos: 0.1.10)
 *  @date    2009/02/12
 */
int      oyDeviceSetProfile          ( oyConfig_s        * device,
                                       const char        * profile_name )
{
  int error = !device || !profile_name || !profile_name[0];
  oyOption_s * od = 0;
  oyOptions_s * options = 0;
  oyConfigs_s * configs = 0;
  oyConfig_s * config = 0,
             * device_tmp = 0;
  oyProfile_s * p = 0;
  int i, j, n, j_n, equal;
  char * d_opt = 0;
  const char * device_name = 0,
             * o_val = 0,
             * d_val = 0;
  oyConfig_s * s = device;

  oyCheckType__m( oyOBJECT_CONFIG_S, return 1 )

  if(error > 0)
  {
    WARNc1_S( "No profile argument provided. Give up. %s",
              oyNoEmptyString_m_(profile_name) );
    return error;
  }


  /** 1. obtain detailed and expensive device informations */
  if(oyOptions_Count( oyConfigPriv_m(device)->backend_core ) < 2)
  { 
    /** 1.1 add "properties" call to module arguments */
    error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/command",
                                   "properties", OY_CREATE_NEW );

    /** 1.2 get monitor device */
    if(error <= 0)
      error = oyDeviceBackendCall( device, options );

    oyOptions_Release( &options );
  }

  if(error <= 0)
    error = !oyOptions_Count( oyConfigPriv_m(device)->backend_core );

  if(error <= 0)
  {
    device_name = oyConfig_FindString( device, "device_name", 0);
    error = !device_name;
  }

  /** 2. check for success of device detection */
  if(error)
  {
    WARNc2_S( "%s: \"%s\"", _("Could not open device"), device_name );
    goto cleanup;
  }

  /** 3 load profile from file name argument */
  p = oyProfile_FromFile( profile_name, 0, 0 );

  /** 3.1 check for success of profile loading */
  error = !p;
  if(error)
  {
    WARNc2_S( "%s: \"%s\"", _("Could not open profile"), profile_name );
    goto cleanup;
  }

  /** 4. Now remove all those DB configurations fully matching the selected
   *     device.  */
  if(error <= 0)
  {
    /** 4.1 get stored DB's configurations */
#ifdef UNHIDE_CMM
    error = oyConfigs_FromDB( oyConfigPriv_m(device)->registration, &configs, 0 );
#else
    error = 1;
#endif

    n = oyConfigs_Count( configs );
    for( i = 0; i < n; ++i )
    {
      config = oyConfigs_Get( configs, i );

      equal = 0;

      j_n = oyOptions_Count( oyConfigPriv_m(device)->backend_core );
      for(j = 0; j < j_n; ++j)
      {
        od = oyOptions_Get( oyConfigPriv_m(device)->backend_core, j );
        d_opt = oyFilterRegistrationToText( oyOption_GetRegistration(od),
                                            oyFILTER_REG_MAX, 0 );
        d_val = oyConfig_FindString( device, d_opt, 0 );

        o_val = oyConfig_FindString( config, d_opt, 0 );

        /** 4.1.1 compare if each device key matches to one configuration
         *          key */
        if( (d_val && o_val &&
             oyStrcmp_( d_val, o_val ) == 0 ) ||
            (!d_val && !o_val) )
          ++equal;
        else
          if(oyStrcmp_( d_opt, "profile_name") == 0)
            ++equal;

        oyOption_Release( &od );
        oyFree_m_( d_opt );
      }

      /** 4.1.2 if the 4.1.1 condition is true remove the configuration */
      if(equal == j_n)
        oyConfig_EraseFromDB( config );

      oyConfig_Release( &config );
    }
    oyConfigs_Release( &configs );
  }

  /** 5. save the new configuration with a associated profile \n
   *  5.1 add the profile simply to the device configuration */
  if(error <= 0)
  {
    error = oyConfig_ClearDBData( device );
    error = oyConfig_AddDBData( device, "profile_name", profile_name,
                                OY_CREATE_NEW );
  }

  /** 5.2 save the configuration to DB (Elektra) */
  if(error <= 0)
    error = oyConfig_SaveToDB( device );
  /** 5.3 reload the DB part */
  if(error <= 0)
    error = oyConfig_GetDB( device, 0 );

  cleanup:
  oyConfig_Release( &device_tmp );

  return error;
}

/** Function oyDeviceProfileFromDB
 *  @brief   look up a profile of a device from DB
 *
 *  The function asks the module for a detailed and possible expensive list
 *  of device information and tries to find a matching configuration in the
 *  DB. The device informations are the same as for saving to DB.
 *
 *  @param[in]     device          a device
 *  @param[in]     profile_name        profile's name in DB
 *  @param[in]     allocateFunc        user allocator
 *  @return                            error
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/01/21 (Oyranos: 0.1.10)
 *  @date    2009/02/09
 */
OYAPI int OYEXPORT oyDeviceProfileFromDB
                                     ( oyConfig_s        * device,
                                       char             ** profile_name,
                                       oyAlloc_f           allocateFunc )
{
  oyOption_s * o = 0;
  oyOptions_s * options = 0;
  int error = !device || !profile_name;
  const char * device_name = 0;
  char * tmp = 0, * tmp2 = 0;
  int32_t rank_value = 0;
  oyConfig_s * s = device;

  oyCheckType__m( oyOBJECT_CONFIG_S, return 1 )

  if(!allocateFunc)
    allocateFunc = oyAllocateFunc_;

  if(error <= 0)
  {
    o = oyConfig_Find( device, "profile_name" );
    device_name = oyConfig_FindString( device, "device_name", 0);

    /* 1. obtain detailed and expensive device informations */
    if( !oyConfig_FindString(s,"manufacturer",0) ||
        !oyConfig_FindString(s,"model",0) )
    { 
      /* 1.1 add "properties" call to module arguments */
      error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/command",
                                     "properties", OY_CREATE_NEW );
      error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/device_name",
                                     device_name, OY_CREATE_NEW );

      device_name = 0;

      /* 1.2 get device */
      if(error <= 0)
        error = oyDeviceBackendCall( device, options );

      oyOptions_Release( &options );

      /* renew outdated string */
      o = oyConfig_Find( device, "profile_name" );
      device_name = oyConfig_FindString( device, "device_name", 0);
      oyOption_Release( &o );
    }

    if(!o)
    {
      error = oyConfig_GetDB( device, &rank_value );
      o = oyConfig_Find( device, "profile_name" );
    }

    if(!o)
    {
      o = oyOptions_Get( oyConfigPriv_m(device)->db, 0 );
      if(o)
        tmp = oyStringCopy_(oyOption_GetRegistration(o), oyAllocateFunc_);
      if(tmp && oyStrrchr_( tmp, OY_SLASH_C))
      {
        tmp2 = oyStrrchr_( tmp, OY_SLASH_C);
        tmp2[0] = 0;
      }
      WARNc3_S( "\n Could not get a \"profile_name\" from %s\n"
                " registration: \"%s\" rank: %d", 
                oyNoEmptyString_m_(device_name), oyNoEmptyString_m_(tmp),
                (int)rank_value )
      if(tmp)
        oyFree_m_(tmp); tmp2 = 0;
      oyOption_Release( &o );
      error = -1;
    } else if(!oyOption_GetValueString(o,0))
    {
      WARNc1_S( "Could not get \"profile_name\" data from %s", 
                oyNoEmptyString_m_(device_name) )
      error = -1;
    } else
      *profile_name = oyOption_GetValueText( o, allocateFunc );

  } else
    WARNc_S( "missed argument(s)" );

  return error;
}

/** Function oyDeviceSelectSimiliar
 *  @brief   get similiar devices by a pattern from a list
 *
 *  The function takes a device and tries to find exact matches, which can be
 *  considered as belonging to the same device. The comparision can be 
 *  influenced by the flags.
 *  The option "profile_name" is ignored during the comparision.
 *
 *  @param[in]     pattern             Pass a device used as reference. String
 *                                     options of this object are compared to
 *                                     the objects in the heap argument
 *                                     depending on the flags argument.
 *                                     "profile_name" and other
 *                                     options from heap objects are ignored.
 *  @param[in]     heap                a list of device objects
 *  @param[in]     flags               - 0 yields exact match
 *                                     - 1 compare manufacturer model and serial
 *                                     - 2 compare only manufacturer and model
 *                                     - 4 compare only device_name
 *  @param[out]    matched_devices     the devices selected from heap
 *  @return                            error
 *
 *  @version Oyranos: 0.1.10
 *  @since   2009/08/27 (Oyranos: 0.1.10)
 *  @date    2009/08/27
 */
OYAPI int OYEXPORT oyDeviceSelectSimiliar
                                     ( oyConfig_s        * pattern,
                                       oyConfigs_s       * heap,
                                       uint32_t            flags,
                                       oyConfigs_s      ** matched_devices )
{
  oyOption_s * odh = 0,
             * od = 0;
  int error  = !pattern || !matched_devices;
  char * od_key = 0;
  const char * od_val = 0,
             * odh_val = 0;
  oyConfig_s * s = pattern,
             * dh = 0;
  oyConfigs_s * matched = 0;
  int i,j,n,j_n;
  int match = 1;

  oyCheckType__m( oyOBJECT_CONFIG_S, return 1 )

  if(error <= 0)
  {
    n = oyConfigs_Count( heap );

    /* Make shure the pattern has as well manufacturer, model included.
     * If not try a "properties" command. */
    if((flags == 0 || flags == 1 || flags == 2) &&
       (!oyConfig_FindString(s,"manufacturer",0) ||
        !oyConfig_FindString(s,"model",0)))
    {
      oyOptions_s * options = 0;
      error = oyOptions_SetFromText( &options, "//" OY_TYPE_STD "/config/command",
                                   "properties", OY_CREATE_NEW );
      oyDeviceBackendCall( s, options );
      oyOptions_Release( &options );
    }

    if((flags == 1 || flags == 2) &&
       (!oyConfig_FindString(s,"manufacturer",0) ||
        !oyConfig_FindString(s,"model",0)))
    {
      return 0;
    }

    matched = oyConfigs_New( 0 );

    for(i = 0; i < n; ++i)
    {
      match = 0;
      dh = oyConfigs_Get( heap, i );

      j_n = oyConfig_Count( pattern );
      for(j = 0; j < j_n; ++j)
      {
        match = 1;
        od = oyConfig_Get( pattern, j );
        od_key = oyFilterRegistrationToText( oyOption_GetRegistration(od),
                                             oyFILTER_REG_MAX, 0);

        od_val = oyOption_GetValueString( od, 0 );
        if(!od_val)
          /* ignore non text options */
          continue;

        /* handle selective flags */
        if(flags == 4 &&
           oyStrcmp_(od_key,"device_name") != 0
          )
          continue;
        else
        if(flags == 2 &&
           oyStrcmp_(od_key,"manufacturer") != 0 &&
           oyStrcmp_(od_key,"model") != 0
          )
          continue;
        else
        if(flags == 1 &&
           oyStrcmp_(od_key,"manufacturer") != 0 &&
           oyStrcmp_(od_key,"model") != 0 &&
           oyStrcmp_(od_key,"serial") != 0
          )
          continue;

        /* ignore a "profile_name" option */
        if(oyStrcmp_(od_key,"profile_name") == 0)
          continue;

        odh = oyOptions_Find( oyConfigPriv_m(dh)->db, od_key );

        odh_val = oyOption_GetValueString( odh, 0 );
        if( !odh_val )
          /* ignore non text options */
          match = 0;

        if(match && oyStrcmp_( od_val, odh_val ) != 0)
          match = 0;

        /*printf("pruefe: %s=%s match = %d flags=%d\n", od_key, od_val, match, flags);*/


        oyOption_Release( &od );

        oyOption_Release( &odh );

        if(match == 0)
          break;
      }

      if(match)
        oyConfigs_MoveIn( matched, &dh, -1 );
      else
        oyConfig_Release( &dh );
    }

    if(oyConfigs_Count( matched ))
      *matched_devices = matched;
    else
      oyConfigs_Release( &matched );

  } else
    WARNc_S( "missed argument(s)" );

  return error;
}

/** Function oyDeviceFromJSON
 *  @brief   generate a device from a JSON device calibration
 *
 *  @param[in]    json_text            the device calibration
 *  @param[in]    options              optional
 *                                     - "underline_key_suffix" will be used as
 *                                       suffix for keys starting with underline
 *                                       '_'
 *                                     - "pos" integer selects position in array
 *  @param[out]   config               the device
 *  @return                            error
 *
 *  @version Oyranos: 0.3.3
 *  @since   2011/08/21 (Oyranos: 0.3.2)
 *  @date    2012/01/06
 */
OYAPI int  OYEXPORT oyDeviceFromJSON ( const char        * json_text,
                                       oyOptions_s       * options,
                                       oyConfig_s       ** device )
{
  int error = !json_text || !device;
  oyConfig_s * device_ = NULL;
  oyjl_value_s * json = 0,
               * json_device;
  char * val, * key, * t = NULL;
  const char * xpath = "org/freedesktop/openicc/device/[0]/[%d]";
  int count, i;
  int32_t pos = 0;
  const char * underline_key_suffix = oyOptions_FindString( options,
                                                    "underline_key_suffix", 0 );

  yajl_status status;

  device_ = oyConfig_FromRegistration( "//" OY_TYPE_STD "/config", 0 );
  status = oyjl_tree_from_json( json_text, &json, 0 );
  if(status != yajl_status_ok)
    WARNc2_S( "\"%s\" %d\n", _("found issues parsing JSON"), status );

  error = oyOptions_FindInt( options, "pos", 0, &pos );
  oyStringAddPrintf_( &t, oyAllocateFunc_, oyDeAllocateFunc_,
                          xpath, pos );

  json_device = oyjl_tree_get_value( json, t );

  if(!json_device)
    WARNc2_S( "\"%s\" %s\n", t,_("not found:") );
  oyFree_m_( t );
      
  count = oyjl_value_count(json_device);
  for(i = 0; i < count; ++i)
  {
    oyjl_value_s * v = oyjl_value_pos_get( json_device, i );
    key = oyStringCopy_(oyjl_print_text( &v->value.object->key ), 0);
    val = oyjl_value_text( v, oyAllocateFunc_ );

    if(key && key[0] && key[0] == '_' && underline_key_suffix)
    {
      t = 0;
      STRING_ADD( t, underline_key_suffix );
      STRING_ADD( t, key );
      oyFree_m_( key );
      key = t; t = 0;
    }

    /* ignore empty keys or values */
    if(key && val)
      oyConfig_AddDBData( device_, key, val, OY_CREATE_NEW );

    if(key) oyDeAllocateFunc_(key);
    if(val) oyDeAllocateFunc_(val);
  }

  *device = device_;
  device_ = NULL;

  return error;
}

#define OPENICC_DEVICE_JSON_HEADER \
  "{\n" \
  "  \"org\": {\n" \
  "    \"freedesktop\": {\n" \
  "      \"openicc\": {\n" \
  "        \"device\": {\n" \
  "          \"%s\": [{\n"
#define OPENICC_DEVICE_JSON_FOOTER \
  "            }\n" \
  "          ]\n" \
  "        }\n" \
  "      }\n" \
  "    }\n" \
  "  }\n" \
  "}\n"

/** Function oyDeviceToJSON
 *  @brief   get JSON format device calibration text from a device
 *
 *  @param[in]     config              the device
 *  @param[in]     options             unused
 *  @param[out]    json_text           the device calibration
 *  @param[in]     allocateFunc        user allocator
 *  @return                            error
 *
 *  @version Oyranos: 0.3.2
 *  @since   2011/08/21 (Oyranos: 0.3.2)
 *  @date    2011/08/21
 */
OYAPI int OYEXPORT oyDeviceToJSON    ( oyConfig_s        * device,
                                       oyOptions_s       * options,
                                       char             ** json_text,
                                       oyAlloc_f           allocateFunc )
{
  int error = 0;
  int count = oyConfig_Count( device ), j, k;
  char * t = NULL; 
  char * value, * key;
  const char * device_class = NULL;
  oyConfDomain_s * domain;

  if(!error)
  {
      {
        oyOption_s * opt = oyConfig_Get( device, 0 );

        domain = oyConfDomain_FromReg( oyOption_GetRegistration( opt ), 0 );
        device_class = oyConfDomain_GetText( domain, "device_class", oyNAME_NICK );
        oyOption_Release( &opt );

        /* add device class */
        oyStringAddPrintf_( &t, oyAllocateFunc_, oyDeAllocateFunc_,
                            OPENICC_DEVICE_JSON_HEADER, device_class );

        /* add device and driver calibration properties */
        for(j = 0; j < count; ++j)
        {
          int vals_n = 0;
          char ** vals = 0, * val = 0;
          opt = oyConfig_Get( device, j );

          key = oyFilterRegistrationToText( oyOption_GetRegistration( opt ),
                                            oyFILTER_REG_MAX, 0 );
          value = oyOption_GetValueText( opt, oyAllocateFunc_ );

          if(value && count > j)
          {
            if(value[0] == '<')
               oyStringAddPrintf_( &t, oyAllocateFunc_, oyDeAllocateFunc_,
               "              \"%s\": \"%s\"",
                     key, value );
            else
            {
              /* split into a array with a useful delimiter */
              vals = oyStringSplit_( value, '?', &vals_n, malloc );
              if(vals_n > 1)
              {
                STRING_ADD( val, "              \"");
                STRING_ADD( val, key );
                STRING_ADD( val, ": [" );
                for(k = 0; k < vals_n; ++k)
                {
                  if(k != 0)
                  STRING_ADD( val, "," );
                  STRING_ADD( val, "\"" );
                  STRING_ADD( val, vals[k] );
                  STRING_ADD( val, "\"" );
                }
                STRING_ADD( val, "]");
                oyStringAddPrintf_( &t, oyAllocateFunc_, oyDeAllocateFunc_,
                     "%s", val );
                if(val) free( val );
              } else
                oyStringAddPrintf_( &t, oyAllocateFunc_, oyDeAllocateFunc_,
                     "              \"%s\": \"%s\"",
                     key, value );

              oyStringListRelease_( &vals, vals_n, free );
            }
          }
          if(value && j < count - 1)
            oyStringAddPrintf_( &t, oyAllocateFunc_, oyDeAllocateFunc_,
                                ",\n" );
          oyOption_Release( &opt );
          if(key)
            oyFree_m_( key );
          if(value)
            oyFree_m_( value );
        }

        oyStringAddPrintf_( &t, oyAllocateFunc_, oyDeAllocateFunc_,
                            "\n"OPENICC_DEVICE_JSON_FOOTER );
        oyConfDomain_Release( &domain );
      }

      if(json_text && t)
      {
        *json_text = oyStringCopy_( t, allocateFunc );
        oyFree_m_( t );
      }
  }

  return error;
}


/**
 *  @} *//* devices_handling
 */
