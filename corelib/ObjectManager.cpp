#include <assert.h>
#include <conio.h>
#include <time.h>
#include <limits.h>
#include <memory>
#include <console\console.h>
#include "ObjectManager.h"
using namespace CoreLib;



ObjectManager::ObjectManager()
{
}

ObjectManager::~ObjectManager()
{
}

bool ObjectManager::is_registered(ObjectRoot const & obj)
{
    Impl & instance = Impl::instance();
    return instance.is_registered(obj);
}

////////////////////////////////////////////////////////////
// mds::ObjectManager::Impl

bool ObjectManager::Impl::is_registered(ObjectRoot const & obj) const
{
    assert(("Invalid reference!", &obj != NULL));

    //TODO ...

    return false;
}

void ObjectManager::Impl::registerObject(ObjectRoot & obj)
{
    if (this->is_registered(obj))
        throw std::exception("mds::Object is already registered!");

    try
    {
		this->objectList->push_back(obj);
    }
    catch (...)
    {
        //TODO....
        throw;
    }
}

void ObjectManager::Impl::unregisterObject(ObjectRoot & obj)
{
    if (!this->is_registered(obj))
        throw std::exception("mds::Object is not registered!");

    try
    {
		this->objectList->remove(obj);
    }
    catch (...)
    {
        //TODO....
        throw;
    }
}

ObjectManager::Impl::Impl()
{
    assert(("mds::ObjectManager is already created!", _instance == NULL));
	this->objectList = new list<ObjectRoot>();
    _instance = this;
}

ObjectManager::Impl::~Impl()
{
    _instance = NULL;
}

ObjectManager::Impl & ObjectManager::Impl::instance()
{
    static Impl instanse;

    Impl * result = _instance;
    assert(("mds::ObjectManager is already deleted!", result != NULL));

    return *result;
}

ObjectManager::Impl * ObjectManager::Impl::_instance = NULL;
