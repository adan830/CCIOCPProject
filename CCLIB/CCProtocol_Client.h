/**************************************************************************************
@author: �²�
@content: ��Ϸ�ͻ������������ͨ�ŵĳ����ͽṹ����
**************************************************************************************/

#ifndef __CC_PROTOCOL_CLIENT_H__
#define __CC_PROTOCOL_CLIENT_H__

//����ʹ��1�ֽڶ���
#pragma pack(1)

// Client����������ͨѶͷ
typedef struct _TClientSocketHead
{
	unsigned int uiSign;				// �ָ��� CS_SEGMENTATION_SIGN
	unsigned short usPackageLen;		// ����ܳ���
	unsigned short usIdent;				// Ident or $8000 = ѹ��   [Ident>=$8000 ����ͷ��2�ֽڱ�ʾѹ��ǰ���ݳ��ȣ���������Ϊѹ����������Ident<$8000 ����ͷ��Ϊ������]
	unsigned int uiIdx;		    	    // �����,�������ݿ��ܻᱻѹ��
}TClientSocketHead, *PClientSocketHead;

// ��������
enum TActionType{
	acNone = 0,
	acStand,
	acWalk,
	acRun,
	acJump,
	acRush,
	acBackStep,
	acAttack1,
	acAttack2,
	acMagic1,
	acMagic2,
	acShoot1,
	acShoot2,
	acAttack0,
	acAttacked,
	acDie,
	acStone,
	acBorn,
	acLRush,
	acRRush,
	acIconSmall,
	acIconBig,
	acstepBack
};

//�ͻ��˵Ĵ�������
enum TClientWindowType{
	cwInValid = 0,                                          // ��Ч����
	cwAuthenWindow = 1,                                     // ��¼��֤����
	cwWarRule,                                              // ͬ���սЭ�鴰��
	cwQueue,                                                // �ŶӴ���
	cwCreateRole,                                           // ������ɫ����
	cwMessageBox,                                           // ��������
	cwDialog,
	cwNotice,                                               // ������Լ
	cwDealRequest,                                          // ����������ʾ��
	cwDealDialog,                                           // ���׿�
	cwVerifyCode,                                           // ��֤�봰��
	cwReLive,                                               // �����
	cwTransDialog,                                          // ���񴰿ڣ������͵Ĵ���ȷ�Ϸ���ʱ���뽫TransIDԭ������
	/*
		cwTransDialog���ʹ��ھ���˵����
		SCM_OPEN_WINDOW��Param 0:ȷ�Ͽ� 1������� 2 : 2�������
						 TransID������ID��TClientWindowRec�ṹ��Ϊ��ʾ��Ϣ
		CM_CLOSE_WINDOW��Param 0 : ȡ��   1��ȷ�� TransID������ID�����������TClientWindowRec�ṹ��Ϊ��������
	*/

	cwScriptDialog,                                         // �ű��Ի���
	cwNpcItemOper,                                          // Npc�Ե�����Ʒ�Ĳ�������
	/*
		cwNpcItemOper���ʹ��ھ���˵����
		Param : 1 ��Ʒ������
	*/

	cwNpcShop,                                              // Npc�̵괰��
	cwIMTransDialog,                                        // IMServer�����Ի���
	cwStorage,                                              // �ֿⴰ��
	cwMonCry,                                               // ����˵��
	/*
		cwMonCry���ʹ��ھ���˵����
		Param: ͷ������
		TransID : ��Ч����
		buf : ˵������
	*/

	cwCopyHint,                                             // ������ʾ
	/*
		cwCopyHint���ʹ��ھ���˵����
		Param: ����ID
	    TransID : 0 : ���� 1 : ͨ�� 2��ʧ��
	    buf : ��ʾ��Ϣ
	*/

	cwColdTime,                                             // ����ʱ
	/*
		cwColdTime���ʹ��ھ���˵����
		Param:����ʱʱ��
		TransID : ����ʱ����ID
		buf : ����ʱ��ʾ
	*/

	cwWeaponUpgrade,                                        // ��������
	cwGuildSet,                                             // �л����ý���
	cwYBDeal,                                               // Ԫ�����׿�
	cwGridData,                                             // �������ݴ���
	/*
		cwGridData���ʹ��ھ���˵����
		Param:TGridType
		buf : ���ڶ�������
	*/

	cwSafeCard,                                             //�ܱ�����֤
	/*
	    cwSafeCard���ʹ��ھ���˵����
		SCM_OPEN_WINDOW
		param : Ϊ3������ֵ
		CM_CLOSE_WINDOW : 
		BUF : Ϊ3�������Ӧ����ֵ�ַ���
	*/

	cwPayPwd,                                               // ֧������
	cwWeaponMelting,                                        // ��������
	cwRoleReName,                                           // ��ɫ������
	cwHopePool,                                             // ��Ը��
	cwEmail,                                                // �ʼ���
	cwPlayerShop,                                           // �������
	cwBag,                                                  // ����
	cwYearCeremony                                          // ���ʢ��
};

// SCM_AUTHEN_RESULT
typedef struct _TAuthenResponse
{
	int iResult;   //1 �ɹ� ���� ʧ��                                      
	//------------------------
	//------------------------
	//--- int64��ô����ȽϺ�
	_int64 dbLastLoginTime;
	char szUniqueID[ACCOUNT_MAX_LEN];
	char szLastLoginIP[IP_ADDRESS_MAX_LEN+1];
	char szMsg[51];
	char szLoginIP[IP_ADDRESS_MAX_LEN + 1];
}TAuthenResponse, *PAuthenResponse;

// CM_SELECT_SERVER
typedef struct _TCMSelectServer
{
	int iMaskServerID;				 // ѡ��Ķ���ķ�������
	int iCheckIP;                    // �������� �̶������� ��ַ���������ֲ�ͬ����������
}TCMSelectServer, *PCMSelectServer;

// CM_CLOSE_WINDOW��SCM_OPEN_WINDOW��SCM_CLOSE_WINDOW
typedef struct _TClientWindowRec
{
	unsigned char WinType;
	int Param;
	unsigned int TransID;          // ����ID
	//buf:��ʾ��Ϣ�ַ���
}TClientWindowRec, *PClientWindowRec;

// SCM_SELECT_AREA
typedef struct _TNextGateInfo
{
	int iSessionID;
	int iGateAddr;
	int iGatePort;
}TNextGateInfo, *PNextGateInfo;

// SCM_ACTFAIL
typedef struct _TPkgActFail
{
	int iObjID;
	unsigned short usX;
	unsigned short usY;
	unsigned char ucDir;
	unsigned short usAct;
}TPkgActFail, *PPkgActFail;

// SCM_ACTGOOD
typedef struct _TPkgActGood
{
	int iObjID;						//ִ�ж���ID
	unsigned short usAct;			//ִ�ж���
	unsigned short usParam;			//ִ�в��������Ǽ�����Ϊ����ID
}TPkgActGood, *PPkgActGood;

// CM_WALK, CM_RUN, CM_TURN��SCM_WALK, SCM_RUN�� SCM_TURN��SCM_RUSH��SCM_BACKSTEP
typedef struct _TPkgAction
{
	int iObjID;
	unsigned short usX;
	unsigned short usY;
	unsigned char ucDir;
}TPkgAction, *PPkgAction;

// SCM_MAPINFO
typedef struct _TPkgMapInfo
{
	int iMapID;
	int iFileID;
	int iMapObjID;    //��������ID
	char szMapDesc[MAP_NAME_MAX_LEN];
}TPkgMapInfo, *PPkgMapInfo;

// SCM_CDTIME_UPDATE
typedef struct _TPkgCDTimeChg
{
	unsigned char ucCDType;
	unsigned int uiCDTime;
}TPkgCDTimeChg, *PPkgCDTimeChg;

//	SCM_SYSTEM_MESSAGE
enum TMesssageType{
	//ϵͳ���桢�����Ϣ���л���Ϣ��˽����Ϣ��������Ϣ����ʾ����Ϣ��
	msSystem = 0, msGroup, msGuild, msWhisper, msRoll, msDialog,
	//����ð�ݡ���������(����)�����塢���Ⱥ�������ʾ��Ϣ����������Ϣ��������Ϣ��ϵͳ���ֹ���(������)��������Ϣ
	msMonster, msCry, msClan, msLoudSpeaker, msHint, msLeftSide, msCenter, msGMSystem, msDebug
};
//bMsgType=msLeftSideʱ nObjID��ʾ���ͣ��������ͼ�ս����Ϣ���ඨ��
//bMsgType=msLoudSpeakerʱ nObjID��ʾ���ͣ�0:��ͨ���� 1��������ʾ����


//�л����ͨ�ýṹ����Ӧ�ֶ�������OpIDΪ��Э��Źҹ�
typedef struct _TPkgGuildOpRec
{
	char strOpStr1[GUILD_OP_MAX_LEN];
	unsigned char ucOpID;
	int iParam;
}TPkgGuildOpRec, *PPkgGuildOpRec;


#pragma pack()



const int CS_SEGMENTATION_CLIENTSIGN = 0XAABBCCDD;          // ��Ϸ�ͻ��˺ͷ�������ͨѶЭ����ʼ��־

//��Ϸ�ͻ��˷��͸�����������Ϣ
const int CM_PING = 10;										 // ����
const int CM_SELECT_SERVER_OLD = 11;						 // �ɰ�ѡ��Э��
const int CM_GAME_CONNECT = 12;								 // ���ӵ�GameGate
const int CM_CLOSE_WINDOW = 14;                              // �رմ���
const int CM_GAME_QUEUE = 15;                                // ��ѯ����
const int CM_REGIST_REQUEST = 17;                            // �½��ʺ�
const int CM_VERIFY_CODE = 18;                               // �ύ��֤��
const int CM_NEXT_VERIFY_CODE = 19;                          // ��һ����֤��
const int CM_LOGON_OK = 21;                                  // ������Ϸok
const int CM_RUSH = 99;										 // ǰ��
const int CM_QUIT = 100;									 // �����˳�

//��Ϸ�ͻ��˲�����Ҫ��Ϣ�����±��
const int CM_SELECT_SERVER = 3011;                           // ѡ��Э��
const int CM_AUTHEN_REQUEST = 3013;                          // ��֤����
const int CM_CREATE_ROLE = 3016;                             // ������ɫ
const int CM_ENTER_GAMESCENE = 3020;                         // ������Ϸ
const int CM_GPS_CHECK_RESPONSE = 4100;                      // ����ң����ݰ�����
const int CM_WALK = 4101;                                    //��
const int CM_RUN = 4102;                                     //��
const int CM_PHYHIT = 4103;                                  //������
const int CM_SPELL = 4104;                                   //ħ������
const int CM_SAY = 4107;                                     //�ͻ��˷���������Ϣ
const int CM_PICKUP = 4108;                                  //ʰȡ��Ʒ
const int CM_CLICK_NPC = 4115;                               //���NPC
const int CM_CLICK_NPC_OPTION = 4116;                        //���NPC�Ի����е�ѡ��
const int CM_USE_BAG_ITEM = 4117;                            //�ͻ���˫��ʹ�ñ�����Ʒ
const int CM_QUESTION_DETAIL_REQ = 4118;                     //�ͻ������������Ϣ
const int CM_QUESTION_ANSWER = 4119;                         //�ش�����
const int CM_PWDPROTECT_CHG = 4120;                          //�������뱣��״̬�л�

//�������˷�����Ϸ�ͻ��˵���Ϣ

const int SCM_AUTHEN_RESULT = 13;                           // ��֤
const int SCM_OPEN_WINDOW = 14;                             // ��������
const int SCM_CLOSE_WINDOW = 15;                            // ���������رմ���
const int SCM_RESSERVER_INFO = 18;                          // ��Դ��������IP
const int SCM_ROLE_INFO = 19;                               // ��ɫ��Ϣ
const int SCM_QUIT = 20;                                    // �˳�


const int SCM_MAPINFO = 100;                                // ��ͼ��Ϣ
const int SCM_SKILL_LIST = 102;                             // �����б�
const int SCM_SKILL_ADD = 103;                              // ���¼�����Ϣ�����ǲ����������


const int SCM_ACTGOOD = 209;                                // ִ�гɹ�
const int SCM_ACTFAIL = 210;                                // ִ��ʧ��
const int SCM_DIE = 214;                                    // ����  
const int SCM_BACKSTEP = 216;                               // ����
const int SCM_JUMP = 220;                                   //��
const int SCM_L_RUSH = 221;                                 //ǰ��(���)
const int SCM_R_RUSH = 222;                                 //ǰ��(�ҽ�)


const int SCM_CDTIME_UPDATE = 312;                          // CD����

const int SCM_RELIVE = 318;                                 // ����



const int SCM_ENCODE = 5008;                                 // ����
const int SCM_DECODE = 5009;                                 // ����
const int SCM_SELECT_AREA = 5011;                            // ѡ��
const int SCM_SYSTEM_MESSAGE = 5017;                         // ��Ϣ
const int SCM_CAN_ENTER = 5022;                              // ���Խ�����Ϸ
const int SCM_GPS_CHECK_REQUEST = 6100;                      // ����Ҽ��
const int SCM_LOGON = 6101;                                  // ���ǽ�����Ϸ
const int SCM_APPEAR = 6202;                                 // NPC����
const int SCM_HUMAN_APPEAR = 6203;                           // ��ɫ����
const int SCM_ITEM_APPEAR = 6204;                            // ��ͼ�������
const int SCM_HP_CHANGE = 6300;                              // ����Ŀ��Ľ���ֵ�ı�


//������������

const int ENCODE_START_LEN = 6;



#endif //__CC_PROTOCOL_CLIENT_H__