// FUN_12fd_007d @ 12fd:007d

void __cdecl16far FUN_12fd_007d(byte *param_1,undefined2 param_2,undefined1 param_3)

{
  undefined1 extraout_AH;
  int iVar1;
  byte in_CL;
  undefined2 unaff_ES;
  
  FUN_11f4_1098();
  iVar1 = CONCAT11(extraout_AH,param_3) << (in_CL & 0x1f);
  *param_1 = *param_1 & ~(byte)((uint)iVar1 >> 8);
  *param_1 = *param_1 | (byte)iVar1;
  return;
}


