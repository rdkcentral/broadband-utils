set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "=== Building libfwbpi ==="

# Parse arguments
BUILD_MODE="release"
CLEAN_FIRST="no"
RUN_TESTS="no"
INSTALL="no"

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_MODE="debug"
            shift
            ;;
        --clean)
            CLEAN_FIRST="yes"
            shift
            ;;
        --test)
            RUN_TESTS="yes"
            shift
            ;;
        --install)
            INSTALL="yes"
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug    Build debug version"
            echo "  --clean    Clean before building"
            echo "  --test     Run tests after building"
            echo "  --install  Install after building"
            echo "  --help     Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Clean if requested
if [[ "$CLEAN_FIRST" == "yes" ]]; then
    echo "Cleaning..."
    make distclean
fi

# Build
echo "Building (mode: $BUILD_MODE)..."
make BUILD_MODE="$BUILD_MODE" all

# Run tests if requested
if [[ "$RUN_TESTS" == "yes" ]]; then
    echo "Running tests..."
    make BUILD_MODE="$BUILD_MODE" check
fi

# Install if requested
if [[ "$INSTALL" == "yes" ]]; then
    echo "Installing..."
    sudo make BUILD_MODE="$BUILD_MODE" install
fi

echo "Build complete!"
