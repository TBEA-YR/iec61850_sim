#pragma once
#include "ServerSimCommon.h"
#include "hal_thread.h"

class CServerSimThreadBase
{
public:
	CServerSimThreadBase(const SIEC61850ServerParam &stParm);
	virtual ~CServerSimThreadBase();

	virtual bool init() = 0;
	virtual bool start();

	virtual bool stop();

    virtual void run(); 
    virtual void simulate() = 0;
protected:
    void updateAttributeValue(DataAttribute *attrValue,const float &fValue);

    void updateAttributeValueAndTime(SNodeValues &stNodeValues, const float &fValue, const msSinceEpoch &curTimeMs);

protected:
    int m_nThreadPeriodMs;    //线程休眠周期
    volatile bool m_bIsRunning; //线程运行标识，先简化处理
	SIEC61850ServerParam m_stParam;
	IedServer m_iedServer;
	IedModel* m_model;
    Thread   m_pThread;
};

typedef std::vector<CServerSimThreadBase*> ServerSimThreadBaseSeq;
