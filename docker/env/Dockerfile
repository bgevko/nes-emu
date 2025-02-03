# Use the official Ubuntu image as the base
FROM ubuntu:latest

# Prevent prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install necessary packages
RUN apt-get update && apt-get install -y \
  zsh \
  git \
  wget \
  curl \
  vim \
  build-essential \
  clang \
  clang-tools \
  clang-tidy \
  clang-format \
  cmake \
  ca-certificates \
  libsdl2-dev \
  libsdl2-2.0-0 \
  bash \
  file \
  && rm -rf /var/lib/apt/lists/*

# Set zsh as the default shell for the root user
RUN chsh -s /usr/bin/zsh root

# Optional: Install Oh My Zsh for an enhanced shell experience
RUN sh -c "$(wget https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)" "" --unattended

# Set the working directory
WORKDIR /root

# Start zsh when a container is launched
CMD ["zsh"]
