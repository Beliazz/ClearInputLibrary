#ifndef CILHashedString_h__
#define CILHashedString_h__

#include "CIL.h"

namespace cil
{

extern void* hash_name(char const * pIdentStr);

#define kpWildcardEventType "*"

typedef class CILHashedString EventType;
class CILHashedString
{
private:
	// note: m_ident is stored as a void* not an int, so that in
	// // the debugger it will show up as hex-values instead of  
	// // integer values. This is a bit more representative of what 
	// // we're doing here and makes it easy to allow external code
	// // to assign event types as desired
	void* m_ident;
	char const * m_identStr;

public:
	explicit CILHashedString (char const * const pIdentStr)
		: m_ident(hash_name(pIdentStr)),
		  m_identStr(pIdentStr)
	{}

	unsigned long getIdent() const
	{
		return reinterpret_cast<unsigned long>(m_ident);
	}
	char const * const getStr() const {return m_identStr;}

	bool operator < ( EventType const & o) const
	{
		return (getIdent() < o.getIdent());
	}
	bool operator ==( EventType const & o) const
	{
		return (getIdent() == o.getIdent());
	}	
};

}


#endif // HashedString_h__