/*---------------------------------------------------------------------------*/
/* Copyright (C) SIEMENS CORP., 2015 All rights reserved.*/
/*---------------------------------------------------------------------------*/
/*  This program is protected by German copyright law and international      */
/*  treaties.                                                                */
/*  The use of this software including but not limited to its Source Code    */
/*  is subject to restrictions as agreed in the license agreement between    */
/*  you and Siemens.                                                         */
/*  Copying or distribution is not allowed unless expressly permitted        */
/*  according to your license agreement with Siemens.                        */
/*---------------------------------------------------------------------------*/
/*  Project             : PROFInet Runtime Software                          */
/*  File                : tst_app_cfg.h                                      */
/*  Version             : V1.0                                               */
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
/*---------------------------------------------------------------------------*/
/*  D e s c r i p t i o n :                                                  */
/*  A few data for configuration of the simulated device                    */
/*  THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER                          */
/*---------------------------------------------------------------------------*/
/*****************************************************************************/


#define CP_INDEX 1 /* Unique identification for the communication module
                      (module index in the component configuration) */

#define FW_Version_V26	// Wenn FW_Version <= 2.5 dann V25 eintragen sonst V26	

#ifdef FW_Version_V26

	#define ANNOT_NAME       "StarterKit"   /* device type (String 25) */
	#define ANNOT_ORDERID    "6GK1 161-6AA00"         /* Order Id    (String 20) */
	#define ANNOT_HW_REV     0              /* HwRevision  (short)     */
	#define ANNOT_SW_PREFIX  'V'            /* SwRevisionPrefix (char) */
	#define ANNOT_SW_REV_1   2              /* SwRevision1 (short)     */
	#define ANNOT_SW_REV_2   6             /* SwRevision2 (short)     */
	#define ANNOT_SW_REV_3   0              /* SwRevision3 (short)     */

	/***************************************************/
	/* Values for PNIO_device_open                     */
	/***************************************************/

	#define NUM_OF_SLOTS 2
	#define VENDOR_ID    0x002a
	#define DEVICE_ID    0x0003
	#define INSTANCE_ID  0x0001
	
	/*  mod         submod  modId   subId   Api */
	#define DEVICE_DATA \
	   { 1,      1,         0x0019, 0x010001, 0x00, 0, 0, 0, 0 }, \
	   { 2,      1,         0x0020, 0x0001, 0x00, 0, 0, 0, 0 }

#define DEVICE_DATA_ENTRIES 2 /* The total number of members of DEVICE_DATA structure */

#else
	#define ANNOT_NAME       "StarterKit"   /* device type (String 25) */
	#define ANNOT_ORDERID    "6GK1 161-6AA00"         /* Order Id    (String 20) */
	#define ANNOT_HW_REV     0              /* HwRevision  (short)     */
	#define ANNOT_SW_PREFIX  'V'            /* SwRevisionPrefix (char) */
	#define ANNOT_SW_REV_1   2              /* SwRevision1 (short)     */
	#define ANNOT_SW_REV_2   5             /* SwRevision2 (short)     */
	#define ANNOT_SW_REV_3   0              /* SwRevision3 (short)     */

	/***************************************************/
	/* Values for PNIO_device_open                     */
	/***************************************************/

	#define NUM_OF_SLOTS 2
	#define VENDOR_ID    0x002a
	#define DEVICE_ID    0x0003
	#define INSTANCE_ID  0x0001
	
	/*  mod         submod  modId   subId   Api */
	#define DEVICE_DATA \
	   { 0,      1,         0x0017, 0x010001, 0x00, 0, 0, 0, 0 }, \
	   { 1,      1,         0x0020, 0x0001, 0x00, 0, 0, 0, 0 }

	#define DEVICE_DATA_ENTRIES 2 /* The total number of members of DEVICE_DATA structure */
#endif

/**************************************************/
/* Known Diagnose alarms for this device          */
/**************************************************/

#define CH_ERR_INVAL_LINKUP     0x0100
#define CH_ERR_INVAL_LINKDOWN   0x0101
#define CH_ERR_NO_REDUND_PS     0x0200
#define CH_ERR_NO_CPLUG         0x0201
#define CH_ERR_CPLUG_ERROR      0x0202


/*****************************************************/
/* help data for plugging of modules and sub modules */
/*****************************************************/
typedef struct device_data_s
{
    int slot;
    int subslot;
    PNIO_UINT32 modId;
    PNIO_UINT32 subslotId;
    PNIO_UINT32 api;
    PNIO_UINT16 maxSubslots;      /* for internal use. set this to 0 */
    int modState;                 /* for internal use. set this to 0 */
    int subState;                 /* for internal use. set this to 0 */
    int dir;                      /* for internal use, set this to 0 */
} device_data_t;


/* In order to tell the application how many modules and
 * submodules are known, the above structure and the data
 * described by DEVICE_DATA have to be defined here.
 *
 * If you want more than one submodule for a slot, file DEVICE_DATA
 * as following:
 * { 0, 1, 0x0001, 0x001, 0x00, 0, 0, 0, 0 }, // device itself
 * { 1, 1, 0x0020, 0x001, 0x00, 0, 0, 0, 0 }, // module 1 subslot 1
 * { 1, 3,      0, 0x001, 0x00, 0, 0, 0, 0 }, // module 1 subslot 3
 * ...
 * The module Id field must be declared for the first entry where
 * this module number appears.
 *
 */



/* Interne Verwaltungsstrukturen */

typedef struct IM_PERMANENT_tag { /*size: 142*/
    PNIO_UINT16     VendorID;
    PNIO_UINT16     OrderID;
    PNIO_UINT8      pIM_Serial_Number[16];
    PNIO_UINT16     IM_Hardware_Revision;
    PNIO_UINT8      IM_Software_Revision[4];
    PNIO_UINT16     IM_Profile_ID;
    PNIO_UINT16     IM_Profile_Specific_Type;
    PNIO_UINT16     IM_Version;
    PNIO_UINT16     IM_Supported;
    PNIO_UINT8      IM_Tag_Function[32];
    PNIO_UINT8      IM_Tag_Location[22];
    PNIO_UINT8      IM_Date[16];
    PNIO_UINT8      IM_Descriptor[54];
    PNIO_UINT8      IM_Signature[54];
} IM_PERMANENT;




/*
 * Anlegen und Ausf�llen eines neuen permanenten Speicherbereich f�r einen IM Recordset 
 * auf der Baugruppe  
 * Beschreiben des Speicherbereichs mit IM-Daten
 * Die IM-Daten werden auch nach Ziehen/Stecken des Submoduls, Beenden/Neustarten der 
 * Applikation und Baugruppenneustart einem READ-Record-Request �bergeben.
 */
PNIO_UINT32 PNIO_imstore_new(PNIO_DEV_ADDR Addr, IM_PERMANENT *pImSet );

/* 
 * L�schen eines permanenten Speicherbereichs f�r einen IM Recordset auf der Baugruppe. 
 * Falls Module getauscht oder permanent entfernt werden.
 * Wird als Pointer auf die Submoduladresse NULL �bergeben, werden alle 
 * IM-Recordsets permanent entfernt und auf der Baugruppe gel�scht.
 */
PNIO_UINT32 PNIO_imstore_delete(PNIO_DEV_ADDR *pAddr);

/* 
 * Lesen eines permanenten IM-Recordsets 
 * (Wird f�r Applikationsseitige Debugausgaben ben�tigt)
 */
PNIO_UINT32 PNIO_imstore_read(PNIO_DEV_ADDR addr, IM_PERMANENT *pImSet);