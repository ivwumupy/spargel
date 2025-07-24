find ./source/ -iname '*.h' -o -iname '*.cpp' -o -iname '*.mm' | xargs clang-format -i
