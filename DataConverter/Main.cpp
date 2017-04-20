#include <windows.h>
#include <stdio.h>
#include "Plugin.h"

/************************************************************************/
/* Basic Defines                                                        */
/************************************************************************/
#define PLUGIN_NAME		"Data Converter"
#define PLUGIN_AUTHOR	"Kido"
#define PLUGIN_VER		"0.02"
#define PLUGIN_CONTACT	"kidoplay@qq.com"

#define MENU_MAIN		"0 &About"
#define MENU_DISASM		"&Data Converter{0 &Asm|1 C#/JAVA|2 &CPP|3 &Delphi|4 &E Language|5 &Lua |6 Ascii String|7 Unicode String|8 About}"
/************************************************************************/
/* Basic Variables                                                      */
/************************************************************************/
HANDLE		hDll;
HWND		hWmain;

/************************************************************************/
/* Utilities                                                            */
/************************************************************************/
void set_clipboard_string(char *data)
{
	HGLOBAL hMem;
	LPSTR lPstr;

	OpenClipboard(NULL);
	EmptyClipboard();

	hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(data) + 1);
	lPstr = (LPSTR)GlobalLock(hMem);
	memcpy(lPstr, data, strlen(data) + 1);
	GlobalUnlock(hMem);
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
}
/************************************************************************/
/* Plugin Callbacks                                                     */
/************************************************************************/
extc void _export cdecl ODBG_Pluginaction(int origin, int action, void *item)
{
	char szAbout[255] = { 0 };
	t_dump		*tDump = (t_dump *)item;
	t_memory	*tMemory = (t_memory *)item;

	ulong uAddr = 0;
	ulong uSize = 0;

	char *bConverted = NULL;
	char *szConverted = NULL;
	char *szTemp = NULL;

	char *fmtHead;
	char *fmtBody;
	char *fmtFoot;

	int bString = false;
	int bRet = false;

	switch (origin)
	{
	case PM_MAIN:
		switch (action)
		{
		case 0:
			sprintf_s(szAbout, ""
				"Plugin :%s\n"
				"Version:%s\n"
				"Author :%s\n"
				"Contact:%s",
				PLUGIN_NAME,
				PLUGIN_VER,
				PLUGIN_AUTHOR,
				PLUGIN_CONTACT);
			MessageBox(hWmain, szAbout, "About", MB_OK);
			break;
		default:
			break;
		}
		break;
	case PM_DISASM:
	case PM_DUMP:
	case PM_CPUDUMP:
		uAddr = tDump->sel0;
		uSize = tDump->sel1 - uAddr;

		bConverted = (char *)malloc(uSize);
		memset(bConverted, 0, uSize);
		szConverted = (char *)malloc(uSize * 30);
		memset(szConverted, 0, uSize * 30);

		Readmemory(bConverted, uAddr, uSize, MM_RESTORE);


		switch (action)
		{
		case 0://Asm
			fmtHead = "data\r\n";
			fmtBody = "db %02Xh, ";
			fmtFoot = "db %02Xh";
			bRet = true;
			break;
		case 1://C#/Java
			fmtHead = "BYTE[] data = {\r\n";
			fmtBody = "0x%02X, ";
			fmtFoot = "0x%02X};";
			bRet = true;
			break;
		case 2://CPP
			fmtHead = new char[255];
			sprintf_s(fmtHead, 255, "BYTE data[%d] = {", uSize);
			fmtBody = "0x%02X, ";
			fmtFoot = "0x%02X};";
			bRet = true;
			break;
		case 3://Delphi
			fmtHead = new char[255];
			sprintf_s(fmtHead, 255, "data: Array[1..%d] Of Byte = (\r\n", uSize + 1);
			fmtBody = "$%02X, ";
			fmtFoot = "$%02X);";
			bRet = true;
			break;
		case 4://E Language
			fmtHead = "{";
			fmtBody = "%d, ";
			fmtFoot = "%d}";
			break;
		case 5://Lua
			fmtHead = "data = {\r\n";
			fmtBody = "0x%02X, ";
			fmtFoot = "0x%02X}";
			bRet = true;
			break;
		case 6:
			bString = true;
			uSize = Decodeascii(uAddr, szConverted, uSize, DASC_ASCII);
			break;
		case 7:
			bString = true;
			uSize = Decodeunicode(uAddr, szConverted, uSize);
			break;
		case 8:
			sprintf_s(szAbout, ""
				"Plugin :%s\n"
				"Version:%s\n"
				"Author :%s\n"
				"Contact:%s",
				PLUGIN_NAME,
				PLUGIN_VER,
				PLUGIN_AUTHOR,
				PLUGIN_CONTACT);
			MessageBox(hWmain, szAbout, "About", MB_OK);
			break;
		default:
			break;
		}

		if (!bString)
		{
			szTemp = (char *)malloc(uSize * 30);
			memset(szTemp, 0, uSize * 30);
			__asm pusha
			__asm popa
			sprintf_s(szConverted, uSize * 30, fmtHead);
			for (ulong i = 0; i < uSize - 1; i++)
			{
				sprintf_s(szTemp, uSize * 30, fmtBody, bConverted[i]);
				strcat_s(szConverted, uSize * 30, szTemp);
				if (!((i + 1) % 12) && bRet)
				{
					sprintf_s(szTemp, uSize * 30, "\r\n");
					strcat_s(szConverted, uSize * 30, szTemp);
				}
			}
			sprintf_s(szTemp, uSize * 30, fmtFoot, bConverted[uSize - 1]);
			strcat_s(szConverted, uSize * 30, szTemp);
		}

		set_clipboard_string(szConverted);
		Flash("Data Convert Success! Total %d bytes.", uSize);
		break;
	default:
		break;
	};
};

extc int _export cdecl ODBG_Pluginmenu(int origin, char data[4096], void *item)
{

	switch (origin)
	{
	case PM_MAIN:
		strcpy_s(data, sizeof(MENU_MAIN), MENU_MAIN);
		break;
	case PM_DISASM:
	case PM_DUMP:
	case PM_CPUDUMP:
		strcpy_s(data, sizeof(MENU_DISASM), MENU_DISASM);
		break;
	default: 
		return 0;
	};
	return 1;
};

extc int  _export cdecl ODBG_Plugindata(char shortname[32])
{
	strcpy_s(shortname, sizeof(PLUGIN_NAME), PLUGIN_NAME);
	return PLUGIN_VERSION;
};

extc int  _export cdecl ODBG_Plugininit(int ollydbgversion, HWND hw, ulong *features)
{
	if (ollydbgversion != PLUGIN_VERSION)
	{
		return -1;
	}
	hWmain = hw;
	Addtolist(0, 0, "%s plugin v%s", PLUGIN_NAME, PLUGIN_VER);
	Addtolist(0, -1, "  Copyright (C) 2008-2016 %s", PLUGIN_AUTHOR);
	return 0;
};

/************************************************************************/
/*DLL                                                                   */
/************************************************************************/
int WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{

	if (dwReason == DLL_PROCESS_ATTACH)
		hDll = hDllHandle;
	return 1;
}

