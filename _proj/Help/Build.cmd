"C:\Program Files\HTML Help Workshop\hhc" ..\..\_src\Help\Approach.hhp

move /Y ..\..\_src\Help\Approach.chm .\Approach.chm

SET %ERRORLEVEL% = 0

if %1 NEQ 1 goto return

echo Performing copying...
copy .\Approach.chm ..\..\Setup\Common32\


:return