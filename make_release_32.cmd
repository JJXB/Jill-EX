@echo off
rem Kept for compatibility: the x86 release package is now produced by
rem build32.cmd itself (staged in build32\Release32\Jill-EX and zipped to
rem Releases\Jill-EX-release-x86.zip).  This simply runs that build.
call "%~dp0build32.cmd" %*
exit /b %errorlevel%
