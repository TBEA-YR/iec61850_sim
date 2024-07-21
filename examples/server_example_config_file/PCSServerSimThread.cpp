#include "PCSServerSimThread.h"
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
    CPCSServerSimThread *pThread = (CPCSServerSimThread*)threadParameter; //传参前this已经存在，暂不判断指针合法性
    pThread->run();
    return NULL;
}

CPCSServerSimThread::CPCSServerSimThread(const SIEC61850ServerParam &stParm)
	:CServerSimThreadBase(stParm)
{
}


CPCSServerSimThread::~CPCSServerSimThread()
{
    stop();
}

bool CPCSServerSimThread::init()
{
    if (!parseDataset())
    {
        return false;
    }

    IedServer_start(m_iedServer, m_stParam.nPort);

    if (!IedServer_isRunning(m_iedServer))
    {
        printf("Starting server failed! Exit.\n");
        IedServer_destroy(m_iedServer);
        return false;
    }
    else
    {
        printf("Starting server. iedname=%s  port=%d \n", m_model->name, m_stParam.nPort);
    }

	return true;
}

bool CPCSServerSimThread::start()
{
    return CServerSimThreadBase::start();
}

bool CPCSServerSimThread::stop()
{
    return CServerSimThreadBase::stop();
}

bool CPCSServerSimThread::parseDataset()
{
    /* parse the configuration file and create the data model */
    m_model = ConfigFileParser_createModelFromConfigFileEx(m_stParam.strCfgPath.c_str());
    if (m_model == NULL) {
        printf("Error parsing config file!\n");
        return false;
    }

    m_iedServer = IedServer_create(m_model);
    if (m_iedServer == NULL)
    {
        printf("create iedServer error!\n");
        return false;
    }

    DataSet *ds = m_model->dataSets;
    while (ds != NULL)
    {
        if (strcmp(ds->logicalDeviceName, "PIGO") == 0)
        {
            //goose数据集，跳过
            ds = ds->sibling;
            continue;
        }

        SDatasetInfo stDSInfo;
        stDSInfo.nChangePeriodMs = m_stParam.nChangePeriodMs;

        DataSetEntry *fcdas = ds->fcdas;
        while (fcdas != NULL)
        {
            SNodeValues stNodeValues;
            if (parseNodeByFCDas(fcdas, stNodeValues))
            {
                stDSInfo.vecNodeValues.push_back(stNodeValues);
            }

            fcdas = fcdas->sibling;
        }

        string strDsName = ds->logicalDeviceName;
        strDsName.append("/").append(ds->name);
        m_mapDSName2Info[strDsName] = stDSInfo;

        ds = ds->sibling;
    }

    return true;
}

void CPCSServerSimThread::simulate()
{
    simAllData();
}

void CPCSServerSimThread::simAllData()
{
    for (auto iter = m_mapDSName2Info.begin();iter != m_mapDSName2Info.end();iter++)
    {
        SDatasetInfo &stDSInfo = iter->second;
        if (Hal_getTimeInMs() - stDSInfo.lLastUpdateTime < stDSInfo.nChangePeriodMs)
        {
            continue; //未到时间
        }

        IedServer_lockDataModel(m_iedServer);
        simLinear(stDSInfo,1);
        IedServer_unlockDataModel(m_iedServer);

        stDSInfo.lLastUpdateTime = Hal_getTimeInMs();
        break; //每次只处理一个数据集，防止CPU过高
    }

}

void CPCSServerSimThread::simLinear(SDatasetInfo &stDSInfo, const float &fIncrement)
{
    msSinceEpoch curTime = Hal_getTimeInMs();
    for (size_t nIdex = 0;nIdex < stDSInfo.vecNodeValues.size();nIdex++)
    {
        SNodeValues &stNode = stDSInfo.vecNodeValues[nIdex];

        updateAttributeValueAndTime(stNode, stNode.fLastValue, curTime);

        stNode.fLastValue += fIncrement;
        if (stNode.fLastValue > stNode.fMaxValue) //先粗暴的比较，不严谨
        {
            stNode.fLastValue = stNode.fMinValue;
        }

    }
}

bool CPCSServerSimThread::parseNodeByFCDas(DataSetEntry * fcdas, SNodeValues & stNodeValues)
{
    //fcdas->variableName
    string strValueNodeName = "";
    size_t nVarNameLen = strlen(fcdas->variableName);

    string strVarNameWithoutFC = StringUtil::m_replace(fcdas->variableName, "$SP$", ".");
    if (strVarNameWithoutFC.size() != nVarNameLen)
    {
        strValueNodeName = "setMag.f";
    }

    if (strValueNodeName.empty())
    {
        strVarNameWithoutFC = StringUtil::m_replace(fcdas->variableName, "$ST$", ".");
        if (nVarNameLen != strVarNameWithoutFC.size())
        {
            strValueNodeName = "stVal";
        }
    }

    if (strValueNodeName.empty())
    {
        strVarNameWithoutFC = StringUtil::m_replace(fcdas->variableName, "$MX$", ".");
        if (nVarNameLen != strVarNameWithoutFC.size())
        {
            strValueNodeName = "mag.f";
        }
    }

    if (strValueNodeName.empty())
    {
        printf("获取节点值属性失败.%s\n", fcdas->variableName);
        return false;
    }
    
    string strVarName = m_model->name;
    strVarName.append(fcdas->logicalDeviceName).append("/").append(strVarNameWithoutFC);

    DataAttribute* objNode = (DataAttribute*)IedModel_getModelNodeByObjectReference(m_model, strVarName.c_str());
    if (objNode == NULL)
    {
        printf("获取节点失败.%s\n", strVarName.c_str());
        return false;
    }

    stNodeValues.strName = strVarName;
    stNodeValues.value = (DataAttribute*)ModelNode_getChild((ModelNode*)objNode, strValueNodeName.c_str());
    stNodeValues.time = (DataAttribute*)ModelNode_getChild((ModelNode*)objNode, "t");

    if (stNodeValues.value == NULL  || stNodeValues.time == NULL)
    {
        printf("获取节点属性失败.%s\n", strVarName.c_str());
        return false;
    }

    if (stNodeValues.value->fc == IEC61850_FC_ST)
    {
        stNodeValues.fMinValue = 0;
        stNodeValues.fMaxValue = 1;
    }
    else
    {
        DataAttribute *minValue = (DataAttribute*)ModelNode_getChild((ModelNode*)objNode, "minVal.f");
        DataAttribute *maxValue = (DataAttribute*)ModelNode_getChild((ModelNode*)objNode, "maxVal.f");
        stNodeValues.setMinValue(minValue);
        stNodeValues.setMaxValue(maxValue);
    }

    stNodeValues.setLastValue(stNodeValues.fMinValue);

    return true;
}

