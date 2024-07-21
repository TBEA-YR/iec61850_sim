#include "ServerSimCommon.h"

using namespace std;

float getValueByMmsValue(MmsValue * value)
{
    switch (MmsValue_getType(value))
    {
    case MMS_BOOLEAN:
        return static_cast<float>(MmsValue_getBoolean(value));
    case MMS_INTEGER:
    case MMS_UNSIGNED:
        return static_cast<float>(MmsValue_toInt32(value));
    case MMS_FLOAT:
        return MmsValue_toFloat(value);
    default:
        break;
    }

    return 0.0f;
}

_SNodeValues::_SNodeValues()
{
    value = NULL;
    time = NULL;
    fMaxValue = 100;
    fMinValue = -100;
    fLastValue = fMinValue;
}

void _SNodeValues::setMinValue(const DataAttribute * value)
{
    if (value == NULL)
    {
        return;
    }

    if (MmsValue_getType(value->mmsValue) == MMS_BOOLEAN)
    {
        this->fMinValue = 0;
    }
    else
    {
        this->fMinValue = getValueByMmsValue(value->mmsValue);
    }
}

void _SNodeValues::setMaxValue(const DataAttribute * value)
{
    if (value == NULL)
    {
        return;
    }

    if (MmsValue_getType(value->mmsValue) == MMS_BOOLEAN)
    {
        this->fMaxValue = 1;
    }
    else
    {
        this->fMaxValue = getValueByMmsValue(value->mmsValue);
    }
}

void _SNodeValues::setLastValue(const float & fValue)
{
    this->fLastValue = fValue;
}

_SDatasetInfo::_SDatasetInfo()
    :nChangePeriodMs(5*60*1000),
    lLastUpdateTime(Hal_getTimeInMs()) //避免刚启动时集中变化导致CPU过高
{
}
