#!/bin/sh
# Keep unrelated runner package repositories out of emulator QA.
set -eu
apt_ubuntu() {
    if [ -f /etc/apt/sources.list.d/ubuntu.sources ]; then
        sudo apt-get -o Acquire::Retries=3 \
          -o Dir::Etc::sourcelist=/etc/apt/sources.list.d/ubuntu.sources \
          -o Dir::Etc::sourceparts=- "$@"
    else
        sudo apt-get -o Acquire::Retries=3 "$@"
    fi
}
for attempt in 1 2 3; do
    if apt_ubuntu update -qq && apt_ubuntu install -y -qq "$@"; then
        exit 0
    fi
    [ "$attempt" -lt 3 ] || exit 1
    sleep 5
done
