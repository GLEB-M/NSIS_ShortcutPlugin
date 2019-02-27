# NSIS-Shortcut-Plugin
Create shortcuts with advanced options

**Examples:**

Similar to the built-in function
```nsis
Shortcut::CreateShortcut "$DESKTOP\test.lnk" "C:\Program Files (x86)\test\test.exe"
```
Do not create shortcut if already exists for test.exe
```nsis
Shortcut::CreateShortcut "$DESKTOP\test.lnk" "C:\Program Files (x86)\test\test.exe" 1
``` 
Do not create if already exists for test.exe and if name test.lnk is already exist and used for another object a new name will be generated (test(1).lnk, test(2).lnk, etc...)
```nsis
Shortcut::CreateShortcut "$DESKTOP\test.lnk" "C:\Program Files (x86)\test\test.exe" 1 1
```
