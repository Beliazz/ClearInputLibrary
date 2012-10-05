#ifndef CILRawInput_h__
#define CILRawInput_h__

#include "CIL.h"

namespace cil
{

class CIL_API CILRawInput : public CILInput
{
private:
	BYTE	m_buffer[256];
	LCID	m_localeID;

	SHORT	m_wheelDelta;
	POINT	m_vRelPosition;
	POINT	m_vDelta;

	std::vector<Key>	m_unprocessed;
	std::vector<Key>	m_keyboard;
	std::vector<Key>	m_mouse;

	BOOL			m_shift;
	BOOL			m_numLock;
	BOOL			m_capsLock;
	BOOL			m_alt;
	BOOL			m_bNewInput;

	void Split( std::vector<Key>& keys, std::vector<Key>* pMouse, std::vector<Key>* pKeys );

public:
	virtual bool Init();
	virtual void Exit();

	void ProcessInput(LPARAM lparam);
	void Update();

	char GetCharFromKey( CIL_KEY key );

	inline LCID GetLocaleID() {return m_localeID;}

	virtual void QTKeyUP(int key);
	virtual void QTKeyDown(int key);

	virtual void QtLButton(POINT cursor, bool down);
	virtual void QtRButton(POINT cursor, bool down);
	virtual void QtMiddleButton(POINT cursor, bool down);

	virtual void QtMouseMove(POINT cursor, POINT delta);
};

}

#endif // CILRawInput_h__

