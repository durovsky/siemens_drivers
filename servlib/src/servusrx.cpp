/****************************************************************************
*  Copyright (C) SIEMENS CORP., 2015 All rights reserved.
*****************************************************************************
* FILE NAME    : servuser.cpp
* ---------------------------------------------------------------------------
* DESCRIPTION  : local download user interface functions
*****************************************************************************/

/* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: */
/* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.                */
/* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer              */
/* in the documentation and/or other materials provided with the distribution.                                                                    */
/* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived               */
/* from this software without specific prior written permission.                                                                                  */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,                  */
/* BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT                 */
/* SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        */
/* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)          */
/* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)              */
/* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                         */


#include "os.h"
#include "tracemcrm.h"
#include "servusrx.h"

/* external functions */
DPR_UINT32 ldah_download_firmware(DPR_UINT32 cp_idx, DPR_CHAR *p_data, DPR_UINT32 data_length);
DPR_UINT32 ldah_download_config(DPR_UINT32 cp_idx, DPR_CHAR *p_data, DPR_UINT32 data_length);
DPR_UINT32 ldah_reset_firmware(DPR_UINT32 cp_idx, DPR_CHAR type);
DPR_UINT32 ldah_cp_set_time (DPR_UINT32 CpIndex, PNIO_CP_SET_TIME_TYPE *pCpTime);
DPR_UINT32 ldah_cp_set_type_of_station(PNIO_UINT32 CpIndex, char* TypeName);
DPR_UINT32 ldah_serv_fw_info(DPR_UINT32 cp_idx, SERV_CP_FW_INFO_TYPE* p_info);

PNIO_UINT32 cpinfo_open(PNIO_UINT32 cp_idx, PNIO_UINT32 * handle, PNIO_UINT32 reserved);
PNIO_UINT32 cpinfo_close(PNIO_UINT32 handle);
PNIO_UINT32 cpinfo_reg_cbf(PNIO_UINT32 handle, SERV_CP_INFO_TYPE info_type, SERV_CP_INFO_CBF cbf,
                                           PNIO_UINT32 reserved);

DPR_MUTEX ServMutexDownload;
DPR_MUTEX ServMutexCpInfo;

/* initiate user trace, read trace settings */

CTrmWrapper ti;

static class StaticInitServlib {
public:
  StaticInitServlib(void) {
    DPR_MUTEX_CREATE_UNLOCKED(ServMutexDownload);
    DPR_MUTEX_CREATE_UNLOCKED(ServMutexCpInfo);
    TRM_INIT_FROM_FILE(&ti, "servtrace.conf");
  };

  ~StaticInitServlib(void) {
    TRM_DEINIT(&ti);
  };
} static_init_servlib;

/*-----------------------------------------------------------------------------
 * Name  : SERV_CP_set_time
 * Descr :
 * Param :
 *  [ IN]:
 *  [OUT]:
 * Return: ==0: ok, != 0 error
 */
PNIO_UINT32 PNIO_CODE_ATTR
SERV_CP_set_time ( PNIO_UINT32 CpIndex, PNIO_CP_SET_TIME_TYPE *pCpTime )
{
    PNIO_UINT32 Ret = PNIO_OK;

    DPR_MUTEX_LOCK(ServMutexDownload);
    TRM_OUT00(&ti, GR_INIT, LV_INFO, "->SERV_CP_set_time");

    Ret = (PNIO_UINT32)ldah_cp_set_time(CpIndex, pCpTime);

    TRM_OUT01(&ti, GR_INIT, LV_INFO, "<-SERV_CP_set_time Ret=%d",Ret);
    DPR_MUTEX_UNLOCK(ServMutexDownload);

    return Ret;

} /* end of SERV_CP_set_time */

/*-----------------------------------------------------------------------------
 * Name  : SERV_CP_set_type_of_station
 * Descr : Writes a given "type_of_station" TypeOfStation name into the flash.
 *         Firmware Reboot is necessary for new type takes effect.
 * Param :
 *  [ IN]:
 * Return: ==0: PNIO_OK, != 0 PNIO_ERR_xxx
 */
PNIO_UINT32 PNIO_CODE_ATTR
SERV_CP_set_type_of_station (PNIO_UINT32 CpIndex, char* TypeName)
{
    PNIO_UINT32 Ret = PNIO_OK;

    DPR_MUTEX_LOCK(ServMutexDownload);
    TRM_OUT01(&ti, GR_INIT, LV_INFO, "->SERV_CP_set_type_of_station: new type='%s'",TypeName);

    Ret = (PNIO_UINT32)ldah_cp_set_type_of_station(CpIndex, TypeName);

    TRM_OUT01(&ti, GR_INIT, LV_INFO, "<-SERV_CP_set_type_of_station: Ret=%d",Ret);
    DPR_MUTEX_UNLOCK(ServMutexDownload);

    return Ret;

} /* end of SERV_CP_set_type_of_station */

/*-----------------------------------------------------------------------------
 * Name  : SERV_CP_download
 * Descr : Downloads S7-Configuration-Data or Firmware
 */

PNIO_UINT32 PNIO_CODE_ATTR
SERV_CP_download (PNIO_UINT32 CpIndex, PNIO_CP_DLD_TYPE DataType,
                  PNIO_UINT8 * pData, PNIO_UINT32 DataLen)
{
  PNIO_UINT32 Ret = PNIO_OK;

  DPR_MUTEX_LOCK(ServMutexDownload);

  TRM_OUT00(&ti, GR_INIT, LV_FCTPUB1, "->SERV_CP_download");

  TRM_IF_ON_EXPR(&ti, GR_INIT, LV_FCTPUB2,
                 OSTREAM trcos;
                trcos << showbase << hex;
                trcos << "  CpIndex=" << CpIndex;
                trcos << ", DataType=" << DataType;
                trcos << ", DataLen=" << (unsigned long)DataLen;
                trcos << ends;
                TRM_OUT_OBJECT(&ti, GR_INIT, LV_FCTPUB2, trcos);
                );

  switch(DataType) {
  case PNIO_CP_DLD_FIRMWARE:
    Ret = ldah_download_firmware(CpIndex, pData, DataLen);
    break;
  case PNIO_CP_DLD_CONFIG:
    Ret = ldah_download_config(CpIndex, pData, DataLen);
    break;
  default:
    Ret = PNIO_ERR_PRM_TYPE;
    break;
  }

  TRM_IF_ON_EXPR(&ti, GR_INIT, LV_FCTPUB1,
                 OSTREAM trcos1;
                trcos1 << showbase << hex;
                trcos1 << "<-SERV_CP_download, ret=" << Ret;
                trcos1 << ends;
                TRM_OUT_OBJECT(&ti, GR_INIT, LV_FCTPUB1, trcos1);
                );

  DPR_MUTEX_UNLOCK(ServMutexDownload);

  return Ret;
} /* end of SERV_CP_download */

/*-----------------------------------------------------------------------------
 * Name  : SERV_CP_reset
 * Descr :
 */
PNIO_UINT32 PNIO_CODE_ATTR SERV_CP_reset (PNIO_UINT32              CpIndex,
                                          PNIO_CP_RESET_TYPE       ResetType)
{
  PNIO_UINT32 Ret = PNIO_OK;

  DPR_MUTEX_LOCK(ServMutexDownload);

  TRM_OUT00(&ti, GR_INIT, LV_FCTPUB1, "->SERV_CP_reset");

  TRM_IF_ON_EXPR(&ti, GR_INIT, LV_FCTPUB2,
                 OSTREAM trcos;
                trcos << showbase << hex;
                trcos << "  CpIndex=" << CpIndex;
                trcos << ", ResetType=" << ResetType;
                trcos << ends;
                TRM_OUT_OBJECT(&ti, GR_INIT, LV_FCTPUB2, trcos);
                );

  switch(ResetType) {
  case PNIO_CP_RESET_SAFE:
    Ret = ldah_reset_firmware(CpIndex, 0);
    break;
  case PNIO_CP_RESET_FORCE:
    Ret = ldah_reset_firmware(CpIndex, 1);
    break;
  default:
    Ret = PNIO_ERR_PRM_TYPE;
    break;
  }

  TRM_IF_ON_EXPR(&ti, GR_INIT, LV_FCTPUB1,
                 OSTREAM trcos1;
                trcos1 << showbase << hex;
                trcos1 << "<-SERV_CP_reset, ret=" << Ret;
                trcos1 << ends;
                TRM_OUT_OBJECT(&ti, GR_INIT, LV_FCTPUB1, trcos1);
                );

  DPR_MUTEX_UNLOCK(ServMutexDownload);

  return Ret;
} /* end of SERV_CP_reset */

PNIO_UINT32 PNIO_CODE_ATTR SERV_CP_info_open (PNIO_UINT32  CpIndex,
                                                PNIO_UINT32  * Handle, PNIO_UINT32 Reserved)
{
  PNIO_UINT32 Ret = PNIO_OK;

  DPR_MUTEX_LOCK(ServMutexCpInfo);

  TRM_OUT00(&ti, GR_INIT, LV_FCTPUB1, "->SERV_CP_info_open");

  TRM_IF_ON_EXPR(&ti, GR_INIT, LV_FCTPUB2,
                 OSTREAM trcos;
                trcos << showbase << hex;
                trcos << "  CpIndex=" << CpIndex;
                trcos << ends;
                TRM_OUT_OBJECT(&ti, GR_INIT, LV_FCTPUB2, trcos);
                );

  Ret = cpinfo_open(CpIndex, Handle, Reserved);

  TRM_IF_ON_EXPR(&ti, GR_INIT, LV_FCTPUB1,
                 OSTREAM trcos1;
                trcos1 << showbase << hex;
                trcos1 << "<-SERV_CP_info_open, ret=" << Ret;
                trcos1 << ends;
                TRM_OUT_OBJECT(&ti, GR_INIT, LV_FCTPUB1, trcos1);
                );

  DPR_MUTEX_UNLOCK(ServMutexCpInfo);

  return Ret;
}

PNIO_UINT32 PNIO_CODE_ATTR SERV_CP_info_close ( PNIO_UINT32  Handle)
{
  PNIO_UINT32 Ret = PNIO_OK;

  DPR_MUTEX_LOCK(ServMutexCpInfo);

  TRM_OUT00(&ti, GR_INIT, LV_FCTPUB1, "->SERV_CP_info_close");

  TRM_IF_ON_EXPR(&ti, GR_INIT, LV_FCTPUB2,
                 OSTREAM trcos;
                trcos << showbase << hex;
                trcos << "  Handle=" << Handle;
                trcos << ends;
                TRM_OUT_OBJECT(&ti, GR_INIT, LV_FCTPUB2, trcos);
                );

  Ret = cpinfo_close(Handle);

  TRM_IF_ON_EXPR(&ti, GR_INIT, LV_FCTPUB1,
                 OSTREAM trcos1;
                trcos1 << showbase << hex;
                trcos1 << "<-SERV_CP_info_close, ret=" << Ret;
                trcos1 << ends;
                TRM_OUT_OBJECT(&ti, GR_INIT, LV_FCTPUB1, trcos1);
                );

  DPR_MUTEX_UNLOCK(ServMutexCpInfo);

  return Ret;
}

PNIO_UINT32 PNIO_CODE_ATTR SERV_CP_info_register_cbf(PNIO_UINT32 Handle,
                                          SERV_CP_INFO_TYPE InfoType, SERV_CP_INFO_CBF Cbf,
                                           PNIO_UINT32  Reserved)
{
  PNIO_UINT32 Ret = PNIO_OK;

  DPR_MUTEX_LOCK(ServMutexCpInfo);

  TRM_OUT00(&ti, GR_INIT, LV_FCTPUB1, "->SERV_CP_info_register_cbf");

  TRM_IF_ON_EXPR(&ti, GR_INIT, LV_FCTPUB2,
                 OSTREAM trcos;
                trcos << showbase << hex;
                trcos << "  Handle=" << Handle;
                trcos << ", InfoType=" << InfoType;
                trcos << ", Cbf=" << Cbf;
                trcos << ends;
                TRM_OUT_OBJECT(&ti, GR_INIT, LV_FCTPUB2, trcos);
                );

  Ret = cpinfo_reg_cbf(Handle, InfoType, Cbf, Reserved);

  TRM_IF_ON_EXPR(&ti, GR_INIT, LV_FCTPUB1,
                 OSTREAM trcos1;
                trcos1 << showbase << hex;
                trcos1 << "<-SERV_CP_info_register_cbf, ret=" << Ret;
                trcos1 << ends;
                TRM_OUT_OBJECT(&ti, GR_INIT, LV_FCTPUB1, trcos1);
                );

  DPR_MUTEX_UNLOCK(ServMutexCpInfo);

  return Ret;
}


/*-----------------------------------------------------------------------------
 * Name  : SERV_CP_get_fw_info
 * Descr : This function was introduce to solve RQ=AP01402756
 *         This diagnostic info function can be used independet of the cp mode
 *         'device' or 'controller'.
 * Param :
 *  [ IN]: CpIndex, id of the cp board. only one board is supported
 *         => CpIndex is always =1
 *  [OUT]: SERV_CP_FW_INFO_TYPE: Structure with config parameters delivered by the
 *         firmware
 * Return: PNIO_OK: if ok, or PNIO_ERR_xxx if failed (see pnioerrx.h)
 *         PNIO_ERR_NO_FW_COMMUNICATION
 */
PNIO_UINT32 PNIO_CODE_ATTR SERV_CP_get_fw_info (PNIO_UINT32 CpIndex, SERV_CP_FW_INFO_TYPE* p_info)
{
    PNIO_UINT32 Ret = PNIO_OK;

    TRM_OUT00(&ti, GR_INIT, LV_INFO, "->SERV_CP_get_fw_info");

    Ret = (PNIO_UINT32)ldah_serv_fw_info(CpIndex, p_info);

    TRM_OUT01(&ti, GR_INIT, LV_INFO, "<-SERV_CP_get_fw_info: Ret=%d",Ret);

    return Ret;


} /* end of SERV_CP_get_fw_info */

