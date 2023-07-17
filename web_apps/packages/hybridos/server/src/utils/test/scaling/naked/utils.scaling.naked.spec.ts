import { computeNakedValue } from '../../../utils';
import { nakedTestCases } from './testCases';

describe('computeNakedValue', () => {
  nakedTestCases.forEach((testCase) => {
    it(`should calculate the correct displayValue and UIScalar for naked publish: ${JSON.stringify(
      testCase.params,
    )}`, () => {
      const result = computeNakedValue(
        testCase.params.value,
        testCase.params.scalar,
        testCase.params.targetUnits,
      );

      expect(result).toStrictEqual(testCase.expected);
    });
  });
});
