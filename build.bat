@echo off

echo Building server...
cl /EHsc /std:c++17 iocp_tcp_server.cpp server_main.cpp /link ws2_32.lib /out:server.exe
if %errorlevel%==0 (
    echo Server build successful.
) else (
    echo Server build failed.
    goto end
)

echo Building client...
cl /EHsc /std:c++17 tcp_client.cpp client_main.cpp /link ws2_32.lib /out:client.exe
if %errorlevel%==0 (
    echo Client build successful.
) else (
    echo Client build failed.
    goto end
)

echo All builds successful.

:end