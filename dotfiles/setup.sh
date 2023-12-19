#/bin/bash

set -x

maybeCreateDir() {
  dir=$1
  if [ ! -r ${dir} ]; then
    echo "Creating ${dir}"
    mkdir -p ${dir}
  fi
}

prog_name=$(basename "$0")
dir_name=$(cd "$(dirname "$0")" || exit 1; pwd -P)

if [ -r ${HOME}/.dotfiles ]; then
  rm -f ${HOME}/.dotfiles
fi
ln -s ${dir_name} ${HOME}/.dotfiles

if [[ -w ${HOME}/.bashrc && "$(realpath ${HOME}/.bashrc)" != "${dir_name}/bashrc" ]]; then
  echo ". ${dir_name}/bashrc" >> ${HOME}/.bashrc
fi

for dotfile in clang-format gitignore ctags bashrc inputrc template.cpp tmux.conf vimrc hgrc pylintrc; do
  if [ -r ${HOME}/.${dotfile} ]; then
    echo "${HOME}/.${dotfile} already exists! Skipped it."
  else
    ln -f -s ${dir_name}/${dotfile} ${HOME}/.${dotfile}
  fi
done

cp -f gitconfig ~/.gitconfig

maybeCreateDir ${HOME}/.vim

for vimfile in filetype.vim; do
  ln -f -s ${dir_name}/${vimfile} ${HOME}/.vim/${vimfile}
done

echo "Installing Tmux Plugin Manager..."

tmuxPluginDir=${HOME}/.tmux/plugins
maybeCreateDir ${tmuxPluginDir}

if [ ! -r ${tmuxPluginDir} ]; then
  mkdir -p ${tmuxPluginDir}
fi
if [ ! -r ${tmuxPluginDir} ]; then
  git clone https://github.com/tmux-plugins/tpm ${tmuxPluginDir}/tpm
fi
tmux source ${HOME}/.tmux.conf || true

if [ ! -r ${HOME}/.ssh/config ]; then
  mkdir -p ${HOME}/.ssh || true
  cp ${dir_name}/ssh_config ${HOME}/.ssh/config
  chown ${UID} ${HOME}/.ssh/config
  chmod 600 ${HOME}/.ssh/config
fi

set +x
