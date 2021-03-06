/****************************************************************************
*  Copyright (C) SIEMENS CORP., 2015 All rights reserved.
*****************************************************************************
* FILE NAME    : traceout.cpp
* ---------------------------------------------------------------------------
* DESCRIPTION  : trace module.
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
/*****************************************************************************/
#define NOTRC
#include "traceout.h"
#include "traceglobal.h"

CTraceOut::CTraceOut(void)
{
    m_pFile = NULL;
    m_lFileCurrentEntries = 0;
    m_lCurrentBackFiles = 0;
    strcpy(m_szFilename, "\0");

    DPR_MUTEX_CREATE_UNLOCKED(m_hMutex);

    m_ulDepth = TRACEDEPTH_DEFAULT;
    m_ulGroup = TRACEGROUP_ALL;
    m_lDest = TRACEDEST_NOTRACE;
    m_lFileFast = 0;
    m_lFileEntries = 10000;

    m_lMaxBackFiles = 1;
    m_lTime = TRACETIME_ON;
    m_lPID = DPR_GET_CURRENT_PROCESS_ID();
}

CTraceOut::~CTraceOut(void)
{
    if(m_pFile) {
        fclose(m_pFile);
    }

    DPR_MUTEX_DESTROY(m_hMutex);
}

long CTraceOut::GetFileName(char *szFilename)
{
    long hr = 0;

    Lock();
    strcpy(szFilename, m_szFilename);
    Unlock();

    return hr;
}

long CTraceOut::SetFileName(char *szFilename)
{
    long hr = 0;

    Lock();

    if(strlen(szFilename) < TRACEMAXNAMELENGTH) {
        if(m_pFile) {
            fclose(m_pFile);
            m_pFile = NULL;
        }

        strncpy(m_szFilename, szFilename, TRACEMAXNAMELENGTH);
    } else {
        hr = -1;
    }

    Unlock();

    return hr;
}

long CTraceOut::GetDest(unsigned long &lDest)
{
    long hr = 0;

    Lock();
    lDest = m_lDest;
    Unlock();

    return hr;
}

long CTraceOut::SetDest(unsigned long lDest)
{
    long hr = 0;

    Lock();
    m_lDest = lDest;
    Unlock();

    return hr;
}

long CTraceOut::GetFileFast(long &lFileFast)
{
    long hr = 0;

    Lock();
    lFileFast = m_lFileFast;
    Unlock();

    return hr;
}

long CTraceOut::SetFileFast(long lFileFast)
{
    long hr = 0;

    Lock();

    if(m_szFilename) {
        if(m_lFileFast != lFileFast) {
            m_lFileFast = lFileFast;

            switch (m_lFileFast) {
            case TRACEFILEFAST_ON:
                if(!m_pFile)
                    m_pFile = fopen(m_szFilename, "a");

                break;

            case TRACEFILEFAST_OFF:
                if(m_pFile)
                    fclose(m_pFile);

                m_pFile = NULL;
                break;

            default:
                hr = -1;
                break;
            }
        }
    } else {
        hr = -1;
    }

    Unlock();

    return hr;
}

long CTraceOut::GetGroup(unsigned long &ulGroup)
{
    long hr = 0;

    Lock();
    ulGroup = m_ulGroup;
    Unlock();

    return hr;
}

long CTraceOut::SetGroup(unsigned long ulGroup)
{
    long hr = 0;

    Lock();
    m_ulGroup = ulGroup;
    Unlock();

    return hr;
}

long CTraceOut::GetMaxBackFiles(long &lMaxBackFiles)
{
    long hr = 0;

    Lock();
    lMaxBackFiles = (m_lMaxBackFiles > 3) ? m_lMaxBackFiles : 3;
    Unlock();

    return hr;
}

long CTraceOut::SetMaxBackFiles(long lMaxBackFiles)
{
    long hr = 0;

    Lock();
    m_lMaxBackFiles = lMaxBackFiles;
    Unlock();

    return hr;
}

long CTraceOut::GetDepth(unsigned long &ulDepth)
{
    long hr = 0;

    Lock();
    ulDepth = m_ulDepth;
    Unlock();

    return hr;
}

long CTraceOut::SetDepth(unsigned long ulDepth)
{
    long hr = 0;

    Lock();

    m_ulDepth = ulDepth;

    Unlock();

    return hr;
}

long CTraceOut::Rename(long lIndex)
{
    char szFile[TRACEMAXNAMELENGTH];
    char szOldFile[TRACEMAXNAMELENGTH];
    char szFileBase[TRACEMAXNAMELENGTH];

    FILE *fp;

    strcpy(szFileBase, m_szFilename);
    char *szFound = strrchr(szFileBase, '.');
    if(szFound != 0)
        *szFound = '\0';
    else
        strcpy(szFileBase, "Trace");

    snprintf(szFile, TRACEMAXNAMELENGTH - 1, "%s_%ld.old", szFileBase, lIndex);
    szFile[TRACEMAXNAMELENGTH - 1] = 0;

    fp = fopen(szFile, "r");
    if(NULL != fp) {            /* file exists */
        fclose(fp);
        fp = NULL;

        if(lIndex == m_lMaxBackFiles) {
            (void)remove(szFile);
        } else {
            Rename(lIndex + 1);
        }
    }

    if(lIndex > 1) {
        snprintf(szOldFile, TRACEMAXNAMELENGTH - 1, "%s_%ld.old", szFileBase, lIndex - 1);
        szOldFile[TRACEMAXNAMELENGTH - 1] = 0;
        (void)rename(szOldFile, szFile);
    }

    return 0;
}

long CTraceOut::Rescue(void)
{
    long hr = 0;

    FILE *fp;

    if(m_lMaxBackFiles > 0) {

        fp = fopen(m_szFilename, "r");
        if(NULL != fp) { /* file exists */
            char szFile[TRACEMAXNAMELENGTH];
            char szFileBase[TRACEMAXNAMELENGTH];

            fclose(fp);
            fp = NULL;

            strcpy(szFileBase, m_szFilename);
            char *szFound = strrchr(szFileBase, '.');
            if(szFound != 0) {
                *szFound = '\0';
            } else {
                strcpy(szFileBase, "Trace");
            }

            Rename(1);

            snprintf(szFile, TRACEMAXNAMELENGTH - 1, "%s_%d.old", szFileBase, 1);
            szFile[TRACEMAXNAMELENGTH - 1] = 0;
            (void)rename(m_szFilename, szFile);
        }
    } else {
        (void)remove(m_szFilename);
    }

    return hr;
}

void CTraceOut::OutTracePrivate(unsigned long TraceGroup, unsigned long TraceDepth, const char *szMsg)
{
    char szTime[TRACEMAXNAMELENGTH] = "";
    FILE *fp;
    long lThreadID;

    if(m_lTime == TRACETIME_ON) {
        TRC_GetFormattedLocalTime(szTime, TRACEMAXNAMELENGTH);
    }

    lThreadID = DPR_GET_CURRENT_THREAD_ID();

    if((m_lDest & TRACEDEST_NEWFILE) || (m_lDest & TRACEDEST_SAMEFILE)) {
        FILE *pFile = NULL;

        if(m_lFileCurrentEntries == 0 && m_lCurrentBackFiles == 0) {
            if(m_lDest & TRACEDEST_NEWFILE) {

                if(m_pFile) {
                    fclose(m_pFile);
                    m_pFile = NULL;
                }

                fp = fopen(m_szFilename, "r");
                if(NULL != fp) { /* file exists */
                    fclose(fp);
                    fp = NULL;
                    (void)remove(m_szFilename);
                }
            }
        }

        if(m_lFileCurrentEntries > m_lFileEntries) {
            if(m_pFile) {
                fclose(m_pFile);
                m_pFile = NULL;
            }

            Rescue();

            m_lFileCurrentEntries = 0;

            if(m_lCurrentBackFiles < m_lMaxBackFiles)
                m_lCurrentBackFiles++;
        }


        if(m_lFileFast == TRACEFILEFAST_ON) {
            if(!m_pFile)
                m_pFile = fopen(m_szFilename, "a");

            pFile = m_pFile;
        } else {
            pFile = fopen(m_szFilename, "a");
        }

        if(pFile) {
            m_lFileCurrentEntries++;


            fprintf(pFile, "D:%02lx [0x%lx:0x%lx] %s: %s \n",
                TraceDepth, m_lPID, lThreadID, szTime, szMsg);

            if(m_lFileFast == TRACEFILEFAST_OFF)
                fclose(pFile);
            else
                fflush(pFile);
        } else {
            /*
             * can not open trace file, write data in OutputDebugStream
             */
            char szDebug[2*TRACEMAXNAMELENGTH] = "\0";

            snprintf(szDebug, 2*TRACEMAXNAMELENGTH, "D:%02lx [0x%lx:0x%lx] %s: %s \n",
                TraceDepth, m_lPID, lThreadID, szTime, szMsg);
            TRC_OutputDebugString(szDebug);
        }
    }

    if(m_lDest & TRACEDEST_DEBUGOUT) {
        char szDebug[2*TRACEMAXNAMELENGTH] = "\0";

        snprintf(szDebug, 2*TRACEMAXNAMELENGTH, "D:%02lx [0x%lx:0x%lx] %s: %s \n",
            TraceDepth, m_lPID, lThreadID, szTime, szMsg);
        TRC_OutputDebugString(szDebug);
    }
}

long CTraceOut::InitFromFile(const char *szFileName)
{
    unsigned long value = 0, cur_group = 0,
        cur_depth = 0, cur_dest = 0;
    char output[TRACEMAXNAMELENGTH], filepath[2*TRACEMAXNAMELENGTH];

    TRC_ExtractBegin(szFileName, filepath, sizeof(filepath));

    if(TRC_ExtractKey("TRACE_DEST", output, TRACEMAXNAMELENGTH) > 0) {
        (void)sscanf(output, "%lx", &value);
        cur_dest = value;
        SetDest(value);
    } else {
        printf("TraceLib: ERROR TRACE_DEST\n");
    }

    if(TRC_ExtractKey("TRACE_DEPTH", output, TRACEMAXNAMELENGTH) > 0) {
        (void)sscanf(output, "%lx", &value);
        cur_depth = value;
        SetDepth(value);
    } else {
        printf("TraceLib: ERROR TRACE_DEPTH\n");
    }

    value = 10000;
    if(TRC_ExtractKey("TRACE_FILE_ENTRIES", output, TRACEMAXNAMELENGTH) > 0) {
        (void)sscanf(output, "%lx", &value);
    } else {
        printf("TraceLib: ERROR TRACE_FILE_ENTRIES\n");
    }
    m_lFileEntries = (value < 200 ? 200 : value);

    if(TRC_ExtractKey("TRACE_MAX_BACK_FILES", output, TRACEMAXNAMELENGTH) > 0) {
        (void)sscanf(output, "%lx", &value);
        SetMaxBackFiles(value);
    } else {
        printf("TraceLib: ERROR TRACE_MAX_BACK_FILES\n");
    }

    if(TRC_ExtractKey("TRACE_GROUP", output, TRACEMAXNAMELENGTH) > 0) {
        (void)sscanf(output, "%lx", &value);
        cur_group = value;
        SetGroup(value);
    } else {
        printf("TraceLib: ERROR TRACE_GROUP\n");
    }

    value = TRACETIME_OFF;
    if(TRC_ExtractKey("TRACE_TIME", output, TRACEMAXNAMELENGTH) > 0) {
        (void)sscanf(output, "%lx", &value);
    } else {
        printf("TraceLib: ERROR TRACE_TIME\n");
    }
    m_lTime = (value == TRACETIME_OFF ? TRACETIME_OFF : TRACETIME_ON);

    if(TRC_ExtractKey("TRACE_FILE_FAST", output, TRACEMAXNAMELENGTH) > 0) {
        (void)sscanf(output, "%lx", &value);
        SetFileFast(value);
    } else {
        printf("TraceLib: ERROR TRACE_FILE_FAST\n");
    }


    /*
     * trace file name is passed as fun arg
     */
    if(TRC_ExtractKey("TRACE_FILE_NAME", output, TRACEMAXNAMELENGTH) > 0) {
        SetFileName(output);
    } else {
        strcpy(output, "trace.log");
        SetFileName(output);
    }

    if(cur_dest && cur_group && cur_depth) {
		//system("cd\n");
        printf("TraceLib: trace is on, and goes into %s!\n", output);
        printf("          configuration file is %s\n", filepath);
        printf("          destination 0x%lx, group 0x%lx, depth 0x%lx\n",
            cur_dest, cur_group, cur_depth);
    }

    TRC_ExtractEnd();

    return 0;
}
