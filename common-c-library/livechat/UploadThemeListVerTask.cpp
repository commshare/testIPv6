/*
 * author: Samson.Fan
 *   date: 2016-04-25
 *   file: UploadThemeListVerTask.cpp
 *   desc: 上传主题包列表版本号Task实现类
 */

#include "UploadThemeListVerTask.h"
#include <json/json/json.h>
#include <common/CheckMemoryLeak.h>

UploadThemeListVerTask::UploadThemeListVerTask(void)
{
	m_listener = NULL;
	
	m_seq = 0;
	m_errType = LCC_ERR_FAIL;
	m_errMsg = "";

	m_themeVer = 0;
}

UploadThemeListVerTask::~UploadThemeListVerTask(void)
{
}

// 初始化参数
bool UploadThemeListVerTask::InitParam(int themeVer)
{
	m_themeVer = themeVer;
	return true;
}

// 初始化
bool UploadThemeListVerTask::Init(ILiveChatClientListener* listener)
{
	bool result = false;
	if (NULL != listener) 
	{
		m_listener = listener;
		result = true;
	}
	return result;
}
	
// 处理已接收数据
bool UploadThemeListVerTask::Handle(const TransportProtocol* tp)
{
	bool result = false;
	// 本命令无返回	
	return result;
}
	
// 获取待发送的数据，可先获取data长度，如：GetSendData(NULL, 0, dataLen);
bool UploadThemeListVerTask::GetSendData(void* data, unsigned int dataSize, unsigned int& dataLen)
{
	bool result = false;
	
	// 构造json协议
	Json::Value root(m_themeVer);
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
TASK_PROTOCOL_TYPE UploadThemeListVerTask::GetSendDataProtocolType()
{
	return JSON_PROTOCOL;
}
	
// 获取命令号
int UploadThemeListVerTask::GetCmdCode()
{
	return TCMD_UPLOADTHEMELISTVER;
}
	
// 设置seq
void UploadThemeListVerTask::SetSeq(unsigned int seq)
{
	m_seq = seq;
}

// 获取seq
unsigned int UploadThemeListVerTask::GetSeq()
{
	return m_seq;
}
	
// 是否需要等待回复。若false则发送后释放(delete掉)，否则发送后会被添加至待回复列表，收到回复后释放
bool UploadThemeListVerTask::IsWaitToRespond()
{
	return false;
}
	
// 获取处理结果
void UploadThemeListVerTask::GetHandleResult(LCC_ERR_TYPE& errType, string& errMsg)
{
	errType = m_errType;
	errMsg = m_errMsg;
}

// 未完成任务的断线通知
void UploadThemeListVerTask::OnDisconnect() 
{
	// 不用回调
}