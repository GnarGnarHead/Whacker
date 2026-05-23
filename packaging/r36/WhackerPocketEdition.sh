#!/bin/sh

PORT_DIR="/storage/roms/ports/WhackerPocketEdition"
LOG="$PORT_DIR/whacker-pocket-edition.log"

cd "$PORT_DIR" || exit 1

export HOME="/storage"
export LD_LIBRARY_PATH="/usr/lib:/lib:/emuelec/lib:/emuelec/lib32:${LD_LIBRARY_PATH}"
export SDL_AUDIODRIVER="${SDL_AUDIODRIVER:-alsa}"
export SDL_VIDEO_GL_DRIVER="${SDL_VIDEO_GL_DRIVER:-/usr/lib/libGL.so}"
export SDL_NOMOUSE=1

{
  echo "Whacker: Pocket Edition launch: $(date 2>/dev/null || echo unknown-time)"
  echo "PWD=$PWD"
  echo "SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-auto}"
  echo "SDL_AUDIODRIVER=$SDL_AUDIODRIVER"
  echo "SDL_VIDEO_GL_DRIVER=$SDL_VIDEO_GL_DRIVER"
  echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
  ./whacker
  code=$?
  echo "exit_code=$code"
} >"$LOG" 2>&1

sync
exit "$code"
