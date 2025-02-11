#!/bin/bash
set -euo pipefail

# Docker image name
IMAGE="project-linter2"

# Common Docker flags for commands that mount the current workspace.
DOCKER_COMMON=(
  --rm
  -v "$(pwd):/workspace"
  -v build:/workspace/build
  -v deps:/workspace/deps
  -w /workspace
)

case "${1:-}" in
build | lint | test)
  docker run "${DOCKER_COMMON[@]}" "$IMAGE" "${1}"
  ;;
bash)
  if [ "${2:-}" = "--no-mount" ]; then
    echo "Running bash without mounting workspace."
    docker run --rm -it --entrypoint /bin/bash -w /workspace "$IMAGE"
  else
    echo "Running bash with mounted workspace."
    docker run --rm -it --entrypoint /bin/bash "${DOCKER_COMMON[@]}" "$IMAGE"
  fi
  ;;
*)
  echo "Usage: $0 {build|lint|test|bash [--no-mount]}"
  exit 1
  ;;
esac
