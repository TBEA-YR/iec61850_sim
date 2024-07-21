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
    int m_nThreadPeriodMs;    //�߳���������
    volatile bool m_bIsRunning; //�߳����б�ʶ���ȼ򻯴���
	SIEC61850ServerParam m_stParam;
	IedServer m_iedServer;
	IedModel* m_model;
    Thread   m_pThread;
};

typedef std::vector<CServerSimThreadBase*> ServerSimThreadBaseSeq;
