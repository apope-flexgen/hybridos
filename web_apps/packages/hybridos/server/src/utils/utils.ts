// we probably won't have any units smaller than pico or

import { SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';

// larger than tera, but this can easily be expanded as needed
const SIPrefixMap = {
  0.000000000001: 'p',
  0.000000001: 'n',
  0.000001: 'μ',
  0.001: 'm',
  0.01: 'c',
  0.1: 'd',
  1: '',
  10: 'da',
  100: 'h',
  1000: 'k',
  1000000: 'M',
  1000000000: 'G',
  1000000000000: 'T',
};

const SIPrefixMapReverse = {
  p: 0.000000000001,
  n: 0.000000001,
  μ: 0.000001,
  m: 0.001,
  c: 0.01,
  d: 0.1,
  '': 1,
  da: 10,
  h: 100,
  k: 1000,
  M: 1000000,
  G: 1000000000,
  T: 1000000000000,
};

export const getSIPrefixFromNumber = (prefixNumber: number): string => {
  return SIPrefixMap[prefixNumber] || '';
};

export const getScalerFromSIPrefix = (prefixString: string): number => {
  return SIPrefixMapReverse[prefixString] || 1;
};

const extractSIPrefix = (units: string): string => {
  let siPrefix: string = '';

  if (units === '') {
    return siPrefix;
  }

  // da is the only metric prefix that is not a single character
  if (units.slice(0, 2) === 'da') {
    siPrefix = 'da';
  } else if (units[0] in SIPrefixMapReverse) {
    siPrefix = units[0];
  }

  return siPrefix;
};

export const computeNakedValue = (
  value: number = 1,
  scalar: number = 1,
  targetUnits: string = '',
  precision: number = 2,
): { value: string; scalar: number } => {
  const baseValue = value * scalar;

  const siPrefix = extractSIPrefix(targetUnits);

  const displayValue = calcDisplayValue(baseValue, siPrefix, precision);
  const uiScalar = calcUIScalar(scalar, siPrefix);

  return { value: displayValue, scalar: uiScalar };
};

export const computeClothedValue = (
  value: number = 1,
  scalar: number = 1,
  unit: string = '',
  siteConfiguration: SiteConfiguration,
  precision: number = 2,
): { value: string; scalar: number; targetUnit: string } => {
  const { units: unitsMap } = siteConfiguration;

  const siPrefix: string = extractSIPrefix(unit);
  const baseUnit: string = unit.replace(siPrefix, '');

  const trueScalar: number = scalar * SIPrefixMapReverse[siPrefix];
  const baseValue = value * trueScalar;

  const targetPrefix: string = unitsMap[baseUnit] ?? siPrefix;

  // calculate the display value and the scalar that the UI will use to
  // scale data inputted on the ui to the correct SI units
  const displayValue = calcDisplayValue(baseValue, targetPrefix, precision);
  const uiScalar = calcUIScalar(trueScalar, targetPrefix);

  return {
    value: displayValue,
    scalar: uiScalar,
    targetUnit: targetPrefix + baseUnit,
  };
};

const calcUIScalar = (scalar: number, siPrefix: string): number => {
  const uiScalar = siPrefix in SIPrefixMapReverse ? SIPrefixMapReverse[siPrefix] / scalar : scalar;
  return uiScalar;
};

const calcDisplayValue = (value: number, siPrefix: string, precision: number = 2): string => {
  const displayValue =
    siPrefix in SIPrefixMapReverse ? value / SIPrefixMapReverse[siPrefix] : value;

  return displayValue.toFixed(precision);
};

export const UriIsRootOfUri = (uri: string, root: string): boolean => {
  if (uri.endsWith('/')) {
    uri = uri.slice(0, -1);
  }
  if (root.endsWith('/')) {
    root = root.slice(0, -1);
  }
  if (uri === root) {
    return true;
  }
  if (uri.startsWith(root) && uri.charAt(root.length) === '/') {
    return true;
  }
  return false;
};

export const TestUriIsRootOfUri = (): void => {
  const testCases = [
    { uri: '/a', root: '/a', expected: true },
    { uri: '/a', root: '/a/', expected: true },
    { uri: '/a', root: '/a/b', expected: false },
    { uri: '/a/b', root: '/a', expected: true },
    { uri: '/a/b', root: '/a/', expected: true },
    { uri: '/a/b', root: '/a/b', expected: true },
    { uri: '/a/b', root: '/a/b/', expected: true },
    { uri: '/a/b', root: '/a/b/c', expected: false },
    { uri: '/a/b/c', root: '/a', expected: true },
    { uri: '/a/b/c', root: '/a/', expected: true },
    { uri: '/a/b/c', root: '/a/b', expected: true },
    { uri: '/a/b/c', root: '/a/b/', expected: true },
    { uri: '/a/b/c', root: '/a/b/c', expected: true },
    { uri: '/a/b/c', root: '/a/b/c/', expected: true },
    { uri: '/a/b/c', root: '/a/b/c/d', expected: false },
    { uri: '/a/b/c', root: '/', expected: true },
    { uri: '/a/b/c', root: '', expected: true },
    { uri: '/a/b/c', root: 'a', expected: false },
    { uri: '/a/b/c', root: 'a/', expected: false },
    { uri: '/a/b/c', root: 'a/b', expected: false },
  ];

  testCases.forEach((testCase) => {
    const result = UriIsRootOfUri(testCase.uri, testCase.root);
    if (result !== testCase.expected) {
      throw new Error(
        `UriIsRootOfUri(${testCase.uri}, ${testCase.root}) returned ${result} but expected ${testCase.expected}`,
      );
    }
  });
};
