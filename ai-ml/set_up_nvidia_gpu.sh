#!/bin/bash

installNvidiaDrive() {
  driver=$(nvidia-detector)
  echo "Will install Nvidia driver: ${driver}"
  sudo apt install -y ${driver} "$(echo ${driver} | sed 's/-driver-/-dkms-/')"
  nvidia-smi
}

# See the latest image: https://docs.nvidia.com/deeplearning/frameworks/pytorch-release-notes/index.html
pytorch_image_version='23.02-py3'
alias pytorch-docker="docker run --gpus all --ipc=host --ulimit memlock=-1 --ulimit stack=67108864 -it --rm nvcr.io/nvidia/pytorch:${pytorch_image_version}"

installNvidiaLibContainer() {
  ubuntuVersion="$(lsb_release -a 2> /dev/null | grep 'Ubuntu .* LTS$' | awk ' { print $3 } ' | cut -d'.' -f1,2)"
  curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey | sudo gpg --dearmor -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg
  curl -s -L https://nvidia.github.io/libnvidia-container/ubuntu${ubuntuVersion}/libnvidia-container.list | sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' | sudo tee /etc/apt/sources.list.d/nvidia-container-toolkit.list
  sudo apt-get update && sudo apt-get install -y nvidia-container-toolkit
  sudo nvidia-ctk runtime configure --runtime=docker
  docker pull nvcr.io/nvidia/pytorch:${pytorch_image_version}
}

installNvidiaDrive

installNvidiaLibContainer

if [ -d "/usr/local/cuda-12" ]; then
  CUDA="$(ls /usr/local/cuda-12\.* -d | head -n1)"
  export PATH=${CUDA}/bin${PATH:+:${PATH}}
  export LD_LIBRARY_PATH=/usr/local/cuda-12.0/lib64:${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
fi