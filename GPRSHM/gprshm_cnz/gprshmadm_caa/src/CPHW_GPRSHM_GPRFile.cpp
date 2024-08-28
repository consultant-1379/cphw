/****************************************************************************/
/**
 *  CPHW_GPRSHM_GPRFile.cpp
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
 *  This is class implementation for CPHW_GPRSHM_GPRFile class that is used
 *  for access to the GPR cache file.
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-01-03  xdtkebo  Use APGCC API to determine the cache file path.
 *  2017-01-25  xjoschu  Fix for gprshm/ folder creation.
 */

#include "CPHW_GPRSHM_GPRFile.h"
#include "CPHW_GPRSHM_Trace.h"
#include <ACS_APGCC_CommonLib.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <fstream>
#include <iostream>
#include <string>
#include <new>

CPHW_GPRSHM_GPRFile* CPHW_GPRSHM_GPRFile::s_singletonPtr = NULL;
unsigned int CPHW_GPRSHM_GPRFile::s_noOfUsers = 0;

/**
 * @brief Static method to get the instance pointer.
 *        The instance will be created if the caller is the first user.
 *
 * @param[out] err Error code. 0 = success, -1 = failure.
 * @return Pointer to the instance.
 */
CPHW_GPRSHM_GPRFile* CPHW_GPRSHM_GPRFile::getInstance(int& err)
{
   if (s_singletonPtr == NULL)
   {
      s_singletonPtr = new(std::nothrow) CPHW_GPRSHM_GPRFile();
      if (s_singletonPtr != NULL)
      {
         err = s_singletonPtr->openFile();
         if (err == 0)
         {
            s_noOfUsers++;
         }
         else
         {
            delete s_singletonPtr;
            s_singletonPtr = NULL;
         }
      }
   }
   else
   {
      err = 0;
      s_noOfUsers++;
   }

   return s_singletonPtr;
}

/**
 * @brief Static method to delete the instance.
 *        The instance will be deleted if the caller is the only user.
 *
 * @param[in,out] object Pointer to the instance. Will be returned as NULL.
 */
void CPHW_GPRSHM_GPRFile::deleteInstance(CPHW_GPRSHM_GPRFile*& object)
{
   if ((object != NULL) && (object == s_singletonPtr))
   {
      s_noOfUsers--;
      if ((s_singletonPtr != NULL) && (s_noOfUsers == 0))
      {
         delete s_singletonPtr;
         s_singletonPtr = NULL;
      }
      object = NULL;
   }
}

/**
 * @brief Constructor.
 */
CPHW_GPRSHM_GPRFile::CPHW_GPRSHM_GPRFile() :
   m_dataPath(),
   m_blockPath(),
   m_cachePath(),
   m_fileHandle(-1),
   m_recAddr(s_noOfRecords, (uint32_t*)0),
   m_mapStartAddr(NULL)
{
}

/**
 * @brief Destructor.
 */
CPHW_GPRSHM_GPRFile::~CPHW_GPRSHM_GPRFile()
{
   if (m_mapStartAddr)
   {
      munmap(m_mapStartAddr, s_noOfRecords * sizeof(uint32_t));
   }

   if (m_fileHandle != -1)
   {
      close(m_fileHandle);
   }
}

/**
 * @brief Open the GPR Cache file.
 *
 * @return Error code. 0 = success, -1 = failure.
 */
int CPHW_GPRSHM_GPRFile::openFile()
{
   ///< Create the directory where the GPR cache file will be stored.
   createDirectories(getBlockPath(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

   ///< Check if this process have write permission on the directory path
   if (access(getBlockPath().c_str(), R_OK | W_OK) != 0)
   {
      TRACE((LOG_LEVEL_ERROR, "GPRFile No access to GPRSHM block directory %s (errno = %d).",
                              0, getBlockPath().c_str(), errno));
      return -1;
   }

   // Clear the calling process's file mode creation mask and save
   // the previous mask value for restoring later.
   mode_t oldMask = umask(0);

   ///< Open an existing or create a new registration file.
   m_fileHandle = open(getCachePath().c_str(), O_RDWR | O_CREAT,
                                               S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

   // Restore the calling process's file mode creation mask.
   umask(oldMask);

   if (m_fileHandle < 0)
   {
      TRACE((LOG_LEVEL_ERROR, "GPRFile Failed to open GPR cache file %s (errno = %d).",
                              0, getCachePath().c_str(), errno));
      return -1;
   }

   // Get information about the cache file, based on its file descriptor.
   struct stat info;
   int statok;
   statok = fstat(m_fileHandle, &info);
   if (statok < 0)
   {
      TRACE((LOG_LEVEL_ERROR, "GPRFile Failed to get stat info for GPR cache file (errno = %d).",
                              0, errno));
      return -1;
   }

   // Create an initial set of GPR records if needed.
   const size_t cacheSize = s_noOfRecords * sizeof(uint32_t);;
   if ((info.st_size == 0) || (info.st_size != cacheSize))
   {
      // The file can contain up to 64*8 GPR records.
      char clearBuf[cacheSize];
      memset(clearBuf, 0, cacheSize);

      ssize_t writeok;
      writeok = write(m_fileHandle, clearBuf, cacheSize);
      if (writeok < 0)
      {
         TRACE((LOG_LEVEL_ERROR, "GPRFile Failed to write initial cache to GPR cache file (errno = %d).",
                                 0, errno));
         return -1;
      }
      if (writeok != static_cast<ssize_t>(cacheSize))
      {
         TRACE((LOG_LEVEL_ERROR, "GPRFile Incomplete write of initial cache to GPR cache file.", 0));
         return -1;
      }
   }

   // Map the GPR cache file to memory.
   m_mapStartAddr = (uint32_t*)mmap((caddr_t)0, cacheSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_fileHandle, 0);
   if (m_mapStartAddr == (uint32_t*)-1)
   {
      TRACE((LOG_LEVEL_ERROR, "GPRFile Failed to map GPR cache file to memory (errno = %d).",
                              0, errno));
      return -1;
   }

   // Fill the m_recAddr vector with valid file-pointers.
   uint32_t* fptr = m_mapStartAddr;
   for (unsigned int index = 0; index < s_noOfRecords; index++, fptr++)
   {
      m_recAddr[index] = fptr;
   }

   TRACE((LOG_LEVEL_INFO, "GPRFile GPR cache file %s is open.", 0, getCachePath().c_str()));
   return 0;
}

/**
 * @brief Write to a GPR register.
 *
 * @param[in] cpId CP identity number.
 * @param[in] address GPR register number.
 * @param[in] value Data value to write.
 *
 * @return Flag indicating success or failure.
 * @retval true Success.
 * @retval false Failure.
 */
bool CPHW_GPRSHM_GPRFile::set(const uint32_t cpId, const uint32_t address, const uint32_t value)
{
   bool done = false;
   unsigned int index = (cpId * NO_OF_GPR) + address;

   if (index < s_noOfRecords)
   {
      *(m_recAddr[index]) = value;
      msync(m_recAddr[index], sizeof(uint32_t), MS_ASYNC);
      done = true;
   }

   return done;
}

/**
 * @brief Read from a GPR register.
 *
 * @param[in] cpId CP identity number.
 * @param[in] address GPR register number.
 * @param[out] value Data value read.
 *
 * @return Flag indicating success or failure.
 * @retval true Success.
 * @retval false Failure.
 */
bool CPHW_GPRSHM_GPRFile::get(const uint32_t cpId, const uint32_t address, uint32_t& value)
{
   bool done = false;
   unsigned int index = (cpId * NO_OF_GPR) + address;

   if (index < s_noOfRecords)
   {
      value = *(m_recAddr[index]);
      done = true;
   }

   return done;
}

/**
 * @brief Create recursive directories.
 *
 * @param[in] path The path name of directory to be created.
 * @param[in] mode The permission bits for file /directory creation..
 *
 * @return Flag indicating success or failure.
 * @retval true Success.
 * @retval false Failure.
 */
bool CPHW_GPRSHM_GPRFile::createDirectories(const std::string& path, mode_t mode)
{
   std::string::size_type search_pos = 0;
   std::string::size_type result_pos = 0;
   int result;

   // Clear the calling process's file mode creation mask and save
   // the previous mask value for restoring later.
   mode_t oldMask = umask(0);

   do
   {
      result_pos = path.find("/", search_pos);

      if (result_pos == string::npos)
      {
         // can't find '/'
         break;
      }

      result = mkdir(path.substr(0, result_pos + 1).c_str(), mode);

      if ((result == -1) && (errno != EEXIST))
      {
         // Can't create directory
         break;
      }

      search_pos = result_pos + 1;
   }
   while (true);

   // Restore the calling process's file mode creation mask.
   umask(oldMask);

   // Check if directory exist
   struct stat buf;
   return (stat(path.c_str(), &buf) == 0);
}

/**
 * @brief Get the path name of the APZ data directory.
 *        The path name will be retrieved from APGCC if it is not already done.
 *
 * @return A string containing the path name.
 */
std::string& CPHW_GPRSHM_GPRFile::getDataPath()
{
   if (m_dataPath.empty())
   {
      ACS_APGCC_CommonLib commonLib;
      ACS_APGCC_DNFPath_ReturnTypeT result;
      int pathlen = 127;
      char buffer[pathlen + 1];

      result = commonLib.GetDataDiskPath("APZ_DATA", buffer, pathlen);
      switch (result)
      {
         case ACS_APGCC_DNFPATH_SUCCESS:
         {
            m_dataPath.assign(buffer);
            break;
         }
         case ACS_APGCC_DNFPATH_FAILURE:
         {
            TRACE((LOG_LEVEL_ERROR, "GPRFile Failed to get data disk path. General Failure.", 0));
            break;
         }
         case ACS_APGCC_FAULTY_CPID:
         {
            TRACE((LOG_LEVEL_ERROR, "GPRFile Failed to get data disk path. Faulty CPID.", 0));
            break;
         }
         case ACS_APGCC_STRING_BUFFER_SMALL:
         {
            TRACE((LOG_LEVEL_ERROR, "GPRFile Failed to get data disk path. Buffer Too Small (required %d).", 0, pathlen));
            break;
         }
         case ACS_APGCC_FAULT_LOGICAL_NAME:
         {
            TRACE((LOG_LEVEL_ERROR, "GPRFile Failed to get data disk path. Bad Logical Name (APZ_DATA).", 0));
            break;
         }
         default:
         {
            TRACE((LOG_LEVEL_ERROR, "GPRFile Failed to get data disk path. Unknown Error (%d).", 0, result));
            break;
         }
      }
   }

   return m_dataPath;
}

/**
 * @brief Get the path name of the GPRSHM's block directory.
 *        The path name will be created if it is not already done.
 *
 * @return A string containing the path name.
 */
std::string& CPHW_GPRSHM_GPRFile::getBlockPath()
{
   if (m_blockPath.empty())
   {
      m_blockPath.assign(getDataPath());
      m_blockPath.append("/gprshm/");
   }

   return m_blockPath;
}

/**
 * @brief Get the path name of the GPR Cache file.
 *        The path name will be created if it is not already done.
 *
 * @return A string containing the path name.
 */
std::string& CPHW_GPRSHM_GPRFile::getCachePath()
{
   if (m_cachePath.empty())
   {
      m_cachePath.assign(getBlockPath());
      m_cachePath.append("/gprcache");
   }

   return m_cachePath;
}

