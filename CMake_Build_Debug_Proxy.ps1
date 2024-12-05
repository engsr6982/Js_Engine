$Env:http_proxy="http://127.0.0.1:7890";$Env:https_proxy="http://127.0.0.1:7890"

cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug

cmake --build build

pause