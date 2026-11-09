// FUN_12fd_009c @ 12fd:009c

undefined2 __cdecl16far FUN_12fd_009c(undefined1 *param_1,undefined2 param_2,undefined1 param_3)

{
  char extraout_AH;
  byte in_CL;
  undefined2 unaff_ES;
  
  FUN_12fd_002c();
  out(0x3ce,CONCAT11(extraout_AH << (in_CL & 0x1f),8));
  out(0x3ce,0x205);
  out(0x3ce,3);
  out(0x3ce,1);
  *param_1 = param_3;
  out(0x3ce,5);
  return 5;
}


