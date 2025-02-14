# Docker Tools

Docker tools that are used in the CI/CD pipeline are also available locally.

## Setup

---

If applicable, remove the old `project-linter` image:

```bash
docker rmi project-linter
```

Install the new image:

```bash
docker build --no-cache -t project-linter2 -f docker/ubuntu-Dockerfile .
```

Subsequent rebuilds can be done without the `--no-cache` flag.

```bash
docker build -t project-linter -f docker/ubuntu-Dockerfile .
```

You'll want to rebuild after:

- Updating `scripts/entrypoint.sh`
- Adding or removing dependencies in `docker-vcpkg/vcpkg.json`
