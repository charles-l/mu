//: Advanced notation for the common/easy case where a recipe takes some fixed
//: number of ingredients and yields some fixed number of products.

:(scenario recipe_with_header)
recipe main [
  1:number/raw <- add2 3, 5
]
recipe add2 x:number, y:number -> z:number [
  local-scope
  load-ingredients
  z:number <- add x, y
  reply z
]
+mem: storing 8 in location 1

//: When loading recipes save any header.

:(before "End recipe Fields")
bool has_header;
vector<reagent> ingredients;
vector<reagent> products;
:(before "End recipe Constructor")
has_header = false;

:(before "End recipe Refinements")
skip_whitespace(in);
if (in.peek() != '[') {
  trace(9999, "parse") << "recipe has a header; parsing" << end();
  load_recipe_header(in, result);
}

:(code)
void load_recipe_header(istream& in, recipe& result) {
  result.has_header = true;
  while (in.peek() != '[') {
    string s = next_word(in);
    if (s == "->") break;
    result.ingredients.push_back(reagent(s));
    trace(9999, "parse") << "header ingredient: " << result.ingredients.back().original_string << end();
    skip_whitespace(in);
  }
  while (in.peek() != '[') {
    string s = next_word(in);
    result.products.push_back(reagent(s));
    trace(9999, "parse") << "header product: " << result.products.back().original_string << end();
    skip_whitespace(in);
  }
  // End Load Recipe Header(result)
}

//: Now rewrite 'load-ingredients' to instructions to create all reagents in
//: the header.

:(before "End Rewrite Instruction(curr, recipe result)")
if (curr.name == "load-ingredients") {
  curr.clear();
  for (long long int i = 0; i < SIZE(result.ingredients); ++i) {
    curr.operation = Recipe_ordinal["next-ingredient"];
    curr.name = "next-ingredient";
    curr.products.push_back(result.ingredients.at(i));
    result.steps.push_back(curr);
    curr.clear();
  }
}

:(scenarios transform)
:(scenario recipe_headers_are_checked)
% Hide_errors = true;
recipe add2 x:number, y:number -> z:number [
  local-scope
  load-ingredients
  z:address:number <- copy 0/raw
  reply z
]
+error: add2: replied with the wrong type at 'reply z'

:(before "End One-time Setup")
  Transform.push_back(check_header_products);

:(code)
void check_header_products(const recipe_ordinal r) {
  const recipe& rr = Recipe[r];
  if (rr.products.empty()) return;
  trace(9991, "transform") << "--- checking reply instructions against header for " << rr.name << end();
  for (long long int i = 0; i < SIZE(rr.steps); ++i) {
    const instruction& inst = rr.steps.at(i);
    if (inst.operation != REPLY) continue;
    if (SIZE(rr.products) != SIZE(inst.ingredients)) {
      raise_error << maybe(rr.name) << "tried to reply the wrong number of products in '" << inst.to_string() << "'\n" << end();
    }
    for (long long int i = 0; i < SIZE(rr.products); ++i) {
      if (!types_match(rr.products.at(i), inst.ingredients.at(i))) {
        raise_error << maybe(rr.name) << "replied with the wrong type at '" << inst.to_string() << "'\n" << end();
      }
    }
  }
}

//: Deduce types from the header if possible.

:(scenarios run)
:(scenario deduce_instruction_types_from_recipe_header)
recipe main [
  1:number/raw <- add2 3, 5
]
recipe add2 x:number, y:number -> z:number [
  local-scope
  load-ingredients
  z <- add x, y  # no type for z
  reply z
]
+mem: storing 8 in location 1

:(before "Transform.push_back(transform_names)")
  Transform.push_back(deduce_types_from_header);

:(code)
void deduce_types_from_header(const recipe_ordinal r) {
  recipe& rr = Recipe[r];
  if (rr.products.empty()) return;
  trace(9991, "transform") << "--- deducing types from header for " << rr.name << end();
  map<string, const type_tree*> header;
  for (long long int i = 0; i < SIZE(rr.ingredients); ++i) {
    header[rr.ingredients.at(i).name] = rr.ingredients.at(i).type;
  }
  for (long long int i = 0; i < SIZE(rr.products); ++i) {
    header[rr.products.at(i).name] = rr.products.at(i).type;
  }
  for (long long int i = 0; i < SIZE(rr.steps); ++i) {
    instruction& inst = rr.steps.at(i);
    trace(9993, "transform") << inst.to_string() << '\n';
    for (long long int i = 0; i < SIZE(inst.ingredients); ++i) {
      if (inst.ingredients.at(i).type) continue;
      inst.ingredients.at(i).type = new type_tree(*header[inst.ingredients.at(i).name]);
    }
    for (long long int i = 0; i < SIZE(inst.products); ++i) {
      if (inst.products.at(i).type) continue;
      inst.products.at(i).type = new type_tree(*header[inst.products.at(i).name]);
      trace(9993, "transform") << "type of " << inst.products.at(i).name << " is " << dump_types(inst.products.at(i)) << '\n';
    }
  }
}

//: One final convenience: no need to say what to return if the information is
//: in the header.

:(scenario reply_based_on_header)
recipe main [
  1:number/raw <- add2 3, 5
]
recipe add2 x:number, y:number -> z:number [
  local-scope
  load-ingredients
  z <- add x, y
  reply
]
+mem: storing 8 in location 1

:(before "End Rewrite Instruction(curr, recipe result)")
if (curr.name == "reply" && curr.ingredients.empty()) {
  for (long long int i = 0; i < SIZE(result.products); ++i) {
    curr.ingredients.push_back(result.products.at(i));
  }
}

:(scenario reply_on_fallthrough_based_on_header)
recipe main [
  1:number/raw <- add2 3, 5
]
recipe add2 x:number, y:number -> z:number [
  local-scope
  load-ingredients
  z <- add x, y
]
+mem: storing 8 in location 1

:(after "int main")
  Transform.push_back(deduce_fallthrough_reply);

:(code)
void deduce_fallthrough_reply(const recipe_ordinal r) {
  recipe& rr = Recipe[r];
  if (rr.products.empty()) return;
  if (rr.steps.empty()) return;
  if (rr.steps.at(SIZE(rr.steps)-1).operation != REPLY) {
    instruction inst;
    inst.operation = REPLY;
    inst.name = "reply";
    for (long long int i = 0; i < SIZE(rr.products); ++i) {
      inst.ingredients.push_back(rr.products.at(i));
    }
    rr.steps.push_back(inst);
  }
}