# Contributing to Stream Buffer

Thank you for considering contributing to Stream Buffer! This document provides guidelines and instructions for contributing to this project.

## Code of Conduct

By participating in this project, you agree to follow our Code of Conduct. Please be respectful, considerate, and constructive in all interactions.

## How to Contribute

### Reporting Bugs

If you find a bug, please create an issue with a clear description of:
- What the bug is
- Steps to reproduce the bug
- Expected behavior
- Actual behavior
- Your environment (OS, compiler version, etc.)

### Suggesting Enhancements

For feature requests, please create an issue with:
- A clear description of the feature
- The motivation for the feature
- Possible implementations or approaches

### Pull Requests

1. Fork the repository
2. Create a branch for your feature or bugfix (`git checkout -b feature/your-feature` or `git checkout -b fix/your-bugfix`)
3. Make your changes
4. Write tests for your changes
5. Run tests and ensure all pass
6. Commit your changes with descriptive commit messages
7. Push to your branch (`git push origin feature/your-feature`)
8. Open a Pull Request against the main branch

## Development Guidelines

### Coding Style

- Follow the C++ programming guidelines in the project documentation
- Use consistent indentation (4 spaces, no tabs)
- Keep functions small and focused on a single task
- Write self-documenting code with clear variable and function names
- Add comments for complex logic or algorithms

### Documentation

- Document all public APIs using Doxygen-style comments
- Keep the documentation up-to-date with code changes
- Include usage examples where appropriate

### Testing

- Write unit tests for all new functionality
- Ensure all tests pass before submitting a pull request
- Consider edge cases and error conditions

### Performance Considerations

- Be mindful of memory usage
- Avoid unnecessary allocations, especially in critical paths
- Consider thread safety implications of your changes

## Review Process

All submissions require review. We use GitHub pull requests for this purpose:

1. A maintainer will review your PR
2. Changes may be requested
3. Once approved, a maintainer will merge your PR

## Getting Help

If you need help with your contribution, feel free to:
- Ask questions in the issue you're working on
- Reach out to project maintainers

Thank you for your contributions! 