# Custom Kernel Modules
IMAGE_INSTALL += "\
    matrix-keypad-module \
"

# Stable /dev/input/keypad symlink for the matrix keypad
IMAGE_INSTALL += "\
    linht-input-rules \
"
