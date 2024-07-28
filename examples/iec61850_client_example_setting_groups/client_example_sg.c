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

        /* ����860ʵʩ�����淶��
        �Ƽ��Ķ�ֵ�޸�����Ϊ��
            1.�ͻ��˷����༭��ֵ����SelectEditSG��Ҳ����дEditSG���������Ӧ
            2.�ͻ��˶�ȡ��ǰ�༭���Ķ�ֵGetSGValues���������Ӧ
            3.�ͻ���д��ֵ����ǰ�༭��SetSGValues���������Ӧ
            4.�ͻ����ٴζ�ȡ��ǰ�༭�����ƣ�����У���Ƿ�д��ɹ�
            5.�ͻ���ȷ�϶�ֵ�޸�ConfirmEditSGValues,Ҳ����дCnfEditΪtrue���������Ӧ
        
        ֻ�Ƕ�ȡ��ֵ�������޸ĵ����̣�
            1.�ͻ��˷����༭��ֵ����SelectEditSG��Ҳ����дEditSG���������Ӧ
            2.�ͻ��˶�ȡ��ǰ�༭���Ķ�ֵGetSGValues���������Ӧ
            3.�ͻ��˷����༭��ֵ����SelectEditSG��Ҳ����дEditSGΪ0���������Ӧ

        ����ResvTms��˵����
            ���վIEC 61850��2���У���ȷ�˶�ͻ��˲�����ֵʱ�Ļ�����ơ���ֵ���ƿ�������ResvTms���ԣ���ʾ���ƿ��ƿ鱻����ռ�õ�ʣ��ʱ�䡣
            ����0��ʾ��ֵ���ƿ鴦�ڱ�ռ��״̬���ͻ��˷���SelectEditSG�ɹ��󣬷���˽�ResvTms��λģ����SettingControl�����õ����ռ��ʱ����еݼ���
            ����������ͷſͻ��˶�SGCBռ�õ�4�������
            1���ͻ��˷���ConfirmEditSGValues
            2���������ռ�ÿͻ��������ж�
            3���ͻ���дeditSGΪ0
            4��ResvTms�ݼ�Ϊ0����ռ�ó�ʱ

         �������������޸Ķ�ֵ������̣������Ҫ����ĳ�鶨ֵ���ͻ��˷���SelectActiveSG��Ҳ����дActSG���������Ӧ.
         ��ֵ��0��ʾ�ͷŶ�ֵ���ƣ����Զ�ֵ���Ǵ�1��ʼ��
         */

        /* Get variable specification of the SGCB (optional) */
        //��ѡ����������Ҳ����ֱ�Ӷ�ȡ��Ӧ����ֵ
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

        //1.�����Ƕ�ȡ��ֵ���̣��������޸�
        //1.1.��һ��ѡ��༭��ֵ��
        int nEditSG = 2;
        IedConnection_writeUnsigned32Value(con, &error, "DEMOPROT/LLN0.SGCB.EditSG", IEC61850_FC_SP, nEditSG);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeUnsigned32Value failed.code=%d\n", error);
            goto close_connection;
        }

        //1.2.�ڶ�����ȡ��ǰ��ֵ

        //����61850���ж�ȡ��������Ľӿ�û�б�¶������MmsConnection_readMultipleVariables����ֻ����һ��һ����ȡ��
        float fStrValue = IedConnection_readFloatValue(con, &error, "DEMOPROT/PTOC1.StrVal.setMag.f", IEC61850_FC_SE);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_readFloatValue failed.code=%d\n", error);
            goto close_connection;
        }
        printf("StrVal$setMag$f: %f\n", fStrValue);

        //1.3.�������ͷſ���Ȩ
        IedConnection_writeUnsigned32Value(con, &error, "DEMOPROT/LLN0.SGCB.EditSG", IEC61850_FC_SP, 0); //�ͷŶ�ֵ����
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeUnsigned32Value failed.code=%d\n", error);
            goto close_connection;
        }


        //2.�������޸Ķ�ֵ����
        //2.1 ��һ��ѡ��༭��ֵ��
        nEditSG = 3;
        IedConnection_writeUnsigned32Value(con, &error, "DEMOPROT/LLN0.SGCB.EditSG", IEC61850_FC_SP, nEditSG);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeUnsigned32Value failed.code=%d\n", error);
            goto close_connection;
        }

        //2.2.�ڶ�����ȡ��ǰ��ֵ
        fStrValue = IedConnection_readFloatValue(con, &error, "DEMOPROT/PTOC1.StrVal.setMag.f", IEC61850_FC_SE);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_readFloatValue failed.code=%d\n", error);
            goto close_connection;
        }
        printf("StrVal$setMag$f: %f\n", fStrValue);

        //2.3 �޸Ķ�ֵ
        IedConnection_writeFloatValue(con, &error, "DEMOPROT/PTOC1.StrVal.setMag.f", IEC61850_FC_SE, 99.9f);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeFloatValue failed.code=%d\n", error);
            goto close_connection;
        }

        //2.4 �ٴζ�ȡ���бȶԣ��Ƿ��޸ĳɹ�
        fStrValue = IedConnection_readFloatValue(con, &error, "DEMOPROT/PTOC1.StrVal.setMag.f", IEC61850_FC_SE);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_readFloatValue failed.code=%d\n", error);
            goto close_connection;
        }
        printf("read StrVal$setMag$f: %f\n", fStrValue);

        //2.5 ȷ��д��
        IedConnection_writeBooleanValue(con, &error, "DEMOPROT/LLN0.SGCB.CnfEdit", IEC61850_FC_SP, true);
        if (error != IED_ERROR_OK)
        {
            printf("IedConnection_writeBooleanValue failed.code=%d\n", error);
            goto close_connection;
        }


        //3.����ĳ����ֵ��
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
