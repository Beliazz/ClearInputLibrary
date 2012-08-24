#ifndef CILInput_h__
#define CILInput_h__

#include "CIL.h"

namespace cil
{

typedef CIL_API ULONG ID;

class CIL_API CILInput
{
protected:
	// event listeners
	std::map<ID, ICILKeyboardHandler*>	m_keyboardListeners;
	std::map<ID, ICILMouseHandler*>	m_mouseListeners;

public:
	CILInput() {}
	~CILInput(){}

	void AddListener(ICILKeyboardHandler* pKeyboardHandler);
	void AddListener(ICILMouseHandler* pMouseHandler);
	void RemoveListener(ICILMouseHandler* pMouseHandler);
	void RemoveListener(ICILKeyboardHandler* pKeyboardHandler);
};

}

#endif // CILInput_h__

