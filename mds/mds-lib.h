#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <corelib\Object.h>
#include <corelib\ObjectManager.h>

#if defined(NDEBUG)
#   pragma comment(lib, "mds-Release.lib")
#else
#   pragma comment(lib, "mds-Debug.lib")
#	define _CRTDBG_MAP_ALLOC
#	include <crtdbg.h>
#   define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

using namespace CoreLib;

void debug_initMemLeaksCheck();

namespace mds
{
    enum Error
    {
        Error_Ok,
        Error_Fatal
    };

    class Message
    {
    private:
        friend class msg_ptr_base;

        long _refCounter;

        long AddRef()
        {
            return ++this->_refCounter;
        }

        long Release()
        {
            if (--this->_refCounter != 0)
                return this->_refCounter;
   
            delete this;
            return 0;
        }

    protected:
        Message();

    public:
        virtual ~Message();
    };

    class msg_ptr_base
    {
    public:
        msg_ptr_base()
        {
            this->_message = NULL;
        }

        msg_ptr_base(Message * ptrToMessage)
        {
            this->_message = ptrToMessage;

            if (this->_message != NULL)
                this->_message->AddRef();
        }

        msg_ptr_base(msg_ptr_base const & src)
        {
            this->_message = src._message;

            if (this->_message != NULL)
                this->_message->AddRef();
        }

        ~msg_ptr_base()
        {
            if (this->_message != NULL)
                this->_message->Release();
        }

        
        inline Message * get() {
            return this->_message; }

        inline Message * operator->() {
            return this->_message; }

        inline Message & operator*() {
            return *this->_message; }

        inline const Message * get() const {
            return const_cast<msg_ptr_base *>(this)->get(); }

        inline const Message * operator->() const {
            return const_cast<msg_ptr_base *>(this)->operator->(); }

        inline const Message & operator*() const {
            return const_cast<msg_ptr_base *>(this)->operator*(); }

        inline msg_ptr_base & operator=(msg_ptr_base const & rhs)
        {
            if (this !=  &rhs)
            {
                if (this->_message != NULL)
                    this->_message->Release();

                this->_message = rhs._message;

                if (this->_message != NULL)
                    this->_message->AddRef();
            }
            return *this;
        }

        inline msg_ptr_base & operator=(Message * ptrToMessage)
        {
            if (this->_message !=  ptrToMessage)
            {
                if (this->_message != NULL)
                    this->_message->Release();

                this->_message = ptrToMessage;

                if (this->_message != NULL)
                    this->_message->AddRef();
            }
            return *this;
        }

    
    private:
        Message * _message;
    };

#if 1
    template <class MessageClass>
    class msg_ptr : public msg_ptr_base
    {
        typedef msg_ptr_base super;

    public:
        msg_ptr()
        {
        }

        msg_ptr(MessageClass * ptrToMessage)
            : super(ptrToMessage)
        {
        }

        inline MessageClass * get() {
            return static_cast<MessageClass *>(this->super::get()); }

        inline const MessageClass * get() const {
            return const_cast<mgs_ptr *>(this)->get(); }

        inline MessageClass * operator->() {
            return static_cast<MessageClass *>(this->super::operator->()); }

        inline const MessageClass * operator->() const {
            return const_cast<mgs_ptr *>(this)->operator->(); }

        inline MessageClass & operator*() {
            return static_cast<MessageClass &>(this->super::operator*()); }

        inline const MessageClass & operator*() const {
            return const_cast<mgs_ptr *>(this)->operator*(); }

        msg_ptr & operator=(msg_ptr const & rhs)
        {
            super::operator=(rhs);
            return *this;
        }

        msg_ptr & operator=(MessageClass * rhs)
        {
            super::operator=(rhs);
            return *this;
        }
    };
#endif

    enum SystemMessageCode
    {
        SystemMessage_Idle,
        SystemMessage_EnterMessageLoop,
        SystemMessage_ExitMessageLoop,
        SystemMessage_Quit
    };

    class SystemMessage : public Message
    {
    public:
        inline SystemMessageCode Code() const {
            return this->_code; }

    private:
        inline SystemMessage(SystemMessageCode code)
        {
            this->_code = code;
        }

        inline ~SystemMessage()
        {
        }

        SystemMessageCode _code;

        friend class MessageLoop;
    };
    
    class KeyboardMessage : public Message
    {
    public:
        KeyboardMessage(int keyCode);

        virtual ~KeyboardMessage();

        inline int KeyCode() const {
            return this->keyCode; }

    private:
        int keyCode;
    };



  



    class MessageLoop
    {
    public:
        static void PostMessageToLoop(Object * reciever, mds::msg_ptr_base const & message, int delay);
        static void PostQuitMessage();

        static Error Run(Object & dispatcher);

    private:
        MessageLoop();
        ~MessageLoop();

        MessageLoop(MessageLoop const &);
        MessageLoop & operator=(MessageLoop const &);
    };
}

inline bool ObjectRoot::is_alive() const
{
    return ObjectManager::is_registered(*this);
}
