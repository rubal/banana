#include "../mds-lib.h"
#include <assert.h>
#include <conio.h>
#include <time.h>
#include <limits.h>
#include <memory>
#include <console\console.h>
#include <corelib\Object.h>
#include "internals.h"
using namespace CoreLib;

#ifndef NDEBUG
#
	void debug_initMemLeaksCheck()
	{
		int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
		_CrtSetDbgFlag(tmpDbgFlag);
	}
#
#else
#
	void debug_initMemLeaksCheck()
	{
		return;
	}
#
#endif

//#if !defined(NDEBUG)
//#define new DEBUG_CLIENTBLOCK
//#endif

//--------------------------


mds::Message::Message()
{
    this->_refCounter = 0;
}

mds::Message::~Message()
{
    assert(this->_refCounter == 0);
}

mds::KeyboardMessage::KeyboardMessage(int keyCode)
{
    this->keyCode = keyCode;
}

mds::KeyboardMessage::~KeyboardMessage()
{
}

ObjectRoot::ObjectRoot() throw(...)
{
    ObjectManager::Impl & inst = ObjectManager::Impl::instance();
    inst.registerObject(*this);
}

ObjectRoot::ObjectRoot(ObjectRoot const &) throw(...)
{
    ObjectManager::Impl & inst = ObjectManager::Impl::instance();
    inst.registerObject(*this);
}


ObjectRoot::~ObjectRoot()
{
    ObjectManager::Impl & inst = ObjectManager::Impl::instance();
    inst.unregisterObject(*this);
}

  
Object::Object() throw(...)
{
}

Object::Object(Object const &) throw(...)
{
}

Object::~Object()
{
}

class Package
{
public:
    Object * reciever;
    mds::msg_ptr_base message;
};

static class Queue * currentQueue = NULL;

class Queue
{
    static const clock_t MaxTime = LONG_MAX;

private:
    class MSG
    {
    public:
        MSG()
        {
            this->next = this->prev = this;

            this->package.reciever = NULL;
            this->package.message = NULL;

            this->arriveTime = MaxTime;
        }

        MSG(Object * reciever, mds::msg_ptr_base const & message, clock_t arriveTime)
        {
            assert(0 <= arriveTime && arriveTime < MaxTime);

            this->next = this->prev = NULL;
            this->package.reciever = reciever;
            this->package.message = message;
            this->arriveTime = arriveTime;
        }

        MSG * next;
        MSG * prev;

        Package package;
        clock_t arriveTime;

        static clock_t CalcArriveTime(int delay) // ms
        {
            clock_t delayInClocks = (delay * CLOCKS_PER_SEC) / 1000;
            clock_t result = clock() + delayInClocks;
            return result;
        }

        static void Post(MSG & barier, MSG * msg)
        {
            MSG * tmp = barier.next;
            while (tmp->arriveTime <= msg->arriveTime)
            {
                tmp = tmp->next;
            }
            assert(tmp->arriveTime > msg->arriveTime);
            
            (((msg->prev = tmp->prev)->next = msg)->next = tmp)->prev = msg;
        }

        static void Clear(MSG & barier)
        {
            for (;;)
            {
                MSG * firstMessage = barier.next;
                if (firstMessage == &barier)
                    return;

                (firstMessage->next->prev = firstMessage->prev)->next = firstMessage->next;
                delete firstMessage;
            }
        }
        static std::auto_ptr<MSG> Get(MSG & barier)
        {
            MSG * firstMessage = barier.next;
            if (firstMessage == &barier)
                return std::auto_ptr<MSG>(NULL);

            clock_t currentTime = clock();
            
            if (firstMessage->arriveTime > currentTime)
                return std::auto_ptr<MSG>(NULL);
                
            (firstMessage->next->prev = firstMessage->prev)->next = firstMessage->next;
            firstMessage->next = firstMessage->prev = NULL;

            return std::auto_ptr<MSG>(firstMessage);
        }
    };

    MSG barier;

    Queue * prevQueue;

    clock_t freezeTime;

    void MarkStopped()
    {
        this->freezeTime = clock();
    }
    
    void MarkRestored()
    {
        clock_t delta = clock() - this->freezeTime;

        for (MSG * tmp = barier.next; tmp != &barier; tmp = tmp->next)
            tmp->arriveTime += delta;

        this->freezeTime = 0;
   }

public:
    Queue()
    {
        this->freezeTime = 0;
        this->prevQueue = currentQueue;
        if (this->prevQueue != NULL)
            this->prevQueue->MarkStopped();
        currentQueue = this;
    }
    
    ~Queue()
    {
        MSG::Clear(this->barier);

        currentQueue = this->prevQueue;
        if (this->prevQueue != NULL)
            this->prevQueue->MarkRestored();
    }

    void PostMessageToLoop(
        Object * reciever,
        mds::msg_ptr_base const & message,
        int delay)
    {
        clock_t arriveTime = MSG::CalcArriveTime(delay);

        MSG * msg = new MSG(reciever, message, arriveTime);
        MSG::Post(this->barier, msg);
    }

     bool Get(Package & package)
     {
         std::auto_ptr<MSG> msg = MSG::Get(this->barier);
         if (msg.get())
         {
             package = msg->package;
             return true;
         }
         else
         {
             package.reciever = NULL;
             package.message = NULL;
             return false;
         }
     }
};

void mds::MessageLoop::PostMessageToLoop(Object * reciever,
                                   mds::msg_ptr_base const & message,
                                   int delay)
{
    assert(currentQueue != NULL);
    currentQueue->PostMessageToLoop(reciever, message, delay);
}

void mds::MessageLoop::PostQuitMessage()
{
    static msg_ptr<SystemMessage> quitMessage(new SystemMessage(mds::SystemMessage_Quit));
    currentQueue->PostMessageToLoop(NULL, quitMessage, 0);
}

mds::Error mds::MessageLoop::Run(Object & dispatcher)
{
    static mds::msg_ptr<mds::SystemMessage> idleMessage(
        new mds::SystemMessage(mds::SystemMessage_Idle));

    static mds::msg_ptr<mds::SystemMessage> enterMessage(
        new mds::SystemMessage(mds::SystemMessage_EnterMessageLoop));

    static mds::msg_ptr<mds::SystemMessage> exitMessage(
        new mds::SystemMessage(mds::SystemMessage_ExitMessageLoop));

    Queue queue;

    assert(&dispatcher != NULL);
    mds::Error error = mds::Error_Ok;
    {
        dispatcher.ProcessMessage(enterMessage);
    }
    Package package;
    do
    {
        if (Console::KeyPressed())
        {
            int key = Console::GetKey();

            msg_ptr<KeyboardMessage> kbdMsg(new KeyboardMessage(key));
            error = dispatcher.ProcessMessage(kbdMsg);
        }
        else if (currentQueue->Get(package))
        {
            if (package.reciever != NULL)
                error = package.reciever->ProcessMessage(package.message);
            else
                error = dispatcher.ProcessMessage(package.message);
        }
        else
        {
            dispatcher.ProcessMessage(idleMessage);
        }
    } while (error == mds::Error_Ok);
    {
        dispatcher.ProcessMessage(exitMessage);
    }
    return error;
}

