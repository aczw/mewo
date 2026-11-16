import { Run } from "./compilation/run";
import { Status } from "./compilation/status";

const Compilation = () => {
  return (
    <div className="flex items-center justify-between pr-0.75 pb-1.5 pl-1.5">
      <Run />
      <Status />
    </div>
  );
};

export { Compilation };
