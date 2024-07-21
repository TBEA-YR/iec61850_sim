#include "BMSServerSimThread.h"
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
    CBMSServerSimThread *pThread = (CBMSServerSimThread*)threadParameter; //����ǰthis�Ѿ����ڣ��ݲ��ж�ָ��Ϸ���
    pThread->run();
    return NULL;
}

CBMSServerSimThread::CBMSServerSimThread(const SIEC61850ServerParam &stParm)
	:CServerSimThreadBase(stParm)
{
}


CBMSServerSimThread::~CBMSServerSimThread()
{
    stop();
}

bool CBMSServerSimThread::init()
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

bool CBMSServerSimThread::start()
{
    return CServerSimThreadBase::start();
}

bool CBMSServerSimThread::stop()
{
    return CServerSimThreadBase::stop();
}

bool CBMSServerSimThread::parseDataset()
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

void CBMSServerSimThread::simulate()
{
    simAllData();
}

void CBMSServerSimThread::simAllData()
{
    for (auto iter = m_mapDSName2Info.begin();iter != m_mapDSName2Info.end();iter++)
    {
        SDatasetInfo &stDSInfo = iter->second;

        if (Hal_getTimeInMs() - stDSInfo.lLastUpdateTime < stDSInfo.nChangePeriodMs)
        {
            continue; //δ��ʱ��
        }

        IedServer_lockDataModel(m_iedServer);
        simLinear(stDSInfo,1);
        IedServer_unlockDataModel(m_iedServer);

        stDSInfo.lLastUpdateTime = Hal_getTimeInMs();
        break; //ÿ��ֻ����һ�����ݼ�����ֹCPU����
    }
}

void CBMSServerSimThread::simLinear(SDatasetInfo &stDSInfo, const float &fIncrement)
{
    msSinceEpoch curTime = Hal_getTimeInMs();

    for (size_t nIdex = 0;nIdex < stDSInfo.vecNodeValues.size();nIdex++)
    {
        SNodeValues &stNode = stDSInfo.vecNodeValues[nIdex];

        updateAttributeValue(stNode.value, stNode.fLastValue);
        //updateAttributeValueAndTime(stNode, stNode.fLastValue, curTime); //����CPU��������ʱ��

        stNode.fLastValue += fIncrement;
        if (stNode.fLastValue > stNode.fMaxValue) //�ȴֱ��ıȽϣ����Ͻ�
        {
            stNode.fLastValue = stNode.fMinValue;
        }
    }
}

bool CBMSServerSimThread::parseNodeByFCDas(DataSetEntry * fcdas, SNodeValues & stNodeValues)
{
    //fcdas->variableName
    string strValueNodeName = "";
    size_t nVarNameLen = strlen(fcdas->variableName);

    string strVarNameWithoutFC = StringUtil::m_replace(fcdas->variableName, "$SP$", ".");
    strVarNameWithoutFC = StringUtil::m_replace(strVarNameWithoutFC, "$ST$", ".");
    strVarNameWithoutFC = StringUtil::m_replace(strVarNameWithoutFC, "$MX$", ".");
    strVarNameWithoutFC = StringUtil::m_replace(strVarNameWithoutFC, "$", ".");
    
    std::vector<std::string> vecSuffix{ ".stVal",".mag.f",".mxVal.f",".setMag.f"};

    for (size_t nSuffixIdx = 0;nSuffixIdx < vecSuffix.size();nSuffixIdx++)
    {
        size_t nPos = strVarNameWithoutFC.rfind(vecSuffix[nSuffixIdx]);
        if (nPos != strVarNameWithoutFC.npos)
        {
            strVarNameWithoutFC = strVarNameWithoutFC.substr(0, nPos);
            strValueNodeName = vecSuffix[nSuffixIdx].substr(1);
            break;
        }
    }

    if (strValueNodeName.empty())
    {
        printf("��ȡ�ڵ�ֵ����ʧ��.%s\n", fcdas->variableName);
        return false;
    }
    
    string strVarName = m_model->name;
    strVarName.append(fcdas->logicalDeviceName).append("/").append(strVarNameWithoutFC);

    DataAttribute* objNode = (DataAttribute*)IedModel_getModelNodeByObjectReference(m_model, strVarName.c_str());
    if (objNode == NULL)
    {
        printf("��ȡ�ڵ�ʧ��.%s\n", strVarName.c_str());
        return false;
    }

    stNodeValues.strName = strVarName;
    stNodeValues.value = (DataAttribute*)ModelNode_getChild((ModelNode*)objNode, strValueNodeName.c_str());
    stNodeValues.time = (DataAttribute*)ModelNode_getChild((ModelNode*)objNode, "t");

    if (stNodeValues.value == NULL /* || stNodeValues.time == NULL*/)
    {
        printf("��ȡ�ڵ�����ʧ��.%s\n", strVarName.c_str());
        return false;
    }

    if (stNodeValues.value->fc == IEC61850_FC_ST)
    {
        stNodeValues.fMinValue = 0;
        stNodeValues.fMaxValue = 1;
    }
    else
    {
        //���ٻ�ȡ�����Сֵ����Ϊģ����δ���õ���2���ֶζ���0
//         DataAttribute *minValue = (DataAttribute*)ModelNode_getChild((ModelNode*)objNode, "minVal.f");
//         DataAttribute *maxValue = (DataAttribute*)ModelNode_getChild((ModelNode*)objNode, "maxVal.f");
//         stNodeValues.setMinValue(minValue);
//         stNodeValues.setMaxValue(maxValue);
    }

    stNodeValues.setLastValue(stNodeValues.fMinValue);

    return true;
}

