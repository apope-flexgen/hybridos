import { configureMdoTestFilepaths } from "./testCases";

const testCases = configureMdoTestFilepaths();

describe("testing configureMdo", () => {
  beforeEach(() => {
    jest.resetModules();
  });

  Object.entries(testCases).forEach(([testName, testCase]) => {
    test(testName, () => {
      while (process.argv.length > 2) process.argv.pop();
      process.argv.push(testCase.filePath);

      const metrics = require("../src/metrics");

      const mdo = metrics.configureMdo();

      expect(metrics.getSubscribes()).toStrictEqual(
        testCase.subscribesOutput.sort()
      );

      expect(mdo).toEqual(testCase.expectedMdo);
    });
  });
});
