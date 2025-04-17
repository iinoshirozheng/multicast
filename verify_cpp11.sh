#!/bin/bash
# Script to verify C++11 compatibility across the codebase
# Usage: ./verify_cpp11.sh [directory]

set -e

# Default directory is stream_buffer if not specified
CHECK_DIR=${1:-"stream_buffer"}

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Checking C++11 compatibility in ${CHECK_DIR}...${NC}"

# Common C++14/17/20 features to check for
CPP_FEATURES=(
  "auto[[:space:]]*\["        # C++14 return type deduction
  "\[\[nodiscard\]\]"         # C++17 attribute with better pattern to avoid false positives
  "constexpr[[:space:]]*if"   # C++17 constexpr if
  "std::optional"             # C++17 optional
  "std::variant"              # C++17 variant
  "std::any"                  # C++17 any
  "std::string_view"          # C++17 string_view
  "std::filesystem"           # C++17 filesystem
  "<execution>"               # C++17 parallel algorithms
  "std::invoke"               # C++17 invoke
  "\[\[fallthrough\]\]"       # C++17 attributes
  "\[\[maybe_unused\]\]"      # C++17 attributes
  "std::byte"                 # C++17 byte
  "template[[:space:]]*auto"  # C++17 template auto
  "if[[:space:]]*constexpr"   # C++17 if constexpr
  "structured[[:space:]]*binding" # C++17 structured bindings
  "<ranges>"                  # C++20 ranges
  "concept"                   # C++20 concepts
  "co_await"                  # C++20 coroutines
  "three-way"                 # C++20 spaceship operator
  "<span>"                    # C++20 span
  "<format>"                  # C++20 format
)

# Files to check
CPP_FILES=$(find "${CHECK_DIR}" -type f -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | sort)

# GCC binary to use, preferring GCC 11 if available
if command -v g++-11 &> /dev/null; then
    GXX="g++-11"
else
    GXX="g++"
fi

# Check if we're in CentOS with devtoolset-11
if [ -f /etc/centos-release ] && [ -f /opt/rh/devtoolset-11/enable ]; then
    echo "CentOS detected with devtoolset-11, sourcing environment"
    source /opt/rh/devtoolset-11/enable
fi

# Report GCC version
echo -e "Using compiler: ${YELLOW}$(${GXX} --version | head -n 1)${NC}"

# Check if make target exists for cpp11-check
if [ -f "${CHECK_DIR}/Makefile" ] && grep -q "cpp11-check:" "${CHECK_DIR}/Makefile"; then
    echo -e "${GREEN}Found cpp11-check target in Makefile, running it...${NC}"
    (cd "${CHECK_DIR}" && make cpp11-check)
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Makefile cpp11-check passed!${NC}"
    else
        echo -e "${RED}Makefile cpp11-check failed!${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}No cpp11-check target found in Makefile, performing manual checks...${NC}"
fi

# Check for post-C++11 features in code
ISSUES_FOUND=0

echo -e "\n${YELLOW}Scanning for C++14/17/20 features in code:${NC}"
for feature in "${CPP_FEATURES[@]}"; do
    # Use -P for Perl regex, more accurate matching
    MATCHES=$(grep -P -r "${feature}" --include="*.cpp" --include="*.h" --include="*.hpp" "${CHECK_DIR}" 2>/dev/null || true)
    if [ -n "$MATCHES" ]; then
        echo -e "${RED}Found potential C++14/17/20 feature '${feature}':${NC}"
        echo "$MATCHES" | head -n 5  # Show at most 5 matches
        TOTAL=$(echo "$MATCHES" | wc -l)
        if [ $TOTAL -gt 5 ]; then
            echo -e "${YELLOW}...and $(($TOTAL - 5)) more occurrences${NC}"
        fi
        echo ""
        ISSUES_FOUND=1
    fi
done

# Try to compile each file with strict C++11 mode
echo -e "\n${YELLOW}Compiling files with strict C++11 mode:${NC}"

COMPILE_FAILURES=0
for file in $CPP_FILES; do
    echo -n "Checking $file ... "
    # Only compile .cpp files directly
    if [[ "$file" == *.cpp ]]; then
        INCLUDE_DIR="${CHECK_DIR}/include"
        if [ ! -d "$INCLUDE_DIR" ]; then
            # If include directory doesn't exist at the expected location, try to find it
            INCLUDE_DIR=$(dirname "$file")
            while [ "$INCLUDE_DIR" != "." ] && [ "$INCLUDE_DIR" != "/" ]; do
                if [ -d "$INCLUDE_DIR/include" ]; then
                    INCLUDE_DIR="$INCLUDE_DIR/include"
                    break
                fi
                INCLUDE_DIR=$(dirname "$INCLUDE_DIR")
            done
            # If still not found, use the current directory
            if [ "$INCLUDE_DIR" = "." ] || [ "$INCLUDE_DIR" = "/" ]; then
                INCLUDE_DIR=$(dirname "$file")
            fi
        fi
        
        COMPILE_OUTPUT=$(${GXX} -std=c++11 -pedantic-errors -Werror -fsyntax-only -I"$INCLUDE_DIR" "$file" 2>&1 || true)
        if [ $? -ne 0 ]; then
            echo -e "${RED}FAILED${NC}"
            echo "$COMPILE_OUTPUT" | head -n 5
            if [ $(echo "$COMPILE_OUTPUT" | wc -l) -gt 5 ]; then
                echo "..."
            fi
            COMPILE_FAILURES=$((COMPILE_FAILURES + 1))
        else
            echo -e "${GREEN}PASSED${NC}"
        fi
    else
        echo -e "${YELLOW}SKIPPED (not a .cpp file)${NC}"
    fi
done

# Summary
echo -e "\n${YELLOW}Summary:${NC}"

if [ $ISSUES_FOUND -eq 0 ] && [ $COMPILE_FAILURES -eq 0 ]; then
    echo -e "${GREEN}All checks passed! Your code appears to be C++11 compatible.${NC}"
    EXIT_CODE=0
else
    if [ $COMPILE_FAILURES -gt 0 ]; then
        echo -e "${RED}Found $COMPILE_FAILURES compilation failures.${NC}"
    fi
    
    if [ $ISSUES_FOUND -eq 1 ]; then
        echo -e "${RED}Detected potential usage of C++14/17/20 features.${NC}"
        echo -e "${YELLOW}Please review the issues above and fix them for C++11 compatibility.${NC}"
    fi
    
    # If we have compiler errors but no detected features, it's likely a genuine issue
    if [ $COMPILE_FAILURES -gt 0 ]; then
        EXIT_CODE=1
    # If we only have detected features but all files compile, it might be a false positive
    elif [ $ISSUES_FOUND -eq 1 ] && [ $COMPILE_FAILURES -eq 0 ]; then
        echo -e "${YELLOW}Note: All files compile successfully with C++11, so detected features might be false positives.${NC}"
        EXIT_CODE=0
    else
        EXIT_CODE=1
    fi
fi

echo -e "\n${GREEN}You can also run unit tests to ensure functionality is preserved:${NC}"
echo "cd ${CHECK_DIR} && make test"

exit $EXIT_CODE 