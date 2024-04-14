#!/bin/bash

set -e

user=$1
shift
uid=$1
shift
group=$1
shift
gid=$1
shift

mkdir /etc/sudoers.d
echo "ALL ALL=(ALL:ALL) NOPASSWD: ALL" > /etc/sudoers.d/sudoall
chmod 0440 /etc/sudoers.d/sudoall

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y tzdata
ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
dpkg-reconfigure --frontend noninteractive tzdata

apt-get install -y cmake git clang++-15 g++15 sudo

groupadd -g $gid $group
useradd -g $gid -m $user
