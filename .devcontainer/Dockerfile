# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.209.6/containers/ubuntu/.devcontainer/base.Dockerfile

# [Choice] Ubuntu version (use hirsuite or bionic on local arm64/Apple Silicon): hirsute, focal, bionic
ARG VARIANT="focal"
FROM mcr.microsoft.com/vscode/devcontainers/base:0-${VARIANT}

# This Dockerfile's base image has a non-root user with sudo access. Use the "remoteUser"
# property in devcontainer.json to use it. On Linux, the container user's GID/UIDs
# will be updated to match your local UID/GID (when using the dockerFile property).
# See https://aka.ms/vscode-remote/containers/non-root-user for details.
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

ENV PICO_ROOT /opt/pico
ENV PICO_SDK_PATH ${PICO_ROOT}/pico/pico-sdk
ENV PICO_EXAMPLES_PATH ${PICO_ROOT}/pico/pico-examples
ENV PICO_EXTRA_PATH ${PICO_ROOT}/pico/pico-extra

# Configure CMake repository
# https://apt.kitware.com/
RUN apt update \
    && apt install -y libusb-1.0-0-dev pkg-config ninja-build gdb-multiarch \
    && mkdir -p ${PICO_ROOT} \
    && cd ${PICO_ROOT} \
	&& wget https://raw.githubusercontent.com/raspberrypi/pico-setup/master/pico_setup.sh 2>/dev/null \
	&& export SKIP_VSCODE=1 \
	&& export SKIP_UART=1 \
	&& export SKIP_OPENOCD=1 \
	&& /bin/bash ./pico_setup.sh \
    #
    # [Optional] Update UID/GID if needed
    && if [ "$USER_GID" != "1000" ] || [ "$USER_UID" != "1000" ]; then \
    groupmod --gid $USER_GID $USERNAME \
    && usermod --uid $USER_UID --gid $USER_GID $USERNAME \
    && chown -R $USER_UID:$USER_GID /home/$USERNAME; \
    fi \
    #
    # Clean up
    && apt autoremove -y \
    && apt clean -y \
    && rm -rf /var/lib/apt/lists/*