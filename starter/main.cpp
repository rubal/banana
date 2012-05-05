#include "../view/mds-lib.h"
#include "../model/console.h"
#include <conio.h>
#include <memory>

//#if !defined(NDEBUG)
//#define new DEBUG_CLIENTBLOCK
//#endif

class TickMessage : public mds::Message
{
public:
    TickMessage() { hbz = 153; }
    ~TickMessage() {}

    int hbz;
};

class Clock : public mds::Object
{
    typedef mds::Object super;

    const char * msg;
    int x;
    int y;
    int delay;
    bool tick;

public:
    Clock(const char * msg, int x, int y, int delay)
    {
        this->tick = true;
        this->msg = msg;
        this->x = x;
        this->y = y;
        this->delay = delay;
    }

    ~Clock()
    {
    }

    void Init()
    {
        postTickMsg();
    }

    virtual mds::Error ProcessMessage(mds::msg_ptr_base & message)
    {
        message;

        this->tick = !this->tick;
        this->readraw();
        postTickMsg();
        return mds::Error_Ok;
    }

private:
    inline void postTickMsg()
    {
        mds::msg_ptr<TickMessage> msg(new TickMessage());
        mds::MessageLoop::PostMessageToLoop(this, msg, this->delay);
    }

    void readraw()
    {
        ConsoleColors color((ConsoleColor)(
            ConsoleColor_Bright |
            ConsoleColor_Red |
            ConsoleColor_Green), 
            ConsoleColor_Red);

        if (this->tick)
            color.SetBackground(ConsoleColor_Green);

        Console::SetColor(color);
        Console::GotoXY(this->x, this->y);
        Console::OutTxt("%s\n", this->msg);
    }
};

class Dummy : public mds::Object
{
public:
    Dummy()
    {
    }
    ~Dummy()
    {
    }

    virtual mds::Error ProcessMessage(mds::msg_ptr_base & message)
    {
        if (mds::KeyboardMessage * msg =
                dynamic_cast<mds::KeyboardMessage *>(message.get()))
        {
            Console::GotoXY(0, 2);
            Console::OutTxt("pressed: %d\n", msg->KeyCode());

            switch (msg->KeyCode())
            {
            case Console::KEY_ESC:
                return mds::Error_Fatal;
            }
        }
        else if (mds::SystemMessage * msg =
                dynamic_cast<mds::SystemMessage *>(message.get()))
        {
            Console::GotoXY(0, 7);

            switch (msg->Code())
            {
            case mds::SystemMessage_EnterMessageLoop:
                Console::OutTxt("ENTER");
                break;

            case mds::SystemMessage_ExitMessageLoop:
                Console::OutTxt("EXIT");
                break;
            case mds::SystemMessage_Idle:
                Console::OutTxt("IDLE");
                break;
            }
            Console::GotoXY(0, 9);
        }
        return mds::Error_Ok;
    }
};


class Game : public mds::Object
{
public:
    Game()
        : message(new TickMessage())
        , clock1("#1", 0, 0, 1000)
        , clock2("-2", 10, 1, 500)
    {
    }
    
    ~Game()
    {
    }

    virtual mds::Error ProcessMessage(mds::msg_ptr_base & message)
    {
        if (mds::KeyboardMessage * msg =
                dynamic_cast<mds::KeyboardMessage *>(message.get()))
        {
            Console::GotoXY(0, 2);
            Console::OutTxt("pressed: %d\n", msg->KeyCode());

            switch (msg->KeyCode())
            {
            case Console::KEY_ESC:
                return mds::Error_Fatal;
            default:
                this->onKeyboard(msg->KeyCode());
            }
        }
        else if (mds::SystemMessage * msg =
                dynamic_cast<mds::SystemMessage *>(message.get()))
        {
            Console::GotoXY(0, 6);

            switch (msg->Code())
            {
            case mds::SystemMessage_EnterMessageLoop:
                Console::OutTxt("ENTER");
                this->onEnter();
                break;

            case mds::SystemMessage_ExitMessageLoop:
                Console::OutTxt("EXIT");
                break;
            case mds::SystemMessage_Idle:
                Console::OutTxt("IDLE");
                break;
            }
            Console::GotoXY(0, 9);
        }
        return mds::Error_Ok;
    }

private:
    mds::msg_ptr<TickMessage> message;
    Clock clock1;
    Clock clock2;

    void onEnter()
    {
        clock1.Init();
        mds::MessageLoop::PostMessageToLoop(
            &clock1,
            mds::msg_ptr_base(new TickMessage()),
            0);
        clock2.Init();
    }

    void onKeyboard(int keyCode)
    {
        if (keyCode == 9)
        {
            for (int i = 0; i < 9; )
               mds::MessageLoop::PostMessageToLoop(&clock1, message, (++i) * 200);

            Dummy dummy;
            mds::MessageLoop::Run(dummy);
        }
    }
};

void main()
{
    debug_initMemLeaksCheck();

#if 0
    mds::msg_ptr<TickMessage> p1(new TickMessage());
    {
        TickMessage & mr = *p1;
        TickMessage * mp = p1.get();

        mds::msg_ptr_base p2 = p1;
    }
#endif

    Game game;

    mds::ObjectManager::is_registered(game);

    mds::MessageLoop::Run(game);
}
