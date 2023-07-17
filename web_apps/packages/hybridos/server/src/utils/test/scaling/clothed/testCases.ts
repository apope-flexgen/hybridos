interface ClothedParams {
  value: number;
  scalar: number;
  unit: string;
  siteConfiguration: {
    units: {
      [key: string]: string;
    };
  };
}

interface ClothedResult {
  value: string;
  scalar: number;
  targetUnit: string;
}

interface ClothedTestCase {
  params: ClothedParams;
  expected: ClothedResult;
}

export const clothedTestCases: ClothedTestCase[] = [
  {
    params: {
      value: 1000,
      scalar: 1000,
      unit: 'W',
      siteConfiguration: {
        units: {
          W: 'M',
        },
      },
    },
    expected: {
      value: '1.00',
      targetUnit: 'MW',
      scalar: 1000,
    },
  },
  {
    params: {
      value: 10000,
      scalar: 1,
      unit: 'W',
      siteConfiguration: {
        units: {
          W: 'M',
        },
      },
    },
    expected: {
      value: '0.01',
      targetUnit: 'MW',
      scalar: 1000000,
    },
  },
  {
    params: {
      value: 1000,
      scalar: 1000,
      unit: 'W',
      siteConfiguration: {
        units: {
          W: 'k',
        },
      },
    },
    expected: {
      value: '1000.00',
      targetUnit: 'kW',
      scalar: 1,
    },
  },
  {
    params: {
      value: 1000,
      scalar: 1,
      unit: 'kW',
      siteConfiguration: {
        units: {},
      },
    },
    expected: {
      value: '1000.00',
      targetUnit: 'kW',
      scalar: 1,
    },
  },
  {
    params: {
      value: 1000,
      scalar: 1,
      unit: 'kVAR/s',
      siteConfiguration: {
        units: { VAR: 'M' },
      },
    },
    expected: {
      value: '1000.00',
      targetUnit: 'kVAR/s',
      scalar: 1,
    },
  },
  {
    params: {
      value: 3,
      scalar: 1000,
      unit: 'kW',
      siteConfiguration: {
        units: { W: 'M' },
      },
    },
    expected: {
      value: '3.00',
      targetUnit: 'MW',
      scalar: 1,
    },
  },
  {
    params: {
      value: 3,
      scalar: 1000,
      unit: 'MW',
      siteConfiguration: {
        units: { W: 'M' },
      },
    },
    expected: {
      value: '3000.00',
      targetUnit: 'MW',
      scalar: 0.001,
    },
  },
  {
    params: {
      value: 3,
      scalar: 1,
      unit: 'GW',
      siteConfiguration: {
        units: { W: 'M' },
      },
    },
    expected: {
      value: '3000.00',
      targetUnit: 'MW',
      scalar: 0.001,
    },
  },
  {
    params: {
      value: 3,
      scalar: 1000,
      unit: 'MW',
      siteConfiguration: {
        units: { W: 'M' },
      },
    },
    expected: {
      value: '3000.00',
      targetUnit: 'MW',
      scalar: 0.001,
    },
  },
  {
    params: {
      value: 3,
      scalar: 1000,
      unit: 'MW',
      siteConfiguration: {
        units: { W: 'k' },
      },
    },
    expected: {
      value: '3000000.00',
      targetUnit: 'kW',
      scalar: 0.000001,
    },
  },
  {
    params: {
      value: 0.5,
      scalar: 1,
      unit: 'GW',
      siteConfiguration: {
        units: { W: 'M' },
      },
    },
    expected: {
      value: '500.00',
      targetUnit: 'MW',
      scalar: 0.001,
    },
  },
  {
    params: {
      value: 0.5,
      scalar: 1,
      unit: 'GW',
      siteConfiguration: {
        units: { W: 'G' },
      },
    },
    expected: {
      value: '0.50',
      targetUnit: 'GW',
      scalar: 1,
    },
  },
  {
    params: {
      value: 10000,
      scalar: 1,
      unit: 'V',
      siteConfiguration: {
        units: { V: 'k' },
      },
    },
    expected: {
      value: '10.00',
      targetUnit: 'kV',
      scalar: 1000,
    },
  },
  {
    params: {
      value: 1,
      scalar: 1000,
      unit: 'kVAR',
      siteConfiguration: {
        units: {
          W: 'M',
        },
      },
    },
    expected: {
      value: '1000.00',
      targetUnit: 'kVAR',
      scalar: 0.001,
    },
  },
];
