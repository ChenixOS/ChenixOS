#if 0
//
// Generated by Microsoft (R) D3DX9 Shader Compiler 
//
//   fxc /T vs_2_0 /E MyVertexShader /Fh simple_vs.h simple.fx
//
//
// Parameters:
//
//   float4x4 matWorldViewProj;
//   float timestep;
//
//
// Registers:
//
//   Name             Reg   Size
//   ---------------- ----- ----
//   matWorldViewProj c0       4
//   timestep         c4       1
//

    vs_2_0
    def c5, 8, 0.159154937, 0.5, 0
    def c6, 6.28318548, -3.14159274, 10, 1
    def c7, -1.55009923e-06, -2.17013894e-05, 0.00260416674, 0.00026041668
    def c8, -0.020833334, -0.125, 1, 0.5
    dcl_position v0
    mul r0.xy, v0, v0
    add r0.x, r0.y, r0.x
    mov r1.x, c5.x
    mad r0.y, r0.x, r1.x, c4.x
    mad r0.x, r0.x, c6.z, c6.w
    mad r0.y, r0.y, c5.y, c5.z
    frc r0.y, r0.y
    mad r0.y, r0.y, c6.x, c6.y
    sincos r1.y, r0.y, c7, c8
    rcp r0.x, r0.x
    mul r0.z, r1.y, r0.x
    mov r0.xyw, v0
    dp4 oPos.x, r0, c0
    dp4 oPos.y, r0, c1
    dp4 oPos.z, r0, c2
    dp4 oPos.w, r0, c3
    mov oT0, r0

// approximately 24 instruction slots used
#endif

const DWORD g_vs20_MyVertexShader[] =
{
    0xfffe0200, 0x002dfffe, 0x42415443, 0x0000001c, 0x0000008b, 0xfffe0200, 
    0x00000002, 0x0000001c, 0x20000100, 0x00000084, 0x00000044, 0x00000002, 
    0x00000004, 0x00000058, 0x00000000, 0x00000068, 0x00040002, 0x00000001, 
    0x00000074, 0x00000000, 0x5774616d, 0x646c726f, 0x77656956, 0x6a6f7250, 
    0xababab00, 0x00030003, 0x00040004, 0x00000001, 0x00000000, 0x656d6974, 
    0x70657473, 0xababab00, 0x00030000, 0x00010001, 0x00000001, 0x00000000, 
    0x325f7376, 0x4d00305f, 0x6f726369, 0x74666f73, 0x29522820, 0x44334420, 
    0x53203958, 0x65646168, 0x6f432072, 0x6c69706d, 0x00207265, 0x05000051, 
    0xa00f0005, 0x41000000, 0x3e22f983, 0x3f000000, 0x00000000, 0x05000051, 
    0xa00f0006, 0x40c90fdb, 0xc0490fdb, 0x41200000, 0x3f800000, 0x05000051, 
    0xa00f0007, 0xb5d00d01, 0xb7b60b61, 0x3b2aaaab, 0x39888889, 0x05000051, 
    0xa00f0008, 0xbcaaaaab, 0xbe000000, 0x3f800000, 0x3f000000, 0x0200001f, 
    0x80000000, 0x900f0000, 0x03000005, 0x80030000, 0x90e40000, 0x90e40000, 
    0x03000002, 0x80010000, 0x80550000, 0x80000000, 0x02000001, 0x80010001, 
    0xa0000005, 0x04000004, 0x80020000, 0x80000000, 0x80000001, 0xa0000004, 
    0x04000004, 0x80010000, 0x80000000, 0xa0aa0006, 0xa0ff0006, 0x04000004, 
    0x80020000, 0x80550000, 0xa0550005, 0xa0aa0005, 0x02000013, 0x80020000, 
    0x80550000, 0x04000004, 0x80020000, 0x80550000, 0xa0000006, 0xa0550006, 
    0x04000025, 0x80020001, 0x80550000, 0xa0e40007, 0xa0e40008, 0x02000006, 
    0x80010000, 0x80000000, 0x03000005, 0x80040000, 0x80550001, 0x80000000, 
    0x02000001, 0x800b0000, 0x90e40000, 0x03000009, 0xc0010000, 0x80e40000, 
    0xa0e40000, 0x03000009, 0xc0020000, 0x80e40000, 0xa0e40001, 0x03000009, 
    0xc0040000, 0x80e40000, 0xa0e40002, 0x03000009, 0xc0080000, 0x80e40000, 
    0xa0e40003, 0x02000001, 0xe00f0000, 0x80e40000, 0x0000ffff
};
