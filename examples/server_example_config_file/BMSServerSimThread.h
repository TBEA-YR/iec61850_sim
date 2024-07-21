#pragma once
#include "ServerSimThreadBase.h"


class CBMSServerSimThread : public CServerSimThreadBase
{
public:
	CBMSServerSimThread(const SIEC61850ServerParam &stParm);
	virtual ~CBMSServerSimThread();

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


