# [Beginning Game Programming v2.0](https://lazyfoo.net/tutorials/SDL/)

This project is a re-do of the first programming tutorial that I did some years ago by myself after learning C in college.\
I never finished the turial properly and been postponing learning git and using GitHub for some time.\
The goal is to rethink my workflow, and apply some standards of coding that are somewhat unusual for a project of this type.

---
## Links

- [Beginning Game Programming v2.0](https://lazyfoo.net/tutorials/SDL/)
- [The Power of Ten – Rules for Developing Safety Critical Code](https://spinroot.com/gerard/pdf/P10.pdf)
- [Markdown - Basic writing and formatting syntax](https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax)
- [GCC Command Option Summary](https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html)
- [Embedding of binary data into programs](https://www.devever.net/~hl/incbin)


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
## Setting up Git and  GitHub

### The initial setup in CLI

The process is simple but not straight foward as i'm not used to it, this section is just a quick reference for myself of what commands that were used to setup this project.

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

### Proper GitHub workflow
   
#### 1. Creating branch locally
```bash
# Make sure it's up to date with main
git checkout main
git pull origin main

# Create and switch to a new branch
git checkout -b feature/branch-name
```

#### 2. Work, diff and commit

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

#### 3. Create the Pull Request
- Go to your repository on GitHub.
- GitHub will show a banner saying "Compare & pull request."
- Click it --> review your changes --> click "Create pull request."
- Once satisfied, click "Merge pull request."

#### 4. Clean up
```bash
# Local Files clean up
git checkout main
git pull

# Deleting branch
git branch -d feature/branch-name
git push origin --delete feature/branch-name
```

### Other GitHub References

#### Discard local changes and reset branch
```bash
git fetch origin
git reset --hard origin/main

```

### View branches, blame, history

#### Branch lists

```bash
# local branches
git branch

# remote branches (on GitHub)
git branch -r

# all (local + remote)
git branch -a

# show tracking/upstream info
git branch -vv

# refresh and prune deleted remote branches
git fetch --prune
```

#### File blame / who changed what

```bash
git blame path/to/file.c
# ignore whitespace-only changes:
git blame -w path/to/file.c
# blame a specific range:
git blame -L 100,160 path/to/file.c
```

#### History browsing

```bash
# compact, decorated commit graph
git log --oneline --graph --decorate --all

# show patch for the last commit
git show

# compare two commits/branches
git diff commit1 commit2
git diff main feature/branch-name

# see your local HEAD movements
git reflog
```
