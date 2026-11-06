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

export { type Ptr, type Dimensions, MewoEvent, MewoResult, kNull };
