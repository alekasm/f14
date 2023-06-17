set missnum=00
\f14\mission\miscomp fw100a.txt /d\f14\mission\basedef /ofw100a.msn
if errorlevel 1 goto error
set missnum=01
\f14\mission\miscomp fw101a.txt /d\f14\mission\basedef /ofw101a.msn
if errorlevel 1 goto error
set missnum=02
\f14\mission\miscomp fw102a.txt /d\f14\mission\basedef /ofw102a.msn
if errorlevel 1 goto error
set missnum=03
\f14\mission\miscomp fw103a.txt /d\f14\mission\basedef /ofw103a.msn
if errorlevel 1 goto error
set missnum=04
\f14\mission\miscomp fw104a.txt /d\f14\mission\basedef /ofw104a.msn
if errorlevel 1 goto error
set missnum=05
\f14\mission\miscomp fw105a.txt /d\f14\mission\basedef /ofw105a.msn
if errorlevel 1 goto error
set missnum=06
\f14\mission\miscomp fw106a.txt /d\f14\mission\basedef /ofw106a.msn
if errorlevel 1 goto error
set missnum=07
\f14\mission\miscomp fw107a.txt /d\f14\mission\basedef /ofw107a.msn
if errorlevel 1 goto error
set missnum=08
\f14\mission\miscomp fw108a.txt /d\f14\mission\basedef /ofw108a.msn
if errorlevel 1 goto error
set missnum=09
\f14\mission\miscomp fw109a.txt /d\f14\mission\basedef /ofw109a.msn
if errorlevel 1 goto error
set missnum=10
\f14\mission\miscomp fw110a.txt /d\f14\mission\basedef /ofw110a.msn
if errorlevel 1 goto error
set missnum=11
\f14\mission\miscomp fw111a.txt /d\f14\mission\basedef /ofw111a.msn
if errorlevel 1 goto error
set missnum=12
\f14\mission\miscomp fw112a.txt /d\f14\mission\basedef /ofw112a.msn
if errorlevel 1 goto error
set missnum=13
\f14\mission\miscomp fw113a.txt /d\f14\mission\basedef /ofw113a.msn
if errorlevel 1 goto error
set missnum=14
\f14\mission\miscomp fw114a.txt /d\f14\mission\basedef /ofw114a.msn
if errorlevel 1 goto error
set missnum=15
\f14\mission\miscomp fw115a.txt /d\f14\mission\basedef /ofw115a.msn
if errorlevel 1 goto error
set missnum=16
\f14\mission\miscomp fw116a.txt /d\f14\mission\basedef /ofw116a.msn
if errorlevel 1 goto error
set missnum=17
\f14\mission\miscomp fw117a.txt /d\f14\mission\basedef /ofw117a.msn
if errorlevel 1 goto error
set missnum=18
\f14\mission\miscomp fw118a.txt /d\f14\mission\basedef /ofw118a.msn
if errorlevel 1 goto error
set missnum=19
\f14\mission\miscomp fw119a.txt /d\f14\mission\basedef /ofw119a.msn
if errorlevel 1 goto error
set missnum=20
\f14\mission\miscomp fw120a.txt /d\f14\mission\basedef /ofw120a.msn
if errorlevel 1 goto error
set missnum=21
\f14\mission\miscomp fw121a.txt /d\f14\mission\basedef /ofw121a.msn
if errorlevel 1 goto error
set missnum=22
\f14\mission\miscomp fw122a.txt /d\f14\mission\basedef /ofw122a.msn
if errorlevel 1 goto error
set missnum=23
\f14\mission\miscomp fw123a.txt /d\f14\mission\basedef /ofw123a.msn

goto end

:error

\f14\mission\miscomp fw1%missnum%a.txt /d\f14\mission\basedef /ofw1%missnum%a.msn > errors
pwb fw1%missnum%a.txt


:end
