#include "CIL.h"

cil::CILKeyboardState::CILKeyboardState( std::string name )
{
	m_name = name;

	// get initial key state
	GetKeyboardState(m_keys);

	// get keyboard layout
	m_keyboardLayout = GetKeyboardLayout(0);
}

std::string cil::CILKeyboardState::ToAscii( CIL_KEY key )
{
	std::string out;
	int scan;
	USHORT buff[2];

	scan = MapVirtualKeyEx(key, 0, m_keyboardLayout);
	if ( ToAsciiEx(key, scan, m_keys, buff, 0, m_keyboardLayout) > 0)
	{
		out += (char)buff[0];
	};
	
	return out;
}

std::wstring cil::CILKeyboardState::ToUnicode( CIL_KEY key )
{
	std::wstring out;
	int scan;
	WCHAR buff[32];
	UINT size = 32;

	scan = MapVirtualKeyEx(key, 0, m_keyboardLayout);
	int numChars = ToUnicodeEx(key, scan, m_keys, buff, size, 0, m_keyboardLayout);

	if (numChars == 1)
	{
		out.push_back(buff[0]);
	}
	else if (numChars > 1)
	{
		WCHAR* pTemp = new WCHAR[numChars + 1];
		memcpy(pTemp, buff, numChars * sizeof(WCHAR));
		pTemp[numChars] = L'\0';

		out += std::wstring(pTemp);
	}

	return out;
}

void cil::CILKeyboardState::OnKeyDown( const CIL_KEY key )
{	
	m_keys[key] |= 129;
	if (key == VK_MENU)
	{
		m_keys[VK_CONTROL] |= 128;
	}

	CustomOnKeyDown(key);
}

void cil::CILKeyboardState::OnKeyUp( const CIL_KEY key )
{
	switch (key)
	{
	case VK_CAPITAL:
	case VK_NUMLOCK:
	case VK_SCROLL:
		{
			if (m_keys[key] & 2)
			{
				 m_keys[key] = 0;
			}
			else
			{
				m_keys[key] = 3;
			}
		} break;
	case VK_MENU:
	{
		m_keys[key] = 0;
		m_keys[VK_CONTROL] = 0;
	}

	default:
		m_keys[key] = 0x00;
	} 

	CustomOnKeyUp(key);
}
