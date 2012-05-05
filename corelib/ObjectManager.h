#pragma once
#include <list>
#include "Object.h"

using namespace std;

namespace CoreLib
{

	class ObjectManager
    {
    public:
        static bool is_registered(ObjectRoot const &);

    private:
        virtual ~ObjectManager();
        ObjectManager();
        ObjectManager(ObjectManager const &);
        ObjectManager & operator=(ObjectManager const &);

        friend ObjectRoot;

        class Impl;
        friend class Impl;
    };

    class ObjectManager::Impl: public ObjectManager
    {
    public:
        static ObjectManager::Impl & instance();

        bool is_registered(ObjectRoot const &) const;

        void registerObject(ObjectRoot &) throw (...);
        void unregisterObject(ObjectRoot &);

    private:
		list<ObjectRoot> *objectList;
        static Impl * _instance;

        ~Impl();
        Impl();

        Impl(Impl const &);
        Impl & operator=(Impl const &);
    };
}