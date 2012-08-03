#ifndef CILInputHandler_h__
#define CILInputHandler_h__

#include "CIL.h"

namespace cil
{

//////////////////////////////////////////////////////////////////////////
// core interfaces
class CIL_API ICILHandler
{
public:
	virtual std::string GetName() PURE;
};

class CIL_API ICILKeyboardHandler : public ICILHandler
{
public:
	virtual void OnKeyDown(const CIL_KEY key) PURE;
	virtual void OnKeyUp(const CIL_KEY key)	PURE;
};

class CIL_API ICILMouseHandler  : public ICILHandler
{
public:
	virtual void OnMouseMove(const POINT &mousePos, const POINT& delta) PURE;
	virtual void OnLButtonDown(const POINT &mousePos)					PURE;
	virtual void OnLButtonUp(const POINT &mousePos)						PURE;
	virtual void OnRButtonDown(const POINT &mousePos)					PURE;
	virtual void OnRButtonUp(const POINT &mousePos)						PURE;
	virtual void OnMButtonDown(const POINT &mousePos)					PURE;
	virtual void OnMButtonUp(const POINT &mousePos)						PURE;
	virtual void OnWheelMove(const POINT &mousePos, const short delta)	PURE;
};

//////////////////////////////////////////////////////////////////////////
// keyboard handler implementation
class CIL_API CILKeyboardState : public ICILKeyboardHandler
{
protected:
	std::string m_name;
	BYTE m_keys[256];
	HKL m_keyboardLayout;

public:
	CILKeyboardState(std::string name);

	inline std::string GetName()				{ return m_name; }
	inline BYTE* GetState()						{ return m_keys; }
	inline BYTE operator[] (USHORT index)		{ return m_keys[index]; }
	inline operator BYTE*()						{ return m_keys;}

	void OnKeyDown(const CIL_KEY key);
	void OnKeyUp(const CIL_KEY key);	

	virtual void CustomOnKeyDown(const CIL_KEY key) { };
	virtual void CustomOnKeyUp(const CIL_KEY key) { };

	std::string	ToAscii( CIL_KEY key);
	std::wstring ToUnicode( CIL_KEY key);
};

}

#endif // InputHandler_h__