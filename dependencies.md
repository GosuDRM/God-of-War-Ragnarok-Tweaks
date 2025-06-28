# External Dependencies

This project uses the following external libraries:

- **imgui** @ `ea0da0b`
  - Repository: https://github.com/ocornut/imgui.git
  - Used for GUI components

- **inipp** @ `c61e699`  
  - Repository: https://github.com/mcmtroffaes/inipp.git
  - Used for INI file parsing

## Setup Instructions

To set up the dependencies for local development:

```bash
# Add submodules
git submodule add https://github.com/ocornut/imgui.git external/imgui
git submodule add https://github.com/mcmtroffaes/inipp.git external/inipp

# Checkout specific commits
cd external/imgui && git checkout ea0da0b
cd ../inipp && git checkout c61e699
cd ../..

# Commit the submodule references
git add .gitmodules external/
git commit -m "Add external dependencies as submodules"
