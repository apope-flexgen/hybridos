interface NakedParams {
  value: number;
  scalar: number;
  targetUnits: string;
}

interface NakedResult {
  value: string;
  scalar: number;
}

interface NakedTestCase {
  params: NakedParams;
  expected: NakedResult;
}

export const nakedTestCases: NakedTestCase[] = [
  {
    params: { value: 1000, scalar: 1, targetUnits: 'V' },
    expected: {
      value: '1000.00',
      scalar: 1,
    },
  },
  {
    params: { value: 1000, scalar: 1000, targetUnits: 'MW' },
    expected: {
      value: '1.00',
      scalar: 1000,
    },
  },
  {
    params: { value: 1, scalar: 1000, targetUnits: 'W' },
    expected: {
      value: '1000.00',
      scalar: 0.001,
    },
  },
  {
    params: { value: 1000, scalar: 1000, targetUnits: 'kVAR/s' },
    expected: {
      value: '1000.00',
      scalar: 1,
    },
  },
  {
    params: { value: 1, scalar: 1000000, targetUnits: 'kVAR/s' },
    expected: {
      value: '1000.00',
      scalar: 0.001,
    },
  },
];
