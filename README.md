# SimplePacker

simplepacker是一个为[ejoy2d](https://github.com/ejoy/ejoy2d)引擎设计的打包工具，可以把包含PNG图片的文件夹打包成[simplepackage](https://github.com/ejoy/ejoy2d/blob/master/doc/apicn.md#simplepackage)可以读取的格式。

Build
=====
windows
* open build\msvc\VS2013.sln
* build release
* run build\msvc\test.bat

macosx
* cd build/osx
* make
* make test

Test
====
test目录下包含两个测试示例。input演示了简单的图片打包与animation自动生成；input_advance演示了自定义animation。
打包后生成test/output，配合test/test.lua可在ejoy2d引擎中运行。