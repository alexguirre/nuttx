/****************************************************************************
 * config/sama5d3x-ek/src/sam_nsh.c
 *
 *   Copyright (C) 2013 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/mount.h>

#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <debug.h>

#ifdef CONFIG_SAMA5_SPI0
#  include <nuttx/spi/spi.h>
#  include <nuttx/mtd.h>
#  include <nuttx/fs/nxffs.h>

#  include "sam_spi.h"
#endif

#include "sama5d3x-ek.h"

/****************************************************************************
 * Pre-Processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

#define HAVE_AT25  1
#define HAVE_MMCSD 1

/* Can't support the AT25 device if it SPI0 or AT25 support are not enabled */

#if !defined(CONFIG_SAMA5_SPI0) || !defined(CONFIG_MTD_AT25)
#  undef HAVE_AT25
#endif

/* Can't support AT25 features if mountpoints are disabled or if we were not
 * asked to mount the AT25 part
 */

#if defined(CONFIG_DISABLE_MOUNTPOINT) || !defined(CONFIG_SAMA5_AT25_AUTOMOUNT)
#  undef HAVE_AT25
#endif

/* If we are going to mount the AT25, then they user must also have told
 * us what to do with it by setting one of these.
 */

#if !defined(CONFIG_SAMA5_AT25_FTL) && !defined(CONFIG_SAMA5_AT25_NXFFS)
#  undef HAVE_AT25
#endif

/* Use minor device number 0 is not is provided */

#ifndef CONFIG_NSH_MMCSDMINOR
#  define CONFIG_NSH_MMCSDMINOR 0
#endif

/* Can't support MMC/SD if the card interface(s) are not enable */

#if !defined(CONFIG_SAMA5_HSMCI0) && !defined(CONFIG_SAMA5_HSMCI0)
#  undef HAVE_MMCSD
#endif

/* Can't support MMC/SD features if mountpoints are disabled */

#if defined(CONFIG_DISABLE_MOUNTPOINT)
#  undef HAVE_MMCSD
#endif

/* Assign minor device numbers.  We basically ignore more of the NSH
 * configuration here (NSH SLOTNO ignored completely; NSH minor extended
 * to handle more devices.
 */

#ifndef CONFIG_NSH_MMCSDMINOR
#  define CONFIG_NSH_MMCSDMINOR 0
#endif

#ifdef HAVE_MMCSD

#  define HSMCI0_SLOTNO 0
#  define HSMCI1_SLOTNO 1

#  ifdef CONFIG_SAMA5_HSMCI0
#     define HSMCI0_MINOR  CONFIG_NSH_MMCSDMINOR
#     define HSMCI1_MINOR  (CONFIG_NSH_MMCSDMINOR+1)
#     define AT25_MINOR    (CONFIG_NSH_MMCSDMINOR+2)
#  else
#     define HSMCI1_MINOR  CONFIG_NSH_MMCSDMINOR
#     define AT25_MINOR    (CONFIG_NSH_MMCSDMINOR+1)
#  endif
#else
#  define AT25_MINOR CONFIG_NSH_MMCSDMINOR
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nsh_archinitialize
 *
 * Description:
 *   Perform architecture specific initialization
 *
 ****************************************************************************/

int nsh_archinitialize(void)
{
#if defined(HAVE_AT25) || defined(HAVE_MMCSD)
  int ret;
#endif

  /* Initialize the AT25 driver */

#ifdef HAVE_AT25
  ret = sam_at25_initialize(AT25_MINOR);
  if (ret < 0)
    {
      fdbg("ERROR: sam_at25_initialize failed: %d\n", ret);
      return ret;
#endif

#ifdef HAVE_MMCSD
#ifdef CONFIG_SAMA5_HSMCI0
  ret = sam_hsmci_initialize(HSMCI0_SLOTNO, HSMCI0_MINOR);
  if (ret < 0)
    {
      fdbg("ERROR: sam_hsmci_initialize(%d,%d) failed: %d\n",
           HSMCI0_SLOTNO, HSMCI0_MINOR, ret);
      return ret;
    }
#endif

#ifdef CONFIG_SAMA5_HSMCI0
  ret = sam_hsmci_initialize(HSMCI1_SLOTNO, HSMCI1_MINOR);
  if (ret < 0)
    {
      fdbg("ERROR: sam_hsmci_initialize(%d,%d) failed: %d\n",
           HSMCI1_SLOTNO, HSMCI1_MINOR, ret);
      return ret;
    }
#endif
#endif

  return OK;
}
