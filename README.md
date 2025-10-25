# [Beginning Game Programming v2.0](https://lazyfoo.net/tutorials/SDL/)

This project is an re-do of the first programming tutorial that I did some years ago by myself after learning C in college.  
  
I never finished the turial properly and been postponing learning git and using GitHub for some time.

The goal is to rethink my workflow, and apply some standards of coding that are somewhat unusual for a project of this type.

---

## Setting up Git and  GitHub

The process is simple but not straight foward, this section is just a quick reference for myself of what commands that were used to setup this project.

1. The initial setup in CLI:
    
    This was done after creating the repo in GitHub.
    ```
    git config --global user.name "User"
    git config --global user.email "user@email.com"
    
    gh auth login
    git init
    add README.md
    commit -m "first commit"
    branch -M main
    git remote add origin https://github.com/duiliokepel/lazyfoo-sdl-tutorial.git
    git push -u origin main
    ```

---

## Proper GitHub workflow
   
1. Creating branch locally:
 
    ```
    # Make sure you're up to date with main
    git checkout main
    git pull origin main
    
    # Create and switch to a new branch
    git checkout -b feature/setup-sdl
    ```

2. Work, diff and commit
   ```
    # See unstaged changes (what you’ve modified but not yet added)
    git --no-pager diff
   
    # Stage for Commit
    git add .
    
    # See staged changes (what will be committed)
    git --no-pager diff --cached
    git --no-pager diff --staged
    
    # Commit
    git commit -m "Add SDL initialization and window code"
 
   ```


3. Push the branch to GitHub
    ```
    git push -u origin feature/setup-sdl
    ```

4. Create the Pull Request

    - Go to your repository on GitHub.
    - GitHub will show a banner saying “Compare & pull request.”
    - Click it --> review your changes --> click “Create pull request.”
    - Once satisfied, click “Merge pull request.”

5. Clean up
    ```
    git checkout main
    git pull
    git branch -d feature/setup-sdl
    git push origin --delete feature/setup-sdl
    ```