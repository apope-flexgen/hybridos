import { User } from 'src/users/dtos/user.dto';
import { PermissionLevel } from '../../interfaces/permission.interface';

interface GetAccessLevelTestCase {
  params: { role: string; uri: string };
  expected: number;
}
interface ConfirmRoleAccessTestCase {
  params: { user: User; permissionLevel: PermissionLevel; uri: string };
  expected: boolean;
}

export const getAccessLevelTestCases: GetAccessLevelTestCase[] = [
  {
    params: {
      role: 'user',
      uri: '/assets/ess',
    },
    expected: 2,
  },
  {
    params: {
      role: 'user',
      uri: '/assets/ess/ess_1/active_power',
    },
    expected: 2,
  },
  {
    params: {
      role: 'user',
      uri: '/assets/ess/ess_1',
    },
    expected: 0,
  },
  {
    params: {
      role: 'user',
      uri: '/assets/ess/ess_1/reactive_power',
    },
    expected: 1,
  },
  {
    params: {
      role: 'admin',
      uri: '/assets/ess/ess_1/active_power',
    },
    expected: 0,
  },

  /** dbi */
  {
    params: {
      role: 'admin',
      uri: '/dbi/assets',
    },
    expected: 2,
  },
  {
    params: {
      role: 'admin',
      uri: '/dbi/solar',
    },
    expected: 1,
  },
  {
    params: {
      role: 'admin',
      uri: '/dbi',
    },
    expected: 1,
  },
  {
    params: {
      role: 'user',
      uri: '/dbi',
    },
    expected: 0,
  },
  {
    params: {
      role: 'user',
      uri: '/dbi/assets',
    },
    expected: 0,
  },
  {
    params: {
      role: 'user',
      uri: '/dbi/solar',
    },
    expected: 0,
  },

  /** /sites and /components replacement */

  {
    params: {
      role: 'rest',
      uri: '/sites/batcave/active_power',
    },
    expected: 2,
  },
  {
    params: {
      role: 'rest',
      uri: '/sites/mustafar/active_power',
    },
    expected: 0,
  },
  {
    params: {
      role: 'rest',
      uri: '/sites/mustafar/reactive_power',
    },
    expected: 2,
  },
  {
    params: {
      role: 'rest',
      uri: '/sites/batcave/reactive_power',
    },
    expected: 2,
  },

  {
    params: {
      role: 'rest',
      uri: '/components/batcave/comp1',
    },
    expected: 1,
  },
  {
    params: {
      role: 'rest',
      uri: '/components/mustafar/comp1',
    },
    expected: 0,
  },
  {
    params: {
      role: 'rest',
      uri: '/components/batcave/comp2',
    },
    expected: 1,
  },
  {
    params: {
      role: 'rest',
      uri: '/components/mustafar/comp2',
    },
    expected: 1,
  },
  {
    params: {
      role: 'rest',
      uri: '/components/hoth/comp2',
    },
    expected: 2,
  },

  /** scheduler special case tests */
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/local_schedule',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/local_schedule/name',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/web_sockets/clients',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/web_sockets/clients/{clientId}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/web_sockets/clients/1234',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/web_sockets/clients/1234/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/web_sockets/clients/1234/ip',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/web_sockets/clients/{clientId}/ip',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/web_sockets/clients/abcd/ip',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/web_sockets/clients/abcd/port',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/web_sockets/clients/{clientId}/port',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/web_sockets/clients/1234/port',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/scada/num_strings',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerConfiguration',
      uri: '/scheduler/configuration/scada/num_bools',
    },
    expected: 0,
  },

  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/name',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/name',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/name',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/name/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/name/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/{setpointType}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/color_code',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/color_code',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/color_code',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/variables',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/constants',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/variables',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/variables',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/constants',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/constants',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/{setpointType}/{setpointId}',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/{setpointType}/abcd',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/{setpointType}/abcd',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/variables/abcd',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/qwerty/variables/abcd',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/qwerty/constants/abcd',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/qwerty/variables/1234',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/qwerty/variables/{setpointId}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/qwerty/abcd',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/{setpointType}/{setpointId}/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/{setpointType}/{setpointId}/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/{setpointType}/abcd/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/variables/abcd/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/qwerty/abcd/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/{setpointType}/{setpointId}/unit',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/{setpointType}/{setpointId}/unit',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/{setpointType}/1234/unit',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/variables/1234/unit',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/constants/{setpointId}/unit',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/qwerty/{setpointId}/unit',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/constants/{setpointId}/name',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/constants/{setpointId}/name',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/variables/{setpointId}/name',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/constants/1234/name',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/constants/qwerty/value',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/variables/qwerty/value',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/1234/constants/qwerty/value',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/{modeId}/constants/qwerty/value',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerModes',
      uri: '/scheduler/modes/abcd/qwerty/qwerty/value',
    },
    expected: 0,
  },

  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/abcd',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/1234',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/qwerty',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/abcd',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/1234/{eventId}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/1234/abcd',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}/start_time',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/abcd/start_time',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/1234/{eventId}/start_time',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/1234/abcd/start_time',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}/variables',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/abcd/variables',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/1234/{eventId}/variables',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/abcd/1234/variables',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}/variables/{variableId}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/abcd/variables/{variableId}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/1234/{eventId}/variables/{variableId}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/qwerty/variables/{variableId}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}/variables/1234',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/1234/variables/abcd',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}/variables/{variableId}/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}/variables/1234/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/abcd/variables/{variableId}/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/1234/{eventId}/variables/qwerty/extraPart',
    },
    expected: 0,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}/repeat/exceptions',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/1234/repeat/exceptions',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/abcd/{eventId}/repeat/exceptions',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/1234/abcd/repeat/exceptions',
    },
    expected: 1,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}/repeat/exceptions/{exceptionIndex}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/1234/{eventId}/repeat/exceptions/{exceptionIndex}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/qwerty/repeat/exceptions/{exceptionIndex}',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}/repeat/exceptions/14',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/{eventId}/repeat/exceptions/5',
    },
    expected: 2,
  },
  {
    params: {
      role: 'schedulerEvents',
      uri: '/scheduler/events/{scheduleId}/abcd/repeat/exceptions/1234',
    },
    expected: 2,
  },
];

const expectedNumToExpected = (expected: number, permLevel: PermissionLevel): boolean => {
  switch (expected) {
    case 0:
      return false;
    case 1:
      return permLevel === PermissionLevel.READ;
    case 2:
      return true;
    default:
      return null;
  }
};

export const confirmRoleAccessTestCases: ConfirmRoleAccessTestCase[] =
  getAccessLevelTestCases.flatMap((testCase) => {
    const readTestCase: ConfirmRoleAccessTestCase = {
      params: {
        user: {
          role: testCase.params.role,
          username: 'username',
        },
        permissionLevel: PermissionLevel.READ,
        uri: testCase.params.uri,
      },
      expected: expectedNumToExpected(testCase.expected, PermissionLevel.READ),
    };
    const readWriteTestCase: ConfirmRoleAccessTestCase = {
      params: {
        user: {
          role: testCase.params.role,
          username: 'username',
        },
        permissionLevel: PermissionLevel.READ_WRITE,
        uri: testCase.params.uri,
      },
      expected: expectedNumToExpected(testCase.expected, PermissionLevel.READ_WRITE),
    };

    return [readTestCase, readWriteTestCase];
  });
