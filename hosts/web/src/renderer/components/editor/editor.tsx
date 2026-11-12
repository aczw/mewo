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

import { kPassIdNone, type PassId } from "../../../mewo/types";
import { useEngine, useEngineSnapshot } from "../../engine/context";
import { calculateDocInfo, type DocInfo, Info } from "./info";

import styles from "./editor.module.css";

type Callbacks = {
  onDocChanged: (source: string) => void;
  onSelectionChanged: (state: EditorState) => void;
};

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
    ".cm-scroller": { overflow: "auto", fontFamily: "var(--font-code)", fontSize: "0.9rem" },
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
  const { passes, activePass } = useEngineSnapshot();

  const parentRef = useRef<HTMLDivElement>(null);
  const viewRef = useRef<EditorView | null>(null);
  const statesRef = useRef(new Map<PassId, EditorState>());
  const shownPassRef = useRef<PassId>(kPassIdNone);

  const [docInfo, setDocInfo] = useState<DocInfo>({ line: 1, column: 1, length: 0 });

  const [extensions] = useState(() =>
    createExtensions({
      onDocChanged: (source) => engine.setPassSource(shownPassRef.current, source),
      onSelectionChanged: (state) => setDocInfo(calculateDocInfo(state)),
    }),
  );

  useEffect(() => {
    const view = new EditorView({ parent: parentRef.current! });
    viewRef.current = view;

    return () => {
      // Save the shown pass' state so a remount restores it
      if (shownPassRef.current !== kPassIdNone)
        statesRef.current.set(shownPassRef.current, view.state);
      shownPassRef.current = kPassIdNone;

      view.destroy();
      viewRef.current = null;
    };
  }, []);

  // Reconcile per-pass states with pass list, then show active pass
  useEffect(() => {
    const view = viewRef.current!;
    const states = statesRef.current;

    // Save currently shown pass' state before reconciling
    if (shownPassRef.current !== kPassIdNone) states.set(shownPassRef.current, view.state);

    const aliveIds = new Set<PassId>();
    for (const { id } of passes) {
      aliveIds.add(id);
      if (!states.has(id))
        states.set(id, EditorState.create({ doc: engine.getPassSource(id), extensions }));
    }

    for (const id of states.keys()) if (!aliveIds.has(id)) states.delete(id);

    // Switch view only if active pass has changed
    if (activePass !== shownPassRef.current) {
      view.setState(states.get(activePass)!);
      setDocInfo(calculateDocInfo(view.state));
      shownPassRef.current = activePass;
    }
  }, [engine, extensions, passes, activePass]);

  return (
    <>
      <div className={styles.wrapper} ref={parentRef} />
      <Info info={docInfo} />
    </>
  );
};

export { Editor };
