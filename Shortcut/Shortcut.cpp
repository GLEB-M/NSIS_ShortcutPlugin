#include <windows.h>
#include <shlobj.h>

#ifdef USE_NSISLIB

#include <pluginapi.h>

#else

#define NSISCALL __stdcall

enum
{
INST_0,         // $0
INST_1,         // $1
INST_2,         // $2
INST_3,         // $3
INST_4,         // $4
INST_5,         // $5
INST_6,         // $6
INST_7,         // $7
INST_8,         // $8
INST_9,         // $9
INST_R0,        // $R0
INST_R1,        // $R1
INST_R2,        // $R2
INST_R3,        // $R3
INST_R4,        // $R4
INST_R5,        // $R5
INST_R6,        // $R6
INST_R7,        // $R7
INST_R8,        // $R8
INST_R9,        // $R9
INST_CMDLINE,   // $CMDLINE
INST_INSTDIR,   // $INSTDIR
INST_OUTDIR,    // $OUTDIR
INST_EXEDIR,    // $EXEDIR
INST_LANG,      // $LANGUAGE
__INST_LAST
};

typedef struct _stack_t {
  struct _stack_t *next;
  char text[1]; // this should be the length of string_size
} stack_t;

#define EXDLL_INIT()           {  \
        g_stringsize=string_size; \
        g_stacktop=stacktop;      \
        g_variables=variables; }

int       g_stringsize;
char     *g_variables;
stack_t **g_stacktop;

int NSISCALL popstring(char *str)
{
  stack_t *th;
  if (!g_stacktop || !*g_stacktop) return 1;
  th=(*g_stacktop);
  if (str) lstrcpyA(str,th->text);
  *g_stacktop = th->next;
  GlobalFree((HGLOBAL)th);
  return 0;
}

int NSISCALL popstringn(char *str, int maxlen)
{
  stack_t *th;
  if (!g_stacktop || !*g_stacktop) return 1;
  th=(*g_stacktop);
  if (str) lstrcpynA(str,th->text,maxlen?maxlen:g_stringsize);
  *g_stacktop = th->next;
  GlobalFree((HGLOBAL)th);
  return 0;
}

void NSISCALL pushstring(const char *str)
{
  stack_t *th;
  if (!g_stacktop) return;
  th=(stack_t*)GlobalAlloc(GPTR,sizeof(stack_t)+g_stringsize);
  lstrcpynA(th->text,str,g_stringsize);
  th->next=*g_stacktop;
  *g_stacktop=th;
}

char * NSISCALL getuservariable(const int varnum)
{
  if (varnum < 0 || varnum >= __INST_LAST) return NULL;
  return g_variables+varnum*g_stringsize;
}

void NSISCALL setuservariable(const int varnum, const char *var)
{
	if (var != NULL && varnum >= 0 && varnum < __INST_LAST) 
		lstrcpyA(g_variables + varnum*g_stringsize, var);
}

// playing with integers

int NSISCALL myatoi(const char *s)
{
  int v=0;
  if (*s == '0' && (s[1] == 'x' || s[1] == 'X'))
  {
    s++;
    for (;;)
    {
      int c=*(++s);
      if (c >= '0' && c <= '9') c-='0';
      else if (c >= 'a' && c <= 'f') c-='a'-10;
      else if (c >= 'A' && c <= 'F') c-='A'-10;
      else break;
      v<<=4;
      v+=c;
    }
  }
  else if (*s == '0' && s[1] <= '7' && s[1] >= '0')
  {
    for (;;)
    {
      int c=*(++s);
      if (c >= '0' && c <= '7') c-='0';
      else break;
      v<<=3;
      v+=c;
    }
  }
  else
  {
    int sign=0;
    if (*s == '-') sign++; else s--;
    for (;;)
    {
      int c=*(++s) - '0';
      if (c < 0 || c > 9) break;
      v*=10;
      v+=c;
    }
    if (sign) v = -v;
  }

  return v;
}

unsigned NSISCALL myatou(const char *s)
{
  unsigned int v=0;

  for (;;)
  {
    unsigned int c=*s++;
    if (c >= '0' && c <= '9') c-='0';
    else break;
    v*=10;
    v+=c;
  }
  return v;
}

int NSISCALL myatoi_or(const char *s)
{
  int v=0;
  if (*s == '0' && (s[1] == 'x' || s[1] == 'X'))
  {
    s++;
    for (;;)
    {
      int c=*(++s);
      if (c >= '0' && c <= '9') c-='0';
      else if (c >= 'a' && c <= 'f') c-='a'-10;
      else if (c >= 'A' && c <= 'F') c-='A'-10;
      else break;
      v<<=4;
      v+=c;
    }
  }
  else if (*s == '0' && s[1] <= '7' && s[1] >= '0')
  {
    for (;;)
    {
      int c=*(++s);
      if (c >= '0' && c <= '7') c-='0';
      else break;
      v<<=3;
      v+=c;
    }
  }
  else
  {
    int sign=0;
    if (*s == '-') sign++; else s--;
    for (;;)
    {
      int c=*(++s) - '0';
      if (c < 0 || c > 9) break;
      v*=10;
      v+=c;
    }
    if (sign) v = -v;
  }

  // Support for simple ORed expressions
  if (*s == '|') 
  {
      v |= myatoi_or(s+1);
  }

  return v;
}

int NSISCALL popint()
{
  char buf[128];
  if (popstringn(buf,sizeof(buf)))
    return 0;

  return myatoi(buf);
}

int NSISCALL popint_or()
{
  char buf[128];
  if (popstringn(buf,sizeof(buf)))
    return 0;

  return myatoi_or(buf);
}

void NSISCALL pushint(int value)
{
	char buffer[1024];
	wsprintf(buffer, "%d", value);
	pushstring(buffer);
}

#endif

//
// strcat
//
void tcscatn( TCHAR *dst, const TCHAR *src, size_t max_len )
{
	size_t len = lstrlen(dst);
	if ( len >= max_len )
		return;

	lstrcpyn(dst+len, src, max_len-len); 
}

//
// ExtractPath
//
void ExtractPath( const TCHAR *full_path, TCHAR *path )
{
	int len = lstrlen(full_path);
	lstrcpyn(path, full_path, len+1);
	for ( int i = len-1; i >= 0; --i )
	{
		if ( path[i] == '\\' )
		{
			path[i] = '\0';
			break;
		}
	}
}

//
// Create shortcut
//
// szLinkFileName - path to shortcut file
// szObjPath      - path to shortcut object
// szObjDir       - object working directory
// szObjDesc      - object description (opt)
//
bool CreateShortcut( const TCHAR *szLinkFileName, const TCHAR *szObjPath, const TCHAR *szObjDir, const TCHAR *szObjDesc = NULL )
{
	HRESULT      res;
	IShellLink   *sl;
	IPersistFile *pf;

	if ( S_OK == (res = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&sl)) )
	{
		sl->SetPath(szObjPath);

		sl->SetWorkingDirectory(szObjDir);

		if ( szObjDesc && *szObjDesc )
			sl->SetDescription(szObjDesc);

		if ( S_OK == (res = sl->QueryInterface(IID_IPersistFile, (void **)&pf)) )
		{
#if !defined( UNICODE )
			WCHAR wszFileName[MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, szLinkFileName, -1, wszFileName, _countof(wszFileName)); 
			pf->Save(wszFileName, TRUE);
#else
			res = pf->Save(szLinkFileName, TRUE);
#endif
			pf->Release();
		}
		sl->Release();
	}
	
	return (res == S_OK);
}

//
// Shortcut parameter
//
// szLinkFileName - path to shortcut file
// szObjPath      - path to shortcut object (out)
// szObjDir       - object working directory (out, opt)
// szObjDesc      - object description (out, opt)
//
bool GetShortcutParams( const TCHAR *szLinkFileName, TCHAR *szObjPath, TCHAR *szObjDir = NULL, TCHAR *szObjDesc = NULL )
{
	HRESULT         res;
	IShellLink      *sl;
	IPersistFile    *pf;
	WIN32_FIND_DATA fd;

	if ( !szObjPath )
		return false;

	if ( S_OK == (res = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&sl)) )
	{
		if ( S_OK == (res = sl->QueryInterface(IID_IPersistFile, (void **)&pf)) )
		{
#if !defined( UNICODE )
			WCHAR wszFileName[MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, szLinkFileName, -1, wszFileName, _countof(wszFileName)); 
			pf->Load(wszFileName, STGM_READ);
#else
			res = pf->Load(szLinkFileName, STGM_READ);
#endif
			if ( res == S_OK )
			{
				if ( S_OK == (res = sl->GetPath(szObjPath, MAX_PATH, &fd, 0)) )
				{
					if ( szObjDir )
						if ( S_OK != sl->GetWorkingDirectory(szObjDir, MAX_PATH) )
							*szObjDir = '\0';

					if ( szObjDesc )
						if ( S_OK != sl->GetDescription(szObjDesc, MAX_PATH) )
							*szObjDesc = '\0';
				}
			}
			
			pf->Release();
		}
		sl->Release();
	}
	
	return (res == S_OK);
}

//
// Enumerate files in directory (without sub-directories)
//
// szMask    - mask
// szDirPath - path to directory
// EnumProc  - enumeration procedure (enumeration ends if EnumProc return false)
// Param     - user parameter
//
bool EnumFiles( const TCHAR *szMask, const TCHAR *szDirPath, bool (*EnumProc)( const TCHAR *szFileName, void *Param ), void *Param )
{
	TCHAR           szFileName[MAX_PATH];
	HANDLE          hFile;
	WIN32_FIND_DATA fd;

	if ( !EnumProc )
		return true;

	lstrcpyn(szFileName, szDirPath, _countof(szFileName));
	tcscatn(szFileName, TEXT("\\"), _countof(szFileName));
	tcscatn(szFileName, szMask, _countof(szFileName));

	if ( INVALID_HANDLE_VALUE == (hFile = FindFirstFile(szFileName, &fd)) )
		return true;

	do
	{
		if ( *fd.cFileName == '.' ) // ".", ".."
			continue;

		TCHAR szPath[MAX_PATH];
		lstrcpyn(szPath, szDirPath, _countof(szPath));
		tcscatn(szPath, "\\", _countof(szPath));
		tcscatn(szPath, fd.cFileName, _countof(szPath));

		if ( !(*EnumProc)(szPath, Param) )
			break; // done
	}
	while ( FindNextFile(hFile, &fd) );

	FindClose(hFile);

	return true;
}

//
// Enumeration procedure
//
bool FilesEnumProc( const TCHAR *szFileName, void *Param )
{
	TCHAR szExistsLinkObjPath[MAX_PATH];

	if ( GetShortcutParams(szFileName, szExistsLinkObjPath) ) // get object path
	{
		TCHAR *szObjPath = (TCHAR *)Param;

		if ( !lstrcmpi(szObjPath, szExistsLinkObjPath) )
		{
			*szObjPath = '\0'; // shortcut for this object found, reset path as sign that you don't need to create new shortcut
			return false;      // enumeration done
		}
	}

	return true;
}

//
// Get free file name
//
// szFileName - desired file name (full path) (in, out)
//
bool GetFreeFileName( TCHAR *szFileName )
{
	HANDLE          hFile;
	WIN32_FIND_DATA fd;
	TCHAR           szNewFileName[MAX_PATH];
	TCHAR           szFileBase[MAX_PATH];
	TCHAR           szFileExt[MAX_PATH];
	TCHAR           szPostfix[2];

	if ( INVALID_HANDLE_VALUE == (hFile = FindFirstFile(szFileName, &fd)) )
		return true; // name is free, do nothing

	FindClose(hFile);

	// split name and extension
	lstrcpyn(szFileBase, szFileName, MAX_PATH);
	szFileExt[0] = '\0';
	for ( int i = lstrlen(szFileName)-1; i >= 0; --i )
	{
		if ( szFileName[i] == '.' )
		{
			lstrcpyn(szFileBase, szFileName, i + 1);
			lstrcpyn(szFileExt, szFileName + i, MAX_PATH);
			break;
		}
	}

	for ( int i = 1; i < 10; ++i )
	{
		szPostfix[0] = i + '0';
		szPostfix[1] = '\0';

		lstrcpyn(szNewFileName, szFileBase, MAX_PATH);
		tcscatn(szNewFileName, TEXT(" ("), MAX_PATH);
		tcscatn(szNewFileName, szPostfix, MAX_PATH);
		tcscatn(szNewFileName, TEXT(")"), MAX_PATH);
		tcscatn(szNewFileName, szFileExt, MAX_PATH);
		
		if ( INVALID_HANDLE_VALUE == (hFile = FindFirstFile(szNewFileName, &fd)) )
		{
			lstrcpyn(szFileName, szNewFileName, MAX_PATH);
			return true; // name is free
		}

		FindClose(hFile);
	}

	return false;
}

//
// Create shortcut
//
void CreateShortcut( )
{
	TCHAR szLinkFileName[MAX_PATH];
	TCHAR szLinkDir[MAX_PATH];
	TCHAR szCheckExist[2];
	TCHAR szSafeName[2];

	TCHAR szObjFileName[MAX_PATH];
	TCHAR szObjDir[MAX_PATH];

	bool  bCheckExist = false;
	bool  bSafeName   = false;

	if ( popstringn(szLinkFileName, _countof(szLinkFileName)) ) // Shortcut file
		return;
	
	if ( popstringn(szObjFileName, _countof(szObjFileName)) )   // Path to object
		return;

	if ( !popstringn(szCheckExist, _countof(szCheckExist)) )    // Check object existance
		bCheckExist = (*szCheckExist != '0');

	if ( !popstringn(szSafeName, _countof(szSafeName)) )        // Retain existing shortcut name when matched and generate new name
		bSafeName = (*szSafeName != '0');

	ExtractPath(szLinkFileName, szLinkDir);                     // Shortcut directory
	ExtractPath(szObjFileName, szObjDir);                       // Object directory

	if ( bSafeName && !GetFreeFileName(szLinkFileName) )
		return;

	if ( bCheckExist )
	{
		if ( EnumFiles(TEXT("*.lnk"), szLinkDir, FilesEnumProc, szObjFileName) )
			if ( *szObjFileName )
				CreateShortcut(szLinkFileName, szObjFileName, szObjDir);
	}
	else
		CreateShortcut(szLinkFileName, szObjFileName, szObjDir);
}

//
// Shortcut::CreateShortcut
//
// Usage: Shortcut::CreateShortcut "$DESKTOP\test.lnk" "C:\Program Files (x86)\test\test.exe" 1 1
//
extern "C" void __declspec(dllexport) CreateShortcut( HWND hwndParent, int string_size, char *variables, stack_t **stacktop, /*extra_parameters*/void *extra )
{
	EXDLL_INIT();
	CreateShortcut();
}

//
// DllMain
//
BOOL WINAPI DllMain( HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved )
{
	return TRUE;
}