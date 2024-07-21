#pragma once
#include "ServerSimThreadBase.h"


class CPCSServerSimThread : public CServerSimThreadBase
{
public:
	CPCSServerSimThread(const SIEC61850ServerParam &stParm);
	virtual ~CPCSServerSimThread();

	virtual bool init();
	virtual bool start();
	virtual bool stop();
    virtual void simulate();

private:
    bool parseDataset();
    void simAllData();

    void simLinear(SDatasetInfo &stDSInfo,const float &fIncrement);

    bool parseNodeByFCDas(DataSetEntry *fcdas, SNodeValues &stNodeValues);

private:
    DsName2InfoMAP m_mapDSName2Info;
};


