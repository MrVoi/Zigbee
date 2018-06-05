# Source code : https://thachpham.com/tools/cach-tao-repository-cho-git.html
# 1. Cau hinh va tao kho chua ban dau tren may
alias config_git = 'git config --global user.name "MrVoi" &&  git config --global user.email "41300885@hcmut.edu.com" && cat ~/.gitconfig'

#Chu y: truoc het vo trong thu muc mong muon lam kho chua roi moi go
alias local_repository = 'git init && git add * && git status'
alias commit = 'git commit -m'
# 2. log va undo commit:
alias undo_commit = 'git commit --amend -m'
alias git_log = 'git log && git config --global alias.lg "log --color --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit"'
alias stagingarea_out = 'git reset HEAD'
# 3. Tag: 
#3.1. Lightweight Tag: Neu go git tag se hien ra danh sach cac tag co trong du an, them ten tag vao sau la danh dau mot tag moi

alias lighttag = 'git tag'
alias commit_tag = 'git show'

#3.2. Annotated Tag:
alias annotate_tag = 'git tag -a v1.2 -m "message"'
alias push_tags = 'git push --tags'


# 4. Kho chua tren server, truoc het clone mot kho chua nao do tren mang ve may, them moi cai gi do thi vo thu muc do go
alias add = 'git add * && git commit -"Newly"'
alias push = 'git push origin master'
alias config_rep = 'git config --global push.default simple'