#ifndef __TASK_H__
#define __TASK_H__

#include <string.h>
#include "types.h"
#include "thread.h"
#include "mutex.h"

#define TASK_NAME_LEN       48
#define TASK_NAME_PREFIX    "live_"

typedef struct T_Node
{
    UInt64 u64Priority;
    void  *pValue;
    T_Node() : u64Priority(0), pValue(NULL) {}
    T_Node(UInt64 i_u64Priority, void *i_pValue) : u64Priority(i_u64Priority), pValue(i_pValue) {}
    friend bool operator< (T_Node n1, T_Node n2)
    {
        // Ĭ��Ԫ��Խ�����ȼ�Խ��; ������������ҪԪ��ԽС�����ȼ�Խ��
        return n1.u64Priority > n2.u64Priority;
    }
}T_Node;

typedef priority_queue<T_Node>  PriorityQueue;

class Task
{
public:
    Task() : m_Events(0), m_pName(NULL)
    {
        SetTaskName("UnnamedTask");
    }
    virtual ~Task()
    {
        ResetEvents();
        if (m_pName)
        {
            delete[] m_pName;
            m_pName = NULL;
        }
    }

    typedef unsigned int EventFlags;
    // EVENTS
    // here are all the events that can be sent to a task
    enum
    {
        kKillEvent    = 0x1 << 0x0,
        kIdleEvent    = 0x1 << 0x1,
        kStartEvent   = 0x1 << 0x2,
        kTimeoutEvent = 0x1 << 0x3,

        // socket events
        kReadEvent    = 0x1 << 0x4,
        kWriteEvent   = 0x1 << 0x5,

        // update event
        kUpdateEvent  = 0x1 << 0x6,
        kMsgEvent     = 0x1 << 0x7
    };

    // return:
    // >0 invoke me after this number of MilSecs with a kIdleEvent
    // 0 don't reinvoke me at all.
    // -1 delete me
    virtual SInt64 Run() = 0;

    // Send an event to this task.
    void PushEvents(Task::EventFlags i_Events, SInt64 i_s64Priority=0);

    EventFlags PopEvents();
    EventFlags GetEvents() { return m_Events; }
    void ResetEvents() { m_Events = 0; }

    inline void SetTaskName(const string &i_strName)
    {
        if (NULL == m_pName)
        {
            m_pName = new char[TASK_NAME_LEN];
        }
        strncpy(m_pName, TASK_NAME_PREFIX, 5);
        strncat(m_pName, i_strName.c_str(), TASK_NAME_LEN-5-1);
    }
    inline bool IsAlive()
    {
        return m_pName && (0 == strncmp(m_pName, TASK_NAME_PREFIX, 5));
    }
private:
    Task::EventFlags m_Events;
    char *m_pName;
    bool m_bIsAlive;
};

typedef queue<Task*>            TaskQueue;
typedef set<Task*>              TaskSet;

class TaskThread: public Thread
{
public:
    TaskThread() : Thread()
    {
    }
    ~TaskThread();

    void Entry();

    void AddTask(Task* i_pTask, SInt64 i_s64Priority=0);

private:
    Task* WaitForTask();
private:
    PriorityQueue m_TimeoutTaskQueue;
    TaskQueue m_TaskQueue;
    TaskSet m_TaskSet;
    Mutex m_lock;
    Cond m_cond;
};

typedef vector<TaskThread*>  ThreadVector;

class TaskThreadPool
{
public:
    static void AddThreads(UInt32 i_u32Num);
    static void RemoveThreads();
    static void AddTask(Task* i_pTask, SInt64 i_s64Priority=0);
    static SInt32 GetThreadNum() { return m_u32ThreadNum; }
private:
    static UInt32 m_u32ThreadNum;
    static UInt32 m_u32ThreadPicker;
    static ThreadVector m_vecThreads;
};

#endif
