#!/bin/bash
# Git pre-commit hook: Check file encoding and line endings
# Installation: Run .\tools\debug\windows\install_git_hooks.ps1

echo "Checking file encoding and line endings..."

# File extensions to check
EXTENSIONS="cpp|h|hpp|cc|cxx|hxx|c|inl|lua|cmake|proto|json|yaml|yml|md"

# Get files to be committed (exclude files in .gitignore)
FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E "\\.($EXTENSIONS)$")

if [ -z "$FILES" ]; then
    echo "OK: No files to check"
    exit 0
fi

HAS_ERROR=0

for FILE in $FILES; do
    # Skip if file doesn't exist
    if [ ! -f "$FILE" ]; then
        continue
    fi
    
    # Skip if file is in .gitignore
    if git check-ignore -q "$FILE"; then
        continue
    fi
    
    # Check for BOM
    if file "$FILE" | grep -q "UTF-8 Unicode (with BOM)"; then
        echo "ERROR: $FILE contains BOM, use UTF-8 without BOM"
        HAS_ERROR=1
    fi
    
    # Check for non-UTF-8 encoding
    if ! file "$FILE" | grep -qE "UTF-8|ASCII"; then
        echo "ERROR: $FILE is not UTF-8 encoded"
        HAS_ERROR=1
    fi
    
    # Check for CRLF (Windows line endings)
    if file "$FILE" | grep -q "CRLF"; then
        echo "ERROR: $FILE uses CRLF line endings, should use LF"
        HAS_ERROR=1
    fi
done

if [ $HAS_ERROR -eq 1 ]; then
    echo ""
    echo "COMMIT REJECTED! Please fix the encoding or line ending issues above."
    echo ""
    echo "How to fix:"
    echo "  1. Windows: Run .\\tools\\debug\\windows\\fix_encoding_and_lineendings.ps1"
    echo "  2. Linux:   Run bash tools/debug/linux/fix_encoding_and_lineendings.sh"
    echo "  3. VS Code: Click 'CRLF' in status bar -> Select 'LF'"
    echo "              Click 'UTF-8 with BOM' -> Select 'UTF-8'"
    echo ""
    exit 1
fi

echo "OK: All files passed encoding and line ending checks"
exit 0
