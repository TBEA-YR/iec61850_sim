/*
 * client_example1.c
 *
 * This example is intended to be used with server_example_basic_io or server_example_goose.
 */

#include "iec61850_client.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {

    char* hostname;
    int tcpPort = 102;
    const char* localIp = NULL;
    int localTcpPort = -1;
    

    if (argc > 1)
        hostname = argv[1];
    else
        hostname = "localhost";

    if (argc > 2)
        tcpPort = atoi(argv[2]);

    if (argc > 3)
        localIp = argv[3];

    if (argc > 4)
        localTcpPort = atoi(argv[4]);

    IedClientError error;

    IedConnection con = IedConnection_create();

    /* Optional bind to local IP address/interface */
    if (localIp) {
        IedConnection_setLocalAddress(con, localIp, localTcpPort);
        printf("Bound to Local Address: %s:%i\n", localIp, localTcpPort);
    }

    IedConnection_connect(con, &error, hostname, tcpPort);
    printf("Connecting to %s:%i\n", hostname, tcpPort);

    if (error == IED_ERROR_OK)
    {
        printf("Connected\n");

        /* 根据860实施技术规范，
        推荐的定值修改流程为：
            1.客户端发出编辑定值请求SelectEditSG，也就是写EditSG，服务端响应
            2.客户端读取当前编辑区的定值GetSGValues，服务端响应
            3.客户端写定值到当前编辑区SetSGValues，服务端响应
            4.客户端再次读取当前编辑区定制，用于校验是否写入成功
            5.客户端确认定值修改ConfirmEditSGValues,也就是写CnfEdit为true，服务端响应
        
        只是读取定值不进行修改的流程：
            1.客户端发出编辑定值请求SelectEditSG，也就是写EditSG，服务端响应
            2.客户端读取当前编辑区的定值GetSGValues，服务端响应
            3.客户端发出编辑定值请求SelectEditSG，也就是写EditSG为0，服务端响应

        关于ResvTms的说明：
            变电站IEC 61850第2版中，明确了多客户端操作定值时的互斥机制。定值控制块新增了ResvTms属性，表示定制控制块被持续占用的剩余时间。
            大于0表示定值控制块处于被占用状态，客户端发出SelectEditSG成功后，服务端将ResvTms置位模型中SettingControl中配置的最大占用时间进行递减。
            服务端主动释放客户端对SGCB占用的4种情况：
            1、客户端发出ConfirmEditSGValues
            2、服务端与占用客户端连接中断
            3、客户端写editSG为0
            4、ResvTms递减为0，即占用超时

         上面描述的是修改定值组的流程，如果需要启用某组定值，客户端发出SelectActiveSG，也就是写ActSG，服务端响应.
         定值号0表示释放定值控制，所以定值号是从1开始的
         */

        /* Get variable specification of the SGCB (optional) */
        //可选操作，下面也可以直接读取对应变量值
        MmsVariableSpecification* sgcbVarSpec = IedConnection_getVariableSpecification(con, &error, "DEMOPROT/LLN0.SGCB", FunctionalConstraint::IEC61850_FC_SP);
        if (error != IED_ERROR_OK || sgcbVarSpec == NULL)
        {
            printf("getVariableSpecification failed.code=%d\n", error);
            goto close_connection;
        }

        MmsValue* sgcbMmsValue = IedConnection_readObject(con, &error, "DEMOPROT/LLN0.SGCB", IEC61850_FC_SP);
        if (sgcbMmsValue == NULL)
        {
            printf("IedConnection_readObject failed.code=%d\n", error);
            goto close_connection;
        }

        MmsValue* numMmsValue = MmsVariableSpecification_getChildValue(sgcbVarSpec, sgcbMmsValue, "NumOfSG");
        MmsValue* actMmsValue = MmsVariableSpecification_getChildValue(sgcbVarSpec, sgcbMmsValue, "ActSG");
        MmsValue* editMmsValue = MmsVariableSpecification_getChildValue(sgcbVarSpec, sgcbMmsValue, "EditSG");
        MmsValue* cnfMmsValue = MmsVariableSpecification_getChildValue(sgcbVarSpec, sgcbMmsValue, "CnfEdit");

        if (numMmsValue == NULL || actMmsValue == NULL || editMmsValue == NULL || cnfMmsValue == NULL)
        {
            printf("MmsVariableSpecification_getChildValue failed.code=%d\n", error);
            goto close_connection;
        }
        printf("NumOfSG: %d\n", MmsValue_toInt32(numMmsValue));
        printf("ActSG: %d\n", MmsValue_toInt32(actMmsValue));
        printf("EditSG: %d\n", MmsValue_toInt32(editMmsValue));
        printf("CnfEdit: %d\n", MmsValue_getBoolean(cnfMmsValue));

        //1.下面是读取定值过程，不进行修改
        //1.1.第一步选择编辑定值区
        int nEditSG = 2;
        IedConnection_writeUnsigned32Value(con, &error, "DEMOPROT/LLN0.SGCB.EditSG", IEC61850_FC_SP, nEditSG);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeUnsigned32Value failed.code=%d\n", error);
            goto close_connection;
        }

        //1.2.第二步读取当前定值

        //由于61850库中读取多个变量的接口没有暴露出来（MmsConnection_readMultipleVariables），只能先一个一个读取了
        float fStrValue = IedConnection_readFloatValue(con, &error, "DEMOPROT/PTOC1.StrVal.setMag.f", IEC61850_FC_SE);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_readFloatValue failed.code=%d\n", error);
            goto close_connection;
        }
        printf("StrVal$setMag$f: %f\n", fStrValue);

        //1.3.第三步释放控制权
        IedConnection_writeUnsigned32Value(con, &error, "DEMOPROT/LLN0.SGCB.EditSG", IEC61850_FC_SP, 0); //释放定值控制
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeUnsigned32Value failed.code=%d\n", error);
            goto close_connection;
        }


        //2.下面是修改定值过程
        //2.1 第一步选择编辑定值区
        nEditSG = 3;
        IedConnection_writeUnsigned32Value(con, &error, "DEMOPROT/LLN0.SGCB.EditSG", IEC61850_FC_SP, nEditSG);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeUnsigned32Value failed.code=%d\n", error);
            goto close_connection;
        }

        //2.2.第二步读取当前定值
        fStrValue = IedConnection_readFloatValue(con, &error, "DEMOPROT/PTOC1.StrVal.setMag.f", IEC61850_FC_SE);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_readFloatValue failed.code=%d\n", error);
            goto close_connection;
        }
        printf("StrVal$setMag$f: %f\n", fStrValue);

        //2.3 修改定值
        IedConnection_writeFloatValue(con, &error, "DEMOPROT/PTOC1.StrVal.setMag.f", IEC61850_FC_SE, 99.9f);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeFloatValue failed.code=%d\n", error);
            goto close_connection;
        }

        //2.4 再次读取进行比对，是否修改成功
        fStrValue = IedConnection_readFloatValue(con, &error, "DEMOPROT/PTOC1.StrVal.setMag.f", IEC61850_FC_SE);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_readFloatValue failed.code=%d\n", error);
            goto close_connection;
        }
        printf("read StrVal$setMag$f: %f\n", fStrValue);

        //2.5 确认写入
        IedConnection_writeBooleanValue(con, &error, "DEMOPROT/LLN0.SGCB.CnfEdit", IEC61850_FC_SP, true);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeBooleanValue failed.code=%d\n", error);
            goto close_connection;
        }


        //3.激活某个定值组
        nEditSG = 4;
        IedConnection_writeUnsigned32Value(con, &error, "DEMOPROT/LLN0.SGCB.ActSG", IEC61850_FC_SP, nEditSG);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeUnsigned32Value failed.code=%d\n", error);
            goto close_connection;
        }

close_connection:

        IedConnection_close(con);
    }
    else {
        printf("Failed to connect to %s:%i\n", hostname, tcpPort);
    }

    IedConnection_destroy(con);

    return 0;
}
