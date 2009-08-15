#ifdef __cplusplus
using namespace oyranos;
extern "C" {
#endif /* __cplusplus */

static void print_option(oyOption_s * opt, int j)
{
   int id = oyOption_GetId(opt);
   if (opt->value_type == oyVAL_STRUCT) {
      oyStruct_s *opt_struct = opt->value->oy_struct;
      oyCMMptr_s *cmm = NULL;
      oyBlob_s *blob = NULL;
      switch (opt_struct->type_) {
         case oyOBJECT_CMM_POINTER_S:
            cmm = (oyCMMptr_s*)opt_struct;
            //printf("\tOption[%d] ID=%d\n\t\t[%s]: CMMptr{%p,%s}\n",
            //      j, id, opt->registration, cmm->ptr, cmm->lib_name);
            break;
         case oyOBJECT_BLOB_S:
            blob = (oyBlob_s *)opt_struct;
            printf("\tOption[%d] ID=%d\n\t\t[%s]: blob{%p,%d}\n",
                  j, id, opt->registration, blob->ptr, blob->size);
            break;
         default:
            printf("\tCan't handle struct of type %d\n", opt_struct->type_);
            break;
      }
   } else {
      char *text = oyOption_GetValueText(opt, malloc);
      printf("\tOption[%d] ID=%d\n\t\t[%s]: \"%s\"\n", j, id, opt->registration, text);
      free(text);
   }
}

static int print_options(oyOptions_s * options)
{
   int j;
   int num_options = oyOptions_Count(options);
   printf("Found %d option%s\n", num_options, num_options > 1 ? "s" : "");
   for (j = 0; j < num_options; j++) {
      oyOption_s *opt = oyOptions_Get(options, j);
      print_option(opt, j);
      oyOption_Release(&opt);
   }

   return num_options;
}

static void print_device(oyConfig_s *device)
{
   int j;
   int num_options = oyConfig_Count(device);
   printf("\tFound %d option%s for device\n", num_options, num_options > 1 ? "s" : "");
   for (j = 0; j < num_options; j++) {
      oyOption_s *opt = oyConfig_Get(device, j);
      print_option(opt, j);
      oyOption_Release(&opt);
   }
}

static int print_devices(oyConfigs_s * devices, const char *name)
{
   int i,j;
   int num_devices = oyConfigs_Count(devices);
   printf("Found %d %s device%s\n", num_devices, name, num_devices > 1 ? "s" : "");

   for (i = 0; i < num_devices; i++) {
      oyConfig_s *device = oyConfigs_Get(devices, i);
      int num_options = oyConfig_Count(device);
      printf("\tFound %d option%s for device %d\n", num_options, num_options > 1 ? "s" : "", i);
      for (j = 0; j < num_options; j++) {
         oyOption_s *opt = oyConfig_Get(device, j);
         print_option(opt, j);
         oyOption_Release(&opt);
      }
      oyConfig_Release(&device);
   }
   return num_devices;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
