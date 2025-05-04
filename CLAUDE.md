# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands
- Build parser: `make`
- Run tests: `make test`
- Test with tokens: `make tokens`
- Test single file: `./smalltalk_parser path/to/file.st`
- Display tokens: `./smalltalk_parser --tokens path/to/file.st`
- Clean build: `make clean`

## Code Style Guidelines
- Indentation: 4 spaces, no tabs
- Function naming: camelCase (e.g., `parserError`, `nextToken`)
- Variable naming: camelCase for variables, ALL_CAPS for token types
- Error handling: Use dedicated error functions (`lexerError`, `parserError`)
- Memory management: Free allocated memory, check malloc return values
- Comments: Use /* */ for multiline and // for single-line comments
- Includes: Group standard library includes first, then project headers
- Function structure: Short functions with clear purposes
- Debug messaging: Use printf statements for debugging token processing