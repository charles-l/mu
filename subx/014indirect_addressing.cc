//: operating on memory at the address provided by some register
//: we'll now start providing data in a separate segment

void test_add_r32_to_mem_at_r32() {
  Reg[EBX].i = 0x10;
  Reg[EAX].i = 0x2000;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  01  18                                       \n"  // add EBX to *EAX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "01 00 00 00\n"  // 0x00000001
  );
  CHECK_TRACE_CONTENTS(
      "run: add EBX to r/m32\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0x00000011\n"
  );
}

:(before "End Mod Special-cases(addr)")
case 0:  // indirect addressing
  switch (rm) {
  default:  // address in register
    trace(Callstack_depth+1, "run") << "effective address is 0x" << HEXWORD << Reg[rm].u << " (" << rname(rm) << ")" << end();
    addr = Reg[rm].u;
    break;
  // End Mod 0 Special-cases(addr)
  }
  break;

//:

:(before "End Initialize Op Names")
put_new(Name, "03", "add rm32 to r32 (add)");

:(code)
void test_add_mem_at_r32_to_r32() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0x10;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  03  18                                       \n"  // add *EAX to EBX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "01 00 00 00\n"  // 0x00000001
  );
  CHECK_TRACE_CONTENTS(
      "run: add r/m32 to EBX\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0x00000011\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x03: {  // add r/m32 to r32
  const uint8_t modrm = next();
  const uint8_t arg1 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "add r/m32 to " << rname(arg1) << end();
  const int32_t* arg2 = effective_address(modrm);
  BINARY_ARITHMETIC_OP(+, Reg[arg1].i, *arg2);
  break;
}

//:: subtract

:(code)
void test_subtract_r32_from_mem_at_r32() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  29  18                                       \n"  // subtract EBX from *EAX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "0a 00 00 00\n"  // 0x0000000a
  );
  CHECK_TRACE_CONTENTS(
      "run: subtract EBX from r/m32\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0x00000009\n"
  );
}

//:

:(before "End Initialize Op Names")
put_new(Name, "2b", "subtract rm32 from r32 (sub)");

:(code)
void test_subtract_mem_at_r32_from_r32() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 10;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  2b     18                                    \n"  // subtract *EAX from EBX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "01 00 00 00\n"  // 0x00000001
  );
  CHECK_TRACE_CONTENTS(
      "run: subtract r/m32 from EBX\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0x00000009\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x2b: {  // subtract r/m32 from r32
  const uint8_t modrm = next();
  const uint8_t arg1 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "subtract r/m32 from " << rname(arg1) << end();
  const int32_t* arg2 = effective_address(modrm);
  BINARY_ARITHMETIC_OP(-, Reg[arg1].i, *arg2);
  break;
}

//:: and
:(code)
void test_and_r32_with_mem_at_r32() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0xff;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  21     18                                    \n"  // and EBX with *EAX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "0d 0c 0b 0a\n"  // 0x0a0b0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: and EBX with r/m32\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0x0000000d\n"
  );
}

//:

:(before "End Initialize Op Names")
put_new(Name, "23", "r32 = bitwise AND of r32 with rm32 (and)");

:(code)
void test_and_mem_at_r32_with_r32() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0x0a0b0c0d;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  23     18                                    \n"  // and *EAX with EBX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "ff 00 00 00\n"  // 0x000000ff
  );
  CHECK_TRACE_CONTENTS(
      "run: and r/m32 with EBX\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0x0000000d\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x23: {  // and r/m32 with r32
  const uint8_t modrm = next();
  const uint8_t arg1 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "and r/m32 with " << rname(arg1) << end();
  const int32_t* arg2 = effective_address(modrm);
  BINARY_BITWISE_OP(&, Reg[arg1].u, *arg2);
  break;
}

//:: or

:(code)
void test_or_r32_with_mem_at_r32() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0xa0b0c0d0;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  09  18                                      #\n"  // EBX with *EAX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "0d 0c 0b 0a\n"  // 0x0a0b0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: or EBX with r/m32\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0xaabbccdd\n"
  );
}

//:

:(before "End Initialize Op Names")
put_new(Name, "0b", "r32 = bitwise OR of r32 with rm32 (or)");

:(code)
void test_or_mem_at_r32_with_r32() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0xa0b0c0d0;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  0b     18                                    \n"  // or *EAX with EBX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "0d 0c 0b 0a\n"  // 0x0a0b0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: or r/m32 with EBX\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0xaabbccdd\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x0b: {  // or r/m32 with r32
  const uint8_t modrm = next();
  const uint8_t arg1 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "or r/m32 with " << rname(arg1) << end();
  const int32_t* arg2 = effective_address(modrm);
  BINARY_BITWISE_OP(|, Reg[arg1].u, *arg2);
  break;
}

//:: xor

:(code)
void test_xor_r32_with_mem_at_r32() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0xa0b0c0d0;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  31     18                                    \n"  // xor EBX with *EAX
      "== 0x2000\n"  // data segment
      "0d 0c bb aa\n"  // 0xaabb0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: xor EBX with r/m32\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0x0a0bccdd\n"
  );
}

//:

:(before "End Initialize Op Names")
put_new(Name, "33", "r32 = bitwise XOR of r32 with rm32 (xor)");

:(code)
void test_xor_mem_at_r32_with_r32() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0xa0b0c0d0;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  33     18                                    \n"  // xor *EAX with EBX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "0d 0c 0b 0a\n"  // 0x0a0b0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: xor r/m32 with EBX\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0xaabbccdd\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x33: {  // xor r/m32 with r32
  const uint8_t modrm = next();
  const uint8_t arg1 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "xor r/m32 with " << rname(arg1) << end();
  const int32_t* arg2 = effective_address(modrm);
  BINARY_BITWISE_OP(|, Reg[arg1].u, *arg2);
  break;
}

//:: not

:(code)
void test_not_of_mem_at_r32() {
  Reg[EBX].i = 0x2000;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  f7     13                                    \n"  // not *EBX
      // ModR/M in binary: 00 (indirect mode) 010 (subop not) 011 (dest EBX)
      "== 0x2000\n"  // data segment
      "ff 00 0f 0f\n"  // 0x0f0f00ff
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: effective address is 0x00002000 (EBX)\n"
      "run: subop: not\n"
      "run: storing 0xf0f0ff00\n"
  );
}

//:: compare (cmp)

:(code)
void test_compare_mem_at_r32_with_r32_greater() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0x0a0b0c07;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  39     18                                    \n"  // compare EBX with *EAX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "0d 0c 0b 0a\n"  // 0x0a0b0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: compare EBX with r/m32\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: SF=0; ZF=0; OF=0\n"
  );
}

:(code)
void test_compare_mem_at_r32_with_r32_lesser() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0x0a0b0c0d;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  39     18                                    \n"  // compare EBX with *EAX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "07 0c 0b 0a\n"  // 0x0a0b0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: compare EBX with r/m32\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: SF=1; ZF=0; OF=0\n"
  );
}

:(code)
void test_compare_mem_at_r32_with_r32_equal() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0x0a0b0c0d;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  39     18                                    \n"  // compare EBX with *EAX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "0d 0c 0b 0a\n"  // 0x0a0b0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: compare EBX with r/m32\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: SF=0; ZF=1; OF=0\n"
  );
}

//:

:(before "End Initialize Op Names")
put_new(Name, "3b", "compare: set SF if r32 < rm32 (cmp)");

:(code)
void test_compare_r32_with_mem_at_r32_greater() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0x0a0b0c0d;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  3b     18                                    \n"  // compare *EAX with EBX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "07 0c 0b 0a\n"  // 0x0a0b0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: compare r/m32 with EBX\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: SF=0; ZF=0; OF=0\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x3b: {  // set SF if r32 < r/m32
  const uint8_t modrm = next();
  const uint8_t reg1 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "compare r/m32 with " << rname(reg1) << end();
  const int32_t arg1 = Reg[reg1].i;
  const int32_t* arg2 = effective_address(modrm);
  const int32_t tmp1 = arg1 - *arg2;
  SF = (tmp1 < 0);
  ZF = (tmp1 == 0);
  int64_t tmp2 = arg1 - *arg2;
  OF = (tmp1 != tmp2);
  trace(Callstack_depth+1, "run") << "SF=" << SF << "; ZF=" << ZF << "; OF=" << OF << end();
  break;
}

:(code)
void test_compare_r32_with_mem_at_r32_lesser() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0x0a0b0c07;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  3b     18                                    \n"  // compare *EAX with EBX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "0d 0c 0b 0a\n"  // 0x0a0b0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: compare r/m32 with EBX\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: SF=1; ZF=0; OF=0\n"
  );
}

:(code)
void test_compare_r32_with_mem_at_r32_equal() {
  Reg[EAX].i = 0x2000;
  Reg[EBX].i = 0x0a0b0c0d;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  3b     18                                    \n"  // compare *EAX with EBX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "0d 0c 0b 0a\n"  // 0x0a0b0c0d
  );
  CHECK_TRACE_CONTENTS(
      "run: compare r/m32 with EBX\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: SF=0; ZF=1; OF=0\n"
  );
}

//:: copy (mov)

:(code)
void test_copy_r32_to_mem_at_r32() {
  Reg[EBX].i = 0xaf;
  Reg[EAX].i = 0x60;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  89     18                                    \n"  // copy EBX to *EAX
      // ModR/M in binary: 00 (indirect mode) 011 (src EAX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: copy EBX to r/m32\n"
      "run: effective address is 0x00000060 (EAX)\n"
      "run: storing 0x000000af\n"
  );
}

//:

:(before "End Initialize Op Names")
put_new(Name, "8b", "copy rm32 to r32 (mov)");

:(code)
void test_copy_mem_at_r32_to_r32() {
  Reg[EAX].i = 0x2000;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  8b     18                                    \n"  // copy *EAX to EBX
      "== 0x2000\n"  // data segment
      "af 00 00 00\n"  // 0x000000af
  );
  CHECK_TRACE_CONTENTS(
      "run: copy r/m32 to EBX\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: storing 0x000000af\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x8b: {  // copy r32 to r/m32
  const uint8_t modrm = next();
  const uint8_t rdest = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "copy r/m32 to " << rname(rdest) << end();
  const int32_t* src = effective_address(modrm);
  Reg[rdest].i = *src;
  trace(Callstack_depth+1, "run") << "storing 0x" << HEXWORD << *src << end();
  break;
}

//:: jump

:(code)
void test_jump_mem_at_r32() {
  Reg[EAX].i = 0x2000;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  ff     20                                    \n"  // jump to *EAX
      // ModR/M in binary: 00 (indirect mode) 100 (jump to r/m32) 000 (src EAX)
      "  05                                 00 00 00 01\n"
      "  05                                 00 00 00 02\n"
      "== 0x2000\n"  // data segment
      "08 00 00 00\n"  // 0x00000008
  );
  CHECK_TRACE_CONTENTS(
      "run: 0x00000001 opcode: ff\n"
      "run: jump to r/m32\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: jumping to 0x00000008\n"
      "run: 0x00000008 opcode: 05\n"
  );
  CHECK_TRACE_DOESNT_CONTAIN("run: 0x00000003 opcode: 05");
}

:(before "End Op ff Subops")
case 4: {  // jump to r/m32
  trace(Callstack_depth+1, "run") << "jump to r/m32" << end();
  const int32_t* arg2 = effective_address(modrm);
  EIP = *arg2;
  trace(Callstack_depth+1, "run") << "jumping to 0x" << HEXWORD << EIP << end();
  break;
}

//:: push

:(code)
void test_push_mem_at_r32() {
  Reg[EAX].i = 0x2000;
  Reg[ESP].u = 0x14;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  ff     30                                    \n"  // push *EAX to stack
      "== 0x2000\n"  // data segment
      "af 00 00 00\n"  // 0x000000af
  );
  CHECK_TRACE_CONTENTS(
      "run: push r/m32\n"
      "run: effective address is 0x00002000 (EAX)\n"
      "run: decrementing ESP to 0x00000010\n"
      "run: pushing value 0x000000af\n"
  );
}

:(before "End Op ff Subops")
case 6: {  // push r/m32 to stack
  trace(Callstack_depth+1, "run") << "push r/m32" << end();
  const int32_t* val = effective_address(modrm);
  push(*val);
  break;
}

//:: pop

:(before "End Initialize Op Names")
put_new(Name, "8f", "pop top of stack to rm32 (pop)");

:(code)
void test_pop_mem_at_r32() {
  Reg[EAX].i = 0x60;
  Reg[ESP].u = 0x2000;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  8f     00                                    \n"  // pop stack into *EAX
      // ModR/M in binary: 00 (indirect mode) 000 (pop r/m32) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "30 00 00 00\n"  // 0x00000030
  );
  CHECK_TRACE_CONTENTS(
      "run: pop into r/m32\n"
      "run: effective address is 0x00000060 (EAX)\n"
      "run: popping value 0x00000030\n"
      "run: incrementing ESP to 0x00002004\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x8f: {  // pop stack into r/m32
  const uint8_t modrm = next();
  const uint8_t subop = (modrm>>3)&0x7;
  switch (subop) {
    case 0: {
      trace(Callstack_depth+1, "run") << "pop into r/m32" << end();
      int32_t* dest = effective_address(modrm);
      *dest = pop();
      break;
    }
  }
  break;
}

//:: special-case for loading address from disp32 rather than register

:(code)
void test_add_r32_to_mem_at_displacement() {
  Reg[EBX].i = 0x10;  // source
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  01     1d            00 20 00 00             \n"  // add EBX to *0x2000
      // ModR/M in binary: 00 (indirect mode) 011 (src EBX) 101 (dest in disp32)
      "== 0x2000\n"  // data segment
      "01 00 00 00\n"  // 0x00000001
  );
  CHECK_TRACE_CONTENTS(
      "run: add EBX to r/m32\n"
      "run: effective address is 0x00002000 (disp32)\n"
      "run: storing 0x00000011\n"
  );
}

:(before "End Mod 0 Special-cases(addr)")
case 5:  // exception: mod 0b00 rm 0b101 => incoming disp32
  addr = next32();
  trace(Callstack_depth+1, "run") << "effective address is 0x" << HEXWORD << addr << " (disp32)" << end();
  break;

//:

:(code)
void test_add_r32_to_mem_at_r32_plus_disp8() {
  Reg[EBX].i = 0x10;  // source
  Reg[EAX].i = 0x1ffe;  // dest
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  01     58            02                      \n"  // add EBX to *(EAX+2)
      // ModR/M in binary: 01 (indirect+disp8 mode) 011 (src EBX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "01 00 00 00\n"  // 0x00000001
  );
  CHECK_TRACE_CONTENTS(
      "run: add EBX to r/m32\n"
      "run: effective address is initially 0x00001ffe (EAX)\n"
      "run: effective address is 0x00002000 (after adding disp8)\n"
      "run: storing 0x00000011\n"
  );
}

:(before "End Mod Special-cases(addr)")
case 1:  // indirect + disp8 addressing
  switch (rm) {
  default:
    addr = Reg[rm].u;
    trace(Callstack_depth+1, "run") << "effective address is initially 0x" << HEXWORD << addr << " (" << rname(rm) << ")" << end();
    break;
  // End Mod 1 Special-cases(addr)
  }
  if (addr > 0) {
    addr += static_cast<int8_t>(next());
    trace(Callstack_depth+1, "run") << "effective address is 0x" << HEXWORD << addr << " (after adding disp8)" << end();
  }
  break;

:(code)
void test_add_r32_to_mem_at_r32_plus_negative_disp8() {
  Reg[EBX].i = 0x10;  // source
  Reg[EAX].i = 0x2001;  // dest
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  01     58            ff                      \n"  // add EBX to *(EAX-1)
      // ModR/M in binary: 01 (indirect+disp8 mode) 011 (src EBX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "01 00 00 00\n"  // 0x00000001
  );
  CHECK_TRACE_CONTENTS(
      "run: add EBX to r/m32\n"
      "run: effective address is initially 0x00002001 (EAX)\n"
      "run: effective address is 0x00002000 (after adding disp8)\n"
      "run: storing 0x00000011\n"
  );
}

//:

:(code)
void test_add_r32_to_mem_at_r32_plus_disp32() {
  Reg[EBX].i = 0x10;  // source
  Reg[EAX].i = 0x1ffe;  // dest
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  01     98            02 00 00 00             \n"  // add EBX to *(EAX+2)
      // ModR/M in binary: 10 (indirect+disp32 mode) 011 (src EBX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "01 00 00 00\n"  // 0x00000001
  );
  CHECK_TRACE_CONTENTS(
      "run: add EBX to r/m32\n"
      "run: effective address is initially 0x00001ffe (EAX)\n"
      "run: effective address is 0x00002000 (after adding disp32)\n"
      "run: storing 0x00000011\n"
  );
}

:(before "End Mod Special-cases(addr)")
case 2:  // indirect + disp32 addressing
  switch (rm) {
  default:
    addr = Reg[rm].u;
    trace(Callstack_depth+1, "run") << "effective address is initially 0x" << HEXWORD << addr << " (" << rname(rm) << ")" << end();
    break;
  // End Mod 2 Special-cases(addr)
  }
  if (addr > 0) {
    addr += next32();
    trace(Callstack_depth+1, "run") << "effective address is 0x" << HEXWORD << addr << " (after adding disp32)" << end();
  }
  break;

:(code)
void test_add_r32_to_mem_at_r32_plus_negative_disp32() {
  Reg[EBX].i = 0x10;  // source
  Reg[EAX].i = 0x2001;  // dest
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  01     98            ff ff ff ff             \n"  // add EBX to *(EAX-1)
      // ModR/M in binary: 10 (indirect+disp32 mode) 011 (src EBX) 000 (dest EAX)
      "== 0x2000\n"  // data segment
      "01 00 00 00\n"  // 0x00000001
  );
  CHECK_TRACE_CONTENTS(
      "run: add EBX to r/m32\n"
      "run: effective address is initially 0x00002001 (EAX)\n"
      "run: effective address is 0x00002000 (after adding disp32)\n"
      "run: storing 0x00000011\n"
  );
}

//:: copy address (lea)

:(before "End Initialize Op Names")
put_new(Name, "8d", "copy address in rm32 into r32 (lea)");

:(code)
void test_copy_address() {
  Reg[EAX].u = 0x2000;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  8d     18                                    \n"  // copy address in EAX into EBX
      // ModR/M in binary: 00 (indirect mode) 011 (dest EBX) 000 (src EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: copy address into EBX\n"
      "run: effective address is 0x00002000 (EAX)\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x8d: {  // copy address of m32 to r32
  const uint8_t modrm = next();
  const uint8_t arg1 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "copy address into " << rname(arg1) << end();
  Reg[arg1].u = effective_address_number(modrm);
  break;
}
