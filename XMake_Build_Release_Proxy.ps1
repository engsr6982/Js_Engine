$Env:http_proxy="http://127.0.0.1:7890";$Env:https_proxy="http://127.0.0.1:7890"

xmake repo -u

xmake f -c -y

xmake -y

pause