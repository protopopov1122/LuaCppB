#include <cstdlib>

void function_examples(int, const char **);
void object_examples(int, const char **);

int main(int argc, const char **argv) {
  function_examples(argc, argv);
  object_examples(argc, argv);
  return EXIT_SUCCESS;
}