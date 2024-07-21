#include "ServerSimThreadBase.h"
#include "iec61850_server.h"
#include "hal_thread.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "StringUtil.h"

#include "iec61850_config_file_parser.h"

using namespace std;

static void* ThreadProcLoop(void* threadParameter)
{
    CServerSimThreadBase *pThread = (CServerSimThreadBase*)threadParameter; //传参前this已经存在，暂不判断指针合法性
    pThread->run();
    return NULL;
}

CServerSimThreadBase::CServerSimThreadBase(const SIEC61850ServerParam &stParm)
	:m_nThreadPeriodMs(500),
    m_bIsRunning(false),
    m_stParam(stParm),
    m_iedServer(NULL),
    m_model(NULL)
{
    m_pThread = Thread_create((ThreadExecutionFunction)ThreadProcLoop, (void*)this, false);
    if (m_pThread == NULL)
    {
        printf("Thread_create is NULL.\n");
    }
}


CServerSimThreadBase::~CServerSimThreadBase()
{
    stop();
}

bool CServerSimThreadBase::start()
{
    if (m_pThread == NULL)
    {
        printf("Starting Thread failed! \n");
        return false;
    }

    Thread_start(m_pThread);
    m_bIsRunning = true;
    printf("start sim thread. iedname=%s  port=%d\n", m_model->name, m_stParam.nPort);
	return true;
}

bool CServerSimThreadBase::stop()
{
    m_bIsRunning = false;

    if (m_pThread != NULL)
    {
        Thread_destroy(m_pThread);
        m_pThread = NULL;
    }

    if (m_iedServer != NULL)
    {
        IedServer_stop(m_iedServer);
        IedServer_destroy(m_iedServer);
        m_iedServer = NULL;
    }

    if (m_model != NULL)
    {
        IedModel_destroy(m_model);
        m_model = NULL;
    }

    printf("close server.port=%d\n", m_stParam.nPort);
	return true;
}

void CServerSimThreadBase::run()
{
    while (m_bIsRunning)
    {
        simulate();
        Thread_sleep(m_nThreadPeriodMs);
    }
}

void CServerSimThreadBase::updateAttributeValue(DataAttribute * attrValue, const float & fValue)
{
    if (attrValue->fc == IEC61850_FC_ST) //bool类型
    {
        IedServer_updateBooleanAttributeValue(m_iedServer, attrValue, static_cast<bool>(fValue));
    }
    else
    {
        IedServer_updateFloatAttributeValue(m_iedServer, attrValue, fValue);
    }
}

void CServerSimThreadBase::updateAttributeValueAndTime(SNodeValues & stNodeValues, const float & fValue, const msSinceEpoch & curTimeMs)
{
    updateAttributeValue(stNodeValues.value, fValue);
    if (stNodeValues.time != NULL)
    {
        IedServer_updateUTCTimeAttributeValue(m_iedServer, stNodeValues.time, curTimeMs);
    }
}

