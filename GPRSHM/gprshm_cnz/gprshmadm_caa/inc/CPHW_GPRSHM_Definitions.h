/****************************************************************************/
/**
 *  CPHW_GPRSHM_Definitions.h
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
 *  This file contains the common constants, structures,...
 */

/**
 *  Revision History:
 *  -----------------
 *  2015-01-06  xtuangu  Created the prototype version.
 *  2016-12-08  xdtkebo  Clean-up of the source code.
 *  2017-02-10  xjoschu  Added partition recovery.
 */

#ifndef _CPHW_GPRSHM_DEFINITIONS_H
#define _CPHW_GPRSHM_DEFINITIONS_H

#ifndef SUPPRESS_UNUSED_MESSAGE
#define SUPPRESS_UNUSED_MESSAGE(_x) do { break; } while (_x)
#endif

#define SUPERVISION_TIME 1000 ///< 1000 miliseconds
#define SUPERVISION_TIME_GPRCACHE 10000 ///< 10 seconds
#define BASE_GPR_ADDRESS 8

#define MAX_CP 64
#define MAX_MSG_DATA 4096

#define MSG_ID_SIZE        1
#define PROT_VER_SIZE      1
#define MSG_LEN_SIZE       2
#define SIZEOFHEADER       (MSG_ID_SIZE + PROT_VER_SIZE + MSG_LEN_SIZE)

#define MSG_ID_OFFSET      0
#define PROT_VER_OFFSET    MSG_ID_OFFSET + MSG_ID_SIZE
#define MSG_LEN_OFFSET     PROT_VER_OFFSET + PROT_VER_SIZE

#define CPID_SIZE            4
#define GPR_ADDR_SIZE        4
#define SENDER_CPID_OFFSET   SIZEOFHEADER
#define TARGET_CPID_OFFSET   SENDER_CPID_OFFSET + CPID_SIZE
#define GPR_ADDR_OFFSET      TARGET_CPID_OFFSET + CPID_SIZE
#define GPR_VALUE_OFFSET     GPR_ADDR_OFFSET + GPR_ADDR_SIZE

/* For GPR_READ_RES message*/
#define CPID_SIZE            4
#define GPR_ADDR_SIZE        4
#define GPR_RESULT_SIZE      4
#define SENDER_CPID_OFFSET   SIZEOFHEADER
#define TARGET_CPID_OFFSET   SENDER_CPID_OFFSET + CPID_SIZE
#define GPR_ADDR_OFFSET      TARGET_CPID_OFFSET + CPID_SIZE
#define GPR_RESULT_OFFSET    GPR_ADDR_OFFSET + GPR_ADDR_SIZE
#define GPR_RESVALUE_OFFSET  GPR_RESULT_OFFSET + GPR_RESULT_SIZE

#define SENDER_CPID_OFFSET   SIZEOFHEADER
#define TARGET_CPID_OFFSET   SENDER_CPID_OFFSET + CPID_SIZE
#define GPR_ADDR_OFFSET      TARGET_CPID_OFFSET + CPID_SIZE
#define GPR_VALUE_OFFSET     GPR_ADDR_OFFSET + GPR_ADDR_SIZE

#define CPID_SIZE 4
#define GPRCACHE_LEN_SIZE  2   //Size of the length of one GPR entry in GPR cache message
#define GPR_VALUE_SIZE     4   //SIZE of a GPR value
#define NO_OF_GPR          8   //Support 8 GPRs currently
#define GPRCACHE_LEN_PER_BLADE  (NO_OF_GPR * GPR_VALUE_SIZE)

#define WRITE_GPR_LEN (SIZEOFHEADER + CPID_SIZE + CPID_SIZE + GPR_ADDR_SIZE + GPR_VALUE_SIZE)
#define READ_GPR_LEN (SIZEOFHEADER + CPID_SIZE + CPID_SIZE + GPR_ADDR_SIZE)
#define READ_GPR__RSP_LEN (SIZEOFHEADER + CPID_SIZE + CPID_SIZE + GPR_ADDR_SIZE + GPR_RESULT_SIZE + GPR_VALUE_SIZE)

/**
 * End point ID
 */
namespace vGPRShlfMgrEndPoint
{
    enum EndPoint
    {
       BC0 = 0, ///< BC id for BC0
       BC1,
       BC2,
       BC3,
       BC4,
       BC5,
       BC6,
       BC7,
       BC8,
       BC9,
       BC10,
       BC11,
       BC12,
       BC13,
       BC14,
       BC15,
       BC16,
       BC17,
       BC18,
       BC19,
       BC20,
       BC21,
       BC22,
       BC23,
       BC24,
       BC25,
       BC26,
       BC27,
       BC28,
       BC29,
       BC30,
       BC31,
       BC32,
       BC33,
       BC34,
       BC35,
       BC36,
       BC37,
       BC38,
       BC39,
       BC40,
       BC41,
       BC42,
       BC43,
       BC44,
       BC45,
       BC46,
       BC47,
       BC48,
       BC49,
       BC50,
       BC51,
       BC52,
       BC53,
       BC54,
       BC55,
       BC56,
       BC57,
       BC58,
       BC59,
       BC60,
       BC61,
       BC62,
       BC63,
       APA,
       APB,
       NumberOfEndPoints,
       Tester = NumberOfEndPoints
    };

};

namespace vGPRShlfMgrErrorCodes
{
    enum GPRMessageErrorCodes
    {
       SUCCESS = 0,
       INCORRECT_GPR_ADDRESS
    };
};

#endif /* _CPHW_GPRSHM_DEFINITIONS_H */

