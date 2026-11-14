type Ptr = number;

const kNull: Ptr = 0 as const;

const MewoEvent = { None: 0, DeviceReady: 1, DeviceFailed: 2, DeviceLost: 3, GpuError: 4 } as const;
type MewoEvent = (typeof MewoEvent)[keyof typeof MewoEvent];

const MewoResult = {
  Success: 0,
  NotReady: -1,
  Internal: -2,
  InvalidArgument: -3,
  InvalidOperation: -4,
} as const;
type MewoResult = (typeof MewoResult)[keyof typeof MewoResult];

const MewoPassCompileStatus = { Pending: 0, Success: 1, Error: 2 } as const;
type MewoPassCompileStatus = (typeof MewoPassCompileStatus)[keyof typeof MewoPassCompileStatus];

const MewoDiagnosticSeverity = { Error: 0, Warning: 1, Info: 2 } as const;
type MewoDiagnosticSeverity = (typeof MewoDiagnosticSeverity)[keyof typeof MewoDiagnosticSeverity];

const MewoDiagnosticSource = { None: 0, Pass: 1, Common: 2, Generated: 3 } as const;
type MewoDiagnosticSource = (typeof MewoDiagnosticSource)[keyof typeof MewoDiagnosticSource];

/** `line` and `column` are 1-based (column in UTF-8 bytes), and 0 when `source` is `None`. */
type PassDiagnostic = {
  severity: MewoDiagnosticSeverity;
  message: string;
  source: MewoDiagnosticSource;
  line: number;
  column: number;
};
type CommonDiagnostic = PassDiagnostic & { passIds: PassId[] };

type Dimensions = { width: number; height: number };

type PassId = number;
type PassCompilation =
  | { status: typeof MewoPassCompileStatus.Pending }
  | {
      status: typeof MewoPassCompileStatus.Success | typeof MewoPassCompileStatus.Error;
      duration: number;
    };

type Pass = {
  id: PassId;
  name: string;
  compilation: PassCompilation | null;
  diagnostics: PassDiagnostic[];
};

/** Maps to `MEWO_PASS_ID_NONE`. */
const kPassIdNone: PassId = 0 as const;

export {
  MewoDiagnosticSeverity,
  MewoDiagnosticSource,
  MewoEvent,
  MewoPassCompileStatus,
  MewoResult,
  kNull,
  kPassIdNone,
};
export type { Ptr, PassDiagnostic, CommonDiagnostic, Dimensions, PassId, PassCompilation, Pass };
