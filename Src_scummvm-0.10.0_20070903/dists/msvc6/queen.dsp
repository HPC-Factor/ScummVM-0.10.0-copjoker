# Microsoft Developer Studio Project File - Name="queen" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=queen - Win32 mad mp3 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "queen.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "queen.mak" CFG="queen - Win32 mad mp3 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "queen - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "queen - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "queen - Win32 mad mp3 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "queen - Win32 mad mp3 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "queen - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "queen___Release"
# PROP Intermediate_Dir "queen___Release"
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

!ELSEIF  "$(CFG)" == "queen - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "queen___Win32_Debug"
# PROP BASE Intermediate_Dir "queen___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "queen___Debug"
# PROP Intermediate_Dir "queen___Debug"
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

!ELSEIF  "$(CFG)" == "queen - Win32 mad mp3 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "queen___Win32_mad mp3_Debug"
# PROP BASE Intermediate_Dir "queen___Win32_mad mp3_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "queen___mad_mp3_Debug"
# PROP Intermediate_Dir "queen___mad_mp3_Debug"
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

!ELSEIF  "$(CFG)" == "queen - Win32 mad mp3 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "queen___Win32_mad mp3_Release"
# PROP BASE Intermediate_Dir "queen___Win32_mad mp3_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "queen___mad_mp3_Release"
# PROP Intermediate_Dir "queen___mad_mp3_Release"
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

# Name "queen - Win32 Release"
# Name "queen - Win32 Debug"
# Name "queen - Win32 mad mp3 Debug"
# Name "queen - Win32 mad mp3 Release"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\queen\bankman.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\bankman.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\command.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\command.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\credits.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\credits.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\cutaway.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\cutaway.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\defs.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\display.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\display.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\graphics.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\graphics.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\grid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\grid.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\input.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\input.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\journal.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\journal.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\logic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\logic.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\music.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\music.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\musicdata.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\queen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\queen.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\resource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\restables.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\state.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\state.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\talk.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\talk.h
# End Source File
# Begin Source File

SOURCE=..\..\queen\walk.cpp
# End Source File
# Begin Source File

SOURCE=..\..\queen\walk.h
# End Source File
# End Group
# End Target
# End Project
