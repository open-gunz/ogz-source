; Skinned Mesh Shader v.0.5 - magicbell

; v0 - position
; v1 - weight(x,y)
; v2 - matrix index( x, y, z )
; v3 - normal

; r0 - temp
; r1 - vertex sum
; r2 - temp only for weight to cal 3rd weight
; r3 - normal sum
; r4 - color result
; r5 - final vertex pos

; 0~2 : Identity Matrix
; 3~80 : Animation
; 81 - light type
; 82 - diffuse color
; 83 - specular color
; 84 - Ambient color
; 85 - Position ( local space )
; 86 - Direction ( not right now...^^ )
; 87 - Range(x) Falloff(y) 
; 88 - Attenuation( x - constant, y - linear, z - Quadratic
; 89 - Inner Angle of Spotlight
; 90 - Outer Angle of Spotlight
; 91 - Constant( 1, -1, 0.5, -0.5)
; 92~95 : world*view*Proj

vs.1.1

; cal Blended Vertex Position 1
mov a0.x, v2.x
dp4 r0.x, v0, c[a0.x + 0]
dp4 r0.y, v0, c[a0.x + 1]
dp4 r0.z, v0, c[a0.x + 2]
mul r1.xyz, r0.xyz, v1.xxx
; cal normal transform
/*
dp3 r0.x, v3, c[a0.x + 0]
dp3 r0.y, v3, c[a0.x + 0]
dp3 r0.z, v3, c[a0.x + 0]
mul r3.xyz, r0.xyz, v1.xxx
*/
; cal Blended Vertex Position 2
mov a0.x, v2.y
dp4 r0.x, v0, c[a0.x + 0]
dp4 r0.y, v0, c[a0.x + 1]
dp4 r0.z, v0, c[a0.x + 2]
mad r1.xyz, r0.xyz, v1.yyy, r1.xyz
; cal normal transform
/*
dp3 r0.x, v3, c[a0.x + 0]
dp3 r0.y, v3, c[a0.x + 0]
dp3 r0.z, v3, c[a0.x + 0]
mad r3.xyz, r0.xyz, v1.yyy, r3.xyz
*/
; cal Blended Vertex Position 3
add r2.x, v1.x, v1.y
sub r2.x, c[0].x, r2.x
dp4 r0.x, v0, c[a0.x + 0]
dp4 r0.y, v0, c[a0.x + 1]
dp4 r0.z, v0, c[a0.x + 2]
mad r1.xyz, r0.xyz, r2.xxx, r1.xyz
; cal normal transform
/*
dp3 r0.x, v3, c[a0.x + 0]
dp3 r0.y, v3, c[a0.x + 0]
dp3 r0.z, v3, c[a0.x + 0]
mad r3.xyz, r0.xyz, v1.yyy, r3.xyz
*/
; vertex pos result
mov r1.w, c[0].x
dp4 oPos.x, r1, c[92]
dp4 oPos.y, r1, c[93]
dp4 oPos.z, r1, c[94]
dp4 oPos.w, r1, c[95]

; light
; distance from light
;sub r0, v0, c[85]
;dp3 r0.w, r0, r0 
mov oT0, v3