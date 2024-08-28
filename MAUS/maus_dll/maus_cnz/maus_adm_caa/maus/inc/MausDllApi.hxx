//  **************************************************************************
//  **************************************************************************
//  ***                                                                    ***
//  ***  COPYRIGHT Ericsson AB 2015. All rights reserved.                  ***
//  ***                                                                    ***
//  ***  The information in this document is the property of Ericsson.     ***
//  ***  Except as specifically authorized in writing by Ericsson, the     ***
//  ***  receiver of this document shall keep the information contained    ***
//  ***  herein confidential and shall protect the same in whole or        ***
//  ***  in part from disclosure and dissemination to third parties.       ***
//  ***  Disclosure and disseminations to the receiver's employees shall   ***
//  ***  only be made on a strict need to know basis.                      ***
//  ***                                                                    ***
//  **************************************************************************
//  **************************************************************************
//
//  Responsible ............ TEI/XPN
//
//< \file
//
//  API exported by MAUCore DLL
//
//>

#ifndef MAUSDLLAPI_HXX
#define MAUSDLLAPI_HXX

//----------------------------------------------------------------------------
//  Imported Interfaces, Types & Definitions
//----------------------------------------------------------------------------

#include <stdint.h>

//----------------------------------------------------------------------------
//  Exported Types & Definitions
//----------------------------------------------------------------------------

#define CREATE_MAU_INFO_SYMBOL "createMausCoreInstance"

#define MAU_API_MAJOR 1
#define MAU_API_MINOR 0

#pragma pack(push, 1)

namespace MausDllApi
{
   typedef void (*LogFn)(int32_t level, int32_t type, const char* msg);

   // Stolen from InternalLog::LogLevel
   enum LogLevel
   {
      LL_Trace,  // 0
      LL_Debug,  // 1
      LL_Info,   // 2
      LL_Warn,   // 3
      LL_Error,  // 4
      LL_Fatal,  // 5
      NumberOfLogLevels
   };

   // Stolen from InternalLog::LogCode
   enum LogCode
   {
      LC_MAUBase,
      LC_CommandConsole,
      LC_FTM,
      LC_SignalDispatcher,
      LC_WSL,
      LC_MSG_Q,
      LC_CCSB,
      LC_QuorumData,
      LC_QuorumState,
      LC_MAUConnection,
      LC_UnsupportedSig,
      LC_LogHandler,
      LC_MAULog,
      NumberOfLogCodes
   };

   class MausCore
   {
      public:
         enum ResultCode
         {
            OK            =  1,  // Success.
            Busy          =  0,  // Function is busy.
            Unsupported   = -1,  // Function is not supported.
            BadInput      = -2,  // Parameter passed to MAUCore is out of range.
            AlreadyExists = -3,  // Attempt to create more than one instance.
            AllocFailed   = -4,  // Failed to allocate memory.
            Init1Failed   = -5,  // Create failed in call to initialize().
            Init2Failed   = -6,  // Create failed in call to initializeCommon().
            NoInit        = -7,  // MAUCore is not initialized.
            NoActiveMAU   = -8   // MAUCore has no active MAU.
         };

         // Values taken from MAUBase.hxx
         enum MaxStrLen
         {
            IntferfaceNameLen = 16,
            LogRootLen = 64
         };

         struct VersionBytes
         {
            int8_t major : 8;
            int8_t minor : 8;
         };

         union Version
         {
            VersionBytes versionbytes;
            int16_t      version;
         };

         struct MausInfo
         {
            int32_t side;                // 0 = AP-A, 1 = AP-B
            int32_t shlf;                // 0 = SCB-RP4, 1 = SCX, 2 = BSP, 3 = ECS
            int32_t sysNo;               // System numbner
            char    interface[2][IntferfaceNameLen]; // Backplane NIC name
            char    logRoot[LogRootLen]; // CLH log root directory
            Version version;             // Client API version
            int32_t mausServiceIdCode;   // 1 = NoRed Service MAUS1, 2 = NoRed Service MAUS2
            int32_t fcState;             // 0 = Idle, 1 = Busy
            int32_t	apgState;			 // -1 == error, 1 == Active, 2 == Passive
         };

         struct MausQuorum
         {
            int32_t connView;    // Bit 0 AP-A, bit 1 AP-B, bit 2 CP-A, bit 3 CP-B
            int32_t quorumView;  // Bit 0 AP-A, bit 1 AP-B, bit 2 CP-A, bit 3 CP-B
            int32_t activeMau;   // 0 = AP-A, 1 = AP-B, 2 = CP-A, 3 = CP-B
            int32_t activeMask;  // Bit 0 AP-A, bit 1 AP-B, bit 2 CP-SB
         };

         enum FCState
         {
            FC_Idle,  // Function change is not in progress.
            FC_Busy   // Function change is in progress.
         };

         MausCore();
         virtual ~MausCore();

         virtual bool isInitialized() = 0;
         virtual void maus_poll() = 0;
         virtual ResultCode maus_create(const MausInfo& info, LogFn logFn = 0) = 0;
         virtual ResultCode maus_destroy() = 0;
         virtual ResultCode maus_get_quorum_info(MausQuorum& quorum) = 0;
         virtual ResultCode maus_set_mask(int32_t mask) = 0;
         virtual ResultCode maus_set_fc_state(int32_t fcState) = 0;
         virtual ResultCode maus_get_property(int32_t property, int32_t* value, uint32_t* length) = 0;
         virtual ResultCode maus_set_property(int32_t property, int32_t* value, uint32_t length) = 0;

      private:
         // Disable the copy constructor and Assignment operator.
         // Copy constructor. Disabled by being declared private.
         MausCore(const MausCore&);

         // Assignment operator. Disabled by being declared private.
         MausCore& operator=(const MausCore&);
   };
}

#pragma pack(pop)

#endif // MAUSDLLAPI_HXX

