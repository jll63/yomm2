FROM ubuntu:latest

ARG USER
ARG UID
ARG GROUP
ARG GID

COPY yomm2-init-image.sh /tmp/yomm2-init-image.sh

RUN /tmp/yomm2-init-image.sh $USER $UID $GROUP $GID && rm /tmp/yomm2-init-image.sh

USER $UID:$GID
RUN echo 'export PS1="[\u@docker] \W $ "' >> /home/$USER/.bashrc
WORKDIR /home/$USER
