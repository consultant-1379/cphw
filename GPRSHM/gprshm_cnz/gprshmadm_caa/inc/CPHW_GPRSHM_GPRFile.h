/****************************************************************************/
/**
 *  CPHW_GPRSHM_GPRFile.h
 *
 *  190 89-CAA 109 1638
 *
 *  COPYRIGHT Ericsson AB 2015-2017. All rights reserved.
 *
 *  The Copyright of the computer program(s) herein is the property of
 *  Ericsson AB, Sweden. The program(s) may be used and/or copied only
 *  with the written permission from Ericsson AB or in accordance
 *  with the terms and conditions stipulated in the agreement/contract
 *  under which the program(s) have been supplied.
 */
/****************************************************************************/

/**
 *  Description:
 *  ------------
 *  This is class declaration for CPHW_GPRSHM_GPRFile class that is used
 *  for access the GPR cache file.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-01-03  xdtkebo  Use APGCC API to determine the cache file path.
 */

#ifndef _CPHW_GPRSHM_GPRFILE_H
#define _CPHW_GPRSHM_GPRFILE_H

#include <vector>
#include <string>
#include <stdint.h>
#include <fcntl.h>

#include "CPHW_GPRSHM_Definitions.h"

class CPHW_GPRSHM_GPRFile
{
public:
   static CPHW_GPRSHM_GPRFile* getInstance(int& err);
   static void deleteInstance(CPHW_GPRSHM_GPRFile*& object);

   int openFile();
   bool set(const uint32_t cpId, const uint32_t address, const uint32_t value);
   bool get(const uint32_t cpId, const uint32_t address, uint32_t& value);

private:
   CPHW_GPRSHM_GPRFile();
   CPHW_GPRSHM_GPRFile(const CPHW_GPRSHM_GPRFile&);
   const CPHW_GPRSHM_GPRFile& operator=(const CPHW_GPRSHM_GPRFile&);

   ~CPHW_GPRSHM_GPRFile();

   bool createDirectories(const std::string& path, mode_t mode = 0775);

   std::string& getDataPath();
   std::string& getBlockPath();
   std::string& getCachePath();

   static CPHW_GPRSHM_GPRFile* s_singletonPtr;
   static unsigned int s_noOfUsers;
   static const unsigned int s_noOfRecords = (MAX_CP * NO_OF_GPR);

   std::string m_dataPath;
   std::string m_blockPath;
   std::string m_cachePath;

   int m_fileHandle;
   std::vector<uint32_t*> m_recAddr;
   uint32_t* m_mapStartAddr;
};

#endif /* _CPHW_GPRSHM_GPRFILE_H */

