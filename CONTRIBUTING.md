# Contributing to Haze

Thank you for your interest in contributing to Haze.

Contributions are welcome as long as they remain consistent with the project's architecture, goals, and development standards.

## Getting Started

### Building Haze

To build Haze, use the build script provided for your platform:

```
cd eng
go run build.go
``` 

Make sure your development environment provides:

* Git
* CMake >= 3.20
* A C11-compatible compiler:

  * GCC
  * Clang
  * MSVC
  * MinGW

After a successful build, the resulting Haze binary is named `hazec`.

### Understanding the Build System

If you already have a basic understanding of software builds and CMake, please read the project's [CMakeLists.txt](./CMakeLists.txt) before making changes to the build system.

This will help you understand how Haze is configured, compiled, and linked.

## Project Structure

The source code is organized into modules under the [source/](./source) directory.

Each directory generally represents a module of the project. Related `.c` and `.h` files are kept together, and modules may contain submodules through nested directories.

When adding new functionality, place it in the module responsible for that functionality.

Avoid placing unrelated functionality into existing modules simply because they are convenient locations.

## Code Style

Haze does not enforce a formal indentation standard. Use modern, consistent C formatting and follow the style of the surrounding code.

### Naming

The project uses the following naming conventions:

* Functions: `PascalCase`
* Types and other declarations: `PascalCase`
* Local variables: `camelCase`

For example:

```c
Channel *ChannelCreate(void);

Channel *channel = ChannelCreate();
int channelCount = 0;
```

## Git Workflow

Create a separate branch for each contribution.

Use descriptive branch names such as:

* `feat/...`
* `fix/...`
* `refactor/...`
* `docs/...`
* `test/...`

Keep each branch focused on a single change or closely related set of changes.

## Commit Messages

Use the following prefixes for commit messages:

* `add(<platform>):` — new functionality
* `fix(<platform>):` — bug fixes
* `refactor(<platform>):` — code restructuring
* `docs(<platform>):` — documentation changes
* `test(<platform>):` — tests
* `build(<platform>):` — build-system changes

For example:

```text
add: channel rename
```

Keep commit messages concise and descriptive.

## Testing

Before submitting a contribution, make sure that:

1. Haze builds successfully on the target platform.
2. Existing functionality has not been unnecessarily broken.
3. Relevant tests are passing.
4. New functionality is tested when appropriate.

## Pull Requests

Before opening a Pull Request:

1. Ensure the project builds successfully.
2. Run the relevant tests.
3. Keep the changes focused.
4. Explain what was changed and why.
5. Mention any relevant architectural considerations.

Pull Requests are reviewed according to the project's architecture, coding standards, and overall goals.

Changes may be requested before a Pull Request is merged.

## Bug Reports

When reporting a bug, provide enough information to reproduce it.

Include, when relevant:

* Operating system
* Compiler and version
* CMake version
* Steps to reproduce the problem
* Expected behavior
* Actual behavior
* Relevant logs or error messages

## Feature Requests

Feature requests should explain:

* What problem the feature solves
* Why it belongs in Haze
* How it could fit into the existing architecture

Large architectural changes should be discussed before implementation whenever possible.

## Final Notes

Contributions should prioritize correctness, maintainability, and consistency with the existing architecture.

When in doubt, inspect the existing implementation and follow established patterns before introducing a new one.
