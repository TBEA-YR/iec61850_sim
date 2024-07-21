/*
 *  server_example_config_file.c
 *
 *  This example shows how to use dynamic server data model with a configuration file.
 *
 *  - How to open and parse the model configuration file
 *  - How to access data attributes by object reference strings
 *  - How to access data attributes by short addresses
 *
 *  Note: If building with cmake the vmd-filestore folder containing the configuration file
 *  (model.cfg) has to be copied to the folder where the example is executed!
 *  The configuration file can be created from the SCL(ICD) file with the Java tool genconfig.jar
 *  that is included in the source distribution of libiec61580.
 *
 */

#include "iec61850_server.h"
#include "hal_thread.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "iec61850_config_file_parser.h"
#include "PCSServerSimThread.h"
#include "BMSServerSimThread.h"
#include "SimpleIni.h"

static int running = 0;

void sigint_handler(int signalId)
{
	running = 0;
}

ServerSimThreadBaseSeq createPCSSim(const std::string &strCfgPath,const int nStartPort,const int nCount,const int nChangePeriodSec)
{
    SIEC61850ServerParam stParam;
    stParam.strCfgPath = strCfgPath;
    stParam.nChangePeriodMs = nChangePeriodSec * 1000;

    ServerSimThreadBaseSeq vecThread;

    for (int i = nStartPort; i < nStartPort + nCount; i++)
    {
        stParam.nPort = i;
        CServerSimThreadBase *pSimThread = new CPCSServerSimThread(stParam);

        if (pSimThread == NULL)
        {
            continue;
        }

        if (!pSimThread->init())
        {
            delete pSimThread;
            pSimThread = NULL;
            continue;
        }

        vecThread.push_back(pSimThread);
    }

    for (size_t nThreadIdx = 0; nThreadIdx < vecThread.size(); nThreadIdx++)
    {
        if (!vecThread[nThreadIdx]->start())
        {
            delete vecThread[nThreadIdx];
            vecThread[nThreadIdx] = NULL;
        }
    }

    return vecThread;
}


ServerSimThreadBaseSeq createBMSSim(const std::string &strCfgPath, const int nStartPort, const int nCount, const int nChangePeriodSec)
{
    SIEC61850ServerParam stParam;
    stParam.strCfgPath = strCfgPath;
    stParam.nChangePeriodMs = nChangePeriodSec * 1000;

    ServerSimThreadBaseSeq vecThread;

    for (int i = nStartPort; i < nStartPort + nCount; i++)
    {
        stParam.nPort = i;
        CServerSimThreadBase *pSimThread = new CBMSServerSimThread(stParam);

        if (pSimThread == NULL)
        {
            continue;
        }

        if (!pSimThread->init())
        {
            delete pSimThread;
            pSimThread = NULL;
            continue;
        }
        vecThread.push_back(pSimThread);
    }

    for (size_t nThreadIdx = 0; nThreadIdx < vecThread.size(); nThreadIdx++)
    {
        if (!vecThread[nThreadIdx]->start())
        {
            delete vecThread[nThreadIdx];
            vecThread[nThreadIdx] = NULL;
        }
    }

    return vecThread;
}

int main(int argc, char** argv)
{
    std::string strIniFilePath = "IEC61850Sim.ini";
    if (argc > 1)
    {
        strIniFilePath = argv[1];
    }

    CSimpleIniA ini;
    SI_Error rc = ini.LoadFile(strIniFilePath.c_str());
    if (rc < 0)
    {
        printf("¼ÓÔØ %s Ê§°Ü\n", strIniFilePath.c_str());
        return -1;
    }

    std::string strPCSCfgPath = ini.GetValue("PCS", "cfg_path");
    int nPCSPort = static_cast<int>(ini.GetLongValue("PCS", "start_port"));
    int nPCSNum = static_cast<int>(ini.GetLongValue("PCS", "num"));
    int nPCSChangePeriodSec = static_cast<int>(ini.GetLongValue("PCS", "change_period"));
    
    std::string strBMSCfgPath = ini.GetValue("BMS", "cfg_path");
    int nBMSPort = static_cast<int>(ini.GetLongValue("BMS", "start_port"));
    int nBMSNum = static_cast<int>(ini.GetLongValue("BMS", "num"));
    int nBMSChangePeriodSec = static_cast<int>(ini.GetLongValue("BMS", "change_period"));

    ServerSimThreadBaseSeq vecAllThread;

    ServerSimThreadBaseSeq vecBMSThread = createBMSSim(strBMSCfgPath,nBMSPort, nBMSNum, nPCSChangePeriodSec);
    ServerSimThreadBaseSeq vecPCSThread = createPCSSim(strPCSCfgPath,nPCSPort, nPCSNum, nBMSChangePeriodSec);

    vecAllThread.insert(vecAllThread.end(), vecBMSThread.begin(), vecBMSThread.end());
    vecAllThread.insert(vecAllThread.end(), vecPCSThread.begin(), vecPCSThread.end());

    running = 1;
    signal(SIGINT, sigint_handler);

    while (running)
    {
        Thread_sleep(1 * 1000);
    }

    for (size_t i = 0;i < vecAllThread.size();i++)
    {
        if (vecAllThread[i] != NULL)
        {
            delete vecAllThread[i];
            vecAllThread[i] = NULL;
        }
    }

    return 0;
} /* main() */
