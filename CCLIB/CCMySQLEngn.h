/**************************************************************************************
@author: 陈昌
@content: mysql读写的封装引擎
**************************************************************************************/
#ifndef __CC_MYSQLENGN_H__
#define __CC_MYSQLENGN_H__

#include <string>
#include <functional>
#include <vector>
#include "my_global.h"
#include "mysql.h"

namespace CC_UTILS{

  const std::string MYSQL_DEFAULT_CHARSET = "gbk";
  const int MYSQL_DEFAULT_PORT = 3306;

  typedef std::function<void(unsigned int uiErrorCode, const std::string &sErrMsg)> TOnMySQLErrorEvent;
  typedef std::function<std::string(int iFieldIdx)> TGetValue;
  typedef std::function<void(void* Sender)> TNotifyEvent;

  //查询字段
  typedef struct _TMySQLField
  {
  public:
	  std::string AsString();
	  double AsDateTime();
	  int AsInteger();	  
	  int64 AsInt64();
  public:
	  enum_field_types Field_Type;
	  std::string Field_Name;
	  int Field_Length;
	  int Field_Idx;
	  TGetValue _OnGetValue;
  }TMySQLField, *PMySQLField;

  //结果集合接口
  class IMySQLFields
  {
  public:
	  virtual int FieldCount() = 0;
	  virtual int RecordCount() = 0;
	  virtual PMySQLField FieldByName(const std::string &sFieldName) = 0;
	  virtual PMySQLField FieldByIdx(int iFieldIdx) = 0;
	  virtual bool Eof() = 0;
	  virtual void First() = 0;
	  virtual void Next() = 0;
	  virtual void Seek(int iRow) = 0;
  };

  //单条查询记录 
  class CMySQLRecord : public IMySQLFields
  {
  public:
	  CMySQLRecord(MYSQL_RES* Res, TOnMySQLErrorEvent ErrorCallBack);
	  virtual ~CMySQLRecord();

	  int FieldCount();
	  int RecordCount();
	  PMySQLField FieldByName(const std::string &sFieldName);
	  PMySQLField FieldByIdx(int iFieldIdx);
	  bool Eof();
	  void First();
	  void Next();
	  void Seek(int iRow);
  private:
	  std::string GetValue(int iFieldIdx);
	  void WriteLog(const std::string sErrMsg);
  private:
	  TOnMySQLErrorEvent m_OnError;
	  bool m_bEof;
	  int m_iFieldsCount;
	  std::vector<TMySQLField> m_Fields;	
	  std::vector<std::string> m_Values;	
  };

  //多条查询记录
  class CMySQLRecords : public IMySQLFields
  {
  public:
	  CMySQLRecords(MYSQL_RES* Res, TOnMySQLErrorEvent ErrorCallBack);
	  virtual ~CMySQLRecords();

	  int FieldCount();
	  int RecordCount();
	  PMySQLField FieldByName(const std::string &sFieldName);
	  PMySQLField FieldByIdx(int iFieldIdx);
	  bool Eof();
	  void First();
	  void Next();
	  void Seek(int iRow);
  private:
	  std::string GetValue(int iFieldIdx);
	  void WriteLog(const std::string &sErrMsg);
  private:
	  TOnMySQLErrorEvent m_OnError;
	  bool m_bEof;
	  int m_iFieldsCount;
	  int m_iRecordCount;
	  MYSQL_RES* m_Res;
	  MYSQL_ROWS* m_CurrentRows;
	  std::vector<TMySQLField> m_Fields;
  };

  //Mysql操作的管理对象
  class CMySQLManager
  {
  public:
	  CMySQLManager();
	  virtual ~CMySQLManager();
	  bool Connect(const std::string &sHost, const std::string &sDBName, const std::string &sUser, const std::string &sPwd, std::string sCharSet=MYSQL_DEFAULT_CHARSET, int iPort = MYSQL_DEFAULT_PORT);
	  void Close();
	  bool Open();
	  bool SelectDB(const std::string &sDBName);
	  bool Exec(const std::string &sSQL, IMySQLFields* pDataSet, int &iAffected);
	  unsigned int EscapeString(char* pSource, unsigned int uiSize, char* pDest);
	  std::string EscapeString(char* pSource, unsigned int uiSize);
	  std::string GetLastError(unsigned int &uiErrorCode);
	  bool IsConnected();
	  void SetConnectString(const std::string &sValue);
  public:
	  int m_iPort;
	  std::string m_sHostName;
	  std::string m_sDBName;
	  std::string m_sUser;
	  std::string m_sPassword;
	  std::string m_sCharSet;	
	  bool m_bAutoReconnect;	
	  bool m_bAutoCreateDB;
	  TNotifyEvent m_OnConnect;
	  TNotifyEvent m_OnDisconnect;
	  TOnMySQLErrorEvent m_OnError;
  protected:
	  virtual bool CheckTables();
	  void DoSetConnectString(const std::string &sHost, const std::string &sDBName, const std::string &sUser, const std::string &sPwd, std::string &sCharSet, int iPort);
  private:
	  void DoError(const std::string &sCause);
	  bool CreateDataBase(const std::string &sDBName);
	  void CheckConnected();
  private:	  
	  MYSQL* m_LibHandle;
	  bool m_bFirstConnect;
	  bool m_bReady;	
	  bool m_bConnected;
	  std::string m_sErrorMsg;
	  std::string m_sConnectString;
	  unsigned int m_uiErrorCode;
  };

}

#endif //__CC_MYSQLENGN_H__