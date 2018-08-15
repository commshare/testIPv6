/*
 * author:Alex shem
 *   date:2016-8-24
 *   file:SendAutoInviteTask.cpp
 *   desc:启动/关闭发送自动邀请消息任务（仅女士）
*/

#include "SendAutoInviteTask.h"
#include "ITaskManager.h"
#include "ILiveChatClient.h"
#include "AmfPublicParse.h"
#include <json/json/json.h>
#include <common/CheckMemoryLeak.h>


SendAutoInviteTask::SendAutoInviteTask(void)
{
	m_listener = NULL;

	m_seq = 0;
	m_errType = LCC_ERR_FAIL;
	m_errMsg  = "";

	m_isOpen = false;
}

SendAutoInviteTask::~SendAutoInviteTask(void)
{
}

//初始化
bool SendAutoInviteTask::Init(ILiveChatClientListener* listener)
{
	bool result = false;
	if(NULL != listener)
	{
		m_listener = listener;
		result = true;
	}
	return result;
}


//处理接收数据
bool SendAutoInviteTask::Handle(const TransportProtocol* tp)
{
	bool result = false;
	
	AmfParser parser;
	amf_object_handle root = parser.Decode((char*)tp->data, tp->GetDataLength());
	if(!root.isnull()){
		if(root->type == DT_FALSE || root->type == DT_TRUE)
		{
			m_errType = root->boolValue ? LCC_ERR_SUCCESS : LCC_ERR_FAIL;
			m_errMsg  = "";
			//m_isOpen = root->boolValue;
			result = true;
		}
	}
	// 协议解析失败
	if(!result){
		m_errType = LCC_ERR_PROTOCOLFAIL;
		m_errMsg = "";
	}
	// ֪ͨlistener
	if(NULL != m_listener){
		m_listener->OnSwitchAutoInviteMsg(m_errType, m_errMsg, m_isOpen);
	}

    return result;
}

// 获取待发送的数据，可先获取data长度，GetSendData(NULL, 0, dataLen);
bool SendAutoInviteTask::GetSendData(void* data, unsigned int dataSize, unsigned int& dataLen)
{
	bool result = false;
	
	// 构造json协议
	Json::Value root;
	root = m_isOpen;
	Json::FastWriter writer;
	string json = writer.write(root);
	
	// 填入buffer
	if (json.length() < dataSize) {
		memcpy(data, json.c_str(), json.length());
		dataLen = json.length();

		result  = true;
	}
	return result;
}

// 获取待发送数据的类型
TASK_PROTOCOL_TYPE SendAutoInviteTask::GetSendDataProtocolType()
{
	return JSON_PROTOCOL;
}

// 获取命令号
int SendAutoInviteTask::GetCmdCode()
{
	return TCMD_SENDAUTOINVITE;
}

// 设置seq
void SendAutoInviteTask::SetSeq(unsigned int seq)
{
	m_seq = seq;
}

// 获取seq
unsigned int SendAutoInviteTask::GetSeq()
{
	return m_seq;
}

// 是否需要等待回复，若false则发送后释放（delete掉），否则发送后会被添加至待回复列表，收到回复后释放
bool SendAutoInviteTask::IsWaitToRespond()
{
	return true;
}

// 获取处理结果
void SendAutoInviteTask::GetHandleResult(LCC_ERR_TYPE& errType, string& errMsg)
{
	errType = m_errType;
	errMsg = m_errMsg;
}

// 初始化参数
bool SendAutoInviteTask::InitParam(bool isOpen)
{
	m_isOpen = isOpen;
	return true;
}

// 未完成任务的断线通知
void SendAutoInviteTask::OnDisconnect()
{
	if (NULL != m_listener) {
		m_listener->OnSwitchAutoInviteMsg(LCC_ERR_CONNECTFAIL, "", false);
	}
}



