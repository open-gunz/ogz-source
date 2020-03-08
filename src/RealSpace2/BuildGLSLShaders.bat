@echo off	
call:CompileShader BasicRender.vert BasicRenderVS
call:CompileShader BasicRender.frag BasicRenderFS
call:CompileShader AlphaTesting.frag AlphaTestingFS
rm temp.spv
goto:eof

:CompileShader
glslangvalidator -V Source/%~1 -o temp.spv
"../BinaryFileToHeader/bin/BinaryFileToHeader.exe" temp.spv -oInclude/%~2.h -name%~2
goto:eof