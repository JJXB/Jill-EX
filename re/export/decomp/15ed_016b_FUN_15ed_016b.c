// FUN_15ed_016b @ 15ed:016b

undefined2 __cdecl16near FUN_15ed_016b(void)

{
  undefined2 in_AX;
  undefined1 uVar1;
  undefined1 in_CL;
  undefined1 in_CH;
  undefined1 in_DL;
  undefined1 in_DH;
  
  uVar1 = (undefined1)((uint)in_AX >> 8);
  out(10,5);
  out(0xc,0);
  out(0xb,in_DH);
  out(2,(char)in_AX);
  out(2,uVar1);
  out(3,in_CL);
  out(3,in_CH);
  out(0x83,in_DL);
  out(10,1);
  return CONCAT11(uVar1,1);
}


