void test_literal() {
  compile("recipe main [\n"
          "  1:integer <- copy 23:literal\n"
          "]\n");
  run("main");
  CHECK_EQ(Memory[1], 23);
}
