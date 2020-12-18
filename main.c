
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include "nvutility.h"

#include "pfm.h"

#include "version.h"


#define PARSE_OFFSET 1000


/*****************************************************************************

    Program:    pfm_unedit

    Purpose:    See Usage message below.

    Programmer: Jan C. Depner

    Date:       01/18/11

*****************************************************************************/


int32_t main (int32_t argc, char **argv)
{
  int32_t             i, j, percent = 0, old_percent = -1, pfm_handle; 
  uint32_t            k;
  PFM_OPEN_ARGS       open_args;
  float               total;
  NV_I32_COORD2       coord;
  DEPTH_RECORD        *dep;
  BIN_RECORD          bin;
  uint8_t             recomp;



  fprintf (stderr, "\n\n%s\n\n", VERSION);
  fflush (stderr);


  if (argc < 2)
    {
      fprintf (stderr, "\n\nUsage: pfm_unedit <PFM_HANDLE_FILE>\n\n");
      fprintf (stderr, "Validates all previously PFM_MANUALLY_INVAL data\n");
      fprintf (stderr, "in a PFM.  This is an emergency recovery tool for\n");
      fprintf (stderr, "those times when someone completely screws the pooch\n");
      fprintf (stderr, "and edits out a bunch of valid data.\n\n");
      exit (-1);
    }


  strcpy (open_args.list_path, argv[1]);


  open_args.checkpoint = 0;
  pfm_handle = open_existing_pfm_file (&open_args);


  if (pfm_handle < 0) pfm_error_exit (pfm_error);


  total = open_args.head.bin_height;

  fprintf (stderr, "\n\n");
  fflush (stderr);


  /*  Loop through the PFM.  */

  for (i = 0; i < open_args.head.bin_height; i++)
    {
      coord.y = i;

      for (j = 0; j < open_args.head.bin_width; j++)
        {
          coord.x = j;

          recomp = NVFalse;

          bin.coord = coord;


          /*  Read the depth array.  */

          if (!read_bin_depth_array_index (pfm_handle, &bin, &dep))
            {
              for (k = 0 ; k < bin.num_soundings ; k++)
                {
                  /*  If it's PFM_DELETED or PFM_REFERENCE, don't mess with it.  */

                  if (!(dep[k].validity & (PFM_DELETED | PFM_REFERENCE)))
                    {
                      /*  Get only PFM_MANUALLY_INVAL data.  */

                      if (dep[k].validity & PFM_MANUALLY_INVAL)
                        {
                          dep[k].validity &= ~PFM_MANUALLY_INVAL;

                          update_depth_record_index (pfm_handle, &dep[k]);
                          recomp = NVTrue;
                        }
                    }
                }


              /*  If we modified anything, recompute the bin values.  */

              if (recomp) 
                {
                  bin.validity &= ~PFM_CHECKED;
                  recompute_bin_values_from_depth_index (pfm_handle, &bin, PFM_CHECKED, dep);
                }
              free (dep);
            }
        }

      percent = (i / total) * 100.0;
      if (percent != old_percent)
        {
          old_percent = percent;
          fprintf (stderr, "Validating manually invalid points : %03d%%          \r", percent);
          fflush (stderr);
        }
    }

  fprintf (stderr, "100%% validated                                             \n\n");
  fflush (stderr);

  close_pfm_file (pfm_handle);


  /*  Please ignore the following line.  It is useless.  Except...

      On some versions of Ubuntu, if I compile a program that doesn't use the math
      library but it calls a shared library that does use the math library I get undefined
      references to some of the math library functions even though I have -lm as the last
      library listed on the link line.  This happens whether I use qmake to build the
      Makefile or I have a pre-built Makefile.  Including math.h doesn't fix it either.
      The following line forces the linker to bring in the math library.  If there is a
      better solution please let me know at area.based.editor AT gmail DOT com.  */

  float ubuntu; ubuntu = 4.50 ; ubuntu = fmod (ubuntu, 1.0);


  return (0);
}
