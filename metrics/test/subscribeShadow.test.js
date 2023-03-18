import { subscribeShadowTestCases } from "./testCases";

process.argv.push("./test/samples/empty_metrics.json");

const metrics = require("../src/metrics");

const testCases = subscribeShadowTestCases();

describe("test subscribeShadow", () => {
  beforeEach(() => {
    metrics.setSubscribes([]);
  });

  Object.entries(testCases).forEach(([testName, testCase]) => {
    test(testName, () => {
      testCase.inputs.forEach((uri) => {
        metrics.subscribeShadow(uri);
      });
      expect(metrics.getSubscribes()).toStrictEqual(testCase.output.sort());
    });
  });
});
