# Microsoft Developer Studio Project File - Name="bs2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bs2 - Win32 mad mp3 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bs2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bs2.mak" CFG="bs2 - Win32 mad mp3 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bs2 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bs2 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bs2 - Win32 mad mp3 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bs2 - Win32 mad mp3 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bs2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bs2___Release"
# PROP Intermediate_Dir "bs2___Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /I "." /I "common" /I "..\..\\" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bs2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bs2___Win32_Debug"
# PROP BASE Intermediate_Dir "bs2___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bs2___Debug"
# PROP Intermediate_Dir "bs2___Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GR /GX /ZI /Od /I "." /I "common" /I "..\..\\" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bs2 - Win32 mad mp3 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bs2___Win32_mad mp3_Debug"
# PROP BASE Intermediate_Dir "bs2___Win32_mad mp3_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bs2___mad_mp3_Debug"
# PROP Intermediate_Dir "bs2___mad_mp3_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /vmg /GX /ZI /Od /I "." /I "common" /I "..\..\\" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /D "USE_MAD" /D "MSVC6_COMPAT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bs2 - Win32 mad mp3 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "bs2___Win32_mad mp3_Release"
# PROP BASE Intermediate_Dir "bs2___Win32_mad mp3_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bs2___mad_mp3_Release"
# PROP Intermediate_Dir "bs2___mad_mp3_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /I "..\..\\" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /D "USE_MAD" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "bs2 - Win32 Release"
# Name "bs2 - Win32 Debug"
# Name "bs2 - Win32 mad mp3 Debug"
# Name "bs2 - Win32 mad mp3 Release"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "driver"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\sword2\driver\_mouse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\_mouse.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\animation.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\animation.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\d_draw.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\d_draw.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\d_sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\d_sound.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\ddutil.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\driver96.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\menu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\menu.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\mouse.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\palette.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\palette.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\rdwin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\rdwin.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\render.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\render.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\driver\sprite.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\sword2\anims.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\anims.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\build_display.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\build_display.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\console.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\console.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\controls.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\controls.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\credits.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\defs.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\events.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\events.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\function.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\function.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\header.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\icons.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\icons.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\interpreter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\interpreter.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\layers.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\layers.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\logic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\logic.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\maketext.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\maketext.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\memory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\memory.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\mouse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\mouse.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\object.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\protocol.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\protocol.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\resman.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\resman.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\router.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\router.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\save_rest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\save_rest.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\scroll.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\scroll.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\speech.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\speech.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\startup.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\startup.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\sword2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\sword2.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\sync.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\sync.h
# End Source File
# Begin Source File

SOURCE=..\..\sword2\walker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword2\walker.h
# End Source File
# End Group
# End Target
# End Project
