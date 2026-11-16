import { Toolbar as Tb } from "@base-ui/react/toolbar";

const Toolbar = () => {
  return (
    <Tb.Root className="flex justify-between rounded-t-lg bg-gray-light p-1.5">
      <Tb.Group className="flex">
        <Tb.Button className="btn hover:bg-gray-med">Uniforms</Tb.Button>
        <Tb.Button className="btn hover:bg-gray-med">Snippets</Tb.Button>
      </Tb.Group>

      <Tb.Button className="btn hover:bg-gray-med">Appearance</Tb.Button>
    </Tb.Root>
  );
};

export { Toolbar };
