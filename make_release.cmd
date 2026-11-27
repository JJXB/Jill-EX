@echo off
rem Kept for compatibility: the x64 release package is now produced by
rem build.cmd itself (staged in build\release\Jill-EX and zipped to
rem Releases\Jill-EX-release-x64.zip).  This simply runs that build.
call "%~dp0build.cmd" %*
exit /b %errorlevel%
