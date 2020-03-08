    vs_1_1
    dcl_position v0
    dcl_blendweight v1
    dcl_blendindices v2
    dcl_normal v3
    dcl_texcoord v4
    mov a0.x, v2.x
    m4x3 r0.xyz, v0, c0[a0.x]
    mul r1.xyz, r0.xyzz, v1.x
    m3x3 r0.xyz, v3, c0[a0.x]
    mul r4.xyz, r0.xyzz, v1.x
    mov a0.x, v2.y
    m4x3 r0.xyz, v0, c0[a0.x]
    mad r1.xyz, r0.xyzz, v1.y, r1.xyzz
    m3x3 r0.xyz, v3, c0[a0.x]
    mad r4.xyz, r0.xyzz, v1.y, r4.xyzz
    add r2.x, v1.x, v1.y
    add r2.w, c10.x, -r2.x
    mov a0.x, v2.z
    m4x3 r0.xyz, v0, c0[a0.x]
    mad r1.xyz, r0.xyzz, r2.w, r1.xyzz
    m3x3 r0.xyz, v3, c0[a0.x]
    mad r4.xyz, r0.xyzz, r2.w, r4.xyzz
    mov r1.w, c10.x
    m4x3 r5.xyz, r1, c3
    mov r5.w, c10.x
    m3x3 r3.xyz, r4, c3
    mov r3.w, c10.x
    m4x4 oPos, r5, c6
    add r7.xyz, c17.xyzz, -r5.xyzz
    dp3 r0.w, r7.xyzz, r7.xyzz
    rsq r0.z, r0.w
    dst r2, r0.w, r0.z
    dp3 r6.x, r2, c27
    rcp r6.x, r6.x
    mul r4.xyz, r7.xyzz, r0.z
    dp3 r8.x, r3, r4
    max r8.y, r8.x, c0.w
    mul r8.y, r8.y, r6.x
    mov r9, c18
    mul r10, c19, r8.y
    add r7.xyz, c22.xyzz, -r5.xyzz
    dp3 r0.w, r7.xyzz, r7.xyzz
    rsq r0.z, r0.w
    dst r2, r0.w, r0.z
    dp3 r6.x, r2, c28
    rcp r6.x, r6.x
    mul r4.xyz, r7.xyzz, r0.z
    dp3 r8.x, r3, r4
    max r8.y, r8.x, c0.w
    mul r8.y, r8.y, r6.x
    sge r0.w, c26.x, r2.y
    add r9, r9, c23
    add r9, r9, c16
    mul r9, r9, c12
    mad r11, c24, r8.y, r10
    mul r11, r11, c13
    add oD0, r9, r11
    mov oD0.w, c13.w
    mov oT0.xy, v4
    mov oFog, c10.x
