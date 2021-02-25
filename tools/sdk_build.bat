xcopy %1 %2 /e /s /f /i /y /q /exclude:%1\tools\xcopy_exclude.txt
echo %3 > %2\revision.txt
python %1\tools\zip_folder.py %2
rmdir /s /q %2
