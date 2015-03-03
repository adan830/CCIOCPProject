/**************************************************************************************
@author: 陈昌
@content: 充值付费管理器
**************************************************************************************/
#include "stdafx.h"
#include "CRechargeManager.h"

const std::string Area_Recharge_Proc = "P_GS_Order_ChargeQueue";
const std::string Single_Recharge_Proc = "P_GS_Order_Charge";
const std::string Recharge_CallBack_Proc = "P_GS_Order_ChargeCallBack";

/************************Start Of CSQLWorkThread******************************************/
CSQLWorkThread::CSQLWorkThread(void* owner, const std::string &sConnectStr) : m_Owner(owner), m_sConnectStr(sConnectStr), m_bEnabled(false), m_pMySQLProc(nullptr)
{}

CSQLWorkThread::~CSQLWorkThread()
{
	WaitThreadExecuteOver();
}

void CSQLWorkThread::DoExecute()
{

}

void CSQLWorkThread::OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg)
{
	if (0 == uiErrorCode)
		Log(sErrMsg, lmtMessage);
	else
		Log(sErrMsg, lmtError);
}

void CSQLWorkThread::CheckProcExists(void* Sender)
{
	m_bEnabled = false;
	std::string sSql("SHOW PROCEDURE STATUS LIKE \"" + Area_Recharge_Proc + "\"");
	int iAffected;
	IMySQLFields* pDataSet;
	if ((m_pMySQLProc->Exec(sSql, pDataSet, iAffected)) && (1 == iAffected))
	{
		sSql = "SHOW PROCEDURE STATUS LIKE \"" + Recharge_CallBack_Proc + "\"";
		m_bEnabled = (m_pMySQLProc->Exec(sSql, pDataSet, iAffected)) && (1 == iAffected);
	}
	if (!m_bEnabled)
		Log("充值接口不存在", lmtWarning);
}

std::string CSQLWorkThread::BuildJsonResult(IMySQLFields* pDataSet)
{}

bool CSQLWorkThread::QueryAreaRecharge(PJsonJobNode pNode)
{}

bool CSQLWorkThread::DBRechargeAck(PJsonJobNode pNode)
{}
/************************End Of CSQLWorkThread********************************************/


/************************Start Of CRechargeManager******************************************/

/************************End Of CRechargeManager******************************************/

