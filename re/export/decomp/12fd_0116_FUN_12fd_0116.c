// FUN_12fd_0116 @ 12fd:0116

void __cdecl16far FUN_12fd_0116(undefined1 *param_1)

{
  char extraout_AH;
  byte in_CL;
  undefined2 unaff_ES;
  
  FUN_12fd_004f();
  out(0x3c4,CONCAT11(extraout_AH << (in_CL & 0x1f),2));
  out(0x3ce,CONCAT11(in_CL,4));
  out(0x3ce,0x4005);
  out(0x3ce,1);
  out(0x3ce,0xff08);
  DAT_2cff_3a46 = *param_1;
  return;
}


