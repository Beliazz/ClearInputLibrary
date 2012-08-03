#include "CIL.h"
 
void cil::CILInput::AddListener( cil::ICILMouseHandler* pMouseHandler )
{
	CILHashedString id(pMouseHandler->GetName().c_str());
	std::pair<ID, ICILMouseHandler*> entry;
	entry = std::make_pair(id.getIdent(), pMouseHandler);
	m_mouseListeners.insert(entry);
}
void cil::CILInput::AddListener( cil::ICILKeyboardHandler* pKeyboardHandler )
{
	CILHashedString id(pKeyboardHandler->GetName().c_str());
	std::pair<ID, ICILKeyboardHandler*> entry;
	entry = std::make_pair(id.getIdent(), pKeyboardHandler);
	m_keyboardListeners.insert(entry);
}
void cil::CILInput::RemoveListener( cil::ICILMouseHandler* pMouseHandler )
{
	CILHashedString id(pMouseHandler->GetName().c_str());
	m_mouseListeners.erase(id.getIdent());
}
void cil::CILInput::RemoveListener( cil::ICILKeyboardHandler* pKeyboardHandler )
{
	CILHashedString id(pKeyboardHandler->GetName().c_str());
	m_keyboardListeners.erase(id.getIdent());
}