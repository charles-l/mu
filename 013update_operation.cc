//: Once all code is loaded, save operation ids of instructions and check that
//: nothing's undefined.

:(after "int main")
  // do this before any other transforms
  Transform.push_back(update_instruction_operations);

:(code)
void update_instruction_operations(recipe_ordinal r) {
  trace(9991, "transform") << "--- compute instruction operations for recipe " << Recipe[r].name << end();
  for (long long int index = 0; index < SIZE(Recipe[r].steps); ++index) {
    instruction& inst = Recipe[r].steps.at(index);
    if (inst.is_label) continue;
    if (Recipe_ordinal.find(inst.name) == Recipe_ordinal.end()) {
      raise_error << maybe(Recipe[r].name) << "instruction " << inst.name << " has no recipe\n" << end();
      return;
    }
    inst.operation = Recipe_ordinal[inst.name];
  }
}

// hook to suppress inserting recipe name into errors and warnings (for later layers)
string maybe(string s) {
  return s + ": ";
}

// temporarily suppress run
void transform(string form) {
  load(form);
  transform_all();
}