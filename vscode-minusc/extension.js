"use strict";

const vscode = require("vscode");

/**
 * Remove string contents for brace / decl analysis (replace with spaces of same length).
 */
function maskStrings(s) {
  let out = "";
  let i = 0;
  while (i < s.length) {
    const c = s[i];
    if (c === '"') {
      out += '"';
      i++;
      while (i < s.length) {
        if (s[i] === "\\" && i + 1 < s.length) {
          out += "  ";
          i += 2;
          continue;
        }
        if (s[i] === '"') {
          out += '"';
          i++;
          break;
        }
        out += " ";
        i++;
      }
      continue;
    }
    out += c;
    i++;
  }
  return out;
}

/** Strip `{name} type` declarations from s for brace balance (mask with spaces). */
function maskVarDecls(s) {
  const re = /\{[^}]+\}\s*(int|str|bool|Array)(\s*<[^>]*>)?/g;
  return s.replace(re, (m) => " ".repeat(m.length));
}

/** Net `{` minus `}` on line after masking strings and var decls. */
function braceDelta(line) {
  let t = maskVarDecls(maskStrings(line));
  let n = 0;
  for (const ch of t) {
    if (ch === "{") n++;
    else if (ch === "}") n--;
  }
  return n;
}

/**
 * Format minusC source: trim trailing whitespace, normalize newlines,
 * re-indent lines using brace depth (respects strings and `{x} type` decls).
 */
function formatMinuscDocument(text, indentSize) {
  const rawLines = text.split(/\r?\n/);
  const lines = rawLines.map((ln) => ln.replace(/\s+$/, ""));
  let depth = 0;
  const out = [];
  let blankRun = 0;
  for (let i = 0; i < lines.length; i++) {
    const trimmed = lines[i].trim();
    if (trimmed === "") {
      blankRun++;
      if (blankRun <= 2) out.push("");
      continue;
    }
    blankRun = 0;
    const indent = " ".repeat(Math.max(0, depth) * indentSize);
    out.push(indent + trimmed);
    depth += braceDelta(trimmed);
    if (depth < 0) depth = 0;
  }
  let result = out.join("\n");
  if (text.endsWith("\n") || rawLines.length === 0) result += "\n";
  return result;
}

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
  const formatter = vscode.languages.registerDocumentFormattingEditProvider("minusc", {
    provideDocumentFormattingEdits(document, options, token) {
      const cfg = vscode.workspace.getConfiguration();
      const indentSize =
        cfg.get("minusc.format.indentSize") ??
        options.tabSize ??
        vscode.workspace.getConfiguration("editor").get("tabSize", 4);
      const full = document.getText();
      const formatted = formatMinuscDocument(full, indentSize);
      if (formatted === full) return [];
      const whole = new vscode.Range(
        document.positionAt(0),
        document.positionAt(full.length)
      );
      return [vscode.TextEdit.replace(whole, formatted)];
    },
  });

  context.subscriptions.push(formatter);
}

function deactivate() {}

module.exports = { activate, deactivate };
