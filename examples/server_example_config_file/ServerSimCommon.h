#pragma once
#include <string>
#include <map>
#include <vector>
#include "iec61850_server.h"

typedef struct _SIEC61850ServerParam
{
	int nPort;				//�˿�
	int nChangePeriodMs;	//���ݱ仯���ڣ���λms
    std::string strCfgPath;	//cfg·��
}SIEC61850ServerParam,*PSIEC61850ServerParam;

typedef struct _SNodeValues
{
    std::string strName;
	DataAttribute *value;
	DataAttribute *time;
    float fMaxValue;
    float fMinValue;
    float fLastValue;

    _SNodeValues();
    void setMinValue(const DataAttribute *value);
    void setMaxValue(const DataAttribute *value);
    void setLastValue(const float &fValue);
}SNodeValues, *PSNodeValues;

typedef std::vector<SNodeValues> NodeValuesSeq;

typedef struct _SDatasetInfo
{
    int nChangePeriodMs;            //���ݱ仯���
    msSinceEpoch lLastUpdateTime;   //���һ�θ���ʱ�䣬UTCʱ��
    NodeValuesSeq vecNodeValues;  //���ݼ��ڽڵ���Ϣ

    _SDatasetInfo();
}SDatasetInfo,*PSDatasetInfo;

typedef std::map<std::string, SDatasetInfo> DsName2InfoMAP; //<���ݼ����� ,���ݼ���Ϣ>

float getValueByMmsValue(MmsValue * value);
