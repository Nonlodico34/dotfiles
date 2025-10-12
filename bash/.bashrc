# ~/.bashrc: executed by bash(1) for non-login shells.
# see /usr/share/doc/bash/examples/startup-files (in the package bash-doc)
# for examples

# Settings
export EDITOR="micro"
export VISUAL="micro"
eval "$(zoxide init bash)"

alias cranger='tmpfile=$(mktemp); ranger --choosedir="$tmpfile"; cd "$(cat $tmpfile)"; rm -f $tmpfile'
alias bat='batcat --paging=never'
alias wiztree=ncdu
alias bashrc="micro ~/.bashrc ; source ~/.bashrc"
alias fd=fdfind
alias sudo='sudo '
alias apt='nala'
alias l='eza --icons=always --sort=type -l'
alias clock='tty-clock -S -s -c -C 5 -f "%d-%m-%Y"'

# Startup position
LC_TIME=it_IT.UTF-8 tmux
cd /home/lukk
clear

# Macro
cpp() {
    file="$1"
    exe="${file%.cpp}"
    g++ "$file" -o "$exe" && "./$exe"
}

run(){
	file="$1"
    exe="${file%.cpp}"
    g++ "$file" -o "$exe" && "./$exe"
    rm "./$exe"
}

# If not running interactively, don't do anything
case $- in
    *i*) ;;
      *) return;;
esac

# don't put duplicate lines or lines starting with space in the history.
# See bash(1) for more options
HISTCONTROL=ignoreboth

# append to the history file, don't overwrite it
shopt -s histappend

# for setting history length see HISTSIZE and HISTFILESIZE in bash(1)
HISTSIZE=1000
HISTFILESIZE=2000

# check the window size after each command and, if necessary,
# update the values of LINES and COLUMNS.
shopt -s checkwinsize

# If set, the pattern "**" used in a pathname expansion context will
# match all files and zero or more directories and subdirectories.
#shopt -s globstar

# make less more friendly for non-text input files, see lesspipe(1)
[ -x /usr/bin/lesspipe ] && eval "$(SHELL=/bin/sh lesspipe)"

# set variable identifying the chroot you work in (used in the prompt below)
if [ -z "${debian_chroot:-}" ] && [ -r /etc/debian_chroot ]; then
    debian_chroot=$(cat /etc/debian_chroot)
fi

# set a fancy prompt (non-color, unless we know we "want" color)
case "$TERM" in
    xterm-color|*-256color) color_prompt=yes;;
esac

# uncomment for a colored prompt, if the terminal has the capability; turned
# off by default to not distract the user: the focus in a terminal window
# should be on the output of commands, not on the prompt
#force_color_prompt=yes

if [ -n "$force_color_prompt" ]; then
    if [ -x /usr/bin/tput ] && tput setaf 1 >&/dev/null; then
	# We have color support; assume it's compliant with Ecma-48
	# (ISO/IEC-6429). (Lack of such support is extremely rare, and such
	# a case would tend to support setf rather than setaf.)
	color_prompt=yes
    else
	color_prompt=
    fi
fi

if [ "$color_prompt" = yes ]; then
    PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
else
    PS1='${debian_chroot:+($debian_chroot)}\u@\h:\w\$ '
fi
unset color_prompt force_color_prompt

# If this is an xterm set the title to user@host:dir
case "$TERM" in
xterm*|rxvt*)
    PS1="\[\e]0;${debian_chroot:+($debian_chroot)}\u@\h: \w\a\]$PS1"
    ;;
*)
    ;;
esac

# enable color support of ls and also add handy aliases
if [ -x /usr/bin/dircolors ]; then
    test -r ~/.dircolors && eval "$(dircolors -b ~/.dircolors)" || eval "$(dircolors -b)"
    #alias dir='dir --color=auto'
    #alias vdir='vdir --color=auto'

    alias grep='grep --color=auto'
    alias fgrep='fgrep --color=auto'
    alias egrep='egrep --color=auto'
fi

# colored GCC warnings and errors
#export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

# Add an "alert" alias for long running commands.  Use like so:
#   sleep 10; alert
alias alert='notify-send --urgency=low -i "$([ $? = 0 ] && echo terminal || echo error)" "$(history|tail -n1|sed -e '\''s/^\s*[0-9]\+\s*//;s/[;&|]\s*alert$//'\'')"'

# Alias definitions.
# You may want to put all your additions into a separate file like
# ~/.bash_aliases, instead of adding them here directly.
# See /usr/share/doc/bash-doc/examples in the bash-doc package.

if [ -f ~/.bash_aliases ]; then
    . ~/.bash_aliases
fi

# enable programmable completion features (you don't need to enable
# this, if it's already enabled in /etc/bash.bashrc and /etc/profile
# sources /etc/bash.bashrc).
if ! shopt -oq posix; then
  if [ -f /usr/share/bash-completion/bash_completion ]; then
    . /usr/share/bash-completion/bash_completion
  elif [ -f /etc/bash_completion ]; then
    . /etc/bash_completion
  fi
fi

# -------------- FZF
[ -f ~/.fzf.bash ] && source ~/.fzf.bash
# alias fzfui="fzf --height 40% --layout reverse --border --preview \"batcat --color=always --style=numbers --line-range=:500 {}\""
# alias fzfall="find / -path /mnt -prune -o -type f -print 2>/dev/null | fzf"
alias fzfui='fzf --style full --layout reverse --height 60% \
  --preview "batcat --color=always --style=numbers --line-range=:500 {}" \
  --bind "result:transform-list-label: if [[ -z \$FZF_QUERY ]]; then echo \" \$FZF_MATCH_COUNT items \"; else echo \" \$FZF_MATCH_COUNT matches for [\$FZF_QUERY] \"; fi" \
  --bind "focus:transform-preview-label:[[ -n {} ]] && printf \" Previewing [%s] \" {}" \
  --bind "focus:+transform-header:file --brief {} || echo \"No file selected\"" \
  --bind "ctrl-r:change-list-label( Reloading the list )+reload(sleep 2; git ls-files)" \
  --color "preview-border:#9999cc,preview-label:#ccccff" \
  --color "list-border:#669966,list-label:#99cc99" \
  --color "input-border:#996666,input-label:#ffcccc" \
  --color "header-border:#6699cc,header-label:#99ccff"'

export FZF_COMPLETION_TRIGGER='**'
export FZF_COMPLETION_OPTS='--layout reverse --height 30%'
export FZF_COMPLETION_PATH_OPTS='--walker file,dir,follow,hidden'
export FZF_COMPLETION_DIR_OPTS='--walker dir,follow'

# Comandi comuni con fuzzy completion
_fzf_setup_completion path vim less cat nano
_fzf_setup_completion dir cd tree
_fzf_setup_completion var export unset
_fzf_setup_completion host ssh telnet
# --------------
