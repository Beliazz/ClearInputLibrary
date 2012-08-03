#include "CIL.h"

bool cil::CILRawInput::Init()
{
	m_vDelta = POINT();
	m_wheelDelta = 0;
	m_bNewInput = false;

	RAWINPUTDEVICE Rid[2];

	// Keyboard
	Rid[0].usUsagePage = 1;
	Rid[0].usUsage = 6;
	Rid[0].hwndTarget=NULL;
	Rid[0].dwFlags = NULL;

	// Mouse
	Rid[1].usUsagePage = 1;
	Rid[1].usUsage = 2;
	Rid[1].hwndTarget=NULL;
	Rid[1].dwFlags = NULL;

	// register input devices
	if (!RegisterRawInputDevices(Rid, 2,sizeof(RAWINPUTDEVICE)))
	{
		return false;
	}

	// check current caps lock state
	BYTE keys[256];
	GetKeyboardState(keys);
	if (keys[VK_CAPITAL] == 0)
	{
		m_capsLock = false;
		m_shift = false;

	}
	else
	{
		m_capsLock = true;
		m_shift = true;
	}

	// check current num lock state
	if (keys[VK_NUMLOCK] == 0)
	{
		m_numLock = false;
	}
	else
	{
		m_numLock = true;
	}

	m_alt = false;

	return true;
}
void cil::CILRawInput::Exit()
{
}
void cil::CILRawInput::ProcessInput(LPARAM lparam)
{
	UINT size = 0;

	// get input data size
	GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));

	if (size == 0 || size > sizeof(m_buffer))
		return;

	// get input data
	GetRawInputData((HRAWINPUT)lparam, RID_INPUT, (LPVOID)&m_buffer, &size, sizeof(RAWINPUTHEADER));
	RAWINPUT* pRaw = (RAWINPUT*) m_buffer;

	if(pRaw)
	{	
		switch(pRaw->header.dwType)
		{
		case RIM_TYPEMOUSE: 
			{
				// Mouse Move
				if(pRaw->data.mouse.lLastX != 0 || pRaw->data.mouse.lLastY != 0)
				{	
					m_vRelPosition.x = GET_X_LPARAM(lparam);
					m_vRelPosition.y = GET_Y_LPARAM(lparam);
					m_vDelta.x += pRaw->data.mouse.lLastX;
					m_vDelta.y += pRaw->data.mouse.lLastY;
					m_bNewInput = true;
				}

				// Left Button
				else if (pRaw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
				{
					m_unprocessed.push_back(Key(VK_LBUTTON, true));
					m_bNewInput = true;
				}
				else if (pRaw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
				{	
					m_unprocessed.push_back(Key(VK_LBUTTON, false));
					m_bNewInput = true;
				}

				// Right Button
				else if (pRaw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
				{	
					m_unprocessed.push_back(Key(VK_RBUTTON, true));
					m_bNewInput = true;
				}
				else if (pRaw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
				{
					m_unprocessed.push_back(Key(VK_RBUTTON, false));
					m_bNewInput = true;
				}

				// Middle Button
				else if (pRaw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
				{
					m_unprocessed.push_back(Key(VK_MBUTTON, true));
					m_bNewInput = true;
				}
				else if (pRaw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
				{	
					m_unprocessed.push_back(Key(VK_MBUTTON, false));
					m_bNewInput = true;
				}

				// Mouse Wheel
				else if (pRaw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
				{
					m_wheelDelta += (short)pRaw->data.mouse.usButtonData;
					m_bNewInput = true;
				}
			}

		case RIM_TYPEKEYBOARD:
			{
				// no key was pressed
				// should in order not be possible
				if (pRaw->data.keyboard.VKey == 0x00)
				{
					break;
				}

				// update key status
				if (pRaw->data.keyboard.Flags & RI_KEY_BREAK)
				{
					m_unprocessed.push_back(Key(pRaw->data.keyboard.VKey, false));
					m_bNewInput = true;
				}
				else
				{
					m_unprocessed.push_back(Key(pRaw->data.keyboard.VKey, true));
					m_bNewInput = true;
				}
			}
		}
	}
}
void cil::CILRawInput::Update()
{
	if (!m_bNewInput)
		return;

	// mouse moved
	if(m_vDelta.x != 0 || m_vDelta.y != 0)
	{
		for(std::map<ID, ICILMouseHandler*>::iterator it = m_mouseListeners.begin(); it != m_mouseListeners.end(); it++)
		{
			it->second->OnMouseMove(m_vRelPosition, m_vDelta);
		}

		ZeroMemory(&m_vDelta, sizeof(POINT));
	}

	// mouse wheel moved
	if(m_wheelDelta != 0)
	{
		for(std::map<ID, ICILMouseHandler*>::iterator it = m_mouseListeners.begin(); it != m_mouseListeners.end(); it++)
		{
			it->second->OnWheelMove(m_vRelPosition, m_wheelDelta);
		}

		m_wheelDelta = 0;
	}

	//  return if no keys were pressed
	if (m_unprocessed.empty())
		return;

	// delete old data
	m_mouse.clear();
	m_keyboard.clear();

	// transform
	Split(m_unprocessed, &m_mouse, &m_keyboard);	

	// clear
	m_unprocessed.clear();

	// mouse buttons pressed
	if (!m_mouse.empty())
	{
		for(std::map<ID, ICILMouseHandler*>::iterator it = m_mouseListeners.begin(); it != m_mouseListeners.end(); it++)
		{
			for (UINT i = 0; i < m_mouse.size(); i++)
			{
				if (m_mouse[i].down)
				{
					switch (m_mouse[i].key)
					{
					case VK_LBUTTON: it->second->OnLButtonDown(m_vRelPosition); break;
					case VK_RBUTTON: it->second->OnRButtonDown(m_vRelPosition); break;
					case VK_MBUTTON: it->second->OnMButtonDown(m_vRelPosition); break;
					}
				}
				else
				{
					switch (m_mouse[i].key)
					{
					case VK_LBUTTON: it->second->OnLButtonDown(m_vRelPosition); break;
					case VK_RBUTTON: it->second->OnRButtonDown(m_vRelPosition); break;
					case VK_MBUTTON: it->second->OnMButtonDown(m_vRelPosition); break;
					}
				}
			}
		}
	}

	// keyboard button pressed
	if (!m_keyboard.empty())
	{
		for(std::map<ID, ICILKeyboardHandler*>::iterator it = m_keyboardListeners.begin(); it != m_keyboardListeners.end(); it++)
		{

			for (UINT i = 0;  i < m_keyboard.size(); i++)
			{
				if(m_keyboard[i].down)
				{
					it->second->OnKeyDown(m_keyboard[i].key);
				}
				else
				{
					it->second->OnKeyUp(m_keyboard[i].key);
				}
			}		
		}
	}

	m_bNewInput = false;
}
void cil::CILRawInput::Split( std::vector<cil::Key>& keys, std::vector<cil::Key>* pMouse,  std::vector<cil::Key>* pKeys)
{
	for (UINT i = 0; i < keys.size(); i++)
	{		
		// mouse input
		if (keys[i].key <= 0x07)
		{
			pMouse->push_back(keys[i]);
			continue;
		}

		// keyboard input
		switch (keys[i].key)
		{
		case VK_SHIFT:
			{
				if (keys[i].down)
				{
					m_shift = true;
				}
				else
				{ 
					if (!m_capsLock) { m_shift = false;}
				}

				break;
			}

		case VK_CAPITAL:
			{
				if (!keys[i].down)
				{ 
					if (!m_capsLock)
					{
						m_shift = true;
						m_capsLock = true;
					}
					else
					{
						m_shift = false;
						m_capsLock = false;
					}
				}	
			} break;

		case VK_NUMLOCK:
			{
				if (!keys[i].down)
				{ 
					if (!m_numLock)
					{
						m_numLock = true;
					}
					else
					{
						m_numLock = false;
					}
				}	
			} break;

		case VK_MENU:
			{
				if (keys[i].down)	{ m_alt = true; }
				else				{ m_alt = false;}	
			}break;
		}

		pKeys->push_back(keys[i]);
	}
}

// unsigned char cil::CILRawInput::TransformToCharGerman( cil::CIL_KEY key )
// {
// 	switch( key )
// 	{
// 		// 1
// 	case KEY_1:
// 		{
// 			if (m_shift)	{ return '!'; }
// 			else			{ return '1'; }
// 		}
// 
// 		// 2
// 	case KEY_2:
// 		{
// 			if (m_shift)	{ return '"'; }
// 			else			{ return '2'; }
// 		} 
// 
// 		// 3
// 	case KEY_3:
// 		{
// 			if (m_shift)	{ return '§'; }
// 			else			{ return '3'; }
// 		} 
// 
// 		// 4
// 	case KEY_4:
// 		{
// 			if (m_shift)	{ return '$'; }
// 			else			{ return '4'; }
// 		} 
// 
// 		// 5
// 	case KEY_5:
// 		{
// 			if (m_shift)	{ return '%'; }
// 			else			{ return '5'; }
// 		} 
// 
// 		// 6
// 	case KEY_6:
// 		{
// 			if (m_shift)	{ return '&'; }
// 			else			{ return '6'; }
// 		} 
// 
// 		// 7
// 	case KEY_7:
// 		{
// 			if (m_alt)		{ return '{'; }
// 			if (m_shift)	{ return '/'; }
// 			else			{ return '7'; }
// 		} 
// 
// 		// 8
// 	case KEY_8:
// 		{
// 			if (m_alt)		{ return '['; }
// 			if (m_shift)	{ return '('; }
// 			else			{ return '8'; }
// 		} 
// 
// 		// 9
// 	case KEY_9:
// 		{
// 			if (m_alt)		{ return ']'; }
// 			if (m_shift)	{ return ')'; }
// 			else			{ return '9'; }
// 		} 
// 
// 		// 0
// 	case KEY_0: 
// 		{
// 			if (m_alt)		{ return '}'; }
// 			if (m_shift)	{ return '='; }
// 			else			{ return '0'; }
// 		}
// 
// 	case KEY_Q:
// 		{
// 			if (m_alt)		{ return '@'; }
// 			if (m_shift)	{ return toupper(key); } 
// 			else			{ return tolower(key); }
// 		} break;
// 
// 	case KEY_E:
// 		{
// 			if (m_alt)		{ return '€'; }
// 			if (m_shift)	{ return toupper(key); } 
// 			else			{ return tolower(key); }	
// 		} break;
// 
// 	case KEY_PLUS:		
// 		{ 
// 			if (m_alt)		{ return '~'; }
// 			if (m_shift)	{ return '*'; }
// 			else			{ return '+'; }
// 		}
// 	case KEY_MINUS:
// 		{ 
// 			if (m_shift)	{ return '_'; }
// 			else			{ return '-'; }
// 		}
// 	case KEY_COMMA:
// 		{ 
// 			if (m_shift)	{ return ';'; }
// 			else			{ return ','; }
// 		}
// 	case KEY_DOT:
// 		{ 
// 			if (m_shift)	{ return ':'; }
// 			else			{ return '.'; }
// 		}
// 	case KEY_SPECIAL_1:		
// 		{
// 			if (m_shift)	{ return 'Ü'; }
// 			else			{ return 'ü'; }
// 		}
// 	case KEY_SPECIAL_2:		
// 		{
// 			if (m_shift)	{ return 'Ö'; }
// 			else			{ return 'ö'; }
// 		}
// 	case KEY_SPECIAL_3:		
// 		{
// 			if (m_shift)	{ return 'Ä'; }
// 			else			{ return 'ä'; }
// 		}
// 	case KEY_SPECIAL_4:		
// 		{
// 			if (m_shift)	{ return '´'; }
// 			else			{ return '`'; }
// 		}
// 	case KEY_SPECIAL_5:		
// 		{
// 			if (m_alt)		{ return '\\'; }
// 			if (m_shift)	{ return '?'; }
// 			else			{ return 'ß'; }
// 		}
// 	case KEY_SPECIAL_6:		
// 		{
// 			if (m_shift)	{ return '°'; }
// 			else			{ return '^'; }
// 		}
// 	case KEY_SPECIAL_7:
// 		{ 
// 			if (m_alt)		{ return '|'; }
// 			if (m_shift)	{ return '>'; }
// 			else			{ return '<'; }
// 		}
// 	case KEY_SPECIAL_8:
// 		{
// 			if (m_shift)	{ return '\''; }
// 			else			{ return '#'; }
// 		}
// 
// 	default:
// 		return 0;
// 	}
// }
// unsigned char cil::CILRawInput::TransformToCharUS( cil::CIL_KEY key )
// {
// 	switch( key )
// 	{
// 		// 1
// 	case KEY_1:
// 		{
// 			if (m_shift)	{ return '!'; }
// 			else			{ return '1'; }
// 		}
// 
// 		// 2
// 	case KEY_2:
// 		{
// 			if (m_shift)	{ return '@'; }
// 			else			{ return '2'; }
// 		} 
// 
// 		// 3
// 	case KEY_3:
// 		{
// 			if (m_shift)	{ return '#'; }
// 			else			{ return '3'; }
// 		} 
// 
// 		// 4
// 	case KEY_4:
// 		{
// 			if (m_shift)	{ return '$'; }
// 			else			{ return '4'; }
// 		} 
// 
// 		// 5
// 	case KEY_5:
// 		{
// 			if (m_shift)	{ return '%'; }
// 			else			{ return '5'; }
// 		} 
// 
// 		// 6
// 	case KEY_6:
// 		{
// 			if (m_shift)	{ return '^'; }
// 			else			{ return '6'; }
// 		} 
// 
// 		// 7
// 	case KEY_7:
// 		{
// 			if (m_shift)	{ return '&'; }
// 			else			{ return '7'; }
// 		} 
// 
// 		// 8
// 	case KEY_8:
// 		{
// 			if (m_shift)	{ return '*'; }
// 			else			{ return '8'; }
// 		} 
// 
// 		// 9
// 	case KEY_9:
// 		{
// 			if (m_shift)	{ return '('; }
// 			else			{ return '9'; }
// 		} 
// 
// 		// 0
// 	case KEY_0: 
// 		{
// 			if (m_shift)	{ return ')'; }
// 			else			{ return '0'; }
// 		}
// 
// 	case KEY_NUMPAD_0:		{ return '0'; }
// 	case KEY_NUMPAD_1:		{ return '1'; }
// 	case KEY_NUMPAD_2:		{ return '2'; }
// 	case KEY_NUMPAD_3:		{ return '3'; }
// 	case KEY_NUMPAD_4:		{ return '4'; }
// 	case KEY_NUMPAD_5:		{ return '5'; }
// 	case KEY_NUMPAD_6:		{ return '6'; }
// 	case KEY_NUMPAD_7:		{ return '7'; }
// 	case KEY_NUMPAD_8:		{ return '8'; }
// 	case KEY_NUMPAD_9:		{ return '9'; }
// 
// 	case KEY_Q:
// 	case KEY_E:
// 	case KEY_A:
// 	case KEY_B:
// 	case KEY_C:
// 	case KEY_D:
// 	case KEY_F:
// 	case KEY_G:
// 	case KEY_H:
// 	case KEY_I:
// 	case KEY_J:
// 	case KEY_K:
// 	case KEY_L:
// 	case KEY_M:
// 	case KEY_N:
// 	case KEY_O:
// 	case KEY_P:
// 	case KEY_R:
// 	case KEY_S:
// 	case KEY_T:
// 	case KEY_U:
// 	case KEY_V:
// 	case KEY_W:
// 	case KEY_X:
// 	case KEY_Y:
// 	case KEY_Z:
// 		{
// 			if (m_shift)	{ return toupper(key); }
// 			else			{ return tolower(key); } 
// 		}	
// 	case KEY_PLUS:		
// 		{ 
// 			if (m_shift)	{ return '='; }
// 			else			{ return '+'; }
// 		}
// 	case KEY_MINUS:
// 		{ 
// 			if (m_shift)	{ return '_'; }
// 			else			{ return '-'; }
// 		}
// 	case KEY_COMMA:
// 		{ 
// 			if (m_shift)	{ return '<'; }
// 			else			{ return ','; }
// 		}
// 	case KEY_DOT:
// 		{ 
// 			if (m_shift)	{ return '>'; }
// 			else			{ return '.'; }
// 		}
// 	case KEY_SPACE:			{ return ' '; }
// 	case KEY_ENTER:			{ return '\n'; }
// 	case KEY_MULTIPLY:		{ return '*'; }
// 	case KEY_DIVIDE:		{ return '/'; }
// 	case KEY_SPECIAL_1:		
// 		{
// 			if (m_shift)	{ return ':'; }
// 			else			{ return ';'; }
// 		}
// 	case KEY_SPECIAL_2:		{ return '~'; }
// 	case KEY_SPECIAL_3:		
// 		{
// 			if (m_shift)	{ return '"'; }
// 			else			{ return '/'; }
// 		}
// 	case KEY_SPECIAL_4:		
// 		{
// 			if (m_shift)	{ return '}'; }
// 			else			{ return ']'; }
// 		}
// 	case KEY_SPECIAL_5:		
// 		{
// 			if (m_shift)	{ return '{'; }
// 			else			{ return '['; }
// 		}
// 	case KEY_SPECIAL_6:		
// 		{
// 			if (m_shift)	{ return '|'; }
// 			else			{ return '\\'; }
// 		}
// 		// 	case KEY_SPECIAL_7:
// 		// 		{ 
// 		// 			if (m_alt)		{ return '|'; }
// 		// 			if (m_shift)	{ return '>'; }
// 		// 			else			{ return '<'; }
// 		// 		}
// 	case KEY_SPECIAL_8:
// 		{
// 			if (m_shift)	{ return '?'; }
// 			else			{ return '/'; }
// 		}
// 
// 
// 	default:
// 		return 0;
// 	}
// }
// cil::CIL_KEY  cil::CILRawInput::TransformKey( USHORT key )
// {
// 	// Chars and nums
// 	if ( key >= 0x30 && key <= 0x5A || 
// 		key >= 0x60 && key <= 0x69 )
// 	{
// 		return (CIL_KEY)key;
// 	}
// 
// 	// other (controls)
// 	switch ( key  )
// 	{
// 	case VK_UP:			{ return KEY_UP;		}
// 	case VK_DOWN:		{ return KEY_DOWN;		}
// 	case VK_RIGHT:		{ return KEY_RIGHT;		}
// 	case VK_LEFT:		{ return KEY_LEFT;		}
// 	case VK_SHIFT:		{ return KEY_SHIFT;		}
// 	case VK_PRIOR:		{ return KEY_PAGE_DOWN;	}
// 	case VK_NEXT:		{ return KEY_PAGE_UP;	}
// 	case VK_CAPITAL:	{ return KEY_CAPSLOCK;	}
// 	case VK_PAUSE:		{ return KEY_PAUSE;		}
// 	case VK_MENU:		{ return KEY_ALT;		}
// 	case VK_CONTROL:	{ return KEY_CTRL;		}
// 	case VK_SPACE:		{ return KEY_SPACE;		}
// 	case VK_DELETE:		{ return KEY_DELETE;	}
// 	case VK_TAB:		{ return KEY_TABULATOR; }
// 	case VK_ESCAPE:		{ return KEY_ESCAPE;	}
// 	case VK_RETURN:		{ return KEY_ENTER;		}
// 	case VK_INSERT:		{ return KEY_INSERT;	}
// 	case VK_BACK:		{ return KEY_BACK;		}
// 	case VK_NUMLOCK:	{ return KEY_NUMLOCK;	}
// 	case VK_F1:			{ return KEY_F1;		}
// 	case VK_F2:			{ return KEY_F2;		}
// 	case VK_F3:			{ return KEY_F3;		}
// 	case VK_F4:			{ return KEY_F4;		}
// 	case VK_F5:			{ return KEY_F5;		}
// 	case VK_F6:			{ return KEY_F6;		}
// 	case VK_F7:			{ return KEY_F7;		}
// 	case VK_F8:			{ return KEY_F8;		}
// 	case VK_F9:			{ return KEY_F9;		}
// 	case VK_F10:		{ return KEY_F10;		}
// 	case VK_F11:		{ return KEY_F11;		}
// 	case VK_F12:		{ return KEY_F12;		}
// 	case VK_PRINT:
// 	case VK_SNAPSHOT:	{ return KEY_PRINT;		}
// 	case VK_OEM_PLUS:	{ return KEY_PLUS;		}
// 	case VK_OEM_MINUS:	{ return KEY_MINUS;		}
// 	case VK_OEM_COMMA:	{ return KEY_COMMA;		}
// 	case VK_OEM_PERIOD: { return KEY_DOT;		}
// 	case VK_DIVIDE:		{ return KEY_DIVIDE;	}
// 	case VK_MULTIPLY:	{ return KEY_MULTIPLY;	}
// 
// 	case 0xBA:			{ return KEY_SPECIAL_1;	}
// 	case 0xC0:			{ return KEY_SPECIAL_2; }
// 	case 0xDE:			{ return KEY_SPECIAL_3; }
// 	case 0xDD:			{ return KEY_SPECIAL_4;	}
// 	case 0xDB:			{ return KEY_SPECIAL_5; }
// 	case 0xDC:			{ return KEY_SPECIAL_6; }
// 	case 0xE2:			{ return KEY_SPECIAL_7;	}
// 	case 0xBF:			{ return KEY_SPECIAL_8; }
// 
// 	default:
// 		return KEY_NONE;
// 	}
// }
// 
// char cil::CILRawInput::GetCharFromKey( Key key, CIL_SUPPORTED_LOCALES locale )
// {
// 	
// 	switch( key )
// 	{
// 	case KEY_1:
// 		{
// 			switch (locale)
// 			{
// 			case 
// 			}
// 		}
// 
// 
// 	// keys that are the same for all locales
// 	case KEY_SPACE:			{ return ' '; }
// 	case KEY_ENTER:			{ return '\n';}
// 	case KEY_MULTIPLY:		{ return '*'; }
// 	case KEY_DIVIDE:		{ return '/'; }
// 	case KEY_NUMPAD_0:		{ return '0'; }
// 	case KEY_NUMPAD_1:		{ return '1'; }
// 	case KEY_NUMPAD_2:		{ return '2'; }
// 	case KEY_NUMPAD_3:		{ return '3'; }
// 	case KEY_NUMPAD_4:		{ return '4'; }
// 	case KEY_NUMPAD_5:		{ return '5'; }
// 	case KEY_NUMPAD_6:		{ return '6'; }
// 	case KEY_NUMPAD_7:		{ return '7'; }
// 	case KEY_NUMPAD_8:		{ return '8'; }
// 	case KEY_NUMPAD_9:		{ return '9'; }
// 	case KEY_A:
// 	case KEY_B:
// 	case KEY_C:
// 	case KEY_D:
// 	case KEY_F:
// 	case KEY_G:
// 	case KEY_H:
// 	case KEY_I:
// 	case KEY_J:
// 	case KEY_K:
// 	case KEY_L:
// 	case KEY_M:
// 	case KEY_N:
// 	case KEY_O:
// 	case KEY_P:
// 	case KEY_R:
// 	case KEY_S:
// 	case KEY_T:
// 	case KEY_U:
// 	case KEY_V:
// 	case KEY_W:
// 	case KEY_X:
// 	case KEY_Y:
// 	case KEY_Z:
// 		{
// 			if (m_shift)	{ return toupper(key); }
// 			else			{ return tolower(key); } 
// 		}	
// 	}
// 
// 
// 
// }

