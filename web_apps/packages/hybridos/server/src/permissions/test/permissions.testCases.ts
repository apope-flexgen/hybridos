interface GetAccessLevelTestCase {
  params: { role: string; uri: string };
  expected: number[];
}

export const getAccessLevelWildcardTestCases: GetAccessLevelTestCase[] = [
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess/ess_1/active_power',
    },
    expected: [1, 4],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess/ess_1',
    },
    expected: [2, 3],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess',
    },
    expected: [1, 2],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess/ess_1/reactive_power',
    },
    expected: [2, 3],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess/ess_2/reactive_power_setpoint',
    },
    expected: [2, 3],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess/ess_3/voltage',
    },
    expected: [2, 3],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess/qwerty',
    },
    expected: [2, 3],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess/qwerty/maint_mode',
    },
    expected: [2, 3],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess/asdf',
    },
    expected: [0, 3],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess/asdf/active_power',
    },
    expected: [0, 3],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/assets/ess/asdf/maint_mode',
    },
    expected: [0, 3],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/batcave',
    },
    expected: [2, 2],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/texasTown/assets/ess/asdf/maint_mode',
    },
    expected: [0, 5],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/batcave/assets/ess/ess_1/active_power',
    },
    expected: [1, 6],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/texasTown/assets/ess/ess_1',
    },
    expected: [2, 5],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/batcave/assets/ess',
    },
    expected: [1, 4],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/batcave/assets/ess/ess_1/reactive_power',
    },
    expected: [2, 5],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/texasTown/assets/ess/ess_2/reactive_power_setpoint',
    },
    expected: [2, 5],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/texasTown/assets/ess/ess_3/voltage',
    },
    expected: [2, 5],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/batcave/assets/ess/qwerty',
    },
    expected: [2, 5],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/texasTown/assets/ess/qwerty/maint_mode',
    },
    expected: [2, 5],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/batcave/assets/ess/asdf',
    },
    expected: [0, 5],
  },
  {
    params: {
      role: 'wildcards',
      uri: '/sites/batcave/assets/ess/asdf/active_power',
    },
    expected: [0, 5],
  },
];
