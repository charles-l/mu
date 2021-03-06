//: operating directly on a register

:(before "End Initialize Op Names")
put_new(Name, "01", "add r32 to rm32 (add)");

:(code)
void test_add_r32_to_r32() {
  Reg[EAX].i = 0x10;
  Reg[EBX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  01     d8                                    \n" // add EBX to EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: add EBX to r/m32\n"
      "run: r/m32 is EAX\n"
      "run: storing 0x00000011\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x01: {  // add r32 to r/m32
  uint8_t modrm = next();
  uint8_t arg2 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "add " << rname(arg2) << " to r/m32" << end();
  int32_t* arg1 = effective_address(modrm);
  BINARY_ARITHMETIC_OP(+, *arg1, Reg[arg2].i);
  break;
}

:(code)
// Implement tables 2-2 and 2-3 in the Intel manual, Volume 2.
// We return a pointer so that instructions can write to multiple bytes in
// 'Mem' at once.
// beware: will eventually have side-effects
int32_t* effective_address(uint8_t modrm) {
  const uint8_t mod = (modrm>>6);
  // ignore middle 3 'reg opcode' bits
  const uint8_t rm = modrm & 0x7;
  if (mod == 3) {
    // mod 3 is just register direct addressing
    trace(Callstack_depth+1, "run") << "r/m32 is " << rname(rm) << end();
    return &Reg[rm].i;
  }
  uint32_t addr = effective_address_number(modrm);
  trace(Callstack_depth+1, "run") << "effective address contains " << read_mem_i32(addr) << end();
  return mem_addr_i32(addr);
}

// beware: will eventually have side-effects
uint32_t effective_address_number(uint8_t modrm) {
  const uint8_t mod = (modrm>>6);
  // ignore middle 3 'reg opcode' bits
  const uint8_t rm = modrm & 0x7;
  uint32_t addr = 0;
  switch (mod) {
  case 3:
    // mod 3 is just register direct addressing
    raise << "unexpected direct addressing mode\n" << end();
    return 0;
  // End Mod Special-cases(addr)
  default:
    cerr << "unrecognized mod bits: " << NUM(mod) << '\n';
    exit(1);
  }
  //: other mods are indirect, and they'll set addr appropriately
  // Found effective_address(addr)
  return addr;
}

string rname(uint8_t r) {
  switch (r) {
  case 0: return "EAX";
  case 1: return "ECX";
  case 2: return "EDX";
  case 3: return "EBX";
  case 4: return "ESP";
  case 5: return "EBP";
  case 6: return "ESI";
  case 7: return "EDI";
  default: raise << "invalid register " << r << '\n' << end();  return "";
  }
}

//:: subtract

:(before "End Initialize Op Names")
put_new(Name, "29", "subtract r32 from rm32 (sub)");

:(code)
void test_subtract_r32_from_r32() {
  Reg[EAX].i = 10;
  Reg[EBX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  29     d8                                    \n"  // subtract EBX from EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: subtract EBX from r/m32\n"
      "run: r/m32 is EAX\n"
      "run: storing 0x00000009\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x29: {  // subtract r32 from r/m32
  const uint8_t modrm = next();
  const uint8_t arg2 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "subtract " << rname(arg2) << " from r/m32" << end();
  int32_t* arg1 = effective_address(modrm);
  BINARY_ARITHMETIC_OP(-, *arg1, Reg[arg2].i);
  break;
}

//:: multiply

:(before "End Initialize Op Names")
put_new(Name, "f7", "negate/multiply rm32 (with EAX if necessary) depending on subop (neg/mul)");

:(code)
void test_multiply_eax_by_r32() {
  Reg[EAX].i = 4;
  Reg[ECX].i = 3;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  f7     e1                                    \n"  // multiply EAX by ECX
      // ModR/M in binary: 11 (direct mode) 100 (subop mul) 001 (src ECX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is ECX\n"
      "run: subop: multiply EAX by r/m32\n"
      "run: storing 0x0000000c\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0xf7: {
  const uint8_t modrm = next();
  trace(Callstack_depth+1, "run") << "operate on r/m32" << end();
  int32_t* arg1 = effective_address(modrm);
  const uint8_t subop = (modrm>>3)&0x7;  // middle 3 'reg opcode' bits
  switch (subop) {
  case 4: {  // mul unsigned EAX by r/m32
    trace(Callstack_depth+1, "run") << "subop: multiply EAX by r/m32" << end();
    const uint64_t result = Reg[EAX].u * static_cast<uint32_t>(*arg1);
    Reg[EAX].u = result & 0xffffffff;
    Reg[EDX].u = result >> 32;
    OF = (Reg[EDX].u != 0);
    trace(Callstack_depth+1, "run") << "storing 0x" << HEXWORD << Reg[EAX].u << end();
    break;
  }
  // End Op f7 Subops
  default:
    cerr << "unrecognized subop for opcode f7: " << NUM(subop) << '\n';
    exit(1);
  }
  break;
}

//:

:(before "End Initialize Op Names")
put_new(Name_0f, "af", "multiply rm32 into r32 (imul)");

:(code)
void test_multiply_r32_into_r32() {
  Reg[EAX].i = 4;
  Reg[EBX].i = 2;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  0f af  d8                                    \n"  // subtract EBX into EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: multiply r/m32 into EBX\n"
      "run: r/m32 is EAX\n"
      "run: storing 0x00000008\n"
  );
}

:(before "End Two-Byte Opcodes Starting With 0f")
case 0xaf: {  // multiply r32 into r/m32
  const uint8_t modrm = next();
  const uint8_t arg2 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "multiply r/m32 into " << rname(arg2) << end();
  const int32_t* arg1 = effective_address(modrm);
  BINARY_ARITHMETIC_OP(*, Reg[arg2].i, *arg1);
  break;
}

//:: negate

:(code)
void test_negate_r32() {
  Reg[EBX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  f7 db                                        \n"  // negate EBX
      // ModR/M in binary: 11 (direct mode) 011 (subop negate) 011 (dest EBX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is EBX\n"
      "run: subop: negate\n"
      "run: storing 0xffffffff\n"
  );
}

:(before "End Op f7 Subops")
case 3: {  // negate r/m32
  trace(Callstack_depth+1, "run") << "subop: negate" << end();
  // one case that can overflow
  if (static_cast<uint32_t>(*arg1) == 0x80000000) {
    trace(Callstack_depth+1, "run") << "overflow" << end();
    SF = true;
    ZF = false;
    OF = true;
    break;
  }
  *arg1 = -(*arg1);
  trace(Callstack_depth+1, "run") << "storing 0x" << HEXWORD << *arg1 << end();
  SF = (*arg1 >> 31);
  ZF = (*arg1 == 0);
  OF = false;
  break;
}

:(code)
// negate can overflow in exactly one situation
void test_negate_can_overflow() {
  Reg[EBX].i = 0x80000000;  // INT_MIN
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  f7 db                                        \n"  // negate EBX
      // ModR/M in binary: 11 (direct mode) 011 (subop negate) 011 (dest EBX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is EBX\n"
      "run: subop: negate\n"
      "run: overflow\n"
  );
}

//:: shift left

:(before "End Initialize Op Names")
put_new(Name, "d3", "shift rm32 by CL bits depending on subop (sal/sar/shl/shr)");

:(code)
void test_shift_left_r32_with_cl() {
  Reg[EBX].i = 13;
  Reg[ECX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  d3     e3                                    \n"  // shift EBX left by CL bits
      // ModR/M in binary: 11 (direct mode) 100 (subop shift left) 011 (dest EBX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is EBX\n"
      "run: subop: shift left by CL bits\n"
      "run: storing 0x0000001a\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0xd3: {
  const uint8_t modrm = next();
  trace(Callstack_depth+1, "run") << "operate on r/m32" << end();
  int32_t* arg1 = effective_address(modrm);
  const uint8_t subop = (modrm>>3)&0x7;  // middle 3 'reg opcode' bits
  switch (subop) {
  case 4: {  // shift left r/m32 by CL
    trace(Callstack_depth+1, "run") << "subop: shift left by CL bits" << end();
    uint8_t count = Reg[ECX].u & 0x1f;
    // OF is only defined if count is 1
    if (count == 1) {
      bool msb = (*arg1 & 0x80000000) >> 1;
      bool pnsb = (*arg1 & 0x40000000);
      OF = (msb != pnsb);
    }
    *arg1 = (*arg1 << count);
    ZF = (*arg1 == 0);
    SF = (*arg1 < 0);
    trace(Callstack_depth+1, "run") << "storing 0x" << HEXWORD << *arg1 << end();
    break;
  }
  // End Op d3 Subops
  default:
    cerr << "unrecognized subop for opcode d3: " << NUM(subop) << '\n';
    exit(1);
  }
  break;
}

//:: shift right arithmetic

:(code)
void test_shift_right_arithmetic_r32_with_cl() {
  Reg[EBX].i = 26;
  Reg[ECX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  d3     fb                                    \n"  // shift EBX right by CL bits, while preserving sign
      // ModR/M in binary: 11 (direct mode) 111 (subop shift right arithmetic) 011 (dest EBX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is EBX\n"
      "run: subop: shift right by CL bits, while preserving sign\n"
      "run: storing 0x0000000d\n"
  );
}

:(before "End Op d3 Subops")
case 7: {  // shift right r/m32 by CL, preserving sign
  trace(Callstack_depth+1, "run") << "subop: shift right by CL bits, while preserving sign" << end();
  uint8_t count = Reg[ECX].u & 0x1f;
  *arg1 = (*arg1 >> count);
  ZF = (*arg1 == 0);
  SF = (*arg1 < 0);
  // OF is only defined if count is 1
  if (count == 1) OF = false;
  trace(Callstack_depth+1, "run") << "storing 0x" << HEXWORD << *arg1 << end();
  break;
}

:(code)
void test_shift_right_arithmetic_odd_r32_with_cl() {
  Reg[EBX].i = 27;
  Reg[ECX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  d3     fb                                    \n"  // shift EBX right by CL bits, while preserving sign
      // ModR/M in binary: 11 (direct mode) 111 (subop shift right arithmetic) 011 (dest EBX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is EBX\n"
      "run: subop: shift right by CL bits, while preserving sign\n"
      // result: 13
      "run: storing 0x0000000d\n"
  );
}

void test_shift_right_arithmetic_negative_r32_with_cl() {
  Reg[EBX].i = 0xfffffffd;  // -3
  Reg[ECX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  d3     fb                                    \n"  // shift EBX right by CL bits, while preserving sign
      // ModR/M in binary: 11 (direct mode) 111 (subop shift right arithmetic) 011 (dest EBX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is EBX\n"
      "run: subop: shift right by CL bits, while preserving sign\n"
      // result: -2
      "run: storing 0xfffffffe\n"
  );
}

//:: shift right logical

:(code)
void test_shift_right_logical_r32_with_cl() {
  Reg[EBX].i = 26;
  Reg[ECX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  d3     eb                                    \n"  // shift EBX right by CL bits, while padding zeroes
      // ModR/M in binary: 11 (direct mode) 101 (subop shift right logical) 011 (dest EBX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is EBX\n"
      "run: subop: shift right by CL bits, while padding zeroes\n"
      // result: 13
      "run: storing 0x0000000d\n"
  );
}

:(before "End Op d3 Subops")
case 5: {  // shift right r/m32 by CL, preserving sign
  trace(Callstack_depth+1, "run") << "subop: shift right by CL bits, while padding zeroes" << end();
  uint8_t count = Reg[ECX].u & 0x1f;
  // OF is only defined if count is 1
  if (count == 1) {
    bool msb = (*arg1 & 0x80000000) >> 1;
    bool pnsb = (*arg1 & 0x40000000);
    OF = (msb != pnsb);
  }
  uint32_t* uarg1 = reinterpret_cast<uint32_t*>(arg1);
  *uarg1 = (*uarg1 >> count);
  ZF = (*uarg1 == 0);
  // result is always positive by definition
  SF = false;
  trace(Callstack_depth+1, "run") << "storing 0x" << HEXWORD << *arg1 << end();
  break;
}

:(code)
void test_shift_right_logical_odd_r32_with_cl() {
  Reg[EBX].i = 27;
  Reg[ECX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  d3     eb                                    \n"  // shift EBX right by CL bits, while padding zeroes
      // ModR/M in binary: 11 (direct mode) 101 (subop shift right logical) 011 (dest EBX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is EBX\n"
      "run: subop: shift right by CL bits, while padding zeroes\n"
      // result: 13
      "run: storing 0x0000000d\n"
  );
}

void test_shift_right_logical_negative_r32_with_cl() {
  Reg[EBX].i = 0xfffffffd;
  Reg[ECX].i = 1;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  d3     eb                                    \n"  // shift EBX right by CL bits, while padding zeroes
      // ModR/M in binary: 11 (direct mode) 101 (subop shift right logical) 011 (dest EBX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is EBX\n"
      "run: subop: shift right by CL bits, while padding zeroes\n"
      "run: storing 0x7ffffffe\n"
  );
}

//:: and

:(before "End Initialize Op Names")
put_new(Name, "21", "rm32 = bitwise AND of r32 with rm32 (and)");

:(code)
void test_and_r32_with_r32() {
  Reg[EAX].i = 0x0a0b0c0d;
  Reg[EBX].i = 0x000000ff;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  21     d8                                    \n"  // and EBX with destination EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: and EBX with r/m32\n"
      "run: r/m32 is EAX\n"
      "run: storing 0x0000000d\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x21: {  // and r32 with r/m32
  const uint8_t modrm = next();
  const uint8_t arg2 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "and " << rname(arg2) << " with r/m32" << end();
  int32_t* arg1 = effective_address(modrm);
  BINARY_BITWISE_OP(&, *arg1, Reg[arg2].u);
  break;
}

//:: or

:(before "End Initialize Op Names")
put_new(Name, "09", "rm32 = bitwise OR of r32 with rm32 (or)");

:(code)
void test_or_r32_with_r32() {
  Reg[EAX].i = 0x0a0b0c0d;
  Reg[EBX].i = 0xa0b0c0d0;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  09     d8                                    \n"  // or EBX with destination EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: or EBX with r/m32\n"
      "run: r/m32 is EAX\n"
      "run: storing 0xaabbccdd\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x09: {  // or r32 with r/m32
  const uint8_t modrm = next();
  const uint8_t arg2 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "or " << rname(arg2) << " with r/m32" << end();
  int32_t* arg1 = effective_address(modrm);
  BINARY_BITWISE_OP(|, *arg1, Reg[arg2].u);
  break;
}

//:: xor

:(before "End Initialize Op Names")
put_new(Name, "31", "rm32 = bitwise XOR of r32 with rm32 (xor)");

:(code)
void test_xor_r32_with_r32() {
  Reg[EAX].i = 0x0a0b0c0d;
  Reg[EBX].i = 0xaabbc0d0;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  31     d8                                    \n"  // xor EBX with destination EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: xor EBX with r/m32\n"
      "run: r/m32 is EAX\n"
      "run: storing 0xa0b0ccdd\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x31: {  // xor r32 with r/m32
  const uint8_t modrm = next();
  const uint8_t arg2 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "xor " << rname(arg2) << " with r/m32" << end();
  int32_t* arg1 = effective_address(modrm);
  BINARY_BITWISE_OP(^, *arg1, Reg[arg2].u);
  break;
}

//:: not

:(code)
void test_not_r32() {
  Reg[EBX].i = 0x0f0f00ff;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  f7     d3                                    \n"  // not EBX
      // ModR/M in binary: 11 (direct mode) 010 (subop not) 011 (dest EBX)
  );
  CHECK_TRACE_CONTENTS(
      "run: operate on r/m32\n"
      "run: r/m32 is EBX\n"
      "run: subop: not\n"
      "run: storing 0xf0f0ff00\n"
  );
}

:(before "End Op f7 Subops")
case 2: {  // not r/m32
  trace(Callstack_depth+1, "run") << "subop: not" << end();
  *arg1 = ~(*arg1);
  trace(Callstack_depth+1, "run") << "storing 0x" << HEXWORD << *arg1 << end();
  SF = (*arg1 >> 31);
  ZF = (*arg1 == 0);
  OF = false;
  break;
}

//:: compare (cmp)

:(before "End Initialize Op Names")
put_new(Name, "39", "compare: set SF if rm32 < r32 (cmp)");

:(code)
void test_compare_r32_with_r32_greater() {
  Reg[EAX].i = 0x0a0b0c0d;
  Reg[EBX].i = 0x0a0b0c07;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  39     d8                                    \n"  // compare EBX with EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: compare EBX with r/m32\n"
      "run: r/m32 is EAX\n"
      "run: SF=0; ZF=0; OF=0\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x39: {  // set SF if r/m32 < r32
  const uint8_t modrm = next();
  const uint8_t reg2 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "compare " << rname(reg2) << " with r/m32" << end();
  const int32_t* arg1 = effective_address(modrm);
  const int32_t arg2 = Reg[reg2].i;
  const int32_t tmp1 = *arg1 - arg2;
  SF = (tmp1 < 0);
  ZF = (tmp1 == 0);
  const int64_t tmp2 = *arg1 - arg2;
  OF = (tmp1 != tmp2);
  trace(Callstack_depth+1, "run") << "SF=" << SF << "; ZF=" << ZF << "; OF=" << OF << end();
  break;
}

:(code)
void test_compare_r32_with_r32_lesser() {
  Reg[EAX].i = 0x0a0b0c07;
  Reg[EBX].i = 0x0a0b0c0d;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  39     d8                                    \n"  // compare EBX with EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: compare EBX with r/m32\n"
      "run: r/m32 is EAX\n"
      "run: SF=1; ZF=0; OF=0\n"
  );
}

void test_compare_r32_with_r32_equal() {
  Reg[EAX].i = 0x0a0b0c0d;
  Reg[EBX].i = 0x0a0b0c0d;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  39     d8                                    \n"  // compare EBX with EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: compare EBX with r/m32\n"
      "run: r/m32 is EAX\n"
      "run: SF=0; ZF=1; OF=0\n"
  );
}

//:: copy (mov)

:(before "End Initialize Op Names")
put_new(Name, "89", "copy r32 to rm32 (mov)");

:(code)
void test_copy_r32_to_r32() {
  Reg[EBX].i = 0xaf;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  89     d8                                    \n"  // copy EBX to EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: copy EBX to r/m32\n"
      "run: r/m32 is EAX\n"
      "run: storing 0x000000af\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x89: {  // copy r32 to r/m32
  const uint8_t modrm = next();
  const uint8_t rsrc = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "copy " << rname(rsrc) << " to r/m32" << end();
  int32_t* dest = effective_address(modrm);
  *dest = Reg[rsrc].i;
  trace(Callstack_depth+1, "run") << "storing 0x" << HEXWORD << *dest << end();
  break;
}

//:: xchg

:(before "End Initialize Op Names")
put_new(Name, "87", "swap the contents of r32 and rm32 (xchg)");

:(code)
void test_xchg_r32_with_r32() {
  Reg[EBX].i = 0xaf;
  Reg[EAX].i = 0x2e;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  87     d8                                    \n"  // exchange EBX with EAX
      // ModR/M in binary: 11 (direct mode) 011 (src EBX) 000 (dest EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: exchange EBX with r/m32\n"
      "run: r/m32 is EAX\n"
      "run: storing 0x000000af in r/m32\n"
      "run: storing 0x0000002e in EBX\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x87: {  // exchange r32 with r/m32
  const uint8_t modrm = next();
  const uint8_t reg2 = (modrm>>3)&0x7;
  trace(Callstack_depth+1, "run") << "exchange " << rname(reg2) << " with r/m32" << end();
  int32_t* arg1 = effective_address(modrm);
  const int32_t tmp = *arg1;
  *arg1 = Reg[reg2].i;
  Reg[reg2].i = tmp;
  trace(Callstack_depth+1, "run") << "storing 0x" << HEXWORD << *arg1 << " in r/m32" << end();
  trace(Callstack_depth+1, "run") << "storing 0x" << HEXWORD << Reg[reg2].i << " in " << rname(reg2) << end();
  break;
}

//:: increment

:(before "End Initialize Op Names")
put_new(Name, "40", "increment EAX (inc)");
put_new(Name, "41", "increment ECX (inc)");
put_new(Name, "42", "increment EDX (inc)");
put_new(Name, "43", "increment EBX (inc)");
put_new(Name, "44", "increment ESP (inc)");
put_new(Name, "45", "increment EBP (inc)");
put_new(Name, "46", "increment ESI (inc)");
put_new(Name, "47", "increment EDI (inc)");

:(code)
void test_increment_r32() {
  Reg[ECX].u = 0x1f;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  41                                           \n"  // increment ECX
  );
  CHECK_TRACE_CONTENTS(
      "run: increment ECX\n"
      "run: storing value 0x00000020\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x40:
case 0x41:
case 0x42:
case 0x43:
case 0x44:
case 0x45:
case 0x46:
case 0x47: {  // increment r32
  const uint8_t reg = op & 0x7;
  trace(Callstack_depth+1, "run") << "increment " << rname(reg) << end();
  ++Reg[reg].u;
  trace(Callstack_depth+1, "run") << "storing value 0x" << HEXWORD << Reg[reg].u << end();
  break;
}

:(before "End Initialize Op Names")
put_new(Name, "ff", "increment/decrement/jump/push/call rm32 based on subop (inc/dec/jmp/push/call)");

:(code)
void test_increment_rm32() {
  Reg[EAX].u = 0x20;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  ff     c0                                    \n"  // increment EAX
      // ModR/M in binary: 11 (direct mode) 000 (subop inc) 000 (EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: increment r/m32\n"
      "run: r/m32 is EAX\n"
      "run: storing value 0x00000021\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0xff: {
  const uint8_t modrm = next();
  const uint8_t subop = (modrm>>3)&0x7;  // middle 3 'reg opcode' bits
  switch (subop) {
    case 0: {  // increment r/m32
      trace(Callstack_depth+1, "run") << "increment r/m32" << end();
      int32_t* arg = effective_address(modrm);
      ++*arg;
      trace(Callstack_depth+1, "run") << "storing value 0x" << HEXWORD << *arg << end();
      break;
    }
    default:
      cerr << "unrecognized subop for ff: " << HEXBYTE << NUM(subop) << '\n';
      DUMP("");
      exit(1);
    // End Op ff Subops
  }
  break;
}

//:: decrement

:(before "End Initialize Op Names")
put_new(Name, "48", "decrement EAX (dec)");
put_new(Name, "49", "decrement ECX (dec)");
put_new(Name, "4a", "decrement EDX (dec)");
put_new(Name, "4b", "decrement EBX (dec)");
put_new(Name, "4c", "decrement ESP (dec)");
put_new(Name, "4d", "decrement EBP (dec)");
put_new(Name, "4e", "decrement ESI (dec)");
put_new(Name, "4f", "decrement EDI (dec)");

:(code)
void test_decrement_r32() {
  Reg[ECX].u = 0x1f;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  49                                           \n"  // decrement ECX
  );
  CHECK_TRACE_CONTENTS(
      "run: decrement ECX\n"
      "run: storing value 0x0000001e\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x48:
case 0x49:
case 0x4a:
case 0x4b:
case 0x4c:
case 0x4d:
case 0x4e:
case 0x4f: {  // decrement r32
  const uint8_t reg = op & 0x7;
  trace(Callstack_depth+1, "run") << "decrement " << rname(reg) << end();
  --Reg[reg].u;
  trace(Callstack_depth+1, "run") << "storing value 0x" << HEXWORD << Reg[reg].u << end();
  break;
}

:(code)
void test_decrement_rm32() {
  Reg[EAX].u = 0x20;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  ff     c8                                    \n"  // decrement EAX
      // ModR/M in binary: 11 (direct mode) 001 (subop inc) 000 (EAX)
  );
  CHECK_TRACE_CONTENTS(
      "run: decrement r/m32\n"
      "run: r/m32 is EAX\n"
      "run: storing value 0x0000001f\n"
  );
}

:(before "End Op ff Subops")
case 1: {  // decrement r/m32
  trace(Callstack_depth+1, "run") << "decrement r/m32" << end();
  int32_t* arg = effective_address(modrm);
  --*arg;
  trace(Callstack_depth+1, "run") << "storing value 0x" << HEXWORD << *arg << end();
  break;
}

//:: push

:(before "End Initialize Op Names")
put_new(Name, "50", "push EAX to stack (push)");
put_new(Name, "51", "push ECX to stack (push)");
put_new(Name, "52", "push EDX to stack (push)");
put_new(Name, "53", "push EBX to stack (push)");
put_new(Name, "54", "push ESP to stack (push)");
put_new(Name, "55", "push EBP to stack (push)");
put_new(Name, "56", "push ESI to stack (push)");
put_new(Name, "57", "push EDI to stack (push)");

:(code)
void test_push_r32() {
  Reg[ESP].u = 0x64;
  Reg[EBX].i = 0x0000000a;
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  53                                           \n"  // push EBX to stack
  );
  CHECK_TRACE_CONTENTS(
      "run: push EBX\n"
      "run: decrementing ESP to 0x00000060\n"
      "run: pushing value 0x0000000a\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x50:
case 0x51:
case 0x52:
case 0x53:
case 0x54:
case 0x55:
case 0x56:
case 0x57: {  // push r32 to stack
  uint8_t reg = op & 0x7;
  trace(Callstack_depth+1, "run") << "push " << rname(reg) << end();
//?   cerr << "push: " << NUM(reg) << ": " << Reg[reg].u << " => " << Reg[ESP].u << '\n';
  push(Reg[reg].u);
  break;
}

//:: pop

:(before "End Initialize Op Names")
put_new(Name, "58", "pop top of stack to EAX (pop)");
put_new(Name, "59", "pop top of stack to ECX (pop)");
put_new(Name, "5a", "pop top of stack to EDX (pop)");
put_new(Name, "5b", "pop top of stack to EBX (pop)");
put_new(Name, "5c", "pop top of stack to ESP (pop)");
put_new(Name, "5d", "pop top of stack to EBP (pop)");
put_new(Name, "5e", "pop top of stack to ESI (pop)");
put_new(Name, "5f", "pop top of stack to EDI (pop)");

:(code)
void test_pop_r32() {
  Reg[ESP].u = 0x02000000;
  Mem.push_back(vma(0x02000000));  // manually allocate memory
  write_mem_i32(0x02000000, 0x0000000a);  // ..before this write
  run(
      "== 0x1\n"  // code segment
      // op     ModR/M  SIB   displacement  immediate
      "  5b                                           \n"  // pop stack to EBX
      "== 0x2000\n"  // data segment
      "0a 00 00 00\n"  // 0x0000000a
  );
  CHECK_TRACE_CONTENTS(
      "run: pop into EBX\n"
      "run: popping value 0x0000000a\n"
      "run: incrementing ESP to 0x02000004\n"
  );
}

:(before "End Single-Byte Opcodes")
case 0x58:
case 0x59:
case 0x5a:
case 0x5b:
case 0x5c:
case 0x5d:
case 0x5e:
case 0x5f: {  // pop stack into r32
  const uint8_t reg = op & 0x7;
  trace(Callstack_depth+1, "run") << "pop into " << rname(reg) << end();
//?   cerr << "pop from " << Reg[ESP].u << '\n';
  Reg[reg].u = pop();
//?   cerr << "=> " << NUM(reg) << ": " << Reg[reg].u << '\n';
  break;
}
:(code)
uint32_t pop() {
  const uint32_t result = read_mem_u32(Reg[ESP].u);
  trace(Callstack_depth+1, "run") << "popping value 0x" << HEXWORD << result << end();
  Reg[ESP].u += 4;
  trace(Callstack_depth+1, "run") << "incrementing ESP to 0x" << HEXWORD << Reg[ESP].u << end();
  return result;
}
