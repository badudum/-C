"use strict";

const vscode = require("vscode");
const { execFile } = require("child_process");
const path = require("path");
const fs = require("fs");

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

function resolveCompilerPath() {
  const cfg = vscode.workspace.getConfiguration("minusc");
  return cfg.get("compilerPath") || "minusC.out";
}

function parseCompilerErrors(text, fallbackFile) {
  const diags = [];
  const lines = text.split(/\r?\n/);
  for (const line of lines) {
    const jsonMatch = line.match(/^\{"file":"([^"]*)","line":(\d+),"message":"(.*)"\}$/);
    if (jsonMatch) {
      const file = jsonMatch[1] || fallbackFile;
      const ln = Math.max(0, parseInt(jsonMatch[2], 10) - 1);
      const msg = jsonMatch[3].replace(/\\"/g, '"').replace(/\\n/g, "\n");
      diags.push({
        file,
        line: ln,
        col: 0,
        endCol: 256,
        message: msg,
      });
      continue;
    }
    const m = line.match(/Error at ([^:]+):(\d+): (.+)/);
    if (m) {
      diags.push({
        file: m[1],
        line: Math.max(0, parseInt(m[2], 10) - 1),
        col: 0,
        endCol: 256,
        message: m[3],
      });
    }
  }
  return diags;
}

function runCompilerCheck(document) {
  const compiler = resolveCompilerPath();
  const filePath = document.uri.fsPath;
  const args = ["--check", "--json-diagnostics", filePath];
  return new Promise((resolve) => {
    execFile(compiler, args, { cwd: path.dirname(filePath) }, (err, stdout, stderr) => {
      const out = `${stderr || ""}${stdout || ""}`;
      if (!err) {
        resolve([]);
        return;
      }
      const parsed = parseCompilerErrors(out, filePath);
      if (!parsed.length) {
        resolve([
          {
            file: filePath,
            line: 0,
            col: 0,
            endCol: 1,
            message: out.trim() || "compile check failed",
          },
        ]);
        return;
      }
      resolve(parsed);
    });
  });
}

/**
 * @typedef {{ name: string, kind: string, line: number, col: number, endCol: number }} SymbolInfo
 */

/** Index top-level and simple scoped symbols from minusC source. */
function indexDocumentSymbols(text) {
  /** @type {SymbolInfo[]} */
  const symbols = [];
  const lines = text.split(/\r?\n/);

  const patterns = [
  { re: /^(\w+)\s*=\s*\([^)]*\)\s*function\b/, kind: "function" },
  { re: /^(\w+)\s*=\s*cust\b/, kind: "type" },
  { re: /^enum\s+(\w+)\b/, kind: "enum" },
  { re: /^\{(\w+)\}\s+[\w<>,\s]+\s*=/, kind: "variable" },
  { re: /^(\w+)\s*=\s*\([^)]*\)\s*function\b/, kind: "function" },
  ];

  for (let i = 0; i < lines.length; i++) {
    const trimmed = lines[i].trim();
    for (const { re, kind } of patterns) {
      const m = trimmed.match(re);
      if (!m) continue;
      const name = m[1];
      const col = lines[i].indexOf(name);
      symbols.push({
        name,
        kind,
        line: i,
        col: col >= 0 ? col : 0,
        endCol: (col >= 0 ? col : 0) + name.length,
      });
      break;
    }
  }
  return symbols;
}

function findSymbolDefinition(name, document) {
  const symbols = indexDocumentSymbols(document.getText());
  return symbols.find((s) => s.name === name) || null;
}

function findSymbolReferences(name, document) {
  const text = document.getText();
  const lines = text.split(/\r?\n/);
  /** @type {{ line: number, col: number, endCol: number }[]} */
  const refs = [];
  const wordRe = new RegExp(`\\b${name.replace(/[.*+?^${}()|[\]\\]/g, "\\$&")}\\b`, "g");
  for (let i = 0; i < lines.length; i++) {
    let m;
    while ((m = wordRe.exec(lines[i])) !== null) {
      refs.push({ line: i, col: m.index, endCol: m.index + name.length });
    }
  }
  return refs;
}

let debounceTimer = null;

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

  const definitionProvider = vscode.languages.registerDefinitionProvider("minusc", {
    provideDefinition(document, position) {
      const wordRange = document.getWordRangeAtPosition(position, /\w+/);
      if (!wordRange) return null;
      const name = document.getText(wordRange);
      const def = findSymbolDefinition(name, document);
      if (!def) return null;
      return new vscode.Location(
        document.uri,
        new vscode.Range(def.line, def.col, def.line, def.endCol)
      );
    },
  });

  const renameProvider = vscode.languages.registerRenameProvider("minusc", {
    provideRenameEdits(document, position, newName) {
      const wordRange = document.getWordRangeAtPosition(position, /\w+/);
      if (!wordRange) return null;
      const oldName = document.getText(wordRange);
      const def = findSymbolDefinition(oldName, document);
      if (!def) return null;
      const refs = findSymbolReferences(oldName, document);
      const edit = new vscode.WorkspaceEdit();
      for (const ref of refs) {
        const range = new vscode.Range(ref.line, ref.col, ref.line, ref.endCol);
        edit.replace(document.uri, range, newName);
      }
      return edit;
    },
  });

  const documentSymbolProvider = vscode.languages.registerDocumentSymbolProvider("minusc", {
    provideDocumentSymbols(document) {
      return indexDocumentSymbols(document.getText()).map((s) => {
        const kindMap = {
          function: vscode.SymbolKind.Function,
          type: vscode.SymbolKind.Class,
          enum: vscode.SymbolKind.Enum,
          variable: vscode.SymbolKind.Variable,
        };
        return new vscode.DocumentSymbol(
          s.name,
          s.kind,
          kindMap[s.kind] || vscode.SymbolKind.Variable,
          new vscode.Range(s.line, s.col, s.line, s.endCol),
          new vscode.Range(s.line, s.col, s.line, s.endCol)
        );
      });
    },
  });

  context.subscriptions.push(formatter, definitionProvider, renameProvider, documentSymbolProvider);

  const diagCollection = vscode.languages.createDiagnosticCollection("minusc");
  context.subscriptions.push(diagCollection);

  async function refreshDiagnostics(document) {
    if (document.languageId !== "minusc") return;
    const raw = await runCompilerCheck(document);
    const diags = raw.map((d) => {
      const uri =
        d.file && fs.existsSync(d.file) ? vscode.Uri.file(d.file) : document.uri;
      return new vscode.Diagnostic(
        new vscode.Range(d.line, d.col, d.line, d.endCol),
        d.message,
        vscode.DiagnosticSeverity.Error
      );
    });
    diagCollection.set(document.uri, diags);
  }

  function scheduleDiagnostics(document) {
    if (debounceTimer) clearTimeout(debounceTimer);
    debounceTimer = setTimeout(() => refreshDiagnostics(document), 500);
  }

  const onSave = vscode.workspace.onDidSaveTextDocument((doc) => {
    refreshDiagnostics(doc);
  });
  const onChange = vscode.workspace.onDidChangeTextDocument((e) => {
    if (e.document.languageId === "minusc") scheduleDiagnostics(e.document);
  });
  context.subscriptions.push(onSave, onChange);

  if (vscode.window.activeTextEditor) {
    refreshDiagnostics(vscode.window.activeTextEditor.document);
  }
}

function deactivate() {}

module.exports = { activate, deactivate, indexDocumentSymbols, formatMinuscDocument };
