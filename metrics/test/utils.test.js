import { uriIsRootOfUriTestCases } from "./testCases";
import { UriIsRootOfUri } from "../src/utils";

const testCases = uriIsRootOfUriTestCases();

describe("test UriIsRootOfUri", () => {
  testCases.forEach((testCase) => {
    test(`${testCase.uri} - ${testCase.root} : ${testCase.expected}`, () => {
      expect(UriIsRootOfUri(testCase.uri, testCase.root)).toBe(
        testCase.expected
      );
    });
  });
});
