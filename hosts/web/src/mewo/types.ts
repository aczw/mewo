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

type Dimensions = { width: number; height: number };

type PassId = number;
type Pass = { id: PassId; name: string };

/** Maps to `MEWO_PASS_ID_NONE`. */
const kPassIdNone: PassId = 0 as const;

export { MewoEvent, MewoResult, kNull, kPassIdNone };
export type { Ptr, Dimensions, PassId, Pass };
