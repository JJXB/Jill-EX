// FUN_12fd_00e7 @ 12fd:00e7

void __cdecl16far FUN_12fd_00e7(undefined1 *param_1,undefined2 param_2,undefined1 param_3)

{
  char extraout_AH;
  byte in_CL;
  undefined2 unaff_ES;
  
  FUN_12fd_004f();
  out(0x3c4,CONCAT11(extraout_AH << (in_CL & 0x1f),2));
  out(0x3ce,0x4005);
  out(0x3ce,1);
  out(0x3ce,0xff08);
  *param_1 = param_3;
  return;
}


