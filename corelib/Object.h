#pragma once
#include "Enums.h"
#include <mds\mds-lib.h>

namespace CoreLib
{
	class ObjectRoot
	{
	protected:
		ObjectRoot() throw(...);
		ObjectRoot(ObjectRoot const & source) throw(...);

	public:
		virtual ~ObjectRoot() = 0;

		inline bool is_alive() const;
	};

	class Object : public ObjectRoot
	{
	protected:
		Object() throw(...);
		Object(Object const & source) throw(...);

	public:
		virtual ~Object() = 0;

		virtual Error ProcessMessage(mds::msg_ptr_base & message) = 0;

		static std::vector<std::string> x;
	};
}
