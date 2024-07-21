#pragma once
#include <string>
#include <map>
#include <vector>
#include "iec61850_server.h"

typedef struct _SIEC61850ServerParam
{
	int nPort;				//端口
	int nChangePeriodMs;	//数据变化周期，单位ms
    std::string strCfgPath;	//cfg路径
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
    int nChangePeriodMs;            //数据变化间隔
    msSinceEpoch lLastUpdateTime;   //最后一次更新时间，UTC时间
    NodeValuesSeq vecNodeValues;  //数据集内节点信息

    _SDatasetInfo();
}SDatasetInfo,*PSDatasetInfo;

typedef std::map<std::string, SDatasetInfo> DsName2InfoMAP; //<数据集名称 ,数据集信息>

float getValueByMmsValue(MmsValue * value);
