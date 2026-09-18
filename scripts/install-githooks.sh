#!/bin/sh
# Install versioned git hooks that strip/block Cursor co-author trailers.
set -e
cd "$(dirname "$0")/.."
if [ ! -d .git/hooks ]; then
    echo "install-githooks: .git/hooks not found (not a git checkout?)" >&2
    exit 1
fi

for hook in filter-cursor-coauthor commit-msg prepare-commit-msg post-commit pre-push; do
    src="githooks/$hook"
    dst=".git/hooks/$hook"
    if [ ! -f "$src" ]; then
        echo "install-githooks: missing $src" >&2
        exit 1
    fi
    cp "$src" "$dst"
    chmod +x "$dst"
done

echo "Installed git hooks: commit-msg prepare-commit-msg post-commit pre-push"
