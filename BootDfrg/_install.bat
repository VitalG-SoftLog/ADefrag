@echo off
copy DfrgBoot.exe %systemroot%\system32\.
regini _addboot.ini
echo Native Example Installed
