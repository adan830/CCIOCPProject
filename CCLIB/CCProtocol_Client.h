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

//��Ϣͷ�����ڷ��Ϳͻ��˷��ͽṹ�������Ϣ��
typedef struct _TRecordMsgHead
{
	unsigned short usRecordSize;		//�ṹ��С
	unsigned short usRecordCount;		//����
}TRecordMsgHead, *PRecordMsgHead;

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
	acStepBack
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

// CM_SPELL
typedef struct _TPkgSpellReq
{
	unsigned short usSkillID;                               //����ID
	unsigned short usFireX;                                 //Ŀ���λ��
	unsigned short usFireY;                                 //Ŀ���λ��
	int iTargID;                                            //Ŀ��ID
	unsigned char ucDir;                                    //Ŀ�귽��
}TPkgSpellReq, *PPkgSpellReq;

// CM_USE_BAG_ITEM 
typedef struct _TPkgUseItem
{
	int iItemID;
	unsigned char ucCDType;
	unsigned short usCDTime;
}TPkgUseItem, *PPkgUseItem;
//buf:Ϊʹ����Ʒ��Ҫ���������ݣ���������Ϊ��������

// CM_TRAN_COMMIT       
typedef struct _TPkgCommitTran
{
	unsigned int uiTranID;									//����ID
	unsigned char ucResult;									//0:ȡ�� 1��ȷ��
	unsigned short usDataLen;								//���ݳ���
}TPkgCommitTran, *PPkgCommitTran;

// SCM_MAPINFO
typedef struct _TPkgMapInfo
{
	int iMapID;
	int iFileID;
	int iMapObjID;    //��������ID
	char szMapDesc[MAP_NAME_MAX_LEN];
}TPkgMapInfo, *PPkgMapInfo;

//���ܻ�����Ϣ
typedef struct _TClientMagicInfo
{
	char szMagicName[SKILL_NAME_MAX_LEN];		//��������
	unsigned short usMagicID;					//���ܱ��
	unsigned short usEffect;					//����������Ч
	unsigned short usNeedMP;					//���ܺ���
	unsigned char ucCDType;						//CD����
	unsigned char ucPriority;					//�������ȼ�
	unsigned char ucSkillLv;					//���ܵ�ǰ�ȼ�
	unsigned int uiCurExp;						//��ǰ������
	unsigned int uiNextNeedExp;					//��һ����Ҫ��������
	unsigned char ucHotKey;						//��ݼ�
	unsigned short usStatus;					//״̬
	unsigned char ucActType;					//ʩ����������TActionType
	unsigned char ucLockType;					//��������
	unsigned char ucMagicFlag;					//���ܱ�ʾ 0���������� 1���������� 2�����ؼ���
	unsigned short usFeature;					//����
	unsigned int uiCurPower;					//��������
	unsigned int uiNextPower;					//�¼�����
	unsigned int uiCDTime;						//CDʱ��
	unsigned short usNextNeedLv;				//��һ����Ҫ������ȼ�
	unsigned char ucShowPriority;				//������ʾ���ȼ��������б��е�����
}TClientMagicInfo, *PClientMagicInfo;
                                                                                                  
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

typedef struct _TPkgMsgHead
{
	int iObjID;
	unsigned char ucColor;	    //ǰ��ɫ
	unsigned char ucBackColor;  //����ɫ��255�ޱ���
	unsigned char ucMsgType;    //��Ϣ����
	unsigned char ucMsgLen;     //��Ϣ����
	//��Ϣ����
}TPkgMsgHead, *PPkgMsgHead;


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

const int CM_STEP_BACKWARD = 105;                            // ����

const int CM_GUILD_OP = 112;                                 // �л������Э��

const int CM_TRAN_COMMIT = 171;                              // �ύһ������

const int CM_EMAIL = 176;                                    // Email��Э��

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

const int SCM_PING = 10;                                    // �����ظ�
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
const int SCM_SELECT_SERVER = 5011;                          // ѡ��
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

//��ɫ����
const int C_WHITE = 0;                                    //��
const int C_GREEN = 1;                                    //��
const int C_RED = 2;									  //��
const int C_BLUE = 3;                                     //��
const int C_PURPLE = 4;                                   //��
const int C_GOLD = 5;                                     //��
const int C_YELLOW = 6;									  //��
const int C_GRAYER = 7;                                   //��ɫ
const int C_BLACK = 8;                                    //��
const int C_ORANGE = 9;                                   //��
const int C_SLAVE_1 = 10;                                 //1��������ɫ
const int C_SLAVE_2 = 11;                                 //2��������ɫ
const int C_SLAVE_3 = 12;                                 //3��������ɫ
const int C_SLAVE_4 = 13;                                 //4��������ɫ
const int C_SLAVE_5 = 14;                                 //5��������ɫ
const int C_SLAVE_6 = 15;                                 //6��������ɫ
const int C_MSG_WHITE = 16;                               // ���� ��
const int C_MSG_GREEN = 17;                               // ���� ��
const int C_MSG_RED = 18;                                 // ���� ��
const int C_MSG_BLUE = 19;                                // ���� ��
const int C_MSG_PURPLE = 20;                              // ���� �ۺ�
const int C_MSG_GOLD = 21;                                // ���� ��
const int C_MSG_SIMPLE_PURPLE = 22;                       // ���� ǳ�ۺ�
const int C_MASTERWORK_GOLD = 23;                         // С��Ʒ��ɫ $FF8C00
const int C_MASTERWORK_BLUE = 24;                         // С��Ʒ��ɫ $1E90FF
const int C_DRESS_SLIVER = 33;                            //����ɫ	$FFC0C0C0
const int C_DRESS_SKYBLUE = 34;                           //����ɫ  $FF87CEEB
const int C_DRESS_GOLD = 35;                              //��ɫ    $FFFFD700
const int C_DRESS_LIGHTGREEN = 36;                        //����ɫ  $FF90EE90
const int C_DRESS_MEDIUMPUERPLE = 37;                     //����ɫ  $FF9370DB
const int C_DRESS_ORANGERED = 38;                         //�Ⱥ�ɫ FFFF4500
const int C_DRESS_DARKRED = 39;                           //���ɫ FF8B0000
const int C_LOUDSPEAKER_TOP_BACK = 40;                    //�ö����ȱ���ɫ
const int C_LOUDSPEAKER_TOP_MSG = 41;                     //�ö�����ǰ��ɫ

//����CD����
const int CD_DENY = 0;										 //��ֹ
const int CD_NOT_DELAY = 1;                                  //��ͨ(����CD)
const int CD_ATTACK = 2;									 //����������CD ID
const int CD_MAGIC = 3;										 //ħ����������CD ID
const int CD_MOVE = 4;										 //λ�ƹ���CD ID
const int CD_SAY = 5;										 //����CD
const int CD_CLICK_NPC = 6;                                  //���NPC
const int CD_USEITEM = 7;                                    //ʹ����Ʒ
const int CD_RELATION_OP = 8;                                //��ϵ����
const int CD_RUSH = 9;										 //ǰ��CD
const int CD_EMAIL = 250;									 //�ʼ�

inline void GetMsgColor(TMesssageType msgType, unsigned char &ucColor, unsigned char ucBackColor, bool bIncludeLink = true)
{
	if (255 == ucColor)
	{
		ucColor = C_WHITE;
		switch (msgType)
		{
		case msSystem:
			ucColor = C_BLACK;
			break;
		case msGroup:
			ucColor = C_MSG_BLUE;
			break;
		case msGuild:
			ucColor = C_MSG_GREEN;
			break;
		case msWhisper:
			ucColor = C_MSG_PURPLE;
			break;
		case msRoll:
			ucColor = C_YELLOW;
			break;
		case msDialog:
		case msMonster:
		case msCry:
			ucColor = C_MSG_WHITE;
			break;
		case msLoudSpeaker:
			ucColor = C_WHITE;
			break;
		case msHint:
			ucColor = C_MSG_WHITE;
			break;
		case msLeftSide:
			ucColor = C_GREEN;
			break;
		case msCenter:
		case msGMSystem:
		case msDebug:
			ucColor = C_WHITE;
			break;
		default:
			break;
		}
	}

	if (bIncludeLink && ((msWhisper == msgType) || (msLoudSpeaker == msgType) || (msCry == msgType)))
		ucColor = ucColor | 0x80;

	if (255 == ucBackColor)
	{
		switch (msgType)
		{
		case msLoudSpeaker: 
			ucBackColor = C_MSG_PURPLE;
			break;
		case msGMSystem:
			ucBackColor = C_RED;
			break;
		case msSystem:
			ucBackColor = C_YELLOW;
		default:
			break;
		}
	}
}



#endif //__CC_PROTOCOL_CLIENT_H__