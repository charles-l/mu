//:: simulated x86 registers

:(before "End Types")
enum {
  EAX,
  ECX,
  EDX,
  EBX,
  ESP,
  EBP,
  ESI,
  EDI,
  NUM_INT_REGISTERS,
};
union reg {
  int32_t i;
  uint32_t u;
};
:(before "End Globals")
reg R[NUM_INT_REGISTERS] = { {0} };
uint32_t EIP = 0;
:(before "End Reset")
bzero(R, sizeof(R));
EIP = 0;

//:: simulated flag registers; just a subset that we care about

:(before "End Globals")
bool SF = false;  // sign flag
bool ZF = false;  // zero flag
bool OF = false;  // overflow flag
:(before "End Reset")
SF = ZF = OF = false;

//: how the flag registers are updated after each instruction

:(before "End Includes")
// beware: no side-effects in args
#define BINARY_ARITHMETIC_OP(op, arg1, arg2) { \
  /* arg1 and arg2 must be signed */ \
  int64_t tmp = arg1 op arg2; \
  arg1 = arg1 op arg2; \
  SF = (arg1 < 0); \
  ZF = (arg1 == 0); \
  OF = (arg1 != tmp); \
}

#define BINARY_BITWISE_OP(op, arg1, arg2) { \
  /* arg1 and arg2 must be unsigned */ \
  arg1 = arg1 op arg2; \
  SF = (arg1 >> 31); \
  ZF = (arg1 == 0); \
  OF = false; \
}

//:: simulated RAM

:(before "End Globals")
map<uint32_t, uint8_t> Memory;
uint32_t End_of_program = 0;
:(before "End Reset")
Memory.clear();
End_of_program = 0;

//:: core interpreter loop

:(scenario add_imm32_to_eax)
# In scenarios, programs are a series of hex bytes, each (variable-length)
# instruction on one line.
#
# x86 instructions consist of the following parts (see cheatsheet.pdf):
#   opcode        ModRM                 SIB                   displacement    immediate
#   instruction   mod, reg, R/M bits    scale, index, base
#   1-3 bytes     0/1 byte              0/1 byte              0/1/2/4 bytes   0/1/2/4 bytes
    05                                                                        0a 0b 0c 0d  # add 0x0d0c0b0a to EAX
# All hex bytes must be exactly 2 characters each. No '0x' prefixes.
+load: 1 -> 05
+load: 2 -> 0a
+load: 3 -> 0b
+load: 4 -> 0c
+load: 5 -> 0d
+run: add imm32 0x0d0c0b0a to reg EAX
+reg: storing 0x0d0c0b0a in reg EAX

:(code)
// helper for tests: load a program into memory from a textual representation
// of its bytes, and run it
void run(const string& text_bytes) {
  load_program(text_bytes);
  EIP = 1;  // preserve null pointer
  while (EIP < End_of_program)
    run_one_instruction();
}

// skeleton of how x86 instructions are decoded
void run_one_instruction() {
  uint8_t op=0, op2=0, op3=0;
  switch(op = next()) {
  // our first opcode
  case 0xf4:  // hlt
    EIP = End_of_program;
    break;
  case 0x05: {  // add imm32 to EAX
    int32_t arg2 = imm32();
    trace(2, "run") << "add imm32 0x" << HEXWORD << arg2 << " to reg EAX" << end();
    BINARY_ARITHMETIC_OP(+, R[EAX].i, arg2);
    trace(98, "reg") << "storing 0x" << HEXWORD << R[EAX].i << " in reg EAX" << end();
    break;
  }
  // End Single-Byte Opcodes
  case 0x0f:
    switch(op2 = next()) {
    // End Two-Byte Opcodes Starting With 0f
    default:
      cerr << "unrecognized second opcode after 0f: " << std::hex << static_cast<int>(op2) << '\n';
      exit(1);
    }
    break;
  case 0xf3:
    switch(op2 = next()) {
    // End Two-Byte Opcodes Starting With f3
    case 0x0f:
      switch(op3 = next()) {
      // End Three-Byte Opcodes Starting With f3 0f
      default:
        cerr << "unrecognized third opcode after f3 0f: " << std::hex << static_cast<int>(op3) << '\n';
        exit(1);
      }
      break;
    default:
      cerr << "unrecognized second opcode after f3: " << std::hex << static_cast<int>(op2) << '\n';
      exit(1);
    }
    break;
  default:
    cerr << "unrecognized opcode: " << std::hex << static_cast<int>(op) << '\n';
    exit(1);
  }
}

void load_program(const string& text_bytes) {
  uint32_t addr = 1;
  istringstream in(text_bytes);
  in >> std::noskipws;
  while (has_data(in)) {
    char c1 = next_hex_byte(in);
    if (c1 == '\0') break;
    if (!has_data(in)) {
      raise << "input program truncated mid-byte\n" << end();
      return;
    }
    char c2 = next_hex_byte(in);
    if (c2 == '\0') {
      raise << "input program truncated mid-byte\n" << end();
      return;
    }
    put(Memory, addr, to_byte(c1, c2));
    trace(99, "load") << addr << " -> " << HEXBYTE << static_cast<int>(get_or_insert(Memory, addr)) << end();  // ugly that iostream doesn't print uint8_t as an integer
    addr++;
  }
  End_of_program = addr;
}

char next_hex_byte(istream& in) {
  while (has_data(in)) {
    char c = '\0';
    in >> c;
    if (c == ' ' || c == '\n') continue;
    while (c == '#') {
      while (has_data(in)) {
        in >> c;
        if (c == '\n') {
          in >> c;
          break;
        }
      }
    }
    if (c >= '0' && c <= '9') return c;
    else if (c >= 'a' && c <= 'f') return c;
    else if (c >= 'A' && c <= 'F') return tolower(c);
    // disallow any non-hex characters, including a '0x' prefix
    if (!isspace(c)) {
      raise << "invalid non-hex character '" << c << "'\n" << end();
      break;
    }
  }
  return '\0';
}

uint8_t to_byte(char hex_byte1, char hex_byte2) {
  return to_hex_num(hex_byte1)*16 + to_hex_num(hex_byte2);
}
uint8_t to_hex_num(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  assert(false);
  return 0;
}

uint8_t next() {
  return get_or_insert(Memory, EIP++);
}

// read a 32-bit immediate in little-endian order from the instruction stream
int32_t imm32() {
  int32_t result = next();
  result |= (next()<<8);
  result |= (next()<<16);
  result |= (next()<<24);
  return result;
}

:(before "End Includes")
#include <iomanip>
#define HEXBYTE  std::hex << std::setw(2) << std::setfill('0')
#define HEXWORD  std::hex << std::setw(8) << std::setfill('0')
#include <stdint.h>