import { SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';
import { computeClothedValue } from '../../../utils';
import { clothedTestCases } from './testCases';

describe('utils', () => {
  describe('computeClothedValue', () => {
    clothedTestCases.forEach((testCase) => {
      it(`should calculate the correct displayValue and UIScalar for clothed publish: ${JSON.stringify(
        testCase.params,
      )}`, () => {
        const result = computeClothedValue(
          testCase.params.value,
          testCase.params.scalar,
          testCase.params.unit,
          testCase.params.siteConfiguration as unknown as SiteConfiguration,
        );

        expect(result).toStrictEqual(testCase.expected);
      });
    });
  });
});
