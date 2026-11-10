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
  rectangularSelection,
} from "@codemirror/view";

import { kPassIdNone, type PassId } from "../../mewo/types";
import { Component } from "../system/component";
import type { EngineSnapshot } from "../system/engine";

type Callbacks = {
  getPassSource: (passId: PassId) => string;
  onSourceChanged: (shownPassId: PassId, newSource: string) => void;
};

class Editor extends Component {
  readonly #extensions: Extension[];
  readonly #states = new Map<PassId, EditorState>();
  readonly #view: EditorView;
  readonly #getPassSource: (passId: PassId) => string;

  #shownPassId: PassId = kPassIdNone;

  constructor(parent: HTMLElement, init: EngineSnapshot, cb: Callbacks) {
    super();

    this.#getPassSource = cb.getPassSource;

    this.#extensions = [
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
      // Allow alt-drag to select rectangular regions
      rectangularSelection(),
      // Change the cursor to a crosshair when holding alt
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
        if (update.docChanged) cb.onSourceChanged(this.#shownPassId, update.state.doc.toString());
      }),
    ];

    this.#view = new EditorView({ parent });
    this.defer(() => this.#view.destroy());

    this.update(init);
  }

  update(snapshot: EngineSnapshot): void {
    // Save the currently shown pass' latest state before reconciling
    if (this.#shownPassId !== kPassIdNone) this.#states.set(this.#shownPassId, this.#view.state);

    const aliveIds = new Set<PassId>();
    for (const { id } of snapshot.passes) {
      aliveIds.add(id);
      if (!this.#states.has(id)) this.#states.set(id, this.#createState(this.#getPassSource(id)));
    }

    for (const passId of this.#states.keys())
      if (!aliveIds.has(passId)) this.#states.delete(passId);

    // Switch the view only if the active pass has changed
    if (snapshot.activePass !== this.#shownPassId) {
      this.#view.setState(this.#states.get(snapshot.activePass)!);
      this.#shownPassId = snapshot.activePass;
    }
  }

  #createState(source: string): EditorState {
    return EditorState.create({ doc: source, extensions: this.#extensions });
  }
}

export { Editor };
