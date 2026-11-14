import {
  autocompletion,
  closeBrackets,
  closeBracketsKeymap,
  completionKeymap,
} from "@codemirror/autocomplete";
import { defaultKeymap, history, historyKeymap } from "@codemirror/commands";
import {
  bracketMatching,
  defaultHighlightStyle,
  foldGutter,
  foldKeymap,
  indentOnInput,
  syntaxHighlighting,
} from "@codemirror/language";
import { lintKeymap } from "@codemirror/lint";
import { highlightSelectionMatches, searchKeymap } from "@codemirror/search";
import { EditorState, type Extension } from "@codemirror/state";
import {
  crosshairCursor,
  drawSelection,
  dropCursor,
  EditorView,
  highlightActiveLine,
  highlightActiveLineGutter,
  highlightSpecialChars,
  keymap,
  lineNumbers,
} from "@codemirror/view";
import { useEffect, useRef, useState } from "react";

import { useEngine, useEngineSnapshot } from "../../engine/context";
import type { Selection } from "../../engine/engine";
import { calculateDocInfo, type DocInfo, Info } from "./info";

import styles from "./editor.module.css";

type Callbacks = {
  onDocChanged: (source: string) => void;
  onSelectionChanged: (state: EditorState) => void;
};

const keyOf = (selection: Selection): string =>
  selection.kind === "pass" ? `pass:${selection.id}` : "common";

const createExtensions = (cb: Callbacks): Extension[] => [
  // A line number gutter
  lineNumbers(),
  // A gutter with code folding markers
  foldGutter(),
  // Replace non-printable characters with placeholders
  highlightSpecialChars(),
  // The undo history
  history(),
  // Replace native cursor/selection with our own
  drawSelection(),
  // Show a drop cursor when dragging over the editor
  dropCursor(),
  // Allow multiple cursors/selections
  EditorState.allowMultipleSelections.of(true),
  // Re-indent lines when typing specific input
  indentOnInput(),
  // Highlight syntax with a default style
  syntaxHighlighting(defaultHighlightStyle),
  // Highlight matching brackets near cursor
  bracketMatching(),
  // Automatically close brackets
  closeBrackets(),
  // Load the autocompletion system
  autocompletion(),
  // Change the cursor to a crosshair when hold
  crosshairCursor(),
  // Style the current line specially
  highlightActiveLine(),
  // Style the gutter for current line specially
  highlightActiveLineGutter(),
  // Highlight text that matches the selected text
  highlightSelectionMatches(),
  keymap.of([
    // Closed-brackets aware backspace
    ...closeBracketsKeymap,
    // A large set of basic bindings
    ...defaultKeymap,
    // Search-related keys
    ...searchKeymap,
    // Redo/undo keys
    ...historyKeymap,
    // Code folding bindings
    ...foldKeymap,
    // Autocompletion keys
    ...completionKeymap,
    // Keys related to the linter system
    ...lintKeymap,
  ]),

  EditorView.updateListener.of((update) => {
    if (update.docChanged) cb.onDocChanged(update.state.doc.toString());
    if (update.docChanged || update.selectionSet) cb.onSelectionChanged(update.state);
  }),

  EditorView.theme({
    "&": { height: "100%" },
    "&.cm-focused": { outline: "none" },
    ".cm-scroller": { overflow: "auto", fontFamily: "var(--font-code)", fontSize: "0.85rem" },
    ".cm-gutters": {
      backgroundColor: "var(--bg-secondary)",
      color: "var(--fg-med)",
      border: "none",
    },
    ".cm-content": { backgroundColor: "white" },
  }),
];

const Editor = () => {
  const engine = useEngine();
  const { passes, selection } = useEngineSnapshot();

  const parentRef = useRef<HTMLDivElement>(null);
  const viewRef = useRef<EditorView | null>(null);
  const statesRef = useRef(new Map<string, EditorState>());
  const selectionRef = useRef<Selection | null>(null);

  const [docInfo, setDocInfo] = useState<DocInfo>({ line: 1, column: 1, length: 0 });

  const [extensions] = useState(() =>
    createExtensions({
      onDocChanged: (source) => {
        if (selectionRef.current === null) return;

        if (selectionRef.current.kind === "pass") {
          engine.setPassSource(selectionRef.current.id, source);
        } else {
          engine.setCommonSource(source);
        }
      },
      onSelectionChanged: (state) => setDocInfo(calculateDocInfo(state)),
    }),
  );

  useEffect(() => {
    const view = new EditorView({ parent: parentRef.current! });
    viewRef.current = view;

    return () => {
      // Save the shown selection so a remount restores it
      if (selectionRef.current !== null)
        statesRef.current.set(keyOf(selectionRef.current), view.state);
      selectionRef.current = null;

      view.destroy();
      viewRef.current = null;
    };
  }, []);

  // Reconcile editor states with pass list, then show the active selection
  useEffect(() => {
    const view = viewRef.current!;
    const states = statesRef.current;

    // Save currently shown selection before reconciling
    if (selectionRef.current !== null) states.set(keyOf(selectionRef.current), view.state);

    const commonKey = keyOf({ kind: "common" });
    const alive = new Set<string>([commonKey]);
    for (const { id } of passes) {
      const passKey = keyOf({ kind: "pass", id });
      alive.add(passKey);

      if (!states.has(passKey))
        states.set(passKey, EditorState.create({ doc: engine.getPassSource(id), extensions }));
    }

    if (!states.has(commonKey))
      states.set(commonKey, EditorState.create({ doc: engine.getCommonSource(), extensions }));

    for (const key of states.keys()) if (!alive.has(key)) states.delete(key);

    // Switch selection only if active selection has changed
    if (selectionRef.current === null || keyOf(selection) !== keyOf(selectionRef.current)) {
      view.setState(states.get(keyOf(selection))!);
      setDocInfo(calculateDocInfo(view.state));
      selectionRef.current = selection;
    }
  }, [engine, extensions, passes, selection]);

  return (
    <>
      <div className={styles.wrapper} ref={parentRef} />
      <Info docInfo={docInfo} />
    </>
  );
};

export { Editor };
