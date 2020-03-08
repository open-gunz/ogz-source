@echo off
SET fxc="C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Utilities\bin\x64\fxc.exe"
SET options=/O3

call:CompileVSPS Shadow
call:CompileVSPS MergeShadowMaps
call:CompileSingle Deferred Deferred vs_main /Tvs_3_0 ps_main /Tps_3_0
call:CompileSingle Lighting PointLight vs_point_light /Tvs_3_0 ps_point_light /Tps_3_0
call:CompileSingle Lighting Ambient vs_ambient /Tvs_3_0 ps_ambient /Tps_3_0
call:CompileSingle DepthCopy DepthCopy vs_main /Tvs_3_0 ps_main /Tps_3_0
call:CompilePS VisualizeLinearDepth
call:CompilePS Monochrome
call:CompilePS ColorInvert
call:CompileShader skin /Tvs_1_1
goto:eof

:CompileVSPS
call:CompileVS %~1VS
call:CompilePS %~1PS
goto:eof

:CompileVS
call:CompileShader %~1 /Tvs_3_0
goto:eof

:CompilePS
call:CompileShader %~1 /Tps_3_0
goto:eof

:CompileSingle
call:CompileShaderOutput %~1 %~2VS /E%~3 %~4
call:CompileShaderOutput %~1 %~2PS /E%~5 %~6
goto:eof

:CompileShader
call:CompileShaderOutput %~1 %~1 %~2 %~3 %~4
goto:eof

:CompileShaderOutput
%fxc% %~3 %~4 %~5 /FhInclude/%~2.h /Vn%~2Data Source/%~1.hlsl %options%
goto:eof