type Options = {
  id?: string;
  class?: string;
  text?: string;
  attributes?: Record<string, string>;
};

function el<K extends keyof HTMLElementTagNameMap>(
  tag: K,
  options: Options = {},
  children: Node[] = [],
): HTMLElementTagNameMap[K] {
  const element = document.createElement(tag);

  if (options.id) element.id = options.id;
  if (options.class) element.className = options.class;
  if (options.text) element.textContent = options.text;

  for (const [key, value] of Object.entries(options.attributes ?? {}))
    element.setAttribute(key, value);

  element.append(...children);
  return element;
}

export { el };
