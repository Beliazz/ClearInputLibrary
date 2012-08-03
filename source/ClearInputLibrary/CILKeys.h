#ifndef CILKeys_h__
#define CILKeys_h__

#include "CIL.h"

namespace cil
{

// map virtual key codes
typedef USHORT CIL_KEY;

// for internal use
struct Key
{
	CIL_KEY key;
	bool down;

	Key(CIL_KEY key, bool down)
	{
		this->key = key;
		this->down = down;
	}
};

}

#endif // Keys_h__
