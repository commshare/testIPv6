/*
 * author: Samson.Fan
 *   date: 2016-04-25
 *   file: RecvThemeMotionTask.cpp
 *   desc: 接收播放主题包动画Task实现类
 */

#include "RecvThemeMotionTask.h"
#include "ITaskManager.h"
#include "ILiveChatClient.h"
#include "AmfPublicParse.h"
#include <json/json/json.h>
#include <common/CheckMemoryLeak.h>

// 返回参数
#define	THEMEID_PARAM		"subjectId"
#define MANID_PARAM			"manId"
#define WOMANID_PARAM		"womanId"

RecvThemeMotionTask::RecvThemeMotionTask(void)
{
	m_listener = NULL;

	m_seq = 0;
	m_errType = LCC_ERR_FAIL;
	m_errMsg = "";

	m_manId = "";
	m_womanId = "";
	m_themeId = "";
}

RecvThemeMotionTask::~RecvThemeMotionTask(void)
{
}

// 初始化
bool RecvThemeMotionTask::Init(ILiveChatClientListener* listener)
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
bool RecvThemeMotionTask::Handle(const TransportProtocol* tp)
{
	bool result = false;
		
	AmfParser parser;
	amf_object_handle root = parser.Decode((char*)tp->data, tp->GetDataLength());
	if (!root.isnull()
		&& root->type == DT_OBJECT)
	{
		// themeId
		amf_object_handle themeIdObject = root->get_child(THEMEID_PARAM);
		if (!themeIdObject.isnull()
			&& themeIdObject->type == DT_STRING)
		{
			m_themeId = themeIdObject->strValue;
		}

		// manId
		amf_object_handle manIdObject = root->get_child(MANID_PARAM);
		if (!manIdObject.isnull()
			&& manIdObject->type == DT_STRING)
		{
			m_manId = manIdObject->strValue;
		}

		// womanId
		amf_object_handle womanIdObject = root->get_child(WOMANID_PARAM);
		if (!womanIdObject.isnull()
			&& womanIdObject->type == DT_STRING)
		{
			m_womanId = womanIdObject->strValue;
		}

		result = true;
	}

	// 通知listener
	if (NULL != m_listener 
		&& result) 
	{
		m_listener->OnRecvThemeMotion(m_themeId, m_manId, m_womanId);
	}
	
	return result;
}
	
// 获取待发送的数据，可先获取data长度，如：GetSendData(NULL, 0, dataLen);
bool RecvThemeMotionTask::GetSendData(void* data, unsigned int dataSize, unsigned int& dataLen)
{
	bool result = false;
	// 本协议没有返回
	return result;
}

// 获取待发送数据的类型
TASK_PROTOCOL_TYPE RecvThemeMotionTask::GetSendDataProtocolType()
{
	return JSON_PROTOCOL;
}
	
// 获取命令号
int RecvThemeMotionTask::GetCmdCode()
{
	return TCMD_RECVTHEMEMOTION;	
}

// 设置seq
void RecvThemeMotionTask::SetSeq(unsigned int seq)
{
	m_seq = seq;
}

// 获取seq
unsigned int RecvThemeMotionTask::GetSeq()
{
	return m_seq;
}

// 是否需要等待回复。若false则发送后释放(delete掉)，否则发送后会被添加至待回复列表，收到回复后释放
bool RecvThemeMotionTask::IsWaitToRespond()
{
	return false;
}

// 获取处理结果
void RecvThemeMotionTask::GetHandleResult(LCC_ERR_TYPE& errType, string& errMsg)
{
	errType = m_errType;
	errMsg = m_errMsg;
}

// 未完成任务的断线通知
void RecvThemeMotionTask::OnDisconnect()
{
	// 不需要回调
}
