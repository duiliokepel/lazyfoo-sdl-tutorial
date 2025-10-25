# [Beginning Game Programming v2.0](https://lazyfoo.net/tutorials/SDL/)

This project is a re-do of the first programming tutorial that I did some years ago by myself after learning C in college. I never finished the turial properly and been postponing learning git and using GitHub for some time. The goal is to rethink my workflow, and apply some standards of coding that are somewhat unusual for a project of this type.

---

## Setting up Git and  GitHub

### The initial setup in CLI

The process is simple but not straight foward, this section is just a quick reference for myself of what commands that were used to setup this project.
    
After creating the repo in GitHub:
```
git config --global user.name "User"
git config --global user.email "user@email.com"

gh auth login
git init
```

Create README and make the first commit:
```
echo "# lazyfoo-sdl-tutorial" > README.md
git add README.md
git commit -m "first commit"
```

Set default branch name and connect to GitHub:
```
git branch -M main
git remote add origin https://github.com/duiliokepel/lazyfoo-sdl-tutorial.git
git push -u origin main
```

---

### Proper GitHub workflow
   
#### 1. Creating branch locally
- Make sure it's up to date with main
```
git checkout main
git pull origin main
```
- Create and switch to a new branch
```
git checkout -b feature/branch-name
```

#### 2. Work, diff and commit
- See unstaged changes (what you’ve modified but not yet added)
```
git --no-pager diff
```
- Stage for Commit
``` 
git add .
```
- See staged changes (what will be committed)
```
git --no-pager diff --cached
git --no-pager diff --staged
```
- Commit
```
git commit
git commit -m "Commit Message"
```

#### 3. Push the branch to GitHub
```
git push -u origin feature/branch-name
```

#### 4. Create the Pull Request
- Go to your repository on GitHub.
- GitHub will show a banner saying "Compare & pull request."
- Click it --> review your changes --> click "Create pull request."
- Once satisfied, click "Merge pull request."

#### 5. Local files clean up
```
git checkout main
git pull
```

#### 6. Deleting branch

```
git branch -d feature/branch-name
git push origin --delete feature/branch-name
```

### Other GitHub References

#### Discard local changes and reset branch
```
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
