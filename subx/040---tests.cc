//: Automatically aggregating functions into test suites.

//: We don't rely on any transforms running in previous layers, but this layer
//: knows about labels and will emit labels for previous layers to transform.
:(after "Begin Transforms")
// Begin Level-4 Transforms
Transform.push_back(create_test_function);
// End Level-4 Transforms

:(scenario run_test)
% Reg[ESP].u = 0x100;
== 0x1
main:
  e8/call run_tests/disp32  # 5 bytes
  f4/halt                   # 1 byte

test_foo:  # offset 7
  01 d8  # just some unique instruction: add EBX to EAX
  c3/return

# check that code in test_foo ran (implicitly called by run_tests)
+run: inst: 0x00000007

:(code)
void create_test_function(program& p) {
  if (p.segments.empty()) return;
  segment& code = p.segments.at(0);
  trace(99, "transform") << "-- create 'run_tests'" << end();
  vector<line> new_insts;
  for (int i = 0;  i < SIZE(code.lines);  ++i) {
    line& inst = code.lines.at(i);
    for (int j = 0;  j < SIZE(inst.words);  ++j) {
      const word& curr = inst.words.at(j);
      if (*curr.data.rbegin() != ':') continue;  // not a label
      if (!starts_with(curr.data, "test_")) continue;
      string fn = drop_last(curr.data);
      new_insts.push_back(call(fn));
    }
  }
  if (new_insts.empty()) return;  // no tests found
  code.lines.push_back(label("run_tests"));
  code.lines.insert(code.lines.end(), new_insts.begin(), new_insts.end());
  code.lines.push_back(ret());
}

string to_string(const segment& s) {
  ostringstream out;
  for (int i = 0;  i < SIZE(s.lines);  ++i) {
    const line& l = s.lines.at(i);
    for (int j = 0;  j < SIZE(l.words);  ++j) {
      if (j > 0) out << ' ';
      out << to_string(l.words.at(j));
    }
    out << '\n';
  }
  return out.str();
}

line call(string s) {
  line result;
  result.words.push_back(call());
  result.words.push_back(disp32(s));
  return result;
}

word call() {
  word result;
  result.data = "e8";
  result.metadata.push_back("call");
  return result;
}

word disp32(string s) {
  word result;
  result.data = s;
  result.metadata.push_back("disp32");
  return result;
}

line ret() {
  line result;
  result.words.push_back(word());
  result.words.back().data = "c3";
  result.words.back().metadata.push_back("return");
  return result;
}