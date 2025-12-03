# [Beginning Game Programming v2.0](https://lazyfoo.net/tutorials/SDL/)
---
## Table of Contents

1. [Introduction](#introduction)
2. [Project Goals](#project-goals)
3. [Project Structure](#project-structure)
5. [GitHub and Git setup](#github-and-git-setup)
6. [GitHub workflow](#github-workflow)
7. [References](#references)

---
## Introduction

This project is a re-do of the first programming tutorial that I did some years ago after learning C in college.\
The first time I tried, the tutorial wasn't finished completely, there was no version control and I didn't had much experience in what works for coding standards.\
Since the tutorial is straight foward and it's main purpose is to demonstrate SDL's functionality, the majority of work here is about setting up Git, GitHub, and automating both the compilation process and coding guidelines compliance.

---
## Project Goals

- [ ] Make all 55 exercises from Lazy Foo's SDL tutorial in C;
- [ ] All assets should be embedded into the executables;
- [ ] Comply with NASA's "Power of Ten" coding guidelines using tools and scripts;
- [ ] Compile from Linux, cross-compile to Windows and Android;
- [ ] Compile from Windows, cross-compile to Linux and Android;

---
## Todo:

Automating Power of 10 Rules:
- [ ] Rule 01 - Restrict all code to very simple control flow constructs
- [ ] Rule 02 - All loops must have a fixed upper-bound.
- [ ] Rule 03 - Do not use dynamic memory allocation after initialization
- [ ] Rule 04 - No function should be longer than what can be printed on a single sheet of
paper
- [ ] Rule 05 - The assertion density of the code should average to a minimum of two
assertions per function
- [ ] Rule 06 - Data objects must be declared at the smallest possible level of scope.
- [ ] Rule 07 - The return value of non-void functions must be checked by each calling
function, and the validity of parameters must be checked inside each function.
- [ ] Rule 08 - The use of the preprocessor must be limited to the inclusion of header files and
simple macro definitions.
- [ ] Rule 09 - The use of pointers should be restricted. Specifically, no more than one level of
dereferencing is allowed.
- [ ] Rule 10 - All code must be compiled, from the first day of development, with all
compiler warnings enabled at the compiler’s most pedantic setting.


Tutorial Development:
- [ ] SDL Tutorial 01 - Hello SDL
- [ ] SDL Tutorial 02 - Getting an Image on the Screen
- [ ] SDL Tutorial 03 - Event Driven Programming
- [ ] SDL Tutorial 04 - Key Presses
- [ ] SDL Tutorial 05 - Optimized Surface Loading and Soft Stretching
- [ ] SDL Tutorial 06 - Extension Libraries and Loading Other Image Formats
- [ ] SDL Tutorial 07 - Texture Loading and Rendering
- [ ] SDL Tutorial 08 - Geometry Rendering
- [ ] SDL Tutorial 09 - The Viewport
- [ ] SDL Tutorial 10 - Color Keying



---
## Project Structure

### Source directories

- [`src/`](src/): contains all C sources files for multiple executables
- [`assets/`](assets/): contains files, images, fonts and audio

### Scripts

- [`README.md`](README.md): this readme.
- [`makefile`](makefile): single makefile for the entire project.
- [`embed.sh`](embed.sh): script to generate binary blobs and header files.

### Generated directories and files

- [`bin/`](bin/): all the linked executables and shared libs.
- [`build/`](build/): objects from compilation.
- [`embed/`](embed/): binary blobs and header files.
- [`deps/`](deps/): generated dependencies.
- [`include/`](include/): generated headers for shared libs.

---
## GitHub and Git setup

After creating the repo in GitHub:
```bash
git config --global user.name "User"
git config --global user.email "user@email.com"
git config --global core.editor "vim"

gh auth login
git init
```

Create README and make the first commit:
```bash
echo "# lazyfoo-sdl-tutorial" > README.md
git add README.md
git commit -m "first commit"
```

Set default branch name and connect to GitHub:
```bash
git branch -M main
git remote add origin https://github.com/duiliokepel/lazyfoo-sdl-tutorial.git
git push -u origin main
```

---
## GitHub workflow
   
### 1. Creating branch locally
```bash
# Make sure it's up to date with main
git checkout main
git pull origin main

# Create and switch to a new branch
git checkout -b feature/branch-name
```

### 2. Work, diff and commit
```bash
# See unstaged changes (what you’ve modified but not yet added)
git --no-pager diff

# Stage for Commit
git add .

# See staged changes (what will be committed)
git --no-pager diff --cached
git --no-pager diff --staged

# Commit
git commit
git commit -m "Commit Message"

# Push the branch to GitHub
git push
git push -u origin feature/branch-name
```

### 3. Create the Pull Request
- Go to your repository on GitHub.
- GitHub will show a banner saying "Compare & pull request."
- Click it --> review your changes --> click "Create pull request."
- Once satisfied, click "Merge pull request."

### 4. Clean up
```bash
# Local Files clean up
git checkout main
git pull

# Deleting branch
git branch -d feature/branch-name
git push origin --delete feature/branch-name
```

---
## References

- [Beginning Game Programming v2.0](https://lazyfoo.net/tutorials/SDL/)
- [The Power of Ten – Rules for Developing Safety Critical Code](https://spinroot.com/gerard/pdf/P10.pdf)
- [Markdown - Basic writing and formatting syntax](https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax)
- [GCC Command Option Summary](https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html)
- [Embedding of binary data into programs](https://www.devever.net/~hl/incbin)

