import {
  MewoDiagnosticSeverity,
  MewoDiagnosticSource,
  MewoPassCompileStatus,
  type Pass,
} from "../../../mewo/types";
import { useEngineSnapshot } from "../../engine/context";

const Pending = () => <p className="leading-none text-fg-subtle">Compiling...</p>;

const ResultLabel = ({ isSuccess, duration }: { isSuccess: boolean; duration: number }) => {
  return (
    <p className="leading-none text-fg-subtle">
      {isSuccess ? "Done" : "Failed"} in {duration.toFixed(0)}ms
    </p>
  );
};

const CommonStatus = ({ passes }: { passes: Pass[] }) => {
  const compilations = passes.map((pass) => pass.compilation);
  if (!compilations.every((comp) => comp !== null)) return <p>Waiting...</p>;

  if (!compilations.every((comp) => comp.status !== MewoPassCompileStatus.Pending))
    return <Pending />;

  const commonFailed = passes.some((pass) =>
    pass.diagnostics.some(
      (diag) =>
        diag.source === MewoDiagnosticSource.Common &&
        diag.severity === MewoDiagnosticSeverity.Error,
    ),
  );
  const maxDuration = Math.max(...compilations.map((comp) => comp.duration));

  return <ResultLabel isSuccess={!commonFailed} duration={maxDuration} />;
};

const PassStatus = () => {
  const { passes, selection } = useEngineSnapshot();

  if (selection.kind === "common") return <CommonStatus passes={passes} />;

  const activePass = passes.find((pass) => pass.id === selection.id);
  if (!activePass || activePass.compilation === null) return <p>Waiting...</p>;

  const { compilation } = activePass;
  if (compilation.status === MewoPassCompileStatus.Pending) return <Pending />;

  return (
    <ResultLabel
      isSuccess={compilation.status === MewoPassCompileStatus.Success}
      duration={compilation.duration}
    />
  );
};

export { PassStatus as Status };
