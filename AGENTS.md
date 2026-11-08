# Programming Language Policy

1. Use Ruby as the default language for scripting, automation, file manipulation, API integrations, and general-purpose utility tasks.

2. Prefer Ruby's standard library wherever practical. Avoid introducing external dependencies unless they provide meaningful benefits.

3. Use Python when:
- A task requires specialised Python libraries.
- Machine learning, data science, or scientific computing is involved.
- An existing Python-based tool or framework is the most appropriate solution.
- Python offers a significant practical advantage over Ruby.

4. Before introducing a new dependency, check whether the task can be accomplished using existing system tools or standard libraries.

5. Prefer simple, readable, maintainable scripts over unnecessarily complex implementations.

6. Do not rewrite existing projects in Ruby simply to comply with this preference. Respect the language and architecture already used by the project.

7. Choose the language that best accomplishes the task when there is a compelling technical reason to deviate from the default.

# Tools Policy

You are free to add any tools that could help development, as long as

1. It isn't a security concern (shady source / potential malware)
2. It doesn't need to be installed system wide and it can be added with current user permission
    - by using nix, and through a a nix-shell
    - using user flatpack

For any other use case, when a tool would be usefull but require admin permission
- ask for user help
- justify how it would help the work that need to be done.

# Cpp Code rules

## Convention
- class and struct type are in CamelCase starting with a uppercase: class ThisIsAClass; struct ThisIsAStruct;
local variable start with lowercase: ThisIsAClass myObject;

function and member function start lowercase.

object member variable for struct and class ends with "_"

## Files
project header are in .hh
project source are in .cc

## Project

Use meson as a build system.

## Modern C++

If compiling with c++20 or latter, use std::format
If compiling with c++23 use std::print when applicable 

## dependencies

Use [nlohmann/json](https://github.com/nlohmann/json)  - single-header for json serialization
When using meson use it as [ meson subpackage ⮺](https://github.com/yduf/covered/blob/master/subprojects/nlohmann_json.wrap)

## Unit Test
for unit test use doctest

It is available as #include "myFunc/doctest.h"
with myFunc located at /home/yves/DEV/cpp/myFunc
