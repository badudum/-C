# minusC (`.minusc`) — VS Code / Cursor extension

Syntax highlighting (TextMate grammar) and a document formatter.

## Install

### From this folder (development)

1. Open the repo in Cursor or VS Code.
2. **Run → Start Debugging** (F5), choose **“Run Extension”** if prompted, and pick `vscode-minusc` as the extension folder **or** open `vscode-minusc` alone and F5.
3. In the new `[Extension Development Host]` window, open a `.minusc` file.

### Install into your editor (persistent)

From a terminal (use the path to your clone):

```bash
code --install-extension /absolute/path/to/-C/vscode-minusc
```

If `code` is not on your `PATH`, use **Command Palette → “Extensions: Install from VSIX…”** after packaging:

```bash
npx --yes @vscode/vsce package
```

Then install the generated `.vsix`.

## Formatter

- **Format Document** (Shift+Option+F / Shift+Alt+F) or enable **Format on Save** for `[minusc]` in settings.
- Indents with **4 spaces** per brace level by default (respects strings and `{name} type` declarations so they do not shift depth).
- Trims trailing whitespace and collapses runs of more than two blank lines to two.

Setting: **`minusc.format.indentSize`** (1–16).

## Grammar

Highlights `reference`, `function`, `if` / `else`, `loop` / `until`, `return`, `and` / `or` / `not`, `Real` / `Fake`, types, strings, `//` line comments, and `comment:` … `;` blocks (best-effort).

## Note

The formatter is heuristic (not the full minusC parser). Unusual layouts may need a manual touch after formatting.
