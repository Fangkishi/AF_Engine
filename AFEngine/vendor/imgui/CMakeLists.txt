#递归将本文件夹下所有cpp放到WRAPPER变量中
file(GLOB_RECURSE IMGUI ./ *.cpp)

#将WRAPPER中所有cpp编译为wrapper这个lib库
add_library(imgui ${IMGUI})