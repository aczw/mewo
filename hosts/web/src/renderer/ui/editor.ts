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

import type { Project } from "../../mewo/classes/project";
import type { PassId } from "../../mewo/types";
import { Component } from "../system/component";

type Init = {
  project: Project;
  activePassId: PassId;
};

type Callbacks = {
  onSourceChanged: (shownPassId: PassId, newSource: string) => void;
};

class Editor extends Component {
  readonly #extensions: Extension[];
  readonly #states = new Map<PassId, EditorState>();
  readonly #view: EditorView;

  #shownPassId: PassId;

  constructor(parent: HTMLElement, init: Init, cb: Callbacks) {
    super();

    this.#shownPassId = init.activePassId;

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

    for (let idx = 0; idx < init.project.getPassCount(); ++idx) {
      const passId = init.project.getPassId(idx);
      if (!passId) continue;
      this.addState(init.project, passId);
    }

    this.#view = new EditorView({ parent, state: this.#states.get(init.activePassId) });
    this.defer(() => this.#view.destroy());
  }

  addState(project: Project, passId: PassId): void {
    this.#states.set(passId, this.#createState(project.getPassSource(passId)));
  }

  showState(passId: PassId): void {
    this.#states.set(this.#shownPassId, this.#view.state);
    this.#shownPassId = passId;
    // The current `EditorState`s are always kept in sync with the project, so this must exist
    this.#view.setState(this.#states.get(passId)!);
  }

  #createState(source: string): EditorState {
    return EditorState.create({ doc: source, extensions: this.#extensions });
  }
}

export { Editor };
